#include "tim4.h"

/* �������û�����ǰ�豸״̬�ṹ��*/
extern dataPoint_t currentDataPoint;

//ͨ�ö�ʱ��4�жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��4!

//���ӣ�TIM4_Int_Init(4999,7199);//10Khz�ļ���Ƶ�ʣ�������5000Ϊ500ms  
//��ʱ�����ڼ��㹫ʽ��[(1+TIM_Prescaler )*(1+TIM_Period )]/72M = T (s)
//�趨��ʱ����Ϊ��1s  1.TIM_Prescaler = 9999     2.TIM_Period=7199

void TIM4_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //ʱ��ʹ��
	
	//��ʱ��TIM4��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM3�ж�,��������ж�

	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //��ռ���ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  //�����ȼ�1��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //��ʼ��NVIC�Ĵ���


	TIM_Cmd(TIM4, ENABLE);  //ʹ��TIMx					 
}
//��ʱ��4�жϷ������
u8 KeyDownNum=0;//����ֵȫ�ֱ���
extern u16 WASH_TIME;
//UI��תָ�� ����
u8 START_W_ARY[] = {0XA5,0X5A,0X04,0X80,0X03,0X00,0X06};//����ϴ�½���
u8 START_D_ARY[] = {0XA5,0X5A,0X04,0X80,0X03,0X00,0X0A};//���ں�ɽ���
u8 START_WD_ARY[] = {0XA5,0X5A,0X04,0X80,0X03,0X00,0X09};//���ں�ϴ����


//10ms��ʱ��
u8 TenMsCnt = 0;
//200ms��ʱ��
u8 Cnt200Ms = 0;
//ȫ��ϴ�»�����״̬λ
extern u8 washing_flag,drying_flag,washDrying_flag;

//����ֵ������ֵ��ȫ�ֱ��� 
extern float Temp,Humi,Pres;
extern int RemainWashTime,DryTime,WashDryTime;

//ϴ��ʱ�����ֵ
extern u16 work_count;

//��������
void userHandle(void);

void TIM4_IRQHandler(void)   //TIM3�ж�
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
		{
			TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );  //���TIMx�����жϱ�־ 
			
			
			
			//**************ÿ80ms����һ�λ�����Э��******************************
			TenMsCnt++;
			if(TenMsCnt >=8){
				TenMsCnt = 0;
				userHandle();//�û��ɼ��������߼���
				gizwitsHandle((dataPoint_t *)&currentDataPoint);//Э�鴦�������߼���
			}
			//**************ÿ200ms��ȡ������һ�δ�������ֵ******************************
			Cnt200Ms++;
			if(Cnt200Ms >=20){
				Cnt200Ms = 0;
				GetEveryDisPara();
				UpdateEveryDisPara(&Temp,&Humi,&Pres,&RemainWashTime,&DryTime,&WashDryTime);
			}
			
			//**************ÿ10ms���һ���������������źŶ�******************************
			VoiceDetect();
			
			//*****���������ƺ�������ƣ���switch�߼��Ļ�if 2018.3.6*********************
			//*****ÿ10ms���һ�δ���������**********************************************
			//@20180307 ����Ϊ��ȡ��ֵ������flag,��main.c��while(1)���������������жϳ�ͻ
			KeyDownNum = ifButtonDown();
			
			if (KeyDownNum == BUTTON1_NUM) {	//ϴ�½��水�� 
				Rst_Wash();//����ϴ�²���
				UpdateRemainingWashTim(&RemainWashTime);//����һ����Ļʣ��ϴ��ʱ��ֵ
				goto tim4_ir_end;
			}
			if (KeyDownNum==BUTTON2_NUM)  {	//���½��水�� 
				Rst_Dry();//����ϴ�²���
			}
			if (KeyDownNum==BUTTON3_NUM)  {	//ϴ����水��
				Rst_Wash();//����ϴ�²���
				Rst_Dry();//���ø��²���
				work_count = 0;
				goto tim4_ir_end;
			}
			if(KeyDownNum==BUTTON4_NUM){	//��ʼϴ�°���
				washing_flag = 1;
				goto tim4_ir_end;
			}
			if (KeyDownNum==BUTTON5_NUM) {	//��ʼ���°���
				drying_flag = 1;
				goto tim4_ir_end;
			}
			if (KeyDownNum==BUTTON6_NUM) {	//��ʼϴ�水��
				washDrying_flag = 1;
				goto tim4_ir_end;
			}
			if (KeyDownNum==BUTTON7_NUM)  {	//��������  ֹͣ����ϴ�����  
				washing_flag = 0;
				drying_flag = 0;
				washDrying_flag = 0;
				goto tim4_ir_end;
			}
			if (KeyDownNum==BUTTON8_NUM)  {	//���ڼӼ�ϴ��ʱ��
//				WASH_TIME = ((USART2_RX_BUF[7]<<2)+USART2_RX_BUF[8])*60;
				goto tim4_ir_end;
			}
			if (KeyDownNum==BUTTON9_NUM)  {	//��ʱ��Ϊ����ﵽ������������ѭ���õİ���
				goto tim4_ir_end;
			}
			
			//**************************END  2018.3.6************************************
			
			tim4_ir_end:;

		}
}












