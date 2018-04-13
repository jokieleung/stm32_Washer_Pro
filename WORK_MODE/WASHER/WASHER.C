#include "washer.h"

u8 Washer_Mode;//洗衣模式标志位

//默认洗衣时间
#define WaterInTimeDefault 0 	//进水时间变量  60s
#define WashTimeDefault 120		//洗衣时间变量  300s
#define WaterOutTimeDefault 150	//排水时间变量  60s

u16 WATERIN_TIME=WaterInTimeDefault;
int WASH_TIME=WashTimeDefault; 
u16 WATEROUT_TIME=WaterOutTimeDefault; 

extern u16 waterin_count;//进水计数值
extern u16 wash_count;//洗衣计数值
extern u16 waterout_count;//排水计数值
//微波加热标志位
extern u8 microwave_mode;
extern u8 microwave_cnt;
//全局洗衣机工作状态位
extern u8 washing_flag,drying_flag,washDrying_flag;

//功能：单洗衣模式
void wash_func(){
	waterin_count = 0;//归零进水时间计数值
	wash_count = 0;//归零洗衣时间计数值
	waterout_count = 0;//归零排水时间计数值
	Washer_Mode = 1;	//开启洗衣模式标志位
	//检测是否按下结束按钮
	if(!washing_flag) {stop_wash();return;}
	//****************S1:进水*************************
	
	WATER_IN = 0;//打开进水阀
	while(waterin_count < WATERIN_TIME)//等待进水完毕
	{
		if(!washing_flag) {stop_wash();return;}
	}
	WATER_IN = 1;//关闭进水阀
	
	//****************S2:清洗*************************
	
	UNTRALSONIC = 0;//打开振子
	while(wash_count < WASH_TIME)//等待洗衣完毕
	{
		if(!washing_flag) {stop_wash();return;}
	}
	UNTRALSONIC = 1;//关闭振子
	
	//****************S3:排水*************************
	
	WATER_OUT = 0;//打开排水阀
	//CONNECT = 0;//打开连通阀
	//GAS_IN = 0;//打开进气阀
	while(waterout_count < WATEROUT_TIME)//等待排水完毕
	{
		if(!washing_flag) {stop_wash();return;}
	}
	WATER_OUT = 1;//关闭排水阀
	//CONNECT = 1;//关闭连通阀
	//GAS_IN = 1;//关闭进气阀
	Washer_Mode = 0;//关闭洗衣模式标志位
	
	//****************S4:跳转到洗衣完成的页面(20180304把这部分写到了JumpToFinishedUI();)*************************
	
}
//功能：终止洗衣
void stop_wash(){
	
	waterin_count = 0;//归零进水时间计数值
	wash_count = 0;//归零洗衣时间计数值
	waterout_count = 0;//归零排水时间计数值
	Washer_Mode = 0;	//关闭洗衣模式标志位
	

	WATERIN_TIME=WaterInTimeDefault; //进水时间变量复位
	WASH_TIME=WashTimeDefault;  //洗衣时间变量复位
	WATEROUT_TIME=WaterOutTimeDefault;  //排水时间变量复位

	WATER_IN = 1;//关闭进水阀
	UNTRALSONIC = 1;//关闭振子
	//CONNECT = 1;
	//GAS_IN = 1;
	
//	后期需要改为检测水位传感器的水量，排完水后关闭排水阀
	WATER_OUT = 1;//关闭排水阀
}
//功能：洗衣参数复位
void Rst_Wash(){
	
	waterin_count = 0;//归零进水时间计数值
	wash_count = 0;//归零洗衣时间计数值
	waterout_count = 0;//归零排水时间计数值
	Washer_Mode = 0;	//关闭洗衣模式标志位
	
	WATERIN_TIME=WaterInTimeDefault; //进水时间变量复位
	WASH_TIME=WashTimeDefault;  //洗衣时间变量复位
	WATEROUT_TIME=WaterOutTimeDefault;  //排水时间变量复位
	
	WATER_IN = 1;//关闭进水阀
	UNTRALSONIC = 1;//关闭振子
	//CONNECT = 1;
	//GAS_IN = 1;
//	后期需要改为检测水位传感器的水量，排完水后关闭排水阀
	WATER_OUT = 1;//关闭排水阀
}

