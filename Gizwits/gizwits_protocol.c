/**
************************************************************
* @file         gizwits_protocol.c
* @brief        Gizwits协议相关文件 (SDK API 接口函数定义)
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
#include "gizwits_protocol.h"

/** 协议全局变量 **/
gizwitsProtocol_t gizwitsProtocol;

/**@name 串口接收环形缓冲区实现
* @{
*/
rb_t pRb;                                               ///< 环形缓冲区结构体变量
static uint8_t rbBuf[RB_MAX_LEN];                       ///< 环形缓冲区数据缓存区

static void rbCreate(rb_t* rb)
{
    if(NULL == rb)
    {
        GIZWITS_LOG("ERROR: input rb is NULL\n");
        return;
    }

    rb->rbHead = rb->rbBuff;
    rb->rbTail = rb->rbBuff;
}

static void rbDelete(rb_t* rb)
{
    if(NULL == rb)
    {
        GIZWITS_LOG("ERROR: input rb is NULL\n");
        return;
    }

    rb->rbBuff = NULL;
    rb->rbHead = NULL;
    rb->rbTail = NULL;
    rb->rbCapacity = 0;
}

static int32_t rbCapacity(rb_t *rb)
{
    if(NULL == rb)
    {
        GIZWITS_LOG("ERROR: input rb is NULL\n");
        return -1;
    }

    return rb->rbCapacity;
}

static int32_t rbCanRead(rb_t *rb)
{
    if(NULL == rb)
    {
        GIZWITS_LOG("ERROR: input rb is NULL\n");
        return -1;
    }

    if (rb->rbHead == rb->rbTail)
    {
        return 0;
    }

    if (rb->rbHead < rb->rbTail)
    {
        return rb->rbTail - rb->rbHead;
    }

    return rbCapacity(rb) - (rb->rbHead - rb->rbTail);
}

static int32_t rbCanWrite(rb_t *rb)
{
    if(NULL == rb)
    {
        GIZWITS_LOG("ERROR: input rb is NULL\n");
        return -1;
    }

    return rbCapacity(rb) - rbCanRead(rb);
}

static int32_t rbRead(rb_t *rb, void *data, size_t count)
{
    int copySz = 0;

    if(NULL == rb)
    {
        GIZWITS_LOG("ERROR: input rb is NULL\n");
        return -1;
    }

    if(NULL == data)
    {
        GIZWITS_LOG("ERROR: input data is NULL\n");
        return -1;
    }

    if (rb->rbHead < rb->rbTail)
    {
        copySz = min(count, rbCanRead(rb));
        memcpy(data, rb->rbHead, copySz);
        rb->rbHead += copySz;
        return copySz;
    }
    else
    {
        if (count < rbCapacity(rb)-(rb->rbHead - rb->rbBuff))
        {
            copySz = count;
            memcpy(data, rb->rbHead, copySz);
            rb->rbHead += copySz;
            return copySz;
        }
        else
        {
            copySz = rbCapacity(rb) - (rb->rbHead - rb->rbBuff);
            memcpy(data, rb->rbHead, copySz);
            rb->rbHead = rb->rbBuff;
            copySz += rbRead(rb, (char*)data+copySz, count-copySz);
            return copySz;
        }
    }
}

static int32_t rbWrite(rb_t *rb, const void *data, size_t count)
{
    int tailAvailSz = 0;

    if(NULL == rb)
    {
        GIZWITS_LOG("ERROR: rb is empty \n");
        return -1;
    }

    if(NULL == data)
    {
        GIZWITS_LOG("ERROR: data is empty \n");
        return -1;
    }

    if (count >= rbCanWrite(rb))
    {
        GIZWITS_LOG("ERROR: no memory %d \n", rbCanWrite(rb));
        return -1;
    }

    if (rb->rbHead <= rb->rbTail)
    {
        tailAvailSz = rbCapacity(rb) - (rb->rbTail - rb->rbBuff);
        if (count <= tailAvailSz)
        {
            memcpy(rb->rbTail, data, count);
            rb->rbTail += count;
            if (rb->rbTail == rb->rbBuff+rbCapacity(rb))
            {
                rb->rbTail = rb->rbBuff;
            }
            return count;
        }
        else
        {
            memcpy(rb->rbTail, data, tailAvailSz);
            rb->rbTail = rb->rbBuff;

            return tailAvailSz + rbWrite(rb, (char*)data+tailAvailSz, count-tailAvailSz);
        }
    }
    else
    {
        memcpy(rb->rbTail, data, count);
        rb->rbTail += count;
        return count;
    }
}
/**@} */

/**
* @brief 向环形缓冲区写入数据
* @param [in] buf        : buf地址
* @param [in] len        : 字节长度
* @return   正确 : 返回写入的数据长度
            失败 : -1
*/
int32_t gizPutData(uint8_t *buf, uint32_t len)
{
    int32_t count = 0;

    if(NULL == buf)
    {
        GIZWITS_LOG("ERROR: gizPutData buf is empty \n");
        return -1;
    }

    count = rbWrite(&pRb, buf, len);
    if(count != len)
    {
        GIZWITS_LOG("ERROR: Failed to rbWrite \n");
        return -1;
    }

    return count;
}

/**
* @brief 报文数据校验和计算
*
* @param [in] buf   : buf地址
* @param [in] len   : 字节长度
*
* @return sum : 从缓冲区第3个字节后所有的字节累加求和
*/
static uint8_t gizProtocolSum(uint8_t *buf, uint32_t len)
{
    uint8_t     sum = 0;
    uint32_t    i = 0;

    if(buf == NULL || len <= 0)
    {
        GIZWITS_LOG("ERROR: gizProtocolSum , buf is empty or len error %d \n", len);
        return 0;
    }

    for(i=2; i<len-1; i++)
    {
        sum += buf[i];
    }

    return sum;
}

/**
* @brief 16位数据字节序转换
*
* @param [in] value : 需要转换的数据
*
* @return  tmp_value: 转换后的数据
*/
static uint16_t gizProtocolExchangeBytes(uint16_t value)
{
    uint16_t    tmp_value;
    uint8_t     *index_1, *index_2;

    index_1 = (uint8_t *)&tmp_value;
    index_2 = (uint8_t *)&value;

    *index_1 = *(index_2+1);
    *(index_1+1) = *index_2;

    return tmp_value;
}

/**
* @brief 32位数据字节序转换
*
* @param [in] value : 需要转换的数据
*
* @return  tmp_value: 转换后的数据
*/
static uint32_t gizExchangeWord(uint32_t  value)
{
    return ((value & 0x000000FF) << 24) |
        ((value & 0x0000FF00) << 8) |
        ((value & 0x00FF0000) >> 8) |
        ((value & 0xFF000000) >> 24) ;
}

/**
* @brief 数组缓冲区网络字节序转换
*
* @param [in] buf      : buf地址
* @param [in] dataLen  : 字节长度
*
* @return 正确 : 0 
          失败 : -1
*/
static int8_t gizByteOrderExchange(uint8_t *buf,uint32_t dataLen)
{
    uint32_t i = 0;
    uint8_t preData = 0;
    uint8_t aftData = 0;

    if(NULL == buf)
    {
        GIZWITS_LOG("gizByteOrderExchange Error , Illegal Param\n");
        return -1;
    }

    for(i = 0;i<dataLen/2;i++)
    {
        preData = buf[i];
        aftData = buf[dataLen - i - 1];
        buf[i] = aftData;
        buf[dataLen - i - 1] = preData;
    }
    return 0;
}

