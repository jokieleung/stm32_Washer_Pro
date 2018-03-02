#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"

#include "stm32f10x_conf.h"
#include "adc.h"
#include "USART_TO_LCD.H"
#include "washer.h"
//����1
extern u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
extern u16 USART_RX_STA;
//����2
extern u8 USART2_RX_BUF[USART2_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
extern u16 USART2_RX_STA; 
 

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
	 
//	 printf("uart_init OK\r\n"); 
	 while(1)
	{	    	    
		
		switch(ifButtonDown()){
			case BUTTON1_NUM:	//ϴ�°���
				wash_func();	
				break;
			case BUTTON2_NUM:	//���°���
				break;
			case BUTTON3_NUM:	//ϴ�水��
				break;
			case BUTTON4_NUM:	//��ʼ����
				break;
			case BUTTON5_NUM:	//��ͣ����
				break;
			
		}		

		UpdateDisPres();
		UpdateDisTemp();
		UpdateDisHumi();
		
		
	}
}
 










 
