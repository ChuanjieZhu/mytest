
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

#define TRACE printf
#define MDL __FUNCTION__, __LINE__

/* Parse text to JSON, then render back to text, and print! */
void doit(char *text)
{
	char *out;cJSON *json;
	
	json=cJSON_Parse(text);
	if (!json) {printf("Error before: [%s]\n",cJSON_GetErrorPtr());}
	else
	{
		out=cJSON_Print(json);
		cJSON_Delete(json);
		printf("%s\n",out);
		free(out);
	}
}

/* Read a file, parse, render back, etc. */
void dofile(char *filename)
{
	FILE *f=fopen(filename,"rb");fseek(f,0,SEEK_END);long len=ftell(f);fseek(f,0,SEEK_SET);
	char *data=(char*)malloc(len+1);fread(data,1,len,f);fclose(f);
	doit(data);
	free(data);
}

/* Used by some code below as an example datatype. */
struct record {const char *precision;double lat,lon;const char *address,*city,*state,*zip,*country; };

/* Create a bunch of objects as demonstration. */
void create_objects()
{
	cJSON *root,*fmt,*img,*thm,*fld;char *out;int i;	/* declare a few. */

	/* Here we construct some JSON standards, from the JSON site. */
	
	/* Our "Video" datatype: */
	root=cJSON_CreateObject();	
	cJSON_AddItemToObject(root, "name", cJSON_CreateString("Jack (\"Bee\") Nimble"));
	cJSON_AddItemToObject(root, "format", fmt=cJSON_CreateObject());
	cJSON_AddStringToObject(fmt,"type",		"rect");
	cJSON_AddNumberToObject(fmt,"width",		1920);
	cJSON_AddNumberToObject(fmt,"height",		1080);
	cJSON_AddFalseToObject (fmt,"interlace");
	cJSON_AddNumberToObject(fmt,"frame rate",	24);
	
	out=cJSON_Print(root);	
    cJSON_Delete(root);	
    printf("%s\n",out);	
    free(out);	/* Print to text, Delete the cJSON, print it, release the string. */

	/* Our "days of the week" array: */
	const char *strings[7]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
	root=cJSON_CreateStringArray(strings,7);

	out=cJSON_Print(root);	cJSON_Delete(root);	printf("%s\n",out);	free(out);

	/* Our matrix: */
	int numbers[3][3]={{0,-1,0},{1,0,0},{0,0,1}};
	root=cJSON_CreateArray();
	for (i=0;i<3;i++) cJSON_AddItemToArray(root,cJSON_CreateIntArray(numbers[i],3));

/*	cJSON_ReplaceItemInArray(root,1,cJSON_CreateString("Replacement")); */
	
	out=cJSON_Print(root);	cJSON_Delete(root);	printf("%s\n",out);	free(out);


	/* Our "gallery" item: */
	int ids[4]={116,943,234,38793};
	root=cJSON_CreateObject();
	cJSON_AddItemToObject(root, "Image", img=cJSON_CreateObject());
	cJSON_AddNumberToObject(img,"Width",800);
	cJSON_AddNumberToObject(img,"Height",600);
	cJSON_AddStringToObject(img,"Title","View from 15th Floor");
	cJSON_AddItemToObject(img, "Thumbnail", thm=cJSON_CreateObject());
	cJSON_AddStringToObject(thm, "Url", "http:/*www.example.com/image/481989943");
	cJSON_AddNumberToObject(thm,"Height",125);
	cJSON_AddStringToObject(thm,"Width","100");
	cJSON_AddItemToObject(img,"IDs", cJSON_CreateIntArray(ids,4));

	out=cJSON_Print(root);	cJSON_Delete(root);	printf("%s\n",out);	free(out);

	/* Our array of "records": */
	struct record fields[2]={
		{"zip",37.7668,-1.223959e+2,"","SAN FRANCISCO","CA","94107","US"},
		{"zip",37.371991,-1.22026e+2,"","SUNNYVALE","CA","94085","US"}};

	root=cJSON_CreateArray();
	for (i=0;i<2;i++)
	{
		cJSON_AddItemToArray(root,fld=cJSON_CreateObject());
		cJSON_AddStringToObject(fld, "precision", fields[i].precision);
		cJSON_AddNumberToObject(fld, "Latitude", fields[i].lat);
		cJSON_AddNumberToObject(fld, "Longitude", fields[i].lon);
		cJSON_AddStringToObject(fld, "Address", fields[i].address);
		cJSON_AddStringToObject(fld, "City", fields[i].city);
		cJSON_AddStringToObject(fld, "State", fields[i].state);
		cJSON_AddStringToObject(fld, "Zip", fields[i].zip);
		cJSON_AddStringToObject(fld, "Country", fields[i].country);
	}
	
/*	cJSON_ReplaceItemInObject(cJSON_GetArrayItem(root,1),"City",cJSON_CreateIntArray(ids,4)); */
	
	out=cJSON_Print(root);	cJSON_Delete(root);	printf("%s\n",out);	free(out);

}