/**
* @brief 转化为协议中的x值及实际通讯传输的值
*
* @param [in] ratio    : 修正系数k
* @param [in] addition : 增量m
* @param [in] preValue : 作为协议中的y值, 是App UI界面的显示值
*
* @return aft_value    : 作为协议中的x值, 是实际通讯传输的值
*/
static uint32_t gizY2X(uint32_t ratio, int32_t addition, int32_t preValue)
{
    uint32_t aftValue = 0;

    //x=(y - m)/k
    aftValue = ((preValue - addition) / ratio);

    return aftValue;
}

/**
* @brief 转化为协议中的y值及App UI界面的显示值
*
* @param [in] ratio    : 修正系数k
* @param [in] addition : 增量m
* @param [in] preValue : 作为协议中的x值, 是实际通讯传输的值
*
* @return aftValue : 作为协议中的y值, 是App UI界面的显示值
*/
static int32_t gizX2Y(uint32_t ratio, int32_t addition, uint32_t preValue)
{
    int32_t aftValue = 0;

    //y=k * x + m
    aftValue = (preValue * ratio + addition);

    return aftValue;
}

/**
* @brief 转化为协议中的x值及实际通讯传输的值,只用作对浮点型数据做处理
*
* @param [in] ratio    : 修正系数k
* @param [in] addition : 增量m
* @param [in] preValue : 作为协议中的y值, 是App UI界面的显示值
*
* @return  aft_value : 作为协议中的x值, 是实际通讯传输的值
*/
static uint32_t gizY2XFloat(float ratio, float addition, float preValue)
{
    uint32_t aftValue = 0;

    //x=(y - m)/k
    aftValue = ((preValue - addition) / ratio);

    return aftValue;
}

/**
* @brief 转化为协议中的y值及App UI界面的显示值,只用作对浮点型数据做处理
*
* @param [in] ratio    : 修正系数k
* @param [in] addition : 增量m
* @param [in] preValue : 作为协议中的x值, 是实际通讯传输的值
*
* @return : 作为协议中的y值, 是App UI界面的显示值
*/
static float gizX2YFloat(float ratio, float addition, uint32_t preValue)
{
    float aftValue = 0;

    //y=k * x + m
    aftValue = (preValue * ratio + addition);

    return aftValue;
}

/**
* @brief 数据点跨字节判断
*
* @param [in] bitOffset     : 位偏移
* @param [in] bitLen        : 占用位长度 
*
* @return 未跨字节 : 0 
            跨字节 : 1
*/
static uint8_t gizAcrossByteJudge(uint32_t bitOffset,uint32_t bitLen)
{
    if((0 == bitOffset)||(0 == bitOffset%8))
    {
        if(bitLen <= 8)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        if(8 - bitOffset%8 >= bitLen)
        {
            return 0;
        }
        else
        {
            return 1; 
        }
    }
}
/**
* @brief 协议头初始化
*
* @param [out] head         : 协议头指针
*
* @return 0， 执行成功， 非 0， 失败    
*/
static int8_t gizProtocolHeadInit(protocolHead_t *head)
{
    if(NULL == head)
    {
        GIZWITS_LOG("ERROR: gizProtocolHeadInit head is empty \n");
        return -1;
    }

    memset((uint8_t *)head, 0, sizeof(protocolHead_t));
    head->head[0] = 0xFF;
    head->head[1] = 0xFF;

    return 0;
}

/**
* @brief 协议ACK校验处理函数
*
* @param [in] data            : 数据地址
* @param [in] len             : 数据长度
*
* @return 0， 执行成功， 非 0， 失败
*/
static int8_t gizProtocolWaitAck(uint8_t *data, uint32_t len)
{
    if(NULL == data)
    {
        GIZWITS_LOG("ERROR: data is empty \n");
        return -1;
    }

    memset((uint8_t *)&gizwitsProtocol.waitAck, 0, sizeof(protocolWaitAck_t));
    memcpy((uint8_t *)gizwitsProtocol.waitAck.buf, data, len);
    gizwitsProtocol.waitAck.dataLen = (uint16_t)len;
    
    gizwitsProtocol.waitAck.flag = 1;
    gizwitsProtocol.waitAck.sendTime = gizGetTimerCount();

    return 0;
}





/**
* @brief bool和enum类型数据点数据解压
*
* @param [in] byteOffset    : 字节偏移
* @param [in] bitOffset     : 位偏移
* @param [in] bitLen        : 占用位长度
* @param [in] arrayAddr     : 数组地址
* @param [in] arrayLen      : 数组长度
*
* @return destValue         : 解压后结果数据,-1表示错误返回
*/
static int32_t ICACHE_FLASH_ATTR gizDecompressionValue(uint32_t byteOffset,uint32_t bitOffset,uint32_t bitLen,uint8_t *arrayAddr,uint32_t arrayLen)
{
    uint8_t ret = 0;
    uint8_t highBit = 0 ,lowBit = 0;
    uint8_t destBufTemp[COUNT_W_BIT];
    int32_t destValue = 0;
    
    if(NULL == arrayAddr || 0 == arrayLen)
    {
        GIZWITS_LOG("gizDecompressionValue Error , Illegal Param\n");
        return -1;
    }

    memcpy(destBufTemp,arrayAddr,arrayLen);
    if(arrayLen > 1)// Judge Byte order conversion
    {
        if(-1 == gizByteOrderExchange(destBufTemp,arrayLen))
        {
            GIZWITS_LOG("gizDecompressionValue gizByteOrderExchange Error \n");
            return -1;
        }
    }
    ret = gizAcrossByteJudge(bitOffset,bitLen);//Judge Byte Step
    if(0 == ret)
    {
        destValue |= ((destBufTemp[byteOffset] >> (bitOffset%8)) & (0xff >> (8 - bitLen)));
    }
    else if(1 == ret)
    {
        /* 暂时只支持最多跨2字节 */
        highBit = destBufTemp[byteOffset + 1]& (0xFF >> (8-(bitLen-(8-bitOffset%8))));
        lowBit = destBufTemp[byteOffset]>> (bitOffset%8);
        destValue |=  (highBit << (8-bitOffset%8));
        destValue |= lowBit;
    }
    return destValue;
}
/**
* @brief bool和enum类型数据点数据压缩
*
* @param [in] byteOffset    : 字节偏移
* @param [in] bitOffset     : 位偏移
* @param [in] bitLen        : 占用位长度
* @param [out] arrayAddr    : 数组地址
* @param [in] srcData       : 原始数据
*
* @return : 0,正确返回;-1，错误返回
*/
static int32_t ICACHE_FLASH_ATTR gizCompressValue(uint32_t byteOffset,uint32_t bitOffset,uint32_t bitLen,uint8_t *bufAddr,uint32_t srcData)
{
    uint8_t highBit = 0;
    uint8_t lowBit = 0;
    uint8_t ret = 0;

    if(NULL == bufAddr)
    {
        GIZWITS_LOG("gizCompressValue Error , Illegal Param\n");
        return -1;
    }

    ret = gizAcrossByteJudge(bitOffset,bitLen);
    if(0 == ret)
    {
        bufAddr[byteOffset] |= (((uint8_t)srcData)<<(bitOffset%8));
    }
    else if(1 == ret)
    {
        /* 暂时支持最多跨两字节的压缩 */ 
        highBit = ((uint8_t)srcData)>>(8-bitOffset%8);
        lowBit = (uint8_t)srcData & (0xFF >> (8-bitOffset%8));
        bufAddr[byteOffset + 1] |=  highBit;
        bufAddr[byteOffset] |= (lowBit<<(bitOffset%8));
    }

    return 0;
}

