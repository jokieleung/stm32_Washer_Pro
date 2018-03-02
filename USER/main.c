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
//串口1
extern u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
extern u16 USART_RX_STA;
//串口2
extern u8 USART2_RX_BUF[USART2_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
extern u16 USART2_RX_STA; 
 

 int main(void)
 {	 
	 delay_init();	    	 //延时函数初始化	  
	 NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	 Adc_Init();
	 TIM3_Int_Init(9999,7199);//计数周期为1s   此后修改为定时洗衣时间以及烘干时间用
	 SHT2x_Init();       //SHT2x初始化
	 uart_init(115200);	 	//串口1初始化为115200   
	 USART2_Init(115200);	//串口2初始化为115200   
	 
	 RELAY_Init();//继电器初始化，初始化默认设置为高电平（继电器关闭）
	 
//	 printf("uart_init OK\r\n"); 
	 while(1)
	{	    	    
		
		switch(ifButtonDown()){
			case BUTTON1_NUM:	//洗衣按键
				wash_func();	
				break;
			case BUTTON2_NUM:	//烘衣按键
				break;
			case BUTTON3_NUM:	//洗烘按键
				break;
			case BUTTON4_NUM:	//开始按键
				break;
			case BUTTON5_NUM:	//暂停按键
				break;
			
		}		

		UpdateDisPres();
		UpdateDisTemp();
		UpdateDisHumi();
		
		
	}
}
 










 
