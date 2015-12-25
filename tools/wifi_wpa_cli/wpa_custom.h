
#ifndef WPA_CLI_CUSTOM_H
#define WPA_CLI_CUSTOM_H

#define MAX_SCAN_RESULT_NUM     50

typedef struct wpa_cli_scan_result {
    char bssid[24];
    int freq;
    int level;
    char flags[128];
    char ssid[64];
} wpa_cli_scan_result;

typedef struct my_wpa_cli_scan_result {
    int count;
    wpa_cli_scan_result results[MAX_SCAN_RESULT_NUM];
    
} my_wpa_cli_scan_result;

#endif /* WPA_CLI_CUSTOM_H */