typedef struct Task_Element
{
    int taskid;
    char taskcode[32];
    char request[1024];
} TASK_ELEMENT;

static int ParaseTasksResult(cJSON *jsonArrayItem, TASK_ELEMENT *result)
{   
    char *taskString = NULL;
    char *reqString = NULL;
    cJSON *jsonRoot;
    cJSON *jsonValue;
    
    if (!jsonArrayItem || !result)
    {

        TRACE("> ERROR, param is NULL. %s %d\r\n", MDL);
        goto done;
    }

    memset(result, 0, sizeof(*result));
    
    taskString = cJSON_Print(jsonArrayItem);
    if (taskString == NULL)
    {
        TRACE("> ERROR, cJSON_Print fail. %s %d\r\n", MDL);
        goto done;
    }

    jsonRoot = cJSON_Parse(taskString);
    if (jsonRoot == NULL)
    {   
        TRACE("> ERROR, cJSON_Parse fail. %s %d\r\n", MDL);
        goto done; 
    }
    
    /* taskId */
    jsonValue = cJSON_GetObjectItem(jsonRoot, "taskId");
    if (jsonValue == NULL || jsonValue->valuestring == NULL)
    {
        TRACE("> ERROR, element's taskId is invalid. %s %d\r\n", MDL);
        goto done; 
    }
    result->taskid = atoi(jsonValue->valuestring);

    /* taskcode */
    jsonValue = cJSON_GetObjectItem(jsonRoot, "taskcode");
    if (jsonValue == NULL || jsonValue->valuestring == NULL)
    {
        TRACE("> ERROR, element's taskcode is invalid. %s %d\r\n", MDL);
        goto done; 
    }

    if (strlen(jsonValue->valuestring) >= sizeof(result->taskcode))
    {
        TRACE("> ERROR, taskcode is too long. %s %d\r\n", MDL);
        goto done;     
    }
    strncpy(result->taskcode, jsonValue->valuestring, sizeof(result->taskcode) - 1);

    /* request */
    jsonValue = cJSON_GetObjectItem(jsonRoot, "request");
    if (jsonValue == NULL)
    {
        TRACE("> ERROR, element's request is invalid. %s %d\r\n", MDL);
        goto done; 
    }

    TRACE("-----------------------------------. %s %d\r\n", MDL);

    reqString = cJSON_Print(jsonValue);
    if (reqString == NULL)
    {
        TRACE("> ERROR, cJSON_Print fail. %s %d\r\n", MDL);
        goto done;
    }
        
    if (strlen(reqString) >= sizeof(result->request))
    {
        TRACE("> ERROR, request is too long. %s %d\r\n", MDL);
        goto done;    
    }
    strncpy(result->request, reqString, sizeof(result->request) - 1);

    TRACE("* taskid: %d %s %d\r\n", result->taskid, MDL);
    TRACE("* taskcode: %s %s %d\r\n", result->taskcode, MDL);
    TRACE("* request: %s %s %d\r\n", result->request, MDL);

done:
    free(reqString);
    free(taskString);
    cJSON_Delete(jsonRoot);
        
    return 0;
}

