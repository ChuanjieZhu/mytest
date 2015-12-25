/********************************************************************************
**  Copyright (c) 2010, 深圳市飞瑞斯科技有限公司
**  All rights reserved.
**	
**  文件说明: afg10 GPS/北斗 nmea格式数据解析接口
**  创建日期: 2014.02.28
**
**  当前版本：1.0
**  作者：
********************************************************************************/

#include "nmea.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>

#define     MIN_TIME_STAMP          (1388534400)
#define     RECV_BUFF_LEN           (2048)

extern char g_recvBuffer[RECV_BUFF_LEN];
extern char *g_pRecvHead;
extern char *g_pRecvTail;
extern nmeaPartPacket nmeaPacket;

static int g_packFlag = 0;

void nmea_print_hex(unsigned char *buffer, int bufLen)
{
    int i;
    printf("\r\n");
    for (i = 0; i < bufLen; i++)
    {
        printf("%02X ", buffer[i]);
    }
    printf("\r\n");
}

/*****************************************************************
函数名称：nmea_parse_result
函数功能: 获取GPS解析结果
输入参数: pack: GPS解析结果
          lat: 纬度
          lon: 经度
          speed: 速度
          ultime: 时间
输出参数: 无
返回值  ：0-GPS定位有效, -1-GPS定位无效
*****************************************************************/
int nmea_parse_result(nmeaGPRMC *pack, double *lat, double *lon, 
                           double *speed, unsigned long *ultime)
{
    int ret = -1;
    double degree_lat;
    double degree_lon;
    time_t localTime = 0;
    struct tm *pTm;
    char time_buff[32] = {0};
    
    localTime = nmea_utc_local(&(pack->utc));
    pTm = localtime(&localTime);

    sprintf(time_buff, "%04d%02d%02d_%02d%02d%02d",
            pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday,
            pTm->tm_hour, pTm->tm_min, pTm->tm_sec);

    nmea_dms_degree(pack, &degree_lat, &degree_lon);

    printf("%s, %lf, %lf, %lf, %s \r\n", 
        (pack->status == 'A' ? "A" : "V"), degree_lat, degree_lon, 
        (pack->speed * NMEA_TUD_KNOTS), time_buff);

    if (pack->status == 'A'
        && degree_lat > 0
        && degree_lon > 0
        && localTime > MIN_TIME_STAMP
        && pack->speed >= 0)
    {
        *lat = degree_lat;
        *lon = degree_lon;
        *speed = (pack->speed * NMEA_TUD_KNOTS);
        *ultime = localTime;
        ret = 0;
    }
    else
    {
        pack->status = 'V';
        ret = -1;
    }

    return ret;
}

/*****************************************************************
函数名称：nmea_atoi
函数功能: 将指定宽度字符串转换成对应进制整数
输入参数: str: 待转换字符串
          str_sz: 转换长度
          radix: 转换进制 
输出参数: 无
返回值  ：0-成功, 其它-失败
*****************************************************************/
int nmea_atoi(const char *str, int str_sz, int radix)
{
    char *tmp_ptr;
    char buff[NMEA_TIMEPARSE_BUF];
    int res = 0;

    if(str_sz < NMEA_TIMEPARSE_BUF)
    {
        memcpy(&buff[0], str, str_sz);
        buff[str_sz] = '\0';
        res = strtol(&buff[0], &tmp_ptr, radix);
    }

    return res;
}

/*****************************************************************
函数名称：nmea_atof
函数功能: 将指定字符串转换成对应浮点数
输入参数: str: 待转换字符串
          str_sz: 转换宽度
输出参数: 无
返回值  ：0-成功, 其它-失败
*****************************************************************/
double nmea_atof(const char *str, int str_sz)
{
    char *tmp_ptr;
    char buff[NMEA_CONVSTR_BUF];
    double res = 0;

    if(str_sz < NMEA_CONVSTR_BUF)
    {
        memcpy(&buff[0], str, str_sz);
        buff[str_sz] = '\0';
        res = strtod(&buff[0], &tmp_ptr);
    }

    return res;
}

