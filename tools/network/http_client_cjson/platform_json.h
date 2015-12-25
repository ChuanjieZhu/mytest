
#ifndef _MRJ_JSON_H_
#define _MRJ_JSON_H_

#ifndef TRACE
#define TRACE printf
#define MDL __FUNCTION__, __LINE__
#endif


#ifdef __cplusplus
extern "C" {
#endif

void free_json_string(char *json_string);

char *create_login_json_string();

#ifdef __cplusplus
}
#endif

#endif /* _MRJ_JSON_H_ */