int jsonArrayParseTest()
{
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

    //TRACE("*** str: %s \r\n", str);

    cJSON *jsonRoot;
    cJSON *jsonValue;
    cJSON *jsonTasks;
    cJSON *jsonArrayItem;
    char *taskString = NULL;
    int success = 0;
    unsigned int count = 0;

    jsonRoot = cJSON_Parse(str);
    if (jsonRoot == NULL)
    {
        TRACE("> ERROR, json parse(%s) fail. %s %d\r\n", cJSON_GetErrorPtr(), MDL); 
        goto down;
    }

    /* success */
    jsonValue = cJSON_GetObjectItem(jsonRoot, "success");
    if (jsonValue == NULL || jsonValue->valuestring == NULL)
    {
        TRACE("> ERROR, json elememt's success is null. %s %d\r\n", MDL);
        goto down;
    }
    
    success = atoi(jsonValue->valuestring);
    TRACE("* tasks return success: %d %s %d\r\n", success, MDL);

    /* tasks */
    jsonValue = cJSON_GetObjectItem(jsonRoot, "tasks");
    if (jsonValue == NULL)
    {
        TRACE("> ERROR, json elememt's tasks is null. %s %d\r\n", MDL);
        goto down;
    }

    taskString = cJSON_Print(jsonValue);
    if (taskString == NULL)
    {
        TRACE("> ERROR, tasks json to string fail. %s %d\r\n", MDL);
        goto down;
    }

    /* ½âÎötasksÊý×é */
    jsonTasks = cJSON_Parse(taskString);
    if (jsonTasks == NULL)
    {
        TRACE("> ERROR, json parse(%s) fail. %s %d\r\n", cJSON_GetErrorPtr(), MDL);
        goto down;
    }
        
    count = cJSON_GetArraySize(jsonTasks);
    
    TRACE("* tasks count: %d %s %d\r\n", count, MDL); 

    TRACE("* ----------------------------------------------------------- %s %d\r\n", MDL);

    TASK_ELEMENT result; 
        
    unsigned int i;
    for (i = 0; i < count; i++)
    {   
        jsonArrayItem = cJSON_GetArrayItem(jsonTasks, i);
        if (jsonArrayItem == NULL)
        {
            continue;
        }
        
        if (ParaseTasksResult(jsonArrayItem, &result) != 0)
        {
            continue;
        }
        /*
        TRACE("* taskId = %d. %s %d\r\n", result.taskid, MDL);
        TRACE("* taskCode = %s. %s %d\r\n", result.taskcode, MDL);
        TRACE("* request = %s. %s %d\r\n", result.request, MDL);
        */
    }

    TRACE("* ----------------------------------------------------------- %s %d\r\n", MDL);

down:

    free(taskString);
    cJSON_Delete(jsonTasks);
    cJSON_Delete(jsonRoot);
    
    return 0;
}