/*****************************************************************
函数名称：nmea_scanf
函数功能: 将指定字符串转换成对应浮点数
输入参数: buff: 待转换字符串
          buff_sz: 字符串长度
          format: 格式化参数
输出参数: 无
返回值  ：格式化结果数据数量
*****************************************************************/
int nmea_scanf(const char *buff, int buff_sz, const char *format, ...)
{
    const char *beg_tok;
    const char *end_buf = buff + buff_sz;

    va_list arg_ptr;
    int tok_type = NMEA_TOKS_COMPARE;
    int width = 0;
    const char *beg_fmt = 0;
    int snum = 0, unum = 0;

    int tok_count = 0;
    void *parg_target;

    va_start(arg_ptr, format);

    for(; *format && buff < end_buf; ++format)
    {
        switch(tok_type)
        {
        case NMEA_TOKS_COMPARE:
            if('%' == *format)
                tok_type = NMEA_TOKS_PERCENT;
            else if(*buff++ != *format)
                goto fail;
            break;
        case NMEA_TOKS_PERCENT:
            width = 0;
            beg_fmt = format;
            tok_type = NMEA_TOKS_WIDTH;
        case NMEA_TOKS_WIDTH:
            if(isdigit(*format))
                break;
            {
                tok_type = NMEA_TOKS_TYPE;
                if(format > beg_fmt)
                    width = nmea_atoi(beg_fmt, (int)(format - beg_fmt), 10);
            }
        case NMEA_TOKS_TYPE:
            beg_tok = buff;

            if(!width && ('c' == *format || 'C' == *format) && *buff != format[1])
                width = 1;

            if(width)
            {
                if(buff + width <= end_buf)
                    buff += width;
                else
                    goto fail;
            }
            else
            {
                if(!format[1] || (0 == (buff = (char *)memchr(buff, format[1], end_buf - buff))))
                    buff = end_buf;
            }

            if(buff > end_buf)
                goto fail;

            tok_type = NMEA_TOKS_COMPARE;
            tok_count++;

            parg_target = 0; width = (int)(buff - beg_tok);

            switch(*format)
            {
            case 'c':
            case 'C':
                parg_target = (void *)va_arg(arg_ptr, char *);
                if(width && 0 != (parg_target))
                    *((char *)parg_target) = *beg_tok;
                break;
            case 's':
            case 'S':
                parg_target = (void *)va_arg(arg_ptr, char *);
                if(width && 0 != (parg_target))
                {
                    memcpy(parg_target, beg_tok, width);
                    ((char *)parg_target)[width] = '\0';
                }
                break;
            case 'f':
            case 'g':
            case 'G':
            case 'e':
            case 'E':
                parg_target = (void *)va_arg(arg_ptr, double *);
                if(width && 0 != (parg_target))
                    *((double *)parg_target) = nmea_atof(beg_tok, width);
                break;
            };

            if(parg_target)
                break;
            if(0 == (parg_target = (void *)va_arg(arg_ptr, int *)))
                break;
            if(!width)
                break;

            switch(*format)
            {
            case 'd':
            case 'i':
                snum = nmea_atoi(beg_tok, width, 10);
                memcpy(parg_target, &snum, sizeof(int));
                break;
            case 'u':
                unum = nmea_atoi(beg_tok, width, 10);
                memcpy(parg_target, &unum, sizeof(unsigned int));
                break;
            case 'x':
            case 'X':
                unum = nmea_atoi(beg_tok, width, 16);
                memcpy(parg_target, &unum, sizeof(unsigned int));
                break;
            case 'o':
                unum = nmea_atoi(beg_tok, width, 8);
                memcpy(parg_target, &unum, sizeof(unsigned int));
                break;
            default:
                goto fail;
            };

            break;
        };
    }

fail:

    va_end(arg_ptr);

    return tok_count;
}

