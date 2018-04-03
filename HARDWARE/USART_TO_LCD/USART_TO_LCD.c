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
//*******************button*********************************************
//������ֵ���顤
u8 WASH_BT[] = {0XA5,0X5A,0X06,0X83,0X00,0X00,0X01,0X01,0X11};
u8 DRY_BT[] = {0XA5,0X5A,0X06,0X83,0X00,0X00,0X01,0X02,0X22};
u8 WSH_D_B[] = {0XA5,0X5A,0X06,0X83,0X00,0X00,0X01,0X03,0X33};
//��ʼϴ�¡����¡�ϴ��
u8 START_W[] = {0XA5,0X5A,0X06,0X83,0X00,0X00,0X01,0X01,0X12};
u8 START_D[] = {0XA5,0X5A,0X06,0X83,0X00,0X00,0X01,0X02,0X21};
u8 START_WD[] = {0XA5,0X5A,0X06,0X83,0X00,0X00,0X01,0X03,0X32};
//����
u8 END[] = {0XA5,0X5A,0X06,0X83,0X00,0X00,0X01,0X04,0X44};
//�Ӽ�ϴ��ʱ��
u8 WSH_TIM[] = {0XA5,0X5A,0X06,0X83,0X00,0X50,0X01};
//��\��ˮʱ��
u8 WATER_IN_TIME[] = {0XA5,0X5A,0X06,0X83,0X00,0X54,0X01};
u8 WATER_OUT_TIME[] = {0XA5,0X5A,0X06,0X83,0X00,0X56,0X01};
//*******************button(end)**************************************