static int jsonArrayCreateBase(cJSON *root)
{
    int ret = -1;
    cJSON *jsonVuale;
    cJSON *jsonReq;
    char *reqstring = NULL;

    if (!root)
    {
        TRACE("> ERROR, param is NULL. %s %d\r\n", MDL);
        goto done;
    }
    
    jsonVuale = cJSON_CreateObject();
    if (!jsonVuale)
    {
        TRACE("> ERROR, json create response object fail. %s %d\r\n", MDL);
        goto done;
    }

    jsonReq = cJSON_CreateObject();
    cJSON_AddNumberToObject(jsonReq, "boxleft", 1);
    cJSON_AddNumberToObject(jsonReq, "boxright", 1);
    cJSON_AddNumberToObject(jsonReq, "boxtop", 1);
    cJSON_AddNumberToObject(jsonReq, "boxbottom", 1);
    cJSON_AddNumberToObject(jsonReq, "direction", 1);
    cJSON_AddNumberToObject(jsonReq, "height", 1);

    reqstring = cJSON_Print(jsonReq);
    if (reqstring == NULL)
    {
        TRACE("> ERROR, cJSON_Print fail. %s %d\r\n", MDL);
        cJSON_Delete(jsonVuale);
        goto done;
    }

    cJSON_Delete(jsonReq);

    cJSON_AddNumberToObject(jsonVuale, "taskid", 10000);
    cJSON_AddNumberToObject(jsonVuale, "state", 1);
    cJSON_AddStringToObject(jsonVuale, "request", reqstring);
    cJSON_AddItemToArray(root, jsonVuale);

    ret = 0;
    
done:
    
    free(reqstring);
    reqstring = NULL;
    
    return ret;
}

static int jsonArrayCreateAlgo(cJSON *root)
{
    int ret = -1;
    cJSON *jsonValue;
    cJSON *jsonReq;
    char *reqstring = NULL;

    if (!root)
    {
        TRACE("> ERROR, param is NULL. %s %d\r\n", MDL);
        goto done;
    }      

    jsonReq = cJSON_CreateObject();
    if (!jsonReq)
    {
        TRACE("> ERROR, json create jsonReq object fail. %s %d\r\n", MDL);
        goto done;
    }
    
    cJSON_AddNumberToObject(jsonReq, "maxLostFrame", 1);
    cJSON_AddNumberToObject(jsonReq, "maxDistance", 1);
    cJSON_AddNumberToObject(jsonReq, "maxFrame", 1);
    cJSON_AddNumberToObject(jsonReq, "goodTrackingCount", 1);
    cJSON_AddNumberToObject(jsonReq, "minDisLen", 1);
    cJSON_AddNumberToObject(jsonReq, "minAngle", 1);
    cJSON_AddNumberToObject(jsonReq, "disCardOpenFlag", 1);
    cJSON_AddNumberToObject(jsonReq, "disCardMaxDistance", 1);

    reqstring = cJSON_Print(jsonReq);
    if (reqstring == NULL)
    {
        TRACE("> ERROR, cJSON_Print fail. %s %d\r\n", MDL);
        cJSON_Delete(jsonReq);
        goto done;
    }

    cJSON_Delete(jsonReq);
    
    jsonValue = cJSON_CreateObject();
    if (!jsonValue)
    {
        TRACE("> ERROR, json create jsonValue object fail. %s %d\r\n", MDL);
        goto done;
    } 
    
    cJSON_AddNumberToObject(jsonValue, "taskid", 10001);
    cJSON_AddNumberToObject(jsonValue, "state", 1);
    cJSON_AddStringToObject(jsonValue, "request", reqstring);
    cJSON_AddItemToArray(root, jsonValue);

    ret = 0;
    
done:

    free(reqstring);
    reqstring = NULL;
    
    return ret;    
}

static int jsonArrayCreateGrap(cJSON *root)
{
    int ret = -1;
    cJSON *jsonValue;
    cJSON *jsonReq;
    char *reqstring = NULL;

    if (!root)
    {
        TRACE("> ERROR, param is NULL. %s %d\r\n", MDL);
        goto done;
    }
    
    jsonReq = cJSON_CreateObject();
    if (!jsonReq)
    {
        TRACE("> ERROR, json create jsonReq object fail. %s %d\r\n", MDL);
        goto done;   
    }
    
    cJSON_AddStringToObject(jsonReq, "image", "ZSDFAadsfxasdfaasdfasdf");
   
    reqstring = cJSON_Print(jsonReq);
    if (reqstring == NULL)
    {
        TRACE("> ERROR, cJSON_Print fail. %s %d\r\n", MDL);
        cJSON_Delete(jsonReq);
        goto done;
    }

    cJSON_Delete(jsonReq);

    jsonValue = cJSON_CreateObject();
    if (!jsonValue)
    {
        TRACE("> ERROR, json create jsonValue object fail. %s %d\r\n", MDL);
        goto done;
    }
        
    cJSON_AddNumberToObject(jsonValue, "taskid", 10001);
    cJSON_AddNumberToObject(jsonValue, "state", 1);
    cJSON_AddStringToObject(jsonValue, "request", reqstring);
    cJSON_AddItemToArray(root, jsonValue);

    ret = 0;
    
done:

    free(reqstring);
    reqstring = NULL;
    
    return ret;
}

