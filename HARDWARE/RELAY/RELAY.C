#include "RELAY.H"

//��ʼ��PD0-->PD.7����ȥPD2�� �����.��ʹ��ʱ��		    
//RELAY_Init��ʼ��
void RELAY_Init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;
		
		
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	 //ʹ��PB�˿�ʱ��
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 |GPIO_Pin_3 |GPIO_Pin_4 |GPIO_Pin_5 |GPIO_Pin_6 |GPIO_Pin_7 ;	 //PD0-->PD.7 �˿����ã���ȥPD2��
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOD, &GPIO_InitStructure);					 //�����趨������ʼ��
	
	
	GPIO_SetBits(GPIOD,GPIO_Pin_0);						 //��ȫ���̵������������Ϊ�ߵ�ƽ
	GPIO_SetBits(GPIOD,GPIO_Pin_1);						 //��ȫ���̵������������Ϊ�ߵ�ƽ
	GPIO_SetBits(GPIOD,GPIO_Pin_3);						 //��ȫ���̵������������Ϊ�ߵ�ƽ
	GPIO_SetBits(GPIOD,GPIO_Pin_4);						 //��ȫ���̵������������Ϊ�ߵ�ƽ
	GPIO_SetBits(GPIOD,GPIO_Pin_5);						 //��ȫ���̵������������Ϊ�ߵ�ƽ
	GPIO_SetBits(GPIOD,GPIO_Pin_6);						 //��ȫ���̵������������Ϊ�ߵ�ƽ
	GPIO_SetBits(GPIOD,GPIO_Pin_7);						 //��ȫ���̵������������Ϊ�ߵ�ƽ
}



 