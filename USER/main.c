#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
#include "tim4.h"

#include "stm32f10x_conf.h"
#include "adc.h"
#include "USART_TO_LCD.H"
#include "washer.h"
#include "dry.h"
//����1
extern u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
extern u16 USART_RX_STA;
//����2
extern u8 USART2_RX_BUF[USART2_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
extern u16 USART2_RX_STA; 
 
extern u16 WASH_TIME;
//����ֵȫ�ֱ���
extern u8 KeyDownNum;
 int main(void)
 {	 
	 delay_init();	    	 //��ʱ������ʼ��	  
	 NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	 Adc_Init();
	 TIM3_Int_Init(9999,7199);//��������Ϊ1s   �˺��޸�Ϊ��ʱϴ��ʱ���Լ����ʱ����
	 
	 SHT2x_Init();       //SHT2x��ʼ��
	 uart_init(115200);	 	//����1��ʼ��Ϊ115200   
	 USART2_Init(115200);	//����2��ʼ��Ϊ115200   
	 
	 RELAY_Init();//�̵�����ʼ������ʼ��Ĭ������Ϊ�ߵ�ƽ���̵����رգ�
	 TIM4_Int_Init(99,7199);//��������Ϊ10ms ���ڰ������
	 
	 while(1)
	{	    	 
		UpdateDisPres();//������Ļѹ����ʾֵ
		UpdateDisTemp();//������Ļ�¶���ʾֵ
		UpdateDisHumi();//������Ļʪ����ʾֵ
		UpdateRemainingWashTim();//������Ļʣ��ϴ��ʱ����ʾֵ
		UpdateUsedDryTim();//������Ļ����ʱ����ʾֵ
		UpdateUsedWholeTim();//������Ļϴ��ʱ����ʾֵ
	}
}
 










 
