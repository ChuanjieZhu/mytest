
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mrjWifi.h"

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


int GetWifiAuthModeAndEncryptTypeByScanResult(char *essid, char *authMode, 
                char *encyType, scan_wifi_result *result)
{
    int ret = -1;
    
    if (essid == NULL 
        || result == NULL
        || authMode == NULL
        || encyType == NULL)
    {
        TRACE("> get wifi scan param fail, param is NULL. %s %d\r\n", MDL);
        return ret;
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
        TRACE("> wifi scan fail, not find ssid(%s) in scan result. %s %d\r\n", essid, MDL);
        return -1;
    }

    TRACE("* find ssid(%s) scan result success. %s %d\r\n", essid, MDL);
    PrintWifiScanResult(&tmpResult);

    /* 启动了wifi密码 */
    if (strcmp(tmpResult.encryption_key, "on") == 0)
    {   
        /* OPEN/SHARE加密方式 */
        if (tmpResult.ie_wpa_v1[0] == '\0' && tmpResult.ie_wpa_v2[0] == '\0')
        {
            *authMode = 1;
            *encyType = 1;
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
                *encyType = 2;
                ret = 0;
            }
            else if (strcmp(tmpResult.ie_wpa_v1_group_cipher, "TKIP") == 0) 
            {
                *encyType = 3; 
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
                *encyType = 2;
                ret = 0;
            }
            else if (strcmp(tmpResult.ie_wpa_v2_group_cipher, "TKIP") == 0)
            {
                *encyType = 3; 
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
                *encyType = 2;
                ret = 0;
            }
            else if (strcmp(tmpResult.ie_wpa_v2_group_cipher, "TKIP") == 0)
            {
                *encyType = 3; 
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
        *encyType = 0;
        ret = 0;    
    }
    else
    {
        TRACE("> unsuport wifi type. %s %d\r\n", MDL);
        ret = -1;    
    }

    return ret;
}

int main(int argc, char *argv[])
{
    int ret = -1;

    ret = SetIfUp(WIFI_INTERFACE_NAME);
    if (ret != 0) {
        TRACE("> set wifi interface(%s) up fail. %s %d\r\n", WIFI_INTERFACE_NAME, MDL);
        return -1;
    }

    static scan_wifi_result scanResult[MAX_WIFI_RESULT];
    memset(&scanResult, 0, MAX_WIFI_RESULT * sizeof(scan_wifi_result));

    int index = -1;
    char authMode = -1;
    char encyType = -1;
    char essid[64] = {0};
    char signal[32] = {0};

    if (argc < 2)
        strncpy(essid, "XIAOZHOU_TENDA", sizeof(essid) - 1);
    else
        strncpy(essid, argv[1], sizeof(essid) - 1);

    printf("* ssid = %s \r\n", essid);    

    ret = DealWifiSignal(essid, signal, sizeof(signal), scanResult, &index);
    if (ret != 0)
    {
         TRACE("> get ssid(%s) signal fail. %s %d\r\n", essid, MDL);
         return -1;
    }

    TRACE("* get ssid(%s) signal ok, signal = %s, index = %d. %s %d\r\n", 
        essid, signal, index, MDL);

    ret = GetWifiAuthModeAndEncryType(essid, &authMode, &encyType, scanResult, index);
    if (ret != 0)
    {
        TRACE("> get ssid(%s) auth and encryType fail. %s %d\r\n", 
            essid, MDL);

        return -1;           
    }

    TRACE("* get ssid(%s) auth and encryType ok, auth = %d, encryType = %d. %s %d\r\n", 
            essid, authMode, encyType, MDL);
    
#if 0    
    int i = 0;
    int scan_num = result->total_scan;
    TRACE("* total_scan = %d \r\n", scan_num);

#if 0
    for (i = 0; i < scan_num; i++)
    {
        TRACE("----------------------------------------------------------- \r\n");
        TRACE("index = %d \r\n", i);
        PrintWifiScanResult(&result[i]);
    }
#else

    char ssid[64] = {0};

    if (argc < 2)
        strncpy(ssid, "XIAOZHOU_TENDA", sizeof(ssid) - 1);
    else
        strncpy(ssid, argv[1], sizeof(ssid) - 1);

    printf("---------- ssid = %s \r\n", ssid);
    
    char authMode = -1;
    char encyType = -1;

    ret = GetWifiAuthModeAndEncryptTypeByScanResult(ssid, &authMode, &encyType, result);
    if (ret != 0)
    {
        TRACE("> get auth mode and encrytion fail. %s %d\r\n", MDL); 
        return -1;
    }

    TRACE("* authMode = %d, encyType = %d %s %d\r\n", authMode, encyType, MDL);
    
#if 0
    for (i = 0; i < scan_num; i++)
    {
        if (strcmp(result[i].essid, ssid) == 0) 
        {
            TRACE("index = %d \r\n", i);
            PrintWifiScanResult(&result[i]);
            break;
        }
    }
    
    if (i >= scan_num)
        printf("----- not find ssid(%s) wifi. \r\n", ssid);
#endif
    
#endif
#endif

    return ret;
}

