#include "USART_TO_LCD.H"
#include "stm32f10x_usart.h"

//串口2使用引脚: tx:PA.2 rx:PA.3

//#if EN_USART2_RX   //如果使能了接收
//串口2中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART2_RX_BUF[USART2_REC_LEN];     //接收缓冲,最大USART2_REC_LEN个字节.
//接收状态(若为要求是接收数据必须是0x0d 0x0a 结尾的)
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
	 
//接收状态(若不要求接收数据必须是0x0d 0x0a 结尾的)
//bit15，	接收完成标志
//bit14~0，	接收到的有效字节数目
u16 USART2_RX_STA=0;       //接收状态标记	  
//*******************button*********************************************
//按键键值数组·
u8 WASH_BT[] = {0XA5,0X5A,0X06,0X83,0X00,0X00,0X01,0X01,0X11};
u8 DRY_BT[] = {0XA5,0X5A,0X06,0X83,0X00,0X00,0X01,0X02,0X22};
u8 WSH_D_B[] = {0XA5,0X5A,0X06,0X83,0X00,0X00,0X01,0X03,0X33};
//开始洗衣、干衣、洗烘
u8 START_W[] = {0XA5,0X5A,0X06,0X83,0X00,0X00,0X01,0X01,0X12};
u8 START_D[] = {0XA5,0X5A,0X06,0X83,0X00,0X00,0X01,0X02,0X21};
u8 START_WD[] = {0XA5,0X5A,0X06,0X83,0X00,0X00,0X01,0X03,0X32};
//结束
u8 END[] = {0XA5,0X5A,0X06,0X83,0X00,0X00,0X01,0X04,0X44};
//加减洗衣时间
u8 WSH_TIM[] = {0XA5,0X5A,0X06,0X83,0X00,0X50,0X01};
//进\出水时间
u8 WATER_IN_TIME[] = {0XA5,0X5A,0X06,0X83,0X00,0X54,0X01};
u8 WATER_OUT_TIME[] = {0XA5,0X5A,0X06,0X83,0X00,0X56,0X01};
//*******************button(end)**************************************

//初始化发送数值变量用的数组
u8 PRE_ARY[] = {0XA5,0X5A,0X05,0X82,0X00,0X20,0X00,0X00};//压力值显示数组 
u8 TMP_ARY[] = {0XA5,0X5A,0X05,0X82,0X00,0X30,0X00,0X00};//温度值显示数组 
u8 HUM_ARY[] = {0XA5,0X5A,0X05,0X82,0X00,0X40,0X00,0X00};//湿度值显示数组 
//剩余时间显示数组
u8 WSH_ARY_M[] = {0XA5,0X5A,0X05,0X82,0X00,0X50,0X00,0X00};//洗衣剩余时间显示数组 
u8 WSH_ARY_S[] = {0XA5,0X5A,0X05,0X82,0X00,0X51,0X00,0X00};//洗衣剩余时间显示数组 
//进出水时间显示数组
//u8 WSH_IN_M[] = {0XA5,0X5A,0X05,0X82,0X00,0X53,0X00,0X00};//洗衣剩余时间显示数组 
u8 WSH_IN_S[] = {0XA5,0X5A,0X05,0X82,0X00,0X54,0X00,0X00};//洗衣剩余时间显示数组 
//u8 WSH_OUT_M[] = {0XA5,0X5A,0X05,0X82,0X00,0X55,0X00,0X00};//洗衣剩余时间显示数组 
u8 WSH_OUT_S[] = {0XA5,0X5A,0X05,0X82,0X00,0X56,0X00,0X00};//洗衣剩余时间显示数组 
//已用时间显示数组
u8 DRY_ARY_M[] = {0XA5,0X5A,0X05,0X82,0X00,0X60,0X00,0X00};//干衣已用时间显示数组 
u8 DRY_ARY_S[] = {0XA5,0X5A,0X05,0X82,0X00,0X61,0X00,0X00};//干衣已用时间显示数组 
u8 WLE_ARY_M[] = {0XA5,0X5A,0X05,0X82,0X00,0X70,0X00,0X00};//洗干衣已用时间显示数组 
u8 WLE_ARY_S[] = {0XA5,0X5A,0X05,0X82,0X00,0X71,0X00,0X00};//洗干衣已用时间显示数组 
//洗衣状态标志位
extern u8 washing_flag;
//变量值，传感值的全局变量 
float Temp,Humi,Pres;
int RemainWashTime,DryTime,WashDryTime;

