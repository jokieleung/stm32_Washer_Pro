/**
************************************************************
* @file         gizwits_product.c
* @brief        Gizwits 控制协议处理,及平台相关的硬件初始化 
* @author       Gizwits
* @date         2016-09-05
* @version      V03010101
* @copyright    Gizwits
* 
* @note         机智云.只为智能硬件而生
*               Gizwits Smart Cloud  for Smart Products
*               链接|增值ֵ|开放|中立|安全|自有|自由|生态
*               www.gizwits.com
*
***********************************************************/

#include <stdio.h>
#include <string.h>
#include "gizwits_protocol.h"
#include "led.h"
#include "usart3.h"
#include "beep.h"
#include "USART_TO_LCD.H"
/**@} */
/**@name Gizwits 用户API接口
* @{
*/
extern dataPoint_t currentDataPoint;
/**
* @brief 事件处理接口

* 说明：

* 1.用户可以对WiFi模组状态的变化进行自定义的处理

* 2.用户可以在该函数内添加数据点事件处理逻辑，如调用相关硬件外设的操作接口

* @param[in] info : 事件队列
* @param[in] data : 协议数据
* @param[in] len : 协议数据长度
* @return NULL
* @ref gizwits_protocol.h
*/

//全局洗衣机工作状态位
extern u8 washing_flag,drying_flag,washDrying_flag;
//全局进水、洗衣、排水时间变量
extern u16 WATERIN_TIME; //进水时间变量
extern u16 WASH_TIME; //洗衣时间变量
extern u16 WATEROUT_TIME; //排水时间变量
extern u16 waterin_count,wash_count,waterout_count;//进水计数值,洗衣计数值,排水计数值
extern int RemainWashTime;
int8_t gizwitsEventProcess(eventInfo_t *info, uint8_t *data, uint32_t len)
{
  uint8_t i = 0;
  dataPoint_t *dataPointPtr = (dataPoint_t *)data;
  moduleStatusInfo_t *wifiData = (moduleStatusInfo_t *)data;
  protocolTime_t *ptime = (protocolTime_t *)data;

  if((NULL == info) || (NULL == data))
  {
    return -1;
  }

  for(i=0; i<info->num; i++)
  {
    switch(info->event[i])
    {
			case EVENT_Wash:
        currentDataPoint.valueWash = dataPointPtr->valueWash;
        GIZWITS_LOG("Evt: EVENT_Wash %d \n", currentDataPoint.valueWash);
        if(0x01 == currentDataPoint.valueWash)
        {
          //user handle
					GIZWITS_LOG("	wash mode on	\n");
		
					washing_flag = 1;
					
        }
        else
        {
          //user handle    
					GIZWITS_LOG("wash mode else-------\n");
					//******bug fixed in 20180307  @Jokie*****************************************
					//原因是"protocol.h"内（#define typedef_t __packed typedef   __packed 关键字是用于字节对齐的）
					//未严格按照机智云的串口协议传输导致的字节错位而引起错误 导致下行传输的数据全是0
					washing_flag = 0;
        }
        break;
      case EVENT_Dry:
        currentDataPoint.valueDry = dataPointPtr->valueDry;
        GIZWITS_LOG("Evt: EVENT_Dry %d \n", currentDataPoint.valueDry);
        if(0x01 == currentDataPoint.valueDry)
        {
          //user handle
					GIZWITS_LOG("Dry mode on \n");

					drying_flag = 1;
        }
        else
        {
          //user handle    
					GIZWITS_LOG("Dry mode else \n");
					drying_flag = 0;
        }
        break;
      case EVENT_WashDry:
        currentDataPoint.valueWashDry = dataPointPtr->valueWashDry;
        GIZWITS_LOG("Evt: EVENT_WashDry %d \n", currentDataPoint.valueWashDry);
        if(0x01 == currentDataPoint.valueWashDry)
        {
          //user handle
					GIZWITS_LOG("WashDry mode on \n");
					washDrying_flag = 1;
				
        }
        else
        {
          //user handle  
					GIZWITS_LOG("WashDry mode else \n");
					washDrying_flag = 0;

        }
        break;
				
				case EVENT_WashTime:
        currentDataPoint.valueWashTime = dataPointPtr->valueWashTime;
        GIZWITS_LOG("Evt:EVENT_WashTime %d\n",currentDataPoint.valueWashTime);
//				printf("down side washtime:%d______ \n",currentDataPoint.valueWashTime);
				WASH_TIME = currentDataPoint.valueWashTime - (WATERIN_TIME+WATEROUT_TIME);//调整洗衣时间
//				GIZWITS_LOG("WASH_TIME___%d\n",WASH_TIME);
				
				RemainWashTime = (WATERIN_TIME+WASH_TIME+WATEROUT_TIME)-waterin_count-wash_count-waterout_count;
				UpdateRemainingWashTim(&RemainWashTime);
        //user handle
        break;
      case EVENT_DryTime:
        currentDataPoint.valueDryTime = dataPointPtr->valueDryTime;
        GIZWITS_LOG("Evt:EVENT_DryTime %d\n",currentDataPoint.valueDryTime);
        //user handle
        break;
      case EVENT_WashDryTime:
        currentDataPoint.valueWashDryTime = dataPointPtr->valueWashDryTime;
        GIZWITS_LOG("Evt:EVENT_WashDryTime %d\n",currentDataPoint.valueWashDryTime);
        //user handle
        break;


      case WIFI_SOFTAP:
        break;
      case WIFI_AIRLINK:
        break;
      case WIFI_STATION:
        break;
      case WIFI_CON_ROUTER:
        break;
      case WIFI_DISCON_ROUTER:
        break;
      case WIFI_CON_M2M:
        break;
      case WIFI_DISCON_M2M:
        break;
      case WIFI_RSSI:
        GIZWITS_LOG("RSSI %d\n", wifiData->rssi);
        break;
      case TRANSPARENT_DATA:
        GIZWITS_LOG("TRANSPARENT_DATA \n");
        //user handle , Fetch data from [data] , size is [len]
        break;
      case WIFI_NTP:
        GIZWITS_LOG("WIFI_NTP : [%d-%d-%d %02d:%02d:%02d][%d] \n",ptime->year,ptime->month,ptime->day,ptime->hour,ptime->minute,ptime->second,ptime->ntp);
        break;
      default:
        break;
    }
  }

  return 0;
}


