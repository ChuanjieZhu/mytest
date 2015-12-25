
#ifndef MRJ_JSON_H
#define MRJ_JSON_H

#include <stdio.h>
#include "cJSON.h"
#include "mrj_netplatform.h"

#ifndef TRACE
#define TRACE printf
#endif

#ifndef MDL
#define MDL __FUNCTION__, __LINE__
#endif

struct algo_obj_item
{
    int result;
    int cfgId;
    int leftPoint;
    int topPoint;
    int rightPoint;
    int bottomPoint;
    int direction;
    int height;
    int init;
};

struct seven_obj_item
{
    int configId;
    int state;
    int maxLostFrame;
    int maxDistance;
    int maxFrame;
    int goodTrackingCount;
    int minDisLen;
    int minAngle;
    int disCardOpenFlag;
    int disCardMaxDistance;
};


struct video_obj_item
{
    int configId;
    int state;
    int brightness;
    int contrast;
    int hue;
    int saturation;
    int exposure;
} ;


char *create_login_json_object(const char *sn, const char *ip, const char *name);

char *create_report_rcd_json_object(struct ReportRcdReq *repq);

char *create_report_algorithm_json_object(struct algo_obj_item *itemp);

char *create_report_seven_json_object(struct seven_obj_item *itemp);

char *create_report_video_json_object(struct video_obj_item *itemp);

char *create_report_upgrade_json_object(const char *sn, const char *token, const char *version);

char *create_query_upgrade_json_object(const char *type, const char *sn, 
                                       const char *token, const char *version);

char *create_report_name_json_object(const char *alias, const char *sn, const char *token);

#endif /* MRJ_JSON_H */
