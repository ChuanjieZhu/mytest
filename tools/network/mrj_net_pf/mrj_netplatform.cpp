
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "cJSON.h"
#include "mrj_json.h"
#include "mrj_netplatform.h"

#define PLATFORMDEBUG

#define TEST_SN "1031412011727"
#define TEST_VERSION "1.2.1"
#define TEST_NAME "MRJ_TEST"
#define TEST_IP "192.168.0.188"

static struct AuthResponse g_authResponse;
static struct LoginResponse g_loginResponse;     // 存储登陆后获取的令牌
static struct PcResponse g_pcResponse; // 与平台通讯的配置信息
static struct SevenResponse g_sevenResponse;
static struct VideoResponse g_videoResponse;
static struct UpgradeResponse g_upgradeResponse;
static struct BindResponse g_bindResponse;
static struct DownloadResponse g_downloadResponse;

long g_dnsFlag1 = 1L;         /* DNS查询标识，0-从本机缓存获取dns解析结果，1-从dns服务器查询解析结果 */
long g_dnsFlag2 = 1L;

static char *get_domain()
{
    return g_authResponse.domain;
}

static char* get_token()
{
    return g_loginResponse.token;
}

static int set_login_response(char *tokenp, char *ltimep, char *ctimep)
{
    if (!tokenp || !ltimep || !ctimep)
        return (-1);

    TRACE("%s, %s, %s %s %d\r\n", tokenp, ltimep, ctimep, MDL);
    
    memset(&g_loginResponse, 0, sizeof(g_loginResponse));
    strncpy(g_loginResponse.token, tokenp, sizeof(g_loginResponse.token) - 1);
    strncpy(g_loginResponse.last_reptime, ltimep, sizeof(g_loginResponse.last_reptime) - 1);
    strncpy(g_loginResponse.cur_time, ctimep, sizeof(g_loginResponse.cur_time) - 1);

    return (0);
}

MRJEcode transfer_domain2ip(const char *domainp, char *resp, size_t reslen, int flag)
{
    MRJEcode res = MRJE_UNKNOWN;

#if 0
    if (!domainp || !resp || !reslen)
        return MRJE_BAD_FUNCTION_ARGUMENT;

    if (deal_dns_resolve(domainp, resp, reslen, flag) == 0) {
        
        TRACE("Transfer domain[%s] to ip[%s] success. %s %d\r\n", domainp, resp, MDL);
        res = MRJE_OK;
    } else {
    
        TRACE("Transfer domain[%s] fail. %s %d\r\n", domainp, MDL);
        res = MRJE_DNS_RESLOVE_ERROR;
    }
#endif

    return res;
}   

static int get_pc_response(struct PcResponse *responsep)
{
    if (!responsep)
        return (-1);

    memset(responsep, 0, sizeof(*responsep));
    memcpy(responsep, &g_pcResponse, sizeof(struct PcResponse));

    return (0);
}


static int get_seven_response(struct SevenResponse *responsep)
{
    if (!responsep)
        return (-1);

    memset(responsep, 0, sizeof(*responsep));
    memcpy(responsep, &g_sevenResponse, sizeof(struct SevenResponse));

    return (0);
}


static int get_video_response(struct VideoResponse *responsep)
{
    if (!responsep)
        return (-1);

    memset(responsep, 0, sizeof(*responsep));
    memcpy(responsep, &g_videoResponse, sizeof(struct VideoResponse));

    return (0);
}

#if 0
static void analyze_pc_response(struct PcResponse *resp)
{
    int iRet = 0;
    int iRoiLeft = 0;
    int iRoiTop = 0;
    int iRoiRight = 0;
    int iRoiBottom = 0;
    int iOriEnter = 0;
    int iHeight = 0;
    int iChannel = 0;
    int iInit = 0;
    
    CONFIG_PC_STR strConfigPC;

    TRACE("---------------------------------------------------------- %s %d\r\n", __FUNCTION__, __LINE__);
    TRACE("state        - %d \r\n", resp->result);
    TRACE("configId     - %d \r\n", resp->configId);
    TRACE("leftPoint    - %d \r\n", resp->leftPoint);
    TRACE("topPoint     - %d \r\n", resp->topPoint);
    TRACE("rightPoint   - %d \r\n", resp->rightPoint);
    TRACE("bottomPoint  - %d \r\n", resp->bottomPoint);
    TRACE("direction    - %d \r\n", resp->direction);
    TRACE("height       - %d \r\n", resp->height);
    TRACE("init         - %d \r\n", resp->init);
    TRACE("---------------------------------------------------------- %s %d\r\n", __FUNCTION__, __LINE__);

    memset(&strConfigPC, 0, sizeof(strConfigPC));
    GetConfigPC(&strConfigPC);

    iInit = resp->init;
            
    if (1 == iInit) { 
        iRoiLeft    = SYS_CONFIG_DEFAULT_LEFTPOINT;
        iRoiTop     = SYS_CONFIG_DEFAULT_TOPPOING;
        iRoiRight   = SYS_CONFIG_DEFAULT_RIGHTPOINT;
        iRoiBottom  = SYS_CONFIG_DEFAULT_BOTTOMPOINT;
        iOriEnter   = SYS_CONFIG_DEFAULT_ORIENTER;

        strConfigPC.nRoiLeft = iRoiLeft;
        strConfigPC.nRoiTop = iRoiTop;
        strConfigPC.nRoiRight = iRoiRight;
        strConfigPC.nRoiBottom = iRoiBottom;
        strConfigPC.nOriEnter = iOriEnter;

        iRet = SetPcDetectArea(iChannel, iRoiLeft, iRoiTop, iRoiRight, iRoiBottom);
        if (iRet != 0) {
            TRACE("SetPcDetectArea Error! iRet = %d %s %d\r\n", iRet, __FUNCTION__, __LINE__);
        }

        if (iRet == 0) {
            SetConfigPC(&strConfigPC);
            g_pcResponse.result      = 1;                    /* ?? */
            g_pcResponse.init        = resp->init; 
            g_pcResponse.configId    = resp->configId;         
            g_pcResponse.leftPoint   = iRoiLeft;
            g_pcResponse.topPoint    = iRoiTop; 
            g_pcResponse.rightPoint  = iRoiRight; 
            g_pcResponse.bottomPoint = iRoiBottom;
            g_pcResponse.direction   = iOriEnter;  
            g_pcResponse.height      = resp->height;
        } else {
            g_pcResponse.result      = 2;                    /* ?? */
            g_pcResponse.init        = resp->init; 
            g_pcResponse.configId    = resp->configId;         
            g_pcResponse.leftPoint   = resp->leftPoint;
            g_pcResponse.topPoint    = resp->topPoint; 
            g_pcResponse.rightPoint  = resp->rightPoint; 
            g_pcResponse.bottomPoint = resp->bottomPoint;
            g_pcResponse.direction   = resp->direction;
            g_pcResponse.height      = resp->height;
        }
    } else {
        iRoiLeft = resp->leftPoint;
        iRoiTop = resp->topPoint;
        iRoiRight = resp->rightPoint;
        iRoiBottom = resp->bottomPoint;

        iHeight = resp->height;
        iOriEnter = resp->direction;
        
        if (iRoiLeft < 0 || iRoiLeft >= CIF_WIDTH) {
            iRet = -1;
        } else if (iRoiTop < 0 || iRoiTop >= CIF_HEIGHT) {
            iRet = -1;
        } else if (iRoiRight < 0 || iRoiRight >= CIF_WIDTH) {
            iRet = -1;
        } else if (iRoiBottom < 0 || iRoiBottom >= CIF_HEIGHT) {
            iRet = -1;
        }

        TRACE("left: %d, top: %d, right: %d, bottom: %d %s %d\r\n", 
                iRoiLeft, iRoiTop, iRoiRight, iRoiBottom, __FUNCTION__, __LINE__);
        
        if (iRet == 0) {
            iRoiLeft = iRoiLeft - (iRoiLeft * 2) / 3;
            iRoiRight = iRoiRight + ((CIF_WIDTH - iRoiRight) * 2) / 3;
            iRoiTop = iRoiTop - (iRoiTop * 2) / 3;
            iRoiBottom = iRoiBottom + ((CIF_HEIGHT - iRoiBottom) * 2) / 3;
        
            strConfigPC.nRoiLeft = iRoiLeft;
            strConfigPC.nRoiTop = iRoiTop;
            strConfigPC.nRoiRight = iRoiRight;
            strConfigPC.nRoiBottom = iRoiBottom;
        } else {
            TRACE("AnalysePlatformConfig Param Error! %s %d\r\n", __FUNCTION__, __LINE__);
        }        

        if (iRet == 0) {
            iRet = SetPcDetectArea(iChannel, iRoiLeft, iRoiTop, iRoiRight, iRoiBottom);
            if (iRet != 0) {
                TRACE("SetPcDetectArea Error! iRet = %d %s %d\r\n", iRet, __FUNCTION__, __LINE__);
            }
        }

        if (iRet == 0) {
            SetConfigPC(&strConfigPC);
            g_pcResponse.result         = 1;        /* ?? */
            g_pcResponse.init           = resp->init;         
            g_pcResponse.configId       = resp->configId;
            g_pcResponse.leftPoint      = iRoiLeft;
            g_pcResponse.rightPoint     = iRoiRight;
            g_pcResponse.topPoint       = iRoiTop;
            g_pcResponse.bottomPoint    = iRoiBottom;
            g_pcResponse.direction      = iOriEnter;
            g_pcResponse.height         = iHeight;
        } else {
            g_pcResponse.result         = 2;                    /* ?? */
            g_pcResponse.init           = resp->init; 
            g_pcResponse.configId       = resp->configId;         
            g_pcResponse.leftPoint      = resp->leftPoint;
            g_pcResponse.topPoint       = resp->topPoint; 
            g_pcResponse.rightPoint     = resp->rightPoint; 
            g_pcResponse.bottomPoint    = resp->bottomPoint;
            g_pcResponse.direction      = resp->direction;
            g_pcResponse.height         = resp->height;
        }
    }
}

void analyze_seven_response(struct SevenResponse *resp)
{
    int initDefault = 0;
    CONFIG_SEVEN_STR strCfgSeven;

    memset(&strCfgSeven, 0, sizeof(strCfgSeven));
    GetConfigSeven(&strCfgSeven);
 
    initDefault = resp->track_init_defalt;

    /* ??seven?????? */
    if (initDefault == 1)
    {   
        strCfgSeven.nTrackingMaxLostFrame       = 20;
        strCfgSeven.nTrackingMaxDistance        = 65;
        strCfgSeven.nTrackingMaxFrame           = 3;
        strCfgSeven.nTrackingGoodTrackingCount  = 4;
        strCfgSeven.nTrackingMinDisLen          = 30;
        strCfgSeven.nTrackingMinAngle           = 45;
        strCfgSeven.nTrackingDisCardOpenFlag    = 1;
        strCfgSeven.nTrackingDisCardMaxDistance = 45;
    }
    else    /* ??seven?? */
    {
        strCfgSeven.nTrackingMaxLostFrame       = resp->max_lost_frame;
        strCfgSeven.nTrackingMaxDistance        = resp->max_distance;
        strCfgSeven.nTrackingMaxFrame           = resp->max_frame_cnt;
        strCfgSeven.nTrackingGoodTrackingCount  = resp->good_track_cnt;
        strCfgSeven.nTrackingMinDisLen          = resp->min_dis_len;
        strCfgSeven.nTrackingMinAngle           = resp->min_angle;

        if (resp->discard_open_flag == 0)
        {
            strCfgSeven.nTrackingDisCardOpenFlag = 0;        /* 0-?? */
        }
        else
        {
            strCfgSeven.nTrackingDisCardOpenFlag = 1;       /* 1-?? */
        }
        
        strCfgSeven.nTrackingDisCardMaxDistance = resp->discard_max_distance;
    }

    SetConfigSeven(&strCfgSeven);
    
    g_sevenResponse.result           = 1;
    g_sevenResponse.config_id            = resp->config_id;
    g_sevenResponse.max_lost_frame       = strCfgSeven.nTrackingMaxLostFrame;
    g_sevenResponse.max_distance         = strCfgSeven.nTrackingMaxDistance;
    g_sevenResponse.max_frame_cnt        = strCfgSeven.nTrackingMaxFrame;
    g_sevenResponse.good_track_cnt       = strCfgSeven.nTrackingGoodTrackingCount;
    g_sevenResponse.min_dis_len          = strCfgSeven.nTrackingMinDisLen;
    g_sevenResponse.min_angle            = strCfgSeven.nTrackingMinAngle;
    g_sevenResponse.discard_open_flag    = strCfgSeven.nTrackingDisCardOpenFlag;
    g_sevenResponse.discard_max_distance = strCfgSeven.nTrackingDisCardMaxDistance;
}

void Analyze_video_response(struct VideoResponse *resp)
{
    int iRet = 0;
    int channel = 0;
    int initValue = 0;
    int brightness = 0;
    int contrast = 0;
    int hue = 0;
    int saturation = 0;
    int exposure = 0;

    initValue = resp->init;
   
    if (initValue == 1) {
        brightness = 128;
        contrast   = 128;
        hue        = 0;
        saturation = 128;
        exposure   = 0;
    } else {
        brightness = resp->brightness;
        contrast   = resp->contrast;
        hue        = resp->hue;
        saturation = resp->saturation;
        exposure   = resp->exposure;
    }

    if (iRet == 0) 
        iRet = SetVideoBrightness(channel, brightness);
	if (iRet == 0) 
        iRet = SetVideoContrast(channel, contrast); 
	if (iRet == 0) 
        iRet = SetVideoHue(channel, hue);  
	if (iRet == 0) 
        iRet = SetVideoSaturation(channel, saturation);    
	if (iRet == 0) 
        iRet = SetVideoExposure(channel, exposure);

    if (iRet == 0)
        g_videoResponse.result = 1;         /* ok */
    else
        g_videoResponse.result = 2;         /* failure */

    g_videoResponse.configId    = resp->configId;
    g_videoResponse.brightness  = brightness;
    g_videoResponse.contrast    = contrast;
    g_videoResponse.hue         = hue;
    g_videoResponse.saturation  = saturation;
    g_videoResponse.exposure    = exposure;
}
#endif

