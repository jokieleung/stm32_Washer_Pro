#include "tim5.h"



//ͨ�ö�ʱ��5�жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��5!

//���ӣ�TIM5_Int_Init(4999,7199);//10Khz�ļ���Ƶ�ʣ�������5000Ϊ500ms  
//��ʱ�����ڼ��㹫ʽ��[(1+TIM_Prescaler )*(1+TIM_Period )]/72M = T (s)
//�趨��ʱ����Ϊ��1s  1.TIM_Prescaler = 9999 2.TIM_Period=7199

void TIM5_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE); //ʱ��ʹ��
	
	//��ʱ��TIM3��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM3�ж�,��������ж�

	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //��ʼ��NVIC�Ĵ���


	TIM_Cmd(TIM5, ENABLE);  //ʹ��TIMx					 
}
//��ʱ��5�жϷ������
//һЩ��Ҫ��ʱ�õ�ȫ�ֱ���
//ϴ���õ�count����
extern u8 Washer_Mode;//ϴ��ģʽ��־λ
extern u8 Dry_Mode;//����ģʽ��־λ
u16 waterin_count=0;//��ˮ����ֵ
u16 wash_count=0;//ϴ�¼���ֵ
u16 waterout_count=0;//��ˮ����ֵ
u16 dry_count = 0;//�Ѹ���ʱ�����ֵ
u16 work_count = 0;//�ѹ�������ʱ�����ֵ
extern u16 WATERIN_TIME; //��ˮʱ�����
extern int WASH_TIME; //ϴ��ʱ�����
extern u16 WATEROUT_TIME; //��ˮʱ�����

extern u8 washDrying_flag;
//΢������ʽ���������õı���
extern u8 microwave_mode;//΢������״̬��־λ
u8 microwave_cnt=0;
void TIM5_IRQHandler(void)   //TIM5�ж�
{
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)  //���TIM5�����жϷ������
		{
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update  );  //���TIMx�����жϱ�־ 
			

			//****************���ϴ��ģʽ��*************************
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
			
			//****************�������ģʽ��*************************
			if(Dry_Mode)
			{
				if(washDrying_flag)
					work_count++;
				dry_count++;
			}
			//****************���΢�����ȴ�*************************
//			if(microwave_mode)
//			{
//				microwave_cnt++;
//				
//			}
								
		}
}












