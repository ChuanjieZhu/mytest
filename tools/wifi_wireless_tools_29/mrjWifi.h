
#ifndef HISI_WIFI_H
#define HISI_WIFI_H

#define TRACE printf
#define MDL __FUNCTION__, __LINE__

#define WIFI_INTERFACE_NAME         "ra0"

#define MAX_WIFI_RESULT     50

/* wifi认证模式类型 */
typedef enum 
{
    AUTH_MODE_WIFI_OPEN         = 0x00,
    AUTH_MODE_WIFI_SHARED       = 0x01,
    AUTH_MODE_WIFI_WPAPSK       = 0x02,
    AUTH_MODE_WIFI_WPA2PSK      = 0x03,
    
} AUTH_MODE_WIFI;   

/* Wifi加密类型 */
typedef enum
{
	ENCRYPTION_TYPE_WIFI_NONE			= 0x00,		/* 无加密 */
	ENCRYPTION_TYPE_WIFI_WEP			= 0x01,		/* wep加密 */
	ENCRYPTION_TYPE_WIFI_AES			= 0x02,		/* CMMP */
	ENCRYPTION_TYPE_WIFI_TKIP		    = 0x03,		/* TKIP */
	
} ENCRYPTION_TYPE_WIFI;


typedef struct tagSCAN_RESULT_WIFI
{
	char					acEssid[32];		/* 最多支持32个字节 */
	int	                    iAuthMode;          /* 安全模式 */
    int                     iEncrypType;        /* 加密类型 */
	int					    cSignalLevel;		/* 信号强度(单位 格)1-5 */
    int                     iSignal;            /* wifi信号强度 */
} SCAN_RESULT_WIFI;


typedef struct _wpa_info_ {
    int token;
    char other_info[128];
    char ie_wpa[32];
    char ie_wpa_group_cipher[32];
    char ie_wpa_pairwise_ciphers[32];
    char ie_wpa_authentication_suites[32];    
} wpa_info;

typedef struct _scan_wifi_result_
{
    int total_scan;
    char essid[64];
    char encryption_key[8];
    char signal_quality[64];
    char ie_wpa_v1[32];
    char ie_wpa_v1_group_cipher[32];
    char ie_wpa_v1_pairwise_ciphers[32];
    char ie_wpa_v1_authentication_suites[32];
    char ie_wpa_v2[32];
    char ie_wpa_v2_group_cipher[32];
    char ie_wpa_v2_pairwise_ciphers[32];
    char ie_wpa_v2_authentication_suites[32];
} scan_wifi_result;


#ifdef __cplusplus
extern "C" {
#endif

int SetIfUp(const char *pIfName);

int GetAccessPointInfoWifi(SCAN_RESULT_WIFI **pptagWifiScanResult);

int CheckWifiFuction();

int IwListScanning(char *dev, scan_wifi_result *result);

int DealWifiSignal(char *essid, char *signalBuf, int buffSize, scan_wifi_result *result, int *pIndex);

int GetWifiAuthModeAndEncryType(char *essid, char *authMode, char *encryType, scan_wifi_result *result, int index);

#ifdef __cplusplus
}
#endif

#endif /* HISI_WIFI_H */


