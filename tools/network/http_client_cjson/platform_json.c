
#include <stdio.h>
#include <stdlib.h>

#include "cJSON.h"
#include "platform_json.h"

void free_json_string(char *json_string)
{
    if (!json_string)
    {
        free(json_string);
        json_string = NULL;
    }
}

char *create_login_json_string()
{
    cJSON *root;
    char *out = NULL;

    root = cJSON_CreateObject();
    if (!root)
    {
        TRACE("> ERROR, cJSON create object failed. %s %d\r\n", MDL);
        return NULL;
    }

    cJSON_AddStringToObject(root, "imei",       "1011411000014");
    cJSON_AddStringToObject(root, "ip",         "192.168.8.101");
    cJSON_AddStringToObject(root, "alias",      "http_test_client");
    cJSON_AddStringToObject(root, "version",    "1.5.0");      /* 带上版本号 */
    cJSON_AddStringToObject(root, "model",      "M1");              /* 设备型号 */
    cJSON_AddStringToObject(root, "nettype",    "wifi");
    cJSON_AddStringToObject(root, "netname",    "xiaozhou_tenda");
    cJSON_AddStringToObject(root, "netpwd",     "xiaozhou@biz");
    
    out = cJSON_Print(root);
    cJSON_Delete(root);

    return out;
}


