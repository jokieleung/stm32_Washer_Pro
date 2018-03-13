#include "tim4.h"

/* 机智云用户区当前设备状态结构体*/
extern dataPoint_t currentDataPoint;

//通用定时器4中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器4!

//例子：TIM4_Int_Init(4999,7199);//10Khz的计数频率，计数到5000为500ms  
//定时器周期计算公式：[(1+TIM_Prescaler )*(1+TIM_Period )]/72M = T (s)
//设定定时周期为：1s  1.TIM_Prescaler = 9999     2.TIM_Period=7199

void TIM4_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //时钟使能
	
	//定时器TIM4初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  //从优先级1级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器


	TIM_Cmd(TIM4, ENABLE);  //使能TIMx					 
}
//定时器4中断服务程序
u8 KeyDownNum=0;//按键值全局变量
extern u16 WASH_TIME;
//UI跳转指令 数组
u8 START_W_ARY[] = {0XA5,0X5A,0X04,0X80,0X03,0X00,0X06};//正在洗衣界面
u8 START_D_ARY[] = {0XA5,0X5A,0X04,0X80,0X03,0X00,0X0A};//正在烘干界面
u8 START_WD_ARY[] = {0XA5,0X5A,0X04,0X80,0X03,0X00,0X09};//正在烘洗界面


//10ms计时数
u8 TenMsCnt = 0;
//200ms计时数
u8 Cnt200Ms = 0;
//全局洗衣机工作状态位
extern u8 washing_flag,drying_flag,washDrying_flag;

//变量值，传感值的全局变量 
extern float Temp,Humi,Pres;
extern int RemainWashTime,DryTime,WashDryTime;

//洗烘时间计数值
extern u16 work_count;

//声明函数
void userHandle(void);

void TIM4_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
		{
			TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );  //清除TIMx更新中断标志 
			
			
			
			//**************每80ms处理一次机智云协议******************************
			TenMsCnt++;
			if(TenMsCnt >=8){
				TenMsCnt = 0;
				userHandle();//用户采集（上行逻辑）
				gizwitsHandle((dataPoint_t *)&currentDataPoint);//协议处理（下行逻辑）
			}
			//**************每200ms获取并更新一次传感器的值******************************
			Cnt200Ms++;
			if(Cnt200Ms >=20){
				Cnt200Ms = 0;
				GetEveryDisPara();
				UpdateEveryDisPara(&Temp,&Humi,&Pres,&RemainWashTime,&DryTime,&WashDryTime);
			}
			
			//**************每10ms检测一次音箱语音控制信号端******************************
			VoiceDetect();
			
			//*****新增机智云和音响控制，将switch逻辑改回if 2018.3.6*********************
			//*****每10ms检测一次串口屏按键**********************************************
			//@20180307 更改为获取键值后设置flag,在main.c的while(1)内做动作，避免中断冲突
			KeyDownNum = ifButtonDown();
			
			if (KeyDownNum == BUTTON1_NUM) {	//洗衣界面按键 
				Rst_Wash();//重置洗衣参数
				UpdateRemainingWashTim(&RemainWashTime);//更新一次屏幕剩余洗衣时间值
				goto tim4_ir_end;
			}
			if (KeyDownNum==BUTTON2_NUM)  {	//烘衣界面按键 
				Rst_Dry();//重置洗衣参数
			}
			if (KeyDownNum==BUTTON3_NUM)  {	//洗烘界面按键
				Rst_Wash();//重置洗衣参数
				Rst_Dry();//重置干衣参数
				work_count = 0;
				goto tim4_ir_end;
			}
			if(KeyDownNum==BUTTON4_NUM){	//开始洗衣按键
				washing_flag = 1;
				goto tim4_ir_end;
			}
			if (KeyDownNum==BUTTON5_NUM) {	//开始干衣按键
				drying_flag = 1;
				goto tim4_ir_end;
			}
			if (KeyDownNum==BUTTON6_NUM) {	//开始洗烘按键
				washDrying_flag = 1;
				goto tim4_ir_end;
			}
			if (KeyDownNum==BUTTON7_NUM)  {	//结束按键  停止所有洗烘操作  
				washing_flag = 0;
				drying_flag = 0;
				washDrying_flag = 0;
				goto tim4_ir_end;
			}
			if (KeyDownNum==BUTTON8_NUM)  {	//用于加减洗衣时间
//				WASH_TIME = ((USART2_RX_BUF[7]<<2)+USART2_RX_BUF[8])*60;
				goto tim4_ir_end;
			}
			if (KeyDownNum==BUTTON9_NUM)  {	//暂时作为仿真达到干燥条件跳出循环用的按键
				goto tim4_ir_end;
			}
			
			//**************************END  2018.3.6************************************
			
			tim4_ir_end:;

		}
}












