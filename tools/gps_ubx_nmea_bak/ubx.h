#ifndef __UBX_H__
#define __UBX_H__

#define     MAX_BUFF_LEN            2048
#define     SEND_BUFF_LEN           64
#define     UBX_SIGNAL_PACK_LEN     44
#define     ACK_DATA_LEN            10

#define     UBX_PACK_POSLLH_LEN     (2 + 2 + 2 + 28 + 2)
#define     UBX_PACK_STATUS_LEN     (2 + 2 + 2 + 16 + 2)
#define     UBX_PACK_TIMEUTC_LEN    (2 + 2 + 2 + 20 + 2)
#define     UBX_PACK_VELNED_LEN     (2 + 2 + 2 + 36 + 2)

#define     UBX_HEADER_B5           0xB5
#define     UBX_HEADER_62           0x62


#define 	UBX_CLASS_RXM 		    0x02
#define 	UBX_CLASS_ACK		    0x05


#define 	UBX_CLASS_MON 		    0x0A
#define 	UBX_CLASS_AID 		    0x0B
#define 	UBX_CLASS_STD		    0xF0		/* Standard nema message id */

#define 	UBX_CLASS_NAV           0x01
#define     UBX_CLASS_NAV_POSLLH    0x02        /* lat-lon */
#define     UBX_CLASS_NAV_STATUS    0x03        /* fix type */
#define     UBX_CLASS_NAV_VELNED    0x12        /* 2d -speed */
#define     UBX_CLASS_NAV_TIMEUTC   0x21        /* utc time */

#define     UBX_CLASS_STD           0xF0
#define 	UBX_CLASS_STD_GPGGA     0x00
#define 	UBX_CLASS_STD_GPGLL     0x01
#define 	UBX_CLASS_STD_GPVTG     0x05
#define 	UBX_CLASS_STD_GPGSV     0x03
#define 	UBX_CLASS_STD_GPGSA     0x02
#define 	UBX_CLASS_STD_GPRMC     0x04

#define     UBX_CLASS_ACK           0x05
#define     UBX_CLASS_ACK_ACK       0x01
#define     UBX_CLASS_ACK_NAK       0x00

#define 	UBX_CLASS_CFG           0x06
#define 	UBX_CLASS_CFG_MSG       0x01
#define     UBX_CLASS_CFG_PRT       0x00


/* ubx time status */
#define     UBX_TOW             0x01
#define     UBX_WKN             0x02
#define     UBX_UTC             0x04

#define     POS_SCALE	        10000000    /* 1e7 */
#define     VEL_SCALE		    100         /* cm -> m */
#define     READ_UINT32(p)    ((*(p + 3) << 24) | (*(p + 2) << 16) | (*(p + 1) << 8) | (*(p)))

typedef struct _cmdRESPACKET
{
    int len;
    unsigned char buffer[ACK_DATA_LEN];
} cmdResPacket;

typedef struct _dataRESPACKET
{
    int len;
    unsigned char buffer[UBX_SIGNAL_PACK_LEN];
} dataResPacket;

typedef struct _ubxTIME
{
    int     year;       /**< Years since 1900 */
    int     mon;        /**< Months since January - [0,11] */
    int     day;        /**< Day of the month - [1,31] */
    int     hour;       /**< Hours since midnight - [0,23] */
    int     min;        /**< Minutes after the hour - [0,59] */
    int     sec;        /**< Seconds after the minute - [0,59] */
    int     hsec;       /**< Hundredth part of second - [0,99] */

} ubxTIME;

typedef struct _ubxDATA
{
    ubxTIME time;
    int timeValid;
    double lat;
    double lon;     /* Longitude, degrees, absolute value */
    int latlonValid;
    float speed2d; /* ground speed (2-D) m/s */
    unsigned char fixtype;
} ubxDATA;

typedef struct _Postion
{
    double lat;
    double lon;
    double speed;
    unsigned long time;
} Postion;

void time_to_datetime(unsigned long time, char *datatime);
unsigned long ubx_utc_local(ubxTIME *ubxTime, char *pDate);
void ubx_checksum(unsigned char *, unsigned char *, int);
int ubx_parse_pack(unsigned char *, int, ubxDATA *);
int ubx_parse_posllh(unsigned char *, ubxDATA *);
int ubx_parse_status(unsigned char *, ubxDATA *);
int ubx_parse_time(unsigned char *, ubxDATA *);
int ubx_parse_velned(unsigned char *, ubxDATA *);
void ubx_parse_result(ubxDATA *);

int ubx_cmd_res_part_content(unsigned char *, int);
int ubx_cmd_res_content(unsigned char *, int, int *);
int ubx_cmd_res_pack(unsigned char *, int *);
int ubx_msg_cfg(int, int, int, unsigned char, unsigned char);
int ubx_prt_cfg(int fd, unsigned char prtType);
void print_hex(unsigned char *buffer, int bufLen);

int ubx_data_pack(unsigned char *buffer, int *dataLen);
int ubx_data_content(unsigned char *buffer, int bufferLen, int *validLen);
int ubx_data_part_content(unsigned char *buffer, int dataLen);

#endif //__UBX_H__
