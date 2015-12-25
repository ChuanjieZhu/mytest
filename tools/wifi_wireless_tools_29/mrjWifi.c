
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/route.h>
#include <net/if.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>

#include "mrjWifi.h"

#define WIFI_SCAN_TIMES 3

int SetIfUp(const char *pIfName)
{
    int s;  
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)  
    {  
        perror("Socket");  
        return -1;  
    }  
    
    struct ifreq ifr;  
    strcpy(ifr.ifr_name, pIfName); 

    short flag;  
    flag = IFF_UP;
    if (ioctl(s, SIOCGIFFLAGS, &ifr) < 0)
    {  
        TRACE("ioctl error(%s)! %s %d\r\n", strerror(errno), MDL);  
        return -1;  
    }
    
    ifr.ifr_ifru.ifru_flags |= flag;  
    if (ioctl(s, SIOCSIFFLAGS, &ifr) < 0)  
    {  
        TRACE("ioctl error(%s)! %s %d\r\n", strerror(errno), MDL);  
        return -1;  
    }
    
    return 0; 
}


int CheckWifiFuction()
{
	int iRet = -1;
	SCAN_RESULT_WIFI *ptagWifiScanResult = NULL;
    
	iRet = GetAccessPointInfoWifi(&ptagWifiScanResult);

	if(iRet >= 0)
	{
		if(NULL != ptagWifiScanResult)
		{
			if(0 < iRet)
			{
                /* 搜索到wifi热点个数大于0 */
                TRACE("Have scan wifi num %d! %s %d\r\n", iRet, MDL);
			}
			else if (0 == iRet)
			{
				/* 当前无Wifi热点 */
                TRACE("Have no found wifi! %s %d\r\n", MDL);
				iRet = 0;
			}
			else
			{
				/* 取得的Wifi热点个数有问题 */
                TRACE("Have found wifi number error! %s %d\r\n", MDL);
				iRet = -3;
			}

			/* 务必保证删除已经分配的空间 */
			free(ptagWifiScanResult);
			ptagWifiScanResult = NULL;
		}
		else
		{
			/* 底层函数分配空间失败 */
            TRACE("Scan wifi malloc error! %s %d\r\n", MDL);
			iRet = -2;
		}
	}
	else
	{
        TRACE("Scan wifi error %d %s %d\r\n", iRet, MDL);
		/* 取得原始Wifi热点结果失败 */
		iRet = -1;
	}
    
	return iRet;
}

static void PrintWifiScanResult(scan_wifi_result *pInfo)
{
    char buffer[256] = {0};

    TRACE("* ----------------------------------------------------------- \r\n");
    snprintf(buffer, sizeof(buffer) - 1, "* %s, %s, %s\r\n", 
        pInfo->essid, pInfo->encryption_key, pInfo->signal_quality);

    TRACE("%s", buffer);
    
    if (strcmp(pInfo->encryption_key, "on") == 0)
    {
        if (pInfo->ie_wpa_v1[0] != '\0')
        {
            snprintf(buffer, sizeof(buffer) - 1, "* %s, %s, %s, %s\r\n", 
                pInfo->ie_wpa_v1, pInfo->ie_wpa_v1_group_cipher, 
                pInfo->ie_wpa_v1_pairwise_ciphers, pInfo->ie_wpa_v1_authentication_suites);

            TRACE("%s", buffer);
        }

        if (pInfo->ie_wpa_v2[0] != '\0')
        {   
            snprintf(buffer, sizeof(buffer) - 1, "* %s, %s, %s, %s\r\n", 
                pInfo->ie_wpa_v2, pInfo->ie_wpa_v2_group_cipher, 
                pInfo->ie_wpa_v2_pairwise_ciphers, pInfo->ie_wpa_v2_authentication_suites);

            TRACE("%s", buffer);
        }
    }
    
    TRACE("* ----------------------------------------------------------- \r\n");
}

int GetWifiSignal(char *essid, char *signalBuf, int buffSize, scan_wifi_result *result, int *index)
{   
    if (essid == NULL
        || signalBuf == NULL
        || result == NULL
        || index == NULL)
    {
        TRACE("> get ssid(%s) signal fail, param is NULL. %s %d\r\n", essid, MDL);
        return -1;
    }

    int i = 0;
    int scanNum = result->total_scan;
    scan_wifi_result tmpResult;
    
    for (i = 0; i < scanNum; i++)
    {
        if (strcmp(result[i].essid, essid) == 0) 
        {
            memset(&tmpResult, 0, sizeof(scan_wifi_result));
            memcpy(&tmpResult, &result[i], sizeof(scan_wifi_result));
            break;
        }
    }

    if (i >= scanNum)
    {
        TRACE("> get wifi signal fail, not find ssid(%s) in scan result. %s %d\r\n", essid, MDL);
        return -1;
    }

    TRACE("* find ssid(%s) scan result success. %s %d\r\n", essid, MDL);
    PrintWifiScanResult(&tmpResult);

    memset(signalBuf, 0, buffSize);
    strncpy(signalBuf, tmpResult.signal_quality, buffSize - 1);

    *index = i;
    
    return 0;
}


int DealWifiScanning(scan_wifi_result *result)
{
    int i;
    int ret = -1;

    if (result == NULL)
    {
        TRACE("> get wifi scan result fail, param is NULL. %s %d\r\n", MDL);
        return -1;
    }

    for (i = 0; i < WIFI_SCAN_TIMES - 1; i++)
    {
        ret = IwListScanning((char *)WIFI_INTERFACE_NAME, result); 
        if (ret == 0)
        {
            break;
        }
    }

    return ret;
}