//��ʼ��������ֵ�����õ�����
u8 PRE_ARY[] = {0XA5,0X5A,0X05,0X82,0X00,0X20,0X00,0X00};//ѹ��ֵ��ʾ���� 
u8 TMP_ARY[] = {0XA5,0X5A,0X05,0X82,0X00,0X30,0X00,0X00};//�¶�ֵ��ʾ���� 
u8 HUM_ARY[] = {0XA5,0X5A,0X05,0X82,0X00,0X40,0X00,0X00};//ʪ��ֵ��ʾ���� 
//ʣ��ʱ����ʾ����
u8 WSH_ARY_M[] = {0XA5,0X5A,0X05,0X82,0X00,0X50,0X00,0X00};//ϴ��ʣ��ʱ����ʾ���� 
u8 WSH_ARY_S[] = {0XA5,0X5A,0X05,0X82,0X00,0X51,0X00,0X00};//ϴ��ʣ��ʱ����ʾ���� 
//����ˮʱ����ʾ����
//u8 WSH_IN_M[] = {0XA5,0X5A,0X05,0X82,0X00,0X53,0X00,0X00};//ϴ��ʣ��ʱ����ʾ���� 
u8 WSH_IN_S[] = {0XA5,0X5A,0X05,0X82,0X00,0X54,0X00,0X00};//ϴ��ʣ��ʱ����ʾ���� 
//u8 WSH_OUT_M[] = {0XA5,0X5A,0X05,0X82,0X00,0X55,0X00,0X00};//ϴ��ʣ��ʱ����ʾ���� 
u8 WSH_OUT_S[] = {0XA5,0X5A,0X05,0X82,0X00,0X56,0X00,0X00};//ϴ��ʣ��ʱ����ʾ���� 
//����ʱ����ʾ����
u8 DRY_ARY_M[] = {0XA5,0X5A,0X05,0X82,0X00,0X60,0X00,0X00};//��������ʱ����ʾ���� 
u8 DRY_ARY_S[] = {0XA5,0X5A,0X05,0X82,0X00,0X61,0X00,0X00};//��������ʱ����ʾ���� 
u8 WLE_ARY_M[] = {0XA5,0X5A,0X05,0X82,0X00,0X70,0X00,0X00};//ϴ��������ʱ����ʾ���� 
u8 WLE_ARY_S[] = {0XA5,0X5A,0X05,0X82,0X00,0X71,0X00,0X00};//ϴ��������ʱ����ʾ���� 
//ϴ��״̬��־λ
extern u8 washing_flag;
//����ֵ������ֵ��ȫ�ֱ��� 
float Temp,Humi,Pres;
int RemainWashTime,DryTime,WashDryTime;

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
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//�����ȼ�0
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
void UpdateDisTemp(float *temp){
	//����Ҫ�õı���
	u16 dis;
	u8 low,high,i;
	u8 ARYlength;
	//��ȡ�¶�ֵ
	//*******************
//	printf("�¶��ǣ�%.2f\n",Temp);	
	dis = (int)((*temp) * 10);
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
void UpdateDisHumi(float *Humi){
	//����Ҫ�õı���
	u16 dis;
	u8 low,high,i;
	u8 ARYlength;
//	printf("ʪ���ǣ�%.2f\n\n",Humi);	
	dis = (int)((*Humi) * 10);
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
void UpdateDisPres(float *Pres){
	//����Ҫ�õı���
	int dis;
	u8 low,high,i;
	u8 ARYlength;
	
	//�����ɻ�ȡAD��ѹ��ֵ
	
	dis = (int)(*Pres);//ѹ��ת��Ϊ����,��λKpa
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
//����ʣ��ϴ��ʱ��   ת����hh:mm��ʽ
extern u16 waterin_count;//��ˮ����ֵ
extern u16 wash_count;//ϴ�¼���ֵ
extern u16 waterout_count;//��ˮ����ֵ
extern u16 WATERIN_TIME; //��ˮʱ�����
extern int WASH_TIME; //ϴ��ʱ�����
extern u16 WATEROUT_TIME; //��ˮʱ�����

void UpdateRemainingWashTim(int *remainSec){
	int Minu,Sec;
	u8 low,high,i;
	u8 ARYlength;
	//�ܵ�ʣ��ʱ�䣬��λs
	
//	printf("remainSec : %d \r\n" ,remainSec);
	Minu = ((*remainSec)/60); //��������
	Sec = ((*remainSec)% 60); //�룺��ȡ�ࣩ
	low = Minu & 0xff;
	high = (Minu>>8) & 0xff;
	WSH_ARY_M[7] = low;
	WSH_ARY_M[6] = high;
	low = Sec & 0xff;
	high = (Sec>>8) & 0xff;
	WSH_ARY_S[7] = low;
	WSH_ARY_S[6] = high;
//		�ѱ��������������ѭ�����͸����ڣ�ÿ�η���һ���ֽ�  Jokie
	ARYlength = sizeof(WSH_ARY_M)/sizeof(WSH_ARY_M[0]); //�������鳤��
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, WSH_ARY_M[i]);//�򴮿�2�ѽ��յ���������ͨ������2����
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
	}
	delay_ms(1);
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, WSH_ARY_S[i]);//�򴮿�2�ѽ��յ���������ͨ������2����
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
	}
}
//���½���ˮʱ��
void UpdateWashInAndOut(u16 *InSec,u16 *OutSec){
	int Sec;
	u8 low,high,i;
	u8 ARYlength;
	
	Sec = *InSec; //��
	low = Sec & 0xff;
	high = (Sec>>8) & 0xff;
	WSH_IN_S[7] = low;
	WSH_IN_S[6] = high;
	ARYlength = sizeof(WSH_IN_S)/sizeof(WSH_IN_S[0]); //�������鳤��
//		�ѱ��������������ѭ�����͸����ڣ�ÿ�η���һ���ֽ�  Jokie
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, WSH_IN_S[i]);//�򴮿�2�ѽ��յ���������ͨ������2����
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
	}
	
	Sec = *OutSec; //�룺��ȡ�ࣩ
	low = Sec & 0xff;
	high = (Sec>>8) & 0xff;
	WSH_OUT_S[7] = low;
	WSH_OUT_S[6] = high;
//		�ѱ��������������ѭ�����͸����ڣ�ÿ�η���һ���ֽ�  Jokie
	ARYlength = sizeof(WSH_OUT_S)/sizeof(WSH_OUT_S[0]); //�������鳤��
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, WSH_OUT_S[i]);//�򴮿�2�ѽ��յ���������ͨ������2����
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
	}
	
}

extern u16 dry_count;
//�����Ѿ�����ʱ��
void UpdateUsedDryTim(int *UsedSec){
	int Minu,Sec;
	u8 low,high,i;
	u8 ARYlength;
	//�ܵ�ʣ��ʱ�䣬��λs
	
//	printf("remainSec : %d \r\n" ,remainSec);
	Minu = ((*UsedSec)/60); //��������
	Sec = ((*UsedSec)% 60); //�룺��ȡ�ࣩ
	low = Minu & 0xff;
	high = (Minu>>8) & 0xff;
	DRY_ARY_M[7] = low;
	DRY_ARY_M[6] = high;
	low = Sec & 0xff;
	high = (Sec>>8) & 0xff;
	DRY_ARY_S[7] = low;
	DRY_ARY_S[6] = high;
//		�ѱ��������������ѭ�����͸����ڣ�ÿ�η���һ���ֽ�  Jokie
	ARYlength = sizeof(DRY_ARY_M)/sizeof(DRY_ARY_M[0]); //�������鳤��
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, DRY_ARY_M[i]);//�򴮿�2�ѽ��յ���������ͨ������2����
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
	}
	delay_ms(1);
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, DRY_ARY_S[i]);//�򴮿�2�ѽ��յ���������ͨ������2����
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
	}
}

