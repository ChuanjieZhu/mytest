
#include "ubx.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

extern char g_recvBuffer[MAX_BUFF_LEN];
extern char *g_pRecvHead;
extern char *g_pRecvTail;

extern ubxResPartPacket resPartPacket;
extern ubxDataPartPacket dataPartPacket;

static int packetFlag = 0;
unsigned char classId;
unsigned char messageId;

void print_hex(unsigned char *buffer, int bufLen)
{
    int i;
    printf("\r\n");
    for (i = 0; i < bufLen; i++)
    {
        printf("%02X ", buffer[i]);
    }
    printf("\r\n");
}

void ubx_checksum(unsigned char *sumValue, unsigned char *checkData, int dataLen)
{
    unsigned long a = 0x00;
    unsigned long b = 0x00;
    int i = 0;

    while(i < dataLen)
    {
        a += checkData[i++];
        b += a;
    }

    sumValue[0] = a & 0xff;
    sumValue[1] = b & 0xff;
}

void time_to_datetime(unsigned long time, char *datatime)
{
	if(datatime) 
    {
		struct tm pTm;
		time_t ti = time;
        
		if(localtime_r(&ti, &pTm) != NULL) 
        {
			sprintf(datatime, "%d%02d%02d_%02d%02d%02d", 
				pTm.tm_year+1900, pTm.tm_mon+1, pTm.tm_mday, 
				pTm.tm_hour, pTm.tm_min, pTm.tm_sec);
		}
	}
}

unsigned long ubx_utc_local(ubxTIME *ubxTime, char *pDate)
{
    struct tm tmTime, pTm;
    time_t localTime = 0;
    
    memset(&tmTime, 0, sizeof(struct tm));

    tmTime.tm_year = ubxTime->year;
    tmTime.tm_mon  = ubxTime->mon;
    tmTime.tm_mday = ubxTime->day;
    tmTime.tm_hour = ubxTime->hour;
    tmTime.tm_min  = ubxTime->min;
    tmTime.tm_sec  = ubxTime->sec;

    localTime = mktime(&tmTime) + (8 * 3600);
    
	if(localtime_r(&localTime, &pTm) != NULL) 
    {
		sprintf(pDate, "%d%02d%02d_%02d%02d%02d", 
			pTm.tm_year+1900, pTm.tm_mon+1, pTm.tm_mday, 
			pTm.tm_hour, pTm.tm_min, pTm.tm_sec);
	}
        
    return localTime;
}

int ubx_parse_pack(unsigned char *buffer, int dataLen, ubxDATA *packet)
{
    int ret = -1;
    unsigned char uclass = buffer[2];
    unsigned char id = buffer[3];

    if (uclass == UBX_CLASS_NAV)
    {
        if (id == UBX_CLASS_NAV_POSLLH)
        {
            ret = ubx_parse_posllh(&buffer[4], packet);
        }
        else if (id == UBX_CLASS_NAV_STATUS)
        {
            ret = ubx_parse_status(&buffer[4], packet);
        }
        else if (id == UBX_CLASS_NAV_TIMEUTC)
        {
            ret = ubx_parse_time(&buffer[4], packet);
        }
        else if (id == UBX_CLASS_NAV_VELNED)
        {
            ret = ubx_parse_velned(&buffer[4], packet);
        }
    }

    return ret;
}

int ubx_parse_posllh(unsigned char *buffer, ubxDATA *packet)
{
    unsigned short payload_len = (buffer[1] << 8) | buffer[0];

    //printf( "payload_len %d\r\n", payload_len);

    if (payload_len != 28)
    {
        printf( "nav posllh: bad length\r\n");
        packet->latlonValid = 0;
        return -1;
    }

    /* Longitude and Latitude */
    packet->lon = (double)READ_UINT32(buffer + 6) / POS_SCALE;
    packet->lat = (double)READ_UINT32(buffer + 10) / POS_SCALE;

    //printf("lon %lf, lat %lf\r\n", packet->lon, packet->lat);

    /* adjust the lon and lat value */
    if (packet->lon > 0 && packet->lat > 0)
    {
        packet->latlonValid = 1;
    }
    else
    {
        packet->latlonValid = 0;
    }

    return 0;
}