/*****************************************************************
函数名称：nmea_checksum
函数功能: NMEA GPS数据校验值计算
输入参数: buff: 待校验字符串
          buff_sz: 字符串长度
输出参数: res_crc: 校验结果
返回值  ：0-校验错误，其它-校验成功
*****************************************************************/
int nmea_checksum(const char *buff, int buff_sz, int *res_crc)
{
    static const int tail_sz = 3 /* *[CRC] */ + 2 /* \r\n */;

    const char *end_buff = buff + buff_sz;
    int nread = 0;
    int crc = 0;

    *res_crc = -1;

    for(;buff < end_buff; ++buff, ++nread)
    {
        if(('$' == *buff) && nread)
        {
            buff = 0;
            break;
        }
        else if('*' == *buff)
        {
            if(buff + tail_sz <= end_buff && '\r' == buff[3] && '\n' == buff[4])
            {
                *res_crc = nmea_atoi(buff + 1, 2, 16);
                nread = buff_sz - (int)(end_buff - (buff + tail_sz));
                if(*res_crc != crc)
                {
                    *res_crc = -1;
                    buff = 0;
                }
            }

            break;
        }
        else if(nread)
        {
            crc ^= (int)*buff;
        }
    }

    if(*res_crc < 0 && buff)
    {
        nread = 0;
    }

    return nread;
}

/*****************************************************************
函数名称：nmea_parse_time
函数功能: NMEA GPS时间解析
输入参数: buff: 待解析字符串
          buff_sz: 字符串长度
输出参数: res: 时间解析结果
返回值  ：0-成功，其它-失败
*****************************************************************/
int nmea_parse_time(const char *buff, int buff_sz, nmeaTIME *res)
{
    int success = 0;

    switch(buff_sz)
    {
    case sizeof("hhmmss") - 1:
        success = (3 == nmea_scanf(buff, buff_sz,
            "%2d%2d%2d", &(res->hour), &(res->min), &(res->sec)
            ));
        break;
    case sizeof("hhmmss.s") - 1:
    case sizeof("hhmmss.ss") - 1:
    case sizeof("hhmmss.sss") - 1:
        success = (4 == nmea_scanf(buff, buff_sz,
            "%2d%2d%2d.%d", &(res->hour), &(res->min), &(res->sec), &(res->hsec)
            ));
        break;
    default:
        printf("Parse of time error (format error)! \r\n");
        success = 0;
        break;
    }

    return (success ? 0 : -1);
}

/*****************************************************************
函数名称：nmea_utc_local
函数功能: NMEA GPS UTC时间转换为本地时间戳
输入参数: nmeaTime: GPS nmea格式时间
输出参数: 无
返回值  ：转换后的本地时间戳
*****************************************************************/
time_t nmea_utc_local(nmeaTIME *nmeaTime)
{
    struct tm tmTime;
    time_t localTime = 0;

    memset(&tmTime, 0, sizeof(struct tm));

    tmTime.tm_year = nmeaTime->year;
    tmTime.tm_mon  = nmeaTime->mon;
    tmTime.tm_mday = nmeaTime->day;
    tmTime.tm_hour = nmeaTime->hour;
    tmTime.tm_min  = nmeaTime->min;
    tmTime.tm_sec  = nmeaTime->sec;

    localTime = mktime(&tmTime) + (8 * 3600);

    return localTime;
}

/*****************************************************************
函数名称：nmea_dms_degree
函数功能: NMEA GPS 时分秒格式经纬度转换为度格式
输入参数: packet: GPS 数据
输出参数: lat: 度格式纬度
          lon: 度格式经度
返回值  ：无
*****************************************************************/
void nmea_dms_degree(nmeaGPRMC *packet, double *lat, double *lon)
{
    int degreeLat = 0;
    int degreeLon = 0;

    double degreeLat2 = 0.0;
    double degreeLon2 = 0.0;

    degreeLat = (packet->lat) / 100;
    degreeLon = (packet->lon) / 100;

    degreeLat2 = (packet->lat) - (degreeLat * 100);
    degreeLon2 = (packet->lon) - (degreeLon * 100);

    *lat = (double)degreeLat + (degreeLat2 / 60);
    *lon = (double)degreeLon + (degreeLon2 / 60);
}

