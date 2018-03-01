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

//按键键值数组·
u8 BUTTON1[] = {0XA5,0X5A,0X06,0X83,0X00,0X00,0X01,0X01,0X11};
u8 BUTTON2[] = {0XA5,0X5A,0X06,0X83,0X00,0X00,0X01,0X02,0X22};
u8 BUTTON3[] = {0XA5,0X5A,0X06,0X83,0X00,0X00,0X01,0X03,0X33};

//初始化发送数值变量用的数组
u8 PRE_ARY[] = {0XA5,0X5A,0X05,0X82,0X00,0X20,0X00,0X00};//压力值显示数组 
u8 TMP_ARY[] = {0XA5,0X5A,0X05,0X82,0X00,0X30,0X00,0X00};//温度值显示数组 
u8 HUM_ARY[] = {0XA5,0X5A,0X05,0X82,0X00,0X40,0X00,0X00};//湿度值显示数组 

//***********************卖家例程*******************************
//*************************************************************
u16 StartNum=0,TalNum=0;
#define BUFFER_SIZE 2048//指令缓冲区大小，用户可根据自己单片机的容量修改
u8 CommBuff[BUFFER_SIZE];//定义指令缓冲区
#define USER_R3 0xA5//帧头
#define USER_RA 0x5A//帧头