void USART2_Init(u32 bound)
{
   //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 //使能USART2，GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
	//USART2_TX   GPIOA.2
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA2  TXD
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  	GPIO_Init(GPIOA, &GPIO_InitStructure);
   
	//USART2_RX	  GPIOA.3初始化
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3  RXD
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA3  
  
	//Usart2 NVIC 配置		此时暂时设置其优先级为最高
  	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

   	//USART2 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	
  	USART_Init(USART2, &USART_InitStructure); //初始化串口2

  	USART_ITConfig(USART2, USART_IT_RXNE|USART_IT_IDLE, ENABLE);//开启串口接受和总线空闲中断
  	
	USART_Cmd(USART2, ENABLE);                    //使能串口2 	
}

//功能 ：更新显示屏温度值
void UpdateDisTemp(float *temp){
	//声明要用的变量
	u16 dis;
	u8 low,high,i;
	u8 ARYlength;
	//获取温度值
	//*******************
//	printf("温度是：%.2f\n",Temp);	
	dis = (int)((*temp) * 10);
	low = dis & 0xff;
	high = (dis>>8) & 0xff;
	TMP_ARY[7] = low;
	TMP_ARY[6] = high;
//		把变量数组里的数据循环发送给串口，每次发送一个字节  Jokie
	ARYlength = sizeof(TMP_ARY)/sizeof(TMP_ARY[0]); //计算数组长度
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, TMP_ARY[i]);//向向串口2把接收的数据重新通过串口2发送
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//等待发送结束
	}
}
//功能 ：更新显示屏湿度值
void UpdateDisHumi(float *Humi){
	//声明要用的变量
	u16 dis;
	u8 low,high,i;
	u8 ARYlength;
//	printf("湿度是：%.2f\n\n",Humi);	
	dis = (int)((*Humi) * 10);
	low = dis & 0xff;
	high = (dis>>8) & 0xff;
	HUM_ARY[7] = low;
	HUM_ARY[6] = high;
//		把变量数组里的数据循环发送给串口，每次发送一个字节  Jokie
	ARYlength = sizeof(HUM_ARY)/sizeof(HUM_ARY[0]); //计算数组长度
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, HUM_ARY[i]);//向向串口2把接收的数据重新通过串口2发送
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//等待发送结束
	}
}

//功能 ：更新显示屏压力值
void UpdateDisPres(float *Pres){
	//声明要用的变量
	int dis;
	u8 low,high,i;
	u8 ARYlength;
	
	//更换成获取AD的压力值
	
	dis = (int)(*Pres);//压力转换为整数,单位Kpa
	low = dis & 0xff;
	high = (dis>>8) & 0xff;
	PRE_ARY[7] = low;
	PRE_ARY[6] = high;
//		把变量数组里的数据循环发送给串口，每次发送一个字节  Jokie
	ARYlength = sizeof(PRE_ARY)/sizeof(PRE_ARY[0]); //计算数组长度
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, PRE_ARY[i]);//向串口2把接收的数据重新通过串口2发送
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//等待发送结束
	}
}
//更新剩余洗衣时间   转换成hh:mm格式
extern u16 waterin_count;//进水计数值
extern u16 wash_count;//洗衣计数值
extern u16 waterout_count;//排水计数值
extern u16 WATERIN_TIME; //进水时间变量
extern int WASH_TIME; //洗衣时间变量
extern u16 WATEROUT_TIME; //排水时间变量