/*****************************************************************
函数名称：nmea_parse_gprmc
函数功能: 解析gps GPRMC 数据
输入参数: buff: 待解析数据缓存
          buff_sz: 数据长度
输出参数: pack: 解析结果
返回值  ：0-成功，-1-失败
*****************************************************************/
int nmea_parse_gprmc(const char *buff, int buff_sz, nmeaGPRMC *pack)
{
    int nsen;
    char time_buff[NMEA_TIMEPARSE_BUF];

    nsen = nmea_scanf(buff, buff_sz,
        "$GPRMC,%s,%C,%f,%C,%f,%C,%f,%f,%2d%2d%2d,%f,%C,%C*",
        &(time_buff[0]),
        &(pack->status), &(pack->lat), &(pack->ns), &(pack->lon), &(pack->ew),
        &(pack->speed), &(pack->direction),
        &(pack->utc.day), &(pack->utc.mon), &(pack->utc.year),
        &(pack->declination), &(pack->declin_ew), &(pack->mode));

    //printf("nmea_scanf return %d \r\n", nsen);

    if(nsen != 13 && nsen != 14)
    {
        //printf("GPRMC parse error! nsen: %d %s %d\r\n", nsen, __FUNCTION__, __LINE__);
        return -1;
    }

    if(0 != nmea_parse_time(&time_buff[0], (int)strlen(&time_buff[0]), &(pack->utc)))
    {
        //printf("GPRMC time parse error!\r\n");
        return -1;
    }
    
    if(pack->utc.year < 90)
        pack->utc.year += 100;
    pack->utc.mon -= 1;
    
    return 0;
}

/*****************************************************************
函数名称：nmea_parse_gnrmc
函数功能: 解析北斗 GNRMC 数据
输入参数: buff: 待解析数据缓存
          buff_sz: 数据长度
输出参数: pack: 解析结果
返回值  ：0-成功，-1-失败
*****************************************************************/
int nmea_parse_gnrmc(const char *buff, int buff_sz, nmeaGPRMC *pack)
{
    int nsen;
    char time_buff[NMEA_TIMEPARSE_BUF];

    nsen = nmea_scanf(buff, buff_sz,
        "$GNRMC,%s,%C,%f,%C,%f,%C,%f,%f,%2d%2d%2d,%f,%C,%C*",
        &(time_buff[0]),
        &(pack->status), &(pack->lat), &(pack->ns), &(pack->lon), &(pack->ew),
        &(pack->speed), &(pack->direction),
        &(pack->utc.day), &(pack->utc.mon), &(pack->utc.year),
        &(pack->declination), &(pack->declin_ew), &(pack->mode));

    //printf("nmea_scanf return %d \r\n", nsen);

    if(nsen != 13 && nsen != 14)
    {
        //printf("GNRMC parse error! nsen: %d %s %d\r\n", nsen, __FUNCTION__, __LINE__);
        return -1;
    }

    if(0 != nmea_parse_time(&time_buff[0], (int)strlen(&time_buff[0]), &(pack->utc)))
    {
        //printf("GNRMC time parse error!\r\n");
        return -1;
    }
    
    if(pack->utc.year < 90)
        pack->utc.year += 100;
    pack->utc.mon -= 1;
    
    return 0;
}

/*****************************************************************
函数名称：nmea_parse_packet
函数功能: 从串口数据中解析出GPRMC gps数据
输入参数: buff: 待解析数据缓存
          dataLen: 数据长度
输出参数: 无 
返回值  ：大于0-成功解析到GPRMC，其它-未解析到GPRMC数据
*****************************************************************/
int nmea_parse_packet(unsigned char *buffer, int *dataLen)
{
    int offset = -1;
    int validLen = 0;
    unsigned char checkBuffer[NMEA_BUFF_LEN + PART_VALID_BUFF_LEN];
    int len = 0;
    int checkLen = 0;
    int checkOffset = 0;

    memset(checkBuffer, 0, sizeof(checkBuffer));

    if (nmeaPacket.len != 0)
    {
        checkOffset = nmeaPacket.len;
        memcpy(checkBuffer, nmeaPacket.buffer, nmeaPacket.len);
        memset(&nmeaPacket, 0, sizeof(nmeaPacket));
    }

    len = (int)g_pRecvHead - (int)g_pRecvTail;
    
    if (len >= 0)
    {
        memcpy(checkBuffer + checkOffset, g_pRecvTail, len);
        checkLen = checkOffset + len;
    }
    else
    {
        len = (int)g_pRecvTail - (int)g_recvBuffer;
        memcpy(checkBuffer + checkOffset, g_pRecvTail, RECV_BUFF_LEN - len);
        checkLen = checkOffset + RECV_BUFF_LEN - len;
    }

    offset = nmea_parse_content(checkBuffer, checkLen, &validLen);    
    
    if (offset != -1)
    {
        *dataLen = validLen;
        memcpy(buffer, checkBuffer + offset, validLen);
        g_pRecvTail += (offset + validLen - checkOffset);
    }
    else if (offset == -1 && g_packFlag == 1)
    {
        nmea_parse_part_content(checkBuffer, checkLen);
        g_pRecvTail += checkLen - checkOffset;
        g_packFlag = 0;
    }
    else
    {
        if (checkLen - checkOffset >= 6)
        {
            g_pRecvTail += 6;
        }
        else if (g_pRecvTail + checkLen - checkOffset >= g_recvBuffer + RECV_BUFF_LEN)
        {
            g_pRecvTail = g_recvBuffer;
        }
        else if (checkBuffer[0] == NMEA_HEAD_24)
        {
            memcpy(buffer, checkBuffer, checkLen);
            *dataLen = checkLen;
            g_pRecvTail += checkLen - checkOffset;
        }
        else
        {
            g_pRecvTail = g_pRecvHead;
        }
    }

    if (g_pRecvTail >= g_recvBuffer + RECV_BUFF_LEN) 
    {
		g_pRecvTail = g_recvBuffer;
	}
    
    return offset;
}