/**
* @brief 根据协议生成“控制型事件”

* @param [in] issuedData  ：控制型数据
* @param [out] info       ：事件队列
* @param [out] dataPoints ：数据点数据
* @return 0，执行成功， 非 0，失败
*/
static int8_t ICACHE_FLASH_ATTR gizDataPoint2Event(gizwitsIssued_t *issuedData, eventInfo_t *info, dataPoint_t *dataPoints)
{
    if((NULL == issuedData) || (NULL == info) ||(NULL == dataPoints))
    {
        GIZWITS_LOG("gizDataPoint2Event Error , Illegal Param\n");
        return -1;
    }
    /** 大于1字节做位序转换 **/
    if(sizeof(issuedData->attrFlags) > 1)
    {
        if(-1 == gizByteOrderExchange((uint8_t *)&issuedData->attrFlags,sizeof(attrFlags_t)))
        {
            GIZWITS_LOG("gizByteOrderExchange Error\n");
            return -1;
        }
    }
    if(0x01 == issuedData->attrFlags.flagWash)
    {
        info->event[info->num] = EVENT_Wash;
        info->num++;
        dataPoints->valueWash = gizDecompressionValue(Wash_BYTEOFFSET,Wash_BITOFFSET,Wash_LEN,(uint8_t *)&issuedData->attrVals.wBitBuf,sizeof(issuedData->attrVals.wBitBuf));
    }


    if(0x01 == issuedData->attrFlags.flagDry)
    {
        info->event[info->num] = EVENT_Dry;
        info->num++;
        dataPoints->valueDry = gizDecompressionValue(Dry_BYTEOFFSET,Dry_BITOFFSET,Dry_LEN,(uint8_t *)&issuedData->attrVals.wBitBuf,sizeof(issuedData->attrVals.wBitBuf));
    }
        
    if(0x01 == issuedData->attrFlags.flagWashDry)
    {
        info->event[info->num] = EVENT_WashDry;
        info->num++;
        dataPoints->valueWashDry = gizDecompressionValue(WashDry_BYTEOFFSET,WashDry_BITOFFSET,WashDry_LEN,(uint8_t *)&issuedData->attrVals.wBitBuf,sizeof(issuedData->attrVals.wBitBuf));
    }
		
		if(0x01 == issuedData->attrFlags.flagWashTime)
    {
        info->event[info->num] = EVENT_WashTime;
        info->num++;
        dataPoints->valueWashTime = gizX2Y(WashTime_RATIO,  WashTime_ADDITION, exchangeBytes(issuedData->attrVals.valueWashTime));
    }
        
    if(0x01 == issuedData->attrFlags.flagDryTime)
    {
        info->event[info->num] = EVENT_DryTime;
        info->num++;
        dataPoints->valueDryTime = gizX2Y(DryTime_RATIO,  DryTime_ADDITION, exchangeBytes(issuedData->attrVals.valueDryTime));
    }
        
    if(0x01 == issuedData->attrFlags.flagWashDryTime)
    {
        info->event[info->num] = EVENT_WashDryTime;
        info->num++;
        dataPoints->valueWashDryTime = gizX2Y(WashDryTime_RATIO,  WashDryTime_ADDITION, exchangeBytes(issuedData->attrVals.valueWashDryTime));
    }



    return 0;
}

/**
* @brief 对比当前数据和上次数据
*
* @param [in] cur        : 当前数据点数据
* @param [in] last       : 上次数据点数据
*
* @return : 0,数据无变化;1，数据有变化
*/
static int8_t ICACHE_FLASH_ATTR gizCheckReport(dataPoint_t *cur, dataPoint_t *last)
{
    int8_t ret = 0;
    static uint32_t lastReportTime = 0;

    if((NULL == cur) || (NULL == last))
    {
        GIZWITS_LOG("gizCheckReport Error , Illegal Param\n");
        return -1;
    }
    if(last->valueWash != cur->valueWash)
    {
        GIZWITS_LOG("valueWash Changed\n");
        ret = 1;
    }
    if(last->valueDry != cur->valueDry)
    {
        GIZWITS_LOG("valueDry Changed\n");
        ret = 1;
    }
    if(last->valueWashDry != cur->valueWashDry)
    {
        GIZWITS_LOG("valueWashDry Changed\n");
        ret = 1;
    }
		if(last->valueWashTime != cur->valueWashTime)
    {
        GIZWITS_LOG("valueWashTime Changed\n");
        ret = 1;
    }
    if(last->valueDryTime != cur->valueDryTime)
    {
        GIZWITS_LOG("valueDryTime Changed\n");
        ret = 1;
    }
    if(last->valueWashDryTime != cur->valueWashDryTime)
    {
        GIZWITS_LOG("valueWashDryTime Changed\n");
        ret = 1;
    }

    if(last->valuePres != cur->valuePres)
    {
        if(gizGetTimerCount()-lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePres Changed\n");
            lastReportTime = gizGetTimerCount();
            ret = 1;
        }
    }
    if(last->valueTemp != cur->valueTemp)
    {
        if(gizGetTimerCount()-lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueTemp Changed\n");
            lastReportTime = gizGetTimerCount();
            ret = 1;
        }
    }
    if(last->valueHumi != cur->valueHumi)
    {
        if(gizGetTimerCount()-lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueHumi Changed\n");
            lastReportTime = gizGetTimerCount();
            ret = 1;
        }
    }


    return ret;
}

/**
* @brief 用户数据点数据转换为机智云上报数据点数据
*
* @param [in]  dataPoints           : 用户数据点数据地址
* @param [out] devStatusPtr         : 机智云上报数据点数据地址
*
* @return 0,正确返回;-1，错误返回
*/
static int8_t ICACHE_FLASH_ATTR gizDataPoints2ReportData(dataPoint_t *dataPoints , devStatus_t *devStatusPtr)
{
    if((NULL == dataPoints) || (NULL == devStatusPtr))
    {
        GIZWITS_LOG("gizDataPoints2ReportData Error , Illegal Param\n");
        return -1;
    }

//    gizMemset((uint8_t *)devStatusPtr->wBitBuf,0,sizeof(devStatusPtr->wBitBuf));
		memset((uint8_t *)devStatusPtr->wBitBuf,0,sizeof(devStatusPtr->wBitBuf));
		
    gizCompressValue(Wash_BYTEOFFSET,Wash_BITOFFSET,Wash_LEN,(uint8_t *)devStatusPtr,dataPoints->valueWash);
    gizCompressValue(Dry_BYTEOFFSET,Dry_BITOFFSET,Dry_LEN,(uint8_t *)devStatusPtr,dataPoints->valueDry);
    gizCompressValue(WashDry_BYTEOFFSET,WashDry_BITOFFSET,WashDry_LEN,(uint8_t *)devStatusPtr,dataPoints->valueWashDry);
    gizByteOrderExchange((uint8_t *)devStatusPtr->wBitBuf,sizeof(devStatusPtr->wBitBuf));

		devStatusPtr->valuePres = gizY2X(Pres_RATIO,  Pres_ADDITION, dataPoints->valuePres); 

    devStatusPtr->valueWashTime = exchangeBytes(gizY2X(WashTime_RATIO,  WashTime_ADDITION, dataPoints->valueWashTime)); 
    devStatusPtr->valueDryTime = exchangeBytes(gizY2X(DryTime_RATIO,  DryTime_ADDITION, dataPoints->valueDryTime)); 
    devStatusPtr->valueWashDryTime = exchangeBytes(gizY2X(WashDryTime_RATIO,  WashDryTime_ADDITION, dataPoints->valueWashDryTime)); 
    devStatusPtr->valueTemp = exchangeBytes(gizY2XFloat(Temp_RATIO,  Temp_ADDITION, dataPoints->valueTemp));     
    devStatusPtr->valueHumi = exchangeBytes(gizY2XFloat(Humi_RATIO,  Humi_ADDITION, dataPoints->valueHumi));     





    return 0;
}

