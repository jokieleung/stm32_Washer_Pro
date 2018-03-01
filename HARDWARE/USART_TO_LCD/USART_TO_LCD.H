#ifndef LCD__USART_H
#define LCD__USART_H
#include "stdio.h"	
#include "sys.h" 
#include "adc.h"
#include "SHT2x.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//串口1初始化		   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/8/18
//版本：V1.5
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved
//********************************************************************************
//V1.3修改说明 
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug
//V1.4修改说明
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式
//V1.5修改说明
//1,增加了对UCOSII的支持
#define USART2_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART2_RX 			1		//使能（1）/禁止（0）串口1接收
//按钮键值	  	
#define BUTTON1_NUM 0x01
#define BUTTON2_NUM 0x02
#define BUTTON3_NUM 0x03		
#define BUTTON4_NUM 0x04
#define BUTTON5_NUM 0x05
#define BUTTON6_NUM 0x06	
#define BUTTON7_NUM 0x07
#define BUTTON8_NUM 0x08
#define BUTTON9_NUM 0x09	
			
			
extern u8  USART2_RX_BUF[USART2_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART2_RX_STA;         		//接收状态标记	
//如果想串口中断接收，请不要注释以下宏定义
void USART2_Init(u32 bound);
//功能：检测串口屏是否有按键按下
//返回值：
//0：无按键按下
//其他：相应按键的键值
u8 ifButtonDown(void);
//更新变量值
void UpdateDisTemp(void);
void UpdateDisHumi(void);
void UpdateDisPres(void);

#endif


