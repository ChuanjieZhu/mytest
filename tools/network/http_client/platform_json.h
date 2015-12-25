
#ifndef _MRJ_JSON_H_
#define _MRJ_JSON_H_

#include "json/json.h"

int string_to_json(char *content, Json::Value &jsonValue);
std::string create_upgrade_json_string();


#endif /* _MRJ_JSON_H_ */