/**
* @brief 接收协议报文并进行相应的协议处理

* Wifi模组接收来自云端或APP端下发的相关协议数据发送到MCU端，经过协议报文解析后将相关协议数据传入次函数，进行下一步的协议处理

* @param[in] inData   : 输入的协议数据
* @param[in] inLen    : 输入数据的长度
* @param[out] outData : 输出的协议数据
* @param[out] outLen  : 输出数据的长度
* @return 0， 执行成功， 非 0， 失败
*/
static int8_t gizProtocolIssuedProcess(uint8_t *inData, uint32_t inLen, uint8_t *outData, uint32_t *outLen)
{
    protocolReport_t *protocolIssuedData = (protocolReport_t *)inData;
    uint8_t issuedAction = 0;
    issuedAction = protocolIssuedData->action;

    if((NULL == inData)||(NULL == outData)||(NULL == outLen))
    {
        GIZWITS_LOG("gizProtocolIssuedProcess Error , Illegal Param\n");
        return -1;
    }
    
    memset((uint8_t *)&gizwitsProtocol.issuedProcessEvent, 0, sizeof(eventInfo_t));
    switch(issuedAction)
    {
        case ACTION_CONTROL_DEVICE:
            gizDataPoint2Event((gizwitsIssued_t *)(inData+sizeof(protocolP0Head_t)), &gizwitsProtocol.issuedProcessEvent,&gizwitsProtocol.gizCurrentDataPoint);
            gizwitsProtocol.issuedFlag = ACTION_CONTROL_TYPE;
            outData = NULL;
            *outLen = 0;
            break;
        
        case ACTION_READ_DEV_STATUS:
            if(0 == gizDataPoints2ReportData(&gizwitsProtocol.gizLastDataPoint,&gizwitsProtocol.reportData.devStatus))
            {
                memcpy(outData, (uint8_t *)&gizwitsProtocol.reportData.devStatus, sizeof(gizwitsReport_t));
                *outLen = sizeof(gizwitsReport_t);
            }
            else
            {
                return -1;
            }
            break;
        case ACTION_W2D_TRANSPARENT_DATA:
            memcpy(gizwitsProtocol.transparentBuff, inData+sizeof(protocolP0Head_t), inLen-sizeof(protocolP0Head_t)-1);
            gizwitsProtocol.transparentLen = inLen - sizeof(protocolP0Head_t) - 1;
            gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = TRANSPARENT_DATA;
            gizwitsProtocol.issuedProcessEvent.num++;
            gizwitsProtocol.issuedFlag = ACTION_W2D_TRANSPARENT_TYPE;
            outData = NULL;
            *outLen = 0;
            break;
        
        default:
            break;
    }

    return 0;
}
/**
* @brief 协议下发数据返回ACK
*
* @param [in] head                  : 协议头指针
* @param [in] data                  : 数据地址
* @param [in] len                   : 数据长度
*
* @return : 有效数据长度,正确返回;-1，错误返回
*/
static int32_t gizProtocolIssuedDataAck(protocolHead_t *head, uint8_t *data, uint32_t len)
{
    int32_t ret = 0;
    uint8_t *ptrData = NULL;
    uint32_t dataLen = 0;
    protocolReport_t protocolReport;
    protocolCommon_t protocolCommon;
    protocolP0Head_t *p0Head = (protocolP0Head_t *)head;

    if((NULL == head)||(NULL == data))
    {
        GIZWITS_LOG("gizProtocolIssuedDataAck Error , Illegal Param\n");
        return -1;
    }
    if(0 == len)
    {
        gizProtocolHeadInit((protocolHead_t *)&protocolCommon);
        protocolCommon.head.cmd = head->cmd+1;
        protocolCommon.head.sn = head->sn;
        protocolCommon.head.len = gizProtocolExchangeBytes(sizeof(protocolCommon_t)-4);
        protocolCommon.sum = gizProtocolSum((uint8_t *)&protocolCommon, sizeof(protocolCommon_t));
        
        ptrData = (uint8_t *)&protocolCommon;
        dataLen = sizeof(protocolCommon_t);
    }
    else
    {
        gizProtocolHeadInit((protocolHead_t *)&protocolReport);
        protocolReport.head.cmd = p0Head->head.cmd+1;
        protocolReport.head.sn = p0Head->head.sn;
        protocolReport.head.len = gizProtocolExchangeBytes(sizeof(protocolP0Head_t)+len+1-4);
        protocolReport.action = p0Head->action+1;

        memcpy((uint8_t *)&protocolReport.reportData, data, len);
        protocolReport.sum = gizProtocolSum((uint8_t *)&protocolReport, sizeof(protocolReport_t));
        
        ptrData = (uint8_t *)&protocolReport;
        dataLen = sizeof(protocolReport_t);
    }
    
    ret = uartWrite(ptrData, dataLen);
    if(ret < 0)
    {
        //打印日志
        GIZWITS_LOG("ERROR: gizProtocolIssuedDataAck uart write error %d dataLen %d \n", ret, dataLen);
    }

    return ret;
}
/**
* @brief 上报数据
*
* @param [in] action            : PO cmd
* @param [in] data              : 数据地址
* @param [in] len               : 数据长度
*
* @return : 正确返回有效数据长度;-1，错误返回
*/
static int32_t gizReportData(uint8_t action, uint8_t *data, uint32_t len)
{
    int32_t ret = 0;
    protocolReport_t protocolReport;

    if(NULL == data)
    {
        GIZWITS_LOG("gizReportData Error , Illegal Param\n");
        return -1;
    }
    gizProtocolHeadInit((protocolHead_t *)&protocolReport);
    protocolReport.head.cmd = CMD_REPORT_P0;
    protocolReport.head.sn = gizwitsProtocol.sn++;
    protocolReport.action = action;
    protocolReport.head.len = gizProtocolExchangeBytes(sizeof(protocolReport_t)-4);
    memcpy((gizwitsReport_t *)&protocolReport.reportData, (gizwitsReport_t *)data,len);
    protocolReport.sum = gizProtocolSum((uint8_t *)&protocolReport, sizeof(protocolReport_t));
    
    ret = uartWrite((uint8_t *)&protocolReport, sizeof(protocolReport_t));
    if(ret < 0)
    {
        //打印日志
        GIZWITS_LOG("ERROR: uart write error %d \n", ret);
    }

    gizProtocolWaitAck((uint8_t *)&protocolReport, sizeof(protocolReport_t));

    return ret;
}

/**
* @brief 上报机制
*
* @param [in] currentData       : 数据地址
*
* @return : 无
*/
static void gizDevReportPolicy(dataPoint_t *currentData)
{
    /** 变化上报机制 **/
    if((1 == gizCheckReport(currentData, (dataPoint_t *)&gizwitsProtocol.gizLastDataPoint)))
    {
        GIZWITS_LOG("changed, report data\n");
        if(0 == gizDataPoints2ReportData(currentData,&gizwitsProtocol.reportData.devStatus))
        {
                gizReportData(ACTION_REPORT_DEV_STATUS, (uint8_t *)&gizwitsProtocol.reportData.devStatus, sizeof(devStatus_t));
        }       
        memcpy((uint8_t *)&gizwitsProtocol.gizLastDataPoint, (uint8_t *)currentData, sizeof(dataPoint_t));
    }
    /** 定时上报机制 **/
    if(0 == (gizGetTimerCount() % (600000)))
    {
        GIZWITS_LOG("Info: 600S report data\n");
        if(0 == gizDataPoints2ReportData(currentData,&gizwitsProtocol.reportData.devStatus))
        {
					gizReportData(ACTION_REPORT_DEV_STATUS, (uint8_t *)&gizwitsProtocol.reportData.devStatus, sizeof(devStatus_t));
        }       
        memcpy((uint8_t *)&gizwitsProtocol.gizLastDataPoint, (uint8_t *)currentData, sizeof(dataPoint_t));
    }
}