int ubx_parse_status(unsigned char *buffer, ubxDATA *packet)
{
    unsigned short payload_len = (buffer[1] << 8) | buffer[0];

    //printf("payload_len %d \r\n", payload_len);

    if (payload_len != 16)
    {
        printf("nav_status: bad length. \r\n");
        return -1;
    }

    /* 0x02 = 2D-fix,0x03 = 3D-fix */
    packet->fixtype = buffer[6];

    //printf("fixtype %d \r\n", packet->fixtype);

    return 0;
}

int ubx_parse_time(unsigned char *buffer, ubxDATA *packet)
{
    unsigned short payload_len = (buffer[1] << 8) | buffer[0];

    //printf("payload_len %d %s %d\r\n", payload_len, __FUNCTION__, __LINE__);;

    if (payload_len != 20)
    {
        printf("nav_timeutc: bad length.\r\n");
        packet->timeValid = 0;
        return -1;
    }

    unsigned char valid = buffer[21];
    //printf("valid %d, (valid & 4) %d \r\n", valid, (valid & 4));
    
    if ((valid & 4) != 4)
    {
        packet->timeValid = 0;
        return -1;
    }

    packet->time.year = ((buffer[15] << 8) | buffer[14]) - 1900;
    //printf( "year %d ", packet->time.year);
    packet->time.mon = buffer[16] - 1;
    //printf( "mon %d ", packet->time.mon);
    packet->time.day = buffer[17];
    //printf( "day %d ", packet->time.day);
    packet->time.hour = buffer[18];
    //printf( "hour %d ", packet->time.hour);
    packet->time.min = buffer[19];
    //printf( "min %d ", packet->time.min);
    packet->time.sec = buffer[20];
    //printf( "sec %d \r\n", packet->time.sec);

    packet->timeValid = 1;

    return 0;
}

int ubx_parse_velned(unsigned char *buffer, ubxDATA *packet)
{
    unsigned short payload_len = ((buffer[1] << 8) | buffer[0]);
    
    //printf("payload_len %d %s %d\r\n", payload_len, __FUNCTION__, __LINE__);

    if (payload_len != 36)
    {
        printf("nav_velned: bad length\r\n");
        return -1;
    }

    /* Ground Speed (2-D) m/s */
    packet->speed2d = (float)READ_UINT32(buffer + 22) / VEL_SCALE;
    //printf("speed2d: %f %s %d\r\n", packet->speed2d, __FUNCTION__, __LINE__);
    return 0;
}

int ubx_msg_cfg(int fd, int status1, int status2, unsigned char uclassId, unsigned char msgId)
{
    unsigned char sendBuf[SEND_BUFF_LEN] = {0};
    unsigned char sum[2] = {0};
    unsigned char len = 0;
    int i, ret = 0;

    sendBuf[len++] = UBX_HEADER_B5;				/* header a */
    sendBuf[len++] = UBX_HEADER_62;				/* header b */
    sendBuf[len++] = UBX_CLASS_CFG;              /* CFG class */
    sendBuf[len++] = UBX_CLASS_CFG_MSG;          /* MGS */

    classId = UBX_CLASS_CFG;
    messageId = UBX_CLASS_CFG_MSG;
    
    sendBuf[len++] = 0x08;						/* payload length */
    sendBuf[len++] = 0x00;

    sendBuf[len++] = uclassId;				/* nav class */
    sendBuf[len++] = msgId;

    for (i = 0; i < 5; i++)
    {
        if (i == 1)
        {
            sendBuf[len++] = status1;
        }
        else if (i == 2)
        {
            sendBuf[len++] = status2;
        }
        else
        {
            sendBuf[len++] = 0x00;
        }
    }

    sendBuf[len++] = 0x00;

    ubx_checksum(sum, &sendBuf[2], len - 2);

    /* add sum value */
    sendBuf[len++] = sum[0];
    sendBuf[len++] = sum[1];

    ret = write(fd, sendBuf, len);
    if (ret != len)
    {
        printf("send ubx msg command fail (%s). ret = %d \r\n", strerror(errno), ret);
        return -1;
    }

    return 0;
}

