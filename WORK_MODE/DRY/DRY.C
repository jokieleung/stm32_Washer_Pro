#include "dry.h"

//全局洗衣机工作状态位
extern u8 washing_flag,drying_flag,washDrying_flag;

//GAS_IN进气阀
//GAS_OUT	出气阀
//VACUUM_PUMP PD6真空泵
//MICROWAVE	磁控管

u8 Dry_Mode=0;//干衣模式标志位
extern u16 dry_count;//已干燥时间计数值

//功能：正常干衣模式
void dry_func(){
	u8 temp;
	dry_count = 0;//归零已干燥时间计数值
	Dry_Mode = 1;	//开启干衣模式标志位
	
	//检测是否按下结束按钮
	if(!drying_flag) {stop_dry();return;}
//	if((ifButtonDown()==BUTTON7_NUM)) {stop_dry();return;}
	
	//****************S1:抽真空（暂时达到真空不关闭真空泵）*************************
	MICROWAVE	= 1;//关闭磁控管
	GAS_IN = 1;//关闭进气（电磁阀）
	GAS_OUT = 0;//开启抽气（电磁阀）
	VACUUM_PUMP = 0;//打开真空泵
	while(GetPresAverage(ADC_Channel_7,10)>=-88)//等待气压达到 -0.088Mpa（约为-0.09Mpa）
	{
		if(!drying_flag) {stop_dry();return;}
	}
	//此时暂时不关闭真空泵，采用连续抽真空模式
//	VACUUM_PUMP = 1;//关闭真空泵
//	GAS_OUT = 1;//关闭抽气（电磁阀）
	
	//****************S2:开启磁控管开始加热（暂定采用间歇式工作模式）*************************
	//*******************直至达到设定干燥的湿度值********************************************
	MICROWAVE	= 0;//开启磁控管
	while(SHT2x_GetHumiPoll()>=10)//等待湿度降到10%  循环调用这个有返回值的函数可能会导致性能问题，先放  Jokie on 2018.3.3
	{
		//***************************************************
		//这块温控后期可做成PID来自控 Jokie 2018.3.3
		//PID输入：实时温度
		//PID输出：磁控管电源需要支持高频的通断才可
		//***************************************************
		if(SHT2x_GetTempPoll()>=48){   //若SHT20 温度大于48度停止磁控管
			MICROWAVE	= 1;//停止磁控管
		}
		else MICROWAVE	= 0;//否则开启磁控管
		
		if(!drying_flag) {stop_dry();return;}
		temp = ifButtonDown();
		printf("ifButtonDown temp:	%d	\n",temp);
		if(temp == BUTTON9_NUM) break;//暂时作为仿真达到干燥条件跳出循环用的按键
		
	}
	
	//****************S3:开启进气阀恢复常压，准备提示取衣******************************************
	MICROWAVE	= 1;//关闭磁控管
	GAS_OUT = 1;//关闭抽气（电磁阀）
	VACUUM_PUMP = 1;//关闭真空泵
	while(GetPresAverage(ADC_Channel_7,10)<=-5)//等待气压达到接近常压（约为-0.005Mpa）
	{
		GAS_IN = 0;//开启进气（电磁阀）
		if(!drying_flag) {stop_dry();return;}//检测是否按下结束按钮
	}
	GAS_IN = 1;//已恢复常压，关闭进气（电磁阀）
	
	Dry_Mode = 0;	//关闭干衣模式标志位
	
	//*******************S4:跳转到干衣完成的页面(20180304把这部分写到了JumpToFinishedUI();)*************************

}
//功能：终止干衣
void stop_dry(){
	dry_count = 0;//归零已干燥时间计数值
	MICROWAVE	= 1;//关闭磁控管
	GAS_OUT = 1;//关闭抽气（电磁阀）
	VACUUM_PUMP = 1;//关闭真空泵
	while(GetPresAverage(ADC_Channel_7,10)<=-5)//等待气压达到接近常压（约为-0.005Mpa）
	{
		GAS_IN = 0;//开启进气（电磁阀）
	}
	GAS_IN = 1;//已恢复常压，关闭进气（电磁阀）
	Dry_Mode = 0;	//关闭干衣模式标志位
	
}
//功能：干衣参数复位
void Rst_Dry(){
	
	MICROWAVE	= 1;//关闭磁控管
	GAS_OUT = 1;//关闭抽气（电磁阀）
	VACUUM_PUMP = 1;//关闭真空泵
	GAS_IN = 1;//关闭进气（电磁阀）
	
	Dry_Mode = 0;	//关闭干衣模式标志位
	
}