void UpdateRemainingWashTim(int *remainSec){
	int Minu,Sec;
	u8 low,high,i;
	u8 ARYlength;
	//总的剩余时间，单位s
	
//	printf("remainSec : %d \r\n" ,remainSec);
	Minu = ((*remainSec)/60); //（整除）
	Sec = ((*remainSec)% 60); //秒：（取余）
	low = Minu & 0xff;
	high = (Minu>>8) & 0xff;
	WSH_ARY_M[7] = low;
	WSH_ARY_M[6] = high;
	low = Sec & 0xff;
	high = (Sec>>8) & 0xff;
	WSH_ARY_S[7] = low;
	WSH_ARY_S[6] = high;
//		把变量数组里的数据循环发送给串口，每次发送一个字节  Jokie
	ARYlength = sizeof(WSH_ARY_M)/sizeof(WSH_ARY_M[0]); //计算数组长度
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, WSH_ARY_M[i]);//向串口2把接收的数据重新通过串口2发送
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//等待发送结束
	}
	delay_ms(1);
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, WSH_ARY_S[i]);//向串口2把接收的数据重新通过串口2发送
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//等待发送结束
	}
}
//更新进出水时间
void UpdateWashInAndOut(u16 *InSec,u16 *OutSec){
	int Sec;
	u8 low,high,i;
	u8 ARYlength;
	
	Sec = *InSec; //秒
	low = Sec & 0xff;
	high = (Sec>>8) & 0xff;
	WSH_IN_S[7] = low;
	WSH_IN_S[6] = high;
	ARYlength = sizeof(WSH_IN_S)/sizeof(WSH_IN_S[0]); //计算数组长度
//		把变量数组里的数据循环发送给串口，每次发送一个字节  Jokie
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, WSH_IN_S[i]);//向串口2把接收的数据重新通过串口2发送
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//等待发送结束
	}
	
	Sec = *OutSec; //秒：（取余）
	low = Sec & 0xff;
	high = (Sec>>8) & 0xff;
	WSH_OUT_S[7] = low;
	WSH_OUT_S[6] = high;
//		把变量数组里的数据循环发送给串口，每次发送一个字节  Jokie
	ARYlength = sizeof(WSH_OUT_S)/sizeof(WSH_OUT_S[0]); //计算数组长度
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, WSH_OUT_S[i]);//向串口2把接收的数据重新通过串口2发送
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//等待发送结束
	}
	
}

extern u16 dry_count;
//更新已经干衣时间
void UpdateUsedDryTim(int *UsedSec){
	int Minu,Sec;
	u8 low,high,i;
	u8 ARYlength;
	//总的剩余时间，单位s
	
//	printf("remainSec : %d \r\n" ,remainSec);
	Minu = ((*UsedSec)/60); //（整除）
	Sec = ((*UsedSec)% 60); //秒：（取余）
	low = Minu & 0xff;
	high = (Minu>>8) & 0xff;
	DRY_ARY_M[7] = low;
	DRY_ARY_M[6] = high;
	low = Sec & 0xff;
	high = (Sec>>8) & 0xff;
	DRY_ARY_S[7] = low;
	DRY_ARY_S[6] = high;
//		把变量数组里的数据循环发送给串口，每次发送一个字节  Jokie
	ARYlength = sizeof(DRY_ARY_M)/sizeof(DRY_ARY_M[0]); //计算数组长度
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, DRY_ARY_M[i]);//向串口2把接收的数据重新通过串口2发送
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//等待发送结束
	}
	delay_ms(1);
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, DRY_ARY_S[i]);//向串口2把接收的数据重新通过串口2发送
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//等待发送结束
	}
}

extern u16 work_count;
void UpdateUsedWholeTim(int *UsedSec){
	int Minu,Sec;
	u8 low,high,i;
	u8 ARYlength;
	//总的剩余时间，单位s
	
//	printf("remainSec : %d \r\n" ,remainSec);
	Minu = ((*UsedSec)/60); //（整除）
	Sec = ((*UsedSec)% 60); //秒：（取余）
	low = Minu & 0xff;
	high = (Minu>>8) & 0xff;
	WLE_ARY_M[7] = low;
	WLE_ARY_M[6] = high;
	low = Sec & 0xff;
	high = (Sec>>8) & 0xff;
	WLE_ARY_S[7] = low;
	WLE_ARY_S[6] = high;
//		把变量数组里的数据循环发送给串口，每次发送一个字节  Jokie
	ARYlength = sizeof(WLE_ARY_M)/sizeof(WLE_ARY_M[0]); //计算数组长度
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, WLE_ARY_M[i]);//向串口2把接收的数据重新通过串口2发送
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//等待发送结束
	}
	delay_ms(1);
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, WLE_ARY_S[i]);//向串口2把接收的数据重新通过串口2发送
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//等待发送结束
	}
}