/**
* @brief 从环形缓冲区中抓取一包数据
*
* @param [in]  rb                  : 输入数据地址
* @param [out] data                : 输出数据地址
* @param [out] len                 : 输出数据长度
*
* @return : 0,正确返回;-1，错误返回;-2，数据校验失败
*/
static int8_t gizProtocolGetOnePacket(rb_t *rb, uint8_t *data, uint16_t *len)
{
    int32_t ret = 0;
    uint8_t sum = 0;
    int32_t i = 0;
    uint8_t tmpData;
    uint8_t tmpLen = 0;
    uint16_t tmpCount = 0;
    static uint8_t protocolFlag = 0;
    static uint16_t protocolCount = 0;
    static uint8_t lastData = 0;
    static uint8_t debugCount = 0;
    uint8_t *protocolBuff = data;
    protocolHead_t *head = NULL;

    if((NULL == rb) || (NULL == data) ||(NULL == len))
    {
        GIZWITS_LOG("gizProtocolGetOnePacket Error , Illegal Param\n");
        return -1;
    }

    tmpLen = rbCanRead(rb);
    if(0 == tmpLen)
    {
        return -1;
    }

    for(i=0; i<tmpLen; i++)
    {
        ret = rbRead(rb, &tmpData, 1);
        if(0 != ret)
        {
            if((0xFF == lastData) && (0xFF == tmpData))
            {
                if(0 == protocolFlag)
                {
                    protocolBuff[0] = 0xFF;
                    protocolBuff[1] = 0xFF;
                    protocolCount = 2;
                    protocolFlag = 1;
                }
                else
                {
                    if((protocolCount > 4) && (protocolCount != tmpCount))
                    {
                        protocolBuff[0] = 0xFF;
                        protocolBuff[1] = 0xFF;
                        protocolCount = 2;
                    }
                }
            }
            else if((0xFF == lastData) && (0x55 == tmpData))
            {
            }
            else
            {
                if(1 == protocolFlag)
                {
                    protocolBuff[protocolCount] = tmpData;
                    protocolCount++;

                    if(protocolCount > 4)
                    {
                        head = (protocolHead_t *)protocolBuff;
                        tmpCount = gizProtocolExchangeBytes(head->len)+4;
                        if(protocolCount == tmpCount)
                        {
                            break;
                        }
                    }
                }
            }

            lastData = tmpData;
            debugCount++;
        }
    }

    if((protocolCount > 4) && (protocolCount == tmpCount))
    {
        sum = gizProtocolSum(protocolBuff, protocolCount);

        if(protocolBuff[protocolCount-1] == sum)
        {
            memcpy(data, protocolBuff, tmpCount);
            *len = tmpCount;
            protocolFlag = 0;

            protocolCount = 0;
            debugCount = 0;
            lastData = 0;

            return 0;
        }
        else
        {
            return -2;
        }
    }

    return 1;
}



/**
* @brief 协议数据重发

* 校验超时且满足重发次数限制的协议数据进行重发

* @param none    
*
* @return none
*/
static void gizProtocolResendData(void)
{
    int32_t ret = 0;

    if(0 == gizwitsProtocol.waitAck.flag)
    {
        return;
    }

    GIZWITS_LOG("Warning: timeout, resend data \n");
    
    ret = uartWrite(gizwitsProtocol.waitAck.buf, gizwitsProtocol.waitAck.dataLen);
    if(ret != gizwitsProtocol.waitAck.dataLen)
    {
        GIZWITS_LOG("ERROR: resend data error\n");
    }

    gizwitsProtocol.waitAck.sendTime = gizGetTimerCount();
}

/**
* @brief 清除ACK协议报文
*
* @param [in] head : 协议头地址
*
* @return 0， 执行成功， 非 0， 失败
*/
static int8_t gizProtocolWaitAckCheck(protocolHead_t *head)
{
    protocolHead_t *waitAckHead = (protocolHead_t *)gizwitsProtocol.waitAck.buf;

    if(NULL == head)
    {
        GIZWITS_LOG("ERROR: data is empty \n");
        return -1;
    }

    if(waitAckHead->cmd+1 == head->cmd)
    {
        memset((uint8_t *)&gizwitsProtocol.waitAck, 0, sizeof(protocolWaitAck_t));
    }

    return 0;
}

/**
* @brief 发送通用协议报文数据
* 
* @param [in] head              : 协议头指针
*
* @return : 有效数据长度,正确返回;-1，错误返回
*/
static int32_t gizProtocolCommonAck(protocolHead_t *head)
{
    int32_t ret = 0;
    protocolCommon_t ack;

    if(NULL == head)
    {
        GIZWITS_LOG("ERROR: gizProtocolCommonAck data is empty \n");
        return -1;
    }
    memcpy((uint8_t *)&ack, (uint8_t *)head, sizeof(protocolHead_t));
    ack.head.cmd = ack.head.cmd+1;
    ack.head.len = gizProtocolExchangeBytes(sizeof(protocolCommon_t)-4);
    ack.sum = gizProtocolSum((uint8_t *)&ack, sizeof(protocolCommon_t));

    ret = uartWrite((uint8_t *)&ack, sizeof(protocolCommon_t));
    if(ret < 0)
    {
        //打印日志
        GIZWITS_LOG("ERROR: uart write error %d \n", ret);
    }

    return ret;
}

/**
* @brief ACK逻辑处理函数

* 发送后的协议数据进行超时判断，超时200ms进行重发，重发上限为三次

* @param none 
*
* @return none
*/
static void gizProtocolAckHandle(void)
{
    if(1 == gizwitsProtocol.waitAck.flag)
    {
        if(SEND_MAX_NUM > gizwitsProtocol.waitAck.num)
        {
            // 超时未收到ACK重发
            if(SEND_MAX_TIME < (gizGetTimerCount() - gizwitsProtocol.waitAck.sendTime))
            {
                GIZWITS_LOG("Warning:gizProtocolResendData %d %d %d\n", gizGetTimerCount(), gizwitsProtocol.waitAck.sendTime, gizwitsProtocol.waitAck.num);
                gizProtocolResendData();
                gizwitsProtocol.waitAck.num++;
            }
        }
        else
        {
            memset((uint8_t *)&gizwitsProtocol.waitAck, 0, sizeof(protocolWaitAck_t));
        }
    }
}

