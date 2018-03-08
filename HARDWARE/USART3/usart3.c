#include "usart3.h"
#include "gizwits_product.h"
 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//����3��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/3/29
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	   

//����3�жϷ�����
void USART3_IRQHandler(void)
{
	u8 res;	      
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)//���յ�����
	{	 
		res =USART_ReceiveData(USART3);		 
		gizPutData(&res, 1);//����д�뵽������
	}  				 											 
}   

//��ʼ��IO ����3
//pclk1:PCLK1ʱ��Ƶ��(Mhz)
//bound:������	  
void usart3_init(u32 bound)
{  

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	                       //GPIOBʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);                          //����3ʱ��ʹ��

 	USART_DeInit(USART3);  //��λ����3
	//USART3_TX   PB10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;                                     //PB10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	                               //�����������
    GPIO_Init(GPIOB, &GPIO_InitStructure);                                         //��ʼ��PB10
   
    //USART3_RX	  PB11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;                          //��������
    GPIO_Init(GPIOB, &GPIO_InitStructure);                                         //��ʼ��PB11
	
	USART_InitStructure.USART_BaudRate = bound;                                    //������һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;                    //�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;                         //һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;                            //����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	               //�շ�ģʽ
  
	USART_Init(USART3, &USART_InitStructure);       //��ʼ������3
  

	USART_Cmd(USART3, ENABLE);                      //ʹ�ܴ��� 
	
	//ʹ�ܽ����ж�
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);  //�����ж�   
	
	//�����ж����ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
}
