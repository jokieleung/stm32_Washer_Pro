#ifndef __RELAY_H
#define __RELAY_H	 
#include "sys.h"

//后面改
#define WATER_IN PDout(0)// PD0  	进水阀 
#define WATER_OUT PDout(1)// PD1 	出水阀	
#define UNTRALSONIC PDout(3)// PD3超声波振子
#define GAS_IN PDout(4)// PD4			进气阀
#define GAS_OUT PDout(5)// PD5		出气阀
#define VACUUM_PUMP PDout(6)// PD6真空泵
#define MICROWAVE PDout(7)// PD7	磁控管

void RELAY_Init(void);//继电器控制口初始化

		 				    
#endif
