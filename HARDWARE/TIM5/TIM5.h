#ifndef __TIMER5_H
#define __TIMER5_H

#include "sys.h"
#include "gizwits_protocol.h"
#include "led.h"
#include "washer.h"
#include "USART_TO_LCD.H"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//��ʱ�� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/3
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////   


void TIM5_Int_Init(u16 arr,u16 psc);
 
#endif
