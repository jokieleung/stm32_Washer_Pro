/**
************************************************************
* @file         gizwits_product.h
* @brief        对应gizwits_product.c的头文件(包含产品软硬件版本定义)
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
#ifndef _GIZWITS_PRODUCT_H
#define _GIZWITS_PRODUCT_H

#include <stdint.h>
#include "gizwits_protocol.h"

/**
* @name 日志打印宏定义
* @{
*/
#define GIZWITS_LOG printf                          ///<运行日志打印
//#define PROTOCOL_DEBUG                              ///<协议数据打印

#ifndef ICACHE_FLASH_ATTR
#define ICACHE_FLASH_ATTR
#endif

/**@} */
//#define typedef_t typedef

/**
* MCU软件版本号
*/
#define SOFTWARE_VERSION "03010101"
/**
* MCU硬件版本号
*/
#define HARDWARE_VERSION "03010100"

#ifndef SOFTWARE_VERSION
    #error "no define SOFTWARE_VERSION"
#endif

#ifndef HARDWARE_VERSION
    #error "no define HARDWARE_VERSION"
#endif

/**@name 设备状态数据上报间隔时间宏定义
* @{
*/
#define REPORT_TIME_MAX 6000 //6S
/**@} */

void mcuRestart(void);
int32_t uartWrite(uint8_t *buf, uint32_t len);
void gizTimerMs(void);
/**@name Gizwits 用户API接口
* @{
*/
//int8_t gizwitsEventProcess(eventInfo_t *info, uint8_t *data, uint32_t len);
/**@} */

#endif