/**@} */

/**
* @brief MCU复位函数

* @param none
* @return none
*/
void mcuRestart(void)
{
//	__set_FAULTMASK(1);//关闭所有中断
//	NVIC_SystemReset();//复位
}

/**@} */
/**
* @brief 系统毫秒计时维护函数,毫秒自增,溢出归零

* @param none
* @return none
*/
static uint32_t timerMsCount;
void gizTimerMs(void)
{
    timerMsCount++;
}

/**
* @brief 读取系统时间毫秒计时数

* @param none
* @return 系统时间毫秒数
*/
uint32_t gizGetTimerCount(void)
{
    return timerMsCount;
}

/**
* @brief 定时器TIM3中断处理函数

* @param none
* @return none
*/
void TIMER_IRQ_FUN(void)
{
  gizTimerMs();
}

/**
* @brief USART2串口中断函数

* 接收功能，用于接收与WiFi模组间的串口协议数据
* @param none
* @return none
*/
void UART_IRQ_FUN(void)
{
  uint8_t value = 0;
  //value = USART_ReceiveData(USART2);//STM32 test demo
  gizPutData(&value, 1);
}


/**
* @brief 串口写操作，发送数据到WiFi模组
*
* @param buf      : 数据地址
* @param len       : 数据长度
*
* @return : 正确返回有效数据长度;-1，错误返回
*/
int32_t uartWrite(uint8_t *buf, uint32_t len)
{
    uint32_t i = 0;
    
    if(NULL == buf)
    {
        return -1;
    }
#ifdef PROTOCOL_DEBUG
    GIZWITS_LOG("MCU2WiFi[%4d:%4d]: ", gizGetTimerCount(), len);
#endif
    
    for(i=0; i<len; i++)
    {
        //USART_SendData(UART, buf[i]);//STM32 test demo
        //实现串口发送函数,将buf[i]发送到模组
		USART_SendData(USART3,buf[i]);
        while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕 
#ifdef PROTOCOL_DEBUG
        GIZWITS_LOG("%02x ", buf[i]);
#endif
        if(i >=2 && buf[i] == 0xFF)
        {
          //实现串口发送函数,将0x55发送到模组
		   USART_SendData(USART3,0x55);
         while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕 
#ifdef PROTOCOL_DEBUG
        GIZWITS_LOG("%02x ", 0x55);
#endif
        }
    }
    
#ifdef PROTOCOL_DEBUG
    GIZWITS_LOG("\n");
#endif
    
    return len;
}