int ubx_prt_cfg(int fd, unsigned char prtType)
{
    unsigned char sendBuf[SEND_BUFF_LEN] = {0};
    unsigned char sum[2] = {0};
    unsigned char len = 0;
    int ret = 0;

    sendBuf[len++] = UBX_HEADER_B5;				/* header a */
    sendBuf[len++] = UBX_HEADER_62;				/* header b */
    sendBuf[len++] = UBX_CLASS_CFG;		/* msg class */
    sendBuf[len++] = UBX_CLASS_CFG_PRT;			/* class id */

    classId = UBX_CLASS_CFG;
    messageId = UBX_CLASS_CFG_PRT;
    
    sendBuf[len++] = 0x14;						/* payload length */
    sendBuf[len++] = 0x00;

    sendBuf[len++] = 0x01;						/* uart port id */
    sendBuf[len++] = 0x00;						/* reserved0 */

    sendBuf[len++] = 0x00;						/* txReady */
    sendBuf[len++] = 0x00;

    sendBuf[len++] = 0xD0;						/* mode */
    sendBuf[len++] = 0x08;
    sendBuf[len++] = 0x00;
    sendBuf[len++] = 0x00;

    sendBuf[len++] = 0x80;						/* baudRate */
    sendBuf[len++] = 0x25;
    sendBuf[len++] = 0x00;
    sendBuf[len++] = 0x00;

    sendBuf[len++] = 0x07;						/* inProtoMask */
    sendBuf[len++] = 0x00;

    sendBuf[len++] = prtType;				    /* outProtoMask 0x01-ubx, 0x02-nmea */
    sendBuf[len++] = 0x00;

    sendBuf[len++] = 0x00;						/* reserved4 */
    sendBuf[len++] = 0x00;

    sendBuf[len++] = 0x00;						/* reserved5 */
    sendBuf[len++] = 0x00;

    ubx_checksum(sum, &sendBuf[2], len - 2);

    sendBuf[len++] = sum[0];						/* check sum */
    sendBuf[len++] = sum[1];

    ret = write(fd, sendBuf, len);
    if (ret != len)
    {
        printf("send ubx print cfg command fail. ret=%d %s %d\r\n", ret, __FUNCTION__, __LINE__);
    }

    return 0;
}

int ubx_cmd_res_pack(unsigned char *buffer, int *dataLen)
{
    int offset = -1;
    int validLen = 0;
    int len = 0;
    int checkLen = 0;
    int checkOffset = 0;
    unsigned char checkBuf[MAX_BUFF_LEN];
    
    *dataLen = 0;
    memset(checkBuf, 0, sizeof(checkBuf));

    if (resPartPacket.len != 0)
    {
        checkOffset = resPartPacket.len;
        memcpy(checkBuf, resPartPacket.buffer, resPartPacket.len);
        memset(&resPartPacket, 0, sizeof(resPartPacket));
    }

    len = (int)g_pRecvHead - (int)g_pRecvTail;
    
    if (len > 0)
    {
        memcpy(checkBuf + checkOffset, g_pRecvTail, len);
        checkLen = checkOffset + len;
    }
    else
    {
        len = (int)g_pRecvTail - (int)g_recvBuffer;
        memcpy(checkBuf + checkOffset, g_pRecvTail, MAX_BUFF_LEN - len);
        checkLen = checkOffset + MAX_BUFF_LEN - len;
    }
    
    offset = ubx_cmd_res_content(checkBuf, checkLen, &validLen);
    
    if (offset != -1)
    {
        *dataLen = validLen;
        memcpy(buffer, checkBuf + offset, validLen);
        g_pRecvTail += (offset + validLen - checkOffset);
    }
    else if (offset == -1 && packetFlag == 1)
    {
        ubx_cmd_res_part_content(checkBuf, checkLen);
        g_pRecvTail += checkLen - checkOffset;
        packetFlag = 0;
    }
    else
    {
        if (checkLen - checkOffset >= 4)
        {
            g_pRecvTail += 4;
        }
        else if (g_pRecvTail + checkLen - checkOffset >= g_recvBuffer + MAX_BUFF_LEN)
        {
            g_pRecvTail = g_recvBuffer;
        }
        else if (checkBuf[0] == UBX_HEADER_B5)
        {
            memcpy(buffer, checkBuf, checkLen);
            *dataLen = checkLen;
            g_pRecvTail += checkLen - checkOffset;
        }
        else
        {
            g_pRecvTail = g_pRecvHead;
        }
    }

    if (g_pRecvTail >= g_recvBuffer + MAX_BUFF_LEN) 
    {
		g_pRecvTail = g_recvBuffer;
	}
    
    return offset;
}