/*****************************************************************
函数名称：nmea_parse_content
函数功能: 从串口数据中搜寻GPRMC数据部分
输入参数: buff: 待解析数据缓存
          dataLen: 数据长度
输出参数: validLen: 找到的GPRMC数据有效长度
返回值  ：大于0-成功找到GPRMC数据部分，其它-未解析到GPRMC数据部分
*****************************************************************/
int nmea_parse_content(unsigned char *buffer, int dataLen, int *validLen)
{
    int i = 0, j;
    int offset = -1;
    g_packFlag = 0;

    *validLen = 0;

    if (dataLen >= 6)
    {
        for (i = 0; i < dataLen; i++)
        {           
            if (buffer[i] == NMEA_HEAD_24           
                && buffer[i + 1] == NMEA_HEAD_47
                && (buffer[i + 2] == NMEA_HEAD_50
                    || buffer[i + 2] == NMEA_BD_HEAD_4E)
                && buffer[i + 3] == NMEA_HEAD_52
                && buffer[i + 4] == NMEA_HEAD_4D
                && buffer[i + 5] == NMEA_HEAD_43)
            {
                offset = i;
                for (j = offset + 6; j < dataLen; j++)
                {
                    if (buffer[j] == NMEA_TAIL_2A
                        && buffer[j + 3] == NMEA_TAIL_0D
                        && buffer[j + 4] == NMEA_TAIL_0A)
                    {
                        *validLen = j + 4 - offset + 1;
                        return offset;
                    }
                }

                /* 如果数据中有$GPRMC头，但是没有结束字符，则需设置分包标识 */
                g_packFlag = 1;
                break;
            }
        }
    }
    else
    {
        g_packFlag = 1;
    }
    
    return -1;
}

/*****************************************************************
函数名称：nmea_parse_part_content
函数功能: 从串口数据中搜寻部分GPRMC数据
输入参数: checkBuffer: 待解析数据缓存
          checkLen: 数据长度
输出参数: 
返回值  ：0-解析数据中含有部分GPRMC数据，其它-数据中无GPRMC有效数据
*****************************************************************/
int nmea_parse_part_content(unsigned char *checkBuffer, int checkLen)
{
    int i = 0;
    int offset = -1;
    
    for (i = 0; i < checkLen; i++)
    {
        if ((checkBuffer[i] == NMEA_HEAD_24)
            && (checkBuffer[i + 1] == NMEA_HEAD_47)
            && (checkBuffer[i + 2] == NMEA_HEAD_50
                || checkBuffer[i + 2] == NMEA_BD_HEAD_4E)
            && (checkBuffer[i + 3] == NMEA_HEAD_52)
            && (checkBuffer[i + 4] == NMEA_HEAD_4D)
            && (checkBuffer[i + 5] == NMEA_HEAD_43))
        {
            offset = i;
            nmeaPacket.len = checkLen - offset;
            if (nmeaPacket.len > PART_VALID_BUFF_LEN)
            {
                nmeaPacket.len = PART_VALID_BUFF_LEN;
            }
            memcpy(nmeaPacket.buffer, checkBuffer + offset, nmeaPacket.len);
            return 0;
        }
    }

    return -1;
}