extern u16 work_count;
void UpdateUsedWholeTim(int *UsedSec){
	int Minu,Sec;
	u8 low,high,i;
	u8 ARYlength;
	//�ܵ�ʣ��ʱ�䣬��λs
	
//	printf("remainSec : %d \r\n" ,remainSec);
	Minu = ((*UsedSec)/60); //��������
	Sec = ((*UsedSec)% 60); //�룺��ȡ�ࣩ
	low = Minu & 0xff;
	high = (Minu>>8) & 0xff;
	WLE_ARY_M[7] = low;
	WLE_ARY_M[6] = high;
	low = Sec & 0xff;
	high = (Sec>>8) & 0xff;
	WLE_ARY_S[7] = low;
	WLE_ARY_S[6] = high;
//		�ѱ��������������ѭ�����͸����ڣ�ÿ�η���һ���ֽ�  Jokie
	ARYlength = sizeof(WLE_ARY_M)/sizeof(WLE_ARY_M[0]); //�������鳤��
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, WLE_ARY_M[i]);//�򴮿�2�ѽ��յ���������ͨ������2����
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
	}
	delay_ms(1);
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, WLE_ARY_S[i]);//�򴮿�2�ѽ��յ���������ͨ������2����
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
	}
}

//��ת��ϴ����ɵĽ���
void JumpToFinishedUI(){
	u8 ARYlength,i;
	u8 FNSH_PAGE[] = {0XA5,0X5A,0X04,0X80,0X03,0X00,0X07};//��תϴ�����ҳ��ָ�� A5 5A 04 80 03 00 07 
	
	ARYlength = sizeof(FNSH_PAGE)/sizeof(FNSH_PAGE[0]); //�������鳤��
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, FNSH_PAGE[i]);//�򴮿�2�ѽ��յ���������ͨ������2����
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
	}
}
//��ת���ض��Ľ���
//���룺7λ��תָ������  eg. FNSH_PAGE[] = {0XA5,0X5A,0X04,0X80,0X03,0X00,0X07};//��ת��7��ҳ��ָ�� A5 5A 04 80 03 00 07 
void JumpToUI(u8 PAGE_ARY[]){
	u8 ARYlength,i;
	
	ARYlength = 7;//sizeof(PAGE_ARY)/sizeof(PAGE_ARY[0]); //�������鳤��
	
	for(i=0;i<ARYlength;i++)
	{
				USART_SendData(USART2, PAGE_ARY[i]);//�򴮿�2�ѽ��յ���������ͨ������2����
				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
	}
}

//���ܣ���ȡ������Ҫ��ʾ�ı���ֵ������ֵ����Ӧ�Ĺ̶�ָ�루��ַ����
void GetEveryDisPara(){
	//��ȡ�¶�ֵ
	Temp = SHT2x_GetTempPoll();
//	printf("Temp:%f\n",Temp);
	//��ȡʪ��ֵ
	Humi = SHT2x_GetHumiPoll();
//	printf("Humi %f\n",Humi);
	//��ȡѹ��ֵ
	Pres = GetPresAverage(ADC_Channel_7,10);
//	printf("Pres %f \n",Pres);
	//��ȡʣ��ϴ��ʱ��
	//if(washing_flag) //�������ϴ�¹���״̬����RemainWashTimeΪ�ܵĵ���ʱ
		RemainWashTime = (WATERIN_TIME+WASH_TIME+WATEROUT_TIME)-waterin_count-wash_count-waterout_count;
	//else 	//�����������ʱ��״̬����RemainWashTimeΪϴ��ʱ��
	//	RemainWashTime = WASH_TIME;
//	printf("RemainWashTime %d \n",RemainWashTime);
	//��ȡ����ʱ��
	DryTime = dry_count;
//	printf("DryTime %d \n",DryTime);
	//��ȡϴ��ʱ��
	WashDryTime = work_count;
//	printf("WashDryTime %d \n",WashDryTime);
}
//@���ܣ���ȡ������Ҫ��ʾ�ı���ֵ������ֵ����Ӧ�Ĺ̶�ָ�루��ַ����
//@para: float *Temp,float *Humi,float *Pres,int *RemainWashTime,int *DryTime,int *WashDryTime