extern u16 work_count;
extern u8 Dry_Mode;
//功能:洗烘一体流程
void WashDryfunc(){
//	u8 temp;
	work_count=0;
	//******************洗衣部分**********************************************************
	//************************************************************************************
	waterin_count = 0;//归零进水时间计数值
	wash_count = 0;//归零洗衣时间计数值
	waterout_count = 0;//归零排水时间计数值
	Washer_Mode = 1;	//开启洗衣模式标志位
	//检测是否按下结束按钮
	if(!washDrying_flag) {goto StopWashDry;}//检测是否按下结束按钮
	//****************S1:进水*************************
	
	WATER_IN = 0;//打开进水阀
	while(waterin_count < WATERIN_TIME)//等待进水完毕
	{
		if(!washDrying_flag) {goto StopWashDry;}//检测是否按下结束按钮
	}
	WATER_IN = 1;//关闭进水阀
	
	//****************S2:清洗*************************
	
	UNTRALSONIC = 0;//打开振子
	while(wash_count < WASH_TIME)//等待洗衣完毕
	{
		if(!washDrying_flag) {goto StopWashDry;}//检测是否按下结束按钮
	}
	UNTRALSONIC = 1;//关闭振子
	
	//****************S3:排水*************************
	
	WATER_OUT = 0;//打开排水阀
	while(waterout_count < WATEROUT_TIME)//等待排水完毕
	{
		if(!washDrying_flag) {goto StopWashDry;}//检测是否按下结束按钮
	}
	WATER_OUT = 1;//关闭排水阀
	
	Washer_Mode = 0;//关闭洗衣模式标志位
	
	//******************烘衣部分**********************************************************
	//************************************************************************************
		Dry_Mode = 1;	//开启干衣模式标志位
	
	if(ifButtonDown()==BUTTON7_NUM) {goto StopWashDry;}//检测是否按下结束按钮
	
	//****************S1:抽真空（暂时达到真空不关闭真空泵）*************************
	MICROWAVE	= 1;//关闭磁控管
	FAN = 1;//关闭磁控管散热风扇
	GAS_IN = 1;//关闭进气（电磁阀）
//	GAS_OUT = 0;//开启抽气（电磁阀）
	CONNECT = 0;//打开连通器
	VACUUM_PUMP = 0;//打开真空泵
	while(GetPresAverage(ADC_Channel_7,10)>=-88)//等待气压达到 -0.088Mpa（约为-0.09Mpa）
	{
		if(!washDrying_flag) {goto StopWashDry;}//检测是否按下结束按钮
	}
	//******************此时暂时不关闭真空泵，采用连续抽真空模式**************************
//	VACUUM_PUMP = 1;//关闭真空泵
//	GAS_OUT = 1;//关闭抽气（电磁阀）
	
	//****************S2:开启磁控管开始加热（暂定采用间歇式工作模式）*************************
	//*******************直至达到设定干燥的湿度值********************************************
	MICROWAVE	= 0;//开启磁控管
	FAN = 0;//开启磁控管散热风扇
		microwave_mode = 1;
	microwave_cnt = 0;
	while(1/*SHT2x_GetHumiPoll()>=10*/)//等待湿度降到10%  循环调用这个有返回值的函数可能会导致性能问题，先放  Jokie on 2018.3.3
	{
		//采用脉冲式方式驱动磁控管
		if(microwave_cnt >= 5){//5s取反一次
			MICROWAVE = !MICROWAVE;
			microwave_cnt = 0;
				}
		
		if(!drying_flag) {stop_dry();return;}
	}
	microwave_mode = 0;
	
	//****************S3:开启进气阀恢复常压，准备提示取衣******************************************
	MICROWAVE	= 1;//关闭磁控管
	FAN = 1;//关闭磁控管散热风扇
//	GAS_OUT = 1;//关闭抽气（电磁阀）
	VACUUM_PUMP = 1;//关闭真空泵
	while(GetPresAverage(ADC_Channel_7,10)<=-2)//等待气压达到接近常压（约为-0.002Mpa）
	{
		GAS_IN = 0;//开启进气（电磁阀）
		if(!washDrying_flag) {goto StopWashDry;}//检测是否按下结束按钮
	}
	GAS_IN = 1;//已恢复常压，关闭进气（电磁阀）
	CONNECT = 1;//关闭连通器
	Dry_Mode = 0;	//关闭干衣模式标志位
	
	StopWashDry:
	stop_wash();
	stop_dry();
	work_count=0;
}



