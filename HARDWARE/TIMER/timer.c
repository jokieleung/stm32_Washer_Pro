#include "timer.h"
#include "led.h"
#include "washer.h"
#include "USART_TO_LCD.H"

//ͨ�ö�ʱ��3�жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��3!

//���ӣ�TIM3_Int_Init(4999,7199);//10Khz�ļ���Ƶ�ʣ�������5000Ϊ500ms  
//��ʱ�����ڼ��㹫ʽ��[(1+TIM_Prescaler )*(1+TIM_Period )]/72M = T (s)
//�趨��ʱ����Ϊ��1s  1.TIM_Prescaler = 9999 2.TIM_Period=7199

void TIM3_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ��ʹ��
	
	//��ʱ��TIM3��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM3�ж�,��������ж�

	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //��ʼ��NVIC�Ĵ���


	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx					 
}
//��ʱ��3�жϷ������
//һЩ��Ҫ��ʱ�õ�ȫ�ֱ���
//ϴ���õ�count����
extern u8 Washer_Mode;//ϴ��ģʽ��־λ
u16 waterin_count;//��ˮ����ֵ
u16 wash_count;//ϴ�¼���ֵ
u16 waterout_count;//��ˮ����ֵ

void TIM3_IRQHandler(void)   //TIM3�ж�
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
		{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //���TIMx�����жϱ�־ 
			
			//���¶�Ӧ����ֵ
//		UpdateDisPres();
//		UpdateDisTemp();
//		UpdateDisHumi();
			
			//****************���ϴ��ģʽ��*************************
			if(Washer_Mode)
			{
				if(waterin_count < WATERIN_TIME )
					waterin_count++;
				
				else if((wash_count < WASH_TIME )/*&&(waterin_count >= WATERIN_TIME)*/)
					wash_count++;
				
				else if((waterout_count < WATEROUT_TIME)/*&&(wash_count >= WASH_TIME )&&(waterin_count >= WATERIN_TIME)*/)
					waterout_count++;
			}
			
			//*********************************************************
		}
}












