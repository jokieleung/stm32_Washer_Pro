#include "timer.h"
#include "led.h"
#include "washer.h"
#include "USART_TO_LCD.H"

//通用定时器3中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!

//例子：TIM3_Int_Init(4999,7199);//10Khz的计数频率，计数到5000为500ms  
//定时器周期计算公式：[(1+TIM_Prescaler )*(1+TIM_Period )]/72M = T (s)
//设定定时周期为：1s  1.TIM_Prescaler = 9999 2.TIM_Period=7199

void TIM3_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器


	TIM_Cmd(TIM3, ENABLE);  //使能TIMx					 
}
//定时器3中断服务程序
//一些需要计时用的全局变量
//洗衣用的count变量
extern u8 Washer_Mode;//洗衣模式标志位
u16 waterin_count;//进水计数值
u16 wash_count;//洗衣计数值
u16 waterout_count;//排水计数值

void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
		{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx更新中断标志 
			
			//更新对应变量值
//		UpdateDisPres();
//		UpdateDisTemp();
//		UpdateDisHumi();
			
			//****************如果洗衣模式打开*************************
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












