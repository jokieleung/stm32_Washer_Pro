#include "USART_TO_LCD.H"
#include "stm32f10x_usart.h"

//����2ʹ������: tx:PA.2 rx:PA.3

//#if EN_USART2_RX   //���ʹ���˽���
//����2�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART2_RX_BUF[USART2_REC_LEN];     //���ջ���,���USART2_REC_LEN���ֽ�.
//����״̬(��ΪҪ���ǽ������ݱ�����0x0d 0x0a ��β��)
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
	 
//����״̬(����Ҫ��������ݱ�����0x0d 0x0a ��β��)
//bit15��	������ɱ�־
//bit14~0��	���յ�����Ч�ֽ���Ŀ
u16 USART2_RX_STA=0;       //����״̬���	  

//������ֵ���顤
u8 BUTTON1[] = {0XA5,0X5A,0X06,0X83,0X00,0X00,0X01,0X01,0X11};
u8 BUTTON2[] = {0XA5,0X5A,0X06,0X83,0X00,0X00,0X01,0X02,0X22};
u8 BUTTON3[] = {0XA5,0X5A,0X06,0X83,0X00,0X00,0X01,0X03,0X33};

//��ʼ��������ֵ�����õ�����
u8 PRE_ARY[] = {0XA5,0X5A,0X05,0X82,0X00,0X20,0X00,0X00};//ѹ��ֵ��ʾ���� 
u8 TMP_ARY[] = {0XA5,0X5A,0X05,0X82,0X00,0X30,0X00,0X00};//�¶�ֵ��ʾ���� 
u8 HUM_ARY[] = {0XA5,0X5A,0X05,0X82,0X00,0X40,0X00,0X00};//ʪ��ֵ��ʾ���� 

//***********************��������*******************************
//*************************************************************
u16 StartNum=0,TalNum=0;
#define BUFFER_SIZE 2048//ָ�������С���û��ɸ����Լ���Ƭ���������޸�
u8 CommBuff[BUFFER_SIZE];//����ָ�����
#define USER_R3 0xA5//֡ͷ
#define USER_RA 0x5A//֡ͷ

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
            if(i==len+3)//У����ɣ�������һ��ָ��ĳ��ȣ���break �������ڽ��������н���CmdBuf
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
    ///���ڽ���ָ��CmdBuf����һ����ָ��
    switch(CmdBuf[3])
    {
            case 0x81:		//���Ĵ���ָ��
                
                break;
            case 0x83:		//�������洢��ָ��								���գ����������͵�   �����洢����дָ�0x82��0x83����˫�ֽڣ���ַ��ΧΪ0x0000~0xffff��
                
                break;
            default:			//������Ч,ɾ��
                    break;
    }
    return;
}
//***********************�������̣�end��***********************************
//*************************************************************

void USART2_Init(u32 bound)
{
   //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 //ʹ��USART2��GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
	//USART2_TX   GPIOA.2
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA2  TXD
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  	GPIO_Init(GPIOA, &GPIO_InitStructure);
   
	//USART2_RX	  GPIOA.3��ʼ��
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3  RXD
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA3  
  
	//Usart2 NVIC ����		��ʱ��ʱ���������ȼ�Ϊ���
  	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

   	//USART2 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	
  	USART_Init(USART2, &USART_InitStructure); //��ʼ������2

  	USART_ITConfig(USART2, USART_IT_RXNE|USART_IT_IDLE, ENABLE);//�������ڽ��ܺ����߿����ж�
  	
	USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ���2 	
}

//���� ��������ʾ���¶�ֵ
void UpdateDisTemp(){
	//����Ҫ�õı���
	float Temp;
	u16 dis;
	u8 low,high,i;
	u8 ARYlength;
	//��ȡ�¶�ֵ
	Temp = SHT2x_GetTempPoll();
//	printf("�¶��ǣ�%.2f\n",Temp);	
	dis = (int)(Temp * 10);
	low = dis & 0xff;
	high = (dis>>8) & 0xff;
	TMP_ARY[7] = low;
	TMP_ARY[6] = high;
//		�ѱ��������������ѭ�����͸����ڣ�ÿ�η���һ���ֽ�  Jokie
	ARYlength = sizeof(TMP_ARY)/sizeof(TMP_ARY[0]); //�������鳤��
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, TMP_ARY[i]);//���򴮿�2�ѽ��յ���������ͨ������2����
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
	}
}
//���� ��������ʾ��ʪ��ֵ
void UpdateDisHumi(){
	//����Ҫ�õı���
	float Humi;
	u16 dis;
	u8 low,high,i;
	u8 ARYlength;
	
	Humi = SHT2x_GetHumiPoll();
//	printf("ʪ���ǣ�%.2f\n\n",Humi);	
	dis = (int)(Humi * 10);
	low = dis & 0xff;
	high = (dis>>8) & 0xff;
	HUM_ARY[7] = low;
	HUM_ARY[6] = high;
//		�ѱ��������������ѭ�����͸����ڣ�ÿ�η���һ���ֽ�  Jokie
	ARYlength = sizeof(HUM_ARY)/sizeof(HUM_ARY[0]); //�������鳤��
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, HUM_ARY[i]);//���򴮿�2�ѽ��յ���������ͨ������2����
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
	}
}