//跳转到洗衣完成的界面
void JumpToFinishedUI(){
	u8 ARYlength,i;
	u8 FNSH_PAGE[] = {0XA5,0X5A,0X04,0X80,0X03,0X00,0X07};//跳转洗衣完成页面指令 A5 5A 04 80 03 00 07 
	
	ARYlength = sizeof(FNSH_PAGE)/sizeof(FNSH_PAGE[0]); //计算数组长度
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, FNSH_PAGE[i]);//向串口2把接收的数据重新通过串口2发送
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//等待发送结束
	}
}
//跳转到特定的界面
//输入：7位跳转指令数组  eg. FNSH_PAGE[] = {0XA5,0X5A,0X04,0X80,0X03,0X00,0X07};//跳转到7号页面指令 A5 5A 04 80 03 00 07 
void JumpToUI(u8 PAGE_ARY[]){
	u8 ARYlength,i;
	
	ARYlength = 7;//sizeof(PAGE_ARY)/sizeof(PAGE_ARY[0]); //计算数组长度
	
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, PAGE_ARY[i]);//向串口2把接收的数据重新通过串口2发送
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//等待发送结束
	}
}

//功能：获取所有需要显示的变量值，并赋值到相应的固定指针（地址）内
void GetEveryDisPara(){
	//获取温度值
	Temp = SHT2x_GetTempPoll();
//	printf("Temp:%f\n",Temp);
	//获取湿度值
	Humi = SHT2x_GetHumiPoll();
//	printf("Humi %f\n",Humi);
	//获取压力值
	Pres = GetPresAverage(ADC_Channel_7,10);
//	printf("Pres %f \n",Pres);
	//获取剩余洗衣时间
	//if(washing_flag) //如果处于洗衣工作状态，则RemainWashTime为总的倒计时
		RemainWashTime = (WATERIN_TIME+WASH_TIME+WATEROUT_TIME)-waterin_count-wash_count-waterout_count;
	//else 	//如果处于设置时间状态，则RemainWashTime为洗衣时间
	//	RemainWashTime = WASH_TIME;
//	printf("RemainWashTime %d \n",RemainWashTime);
	//获取干燥时间
	DryTime = dry_count;
//	printf("DryTime %d \n",DryTime);
	//获取洗烘时间
	WashDryTime = work_count;
//	printf("WashDryTime %d \n",WashDryTime);
}
//@功能：获取所有需要显示的变量值，并赋值到相应的固定指针（地址）内
//@para: float *Temp,float *Humi,float *Pres,int *RemainWashTime,int *DryTime,int *WashDryTime

void UpdateEveryDisPara(float *Temp,float *Humi,float *Pres,int *RemainWashTime,int *DryTime,int *WashDryTime){
	//更新温度值
	UpdateDisTemp(Temp);
	//更新湿度值
	UpdateDisHumi(Humi);
	//更新压力值
	UpdateDisPres(Pres);
	//更新剩余洗衣时间
	if(washing_flag)
		UpdateRemainingWashTim(RemainWashTime);
	//更新干燥时间
	UpdateUsedDryTim(DryTime);
	//更新洗烘时间
	UpdateUsedWholeTim(WashDryTime);
}