static char *jsonArrayCreateTest()
{
    cJSON *root;
    char *out = NULL;

    root = cJSON_CreateArray();
    if (root == NULL)
    {   
        TRACE("> ERROR, create array fail. %s %d\r\n", MDL);
        return NULL;
    }       
    
    if (jsonArrayCreateBase(root) != 0)
    {
        TRACE("> ERROR, create array fail. %s %d\r\n", MDL);
        cJSON_Delete(root);
        return NULL;   
    }
            
    if (jsonArrayCreateAlgo(root) != 0)
    {
        TRACE("> ERROR, create array fail. %s %d\r\n", MDL);
        cJSON_Delete(root);
        return NULL;      
    }
    
    if (jsonArrayCreateGrap(root) != 0)
    {
        TRACE("> ERROR, create array fail. %s %d\r\n", MDL);
        cJSON_Delete(root);
        return NULL;    
    }
    
    out = cJSON_Print(root);
    cJSON_Delete(root);

    TRACE("* out: %s %s %d\r\n", out, MDL);
    
    return out;
}

int main (int argc, const char * argv[]) 
{

#if 0   
	/* a bunch of json: */
	char text1[]="{\n\"name\": \"Jack (\\\"Bee\\\") Nimble\", \n\"format\": {\"type\":       \"rect\", \n\"width\":      1920, \n\"height\":     1080, \n\"interlace\":  false,\"frame rate\": 24\n}\n}";	
	char text2[]="[\"Sunday\", \"Monday\", \"Tuesday\", \"Wednesday\", \"Thursday\", \"Friday\", \"Saturday\"]";
	char text3[]="[\n    [0, -1, 0],\n    [1, 0, 0],\n    [0, 0, 1]\n	]\n";
	char text4[]="{\n		\"Image\": {\n			\"Width\":  800,\n			\"Height\": 600,\n			\"Title\":  \"View from 15th Floor\",\n			\"Thumbnail\": {\n				\"Url\":    \"http:/*www.example.com/image/481989943\",\n				\"Height\": 125,\n				\"Width\":  \"100\"\n			},\n			\"IDs\": [116, 943, 234, 38793]\n		}\n	}";
	char text5[]="[\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":  37.7668,\n	 \"Longitude\": -122.3959,\n	 \"Address\":   \"\",\n	 \"City\":      \"SAN FRANCISCO\",\n	 \"State\":     \"CA\",\n	 \"Zip\":       \"94107\",\n	 \"Country\":   \"US\"\n	 },\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":  37.371991,\n	 \"Longitude\": -122.026020,\n	 \"Address\":   \"\",\n	 \"City\":      \"SUNNYVALE\",\n	 \"State\":     \"CA\",\n	 \"Zip\":       \"94085\",\n	 \"Country\":   \"US\"\n	 }\n	 ]";

	/* Process each json textblock by parsing, then rebuilding: */
	doit(text1);
	doit(text2);	
	doit(text3);
	doit(text4);
	doit(text5);

	/* Parse standard testfiles: */
/*	dofile("../../tests/test1"); */
/*	dofile("../../tests/test2"); */
/*	dofile("../../tests/test3"); */
/*	dofile("../../tests/test4"); */
/*	dofile("../../tests/test5"); */

	/* Now some samplecode for building objects concisely: */
	create_objects();
#endif

    jsonArrayParseTest();

    jsonArrayCreateTest();
	
	return 0;
}
