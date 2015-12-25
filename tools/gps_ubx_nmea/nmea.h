
#ifndef __NMEA_H__
#define __NMEA_H__

#define MAX_BUFF_LEN        (2048)
#define PART_VALID_BUFF_LEN (80)
#define NMEA_DEF_PARSEBUFF  (2048)

#define NMEA_TOKS_COMPARE   (1)
#define NMEA_TOKS_PERCENT   (2)
#define NMEA_TOKS_WIDTH     (3)
#define NMEA_TOKS_TYPE      (4)

#define NMEA_CONVSTR_BUF    (256)
#define NMEA_TIMEPARSE_BUF  (256)

#define NMEA_HEAD_24        0x24
#define NMEA_HEAD_47        0x47
#define NMEA_HEAD_50        0x50
#define NMEA_HEAD_52        0x52
#define NMEA_HEAD_4D        0x4D
#define NMEA_HEAD_43        0x43

#define NMEA_TAIL_2A        0x2A
#define NMEA_TAIL_0D        0x0D
#define NMEA_TAIL_0A        0x0A

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

int nmea_packet(nmeaPARSER *parser, nmeaGPRMC *pack);
int getNmeaPacket(unsigned char *buff, int *dataLen);
int checkNmeaPackContent(unsigned char *, int, int *);
int getPartValidNmeaPackContent(unsigned char *buff, int buffSize);
int nmea_parse_time(const char *buff, int buff_sz, nmeaTIME *res);
int nmea_atoi(const char *str, int str_sz, int radix);
int nmea_scanf(const char *buff, int buff_sz, const char *format, ...);
double nmea_atof(const char *str, int str_sz);
int nmea_parse_gprmc(const char *, int , nmeaGPRMC *);
int nmea_checksum(const char *, int, int *);

void nmea_parse_result(nmeaGPRMC *);
long nmea_utc_local(nmeaTIME *);
void nmea_dms_degree(nmeaGPRMC *, double *, double *);

void nmea_parse_init(nmeaPARSER *parser);
int nmea_parser_push(nmeaPARSER *parser, const char *buff, int buff_sz, nmeaGPRMC *pack);
int nmea_parser_real_push(nmeaPARSER *parser, const char *buff, int buff_sz, nmeaGPRMC *pack);

int nmea_parse_packet(unsigned char *validBuffer, int *validDataLen);
int nmea_parse_content(unsigned char *checkBuffer, int bufferLen, int *validLen);
int nmea_parse_part_content(unsigned char *checkBuffer, int checkLen);
#endif //__NMEA_H__