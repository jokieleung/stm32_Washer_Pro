#include "SHT2x.h"
#include "delay.h"
#include "myiic.h"
#include<stdio.h>

SHT2x_data SHT20;
u8 SHT2x_Init(void)
{
	u8 err;
   I2C_INIT();
   err = SHT2x_SoftReset();
	return err;
}

u8 SHT2x_SoftReset(void) //SHT20软件复位
{
	u8 err=0;
	I2C_Start();
  I2C_Send_Byte(0x80);
	err = I2C_Wait_Ack();
	I2C_Send_Byte(0xFE);
	err = I2C_Wait_Ack();
	I2C_Stop();
	return err;
}
float SHT2x_GetTempPoll(void)
{
    float TEMP;
    u8 ack, tmp1, tmp2;
    u16 ST;
    u16 i=0;
    I2C_Start();				//发送IIC开始信号
    I2C_Send_Byte(I2C_ADR_W);			//IIC发送一个字节 
		ack = I2C_Wait_Ack();	
		I2C_Send_Byte(TRIG_TEMP_MEASUREMENT_POLL);
		ack = I2C_Wait_Ack();

    do {
        delay_ms(100);               
        I2C_Start();				//发送IIC开始信号
        I2C_Send_Byte(I2C_ADR_R);	
			  i++;
			  ack = I2C_Wait_Ack();
			  if(i==1000)break;
    } while(ack!=0);
    tmp1 = I2C_Read_Byte(1);
    tmp2 = I2C_Read_Byte(1);
		 I2C_Read_Byte(0);
    I2C_Stop();
    
    ST = (tmp1 << 8) | (tmp2 << 0);
	
    ST &= ~0x0003;
    TEMP = ((float)ST * 0.00268127) - 46.85;

    return (TEMP);	  
}

float SHT2x_GetHumiPoll(void)
{
    float HUMI;
    u8 ack, tmp1, tmp2;
    u16 SRH;
		u16 i=0;
    
    I2C_Start();				//发送IIC开始信号
    I2C_Send_Byte(I2C_ADR_W);			//IIC发送一个字节 
		ack = I2C_Wait_Ack();	
		I2C_Send_Byte(TRIG_HUMI_MEASUREMENT_POLL);
		ack = I2C_Wait_Ack();    
    do {
       delay_ms(100);               
        I2C_Start();				//发送IIC开始信号
        I2C_Send_Byte(I2C_ADR_R);	
			  i++;
			  ack = I2C_Wait_Ack();
			  if(i==100)break;
    } while(ack!=0);
    
    tmp1 = I2C_Read_Byte(1);
   
    tmp2 = I2C_Read_Byte(1);
     I2C_Read_Byte(0);
    I2C_Stop();
    
    SRH = (tmp1 << 8) | (tmp2 << 0);
    SRH &= ~0x0003;
    HUMI = ((float)SRH * 0.00190735) - 6;

    return (HUMI);
}


