#ifndef __RELAY_H
#define __RELAY_H	 
#include "sys.h"

//�����
#define WATER_IN PDout(0)// PD0  	��ˮ�� 
#define WATER_OUT PDout(1)// PD1 	��ˮ��	
#define UNTRALSONIC PDout(3)// PD3����������
#define GAS_IN PDout(4)// PD4			������
#define GAS_OUT PDout(5)// PD5		������
#define VACUUM_PUMP PDout(6)// PD6��ձ�
#define MICROWAVE PDout(7)// PD7	�ſع�

void RELAY_Init(void);//�̵������ƿڳ�ʼ��

		 				    
#endif