//功能 ：检测串口屏是否有按键按下
//返回值：
//0：无按键按下
//其他：相应按键的键值
u8 ifButtonDown(){
	u16 t;
	u16 rx_len;
	u8 button1_cnt=0,button2_cnt=0,button3_cnt=0,button4_cnt=0,button5_cnt=0,button6_cnt=0,button7_cnt=0,button8_cnt=0,button9_cnt=0,button10_cnt=0;//用来判定BUTTON BUF与哪个BUTTON的键值是一致的变量			
			//串口2测试  成功  2018-1-22

			rx_len=USART2_RX_STA&0x3fff;//得到此次接收到的数据长度
			if(rx_len!=0)
			{
				USART2_RX_STA=0;		//归零该STA寄存器要在for循环前面（未知为什么）
				
	//			思路：循环获取接收到的BUF内的数据，完成for循环（即获取完成后），将该数据验证是否为对应button键值
				
				//STEP1 把串口2接收BUF与各BUTTON键值比较
				//归零Cnt
				button1_cnt=0;button2_cnt=0;button3_cnt=0;button4_cnt=0;button5_cnt=0;button6_cnt=0;button7_cnt=0;
				button8_cnt=0;button9_cnt=0;button10_cnt=0;
				for(t=0;t<rx_len;t++)
				{
					
					if(WASH_BT[t]==USART2_RX_BUF[t])	button1_cnt++;
					if(DRY_BT[t]==USART2_RX_BUF[t])		button2_cnt++;
					if(WSH_D_B[t]==USART2_RX_BUF[t])	button3_cnt++;
					if(START_W[t]==USART2_RX_BUF[t])	button4_cnt++;
					if(START_D[t]==USART2_RX_BUF[t])	button5_cnt++;
					if(START_WD[t]==USART2_RX_BUF[t])	button6_cnt++;
					if(END[t]==USART2_RX_BUF[t])			button7_cnt++;
					if(WSH_TIM[t]==USART2_RX_BUF[t])	button8_cnt++;  //加减洗衣时间 设置
					//进出水时间设置
					if(WATER_IN_TIME[t]==USART2_RX_BUF[t])	button9_cnt++;
					if(WATER_OUT_TIME[t]==USART2_RX_BUF[t])	button10_cnt++;
					//通过串口1观察 debug用  测试成功 2018.03.02 13：44
	//				USART_SendData(USART1, USART2_RX_BUF[t]);//把接收的串口2数据重新通过串口1发送
	//				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
				}
				
				//STEP2 判定相应按钮是否按下以做出对应的响应
					if(button1_cnt==rx_len){											//按键1
						return BUTTON1_NUM;
					} 
					else if(button2_cnt==rx_len){											//按键2
						return BUTTON2_NUM;
					}
					else if(button3_cnt==rx_len){											//按键3
						return BUTTON3_NUM;
					}
					else if(button4_cnt==rx_len){											//按键4
						return BUTTON4_NUM;
					}
					else if(button5_cnt==rx_len){											//按键5
						return BUTTON5_NUM;
					}
					else if(button6_cnt==rx_len){											//按键6
						return BUTTON6_NUM;
					}
					else if(button7_cnt==rx_len){											//按键7
						return BUTTON7_NUM;
					}
					else if((button8_cnt+2)==rx_len){											//按键8  获取加减洗衣时间变量的值
//						printf("button8_cnt: %d\r\n",button8_cnt);
						WASH_TIME=((USART2_RX_BUF[7]<<2)+USART2_RX_BUF[8])*60;//把赋值放在这里，保证实时性
						UpdateRemainingWashTim(&WASH_TIME);
						return BUTTON8_NUM;
					}
					else if((button9_cnt+2)==rx_len){											//按键9	获取加减进水时间变量的值
						WATERIN_TIME=((USART2_RX_BUF[7]<<2)+USART2_RX_BUF[8]);//把赋值放在这里，保证实时性
						UpdateWashInAndOut(&WATERIN_TIME,&WATEROUT_TIME);
						return BUTTON9_NUM;
					}
					else if((button10_cnt+2)==rx_len){											//按键10	获取加减排水时间变量的值
						WATEROUT_TIME=((USART2_RX_BUF[7]<<2)+USART2_RX_BUF[8]);//把赋值放在这里，保证实时性
						UpdateWashInAndOut(&WATERIN_TIME,&WATEROUT_TIME);
						return BUTTON10_NUM;
					}
					rx_len=0;
			}
			else return 0;
}

void USART2_IRQHandler( void )
{	
//----------------------------------------------------------------------------------------
//-----------------------------在串口中断中接收数据   BY Jokie---------------------------
//----------------------------------------------------------------------------------------
		u8 Res;
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
		//changed by jokie :接收数据不管什么结尾
		if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断(接收数据不管什么结尾)
	{
		Res =USART_ReceiveData(USART2);	//读取接收到的数据
			USART2_RX_BUF[USART2_RX_STA&0X3FFF]=Res ;
			USART2_RX_STA++;
			if(USART2_RX_STA>(USART2_REC_LEN-1))USART2_RX_STA=0;
  }
//----------------------------------------------------------------------------------------
//-----------------------------在串口中断中接收数据   BY Jokie(end)---------------------

}