int ubx_cmd_res_content(unsigned char *buffer, int dataLen, int *validLen)
{
    int i = 0;
    unsigned char checkSum[2];
    int offset = -1;
    packetFlag = 0;

    if (dataLen >= 4)
    {
        for (i = 0; i < dataLen; i++)
        {       
            if (buffer[i] == UBX_HEADER_B5 
                && buffer[i + 1] == UBX_HEADER_62
                && buffer[i + 2] == UBX_CLASS_ACK
                && (buffer[i + 3] == UBX_CLASS_ACK_ACK 
                    || buffer[i + 3] == UBX_CLASS_ACK_NAK)
                && buffer[i + 6] == classId
                    && buffer[i + 7] == messageId)
            {
                offset = i;
                
                ubx_checksum(checkSum, &buffer[offset + 2], ACK_DATA_LEN - 4);
                if (checkSum[0] == buffer[offset + 8]
                    && checkSum[1] == buffer[offset + 9])
                {
                    //printf("get a ack packet. %s %d\r\n", __FUNCTION__, __LINE__);
                    *validLen = ACK_DATA_LEN;
                    return offset;
                }
                else
                {
                    packetFlag = 1;
                    break;
                }
            }
        }
    }
    else
    {
        packetFlag = 1;
    }
    
    return -1;
}

int ubx_cmd_res_part_content(unsigned char *buffer, int checkLen)
{
    int i = 0, index;
    int offset = -1;

    if (checkLen - ACK_DATA_LEN > 0)
    {
        index = checkLen - ACK_DATA_LEN;
    }
    else
    {
        index = 0;
    }
        
    for (i = index; i < checkLen; i++)
    {
        if ((buffer[i] == UBX_HEADER_B5)
            && (buffer[i + 1] == UBX_HEADER_62))
        {
            offset = i;
            resPartPacket.len = checkLen - offset;
            if (resPartPacket.len > ACK_DATA_LEN)
            {
                resPartPacket.len = ACK_DATA_LEN;
            }
            
            memcpy(resPartPacket.buffer, buffer + offset, resPartPacket.len);

            return 0;
        }
    }

    return -1;
}

int ubx_data_pack(unsigned char *buffer, int *dataLen)
{
    int offset = -1;
    int validLen = 0;
    int len = 0;
    int checkLen = 0;
    int checkOffset = 0;
    unsigned char checkBuff[MAX_BUFF_LEN];

    *dataLen = 0;
    memset(checkBuff, 0, sizeof(checkBuff));

    if (dataPartPacket.len != 0)
    {
        checkOffset = dataPartPacket.len;
        memcpy(checkBuff, dataPartPacket.buffer, dataPartPacket.len);
        memset(&dataPartPacket, 0, sizeof(dataPartPacket));
    }

    len = (int)g_pRecvHead - (int)g_pRecvTail;
    if (len > 0)
    {
        memcpy(checkBuff + checkOffset, g_pRecvTail, len);
        checkLen = checkOffset + len;
    }
    else
    {
        len = (int)g_pRecvTail - (int)g_recvBuffer;
        memcpy(checkBuff + checkOffset, g_pRecvTail, MAX_BUFF_LEN - len);
        checkLen = checkOffset + MAX_BUFF_LEN - len;
    }

    offset = ubx_data_content(checkBuff, checkLen, &validLen);

    if (offset != -1)
    {
        memcpy(buffer, checkBuff + offset, validLen);
        *dataLen = validLen;
        g_pRecvTail += (offset + validLen - checkOffset);
    }
    else if (offset == -1 && packetFlag == 1)
    {
        ubx_data_part_content(checkBuff, checkLen);
        g_pRecvTail += checkLen - checkOffset;
        packetFlag = 0;
    }
    else
    {
        if (checkLen - checkOffset >= 4)
        {
            g_pRecvTail += 4;
        }
        else if (g_pRecvTail + checkLen - checkOffset >= g_recvBuffer + MAX_BUFF_LEN)
        {
            g_pRecvTail = g_recvBuffer;
        }
        else if (checkBuff[0] == UBX_HEADER_B5)
        {
            memcpy(buffer, checkBuff, checkLen);
            *dataLen = checkLen;
            g_pRecvTail += checkLen - checkOffset;
        }
        else
        {
            g_pRecvTail = g_pRecvHead;
        }
    }

    if (g_pRecvTail >= g_recvBuffer + MAX_BUFF_LEN)
    {
        g_pRecvTail = g_recvBuffer;
    }
    
    return offset;
}

