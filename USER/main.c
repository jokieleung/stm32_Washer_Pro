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
//串口1
extern u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
extern u16 USART_RX_STA;
//串口2
extern u8 USART2_RX_BUF[USART2_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
extern u16 USART2_RX_STA; 
 
extern u16 WASH_TIME;
//按键值全局变量
extern u8 KeyDownNum;
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
	 TIM4_Int_Init(99,7199);//计数周期为10ms 用于按键检测
	 
	 while(1)
	{	    	 
		UpdateDisPres();//更新屏幕压力显示值
		UpdateDisTemp();//更新屏幕温度显示值
		UpdateDisHumi();//更新屏幕湿度显示值
		UpdateRemainingWashTim();//更新屏幕剩余洗衣时间显示值
		UpdateUsedDryTim();//更新屏幕干衣时间显示值
		UpdateUsedWholeTim();//更新屏幕洗烘时间显示值
	}
}
 










 
