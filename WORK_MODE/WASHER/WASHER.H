#ifndef _WASHER_H
#define _WASHER_H	

#include "sys.h"
#include "RELAY.H"
#include "USART_TO_LCD.H"
#include "dry.h"
//#define WATERIN_TIME 30 //进水时间变量  30s
//#define WASH_TIME 300 //洗衣时间变量  300s
//#define WATEROUT_TIME 30 //排水时间变量  30s

void wash_func(void);

void stop_wash(void);

void Rst_Wash(void);

void WashDryfunc(void);
#endif

