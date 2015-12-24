


#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
       
#include <iostream>
#include <map>

#include "json/json.h"

using namespace std;
using namespace Json;

#define TRACE printf
#define MDL __FUNCTION__, __LINE__

int JsonArrayParse()
{
    //const char *str = "{\"success\": \"10000\",\"feedbak\": \"12121\",\"tasks\": [{\"taskId\": \"10928\",\"taskcode\": \"base\",\"request\": {\"leftPoint\": \"1\",\"rightPoint\": \"351\"}}]}";

    const char *str = "{\
        \"success\": \"10000\",\
        \"feedbak\": \"\",\
        \"tasks\": [\
        {\
            \"taskId\": \"10928\",\
            \"taskcode\": \"base\",\
            \"request\": {\
                \"leftPoint\": \"1\",\
                \"rightPoint\": \"351\",\
                \"topPoint\": \"48\",\
                \"bottomPoint\": \"240\",\
                \"boxleft\": \"92\",\
                \"boxright\": \"256\",\
                \"boxtop\": \"84\",\
                \"boxbottom\": \"166\",\
                \"direction\": \"0\",\
                \"height\": \"3\",\
                \"init\": \"false\"\
            }\
        },\
        {\
            \"taskId\": \"10929\",\
            \"taskcode\": \"algorithm\",\
            \"request\": {\
                \"maxLostFrame\": \"20\",\
                \"maxDistance\": \"65\",\
                \"maxFrame\": \"3\",\
                \"goodTrackingCount\": \"4\",\
                \"minDisLen\": \"30\",\
                \"minAngle\": \"45\",\
                \"disCardOpenFlag\": \"1\",\
                \"disCardMaxDistance\": \"45\",\
                \"init\": \"false\"\
            }\
        },\
        {\
            \"taskId\": \"10930\",\
            \"taskcode\": \"camera\",\
            \"request\": {\
                \"brightness\": \"2\",\
                \"contrast\": \"3\",\
                \"hue\": \"3\",\
                \"saturation\": \"3\",\
                \"exposure\": \"4\",\
                \"init\": \"false\"\
            }\
        },\
        {\
            \"taskId\": \"10933\",\
            \"taskcode\": \"grap\",\
            \"request\": \"\"\
        },\
        {\
            \"taskId\": \"10932\",\
            \"taskcode\": \"restart\",\
            \"request\": \"\"\
        },\
        {\
            \"taskId\": \"10934\",\
            \"taskcode\": \"netdel\",\
            \"request\": \"\"\
        }\
    ]\
    }";

    TRACE("*** str: %s \r\n", str);

    Value jsonValue;
    Reader jsonReader;  //json解析 
    string Str = string(str);
    if (!jsonReader.parse(Str, jsonValue)) //解析出json放到json中区  
    {
        return -1;
    }

    int taskId = 0;
    char taskCode[32] = {0};
    int returnCode = atoi(jsonValue["success"].asCString());

    TRACE("* ----------------------------------------------------------- %s %d\r\n", MDL);
        
    TRACE("* returnCode = %d. %s %d\r\n", returnCode, MDL);
    
    Value arrayObj = jsonValue["tasks"];
    unsigned int i;
    for (i = 0; i < arrayObj.size(); i++)
    {
        taskId = atoi(arrayObj[i]["taskId"].asCString());
        TRACE("* taskId = %d. %s %d\r\n", taskId, MDL);
        memset(taskCode, 0, sizeof(taskCode));
        strncpy(taskCode, arrayObj[i]["taskcode"].asCString(), sizeof(taskCode) - 1);
        TRACE("* taskCode = %s. %s %d\r\n", taskCode, MDL);
        Value request = arrayObj[i]["request"];
        if (i != arrayObj.size() - 1)
            TRACE("\r\n");
    }

    TRACE("* ----------------------------------------------------------- %s %d\r\n", MDL);
    
    return 0;
}

string JsonArrayCreate()
{
    Value item;
    Value request;
    Value array;
    
    item.clear();
    request.clear();
    item["taskId"]          = 1;
    item["state"]           = 11;
    request["boxleft"]      = 12;
    request["boxright"]     = 13;
    request["boxtop"]       = 14;
    request["boxbottom"]    = 15;
    request["direction"]    = 1;
    request["height"]       = 2;
    item["request"]         = request;
    array.append(item);

   
    item.clear();
    item["taskId"]              = 10001;
    item["state"]               = 2;
    item["request"]             = "";
    array.append(item);         

    string out = array.toStyledString();
    cout << out << endl;

    Json::FastWriter writer;
    return writer.write(array);
}

int JsonStringParse(const char *data)
{   
    /* eg: data = {"success":"10000","feedbak":"\u8bbe\u5907\u5141\u8bb8\u4e0a\u4f20\u6570\u636e","report":"1"} */
    
    size_t len = strlen(data);
    Value jsonValue;    //表示一个json格式的对象
    Reader jsonReader;  //json解析

    string Str = string(data);
    if (!jsonReader.parse(Str, jsonValue)) //解析出json放到json中区  
    {
        return -1;
    }

    if (jsonValue["success"].isNull() ||
        !jsonValue["success"].isString())
    {
        return -1;
    }

    int success = atoi(jsonValue["success"].asCString());
    TRACE("* success = %d. %s %d\r\n", success, MDL);

    if (jsonValue["report"].isNull()
        || !jsonValue["report"].isString())
    {
        return -1;
    }

    int report = atoi(jsonValue["success"].asCString());
    TRACE("* report = %d. %s %d\r\n", report, MDL);

   return 0;
}

int main()
{
    //const char *data = "{\"success\": \"10000\",\"feedbak\": \"12121\",\"tasks\": [{\"taskId\": \"10928\",\"taskcode\": \"base\",\"request\": {\"leftPoint\": \"1\",\"rightPoint\": \"351\"}}]}";
    const char *data = "{\"success\":\"10000\",\"feedbak\":\"\u8bbe\u5907\u5141\u8bb8\u4e0a\u4f20\u6570\u636e\",\"report\":\"1\"}";
    
    /* 创建json数组 */
    JsonArrayCreate();
    
    /* 解析JSON数组 */
    JsonArrayParse();

    JsonStringParse(data);
    
    return 0;
}