//���� ��������ʾ��ѹ��ֵ
void UpdateDisPres(){
	//����Ҫ�õı���
	float Pres;
	int dis;
	u8 low,high,i;
	u8 ARYlength;
	
	//�����ɻ�ȡAD��ѹ��ֵ
	Pres = GetPresAverage(ADC_Channel_1,10);
	dis = (int)Pres;//ѹ��ת��Ϊ����,��λKpa
	low = dis & 0xff;
	high = (dis>>8) & 0xff;
	PRE_ARY[7] = low;
	PRE_ARY[6] = high;
//		�ѱ��������������ѭ�����͸����ڣ�ÿ�η���һ���ֽ�  Jokie
	ARYlength = sizeof(PRE_ARY)/sizeof(PRE_ARY[0]); //�������鳤��
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, PRE_ARY[i]);//�򴮿�2�ѽ��յ���������ͨ������2����
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
	}
}


//���� ����⴮�����Ƿ��а�������
//����ֵ��
//0���ް�������
//��������Ӧ�����ļ�ֵ
u8 ifButtonDown(){
	u16 t;
	u16 rx_len;
	u8 button1_cnt=0,button2_cnt=0,button3_cnt=0;//�����ж�BUTTON BUF���ĸ�BUTTON�ļ�ֵ��һ�µı���			
			//����2����  �ɹ�  2018-1-22
		if(USART2_RX_STA&0x8000)
		{
			
			rx_len=USART2_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
//			printf("\r\n�����͵���ϢΪ:\r\n\r\n");
			USART2_RX_STA=0;		//�����STA�Ĵ���Ҫ��forѭ��ǰ�棨δ֪Ϊʲô��
			
			//�ô��ڵ������ֵ���ʱ��
//						��BUF�����������ѭ�����͸����ڣ�ÿ�η���һ���ֽ�  Jokie
//			for(t=0;t<rx_len;t++)
//			{
//				USART_SendData(USART2, USART2_RX_BUF[t]);//���򴮿�2�ѽ��յ���������ͨ������2����
//				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
//			}
			
//			˼·��ѭ����ȡ���յ���BUF�ڵ����ݣ����forѭ��������ȡ��ɺ󣩣�����������֤�Ƿ�Ϊ��Ӧbutton��ֵ
			
			//STEP1 �Ѵ���2����BUF���BUTTON��ֵ�Ƚ�
			button1_cnt=0,button2_cnt=0,button3_cnt=0;//����Cnt
			for(t=0;t<rx_len;t++)
			{
				if(BUTTON1[t]==USART2_RX_BUF[t])	button1_cnt++;
				if(BUTTON2[t]==USART2_RX_BUF[t])	button2_cnt++;
				if(BUTTON3[t]==USART2_RX_BUF[t])	button3_cnt++;
			}
			//STEP2 �ж���Ӧ��ť�Ƿ�����������Ӧ����Ӧ
			if(button1_cnt==rx_len){											//����1
				if(GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_5))//���ԭ��Ϊ�ߵ�ƽ������Ϊ�͵�ƽ
					GPIO_ResetBits(GPIOB,GPIO_Pin_5);
				else																				//���ԭ��Ϊ�͵�ƽ������Ϊ�ߵ�ƽ	
					GPIO_SetBits(GPIOB,GPIO_Pin_5);
				return BUTTON1_NUM;
			} 
			else if(button2_cnt==rx_len){											//����2
				if(GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_5))//���ԭ��Ϊ�ߵ�ƽ������Ϊ�͵�ƽ
					GPIO_ResetBits(GPIOB,GPIO_Pin_5);
				else																				//���ԭ��Ϊ�͵�ƽ������Ϊ�ߵ�ƽ	
					GPIO_SetBits(GPIOB,GPIO_Pin_5);
				return BUTTON2_NUM;
			}
			else if(button3_cnt==rx_len){											//����3
				if(GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_5))//���ԭ��Ϊ�ߵ�ƽ������Ϊ�͵�ƽ
					GPIO_ResetBits(GPIOB,GPIO_Pin_5);
				else																				//���ԭ��Ϊ�͵�ƽ������Ϊ�ߵ�ƽ	
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
//-----------------------------�ڴ����ж��н�������   BY Jokie---------------------------
//----------------------------------------------------------------------------------------
		u8 Res;
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
		//changed by jokie :�������ݲ���ʲô��β
		if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //�����ж�(�������ݲ���ʲô��β)
	{
		Res =USART_ReceiveData(USART2);	//��ȡ���յ�������
		if((USART2_RX_STA&0x8000)==0)//����δ���
		{
			USART2_RX_BUF[USART2_RX_STA&0X3FFF]=Res ;
			USART2_RX_STA++;
			if(USART2_RX_STA>(USART2_REC_LEN-1))USART2_RX_STA=0;
			USART2_RX_STA|=0x8000;		
		}			
   }
//----------------------------------------------------------------------------------------
//-----------------------------�ڴ����ж��н�������   BY Jokie(end)---------------------
//----------------------------------------------------------------------------------------
	 
	 
	 
	 
	 
	 

	 
//----------------------------------------------------------------------------------------
//-----------------------------���Ҹ��������жϺ�������---------------------------
//******************************************************************************************
	 
	 
//	 		if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //�����ж�(�������ݲ���ʲô��β)
//	{
//		 CommBuff[TalNum++]=USART_ReceiveData(USART2);//���洮������
//    if(TalNum==BUFFER_SIZE)
//      TalNum=0;   			
//   }
	
	 
	 
//******************************************************************************************
//-----------------------------���Ҹ��������жϺ�������(end)---------------------
//----------------------------------------------------------------------------------------	 

}
