

#include "public.h"
#include "mrj_json.h"
#include "platform.h"

using namespace Json;
using namespace std;

extern struct configLogin g_cfgLogin;

int string_to_json(char *content, Value &jsonValue)
{
    Reader jsonReader;  //json解析 
    string Str = string(content);
    if (!jsonReader.parse(Str, jsonValue)) //解析出json放到json中区  
    {
        return -1;
    }

    return 0;
}

string create_upgrade_json_string()
{
    Value index;

    TRACE("strSN: %s, version: %s. %s %d \r\n", "1011411000014", "1.1.0", __FUNCTION__, __LINE__);
    
    index["type"]    = Value("DVS");
    index["version"] = Value("1.1.0");
    index["imei"]    = Value("1011411000014");
    index["token"]   = Value(g_cfgLogin.token);

    Json::FastWriter writer;
    return writer.write(index);
}