void UpdateEveryDisPara(float *Temp,float *Humi,float *Pres,int *RemainWashTime,int *DryTime,int *WashDryTime){
	//�����¶�ֵ
	UpdateDisTemp(Temp);
	//����ʪ��ֵ
	UpdateDisHumi(Humi);
	//����ѹ��ֵ
	UpdateDisPres(Pres);
	//����ʣ��ϴ��ʱ��
	if(washing_flag)
		UpdateRemainingWashTim(RemainWashTime);
	//���¸���ʱ��
	UpdateUsedDryTim(DryTime);
	//����ϴ��ʱ��
	UpdateUsedWholeTim(WashDryTime);
}






//���� ����⴮�����Ƿ��а�������
//����ֵ��
//0���ް�������
//��������Ӧ�����ļ�ֵ
u8 ifButtonDown(){
	u16 t;
	u16 rx_len;
	u8 button1_cnt=0,button2_cnt=0,button3_cnt=0,button4_cnt=0,button5_cnt=0,button6_cnt=0,button7_cnt=0,button8_cnt=0,button9_cnt=0,button10_cnt=0;//�����ж�BUTTON BUF���ĸ�BUTTON�ļ�ֵ��һ�µı���			
			//����2����  �ɹ�  2018-1-22

			rx_len=USART2_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
			if(rx_len!=0)
			{
				USART2_RX_STA=0;		//�����STA�Ĵ���Ҫ��forѭ��ǰ�棨δ֪Ϊʲô��
				
	//			˼·��ѭ����ȡ���յ���BUF�ڵ����ݣ����forѭ��������ȡ��ɺ󣩣�����������֤�Ƿ�Ϊ��Ӧbutton��ֵ
				
				//STEP1 �Ѵ���2����BUF���BUTTON��ֵ�Ƚ�
				//����Cnt
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
					if(WSH_TIM[t]==USART2_RX_BUF[t])	button8_cnt++;  //�Ӽ�ϴ��ʱ�� ����
					//����ˮʱ������
					if(WATER_IN_TIME[t]==USART2_RX_BUF[t])	button9_cnt++;
					if(WATER_OUT_TIME[t]==USART2_RX_BUF[t])	button10_cnt++;
					//ͨ������1�۲� debug��  ���Գɹ� 2018.03.02 13��44
	//				USART_SendData(USART1, USART2_RX_BUF[t]);//�ѽ��յĴ���2��������ͨ������1����
	//				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
				}
				
				//STEP2 �ж���Ӧ��ť�Ƿ�����������Ӧ����Ӧ
					if(button1_cnt==rx_len){											//����1
						return BUTTON1_NUM;
					} 
					else if(button2_cnt==rx_len){											//����2
						return BUTTON2_NUM;
					}
					else if(button3_cnt==rx_len){											//����3
						return BUTTON3_NUM;
					}
					else if(button4_cnt==rx_len){											//����4
						return BUTTON4_NUM;
					}
					else if(button5_cnt==rx_len){											//����5
						return BUTTON5_NUM;
					}
					else if(button6_cnt==rx_len){											//����6
						return BUTTON6_NUM;
					}
					else if(button7_cnt==rx_len){											//����7
						return BUTTON7_NUM;
					}
					else if((button8_cnt+2)==rx_len){											//����8  ��ȡ�Ӽ�ϴ��ʱ�������ֵ
//						printf("button8_cnt: %d\r\n",button8_cnt);
						WASH_TIME=((USART2_RX_BUF[7]<<2)+USART2_RX_BUF[8])*60;//�Ѹ�ֵ���������֤ʵʱ��
						UpdateRemainingWashTim(&WASH_TIME);
						return BUTTON8_NUM;
					}
					else if((button9_cnt+2)==rx_len){											//����9	��ȡ�Ӽ���ˮʱ�������ֵ
						WATERIN_TIME=((USART2_RX_BUF[7]<<2)+USART2_RX_BUF[8]);//�Ѹ�ֵ���������֤ʵʱ��
						UpdateWashInAndOut(&WATERIN_TIME,&WATEROUT_TIME);
						return BUTTON9_NUM;
					}
					else if((button10_cnt+2)==rx_len){											//����10	��ȡ�Ӽ���ˮʱ�������ֵ
						WATEROUT_TIME=((USART2_RX_BUF[7]<<2)+USART2_RX_BUF[8]);//�Ѹ�ֵ���������֤ʵʱ��
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
			USART2_RX_BUF[USART2_RX_STA&0X3FFF]=Res ;
			USART2_RX_STA++;
			if(USART2_RX_STA>(USART2_REC_LEN-1))USART2_RX_STA=0;
  }
//----------------------------------------------------------------------------------------
//-----------------------------�ڴ����ж��н�������   BY Jokie(end)---------------------

}



