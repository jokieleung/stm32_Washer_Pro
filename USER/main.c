#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
#include "usart3.h"
#include "beep.h"
#include "gizwits_product.h" 

#include "tim5.h"
#include "tim4.h"

#include "stm32f10x_conf.h"
#include "adc.h"
#include "USART_TO_LCD.H"
#include "washer.h"
#include "dry.h"
#include "VOICE.H"
/************************************************
 机智云-IOT超声波真空微波洗烘一体机 V1.0
 武汉触米科技有限公司 
 作者：Jokie @jokieleung@qq.com
************************************************/


/* 用户区当前设备状态结构体*/
dataPoint_t currentDataPoint;

//全局洗衣机工作状态位
u8 washing_flag=0,drying_flag=0,washDrying_flag=0;

//变量值，传感值的全局变量 
extern float Temp,Humi,Pres;
extern int RemainWashTime,DryTime,WashDryTime;

//UI跳转指令 数组
extern u8 START_W_ARY[];//正在洗衣界面
extern u8 START_D_ARY[];//正在烘干界面
extern u8 START_WD_ARY[];//正在烘洗界面

//协议初始化
void Gizwits_Init(void)
{  
	
	TIM3_Int_Init(9,7199);//1MS系统定时
    usart3_init(9600);//WIFI初始化
	memset((uint8_t*)&currentDataPoint, 0, sizeof(dataPoint_t));//设备状态结构体初始化
	gizwitsInit();//缓冲区初始化
}
//数据采集（上行逻辑）
void userHandle(void)
{
   
	//工作状态上传
	currentDataPoint.valueWash = washing_flag;
	currentDataPoint.valueDry = drying_flag;
	currentDataPoint.valueWashDry = washDrying_flag;
	//真空度、温湿度上传
	currentDataPoint.valuePres = Pres;
	currentDataPoint.valueTemp = Temp;
	currentDataPoint.valueHumi = Humi;
	//各工作状态的实时时间上传
	currentDataPoint.valueWashTime = RemainWashTime;
  currentDataPoint.valueDryTime = DryTime;
  currentDataPoint.valueWashDryTime = WashDryTime;
	
}


//主函数
 int main(void)
 {		
	 u8 DRY_FSH[]={0XA5,0X5A,0X04,0X80,0X03,0X00,0X0D};//烘衣完成界面
	 u8 WSHDRY_FSH[]={0XA5,0X5A,0X04,0X80,0X03,0X00,0X0C};//烘洗完成界面
	 delay_init();	    	 //延时函数初始化	  
	 NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	 Adc_Init();
	 SHT2x_Init();       //SHT2x初始化 含I2C初始化
	 USART2_Init(115200);	//串口2初始化为115200，与串口屏通信
	 RELAY_Init();//继电器初始化，初始化默认设置为高电平（继电器关闭）
	 TIM4_Int_Init(99,7199);//计数周期为10ms 用于屏幕的按键检测
	 uart_init(115200);	    //串口1初始化为115200 调试打印用
	 LED_Init();			    //LED端口初始化
	 KEY_Init();             //按键初始化 
	 BEEP_Init();            //蜂鸣器初始化
   Gizwits_Init();         //协议初始化
	 TIM5_Int_Init(9999,7199);//计数周期为1s   此后修改为定时洗衣时间以及烘干时间用
	 Voice_Init();
	 printf("--------机智云IOT-VacuumWasher----------\r\n");
	 printf("Init Over\r\n\r\n");
   	while(1)
	{
		
		//洗衣状态
		if(washing_flag){
			JumpToUI(START_W_ARY);
			wash_func();
			JumpToFinishedUI();//跳转到洗衣完成的页面
			washing_flag = 0;
		}
		//干衣状态
		if(drying_flag){
			JumpToUI(START_D_ARY);
			dry_func();
			JumpToUI(DRY_FSH);//跳转到干衣完成的页面
			drying_flag = 0;
		}
		//洗烘状态
		if(washDrying_flag){
			JumpToUI(START_WD_ARY);
			WashDryfunc();
			JumpToUI(WSHDRY_FSH);//跳转到洗干衣完成的页面
			washDrying_flag = 0;
		}
		
	}	 

} 
