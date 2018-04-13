#include "tim5.h"



//通用定时器5中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器5!

//例子：TIM5_Int_Init(4999,7199);//10Khz的计数频率，计数到5000为500ms  
//定时器周期计算公式：[(1+TIM_Prescaler )*(1+TIM_Period )]/72M = T (s)
//设定定时周期为：1s  1.TIM_Prescaler = 9999 2.TIM_Period=7199

void TIM5_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE); //时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器


	TIM_Cmd(TIM5, ENABLE);  //使能TIMx					 
}
//定时器5中断服务程序
//一些需要计时用的全局变量
//洗衣用的count变量
extern u8 Washer_Mode;//洗衣模式标志位
extern u8 Dry_Mode;//干衣模式标志位
u16 waterin_count=0;//进水计数值
u16 wash_count=0;//洗衣计数值
u16 waterout_count=0;//排水计数值
u16 dry_count = 0;//已干燥时间计数值
u16 work_count = 0;//已工作的总时间计数值
extern u16 WATERIN_TIME; //进水时间变量
extern int WASH_TIME; //洗衣时间变量
extern u16 WATEROUT_TIME; //排水时间变量

extern u8 washDrying_flag;
//微波脉冲式驱动加热用的变量
extern u8 microwave_mode;//微波加热状态标志位
u8 microwave_cnt=0;
void TIM5_IRQHandler(void)   //TIM5中断
{
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)  //检查TIM5更新中断发生与否
		{
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update  );  //清除TIMx更新中断标志 
			

			//****************如果洗衣模式打开*************************
			if(Washer_Mode)
			{
				if(washDrying_flag)
					work_count++;
				if(waterin_count < WATERIN_TIME )
					waterin_count++;
				
				else if((wash_count < WASH_TIME )/*&&(waterin_count >= WATERIN_TIME)*/)
					wash_count++;
				
				else if((waterout_count < WATEROUT_TIME)/*&&(wash_count >= WASH_TIME )&&(waterin_count >= WATERIN_TIME)*/)
					waterout_count++;
				
			}
			
			//****************如果干衣模式打开*************************
			if(Dry_Mode)
			{
				if(washDrying_flag)
					work_count++;
				dry_count++;
			}
			//****************如果微波加热打开*************************
//			if(microwave_mode)
//			{
//				microwave_cnt++;
//				
//			}
								
		}
}