void deal_command()
{
    u16 i,CurNum,tem_TalNum;
    u8 CmdBuf[256];
    u16 nowbuffer,len;
    len=StartNum;
    tem_TalNum=TalNum;
    if(tem_TalNum==len)//
       return;
    if(CommBuff[StartNum]!=USER_R3)
    {
        StartNum++;
        if(StartNum==BUFFER_SIZE)
          StartNum=0;
        return;
    }
    if(tem_TalNum>len)
      nowbuffer=tem_TalNum-len;
    else
      nowbuffer=tem_TalNum+BUFFER_SIZE-len;
    if(nowbuffer<5)
      return;
    CurNum=StartNum+2;
    if(CurNum>BUFFER_SIZE-1)
      CurNum-=BUFFER_SIZE;
    len=CommBuff[CurNum]+3;
    if(nowbuffer<len)
      return;
    i=0;
    CurNum=StartNum;
    while(1)
    {
        CmdBuf[i++]=CommBuff[CurNum++];
        if(CurNum==BUFFER_SIZE)
          CurNum=0;
        if(i==4)
        {
            if(CmdBuf[0]!=USER_R3||CmdBuf[1]!=USER_RA)//
            {
                StartNum=CurNum;
                return;
            }
            len=CmdBuf[2];
        }
        else if(i>4)
        {
            if(i==len+3)//校验完成，正好是一条指令的长度，则break ，用以在接下来进行解析CmdBuf
            {
                StartNum=CurNum;
                break;
            }
            else if(i>255)//
            {
                StartNum=CurNum;
                return;
            }
            else if(CurNum==tem_TalNum)
              return;
        }        
    }
    ///现在解析指令CmdBuf保存一整条指令
    switch(CmdBuf[3])
    {
            case 0x81:		//读寄存器指令
                
                break;
            case 0x83:		//读变量存储器指令								接收，串口屏发送的   变量存储器读写指令（0x82、0x83）是双字节，地址范围为0x0000~0xffff。
                
                break;
            default:			//命令无效,删除
                    break;
    }
    return;
}
//***********************卖家例程（end）***********************************
//*************************************************************

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
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级0
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
void UpdateDisTemp(){
	//声明要用的变量
	float Temp;
	u16 dis;
	u8 low,high,i;
	u8 ARYlength;
	//获取温度值
	Temp = SHT2x_GetTempPoll();
//	printf("温度是：%.2f\n",Temp);	
	dis = (int)(Temp * 10);
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
void UpdateDisHumi(){
	//声明要用的变量
	float Humi;
	u16 dis;
	u8 low,high,i;
	u8 ARYlength;
	
	Humi = SHT2x_GetHumiPoll();
//	printf("湿度是：%.2f\n\n",Humi);	
	dis = (int)(Humi * 10);
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
void UpdateDisPres(){
	//声明要用的变量
	float Pres;
	int dis;
	u8 low,high,i;
	u8 ARYlength;
	
	//更换成获取AD的压力值
	Pres = GetPresAverage(ADC_Channel_1,10);
	dis = (int)Pres;//压力转换为整数,单位Kpa
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


//功能 ：检测串口屏是否有按键按下
//返回值：
//0：无按键按下
//其他：相应按键的键值
u8 ifButtonDown(){
	u16 t;
	u16 rx_len;
	u8 button1_cnt=0,button2_cnt=0,button3_cnt=0;//用来判定BUTTON BUF与哪个BUTTON的键值是一致的变量			
			//串口2测试  成功  2018-1-22
		if(USART2_RX_STA&0x8000)
		{
			
			rx_len=USART2_RX_STA&0x3fff;//得到此次接收到的数据长度
//			printf("\r\n您发送的消息为:\r\n\r\n");
			USART2_RX_STA=0;		//归零该STA寄存器要在for循环前面（未知为什么）
			
			//用串口调试助手调试时用
//						把BUF数组里的数据循环发送给串口，每次发送一个字节  Jokie
//			for(t=0;t<rx_len;t++)
//			{
//				USART_SendData(USART2, USART2_RX_BUF[t]);//向向串口2把接收的数据重新通过串口2发送
//				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//等待发送结束
//			}
			
//			思路：循环获取接收到的BUF内的数据，完成for循环（即获取完成后），将该数据验证是否为对应button键值
			
			//STEP1 把串口2接收BUF与各BUTTON键值比较
			button1_cnt=0,button2_cnt=0,button3_cnt=0;//归零Cnt
			for(t=0;t<rx_len;t++)
			{
				if(BUTTON1[t]==USART2_RX_BUF[t])	button1_cnt++;
				if(BUTTON2[t]==USART2_RX_BUF[t])	button2_cnt++;
				if(BUTTON3[t]==USART2_RX_BUF[t])	button3_cnt++;
			}
			//STEP2 判定相应按钮是否按下以做出对应的响应
			if(button1_cnt==rx_len){											//按键1
				if(GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_5))//如果原来为高电平，则设为低电平
					GPIO_ResetBits(GPIOB,GPIO_Pin_5);
				else																				//如果原来为低电平，则设为高电平	
					GPIO_SetBits(GPIOB,GPIO_Pin_5);
				return BUTTON1_NUM;
			} 
			else if(button2_cnt==rx_len){											//按键2
				if(GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_5))//如果原来为高电平，则设为低电平
					GPIO_ResetBits(GPIOB,GPIO_Pin_5);
				else																				//如果原来为低电平，则设为高电平	
					GPIO_SetBits(GPIOB,GPIO_Pin_5);
				return BUTTON2_NUM;
			}
			else if(button3_cnt==rx_len){											//按键3
				if(GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_5))//如果原来为高电平，则设为低电平
					GPIO_ResetBits(GPIOB,GPIO_Pin_5);
				else																				//如果原来为低电平，则设为高电平	
					GPIO_SetBits(GPIOB,GPIO_Pin_5);
				return BUTTON3_NUM;
			}
			else return 0;
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
		if((USART2_RX_STA&0x8000)==0)//接收未完成
		{
			USART2_RX_BUF[USART2_RX_STA&0X3FFF]=Res ;
			USART2_RX_STA++;
			if(USART2_RX_STA>(USART2_REC_LEN-1))USART2_RX_STA=0;
			USART2_RX_STA|=0x8000;		
		}			
   }
//----------------------------------------------------------------------------------------
//-----------------------------在串口中断中接收数据   BY Jokie(end)---------------------
//----------------------------------------------------------------------------------------
	 
	 
	 
	 
	 
	 

	 
//----------------------------------------------------------------------------------------
//-----------------------------卖家给的例程中断函数部分---------------------------
//******************************************************************************************
	 
	 
//	 		if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断(接收数据不管什么结尾)
//	{
//		 CommBuff[TalNum++]=USART_ReceiveData(USART2);//保存串口数据
//    if(TalNum==BUFFER_SIZE)
//      TalNum=0;   			
//   }
	
	 
	 
//******************************************************************************************
//-----------------------------卖家给的例程中断函数部分(end)---------------------
//----------------------------------------------------------------------------------------	 

}