/**
* @brief 协议4.1 WiFi模组请求设备信息
*
* @param head : 待发送的协议报文数据
*
* @return 返回有效数据长度; -1，错误返回
*/
static int32_t gizProtocolGetDeviceInfo(protocolHead_t * head)
{
    int32_t ret = 0;
    protocolDeviceInfo_t deviceInfo;

    if(NULL == head)
    {
        GIZWITS_LOG("gizProtocolGetDeviceInfo Error , Illegal Param\n");
        return -1;
    }

    gizProtocolHeadInit((protocolHead_t *)&deviceInfo);
    deviceInfo.head.cmd = ACK_GET_DEVICE_INFO;
    deviceInfo.head.sn = head->sn;
    memcpy((uint8_t *)deviceInfo.protocolVer, protocol_VERSION, 8);
    memcpy((uint8_t *)deviceInfo.p0Ver, P0_VERSION, 8);
    memcpy((uint8_t *)deviceInfo.softVer, SOFTWARE_VERSION, 8);
    memcpy((uint8_t *)deviceInfo.hardVer, HARDWARE_VERSION, 8);
    memcpy((uint8_t *)deviceInfo.productKey, PRODUCT_KEY, strlen(PRODUCT_KEY));
    memcpy((uint8_t *)deviceInfo.productSecret, PRODUCT_SECRET, strlen(PRODUCT_SECRET));
    memset((uint8_t *)deviceInfo.devAttr, 0, 8);
    deviceInfo.devAttr[7] |= DEV_IS_GATEWAY<<0;
    deviceInfo.devAttr[7] |= (0x01<<1);
    deviceInfo.ninableTime = gizProtocolExchangeBytes(NINABLETIME);
    deviceInfo.head.len = gizProtocolExchangeBytes(sizeof(protocolDeviceInfo_t)-4);
    deviceInfo.sum = gizProtocolSum((uint8_t *)&deviceInfo, sizeof(protocolDeviceInfo_t));

    ret = uartWrite((uint8_t *)&deviceInfo, sizeof(protocolDeviceInfo_t));
    if(ret < 0)
    {
        //打印日志
        GIZWITS_LOG("ERROR: uart write error %d \n", ret);
    }
    
    return ret;
}

/**
* @brief 协议4.7 非法消息通知 的处理

* @param[in] head  : 协议头指针
* @param[in] errno : 非法消息通知类型
* @return 0， 执行成功， 非 0， 失败
*/
static int32_t gizProtocolErrorCmd(protocolHead_t *head,errorPacketsType_t errno)
{
    int32_t ret = 0;
    protocolErrorType_t errorType;

    if(NULL == head)
    {
        GIZWITS_LOG("gizProtocolErrorCmd Error , Illegal Param\n");
        return -1;
    }
    gizProtocolHeadInit((protocolHead_t *)&errorType);
    errorType.head.cmd = ACK_ERROR_PACKAGE;
    errorType.head.sn = head->sn;
    
    errorType.head.len = gizProtocolExchangeBytes(sizeof(protocolErrorType_t)-4);
    errorType.error = errno;
    errorType.sum = gizProtocolSum((uint8_t *)&errorType, sizeof(protocolErrorType_t));
    
    ret = uartWrite((uint8_t *)&errorType, sizeof(protocolErrorType_t));
    if(ret < 0)
    {
        //打印日志
        GIZWITS_LOG("ERROR: uart write error %d \n", ret);
    }

    return ret;
}

/**
* @brief 对应协议 4.13 接收返回的网络时间 中
*
* @param [in] head : 协议头地址
*
* @return 0， 执行成功， 非 0， 失败
*/
static int8_t gizProtocolNTP(protocolHead_t *head)
{
    int32_t ret = 0;
    
    protocolUTT_t *UTTInfo = (protocolUTT_t *)head;
    
    if(NULL == head)
    {
        GIZWITS_LOG("ERROR: NTP is empty \n");
        return -1;
    }
    
    memcpy((uint8_t *)&gizwitsProtocol.TimeNTP,(uint8_t *)UTTInfo->time, (7 + 4));
    gizwitsProtocol.TimeNTP.year = gizProtocolExchangeBytes(gizwitsProtocol.TimeNTP.year);
    gizwitsProtocol.TimeNTP.ntp = gizExchangeWord(gizwitsProtocol.TimeNTP.ntp);

    gizwitsProtocol.NTPEvent.event[gizwitsProtocol.NTPEvent.num] = WIFI_NTP;
    gizwitsProtocol.NTPEvent.num++;
    
    gizwitsProtocol.issuedFlag = GET_NTP_TYPE;
    
    
    return 0;
}

/**
* @brief 协议4.4 设备MCU重置WiFi模组 的相关操作

* @param none
* @return none
*/
static void gizProtocolReboot(void)
{
    uint32_t timeDelay = gizGetTimerCount();
    
    /*Wait 600ms*/
    while((gizGetTimerCount() - timeDelay) <= 600);
    mcuRestart();
}

/**
* @brief 协议 4.5 WiFi模组向设备MCU通知WiFi模组工作状态的变化

* @param[in] status WiFi模组工作状态
* @return none
*/
static int8_t gizProtocolModuleStatus(protocolWifiStatus_t *status)
{
    static wifiStatus_t lastStatus;

    if(NULL == status)
    {
        GIZWITS_LOG("gizProtocolModuleStatus Error , Illegal Param\n");
        return -1;
    }

    status->ststus.value = gizProtocolExchangeBytes(status->ststus.value);
   
    //OnBoarding mode status
    if(lastStatus.types.onboarding != status->ststus.types.onboarding)
    {
        if(1 == status->ststus.types.onboarding)
        {
            if(1 == status->ststus.types.softap)
            {
                gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_SOFTAP;
                gizwitsProtocol.wifiStatusEvent.num++;
                GIZWITS_LOG("OnBoarding: SoftAP or Web mode\n");
            }

            if(1 == status->ststus.types.station)
            {
                gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_AIRLINK;
                gizwitsProtocol.wifiStatusEvent.num++;
                GIZWITS_LOG("OnBoarding: AirLink mode\n");
            }
        }
        else
        {
            if(1 == status->ststus.types.softap)
            {
                gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_SOFTAP;
                gizwitsProtocol.wifiStatusEvent.num++;
                GIZWITS_LOG("OnBoarding: SoftAP or Web mode\n");
            }

            if(1 == status->ststus.types.station)
            {
                gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_STATION;
                gizwitsProtocol.wifiStatusEvent.num++;
                GIZWITS_LOG("OnBoarding: Station mode\n");
            }
        }
    }

    //binding mode status
    if(lastStatus.types.binding != status->ststus.types.binding)
    {
        lastStatus.types.binding = status->ststus.types.binding;
        if(1 == status->ststus.types.binding)
        {
            gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_OPEN_BINDING;
            gizwitsProtocol.wifiStatusEvent.num++;
            GIZWITS_LOG("WiFi status: in binding mode\n");
        }
        else
        {
            gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_CLOSE_BINDING;
            gizwitsProtocol.wifiStatusEvent.num++;
            GIZWITS_LOG("WiFi status: out binding mode\n");
        }
    }

    //router status
    if(lastStatus.types.con_route != status->ststus.types.con_route)
    {
        lastStatus.types.con_route = status->ststus.types.con_route;
        if(1 == status->ststus.types.con_route)
        {
            gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_CON_ROUTER;
            gizwitsProtocol.wifiStatusEvent.num++;
            GIZWITS_LOG("WiFi status: connected router\n");
        }
        else
        {
            gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_DISCON_ROUTER;
            gizwitsProtocol.wifiStatusEvent.num++;
            GIZWITS_LOG("WiFi status: disconnected router\n");
        }
    }

    //M2M server status
    if(lastStatus.types.con_m2m != status->ststus.types.con_m2m)
    {
        lastStatus.types.con_m2m = status->ststus.types.con_m2m;
        if(1 == status->ststus.types.con_m2m)
        {
            gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_CON_M2M;
            gizwitsProtocol.wifiStatusEvent.num++;
            GIZWITS_LOG("WiFi status: connected m2m\n");
        }
        else
        {
            gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_DISCON_M2M;
            gizwitsProtocol.wifiStatusEvent.num++;
            GIZWITS_LOG("WiFi status: disconnected m2m\n");
        }
    }

    //APP status
    if(lastStatus.types.app != status->ststus.types.app)
    {
        lastStatus.types.app = status->ststus.types.app;
        if(1 == status->ststus.types.app)
        {
            gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_CON_APP;
            gizwitsProtocol.wifiStatusEvent.num++;
            GIZWITS_LOG("WiFi status: app connect\n");
        }
        else
        {
            gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_DISCON_APP;
            gizwitsProtocol.wifiStatusEvent.num++;
            GIZWITS_LOG("WiFi status: no app connect\n");
        }
    }

    //test mode status
    if(lastStatus.types.test != status->ststus.types.test)
    {
        lastStatus.types.test = status->ststus.types.test;
        if(1 == status->ststus.types.test)
        {
            gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_OPEN_TESTMODE;
            gizwitsProtocol.wifiStatusEvent.num++;
            GIZWITS_LOG("WiFi status: in test mode\n");
        }
        else
        {
            gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_CLOSE_TESTMODE;
            gizwitsProtocol.wifiStatusEvent.num++;
            GIZWITS_LOG("WiFi status: out test mode\n");
        }
    }

    gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_RSSI;
    gizwitsProtocol.wifiStatusEvent.num++;
    gizwitsProtocol.wifiStatusData.rssi = status->ststus.types.rssi;
    GIZWITS_LOG("RSSI is %d \n", gizwitsProtocol.wifiStatusData.rssi);

    gizwitsProtocol.issuedFlag = WIFI_STATUS_TYPE;

    return 0;
}


