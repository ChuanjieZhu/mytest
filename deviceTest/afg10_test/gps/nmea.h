/********************************************************************************
**  Copyright (c) 2010, 深圳市飞瑞斯科技有限公司
**  All rights reserved.
**	
**  文件说明: afg10 GPS nmea格式数据解析接口
**  创建日期: 2014.02.28
**
**  当前版本：1.0
**  作者：
********************************************************************************/

#ifndef __NMEA_H__
#define __NMEA_H__

#define NMEA_BUFF_LEN        (2048)
#define PART_VALID_BUFF_LEN (80)
#define NMEA_DEF_PARSEBUFF  (2048)

#define NMEA_TOKS_COMPARE   (1)
#define NMEA_TOKS_PERCENT   (2)
#define NMEA_TOKS_WIDTH     (3)
#define NMEA_TOKS_TYPE      (4)

#define NMEA_CONVSTR_BUF    (256)
#define NMEA_TIMEPARSE_BUF  (256)

#define NMEA_HEAD_24        0x24        /* $ */
#define NMEA_HEAD_47        0x47        /* G */
#define NMEA_HEAD_50        0x50        /* p */
#define NMEA_BD_HEAD_4E     0x4E        /* N 北斗 */    
#define NMEA_HEAD_52        0x52        /* R */
#define NMEA_HEAD_4D        0x4D        /* M */
#define NMEA_HEAD_43        0x43        /* C */

#define NMEA_TAIL_2A        0x2A        /* * */
#define NMEA_TAIL_0D        0x0D        /* \r */
#define NMEA_TAIL_0A        0x0A        /* \n */

#define NMEA_TUD_KNOTS      (1.852)

enum nmeaPACKTYPE
{
    GPNON   = 0x0000,   /**< Unknown packet type. */
    GPGGA   = 0x0001,   /**< GGA - Essential fix data which provide 3D location and accuracy data. */
    GPGSA   = 0x0002,   /**< GSA - GPS receiver operating mode, SVs used for navigation, and DOP values. */
    GPGSV   = 0x0004,   /**< GSV - Number of SVs in view, PRN numbers, elevation, azimuth & SNR values. */
    GPRMC   = 0x0008,   /**< RMC - Recommended Minimum Specific GPS/TRANSIT Data. */
    GPVTG   = 0x0010    /**< VTG - Actual track made good and speed over ground. */
};

typedef struct _nmeaPartPacket
{
    long len;
    unsigned char buffer[PART_VALID_BUFF_LEN];
} nmeaPartPacket;

typedef struct _nmeaTIME
{
    int     year;       /**< Years since 1900 */
    int     mon;        /**< Months since January - [0,11] */
    int     day;        /**< Day of the month - [1,31] */
    int     hour;       /**< Hours since midnight - [0,23] */
    int     min;        /**< Minutes after the hour - [0,59] */
    int     sec;        /**< Seconds after the minute - [0,59] */
    int     hsec;       /**< Hundredth part of second - [0,99] */

} nmeaTIME;

typedef struct _nmeaGPRMC
{
    nmeaTIME utc;       /**< UTC of position */
    char    status;     /**< Status (A = active or V = void) */
    double  lat;        /**< Latitude in NDEG - [degree][min].[sec/60] */
    char    ns;         /**< [N]orth or [S]outh */
    double  lon;        /**< Longitude in NDEG - [degree][min].[sec/60] */
    char    ew;         /**< [E]ast or [W]est */
    double  speed;      /**< Speed over the ground in knots */
    double  direction;  /**< Track angle in degrees True */
    double  declination; /**< Magnetic variation degrees (Easterly var. subtracts from true course) */
    char    declin_ew;  /**< [E]ast or [W]est */
    char    mode;       /**< Mode indicator of fix type (A = autonomous, D = differential, E = estimated, N = not valid, S = simulator) */

} nmeaGPRMC;

typedef struct _nmeaPARSER
{
    unsigned char buffer[NMEA_DEF_PARSEBUFF];
    int buff_size;
    int buff_use;
} nmeaPARSER;

#ifdef __cplusplus
extern "C" {
#endif

int nmea_parse_time(const char *buff, int buff_sz, nmeaTIME *res);
int nmea_atoi(const char *str, int str_sz, int radix);
int nmea_scanf(const char *buff, int buff_sz, const char *format, ...);
double nmea_atof(const char *str, int str_sz);
int nmea_parse_gprmc(const char *, int , nmeaGPRMC *);
int nmea_parse_gnrmc(const char *buff, int buff_sz, nmeaGPRMC *pack);
int nmea_checksum(const char *, int, int *);
int nmea_parse_result(nmeaGPRMC *, double *lat, double *lon, double *speed, unsigned long *ultime);
long nmea_utc_local(nmeaTIME *);
void nmea_dms_degree(nmeaGPRMC *, double *, double *);
int nmea_parse_packet(unsigned char *validBuffer, int *validDataLen);
int nmea_parse_content(unsigned char *checkBuffer, int bufferLen, int *validLen);
int nmea_parse_part_content(unsigned char *checkBuffer, int checkLen);

#ifdef __cplusplus
}
#endif

#endif //__NMEA_H__