int ubx_data_content(unsigned char *buffer, int dataLen, int *validLen)
{
    int i = 0;
    unsigned char checkSum[2];
    int offset = -1;
    packetFlag = 0;

    if (dataLen >= 4)
    {
        for (i = 0; i < dataLen; i++)
        {
            if (buffer[i] == UBX_HEADER_B5
                && buffer[i + 1] == UBX_HEADER_62)
            {
                offset = i;

                if (offset + 3 > dataLen)
                {
                    packetFlag = 1;
                    break;
                }

                if (buffer[offset + 2] == UBX_CLASS_NAV)
                {
                    if (buffer[offset + 3] == UBX_CLASS_NAV_POSLLH)
                    {
                        if (offset + UBX_PACK_POSLLH_LEN > dataLen)
                        {
                            packetFlag = 1;
                            break;
                        }

                        ubx_checksum(checkSum, &buffer[offset + 2], UBX_PACK_POSLLH_LEN - 4);
                        if (checkSum[0] == buffer[UBX_PACK_POSLLH_LEN - 2]
                                && checkSum[1] == buffer[UBX_PACK_POSLLH_LEN - 1])
                        {
                            //printf("get ubx posllh packet. \r\n");
                            *validLen = UBX_PACK_POSLLH_LEN;
                            return offset;
                        }
                    }
                    else if (buffer[offset + 3] == UBX_CLASS_NAV_STATUS)
                    {
                        if (offset + UBX_PACK_STATUS_LEN > dataLen)
                        {
                            packetFlag = 1;
                            break;
                        }

                        ubx_checksum(checkSum, &buffer[offset + 2], UBX_PACK_STATUS_LEN - 4);
                        if (checkSum[0] == buffer[UBX_PACK_STATUS_LEN - 2]
                                && checkSum[1] == buffer[UBX_PACK_STATUS_LEN - 1])
                        {
                            //printf("get ubx status packet. \r\n");
                            *validLen = UBX_PACK_STATUS_LEN;
                            return offset;
                        }
                    }
                    else if (buffer[offset + 3] == UBX_CLASS_NAV_TIMEUTC)
                    {
                        if (offset + UBX_PACK_TIMEUTC_LEN > dataLen)
                        {
                            packetFlag = 1;
                            break;
                        }

                        ubx_checksum(checkSum, &buffer[offset + 2], UBX_PACK_TIMEUTC_LEN - 4);
                        if (checkSum[0] == buffer[UBX_PACK_TIMEUTC_LEN - 2]
                                && checkSum[1] == buffer[UBX_PACK_TIMEUTC_LEN - 1])
                        {
                            //printf("get ubx time packet. \r\n");
                            *validLen = UBX_PACK_TIMEUTC_LEN;
                            return offset;
                        }
                    }
                    else if (buffer[offset + 3] == UBX_CLASS_NAV_VELNED)
                    {
                        if (offset + UBX_PACK_VELNED_LEN > dataLen)
                        {
                            packetFlag = 1;
                            break;
                        }

                        ubx_checksum(checkSum, &buffer[offset + 2], UBX_PACK_VELNED_LEN - 4);
                        if (checkSum[0] == buffer[UBX_PACK_VELNED_LEN - 2]
                                && checkSum[1] == buffer[UBX_PACK_VELNED_LEN - 1])
                        {
                            //printf("get ubx velned packet. \r\n");
                            *validLen = UBX_PACK_VELNED_LEN;
                            return offset;
                        }
                    }
                }
            }
        }
    }
    else
    {
        packetFlag = 1;
    }
    
    return -1;
}

int ubx_data_part_content(unsigned char *buffer, int dataLen)
{
    int i = 0;
    int offset = -1;
    int index = 0;
    
    if (dataLen - UBX_SIGNAL_PACK_LEN > 0)
    {
        index = dataLen - UBX_SIGNAL_PACK_LEN;
    }
    else
    {
        index = 0;
    }
    
    for (i = index; i < dataLen; i++)
    {
        if ((buffer[i] == UBX_HEADER_B5)
            && (buffer[i + 1] == UBX_HEADER_62))
        {
            offset = i;
            dataPartPacket.len = dataLen - offset;
            if (dataPartPacket.len > UBX_SIGNAL_PACK_LEN)
            {
                dataPartPacket.len = UBX_SIGNAL_PACK_LEN;
            }

            memcpy(dataPartPacket.buffer, buffer + offset, dataPartPacket.len);

            return 0;
        }
    }

    return -1;
}