int GetWifiAuthModeAndEncryType(char *essid, char *authMode, char *encryType, scan_wifi_result *result, int index)
{
    int ret = -1;
    
    if (essid == NULL 
        || result == NULL
        || authMode == NULL
        || encryType == NULL)
    {
        TRACE("> get wifi scan param fail, param is NULL. %s %d\r\n", MDL);
        return ret;
    }

    int scanNum = result->total_scan;
    if (index < 0 || index >= scanNum)
    {
        TRACE("> get wifi scan param fail, index(%d/%d) is invalid. %s %d\r\n", index, scanNum, MDL);
        return -1;
    }
    
    scan_wifi_result tmpResult;
    memset(&tmpResult, 0, sizeof(scan_wifi_result));
    memcpy(&tmpResult, &result[index], sizeof(scan_wifi_result));

    TRACE("* find ssid(%s) scan result success. %s %d\r\n", essid, MDL);
    PrintWifiScanResult(&tmpResult);

    /* 启动了wifi密码 */
    if (strcmp(tmpResult.encryption_key, "on") == 0)
    {   
        /* OPEN/SHARE加密方式 */
        if (tmpResult.ie_wpa_v1[0] == '\0' && tmpResult.ie_wpa_v2[0] == '\0')
        {
            *authMode = 1;
            *encryType = 1;
            ret = 0;
        }
        /* WPAPSK加密方式 */
        else if ((tmpResult.ie_wpa_v1[0] != '\0' 
                && strcmp(tmpResult.ie_wpa_v1, "WPA") == 0
                && strcmp(tmpResult.ie_wpa_v1_authentication_suites, "PSK") == 0
                && tmpResult.ie_wpa_v1_group_cipher[0] != '\0')
            && tmpResult.ie_wpa_v2[0] == '\0')
        {
            *authMode = 2;
            
            if (strcmp(tmpResult.ie_wpa_v1_group_cipher, "CCMP") == 0)
            {
                *encryType = 2;
                ret = 0;
            }
            else if (strcmp(tmpResult.ie_wpa_v1_group_cipher, "TKIP") == 0) 
            {
                *encryType = 3; 
                ret = 0;
            }
            else
            {
                TRACE("> unknown encrypt type(%s). %s %d\r\n", tmpResult.ie_wpa_v1_group_cipher, MDL);
                ret = -1;
            }
        }
        /* WPA2PSK加密方式 */
        else if ((tmpResult.ie_wpa_v2[0] != '\0' 
                && strcmp(tmpResult.ie_wpa_v2, "WPA2") == 0
                && strcmp(tmpResult.ie_wpa_v2_authentication_suites, "PSK") == 0
                && tmpResult.ie_wpa_v2_group_cipher[0] != '\0')
            && tmpResult.ie_wpa_v1[0] == '\0')
        {
            *authMode = 3;
            
            if (strcmp(tmpResult.ie_wpa_v2_group_cipher, "CCMP") == 0)
            {
                *encryType = 2;
                ret = 0;
            }
            else if (strcmp(tmpResult.ie_wpa_v2_group_cipher, "TKIP") == 0)
            {
                *encryType = 3; 
                ret = 0;
            }
            else
            {
                TRACE("> unknown encrypt type(%s). %s %d\r\n", tmpResult.ie_wpa_v2_group_cipher, MDL);
                ret = -1;    
            }
        }
        /* wpapsk/wpa2psk混合加密 */
        else if ((tmpResult.ie_wpa_v1[0] != '\0' 
                && strcmp(tmpResult.ie_wpa_v1, "WPA") == 0
                && strcmp(tmpResult.ie_wpa_v1_authentication_suites, "PSK") == 0
                /* && tmpResult.ie_wpa_v1_group_cipher[0] != '\0' */)
                && (tmpResult.ie_wpa_v2[0] != '\0' 
                && strcmp(tmpResult.ie_wpa_v2, "WPA2") == 0
                && strcmp(tmpResult.ie_wpa_v2_authentication_suites, "PSK") == 0
                && tmpResult.ie_wpa_v2_group_cipher[0] != '\0'))
        {
            *authMode = 3;
            if (strcmp(tmpResult.ie_wpa_v2_group_cipher, "CCMP") == 0)
            {
                *encryType = 2;
                ret = 0;
            }
            else if (strcmp(tmpResult.ie_wpa_v2_group_cipher, "TKIP") == 0)
            {
                *encryType = 3; 
                ret = 0;
            }
            else
            {
                TRACE("> unknown encrypt type(%s). %s %d\r\n", tmpResult.ie_wpa_v2_group_cipher, MDL);
                ret = -1;    
            }
        }
        else
        {
            TRACE("> unsuport wifi type. %s %d\r\n", MDL);
            ret = -1;
        }
    }
    /* 关闭了wifi密码 */
    else if (strcmp(tmpResult.encryption_key, "off") == 0)
    {   
        *authMode = 0;
        *encryType = 0;
        ret = 0;    
    }
    else
    {
        TRACE("> unsuport wifi type. %s %d\r\n", MDL);
        ret = -1;    
    }

    return ret;    
}


int DealWifiSignal(char *essid, char *signalBuf, int buffSize, scan_wifi_result *result, int *pIndex)
{
    int i;
    int ret = -1;

    if (essid == NULL
        || signalBuf == NULL
        || result == NULL
        || pIndex == NULL)
    {
        TRACE("> get ssid(%s) signal fail, param is NULL. %s %d\r\n", essid, MDL);
        return -1;
    }
    
    for (i = 0; i < WIFI_SCAN_TIMES; i++)
    {
        if (DealWifiScanning(result) == 0)
        {
            if (GetWifiSignal(essid, signalBuf, buffSize, result, pIndex) == 0)
            {
                ret = 0;
                break;
            }
        }
    }

    return ret;
}

