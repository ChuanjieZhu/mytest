
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mrj_json.h"

char * create_login_json_object(const char *sn, const char *ip, const char *name)
{
    cJSON *root;
    char *out = NULL;
    
    root = cJSON_CreateObject();
    if (!root)
    {
        TRACE("> json create array failed. %s %d\r\n", MDL);
        return NULL;
    }
    
    cJSON_AddStringToObject(root, "imei", sn);
    cJSON_AddStringToObject(root, "ip", ip);
    cJSON_AddStringToObject(root, "alias", name);

    out = cJSON_Print(root);
    cJSON_Delete(root);
    
    //printf("%s\n", out);

    return out;
}


char *create_report_rcd_json_object(struct ReportRcdReq *reqp)
{
    int i;
    cJSON *root = NULL;
    cJSON *rcd = NULL;
    char *out = NULL;

    root = cJSON_CreateArray();
    if (!root)
    {
        TRACE("> json create array failed. %s %d\r\n", MDL);
        return NULL;
    }
    
    for (i = 0; i < reqp->num; i++)
    {
        cJSON_AddItemToArray(root, rcd = cJSON_CreateObject());
        cJSON_AddStringToObject(rcd, "imsi", reqp->data[i].imsi);
        cJSON_AddStringToObject(rcd, "time", reqp->data[i].time);
        cJSON_AddNumberToObject(rcd, "trafficIn", reqp->data[i].in);
        cJSON_AddNumberToObject(rcd, "trafficOut", reqp->data[i].out);
        cJSON_AddNumberToObject(rcd, "trafficStrand", reqp->data[i].strand);
        cJSON_AddNumberToObject(rcd, "flag", reqp->data[i].flag);
    }

    out = cJSON_Print(root);	
    cJSON_Delete(root);
    
    //printf("%s\n",out);

    return out;
}

char *create_report_algorithm_json_object(struct algo_obj_item *itemp)
{   
    cJSON *root = NULL;
    char *out = NULL;
    
    root = cJSON_CreateObject();
    if (!root)
    {
        TRACE("> json create array failed. %s %d\r\n", MDL);
        return NULL;
    }
    
    cJSON_AddNumberToObject(root, "state", itemp->result);
    cJSON_AddNumberToObject(root, "configId", itemp->cfgId);
    cJSON_AddNumberToObject(root, "leftPoint", itemp->leftPoint);
    cJSON_AddNumberToObject(root, "rightPoint", itemp->rightPoint);
    cJSON_AddNumberToObject(root, "topPoint", itemp->topPoint);
    cJSON_AddNumberToObject(root, "bottomPoint", itemp->bottomPoint);
    cJSON_AddNumberToObject(root, "direction", itemp->direction);
    cJSON_AddNumberToObject(root, "height", itemp->height);
    cJSON_AddNumberToObject(root, "init", itemp->init);

    out = cJSON_Print(root);
    cJSON_Delete(root);
    
    printf("%s\n", out);

    return out;           
}

char *create_report_seven_json_object(struct seven_obj_item *itemp)
{
    cJSON *root = NULL;
    char *out = NULL;
    
    root = cJSON_CreateObject();
    if (!root)
    {
        TRACE("> json create array failed. %s %d\r\n", MDL);
        return NULL;
    }
    
    cJSON_AddNumberToObject(root, "configId", itemp->configId);
    cJSON_AddNumberToObject(root, "state", itemp->state);
    cJSON_AddNumberToObject(root, "maxLostFrame", itemp->maxLostFrame);
    cJSON_AddNumberToObject(root, "maxDistance", itemp->maxDistance);
    cJSON_AddNumberToObject(root, "maxFrame", itemp->maxFrame);
    cJSON_AddNumberToObject(root, "goodTrackingCount", itemp->goodTrackingCount);
    cJSON_AddNumberToObject(root, "minDisLen", itemp->minDisLen);
    cJSON_AddNumberToObject(root, "minAngle", itemp->minAngle);
    cJSON_AddNumberToObject(root, "disCardOpenFlag", itemp->disCardOpenFlag);
    cJSON_AddNumberToObject(root, "disCardMaxDistance", itemp->disCardMaxDistance);
    
    out = cJSON_Print(root);
    cJSON_Delete(root);
    
    printf("%s\n", out);

    return out;    
}


char *create_report_video_json_object(struct video_obj_item *itemp)
{
    cJSON *root = NULL;
    char *out = NULL;
    
    root = cJSON_CreateObject();
    if (!root)
    {
        TRACE("> json create array failed. %s %d\r\n", MDL);
        return NULL;
    }
    
    cJSON_AddNumberToObject(root, "configId", itemp->configId);
    cJSON_AddNumberToObject(root, "state", itemp->state);
    cJSON_AddNumberToObject(root, "brightness", itemp->brightness);
    cJSON_AddNumberToObject(root, "contrast", itemp->contrast);
    cJSON_AddNumberToObject(root, "hue", itemp->hue);
    cJSON_AddNumberToObject(root, "saturation", itemp->saturation);
    cJSON_AddNumberToObject(root, "exposure", itemp->exposure);
    
    out = cJSON_Print(root);
    cJSON_Delete(root);
    
    printf("%s\n", out);

    return out;    
}

char *create_report_upgrade_json_object(const char *sn, const char *token, const char *version)
{
    cJSON *root = NULL;
    char *out = NULL;
    
    root = cJSON_CreateObject();
    if (!root)
    {
        TRACE("> json create array failed. %s %d\r\n", MDL);
        return NULL;
    }
    
    cJSON_AddStringToObject(root, "imei", sn);
    cJSON_AddStringToObject(root, "token", token);
    cJSON_AddStringToObject(root, "version", version);
    
    out = cJSON_Print(root);
    cJSON_Delete(root);
    
    printf("%s\n", out);

    return out;        
}

char* create_query_upgrade_json_object(const char *type, const char *sn, 
                                       const char *token, const char *version)
{
    cJSON *root = NULL;
    char *out = NULL;
    
    root = cJSON_CreateObject();
    if (!root)
    {
        TRACE("> json create array failed. %s %d\r\n", MDL);
        return NULL;
    }

    cJSON_AddStringToObject(root, "type", type);
    cJSON_AddStringToObject(root, "imei", sn);
    cJSON_AddStringToObject(root, "token", token);
    cJSON_AddStringToObject(root, "version", version);
    
    out = cJSON_Print(root);
    cJSON_Delete(root);
    
    printf("%s\n", out);

    return out;        
}

char * create_report_name_json_object(const char *alias, const char *sn, const char *token)
{
    cJSON *root = NULL;
    char *out = NULL;
    
    root = cJSON_CreateObject();
    if (!root)
    {
        TRACE("> json create array failed. %s %d\r\n", MDL);
        return NULL;
    }
    
    cJSON_AddStringToObject(root, "imei", sn);
    cJSON_AddStringToObject(root, "token", token);
    cJSON_AddStringToObject(root, "alias", alias);
    
    out = cJSON_Print(root);
    cJSON_Delete(root);
    
    printf("%s\n", out);

    return out;    
}


