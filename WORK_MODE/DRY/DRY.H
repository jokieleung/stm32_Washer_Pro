#ifndef _DRY_H__
#define _DRY_H__	

#include "sys.h"
#include "RELAY.H"
#include "USART_TO_LCD.H"
 #include "adc.h"
 #include "SHT2x.h"
 
////#define WATERIN_TIME 30 //进水时间变量  30s
////#define WASH_TIME 300 //洗衣时间变量  300s
////#define WATEROUT_TIME 30 //排水时间变量  30s

void dry_func(void);

void stop_dry(void);

void Rst_Dry(void);
#endif