const char *get_sn()
{
    return TEST_SN;
}

const char *get_version()
{
    return TEST_VERSION;    
}

const char *get_name()
{
    return TEST_NAME;
}

const char *get_ip()
{
    return TEST_IP;
}

int CopyFileToBuf(char *buf, int n, char *file)
{
	FILE *fp = NULL;
	int retVal = -1;
	unsigned int len = 0;

	if (buf != NULL && n > 0 && file != NULL)
	{
		len = (unsigned int)n;

		if ((fp = fopen(file, "r")) != NULL)
		{
			if (fread(buf, sizeof(char), len, fp) == len)
			{
				retVal = 0;
			}

			fclose(fp);
		}
	}

	return retVal;
}

int base64_encode(char *dest, const char *src, int count)
{ 
	const unsigned char Base64_EnCoding[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int len = 0; 
	unsigned char *pt = (unsigned char *)src;

	if( count < 0 )
	{
		while( *pt++ )
		{ 
			count++; 
		}
		
		pt = (unsigned char *)src;
	}
	
	if( !count )
	{
		return 0;
	}
	
	while( count > 0 )
	{
		*dest++ = Base64_EnCoding[ ( pt[0] >> 2 ) & 0x3f];
		if( count > 2 )
		{
			*dest++ = Base64_EnCoding[((pt[0] & 3) << 4) | (pt[1] >> 4)];
			*dest++ = Base64_EnCoding[((pt[1] & 0xF) << 2) | (pt[2] >> 6)];
			*dest++ = Base64_EnCoding[ pt[2] & 0x3F];
		}
		else
		{
			switch( count )
			{
				case 1:
				{
					*dest++ = Base64_EnCoding[(pt[0] & 3) << 4 ];
					*dest++ = '=';
					*dest++ = '=';
					
					break;
				}
				case 2: 
				{
					*dest++ = Base64_EnCoding[((pt[0] & 3) << 4) | (pt[1] >> 4)]; 
					*dest++ = Base64_EnCoding[((pt[1] & 0x0F) << 2) | (pt[2] >> 6)]; 
					*dest++ = '='; 

					break;
				}
			} 
		} 
		
		pt += 3; 
		count -= 3; 
		len += 4; 
	} 
	
	*dest = 0; 

	return len; 
}

char *create_content(void *arg)
{
    int *lenp = (arg) ? (int *)arg : NULL;
    int len = 0;
    int size = MRJ_MAX_CONTENT;
    
    char *contentp = (char *)malloc(size);
    if (!contentp)
        return NULL;

    memset(contentp, 0, size);
    len = snprintf(contentp, size - 1, 
            "imei=%s&token=%s", get_sn(), get_token());

    if (lenp) {
        *lenp = len;
    }

    return contentp;
}

char *create_auth_content(void *arg)
{
    int *lenp = (arg != NULL) ? (int *)arg : NULL;
    int len = 0;
    int size = MRJ_MAX_CONTENT;
    
    char *contentp = (char *)malloc(size);
    if (contentp == NULL)
    {
        return NULL;
    }
    
    memset(contentp, 0, size);
    len = snprintf(contentp, size - 1, 
            "imei=%s&version=%s", get_sn(), get_version());

    if (lenp != NULL) 
    {
        *lenp = len;
    }

    return contentp;    
}

char *create_report_photo_content(void *arg)
{
    char *reqp = (arg) ? (char *)arg : NULL;
    struct stat st;
    int filesize = 0;
    int encodesize = 0;
    int base64len = 0;
    
    if (stat(reqp, &st) != 0) {
        TRACE("%s not exist! %s %d\r\n", reqp, MDL);
        return NULL;
    }

    filesize = st.st_size;
    TRACE("Report file size: %d %s %d\r\n", filesize, MDL);
    
    char *filep = (char *)malloc(filesize);
    if (!filep) {
        TRACE("malloc copy file buffer fail. %s %d\r\n", MDL);
        return NULL;
    }
    
    memset(filep, 0, filesize);
    if (0 != CopyFileToBuf(filep, filesize, reqp))
    {
        TRACE("Copy file to buffer fail! %s %d\r\n", MDL);
        free(filep);
        return NULL;
    }

    encodesize = filesize * 2;
    char *encodep = (char *)malloc(encodesize);
    if (!encodep) {
        TRACE("malloc base64 encode buffer fail. %s %s %d\r\n", STR_ERRNO, MDL);
        free(filep);
        return NULL;
    }

    memset(encodep, 0, encodesize);
    memcpy(encodep, "data=", 5);
    base64len = base64_encode(encodep + 5, filep, filesize);

    free(filep);

    return encodep;
}

char *create_login_json_string(void *arg)
{ 
    int *lenp = (arg) ? (int *)arg : NULL;
    int n = 0;
    char acName[32] = {0};
    char acNameBase64[128] = {0};
   
    strncpy(acName, get_name(), sizeof(acName) - 1);

    TRACE("strSN: %s, ip: %s, acName: %s %s %d\r\n", get_sn(), get_ip(), acName, MDL);
    
    base64_encode(acNameBase64, acName, strlen(acName));

#if 0    
    contMap.insert(pair<string, string>("imei", get_sn()));
    contMap.insert(pair<string, string>("ip", get_ip()));
    contMap.insert(pair<string, string>("alias", acNameBase64));
        
    map_element_to_send(contMap, content);

    size_t len = strlen("data=") + 1;
    len += content.length();
#endif

    char *contentp = create_login_json_object(get_sn(), get_ip(), acNameBase64);
    if (contentp == NULL)
    {
        TRACE("> create login json objects fail. %s %d\r\n", MDL);
        return NULL;
    }

    size_t len = strlen("data=") + 2;
    len += strlen(contentp);
    
    TRACE("len: %d %s %d\r\n", len, MDL);

    char *dptr = (char *)malloc(len);
    if (!dptr) 
    {
        TRACE("malloc error. %s %d\r\n", MDL);
        free(contentp);
        return NULL;
    }

    memset(dptr, 0, len);
    n = snprintf(dptr, len - 1, "data=%s", contentp);
    if (lenp)
    {
        *lenp = n;
    }

    free(contentp);
    
    return dptr;
}

char *create_report_rcd_json_string(void *arg)
{   
    struct ReportRcdReq *reqp = (arg) ? (struct ReportRcdReq *)arg : NULL;
    size_t len = 0;
    int i;

    if (!reqp)
        return NULL;

    char *contentp = create_report_rcd_json_object(reqp);
    if (contentp == NULL)
    {
        TRACE("> create record json object failed. %s %d\r\n", MDL);
        return NULL;
    }
    
    len = strlen("data=") + 2;
    len += strlen(contentp);

    TRACE("len: %d %s %d\r\n", len, MDL);

    char *dptr = (char *)malloc(len);
    if (!dptr)
    {
        free(contentp);
        return NULL;
    }
    
    memset(dptr, 0, len);
    reqp->reqdatalen = snprintf(dptr, len - 1, "data=%s", contentp);
    
    free(contentp);
    
    return dptr;
}

#if 0
char* create_report_app_pc_json_string(void *arg)
{
    int *lenp = (arg) ? (int *)arg : NULL;
    int n = 0;
    size_t len = 0;

    struct algo_obj_item item;
    memset(&item, 0, sizeof(item));
    
    char *contentp = create_report_algorithm_json_object(&item);
    if (contentp == NULL)
    {
        TRACE("> create report algo json object failed. %s %d\r\n", MDL);
        return NULL;
    }

    len = strlen("data=") + 2;
    len += strlen(contentp);

    TRACE("len: %d %s %d\r\n", len, MDL);

    char *dptr = (char *)malloc(len);
    if (!dptr)
    {
        free(contentp);
        return NULL;
    }
    
    memset(dptr, 0, len);
    n = snprintf(dptr, len - 1, "data=%s", contentp);

    if (lenp)
        *lenp = n;

    free(contentp);
        
    return dptr;
}


char *create_report_pc_json_string(void *arg)
{
    int *lenp = (arg) ? (int *)arg : NULL;
    int n;
    struct PcResponse response = {0};

    if (get_pc_response(&response) != 0)
        return NULL;

    TRACE("----------------------------------------------------------\r\n");
    TRACE("state        - %d \r\n", response.result);
    TRACE("configId     - %d \r\n", response.configId);
    TRACE("leftPoint    - %d \r\n", response.leftPoint);
    TRACE("topPoint     - %d \r\n", response.topPoint);
    TRACE("rightPoint   - %d \r\n", response.rightPoint);
    TRACE("bottomPoint  - %d \r\n", response.bottomPoint);
    TRACE("direction    - %d \r\n", response.direction);
    TRACE("height       - %d \r\n", response.height);
    TRACE("init         - %d \r\n", response.init);
    TRACE("----------------------------------------------------------\r\n");
    
    struct algo_obj_item item;
    memset(&item, 0, sizeof(item));
    
    char *contentp = create_report_algorithm_json_object(&item);
    if (contentp == NULL)
    {
        TRACE("> create report algo json object failed. %s %d\r\n", MDL);
        return NULL;
    }

    size_t len = strlen("data=") + 2;
    len += strlen(contentp);

    TRACE("len: %d %s %d\r\n", len, MDL);
    
    char *dptr = (char *)malloc(len);
    if (!dptr)
    {
        free(contentp);
        return NULL;
    }
    
    memset(dptr, 0, len);
    n = snprintf(dptr, len - 1, "data=%s", contentp);
    if (lenp)
        *lenp = n;

    free(contentp);
    
    return dptr;
}


char *create_report_seven_json_string(void *arg)
{   
    int *lenp = (arg) ? (int *)arg : NULL;
    int n = 0;
    size_t len = 0;
    Value index;
    string content;
    struct SevenResponse response;

    if (get_seven_response(&response) != 0)
        return NULL;

    TRACE("----------------------------------------------------------\r\n");
    TRACE("config_id            - %d \r\n", response.config_id);
    TRACE("result           - %d \r\n", response.result);
    TRACE("max_lost_frame       - %d \r\n", response.max_lost_frame);
    TRACE("max_distance         - %d \r\n", response.max_distance);
    TRACE("max_frame_cnt        - %d \r\n", response.max_frame_cnt);
    TRACE("good_track_cnt       - %d \r\n", response.good_track_cnt);
    TRACE("min_dis_len          - %d \r\n", response.min_dis_len);
    TRACE("min_angle            - %d \r\n", response.min_angle);
    TRACE("discard_open_flag    - %d \r\n", response.discard_open_flag);
    TRACE("discard_max_distance - %d \r\n", response.discard_max_distance);
    TRACE("----------------------------------------------------------\r\n");

    index["configId"]           = response.config_id;
    index["state"]              = response.result;
    index["maxLostFrame"]       = response.max_lost_frame;
    index["maxDistance"]        = response.max_distance;
    index["maxFrame"]           = response.max_frame_cnt;
    index["goodTrackingCount"]  = response.good_track_cnt;
    index["minDisLen"]          = response.min_dis_len;
    index["minAngle"]           = response.min_angle;
    index["disCardOpenFlag"]    = response.discard_open_flag;
    index["disCardMaxDistance"] = response.discard_max_distance;

    Json::FastWriter writer;
    content = writer.write(index);

    len = strlen("data=") + 1;
    len += content.length();

    char *dptr = (char *)malloc(len);
    if (!dptr)
        return NULL;

    memset(dptr, 0, len);
    n = snprintf(dptr, len - 1, "data=%s", content.c_str());
    if (lenp)
        *lenp = n;

    return dptr;
}


char *create_report_video_json_string(void *arg)
{
    int *lenp = (arg) ? (int *)arg : NULL;
    int n;
    size_t len = 0;
    string content;
    Value index;
    struct VideoResponse response;
    

    if (get_video_response(&response) != 0)
        return NULL;

    TRACE("----------------------------------------------------------\r\n");
    TRACE("configId   - %d \r\n", response.configId);
    TRACE("state      - %d \r\n", response.result);
    TRACE("brightness - %d \r\n", response.brightness);
    TRACE("contrast   - %d \r\n", response.contrast);
    TRACE("hue        - %d \r\n", response.hue);
    TRACE("saturation - %d \r\n", response.saturation);
    TRACE("exposure   - %d \r\n", response.exposure);
    TRACE("----------------------------------------------------------\r\n");

    index["configId"]   = response.configId;
    index["state"]      = response.result;
    index["brightness"] = response.brightness;
    index["contrast"]   = response.contrast;
    index["hue"]        = response.hue;
    index["saturation"] = response.saturation;
    index["exposure"]   = response.exposure;

    Json::FastWriter writer;
    content = writer.write(index);

    len = strlen("data=") + 1;
    len += content.length();

    TRACE("len: %d %s %d\r\n", len, MDL);

    char *dptr = (char *)malloc(len);
    if (!dptr)
        return NULL;

    memset(dptr, 0, len);
    n = snprintf(dptr, len - 1, "data=%s", content.c_str());
    if (lenp)
        *lenp = n;

    return dptr;
}

char* create_report_upgrade_json_string(void *arg)
{   
    int *lenp = (arg) ? (int *)arg : NULL;
    int n;
    size_t len = 0;
    string content;
    Value index;

    TRACE("strSN: %s, version: %s %s %d\r\n", get_sn(), get_version(), __FUNCTION__, __LINE__);
    
    index["imei"]       = Value(get_sn());
    index["token"]      = Value(get_token());
    index["version"]    = Value(get_version());

    Json::FastWriter writer;
    content = writer.write(index);

    len = strlen("data=") + 1;
    len += content.length();

    TRACE("len: %d %s %d\r\n", len, MDL);

    char *dptr = (char *)malloc(len);
    if (!dptr)
        return NULL;

    memset(dptr, 0, len);
    n = snprintf(dptr, len - 1, "data=%s", content.c_str());
    if (lenp)
        *lenp = n;

    return dptr;
}

char* create_query_upgrade_json_string(void *arg)
{
    int *lenp = (arg) ? (int *)arg : NULL;
    int n;
    size_t len = 0;
    Value index;
    string content;

    TRACE("strSN: %s, version: %s. %s %d \r\n", get_sn(), get_version(), __FUNCTION__, __LINE__);
    
    index["type"]    = Value("DVS");
    index["version"] = Value(get_version());
    index["imei"]    = Value(get_sn());
    index["token"]   = Value(get_token());

    Json::FastWriter writer;
    content = writer.write(index);

    len = strlen("data=") + 1;
    len += content.length();

    TRACE("len: %d %s %d\r\n", len, MDL);

    char *dptr = (char *)malloc(len);
    if (!dptr)
        return NULL;

    memset(dptr, 0, len);
    n = snprintf(dptr, len - 1, "data=%s", content.c_str());
    if (lenp)
        *lenp = n;

    return dptr;
}

char * create_report_name_json_string(void *arg)
{
    int *lenp = (arg) ? (int *)arg : NULL;
    int n;
    size_t len = 0;
    string content;
    char acName[32] = {0};
    char acNameBase64[128] = {0};
    map<string, string> contMap;

    TRACE("strSN: %s, DevName: %s %s %d\r\n", get_sn(), get_name(), __FUNCTION__, __LINE__);
    
    base64_encode(acNameBase64, get_name(), strlen(get_name()));
    
    contMap.insert(pair<string, string>("imei", get_sn()));
    contMap.insert(pair<string, string>("alias", acNameBase64));
    contMap.insert(pair<string, string>("token", get_token()));
        
    map_element_to_send(contMap, content);

    len = strlen("data=") + 1;
    len += content.length();

    TRACE("len: %d %s %d\r\n", len, MDL);

    char *dptr = (char *)malloc(len);
    if (!dptr)
        return NULL;

    memset(dptr, 0, len);
    n = snprintf(dptr, len - 1, "data=%s", content.c_str());
    if (lenp)
        *lenp = n;

    return dptr;
}

#endif



static size_t parse_response_auth(void *buffer, size_t size, size_t nmemb, void *stream)
{
    struct AuthResponse *resp = (struct AuthResponse *)stream;
    size_t datasize = 0;
    int pfcode = 0;
    char *bufp = (char *)buffer;
    cJSON *json = NULL;
    cJSON *json_value = NULL;

    datasize = size * nmemb;
    bufp[datasize] = '\0';
    
    TRACE("* %s %s %d\r\n", bufp, MDL);

    if (datasize <= 0) 
    {   
        TRACE("> Receive data error, datasize(%d). %s %d\r\n", datasize, MDL);
        resp->result = -1;
        goto done;
    }

    json = cJSON_Parse(bufp);
    if (json == NULL)
    {
        TRACE("Error before: [%s]\n",cJSON_GetErrorPtr());
        goto done;
    }

    json_value = cJSON_GetObjectItem(json , "success");
    pfcode = atoi(json_value->valuestring);
    if (pfcode != 10000)
    {
        resp->result = -1;
        cJSON_Delete(json);
        goto done;    
    }
    
    json_value = cJSON_GetObjectItem(json , "devsecret");
    strncpy(resp->devsecret, json_value->valuestring, sizeof(resp->devsecret) - 1);
    TRACE("* devsecret: %s %s %d\r\n", resp->devsecret, MDL);

    json_value = cJSON_GetObjectItem(json , "url");
    strncpy(resp->domain, json_value->valuestring, sizeof(resp->domain) - 1);
    TRACE("* domain: %s %s %d\r\n", resp->domain, MDL);

    resp->result = 0;
    
    cJSON_Delete(json);

done:
    return datasize;
}

static size_t parse_response_login(void *buffer, size_t size, size_t nmemb, void *stream)
{       
    struct LoginResponse *resp = (struct LoginResponse *)stream;
    int data_size = 0;
    int pfcode = -1;
    char *pBuffer = (char *)buffer;
    cJSON *json = NULL;
    cJSON *json_value = NULL;
    
    data_size = size * nmemb;
    pBuffer[data_size] = '\0';
    TRACE("%s %s %d\r\n", pBuffer, __FUNCTION__, __LINE__);

    resp->result = -1;

    if (data_size <= 0)
    {
        TRACE("Receive data error, data_size: %d. %s %d\r\n", data_size, __FUNCTION__, __LINE__);
        resp->result = -1;
        goto done;
    }

    json = cJSON_Parse(pBuffer);
    if (json == NULL)
    {
        TRACE("Error before: [%s]\n",cJSON_GetErrorPtr());
        goto done;
    }

    json_value = cJSON_GetObjectItem(json , "success");
    pfcode = atoi(json_value->valuestring);
    if (pfcode != 10000)
    {
        resp->result = -1;
        cJSON_Delete(json);
        goto done;    
    }
    
    json_value = cJSON_GetObjectItem(json , "lastReport");
    strncpy(resp->last_reptime, json_value->valuestring, 
            sizeof(resp->last_reptime) - 1);
    TRACE("* last_reptime: %s %s %d\r\n", resp->last_reptime, MDL);
    
    json_value = cJSON_GetObjectItem(json , "token");
    strncpy(resp->token, json_value->valuestring, 
            sizeof(resp->token) - 1);
    TRACE("* token: %s %s %d\r\n", resp->token, MDL);

    json_value = cJSON_GetObjectItem(json , "currentTime");
    strncpy(resp->cur_time, json_value->valuestring, 
            sizeof(resp->cur_time) - 1);
    TRACE("* cur_time: %s %s %d\r\n", resp->cur_time, MDL);

    resp->result = 0;

    cJSON_Delete(json);
    
done:    
    return data_size;
} 


static size_t parse_response_report_rcd(void *buffer, size_t size, size_t nmemb, void *stream)
{       
    int *iResult = (int *)stream;
    int data_size = 0;
    int pfcode = -1;
    char *pBuffer = (char *)buffer;
    cJSON *json = NULL;
    cJSON *json_value = NULL;
    
    data_size = size * nmemb; 
    pBuffer[data_size] = '\0';
    TRACE("%s %s %d\r\n", pBuffer, __FUNCTION__, __LINE__);
    
    if (data_size <= 0)
    {
        *iResult = -1;
        goto done;
    }

    json = cJSON_Parse(pBuffer);
    if (json == NULL)
    {
        TRACE("Error before: [%s]\n", cJSON_GetErrorPtr());
        goto done;
    }

    json_value = cJSON_GetObjectItem(json , "success");
    
    pfcode = atoi(json_value->valuestring);
    if (pfcode != 10000)
    {
        goto done;    
    }

    *iResult = 0;

    cJSON_Delete(json);
    
done:
    return data_size;
}

static size_t parse_response_report_photo(void *buffer, size_t size, size_t nmemb, void *stream)
{       
    int *iResult = (int *)stream;
    int data_size = 0;
    int pfcode = -1;
    char *pBuffer = (char *)buffer;
    cJSON *json = NULL;
    cJSON *json_value = NULL;
    
    data_size = size * nmemb; 
    pBuffer[data_size] = '\0';
    TRACE("%s %s %d\r\n", pBuffer, __FUNCTION__, __LINE__);
    
    if (data_size <= 0)
    {
        *iResult = -1;
        goto done;
    }
    
    json = cJSON_Parse(pBuffer);
    if (json == NULL)
    {
        TRACE("Error before: [%s]\n", cJSON_GetErrorPtr());
        goto done;
    }

    json_value = cJSON_GetObjectItem(json , "success");
    
    pfcode = atoi(json_value->valuestring);
    if (pfcode != 10000)
    {
        goto done;    
    }

    *iResult = 0;

    cJSON_Delete(json);

done:
    return data_size;
} 

#if 0
static size_t parse_response_download(void *buffer, size_t size, size_t nmemb, void *user_p) 
{ 	
    struct DownloadResponse *resp = (struct DownloadResponse *)user_p; 	
    size_t return_size = 0;

    if (resp->fp == NULL) {
        TRACE("Error, update fp is NULL. %s %d\r\n", MDL);
        resp->result = -1;
        goto End;
    }
  
    return_size = fwrite(buffer, size, nmemb, resp->fp);
    if (return_size != size * nmemb) {
        resp->result = -1;
        TRACE("Download write error(%s), %s %d\r\n", STR_ERRNO, MDL);
        goto End;
    }
    
    resp->alreadysize += return_size;
    
    resp->flushsize += return_size;
    if (resp->flushsize >= DOWNLOAD_FLUSH_SIZE) {
        fflush(resp->fp);
        resp->flushsize = 0;
    }
    
    TRACE("+++ down bytes %d(%d, %d) download size(%ld) total size(%ld) +++ \r\n", 
        size*nmemb, size, nmemb, resp->alreadysize, resp->totalsize);

    if (resp->alreadysize >= resp->totalsize)
    {
        TRACE("Downlaod upgrade file success. %s %d\r\n", MDL);
        fflush(resp->fp);
        fclose(resp->fp);
        resp->fp = NULL;
        resp->result = 0;
        goto End;
    }
    
End:        
    return return_size; 
}

static size_t parse_response_query_seven(void *buffer, size_t size, size_t nmemb, void *stream)
{       
    struct SevenResponse *responsep = (struct SevenResponse *)stream;
    int data_size = size * nmemb;
    int returnCode = -1;
    string data;
    Value jsonValue;    //表示一个json格式的对象
    char *pBuffer = (char *)buffer;

    pBuffer[data_size] = '\0';
    TRACE("%s %s %d\r\n", pBuffer, __FUNCTION__, __LINE__);

    if (data_size <= 0)
    {
        responsep->result = 2;
        goto Done;
    }

    if (string_to_jsonobj(pBuffer, jsonValue) != 0)
    {
        responsep->result = 2;
        goto Done;
    }

    if (jsonValue["success"].isNull() ||
        !jsonValue["success"].isString())
    {
        responsep->result = 2;
        goto Done;
    }

    returnCode = atoi(jsonValue["success"].asCString());
    if (returnCode != PLATFORM_OK)
    {
        /* 没有最新配置 */
        if (returnCode == 10005)
        {
            responsep->result = 3;     /* 没有最新配置 */
            goto Done;
        }
        else
        {
            responsep->result = 2;
            goto Done;
        }
    }

    if (jsonValue["init"].isNull()
        || !jsonValue["init"].isString()
        || jsonValue["configId"].isNull()
        || !jsonValue["configId"].isString()
        || jsonValue["maxLostFrame"].isNull()
        || !jsonValue["maxLostFrame"].isString()
        || jsonValue["maxDistance"].isNull()
        || !jsonValue["maxDistance"].isString()
        || jsonValue["maxFrame"].isNull()
        || !jsonValue["maxFrame"].isString()
        || jsonValue["goodTrackingCount"].isNull()
        || !jsonValue["goodTrackingCount"].isString()
        || jsonValue["minDisLen"].isNull()
        || !jsonValue["minDisLen"].isString()
        || jsonValue["minAngle"].isNull()
        || !jsonValue["minAngle"].isString()
        || jsonValue["disCardOpenFlag"].isNull()
        || !jsonValue["disCardOpenFlag"].isString()
        || jsonValue["disCardMaxDistance"].isNull()
        || !jsonValue["disCardMaxDistance"].isString())
    {
        responsep->result = 2;
        goto Done;
    }

    responsep->result            = 1;       /* 获取配置成功 */
    responsep->track_init_defalt     = atoi(jsonValue["init"].asCString());
    responsep->config_id             = atoi(jsonValue["configId"].asCString());
    responsep->max_lost_frame        = atoi(jsonValue["maxLostFrame"].asCString());
    responsep->max_distance          = atoi(jsonValue["maxDistance"].asCString()); 
    responsep->max_frame_cnt         = atoi(jsonValue["maxFrame"].asCString());
    responsep->good_track_cnt        = atoi(jsonValue["goodTrackingCount"].asCString());
    responsep->min_dis_len           = atoi(jsonValue["minDisLen"].asCString());
    responsep->min_angle             = atoi(jsonValue["minAngle"].asCString());
    responsep->discard_open_flag     = atoi(jsonValue["disCardOpenFlag"].asCString());
    responsep->discard_max_distance  = atoi(jsonValue["disCardMaxDistance"].asCString());

Done:
    return data_size;
} 


static size_t parse_response_query_video(void *buffer, size_t size, size_t nmemb, void *stream)
{
    struct VideoResponse *responsep = (struct VideoResponse *)stream;
    int data_size = size * nmemb;
    int returnCode = -1;
    string data;
    Value jsonValue;    //表示一个json格式的对象
    char *pBuffer = (char *)buffer;

    pBuffer[data_size] = '\0';
    TRACE("%s %s %d\r\n", pBuffer, __FUNCTION__, __LINE__);

    if (data_size <= 0)
    {
        responsep->result = 2;
        goto Done;
    }

    if (string_to_jsonobj(pBuffer, jsonValue) != 0)
    {
        responsep->result = 2;
        goto Done;
    }

    if (jsonValue["success"].isNull() ||
        !jsonValue["success"].isString())
    {
        responsep->result = 2;
        goto Done;
    }

    returnCode = atoi(jsonValue["success"].asCString());
    if (returnCode != PLATFORM_OK)
    {
        /* 没有最新配置 */
        if (returnCode == 10005)
        {
            responsep->result = 3;     /* 没有最新配置 */
            goto Done;
        }
        else
        {
            responsep->result = 2;
            goto Done;
        }
    }

    if (jsonValue["init"].isNull()
        || !jsonValue["init"].isString()
        || jsonValue["configId"].isNull()
        || !jsonValue["configId"].isString()
        || jsonValue["brightness"].isNull()
        || !jsonValue["brightness"].isString()
        || jsonValue["contrast"].isNull()
        || !jsonValue["contrast"].isString()
        || jsonValue["hue"].isNull()
        || !jsonValue["hue"].isString()
        || jsonValue["saturation"].isNull()
        || !jsonValue["saturation"].isString()
        || jsonValue["exposure"].isNull()
        || !jsonValue["exposure"].isString())
    {
        responsep->result = 2;
        goto Done;
    }

    responsep->result      = 1;       /* 获取配置成功 */
    responsep->init        = atoi(jsonValue["init"].asCString());
    responsep->configId    = atoi(jsonValue["configId"].asCString());
    responsep->brightness  = atoi(jsonValue["brightness"].asCString());
    responsep->contrast    = atoi(jsonValue["contrast"].asCString()); 
    responsep->hue         = atoi(jsonValue["hue"].asCString());
    responsep->saturation  = atoi(jsonValue["saturation"].asCString());
    responsep->exposure    = atoi(jsonValue["exposure"].asCString());
    
Done:
    return data_size;        
}

static size_t parse_response_query_pc(void *buffer, size_t size, size_t nmemb, void *stream)
{       
    struct PcResponse *config = (struct PcResponse *)stream;
    int data_size = size * nmemb;
    int returnCode = -1;
    string data;
    Value jsonValue;    //表示一个json格式的对象
    char *pBuffer = (char *)buffer;

    pBuffer[data_size] = '\0';
    TRACE("%s %s %d\r\n", pBuffer, __FUNCTION__, __LINE__);

    if (data_size <= 0)
    {
        config->result = 2;
        goto End;
    }

    if (string_to_jsonobj(pBuffer, jsonValue) != 0)
    {
        config->result = 2;
        goto End;
    }

    if (jsonValue["success"].isNull() ||
        !jsonValue["success"].isString())
    {
        config->result = 2;
        goto End;
    }

    returnCode = atoi(jsonValue["success"].asCString());
    if (returnCode != PLATFORM_OK)
    {
        if (returnCode == 10005)
        {
            config->result = 3;     /* 没有最新配置 */
            goto End;
        }
        else
        {
            config->result = 2;
            goto End;
        }
    }

    if (jsonValue["configId"].isNull()
        || !jsonValue["configId"].isString()
        || jsonValue["leftPoint"].isNull()
        || !jsonValue["leftPoint"].isString()
        || jsonValue["rightPoint"].isNull()
        || !jsonValue["rightPoint"].isString()
        || jsonValue["topPoint"].isNull()
        || !jsonValue["topPoint"].isString()
        || jsonValue["bottomPoint"].isNull()
        || !jsonValue["bottomPoint"].isString()
        || jsonValue["direction"].isNull()
        || !jsonValue["direction"].isString()
        || jsonValue["height"].isNull()
        || !jsonValue["height"].isString()
        || jsonValue["init"].isNull()
        || !jsonValue["init"].isString())
    {
        config->result = 2;
        goto End;
    }

    /* 获取配置成功 */
    config->result = 1;
    config->configId    = atoi(jsonValue["configId"].asCString());
    config->init        = atoi(jsonValue["init"].asCString());
    config->leftPoint   = atoi(jsonValue["leftPoint"].asCString());
    config->topPoint    = atoi(jsonValue["topPoint"].asCString());
    config->rightPoint  = atoi(jsonValue["rightPoint"].asCString());
    config->bottomPoint = atoi(jsonValue["bottomPoint"].asCString());
    config->direction   = atoi(jsonValue["direction"].asCString());
    config->height      = atoi(jsonValue["height"].asCString());

End:
    return data_size;
} 

static size_t parse_response_query_upgrade(void *buffer, size_t size, size_t nmemb, void *stream)
{       
    int data_size = 0;
    Value jsonReceive;    //表示一个json格式的对象
    int returnCode = -1;
    char *pBuffer = (char *)buffer;
    struct UpgradeResponse *resp = (struct UpgradeResponse *)stream;
    
    data_size = size * nmemb;
    pBuffer[data_size] = '\0';
    TRACE("%s %s %d\r\n", pBuffer, __FUNCTION__, __LINE__);

    if (data_size <= 0)
    {
        TRACE("Receive data error, data_size: %d. %s %d\r\n", data_size, __FUNCTION__, __LINE__);
        resp->result = -1;
        goto End;
    }

    if (string_to_jsonobj(pBuffer, jsonReceive) != 0)
    {
        TRACE("String to json object fail. %s %d \r\n", __FUNCTION__, __LINE__);
        resp->result = -1;
        goto End;
    }

    if (jsonReceive["success"].isNull() ||
        !jsonReceive["success"].isString())
    {
        TRACE("Platform response json success elem is null. %s %d\r\n", __FUNCTION__, __LINE__); 
        resp->result = -1;
        goto End;
    }

    returnCode = atoi(jsonReceive["success"].asCString());
    if (returnCode != PLATFORM_OK)
    {
        if (returnCode == 10004         /* 当前版本已是最新版本 */
            || returnCode == 10003)     /* 没有最新版本 */
        {
            resp->result = 1;
            goto End;
        }
        else
        {
            resp->result = -1;
            goto End;
        }
    }

    if (jsonReceive["url"].isNull()
        || !jsonReceive["url"].isString()
        || jsonReceive["filesize"].isNull()
        || !jsonReceive["filesize"].isString()
        || jsonReceive["version"].isNull()
        || !jsonReceive["version"].isString())
    {
        TRACE("Json (url/filesize/version) elem null or not string. %s %d\r\n", __FUNCTION__, __LINE__); 
        resp->result = -1;
        goto End;
    }

    resp->result = 0;
    resp->filesize = atoi(jsonReceive["filesize"].asCString());
    strncpy(resp->downloadurl, jsonReceive["url"].asCString(), sizeof(resp->downloadurl) - 1);
    strncpy(resp->newversion, jsonReceive["version"].asCString(), sizeof(resp->newversion) - 1);  

End:    
    return data_size;
} 

/*******************************************************************************\
** 函数名称： ParaseResponseBinding
** 函数功能： 设备绑定回调函数
** 函数参数： buffer 返回的数据缓冲区
**            size*nmemb  返回的数据大小
**            nmemb 用户自定义参数
** 函数返回： 
** 创建作者： lxd
** 创建日期： 2014-10-10
** 修改作者： 
** 修改日期： 
\*******************************************************************************/ 
static size_t parse_response_query_bind(void *buffer, size_t size, size_t nmemb, void *stream)
{       
    struct BindResponse *resp = (struct BindResponse *)stream;
    int returnCode = -1;
    int data_size = 0;
    Value jsonReceive;    //表示一个json格式的对象
    char *pBuffer = (char *)buffer;
    
    data_size = size * nmemb; 
    pBuffer[data_size] = '\0';
    
    TRACE("%s %s %d\r\n", pBuffer, __FUNCTION__, __LINE__);

    if (data_size <= 0)
    {
        resp->result = -1;
        goto End;
    }

    if (0 != string_to_jsonobj(pBuffer, jsonReceive))
    {
        resp->result = -1;
        goto End;
    }

    if (jsonReceive["success"].isNull() || 
        !jsonReceive["success"].isString())
    {
        resp->result = -1;
        goto End;
    }

    returnCode = atoi(jsonReceive["success"].asCString());
    if (returnCode != PLATFORM_OK)
    {
        resp->result = -1;
        goto End;
    }

    if (jsonReceive["isBind"].isNull() ||
        !jsonReceive["isBind"].isString())
    {
        resp->result = -1;
        goto End;
    }
    
    resp->result = 0;
    resp->bindFlag = atoi(jsonReceive["isBind"].asCString());
                        
End:
    return data_size;
} 

static size_t parse_response_report_conntest(void *buffer, size_t size, size_t nmemb, void *stream)
{
    int *result = (int *)stream;
    int data_size = 0;
    Value jsonReceive;
    char *recvBuffer = (char *)buffer;

    data_size = size * nmemb;
    recvBuffer[data_size] = '\0';
    TRACE("%s %s %d\r\n", recvBuffer, __FUNCTION__, __LINE__);

    if (data_size <= 0)
    {
        TRACE("Receive data error, data_size: %d. %s %d\r\n", data_size, __FUNCTION__, __LINE__);
        *result = -1;
        goto done;
    }

    if (0 != string_to_jsonobj(recvBuffer, jsonReceive))
    {
        TRACE("String to json object fail. %s %d \r\n", __FUNCTION__, __LINE__);
        *result = -1;
        goto done;
    }

    if (jsonReceive["success"].isNull() ||
        !jsonReceive["success"].isString())
    {
        TRACE("Platform response json success elem is null. %s %d\r\n", __FUNCTION__, __LINE__); 
        *result = -1;
        goto done;
    }

    *result = 0;
    
done:
    
    return data_size;
}


static size_t parse_response_report_upgrade(void *buffer, size_t size, size_t nmemb, void *stream)
{       
    int *iResult = (int *)stream;
    int data_size = 0;
    Value jsonReceive;    //表示一个json格式的对象
    int returnCode = -1;
    char *pBuffer = (char *)buffer;

    data_size = size * nmemb; 
    pBuffer[data_size] = '\0';
    TRACE("%s %s %d\r\n", pBuffer, __FUNCTION__, __LINE__);

    if (data_size <= 0)
    {
        *iResult = -1;
        goto End;
    }

    if (string_to_jsonobj(pBuffer, jsonReceive) != 0)
    {
        *iResult = -1;
        goto End;
    }

    if (jsonReceive["success"].isNull() ||
        !jsonReceive["success"].isString())
    {
        *iResult = -1;
        goto End;
    }

    returnCode = atoi(jsonReceive["success"].asCString());
    if (returnCode != PLATFORM_OK)
    {
        *iResult = -1;
        goto End;
    }

    *iResult = 0;

End:    
    return data_size;
}


static size_t parse_response_report_pc(void *buffer, size_t size, size_t nmemb, void *stream)
{       
    int *iResult = (int *)stream;
    int data_size = 0;
    Value jsonReceive;    //表示一个json格式的对象
    int returnCode = -1;
    char *pBuffer = (char *)buffer;

    data_size = size * nmemb; 
    pBuffer[data_size] = '\0';
    TRACE("%s %s %d\r\n", pBuffer, __FUNCTION__, __LINE__);

    if (data_size <= 0)
    {
        *iResult = -1;
        goto End;
    }

    if (string_to_jsonobj(pBuffer, jsonReceive) != 0)
    {
        *iResult = -1;
        goto End;
    }

    if (jsonReceive["success"].isNull() ||
        !jsonReceive["success"].isString())
    {
        *iResult = -1;
        goto End;
    }

    returnCode = atoi(jsonReceive["success"].asCString());
    if (returnCode != PLATFORM_OK)
    {
        *iResult = -1;
        goto End;
    }

    *iResult = 0;

End:
    return data_size;
} 


static size_t parse_response_report_seven(void *buffer, size_t size, size_t nmemb, void *stream)
{       
    int *ret_result = (int *)stream;
    int data_size = 0;
    Value jsonReceive;    //表示一个json格式的对象
    int returnCode = -1;
    char *pBuffer = (char *)buffer;

    data_size = size * nmemb; 
    pBuffer[data_size] = '\0';
    TRACE("%s %s %d\r\n", pBuffer, __FUNCTION__, __LINE__);

    if (data_size <= 0)
    {
        *ret_result = -1;
        goto Done;
    }

    if (string_to_jsonobj(pBuffer, jsonReceive) != 0)
    {
        *ret_result = -1;
        goto Done;
    }

    if (jsonReceive["success"].isNull() ||
        !jsonReceive["success"].isString())
    {
        *ret_result = -1;
        goto Done;
    }

    returnCode = atoi(jsonReceive["success"].asCString());
    if (returnCode != PLATFORM_OK)
    {
        *ret_result = -1;
        goto Done;
    }

    *ret_result = 0;

Done:
    return data_size;
} 


static size_t parse_response_report_video(void *buffer, size_t size, size_t nmemb, void *stream)
{
    int *retResult = (int *)stream;
    Value jsonReceive;
    int returnCode = -1;
    char *pBuffer = (char *)buffer;
    size_t data_size = size * nmemb;

    if (data_size <= 0)
    {
        *retResult = -1;
        goto Done;
    }

    if (string_to_jsonobj(pBuffer, jsonReceive) != 0)
    {
        *retResult = -1;
        goto Done;
    }

    if (jsonReceive["success"].isNull() ||
        !jsonReceive["success"].isString())
    {
        *retResult = -1;
        goto Done;
    }

    returnCode = atoi(jsonReceive["success"].asCString());
    if (returnCode != PLATFORM_OK)
    {
        *retResult = -1;
        goto Done;
    }

    *retResult = 0;

Done:
    return data_size;
}


/*******************************************************************************\
** 函数名称： ParaseResponseReportRecord
** 函数功能： 人头统计结果上报回调函数
** 函数参数： buffer 返回的数据缓冲区
**            size*nmemb  返回的数据大小
**            nmemb 用户自定义参数
** 函数返回： 
** 创建作者： lxd
** 创建日期： 2014-10-10
** 修改作者： 
** 修改日期： 
\*******************************************************************************/ 
static size_t parse_response_report_rcd(void *buffer, size_t size, size_t nmemb, void *stream)
{       
    int *iResult = (int *)stream;
    int data_size = 0;
    Value jsonReceive;    //表示一个json格式的对象
    int returnCode = -1;
    char *pBuffer = (char *)buffer;

    data_size = size * nmemb; 
    pBuffer[data_size] = '\0';
    TRACE("%s %s %d\r\n", pBuffer, __FUNCTION__, __LINE__);
    
    if (data_size <= 0)
    {
        *iResult = -1;
        goto End;
    }

    if (string_to_jsonobj(pBuffer, jsonReceive) != 0)
    {
        *iResult = -1;
        goto End;
    }

    if (jsonReceive["success"].isNull() ||
        !jsonReceive["success"].isString())
    {
        *iResult = -1;
        goto End;
    }

    returnCode = atoi(jsonReceive["success"].asCString());
    if (returnCode != PLATFORM_OK)
    {
        *iResult = -1;
        goto End;
    }

    *iResult = 0;

End:
    return data_size;
} 


/*******************************************************************************\
** 函数名称： ParaseResponseUploadPhoto
** 函数功能： 通用的http通讯回调函数
** 函数参数： buffer 返回的数据缓冲区
**            size*nmemb  返回的数据大小
**            nmemb 用户自定义参数
** 函数返回： 
** 创建作者： lxd
** 创建日期： 2014-10-10
** 修改作者： 
** 修改日期： 
\*******************************************************************************/ 


static size_t parse_response_report_name(void *buffer, size_t size, size_t nmemb, void *stream)
{       
    int *pResult = (int *)stream;
    int returnCode = -1;
    int data_size = 0;
    Value jsonReceive;    //表示一个json格式的对象
    char *pBuffer = (char *)buffer;
    
    data_size = size * nmemb; 
    pBuffer[data_size] = '\0';
    
    TRACE("%s %s %d\r\n", pBuffer, __FUNCTION__, __LINE__);

    if (data_size <= 0)
    {
        *pResult = -1;
        goto End;
    }

    if (0 != string_to_jsonobj(pBuffer, jsonReceive))
    {
        *pResult = -1;
        goto End;
    }

    if (jsonReceive["success"].isNull() ||
        !jsonReceive["success"].isString())
    {
        *pResult = -1;
        goto End;
    }

    returnCode = atoi(jsonReceive["success"].asCString());
    if (returnCode != PLATFORM_OK)
    {
        *pResult = -1;
        goto End;
    }

    *pResult = 0;

End:    
    return data_size;
}
#endif

/*******************************************************************************\
** 函数名称： ParaseResponseBinding
** 函数功能： 设备绑定回调函数
** 函数参数： buffer 返回的数据缓冲区
**            size*nmemb  返回的数据大小
**            nmemb 用户自定义参数
** 函数返回： 
** 创建作者： lxd
** 创建日期： 2014-10-10
** 修改作者： 
** 修改日期： 
\*******************************************************************************/ 

MRJEcode pf_get_data(const char *url, long from, MRJ_HTTP_CALLBACK func, void *arg)
{
    MRJEcode res = MRJE_UNKNOWN;
    MRJHTTP *http = NULL;
    
    http = mrjhttp_init();
    if (!http) {
        TRACE("Mrj http init fail. %s %d\r\n", MDL);
        return MRJE_FAILED_INIT; 
    }

    mrjhttp_setopt(http, MRJOPT_URL, url);
    mrjhttp_setopt(http, MRJOPT_WRITEFUNCTION, func);  
    mrjhttp_setopt(http, MRJOPT_WRITEDATA, arg);  
    mrjhttp_setopt(http, MRJOPT_RESUME_FROM, from);    /* 从from处开始断点续传 */
    mrjhttp_setopt(http, MRJOPT_TIMEOUT, 600L);          /* 升级文件下载最长时间600s */
    mrjhttp_setopt(http, MRJOPT_CONNECTTIMEOUT, 30L);
    
#ifdef MRJDEBUG
    mrjhttp_setopt(http, MRJOPT_VERBOSE, 1);
#endif

    res = mrjhttp_perform(http);
    if (res != MRJE_OK)
    {
        TRACE("Http client perform failed: %d(%s) %s %d\n", res, mrjhttp_strerror(res), __FUNCTION__, __LINE__);
        mrjhttp_cleanup(http);
        return res;
    }

    mrjhttp_cleanup(http);

    return MRJE_OK;
}


MRJEcode pf_post_data(const char *url, char *content, MRJ_HTTP_CALLBACK func, void *arg)
{
    MRJEcode res = MRJE_OK;
    MRJHTTP *handle = NULL;

    handle = mrjhttp_init();
    if (!handle) {
        TRACE("Mrj http init fail. %s %d\r\n", MDL);
        return MRJE_INTERFACE_FAILED; 
    }

    //TRACE("content: %s %s %d\r\n", content, MDL);
    
    mrjhttp_setopt(handle, MRJOPT_URL, url);
    mrjhttp_setopt(handle, MRJOPT_POSTFIELDS, content);
    mrjhttp_setopt(handle, MRJOPT_WRITEFUNCTION, func);
    mrjhttp_setopt(handle, MRJOPT_WRITEDATA, arg);
    mrjhttp_setopt(handle, MRJOPT_POST, 1L);

    mrjhttp_setopt(handle, MRJOPT_HTTP_VERSION, MRJ_HTTP_VERSION_1_0);
    mrjhttp_setopt(handle, MRJOPT_TIMEOUT, 30L);
    mrjhttp_setopt(handle, MRJOPT_CONNECTTIMEOUT, 10L);
    
#ifdef MRJDEBUG
    mrjhttp_setopt(handle, MRJOPT_VERBOSE, 1L);
#endif  

    res = mrjhttp_perform(handle);

    if (res != MRJE_OK) {
        TRACE("http perform error. %s %d\r\n", MDL);
    }

    long httpcode = 0;
    res = mrjhttp_getinfo(handle, MRJINFO_RESPONSE_CODE, &httpcode);

    TRACE("httpcode: %ld %s %d\r\n", httpcode, MDL);
    
    mrjhttp_cleanup(handle);

    return res;
}


static MRJEcode pf_setstropt(char **charp, char *s)
{
    MRJ_safefree(*charp);

    if(s) {
        s = strdup(s);

        if(!s)
            return MRJE_OUT_OF_MEMORY;

        *charp = s;
    }
    
    return MRJE_OK;
}


static MRJEcode pf_setreq_opt(MRJ_PFReq option, const char *domainp, int dnsflag, char *urlp, size_t len, va_list param)
{
    if (!urlp || len < MRJ_MAX_URL)
        return MRJE_BAD_FUNCTION_ARGUMENT;

    MRJEcode res = MRJE_OK;
    long arg;
    char *argptr;
    char ip[64] = {0};
    char domain[128] = {0};
    
    memset(urlp, 0, len);
    
#ifdef PLATFORMDEBUG
    if (option == PFREQ_DOWNLOAD)
        strncpy(ip, HTTP_SERVER_IP, sizeof(ip) - 1);
    else
        strncpy(ip, HTTP_SERVER_IP, sizeof(ip) - 1);    
#else
    if (!domainp)
        return MRJE_BAD_FUNCTION_ARGUMENT;
   
    res = transfer_domain2ip(domainp, ip, sizeof(ip), dnsflag);
    if (res != MRJE_OK) 
        return res;
#endif
    
    switch (option)
    {
    case PFREQ_AUTH:
        snprintf(urlp, len - 1, "http://%s:8888/index.php", ip);
        break;
        
    case PFREQ_LOGIN:
        snprintf(urlp, len - 1, "http://%s/index.php?action=login", ip);
        break;

    case PFREQ_DOWNLOAD:
        argptr = va_arg(param, char *);
        snprintf(urlp, len - 1, "http://%s:8080/myCount/upload/softVersion/%s", ip, argptr);
        break;
        
    case PFREQ_QUERY_BIND:
        snprintf(urlp, len - 1, "http://%s/index.php?action=state", ip);
        break;
        
    case PFREQ_QUERY_PC:
        snprintf(urlp, len - 1, "http://%s/index.php?action=config", ip);
        break;
        
    case PFREQ_QUERY_UPGRADE:
        snprintf(urlp, len - 1, "http://%s/index.php?action=upgrade", ip);
        break;

    case PFREQ_QUERY_SEVEN:
        snprintf(urlp, len - 1, "http://%s/index.php?action=algorithm", ip);
        break;
        
    case PFREQ_QUERY_VIDEO:
        snprintf(urlp, len - 1, "http://%s/index.php?action=lens", ip);
        break;    
        
    case PFREQ_REPORT_CONNTEST:
        snprintf(urlp, len - 1, "http://%s/index.php?action=contest", ip);
        break;
   
    case PFREQ_REPORT_DEV_NAME:
        snprintf(urlp, len - 1, "http://%s/index.php?action=alias", ip);
        break;
        
    case PFREQ_REPORT_PC:
        snprintf(urlp, len - 1, "http://%s/index.php?action=setting&imei=%s", ip, get_sn());
        break;    
        
    case PFREQ_REPORT_PHOTO:
        arg = va_arg(param, long);
        snprintf(urlp, len - 1, "http://%s/index.php?action=upload&imei=%s&token=%s&sort=%ld", 
            ip, get_sn(), get_token(), arg);
        break;
        
    case PFREQ_REPORT_RCD:
        snprintf(urlp, len - 1, "http://%s/index.php?action=report&imei=%s&token=%s",
            ip, get_sn(), get_token());
        break;
    
    case PFREQ_REPORT_SEVEN:
        snprintf(urlp, len - 1, "http://%s/index.php?action=setalgorithm&imei=%s&token=%s",
            ip, get_sn(), get_token());
        break;
        
    case PFREQ_REPORT_UPGRADE:
        snprintf(urlp, len - 1, "http://%s/index.php?action=reportUpgrade", ip);
        break;
        
    case PFREQ_REPORT_VIDEO:
        snprintf(urlp, len - 1, "http://%s/index.php?action=setlens&imei=%s&token=%s",
            ip, get_sn(), get_token());
        break;
        
    default:
        TRACE("Unknow pf request, pfreq = %d. %s %d \r\n", (int)option, MDL);
        return MRJE_UNSUPPORTED_PROTOCOL;
    }

    return res;
}


MRJEcode pf_setreq_url(MRJ_PFReq pfreq, const char *domainp, int dnsflag, char *urlp, size_t len, ...)
{
    va_list arg;
    MRJEcode res;   
        
    if (!urlp || len < MRJ_MAX_URL)
        return MRJE_BAD_FUNCTION_ARGUMENT;

    va_start(arg, len);

    res = pf_setreq_opt(pfreq, domainp, dnsflag, urlp, len, arg);

    va_end(arg);

    return res;    
}

static MRJEcode pf_close(struct MRJpfSessionHandle *data)
{
    MRJ_safefree(data->domain);         /* pf_setstropt's strdup */
    MRJ_safefree(data->contentp);       /* malloc */
    MRJ_safefree(data->remote_file);     /* pf_setstropt's strdup */

    free(data);
    
    return MRJE_OK;
}


static MRJPF *pf_init(void)
{
    struct MRJpfSessionHandle *data;

    data = (struct MRJpfSessionHandle *)malloc(sizeof(struct MRJpfSessionHandle));
    if (!data) {
        TRACE("malloc SessionHandle fail. %s %d\r\n", MDL);
        return NULL;
    }

    memset(data, 0, sizeof(struct MRJpfSessionHandle));
    data->pfreq = PFREQ_UNKOWN;
    return data;   
}

static MRJEcode pf_setopt(struct MRJpfSessionHandle *data, MRJPFoption option,
                     va_list param)
{
    char *argptr;
    MRJEcode result = MRJE_OK;
    long arg;

    switch (option) {
/*-------------------------------------------------------------- */
    case PFOPT_AUTH:
        if (va_arg(param, long))
            data->pfreq = PFREQ_AUTH;
        break;
        
    case PFOPT_LOGIN:
        if (va_arg(param, long))
            data->pfreq = PFREQ_LOGIN;
        break;

    case PFOPT_DOWNLOAD:
        if (va_arg(param, long))
            data->pfreq = PFREQ_DOWNLOAD;
        break;

    case PFOPT_QUERY_SEVEN:
        if (va_arg(param, long))
            data->pfreq = PFREQ_QUERY_SEVEN;
        break;
    case PFOPT_QUERY_VIDEO:
        if (va_arg(param, long))
            data->pfreq = PFREQ_QUERY_VIDEO;
        break;

    case PFOPT_QUERY_PC:
        if (va_arg(param, long))
            data->pfreq = PFREQ_QUERY_PC;
        break;

    case PFOPT_QUERY_UPGRADE:
        if (va_arg(param, long))
            data->pfreq = PFREQ_QUERY_UPGRADE;
        break;

    case PFOPT_QUERY_BIND:
        if (va_arg(param, long))
            data->pfreq = PFREQ_QUERY_BIND;
        break;

    case PFOPT_REPORT_CONNTEST:
        if (va_arg(param, long))
            data->pfreq = PFREQ_REPORT_CONNTEST;
        break;
    case PFOPT_REPORT_UPGRADE:
        if (va_arg(param, long))
            data->pfreq = PFREQ_REPORT_UPGRADE;
        break;
        
    case PFOPT_REPORT_PC:
        if (va_arg(param, long))
            data->pfreq = PFREQ_REPORT_PC;
        break;
        
    case PFOPT_REPORT_PC_APP:
        if (va_arg(param, long))
            data->pfreq = PFREQ_REPORT_PC_APP;
        break;
        
    case PFOPT_REPORT_SEVEN:
        if (va_arg(param, long))
            data->pfreq = PFREQ_REPORT_SEVEN;
        break;

    case PFOPT_REPORT_VIDEO:
        if (va_arg(param, long))
            data->pfreq = PFREQ_REPORT_VIDEO;
        break;

    case PFOPT_REPORT_RCD:
        if (va_arg(param, long))
            data->pfreq = PFREQ_REPORT_RCD;
        break;

    case PFOPT_REPORT_PHOTO:
        if (va_arg(param, long))
            data->pfreq = PFREQ_REPORT_PHOTO;
        break;

    case PFOPT_REPORT_DEV_NAME:
        if (va_arg(param, long))
            data->pfreq = PFREQ_REPORT_DEV_NAME;
        break;
        
/*-------------------------------------------------------------- */        
        
    case PFOPT_VERBOSE:
        data->verbose = (0 != va_arg(param, long)) ? 1 : 0;
        break;
        
    case PFOPT_WRITEDATA:
        data->httparg = va_arg(param, void *);
        break;
        
    case PFOPT_WRITEFUNCTION:
        data->httpfunc = va_arg(param, MRJ_HTTP_CALLBACK);
        if (!data->httpfunc)
            data->is_httpfunc_set = 0;
        else 
            data->is_httpfunc_set = 1;
        break;

    case PFOPT_CREATDATA:
        data->pfarg = va_arg(param, void *);
        break;
        
    case PFOPT_CREATFUNCTION:
        data->pffunc = va_arg(param, MRJ_PF_CALLBACK);
        if (!data->pffunc)
            data->is_pffunc_set = 0;
        else
            data->is_pffunc_set = 1;
        break;
        
    case PFOPT_DOMAINFIELDS:
        result = pf_setstropt(&data->domain, va_arg(param, char *));      /* set domain */
        break;
        
    case PFOPT_DNS_RESLOVE:
        if(va_arg(param, long))
            data->dnsflag = DNSREQ_IO;
        else
            data->dnsflag = DNSREQ_APP;
        break;

    case PFOPT_PHOTO_INDEX:
        data->photo_index = va_arg(param, long);
        break;
    
    case PFOPT_REMOTE_FILE:
        result = pf_setstropt(&data->remote_file, va_arg(param, char *));
        break;
        
    case PFOPT_RESUME_FROM:
        data->resume_from = va_arg(param, long);
        break;
        
    default:
        result = MRJE_UNKNOWN_OPTION;
        break;
    }

    return result;
}


MRJEcode pf_perform(struct MRJpfSessionHandle *handle)
{
    MRJEcode res = MRJE_OK;
    char url[MRJ_MAX_URL] = {0};
    
    if (!handle)
        return MRJE_BAD_FUNCTION_ARGUMENT;
    
    if (handle->pfreq == PFREQ_REPORT_PHOTO) { 
        res = pf_setreq_url(handle->pfreq, handle->domain, handle->dnsflag, 
                            url, sizeof(url), handle->photo_index);
        
    } else if (handle->pfreq == PFREQ_DOWNLOAD) { 
        res = pf_setreq_url(handle->pfreq, handle->domain, handle->dnsflag, 
                            url, sizeof(url), handle->remote_file);
        
    } else { 
        res = pf_setreq_url(handle->pfreq, handle->domain, handle->dnsflag, 
                            url, sizeof(url));
    }

    TRACE("URL: %s %s %d\r\n", url, MDL);
    
    if (res != MRJE_OK)
        return res;

    if (handle->pffunc) {
        handle->contentp = handle->pffunc(handle->pfarg);
        if (!handle->contentp)
            return MRJE_OUT_OF_MEMORY;
    }

    if (handle->pfreq == PFREQ_DOWNLOAD)
        res = pf_get_data(url, handle->resume_from, handle->httpfunc, handle->httparg);
    else 
        res = pf_post_data(url, handle->contentp, handle->httpfunc, handle->httparg);
    
    return res;
}


MRJPF *MRJpf_init(void)
{
    return pf_init();
}

void MRJpf_cleanup(MRJPF *handle)
{
    struct MRJpfSessionHandle *data = (struct MRJpfSessionHandle *)handle;

    if (!data)
        return;

    pf_close(data);
}

MRJEcode MRJpf_setopt(MRJPF *handle,  MRJPFoption tag, ...)
{
    va_list arg;
    struct MRJpfSessionHandle *data = (struct MRJpfSessionHandle *)handle;
    MRJEcode res;

    if (!handle)
        return MRJE_BAD_FUNCTION_ARGUMENT;

    va_start(arg, tag);

    res = pf_setopt(data, tag, arg);

    va_end(arg);

    return res;
}

MRJEcode MRJpf_perform(MRJPF *handle)
{
    return pf_perform((struct MRJpfSessionHandle *)handle);
}

int MRJpf_auth()
{
    MRJEcode res = MRJE_ERR;
    MRJPF *pfhandle = NULL;
    struct AuthResponse *resp = &g_authResponse;

    memset(resp, 0, sizeof(struct AuthResponse));
    resp->result = -1;

    pfhandle = MRJpf_init();
    if (pfhandle == NULL)
    {
        TRACE("> MRJpf init failed. %s %d", MDL);
        return -1;
    }

    MRJpf_setopt(pfhandle, PFOPT_AUTH, 1L);
    MRJpf_setopt(pfhandle, PFOPT_DOMAINFIELDS, PLATFORM_AUTH_NAME);
    MRJpf_setopt(pfhandle, PFOPT_DNS_RESLOVE, 0L);
    MRJpf_setopt(pfhandle, PFOPT_WRITEDATA, resp);
    
    MRJpf_setopt(pfhandle, PFOPT_CREATFUNCTION, create_auth_content);

    MRJpf_setopt(pfhandle, PFOPT_WRITEFUNCTION, parse_response_auth);

    res = MRJpf_perform(pfhandle);
    if (res != MRJE_OK) 
    {
        TRACE("> Http client perform failed(%d). %s %d\n", res, MDL);
        MRJpf_cleanup(pfhandle);
        return -1;
    }

    MRJpf_cleanup(pfhandle);

    if (resp->result != 0)
    {
        return -1;
    }
    
    return 0;
}

int MRJpf_login()
{
    MRJEcode res = MRJE_UNKNOWN;
    MRJPF *pfhandle = NULL;
    struct LoginResponse *resp = &g_loginResponse;
    long flag = g_dnsFlag1;

    memset(resp, 0, sizeof(struct LoginResponse));
    resp->result = -1;
    
    pfhandle = MRJpf_init();
    if (!pfhandle) {
        TRACE("MRJpf init fail. %s %d\r\n", MDL);
        return MRJE_INTERFACE_FAILED;
    }
    
    MRJpf_setopt(pfhandle, PFOPT_LOGIN, 1L);
    MRJpf_setopt(pfhandle, PFOPT_DOMAINFIELDS, get_domain());
    MRJpf_setopt(pfhandle, PFOPT_DNS_RESLOVE, flag);
    MRJpf_setopt(pfhandle, PFOPT_WRITEDATA, resp);
    MRJpf_setopt(pfhandle, PFOPT_WRITEFUNCTION, parse_response_login);
    MRJpf_setopt(pfhandle, PFOPT_CREATFUNCTION, create_login_json_string);

    res = MRJpf_perform(pfhandle);
    if (res != MRJE_OK) 
    {
        TRACE("Http client perform failed: %d %s %d\n", res, MDL);
        MRJpf_cleanup(pfhandle);
        return (-1);
    }

    MRJpf_cleanup(pfhandle);

    if (resp->result != 0)
        return (-1);

    return (0);
}

int MRJpf_report_rcd(struct ReportRcdReq *reqp)
{
    MRJEcode res = MRJE_UNKNOWN;
    MRJPF *pfhandle = NULL;
    int result = -1;
    long flag = g_dnsFlag1;
    
    pfhandle = MRJpf_init();
    if (!pfhandle) {
        TRACE("MRJpf init fail. %s %d\r\n", MDL);
        return (-1);
    }
    
    MRJpf_setopt(pfhandle, PFOPT_REPORT_RCD, 1L);
    MRJpf_setopt(pfhandle, PFOPT_DOMAINFIELDS, get_domain());
    MRJpf_setopt(pfhandle, PFOPT_DNS_RESLOVE, flag);

    MRJpf_setopt(pfhandle, PFOPT_CREATDATA, reqp);
    MRJpf_setopt(pfhandle, PFOPT_CREATFUNCTION, create_report_rcd_json_string);

    MRJpf_setopt(pfhandle, PFOPT_WRITEDATA, &result);
    MRJpf_setopt(pfhandle, PFOPT_WRITEFUNCTION, parse_response_report_rcd);
    
    res = MRJpf_perform(pfhandle);
    if (res != MRJE_OK) {
        TRACE("Http client perform failed: %d %s %d\n", res, MDL);
        MRJpf_cleanup(pfhandle);
        return (-1);
    }

    MRJpf_cleanup(pfhandle);

    if (result != 0) 
        return (-1);

    return (0);
}

int MRJpf_report_photo(char *photopath, long index)
{
    MRJEcode res = MRJE_UNKNOWN;
    MRJPF *pfhandle = NULL;
    int result = -1;
    long flag = g_dnsFlag1;
    
    pfhandle = MRJpf_init();
    if (!pfhandle) {
        TRACE("MRJpf init fail. %s %d\r\n", MDL);
        return (-1);
    }
    
    MRJpf_setopt(pfhandle, PFOPT_REPORT_PHOTO, 1L);
    MRJpf_setopt(pfhandle, PFOPT_DOMAINFIELDS, get_domain());
    MRJpf_setopt(pfhandle, PFOPT_PHOTO_INDEX, index);
    MRJpf_setopt(pfhandle, PFOPT_DNS_RESLOVE, flag);
    
    MRJpf_setopt(pfhandle, PFOPT_CREATDATA, photopath);
    MRJpf_setopt(pfhandle, PFOPT_CREATFUNCTION, create_report_photo_content);

    MRJpf_setopt(pfhandle, PFOPT_WRITEDATA, &result);
    MRJpf_setopt(pfhandle, PFOPT_WRITEFUNCTION, parse_response_report_photo);
    
    res = MRJpf_perform(pfhandle);
    if (res != MRJE_OK) {
        TRACE("Http client perform failed: %d %s %d\n", res, MDL);
        MRJpf_cleanup(pfhandle);
        return (-1);
    }

    MRJpf_cleanup(pfhandle);

    if (result != 0) 
        return (-1);

    return (0);
}

#if 0
static MRJEcode MRJpf_download_init(struct DownloadResponse *resp, const char *filepath, long totalsize)
{
    if (!resp || !filepath || totalsize <= 0) 
        return MRJE_BAD_FUNCTION_ARGUMENT;

    memset(resp, 0, sizeof(struct DownloadResponse));
    resp->totalsize = totalsize;
    resp->result = -1;
    resp->alreadysize = 0;
    resp->flushsize = 0;
    resp->fp = NULL;
    
    struct stat st;
    memset(&st, 0, sizeof(struct stat));
    
    if (stat(filepath, &st) == 0
        && st.st_size > 0 
        && st.st_size < resp->totalsize) {
        
        resp->fp = fopen(filepath, "ab+");
        if (resp->fp == NULL) {
            TRACE("fopen %s(%s) fail. %s %d\r\n", filepath, STR_ERRNO, MDL);
            return MRJE_OPEN_FILE_ERROR;
        }
        
        resp->alreadysize = st.st_size;       /* 已经下载的文件大小 */
        TRACE("Download resume, already download size(%ld). %s %d\r\n", resp->alreadysize, __FUNCTION__, __LINE__);

    } else {
    
        /* 重新下载文件 */
        resp->fp = fopen(filepath, "wb+");
        if (NULL == resp->fp) {
            TRACE("fopen %s(%s) fail. %s %d\r\n", filepath, STR_ERRNO, MDL);
            return MRJE_OPEN_FILE_ERROR;
        }
        
        resp->alreadysize = 0;
    }

    return MRJE_OK;
}

/*******************************************************************************\
** 函数名称： DealDownloadFromPlatformResume
** 函数功能： 平台下载升级文件，支持断点续传
** 函数参数： downUrl: 升级文件下载链接
**            filePath: 升级文件本地保存地址
** 函数返回： 正确返回0，错误返回-1
** 创建作者： lxd
** 创建日期： 2014-10-10
** 修改作者： 
** 修改日期： 
\*******************************************************************************/
int MRJpf_download(const char *url, const char *filepath, char *remotename, long totalsize)
{
    MRJEcode res = MRJE_UNKNOWN;
    MRJPF *pfhandle = NULL;
    long flag = g_dnsFlag2;
    long resume_from = 0;
    struct DownloadResponse *resp = &g_downloadResponse;

    res = MRJpf_download_init(resp, filepath, totalsize);
    if (res != MRJE_OK) return (-1);

    resume_from = resp->alreadysize;
    
    pfhandle = MRJpf_init();
    if (!pfhandle) {
        TRACE("MRJpf init fail. %s %d\r\n", MDL);
        return (-1);
    }

    MRJpf_setopt(pfhandle, PFOPT_DOWNLOAD, 1L);
    MRJpf_setopt(pfhandle, PFOPT_DOMAINFIELDS, HTTP_APP_SERVER_NAME);
    MRJpf_setopt(pfhandle, PFOPT_DNS_RESLOVE, flag);
    MRJpf_setopt(pfhandle, PFOPT_REMOTE_FILE, remotename);
    MRJpf_setopt(pfhandle, PFOPT_RESUME_FROM, resume_from);

    MRJpf_setopt(pfhandle, PFOPT_WRITEDATA, resp);
    MRJpf_setopt(pfhandle, PFOPT_WRITEFUNCTION, parse_response_download);
    
    res = MRJpf_perform(pfhandle);
    if (res != MRJE_OK) {
        TRACE("Http client perform failed: %d %s %d\n", res, __FUNCTION__, __LINE__);
        MRJpf_cleanup(pfhandle);
        return (-1);
    }

    MRJpf_cleanup(pfhandle);
    
    if (resp->result != 0)
        return (-1);
    
    return (0);
}

/*******************************************************************************\
** 函数名称： DealReportConnTestToPlatform
** 函数功能： 上报网络连接测试到平台
** 函数参数： 
** 函数返回： 正确返回0，错误返回-1
** 创建作者： lxd
** 创建日期： 2014-10-10
** 修改作者： 
** 修改日期： 
\*******************************************************************************/
int MRJpf_report_conntest()
{   
    MRJEcode res = MRJE_UNKNOWN;
    MRJPF *pfhandle = NULL;
    int result = -1;
    long flag = g_dnsFlag1;
    
    pfhandle = MRJpf_init();
    if (!pfhandle) {
        TRACE("MRJpf init fail. %s %d\r\n", MDL);
        return (-1);
    }
    
    MRJpf_setopt(pfhandle, PFOPT_REPORT_CONNTEST, 1L);
    MRJpf_setopt(pfhandle, PFOPT_DOMAINFIELDS, HTTP_SERVER_NAME);
    MRJpf_setopt(pfhandle, PFOPT_DNS_RESLOVE, flag);
    
    MRJpf_setopt(pfhandle, PFOPT_WRITEDATA, &result);
    MRJpf_setopt(pfhandle, PFOPT_WRITEFUNCTION, parse_response_report_conntest);
    
    MRJpf_setopt(pfhandle, PFOPT_CREATFUNCTION, create_content);

    res = MRJpf_perform(pfhandle);
    if (res != MRJE_OK) {
        TRACE("Http client perform failed: %d %s %d\n", res, __FUNCTION__, __LINE__);
        MRJpf_cleanup(pfhandle);
        return (-1);
    }

    MRJpf_cleanup(pfhandle);

    if (result != 0) 
        return (-1);

    return (0);
}


/*******************************************************************************\
** 函数名称： DealReportConfigToPlatform
** 函数功能： 更改配置参数状态上报
** 函数参数：    
** 函数返回： 正确返回0，错误返回-1
** 创建作者： lxd
** 创建日期： 2014-10-13
** 修改作者： 
** 修改日期： 
\*******************************************************************************/
int MRJpf_report_pc()
{
    MRJEcode res = MRJE_UNKNOWN;
    MRJPF *pfhandle = NULL;
    int result = -1;
    long flag = g_dnsFlag1;
    
    pfhandle = MRJpf_init();
    if (!pfhandle) {
        TRACE("MRJpf init fail. %s %d\r\n", MDL);
        return (-1);
    }
    
    MRJpf_setopt(pfhandle, PFOPT_REPORT_PC, 1L);
    MRJpf_setopt(pfhandle, PFOPT_DOMAINFIELDS, HTTP_SERVER_NAME);
    MRJpf_setopt(pfhandle, PFOPT_DNS_RESLOVE, flag);
    
    MRJpf_setopt(pfhandle, PFOPT_CREATFUNCTION, create_report_pc_json_string);

    MRJpf_setopt(pfhandle, PFOPT_WRITEDATA, &result);
    MRJpf_setopt(pfhandle, PFOPT_WRITEFUNCTION, parse_response_report_pc);
    
    res = MRJpf_perform(pfhandle);
    if (res != MRJE_OK) {
        TRACE("Http client perform failed: %d %s %d\n", res, __FUNCTION__, __LINE__);
        MRJpf_cleanup(pfhandle);
        return (-1);
    }

    MRJpf_cleanup(pfhandle);

    if (result != 0)
        return (-1);

    return (0);
}

/*******************************************************************************\
** 函数名称： DealReportPCParamToPlatform
** 函数功能： 上报更改后的算法参数给平台
** 函数参数：           
** 函数返回： 正确返回0，错误返回-1
** 创建作者： lxd
** 创建日期： 2014-10-13
** 修改作者： 
** 修改日期： 
\*******************************************************************************/
int MRJpf_report_pc_app()
{
    MRJEcode res = MRJE_UNKNOWN;
    MRJPF *pfhandle = NULL;
    int result = -1;
    long flag = g_dnsFlag1;
    
    pfhandle = MRJpf_init();
    if (!pfhandle) {
        TRACE("MRJpf init fail. %s %d\r\n", MDL);
        return (-1);
    }
    
    MRJpf_setopt(pfhandle, PFOPT_REPORT_PC_APP, 1L);
    MRJpf_setopt(pfhandle, PFOPT_DOMAINFIELDS, HTTP_SERVER_NAME);
    MRJpf_setopt(pfhandle, PFOPT_DNS_RESLOVE, flag);

    MRJpf_setopt(pfhandle, PFOPT_CREATFUNCTION, create_report_app_pc_json_string);
    
    MRJpf_setopt(pfhandle, PFOPT_WRITEDATA, &result);
    MRJpf_setopt(pfhandle, PFOPT_WRITEFUNCTION, parse_response_report_pc);

    res = MRJpf_perform(pfhandle);
    if (res != MRJE_OK) {
        TRACE("Http client perform failed: %d %s %d\n", res, __FUNCTION__, __LINE__);
        MRJpf_cleanup(pfhandle);
        return (-1);
    }

    MRJpf_cleanup(pfhandle);

    if (result != 0) 
        return (-1);

    return (0);
}

/*******************************************************************************\
** 函数名称： DealReportSevenToPlatform
** 函数功能： 更改seven参数状态上报
** 函数参数：    
** 函数返回： 正确返回0，错误返回-1
** 创建作者： lxd
** 创建日期： 2014-10-13
** 修改作者： 
** 修改日期： 
\*******************************************************************************/
int MRJpf_report_seven()
{
    MRJEcode res = MRJE_UNKNOWN;
    MRJPF *pfhandle = NULL;
    int result = -1;
    long flag = g_dnsFlag1;
    
    pfhandle = MRJpf_init();
    if (!pfhandle) {
        TRACE("MRJpf init fail. %s %d\r\n", MDL);
        return (-1);
    }
    
    MRJpf_setopt(pfhandle, PFOPT_REPORT_SEVEN, 1L);
    MRJpf_setopt(pfhandle, PFOPT_DOMAINFIELDS, HTTP_SERVER_NAME);
    MRJpf_setopt(pfhandle, PFOPT_DNS_RESLOVE, flag);
    
    MRJpf_setopt(pfhandle, PFOPT_CREATFUNCTION, create_report_seven_json_string);

    MRJpf_setopt(pfhandle, PFOPT_WRITEDATA, &result);
    MRJpf_setopt(pfhandle, PFOPT_WRITEFUNCTION, parse_response_report_seven);
    
    res = MRJpf_perform(pfhandle);
    if (res != MRJE_OK) {
        TRACE("Http client perform failed: %d %s %d\n", res, __FUNCTION__, __LINE__);
        MRJpf_cleanup(pfhandle);
        return (-1);
    }

    MRJpf_cleanup(pfhandle);

    if (result != 0)
        return (-1);

    return (0);
}

/*******************************************************************************\
** 函数名称： DealReportVideoToPlatform
** 函数功能： 更改video参数状态上报
** 函数参数：    
** 函数返回： 正确返回0，错误返回-1
** 创建作者： lxd
** 创建日期： 2014-10-13
** 修改作者： 
** 修改日期： 
\*******************************************************************************/
int MRJpf_report_video()
{   
    MRJEcode res = MRJE_UNKNOWN;
    MRJPF *pfhandle = NULL;
    int result = -1;
    long flag = g_dnsFlag1;
    
    pfhandle = MRJpf_init();
    if (!pfhandle) {
        TRACE("MRJpf init fail. %s %d\r\n", MDL);
        return (-1);
    }
    
    MRJpf_setopt(pfhandle, PFOPT_REPORT_VIDEO, 1L);
    MRJpf_setopt(pfhandle, PFOPT_DOMAINFIELDS, HTTP_SERVER_NAME);
    MRJpf_setopt(pfhandle, PFOPT_DNS_RESLOVE, flag);
    
    MRJpf_setopt(pfhandle, PFOPT_CREATFUNCTION, create_report_video_json_string);

    MRJpf_setopt(pfhandle, PFOPT_WRITEDATA, &result);
    MRJpf_setopt(pfhandle, PFOPT_WRITEFUNCTION, parse_response_report_video);
    
    res = MRJpf_perform(pfhandle);
    if (res != MRJE_OK) {
        TRACE("Http client perform failed: %d %s %d\n", res, __FUNCTION__, __LINE__);
        MRJpf_cleanup(pfhandle);
        return (-1);
    }

    MRJpf_cleanup(pfhandle);

    if (result != 0)
        return (-1);

    return (0);
}

/*******************************************************************************\
** 函数名称： DealReportNameToPlatform
** 函数功能： 上传修改设备名到平台
** 函数参数： 
** 函数返回： 正确返回0，错误返回-1
** 创建作者： lxd
** 创建日期： 2014-10-10
** 修改作者： 
** 修改日期： 
\*******************************************************************************/
int MRJpf_report_devname()
{
    MRJEcode res = MRJE_UNKNOWN;
    MRJPF *pfhandle = NULL;
    int result = -1;
    long flag = g_dnsFlag1;
    
    pfhandle = MRJpf_init();
    if (!pfhandle) {
        TRACE("MRJpf init fail. %s %d\r\n", MDL);
        return (-1);
    }
    
    MRJpf_setopt(pfhandle, PFOPT_REPORT_DEV_NAME, 1L);
    MRJpf_setopt(pfhandle, PFOPT_DOMAINFIELDS, HTTP_SERVER_NAME);
    MRJpf_setopt(pfhandle, PFOPT_DNS_RESLOVE, flag);

    MRJpf_setopt(pfhandle, PFOPT_CREATFUNCTION, create_report_name_json_string);
    
    MRJpf_setopt(pfhandle, PFOPT_WRITEDATA, &result);
    MRJpf_setopt(pfhandle, PFOPT_WRITEFUNCTION, parse_response_report_name);

    res = MRJpf_perform(pfhandle);
    if (res != MRJE_OK) {
        TRACE("Http client perform failed: %d %s %d\n", res, __FUNCTION__, __LINE__);
        MRJpf_cleanup(pfhandle);
        return (-1);
    }

    MRJpf_cleanup(pfhandle);

    if (result != 0) 
        return (-1);

    return (0);
}

/*******************************************************************************\
** 函数名称： DealReportUpgradeToPlatform
** 函数功能： 终端调用该接口将设备升级后的软件版本上报至平台, 保证平台和设备软件版本一致性.
** 函数参数：         
** 函数返回： 正确返回0，错误返回-1
** 创建作者： lxd
** 创建日期： 2014-10-13
** 修改作者： 
** 修改日期： 
\*******************************************************************************/
int MRJpf_report_upgrade()
{   
    MRJEcode res = MRJE_UNKNOWN;
    MRJPF *pfhandle = NULL;
    int result = -1;
    long flag = g_dnsFlag1;
    
    pfhandle = MRJpf_init();
    if (!pfhandle) {
        TRACE("MRJpf init fail. %s %d\r\n", MDL);
        return (-1);
    }
    
    MRJpf_setopt(pfhandle, PFOPT_REPORT_UPGRADE, 1L);
    MRJpf_setopt(pfhandle, PFOPT_DOMAINFIELDS, HTTP_SERVER_NAME);
    MRJpf_setopt(pfhandle, PFOPT_DNS_RESLOVE, flag);
    
    MRJpf_setopt(pfhandle, PFOPT_CREATFUNCTION, create_report_upgrade_json_string);

    MRJpf_setopt(pfhandle, PFOPT_WRITEDATA, &result);
    MRJpf_setopt(pfhandle, PFOPT_WRITEFUNCTION, parse_response_report_upgrade);
    
    res = MRJpf_perform(pfhandle);
    if (res != MRJE_OK) {
        TRACE("Http client perform failed: %d %s %d\n", res, __FUNCTION__, __LINE__);
        MRJpf_cleanup(pfhandle);
        return (-1);
    }

    MRJpf_cleanup(pfhandle);

    if (result != 0)
        return (-1);

    return (0);
}

/*******************************************************************************\
** 函数名称： DealCheckConfigFromPlatform
** 函数功能： 设备配置参数获取,从平台获取最新配置
** 函数参数： pstrConfig: 平台配置参数保存结构体    
** 函数返回： 正确返回0，错误返回-1
** 创建作者： lxd
** 创建日期： 2014-10-13
** 修改作者： 
** 修改日期： 
\*******************************************************************************/
int MRJpf_query_pc()
{   
    MRJEcode res = MRJE_UNKNOWN;
    MRJPF *pfhandle = NULL;
    struct PcResponse *resp = &g_pcResponse;
    long flag = g_dnsFlag1;

    memset(resp, 0, sizeof(struct PcResponse));
    resp->result = 2;
    
    pfhandle = MRJpf_init();
    if (!pfhandle) {
        TRACE("MRJpf init fail. %s %d\r\n", MDL);
        return (-1);
    }
    
    MRJpf_setopt(pfhandle, PFOPT_QUERY_PC, 1L);
    MRJpf_setopt(pfhandle, PFOPT_DOMAINFIELDS, HTTP_SERVER_NAME);
    MRJpf_setopt(pfhandle, PFOPT_DNS_RESLOVE, flag);
    
    MRJpf_setopt(pfhandle, PFOPT_CREATFUNCTION, create_content);

    MRJpf_setopt(pfhandle, PFOPT_WRITEDATA, resp);
    MRJpf_setopt(pfhandle, PFOPT_WRITEFUNCTION, parse_response_query_pc);
    
    res = MRJpf_perform(pfhandle);
    if (res != MRJE_OK) {
        TRACE("Http client perform failed: %d %s %d\n", res, __FUNCTION__, __LINE__);
        MRJpf_cleanup(pfhandle);
        return (-1);
    }

    MRJpf_cleanup(pfhandle);

    if (resp->result == 2)
        return (-1);

    return (0);
}


int MRJpf_query_seven()
{
    MRJEcode res = MRJE_UNKNOWN;
    MRJPF *pfhandle = NULL;
    struct SevenResponse *resp = &g_sevenResponse;
    long flag = g_dnsFlag1;

    memset(resp, 0, sizeof(struct SevenResponse));
    resp->result = 2;
    
    pfhandle = MRJpf_init();
    if (!pfhandle) {
        TRACE("MRJpf init fail. %s %d\r\n", MDL);
        return (-1);
    }
    
    MRJpf_setopt(pfhandle, PFOPT_QUERY_SEVEN, 1L);
    MRJpf_setopt(pfhandle, PFOPT_DOMAINFIELDS, HTTP_SERVER_NAME);
    MRJpf_setopt(pfhandle, PFOPT_DNS_RESLOVE, flag);
    
    MRJpf_setopt(pfhandle, PFOPT_CREATFUNCTION, create_content);

    MRJpf_setopt(pfhandle, PFOPT_WRITEDATA, resp);
    MRJpf_setopt(pfhandle, PFOPT_WRITEFUNCTION, parse_response_query_seven);
    
    res = MRJpf_perform(pfhandle);
    if (res != MRJE_OK) {
        TRACE("Http client perform failed: %d %s %d\n", res, __FUNCTION__, __LINE__);
        MRJpf_cleanup(pfhandle);
        return (-1);
    }

    MRJpf_cleanup(pfhandle);

    if (resp->result == 2)
        return (-1);

    return (0);
}   


int MRJpf_query_video()
{
    MRJEcode res = MRJE_UNKNOWN;
    MRJPF *pfhandle = NULL;
    struct VideoResponse *resp = &g_videoResponse;
    long flag = g_dnsFlag1;

    memset(resp, 0, sizeof(struct VideoResponse));
    resp->result = 2;
    
    pfhandle = MRJpf_init();
    if (!pfhandle) {
        TRACE("MRJpf init fail. %s %d\r\n", MDL);
        return (-1);
    }
    
    MRJpf_setopt(pfhandle, PFOPT_QUERY_VIDEO, 1L);
    MRJpf_setopt(pfhandle, PFOPT_DOMAINFIELDS, HTTP_SERVER_NAME);
    MRJpf_setopt(pfhandle, PFOPT_DNS_RESLOVE, flag);
    
    MRJpf_setopt(pfhandle, PFOPT_CREATFUNCTION, create_content);

    MRJpf_setopt(pfhandle, PFOPT_WRITEDATA, &resp);
    MRJpf_setopt(pfhandle, PFOPT_WRITEFUNCTION, parse_response_query_video);
    
    res = MRJpf_perform(pfhandle);
    if (res != MRJE_OK) {
        TRACE("Http client perform failed: %d %s %d\n", res, __FUNCTION__, __LINE__);
        MRJpf_cleanup(pfhandle);
        return (-1);
    }

    MRJpf_cleanup(pfhandle);

    if (resp->result == 2)
        return (-1);

    return (0);
}

/*******************************************************************************\
** 函数名称： DealCheckUpgradeFromPlatform
** 函数功能： 查询是否需要升级
** 函数参数： pUpgradeInfo: 升级参数保存结构体
** 函数返回： 正确返回0，错误返回-1
** 创建作者： lxd
** 创建日期： 2014-10-13
** 修改作者： 
** 修改日期： 
\*******************************************************************************/
int MRJpf_query_upgrade()
{
    MRJEcode res = MRJE_UNKNOWN;
    MRJPF *pfhandle = NULL;
    struct UpgradeResponse *resp = &g_upgradeResponse;
    long flag = g_dnsFlag1;

    memset(resp, 0, sizeof(struct UpgradeResponse));
    resp->result = -1;
    
    pfhandle = MRJpf_init();
    if (!pfhandle) {
        TRACE("MRJpf init fail. %s %d\r\n", MDL);
        return (-1);
    }
    
    MRJpf_setopt(pfhandle, PFOPT_QUERY_UPGRADE, 1L);
    MRJpf_setopt(pfhandle, PFOPT_DOMAINFIELDS, HTTP_SERVER_NAME);
    MRJpf_setopt(pfhandle, PFOPT_DNS_RESLOVE, flag);
    
    MRJpf_setopt(pfhandle, PFOPT_CREATFUNCTION, create_query_upgrade_json_string);

    MRJpf_setopt(pfhandle, PFOPT_WRITEDATA, &resp);
    MRJpf_setopt(pfhandle, PFOPT_WRITEFUNCTION, parse_response_query_upgrade);
    
    res = MRJpf_perform(pfhandle);
    if (res != MRJE_OK) {
        TRACE("Http client perform failed: %d %s %d\n", res, __FUNCTION__, __LINE__);
        MRJpf_cleanup(pfhandle);
        return (-1);
    }

    MRJpf_cleanup(pfhandle);

    if (resp->result == -1)
        return (-1);

    return (0);
}

/*******************************************************************************\
** 函数名称： DealCheckBindingFromPlatform
** 函数功能： 平台查询设备绑定信息
** 函数参数： pCheckBind: 绑定信息保存结构体
** 函数返回： 正确返回0，错误返回-1
** 创建作者： lxd
** 创建日期： 2014-10-10
** 修改作者： 
** 修改日期： 
\*******************************************************************************/
int MRJpf_query_bind()
{
    MRJEcode res = MRJE_UNKNOWN;
    MRJPF *pfhandle = NULL;
    struct BindResponse *resp = &g_bindResponse;
    long flag = g_dnsFlag1;

    memset(resp, 0, sizeof(struct BindResponse));
    resp->result = -1;
    
    pfhandle = MRJpf_init();
    if (!pfhandle) {
        TRACE("MRJpf init fail. %s %d\r\n", MDL);
        return (-1);
    }
    
    MRJpf_setopt(pfhandle, PFOPT_QUERY_BIND, 1L);
    MRJpf_setopt(pfhandle, PFOPT_DOMAINFIELDS, HTTP_SERVER_NAME);
    MRJpf_setopt(pfhandle, PFOPT_DNS_RESLOVE, flag);
    
    MRJpf_setopt(pfhandle, PFOPT_CREATFUNCTION, create_content);

    MRJpf_setopt(pfhandle, PFOPT_WRITEDATA, &resp);
    MRJpf_setopt(pfhandle, PFOPT_WRITEFUNCTION, parse_response_query_bind);
    
    res = MRJpf_perform(pfhandle);
    if (res != MRJE_OK) {
        TRACE("Http client perform failed: %d %s %d\n", res, __FUNCTION__, __LINE__);
        MRJpf_cleanup(pfhandle);
        return (-1);
    }

    MRJpf_cleanup(pfhandle);

    if (resp->result != 0)
        return (-1);

    return (0);
}
#endif

struct ReportRcdTest
{
    char *contentbuffer;
    int contentlen;
};

char *create_report_rcd_test_json_string(void *param)
{
    struct ReportRcdTest *contentp = (param) ? (struct ReportRcdTest *)param : NULL;

    char *data = (char *)malloc(contentp->contentlen);
    if (data == NULL)
        return NULL;

    memset(data, 0, contentp->contentlen);
    memcpy(data, contentp->contentbuffer, contentp->contentlen);

    return data;
}

int MRJpf_report_record_test(struct ReportRcdTest *cotentp)
{
    MRJEcode res = MRJE_UNKNOWN;
    MRJPF *pfhandle = NULL;
    int result = -1;
    long flag = 0L;
    
    pfhandle = MRJpf_init();
    if (!pfhandle) 
    {
        TRACE("MRJpf init fail. %s %d\r\n", MDL);
        return (-1);
    }
    
    MRJpf_setopt(pfhandle, PFOPT_REPORT_RCD, 1L);
    MRJpf_setopt(pfhandle, PFOPT_DOMAINFIELDS, get_domain());
    MRJpf_setopt(pfhandle, PFOPT_DNS_RESLOVE, flag);

    MRJpf_setopt(pfhandle, PFOPT_CREATDATA, cotentp);
    MRJpf_setopt(pfhandle, PFOPT_CREATFUNCTION, create_report_rcd_test_json_string);

    MRJpf_setopt(pfhandle, PFOPT_WRITEDATA, &result);
    //MRJpf_setopt(pfhandle, PFOPT_WRITEFUNCTION, parse_response_report_rcd);
    
    res = MRJpf_perform(pfhandle);
    if (res != MRJE_OK) 
    {
        TRACE("Http client perform failed: %d %s %d\n", res, MDL);
        MRJpf_cleanup(pfhandle);
        return (-1);
    }

    MRJpf_cleanup(pfhandle);

    if (result != 0) 
        return (-1);

    return (0);      
}

int MRJpf_read_file(const char *filepath, char *buffer, int buffersize)
{
    struct stat st;
    const char *rcdfile = "rcd.txt";
    FILE *fp = NULL;
    int filesize = 0;
    int readsize = 0;
    
    memset(&st, 0, sizeof(st));
    if (stat(rcdfile, &st) != 0 || st.st_size == 0)
    {
        TRACE("> file (%s) not exist. %s %d\r\n", rcdfile, MDL);
        return -1;
    }

    TRACE("* file size: %d. %s %d\r\n", st.st_size, MDL);
    
    filesize = st.st_size;
        
    if (filesize > buffersize)
    {
        TRACE("* file size: %d. %s %d\r\n", st.st_size, MDL);
        return -1;
    }
     
    fp = fopen(rcdfile, "r");
    if (fp == NULL)
    {
        TRACE("> fopen %s file failed. %s %d\r\n", rcdfile, MDL);
        return -1;
    }

    memset(buffer, 0, buffersize);
    
    readsize = fread(buffer, sizeof(char), filesize, fp);
    if (readsize != filesize)
    {
        TRACE("> fread %s file failed. %s %d\r\n");
        fclose(fp);
        return -1;
    }
    
    fclose(fp);

    return readsize;
}

int g_exitFlag = 0;
int g_authFlag = 0;
int g_loginFlag = 0;
time_t g_timeNow = 0;
time_t g_lastReportRcd = 0;
time_t g_lastReportPhoto = 0;

void quit(int signo)
{
    TRACE("* receive exit message by CTRL+C. %s %d\r\n", MDL);
    g_exitFlag = 1;   
}

int main()
{
    int i;
    int ret = 0;
    int count = 0;

    signal(SIGINT, quit);

    struct ReportRcdReq reportRcdRequest;
    memset(&reportRcdRequest, 0, sizeof(reportRcdRequest));
    reportRcdRequest.num = 100;
    for (i = 0; i < reportRcdRequest.num; i++)
    {
        strncpy(reportRcdRequest.data[i].imsi, "EE:02:13:00:12:30", sizeof(reportRcdRequest.data[i].imsi) - 1);
        strncpy(reportRcdRequest.data[i].time, "2015-04-13 21:22:04", sizeof(reportRcdRequest.data[i].time) - 1);
        reportRcdRequest.data[i].in = 0;
        reportRcdRequest.data[i].out = 0;
        reportRcdRequest.data[i].strand = 0;
        reportRcdRequest.data[i].flag = 1;
    }   

    while (1)
    {
        usleep(100 * 1000);

        if (g_exitFlag == 1)
        {
            TRACE("* platform test program exit. %s %d\r\n", MDL);
            break;
        }
        
        if (count % 100 == 0)
        {
            g_timeNow = time(NULL);    
        }

        count++;
        
        if (g_authFlag == 0)
        {
            if (MRJpf_auth() == 0)
            {   
                TRACE("* auth success. %s %d\r\n", MDL);
                g_authFlag = 1;
            }
        }

        if (g_authFlag == 1)
        {
            if (g_loginFlag == 0)
            {
                if (MRJpf_login() == 0)
                {   
                    TRACE("* login success. %s %d\r\n", MDL);
                    g_loginFlag = 1;
                }
            }
        }

        if (g_authFlag == 0 || g_loginFlag == 0)
        {
            continue;
        }

        if (count % 2 == 0)
        {
            if (g_timeNow - g_lastReportRcd >= 10)
            {
                g_lastReportRcd = g_timeNow;
#if 0
                struct ReportRcdTest contentp;
                memset(&contentp, 0, sizeof(contentp));
                contentp.contentbuffer = buffer;
                contentp.contentlen = filesize;
                ret = MRJpf_report_record_test(&contentp);
#else
                ret = MRJpf_report_rcd(&reportRcdRequest);
#endif
                
                if (ret != 0)
                {
                    TRACE("> =========== report record failed. ================ %s %d\r\n", MDL);
                }
            }
        } 
        else 
        {
            if (g_timeNow - g_lastReportPhoto >= 10)
            {
                g_lastReportPhoto = g_timeNow;
                ret = MRJpf_report_photo((char *)"photo.jpg", 0L);
                if (ret != 0)
                {
                    TRACE("> =========== report photo failed. ================ %s %d\r\n", MDL);
                }
            }
        }
    }
    
    return 0;
}

