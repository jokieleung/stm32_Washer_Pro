#ifndef _WASHER_H
#define _WASHER_H	

#include "sys.h"
#include "RELAY.H"
#include "USART_TO_LCD.H"
#include "dry.h"

void wash_func(void);

void stop_wash(void);

void Rst_Wash(void);

void WashDryfunc(void);
#endif

