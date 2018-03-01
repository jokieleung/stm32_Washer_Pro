#include "RELAY.H"

//初始化PD0-->PD.7（除去PD2） 输出口.并使能时钟		    
//RELAY_Init初始化
void RELAY_Init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;
		
		
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	 //使能PB端口时钟
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 |GPIO_Pin_3 |GPIO_Pin_4 |GPIO_Pin_5 |GPIO_Pin_6 |GPIO_Pin_7 ;	 //PD0-->PD.7 端口配置（除去PD2）
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOD, &GPIO_InitStructure);					 //根据设定参数初始化
	
	
	GPIO_SetBits(GPIOD,GPIO_Pin_0);						 //将全部继电器输出口设置为高电平
	GPIO_SetBits(GPIOD,GPIO_Pin_1);						 //将全部继电器输出口设置为高电平
	GPIO_SetBits(GPIOD,GPIO_Pin_3);						 //将全部继电器输出口设置为高电平
	GPIO_SetBits(GPIOD,GPIO_Pin_4);						 //将全部继电器输出口设置为高电平
	GPIO_SetBits(GPIOD,GPIO_Pin_5);						 //将全部继电器输出口设置为高电平
	GPIO_SetBits(GPIOD,GPIO_Pin_6);						 //将全部继电器输出口设置为高电平
	GPIO_SetBits(GPIOD,GPIO_Pin_7);						 //将全部继电器输出口设置为高电平
}



 