/**@name Gizwits 用户API接口
* @{
*/

/**
* @brief gizwits协议初始化接口

* 用户调用该接口可以完成Gizwits协议相关初始化（包括协议相关定时器、串口的初始）

* 用户可在在此接口内完成数据点的初始化状态设置

* @param none
* @return none
*/
void gizwitsInit(void)
{    
    pRb.rbCapacity = RB_MAX_LEN;
    pRb.rbBuff = rbBuf;
    rbCreate(&pRb);
    
    memset((uint8_t *)&gizwitsProtocol, 0, sizeof(gizwitsProtocol_t));
}

/**
* @brief WiFi配置接口

* 用户可以调用该接口使WiFi模组进入相应的配置模式或者复位模组

* @param[in] mode 配置模式选择：0x0， 模组复位 ;0x01， SoftAp模式 ;0x02， AirLink模式 ;0x03， 产测模式模式; 0x04:允许用户绑定设备

* @return 错误命令码
*/
int32_t gizwitsSetMode(uint8_t mode)
{
    int32_t ret = 0;
    protocolCfgMode_t cfgMode;
    protocolCommon_t setDefault;

    switch(mode)
    {
        case WIFI_RESET_MODE:
            gizProtocolHeadInit((protocolHead_t *)&setDefault);
            setDefault.head.cmd = CMD_SET_DEFAULT;
            setDefault.head.sn = gizwitsProtocol.sn++;
            setDefault.head.len = gizProtocolExchangeBytes(sizeof(protocolCommon_t)-4);
            setDefault.sum = gizProtocolSum((uint8_t *)&setDefault, sizeof(protocolCommon_t));
            ret = uartWrite((uint8_t *)&setDefault, sizeof(protocolCommon_t));
            if(ret < 0)
            {
                GIZWITS_LOG("ERROR: uart write error %d \n", ret);
            }

            gizProtocolWaitAck((uint8_t *)&setDefault, sizeof(protocolCommon_t));   
            break;
        case WIFI_SOFTAP_MODE:
            gizProtocolHeadInit((protocolHead_t *)&cfgMode);
            cfgMode.head.cmd = CMD_WIFI_CONFIG;
            cfgMode.head.sn = gizwitsProtocol.sn++;
            cfgMode.cfgMode = mode;
            cfgMode.head.len = gizProtocolExchangeBytes(sizeof(protocolCfgMode_t)-4);
            cfgMode.sum = gizProtocolSum((uint8_t *)&cfgMode, sizeof(protocolCfgMode_t));
            ret = uartWrite((uint8_t *)&cfgMode, sizeof(protocolCfgMode_t));
            if(ret < 0)
            {
                GIZWITS_LOG("ERROR: uart write error %d \n", ret);
            }
            gizProtocolWaitAck((uint8_t *)&cfgMode, sizeof(protocolCfgMode_t)); 
            break;
        case WIFI_AIRLINK_MODE:
            gizProtocolHeadInit((protocolHead_t *)&cfgMode);
            cfgMode.head.cmd = CMD_WIFI_CONFIG;
            cfgMode.head.sn = gizwitsProtocol.sn++;
            cfgMode.cfgMode = mode;
            cfgMode.head.len = gizProtocolExchangeBytes(sizeof(protocolCfgMode_t)-4);
            cfgMode.sum = gizProtocolSum((uint8_t *)&cfgMode, sizeof(protocolCfgMode_t));
            ret = uartWrite((uint8_t *)&cfgMode, sizeof(protocolCfgMode_t));
            if(ret < 0)
            {
                GIZWITS_LOG("ERROR: uart write error %d \n", ret);
            }
            gizProtocolWaitAck((uint8_t *)&cfgMode, sizeof(protocolCfgMode_t)); 
            break;
        case WIFI_PRODUCTION_TEST:
            gizProtocolHeadInit((protocolHead_t *)&setDefault);
            setDefault.head.cmd = CMD_PRODUCTION_TEST;
            setDefault.head.sn = gizwitsProtocol.sn++;
            setDefault.head.len = gizProtocolExchangeBytes(sizeof(protocolCommon_t)-4);
            setDefault.sum = gizProtocolSum((uint8_t *)&setDefault, sizeof(protocolCommon_t));
            ret = uartWrite((uint8_t *)&setDefault, sizeof(protocolCommon_t));
            if(ret < 0)
            {
                GIZWITS_LOG("ERROR: uart write error %d \n", ret);
            }

            gizProtocolWaitAck((uint8_t *)&setDefault, sizeof(protocolCommon_t));
            break;
        case WIFI_NINABLE_MODE:
            gizProtocolHeadInit((protocolHead_t *)&setDefault);
            setDefault.head.cmd = CMD_NINABLE_MODE;
            setDefault.head.sn = gizwitsProtocol.sn++;
            setDefault.head.len = gizProtocolExchangeBytes(sizeof(protocolCommon_t)-4);
            setDefault.sum = gizProtocolSum((uint8_t *)&setDefault, sizeof(protocolCommon_t));
            ret = uartWrite((uint8_t *)&setDefault, sizeof(protocolCommon_t));
            if(ret < 0)
            {
                GIZWITS_LOG("ERROR: uart write error %d \n", ret);
            }

            gizProtocolWaitAck((uint8_t *)&setDefault, sizeof(protocolCommon_t)); 
            break;
        default:
            GIZWITS_LOG("ERROR: CfgMode error!\n");
            break;
    }

    return ret;
}

/**
* @brief 获取网络时间的接口

* 对应协议 4.13 MCU请求获取网络时间 中的 设备MCU发送

* @param[in] none
* @return none
*/
void gizwitsGetNTP(void)
{
    int32_t ret = 0;
    protocolCommon_t getNTP;

    gizProtocolHeadInit((protocolHead_t *)&getNTP);
    getNTP.head.cmd = CMD_GET_NTP;
    getNTP.head.sn = gizwitsProtocol.sn++;
    getNTP.head.len = gizProtocolExchangeBytes(sizeof(protocolCommon_t)-4);
    getNTP.sum = gizProtocolSum((uint8_t *)&getNTP, sizeof(protocolCommon_t));
    ret = uartWrite((uint8_t *)&getNTP, sizeof(protocolCommon_t));
    if(ret < 0)
    {
        GIZWITS_LOG("ERROR[NTP]: uart write error %d \n", ret);
    }

    gizProtocolWaitAck((uint8_t *)&getNTP, sizeof(protocolCommon_t));
}

