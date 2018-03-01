#include "washer.h"

u8 Washer_Mode;//洗衣模式标志位



extern u16 waterin_count;//进水计数值
extern u16 wash_count;//洗衣计数值
extern u16 waterout_count;//排水计数值
//功能：正常洗衣模式
void wash_func(){
	
	waterin_count = 0;//归零进水时间计数值
	wash_count = 0;//归零洗衣时间计数值
	waterout_count = 0;//归零排水时间计数值
	Washer_Mode = 1;	//开启洗衣模式标志位
	
	//****************S1:进水*************************
	
	WATER_IN = 0;//打开进水阀
	while(waterin_count < WATERIN_TIME)//等待进水完毕
	{
		UpdateDisPres();
		UpdateDisTemp();
		UpdateDisHumi();
	}
	WATER_IN = 1;//关闭进水阀
	
	//****************S2:清洗*************************
	
	UNTRALSONIC = 0;//打开振子
	while(wash_count < WASH_TIME)//等待洗衣完毕
	{
		UpdateDisPres();
		UpdateDisTemp();
		UpdateDisHumi();
	}
	UNTRALSONIC = 1;//关闭振子
	
	//****************S3:排水*************************
	
	WATER_OUT = 0;//打开排水阀
	while(waterout_count < WATEROUT_TIME)//等待排水完毕
	{
		UpdateDisPres();
		UpdateDisTemp();
		UpdateDisHumi();
	}
	WATER_OUT = 1;//关闭排水阀
	
	Washer_Mode = 0;//关闭洗衣模式标志位
	
}