/**
* @brief 协议处理函数

* 该函数中完成了相应协议数据的处理及数据主动上报的相关操作

* @param [in] currentData 待上报的协议数据指针
* @return none
*/
int32_t gizwitsHandle(dataPoint_t *currentData)
{
    int8_t ret = 0;
    uint16_t i = 0;
    uint8_t ackData[RB_MAX_LEN];
    uint16_t protocolLen = 0;
    uint32_t ackLen = 0;
    protocolHead_t *recvHead = NULL;

    if(NULL == currentData)
    {
        GIZWITS_LOG("GizwitsHandle Error , Illegal Param\n");
        return -1;
    }

    /*重发机制*/
    gizProtocolAckHandle();
    ret = gizProtocolGetOnePacket(&pRb, gizwitsProtocol.protocolBuf, &protocolLen);

    if(0 == ret)
    {
        GIZWITS_LOG("Get One Packet!\n");
        
#ifdef PROTOCOL_DEBUG
        GIZWITS_LOG("WiFi2MCU[%4d:%4d]: ", gizGetTimerCount(), protocolLen);
        for(i=0; i<protocolLen;i++)
        {
            GIZWITS_LOG("%02x ", gizwitsProtocol.protocolBuf[i]);
        }
        GIZWITS_LOG("\n");
#endif

        recvHead = (protocolHead_t *)gizwitsProtocol.protocolBuf;
        switch (recvHead->cmd)
        {
            case CMD_GET_DEVICE_INTO:
                gizProtocolGetDeviceInfo(recvHead);
                break;
            case CMD_ISSUED_P0:
                if(0 == gizProtocolIssuedProcess(gizwitsProtocol.protocolBuf, protocolLen, ackData, &ackLen))
                {
                    gizProtocolIssuedDataAck(recvHead, ackData, ackLen);
                }
                break;
            case CMD_HEARTBEAT:
                gizProtocolCommonAck(recvHead);
                break;
            case CMD_WIFISTATUS:
                gizProtocolCommonAck(recvHead);
                gizProtocolModuleStatus((protocolWifiStatus_t *)recvHead);
                break;
            case ACK_REPORT_P0:
            case ACK_WIFI_CONFIG:
            case ACK_SET_DEFAULT:
            case ACK_NINABLE_MODE:
                gizProtocolWaitAckCheck(recvHead);
                break;
            case CMD_MCU_REBOOT:
                gizProtocolCommonAck(recvHead);
                GIZWITS_LOG("report:MCU reboot!\n");
                
                gizProtocolReboot();
                break;
            case CMD_ERROR_PACKAGE:
                break;
            case ACK_PRODUCTION_TEST:
                gizProtocolWaitAckCheck(recvHead);
                GIZWITS_LOG("Ack PRODUCTION_MODE success \n");
                break;           
            case ACK_GET_NTP:
                gizProtocolWaitAckCheck(recvHead);
                gizProtocolNTP(recvHead);
                GIZWITS_LOG("Ack GET_UTT success \n");
                break;   
            default:
                gizProtocolErrorCmd(recvHead,ERROR_CMD);
                GIZWITS_LOG("ERROR: cmd code error!\n");
                break;
        }
    }
    else if(-2 == ret)
    {
        //校验失败，报告异常
        recvHead = (protocolHead_t *)gizwitsProtocol.protocolBuf;
        gizProtocolErrorCmd(recvHead,ERROR_ACK_SUM);
        GIZWITS_LOG("ERROR: check sum error!\n");
        return -2;
    }
    
    switch(gizwitsProtocol.issuedFlag)
    {
        case ACTION_CONTROL_TYPE:
            gizwitsProtocol.issuedFlag = STATELESS_TYPE;
            gizwitsEventProcess(&gizwitsProtocol.issuedProcessEvent, (uint8_t *)&gizwitsProtocol.gizCurrentDataPoint, sizeof(dataPoint_t));
            memset((uint8_t *)&gizwitsProtocol.issuedProcessEvent,0x0,sizeof(gizwitsProtocol.issuedProcessEvent));  
            break;
        case WIFI_STATUS_TYPE:
            gizwitsProtocol.issuedFlag = STATELESS_TYPE;
            gizwitsEventProcess(&gizwitsProtocol.wifiStatusEvent, (uint8_t *)&gizwitsProtocol.wifiStatusData, sizeof(moduleStatusInfo_t));
            memset((uint8_t *)&gizwitsProtocol.wifiStatusEvent,0x0,sizeof(gizwitsProtocol.wifiStatusEvent));
            break;
        case ACTION_W2D_TRANSPARENT_TYPE:
            gizwitsProtocol.issuedFlag = STATELESS_TYPE;
            gizwitsEventProcess(&gizwitsProtocol.issuedProcessEvent, (uint8_t *)gizwitsProtocol.transparentBuff, gizwitsProtocol.transparentLen);
            break;
        case GET_NTP_TYPE:
            gizwitsProtocol.issuedFlag = STATELESS_TYPE;
            gizwitsEventProcess(&gizwitsProtocol.NTPEvent, (uint8_t *)&gizwitsProtocol.TimeNTP, sizeof(protocolTime_t));
            memset((uint8_t *)&gizwitsProtocol.NTPEvent,0x0,sizeof(gizwitsProtocol.NTPEvent));
            break;      
    }

    gizDevReportPolicy(currentData);

    return 0;
}

/**
* @brief gizwits上报透传数据接口

* 用户调用该接口可以完成私有协议数据的上报

* @param [in] data 输入的私有协议数据
* @param [in] len 输入的私有协议数据长度
* @return -1 , 错误返回;0,正确返回
*/
int32_t gizwitsPassthroughData(uint8_t * data, uint32_t len)
{
	int32_t ret = 0;
	uint8_t tx_buf[MAX_PACKAGE_LEN];
    if(NULL == data)
    {
        GIZWITS_LOG("[Error] gizwitsPassthroughData Error \n");
        return (-1);
    }
	uint8_t *pTxBuf = tx_buf;
	uint16_t data_len = 6+len;
	*pTxBuf ++= 0xFF;
	*pTxBuf ++= 0xFF;
	*pTxBuf ++= (uint8_t)(data_len>>8);//len
	*pTxBuf ++= (uint8_t)(data_len);
	*pTxBuf ++= CMD_REPORT_P0;//0x1b cmd
	*pTxBuf ++= gizwitsProtocol.sn++;//sn
	*pTxBuf ++= 0x00;//flag
	*pTxBuf ++= 0x00;//flag
	*pTxBuf ++= ACTION_D2W_TRANSPARENT_DATA;//P0_Cmd

    memcpy(&tx_buf[9],data,len);
    tx_buf[data_len + 4 - 1 ] = gizProtocolSum( tx_buf , (data_len+4));
    
	ret = uartWrite(tx_buf, data_len+4);
    if(ret < 0)
    {
        //打印日志
        GIZWITS_LOG("ERROR: uart write error %d \n", ret);
    }

    gizProtocolWaitAck(tx_buf, data_len+4);

    return 0;
}
/**@} */


/**
 * htons unsigned short -> Network byte order
 * ntohs Network byte order -> unsigned short
 */
uint16_t ICACHE_FLASH_ATTR exchangeBytes(uint16_t value)
{
    uint16_t tmp_value = 0;
    uint8_t *index_1, *index_2;

    index_1 = (uint8_t *)&tmp_value;
    index_2 = (uint8_t *)&value;

    *index_1 = *(index_2+1);
    *(index_1+1) = *index_2;

    return tmp_value;
}
