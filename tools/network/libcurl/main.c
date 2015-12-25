
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <map>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "curl.h"
#include "json.h"


using namespace std;
using namespace Json;

#if 0
void PutJson(Value &jsonValue, string paramString, string valueString)
{
    jsonValue[paramString] = Value(valueString);
}


int ToJson(string &content, Value &jsonValue)
{
    Reader jsonReader;
    if (!jsonReader.parse(content, jsonValue))
    {
        return -1;
    }

    return 0;
}


void ToString(Value &jsonValue, string &String)
{
    FastWriter jsonFastwriter;
    String = jsonFastwriter.write(jsonValue);
}


void MapForSend(const map<string, string> content, string &sendContent)
{
    Value jsonValue;

    map<string, string>::const_iterator it;

    for (it = content.begin(); it != content.end(); it++)
    {
        PutJson(jsonValue, it->first, it->second);
    }

    sendContent = ToString(jsonValue);

    //cout << "sendContent: " << sendContent << endl;
}


string TestHttpPost()
{
    map<string, string> contMap;
    contMap.insert(pair<string, string>("name", "chenpan"));
    contMap.insert(pair<string, string>("password", "123456"));

    string sendContent;
    MapForSend(contMap, sendContent);

    return sendContent;
}


string jsonTest()
{
    Value jsonValue;
    PutJson(jsonValue, "name", "chenpan");
    PutJson(jsonValue, "password", "123456");

    string jsonString = ToString(jsonValue);

    return jsonString;
}

struct HttpFile {
  	const char *filename;
  	FILE *stream;
};


static size_t ReadResponse(void *buffer, size_t size, size_t nmemb, void *stream)
{
    int data_size = size * nmemb;
	
	printf("size %d nmemb %d... %s %d\r\n", size, nmemb, __FUNCTION__, __LINE__);
    printf("content: %s \r\n", (char *)buffer);

    if (data_size > 0)
    {
        Value jsonValue;
        string Str = string((char *)buffer);

        cout << "Str: " << Str << endl;
        
        if (0 == ToJson(Str, jsonValue))
        {
            if (jsonValue["success"].isNull() || jsonValue["feedback"].isNull())
            {
                printf("http Server Return Error. %s %d\r\n", __FUNCTION__, __LINE__);
            }
            else
            {
                cout << jsonValue["success"] << endl;
                cout << jsonValue["feedback"] << endl;
            }
        }
        else
        {
            printf("ToJson Error......\r\n");
        }
    }
    
    return data_size;
}    


int curlTest()
{
    CURL *curl = NULL;
    CURLcode res;
    char content[256] = {0};
    char filename[128] = {0};
    struct HttpFile httpFile;

    snprintf(filename, 127, "./%s", "httpResponse");

	httpFile.filename = filename;
	httpFile.stream = NULL;
        
     /* In windows, this will init the winsock stuff */ 
    curl_global_init(CURL_GLOBAL_ALL);
            
    /* get a curl handle */ 
    curl = curl_easy_init();
    
    if (curl != NULL) 
    {
        curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.1.112/TWebservice/index.php");

        /* Now specify the POST data */
        memset(content, 0, sizeof(content));
        
        snprintf(content, sizeof(content) - 1, "data=%s", TestHttpPost().c_str());
        cout << "content: " << content << endl;

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ReadResponse); 
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &httpFile);  
        curl_easy_setopt(curl, CURLOPT_POST, 1);   
    
	    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        
        /* always cleanup */ 
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return 0;
}

#endif 

#define HTTP_SERVER_IP          "192.168.1.112"

void PutJson(Value &jsonValue, string paramString, string valueString)
{
    jsonValue[paramString] = Value(valueString);
}


int StringToJson(char *content, Value &jsonValue)
{
    Reader jsonReader;

    string Str = string(content);
    
    if (!jsonReader.parse(Str, jsonValue))
    {
        return -1;
    }

    return 0;
}


void JsonToString(Value &jsonValue, string &outString)
{
    FastWriter jsonFastwriter;
    outString = jsonFastwriter.write(jsonValue);
}



void MapForSend(const map<string, string> mapContent, string &sendContent)
{
    Value jsonValue;

    map<string, string>::const_iterator it;

    for (it = mapContent.begin(); it != mapContent.end(); it++)
    {
        PutJson(jsonValue, it->first, it->second);
    }

    JsonToString(jsonValue, sendContent);
}


void GetAuthContent(const char *deviceSN, string &sendContent)
{
    map<string, string> contMap;
    
    contMap.insert(pair<string, string>("name", deviceSN));
    contMap.insert(pair<string, string>("password", deviceSN));

    MapForSend(contMap, sendContent);
}


void GetSmartContent(char *contentId, string &sendContent)
{
    ;
}



static size_t ParaseResponse(void *buffer, size_t size, size_t nmemb, void *stream)
{       
    int *iResult = (int *)stream;
    int data_size = size * nmemb;
    string data;
    Value jsonValue;

    if (data_size > 0)
    {   
        if (0 == StringToJson((char *)buffer, jsonValue))
        {
            JsonToString(jsonValue["success"], data);
            if (data.find("10000") != string::npos)
            {
                cout << jsonValue["success"] << jsonValue["feedback"];
                *iResult = 0;
            }
            else
            {
                *iResult = -1;
                printf("Response Fail......\r\n");
            }
        }
        else
        {
            *iResult = -1;
            printf("ToJson Error......\r\n");
        }
    }
    else
    {
        *iResult = -1;
    }
    
    return data_size;
} 


typedef struct Record {
    char mac[32];
    char time[32];
    int in;
    int out;
    int strand;
    char flag;
} RECORD;


typedef struct Upload {
    RECORD rcd[20];
    int num;
} UPLOAD;

UPLOAD upload;

int CreateUpload(int num)
{
    if (num > 20)
        num = 20;

    memset(&upload, 0, sizeof(upload));

    int i;
    for (i = 0; i < num; i++)
    {
        strcpy(upload.rcd[i].time, "2014-10-10 10:00:00"); 
        strcpy(upload.rcd[i].mac, "AA:BB:CC:DD:33:44");
        upload.rcd[i].in = 1;
        upload.rcd[i].out = 1;
        upload.rcd[i].strand = upload.rcd[i].in - upload.rcd[i].out;
        upload.rcd[i].flag = 0;
    }

    upload.num = num;

    return 0;
}

/* 构造json数组 */
string CreateUploadJsonString()
{
    Value pieces;

    int i;
    for (i = 0; i < upload.num; i++)
    {
        Value index;
        index["imsi"] = Value(upload.rcd[i].mac);
        index["time"] = Value(upload.rcd[i].time);
        index["trafficIn"] = upload.rcd[i].in;
        index["trafficOut"] = upload.rcd[i].out;
        index["trafficStrand"] = upload.rcd[i].strand;
        index["flag"] = upload.rcd[i].flag;

        pieces.append(index);
    }

    Json::FastWriter writer;
    return writer.write(pieces);
}




int SendAuthToPlatform(int *iResResult)
{
    CURL *curl = NULL;
    CURLcode res;
    char url[128] = {0};
    char content[256] = {0};
    int iResult = -1;
    string sendContent;

    GetAuthContent("8888888888", sendContent);
      
    res = curl_global_init(CURL_GLOBAL_ALL);
    if (res != CURLE_OK)
    {
       fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
       return -1;
    }
    
    curl = curl_easy_init();
    if (curl == NULL)
    {
        curl_global_cleanup();
        return -1;
    }

    snprintf(url, sizeof(url) - 1, "http://%s/TWebservice/index.php", HTTP_SERVER_IP);
    curl_easy_setopt(curl, CURLOPT_URL, url);

    /* Now specify the POST data */
    memset(content, 0, sizeof(content));
    snprintf(content, sizeof(content) - 1, "data=%s", sendContent.c_str());

    cout << "content: " << content << endl;

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ParaseResponse); 
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &iResult);  
    curl_easy_setopt(curl, CURLOPT_POST, 1);   

    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);

    #ifdef V_DEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    #else
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
    #endif
    
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return -1;
    }
    
    /* always cleanup */ 
    curl_easy_cleanup(curl);

    curl_global_cleanup();

    *iResResult = iResult;

    return 0;
}

#if 0
int SendSmartResultToPlatform(char *Data1, char *Data2, int *Response)
{
    CURL *curl = NULL;
    CURLcode res;
    char url[128] = {0}
    char content[256] = {0};
    int iResult;
        
    res = curl_global_init(CURL_GLOBAL_ALL);
    if (res != CURLE_OK)
    {
       fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
       return -1;
    }
    
    curl = curl_easy_init();
    if (curl == NULL)
    {
        curl_global_cleanup();
        return -1;
    }

    snprintf(url, sizeof(url) - 1, "http://%s/TWebservice/index.php", HTTP_SERVER_IP);
    curl_easy_setopt(curl, CURLOPT_URL, url);

    memset(content, 0, sizeof(content));
    snprintf(content, sizeof(content) - 1, "data=%s", Data1);

    cout << "content: " << content << endl;

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ParaseResponse);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &iResult);  
    curl_easy_setopt(curl, CURLOPT_POST, 1);   

    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return -1;
    }
    
    /* always cleanup */ 
    curl_easy_cleanup(curl);

    curl_global_cleanup();

    return 0;
}
#endif

/* http upload file */
size_t read_file(void *buff, size_t size, size_t nmemb, void *userp)
{
    FILE *fp = (FILE *)userp;
    size_t sizes = fread(buff, size, nmemb, fp);
    return sizes;
}


int check_error(int returnCode)
{
    if (returnCode != 200)
        return -1;

    return 0;
}


int upload_file(const char *url, const char *file_name)
{   
    if (*url == NULL || *file_name == NULL)
    {
        return -1;
    }

    CURLcode res;
    res = curl_global_init(CURL_GLOBAL_ALL);  
    if (res != CURLE_OK)  
    {  
        return -1;  
    }  

    FILE* r_file = fopen(file_name, "rb");  
    if (NULL == r_file)  
    {  
        curl_global_cleanup();
        return -1;  
    }

    double speed_upload, total_time;
    CURL* easy_handle;  
    easy_handle = curl_easy_init();  
    if (0 == easy_handle)  
    {  
        printf("get a easy_handle handle fail!\r\n");  
        fclose(r_file);  
        curl_global_cleanup();  
        curl_easy_cleanup(easy_handle);  
        return -1;  
    }  
  
    // 获取文件大小  
    fseek(r_file, 0, 2);      
    int file_size = ftell(r_file);  
    rewind(r_file);

    printf("file_size: %d \r\n", file_size);
  
    curl_easy_setopt(easy_handle, CURLOPT_URL, url);                    //获取URL地址  
    curl_easy_setopt(easy_handle, CURLOPT_UPLOAD, 1L); 
    curl_easy_setopt(easy_handle, CURLOPT_PUT, 1L);                            //告诉easy_handle是做上传操作  
    curl_easy_setopt(easy_handle, CURLOPT_READDATA, r_file);
    curl_easy_setopt(easy_handle, CURLOPT_INFILESIZE, file_size);         //上传的字节数
    //curl_easy_setopt(easy_handle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_size);    
    
    //curl_easy_setopt(easy_handle, CURLOPT_READFUNCTION, &read_file);    //调用重写的读文件流函数  
    //curl_easy_setopt(easy_handle, CURLOPT_READDATA, r_file);            //往read_file()函数中传入用户自定义的数据类型  
    //curl_easy_setopt(easy_handle, CURLOPT_INFILE, r_file);              //定位作为上传的输入文件  
    //curl_easy_setopt(easy_handle, CURLOPT_INFILESIZE, file_size);         //上传的字节数
    curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(easy_handle);  
    /* Check for errors */ 
    if(res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    }
    else {
      /* now extract transfer info */ 
      curl_easy_getinfo(easy_handle, CURLINFO_SPEED_UPLOAD, &speed_upload);
      curl_easy_getinfo(easy_handle, CURLINFO_TOTAL_TIME, &total_time);
 
      fprintf(stderr, "Speed: %.3f bytes/sec during %.3f seconds\n",
              speed_upload, total_time);
 
    }
    
    printf("============ res: %d \r\n", res);
    
    //获取HTTP错误码  
    int retcode = 0;  
    curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &retcode);  

    printf("============ retcode: %d \r\n", retcode);
    
    fclose(r_file);  
    curl_global_cleanup();  
    curl_easy_cleanup(easy_handle);  
  
    printf("local upload file: %s \r\n", file_name);

    if (res == CURLE_OK && check_error(retcode) == 0)
    {
        printf("upload file success. \r\n");
        return 0;
    }
    else
    {
        printf("upload file fail. \r\n");
        return -1; 
    } 
}


#define     MAX_BUFF_SIZE   2 * 1024 * 1024


typedef struct content {
    char buffer[MAX_BUFF_SIZE];
    int filelen;
}  CONTENT;


size_t process_data(char *buff, size_t size, size_t nmemb, void *userp)
{
    printf("size %d nmemb %d... %s %d\r\n", size, nmemb, __FUNCTION__, __LINE__);

    static int offset = 0;
    CONTENT *cont = (CONTENT *)userp;
    
    int temp = size * nmemb;
    if (temp > 0) 
    {
        cont->filelen += temp;
        memcpy(cont->buffer + offset, buff, temp);
        offset += temp;
    }

    printf("cont->filelen %d... %s %d\r\n", cont->filelen, __FUNCTION__, __LINE__);
    
    //printf("content: %s \r\n", (char *)buff);

    userp = buff;
    return nmemb;
}

const int FILE_EXIST = 200;

int down_file(const char* url, const char* down_file_name)
{  
    CONTENT cont;
    memset(&cont, 0, sizeof(cont));
    
    // 初始化libcurl
    CURLcode return_code;  
    return_code = curl_global_init(CURL_GLOBAL_ALL);
    if (CURLE_OK != return_code)  
    {  
        printf("init libcurl failed. \r\n");  
        curl_global_cleanup();  
        return -1;  
    }  
  
    // 获取easy handle
    CURL *easy_handle = curl_easy_init();
    if (NULL == easy_handle)
    {         
        printf("get a easy handle failed. \r\n");  
        curl_easy_cleanup(easy_handle);  
        curl_global_cleanup();  
        return -1;  
    }  
  
    return_code = curl_easy_setopt(easy_handle, CURLOPT_URL, url);  
    return_code = curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &process_data);  
    return_code = curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &cont);    
    return_code = curl_easy_perform(easy_handle);     
  
    //判断获取响应的http地址是否存在,若存在则返回200,400以上则为不存在,一般不存在为404错误  
    int retcode = 0;  
    curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE , &retcode); 

    printf("============ retcode: %d \r\n", retcode);
    
    if (CURLE_OK == return_code && FILE_EXIST == retcode)
    {  
        //double length = 0;  
        //return_code = curl_easy_getinfo(easy_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length);

        printf("============ length: %d \r\n", cont.filelen);
        FILE *fp = fopen(down_file_name, "wb+");
        fwrite(cont.buffer, 1, cont.filelen, fp); //返回实际写入文本的长度,若不等于length则写文件发生错误.  
        fclose(fp);  
    }  
    else  
    {  
        printf("down file not exist! \r\n");
        curl_easy_cleanup(easy_handle);  
        curl_global_cleanup();  
        return -1;  
    }  
  
    // 释放资源   
    curl_easy_cleanup(easy_handle);  
    curl_global_cleanup();  
  
    return 0;  
}  

  
/*
 * This example shows an FTP upload, with a rename of the file just after
 * a successful upload.
 *
 * Example based on source code provided by Erick Nuwendam. Thanks!
 */
  
#define LOCAL_FILE      "feed.txt"
#define RENAME_FILE_TO  "feed.zip"
#define UPLOAD_FILE_AS  "feed.txt"
 
  
/* NOTE: if you want this example to work on Windows with libcurl as a
   DLL, you MUST also provide a read callback with CURLOPT_READFUNCTION.
   Failing to do so will give you a crash since a DLL may not use the
   variable's memory when passed in to it from an app like this. */
static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  /* in real-world cases, this would probably get this data differently
     as this fread() stuff is exactly what the library already would do
     by default internally */
  size_t retcode = fread(ptr, size, nmemb, (FILE *)stream);
  
  printf("*** We read %d bytes from file\n", retcode);
  return retcode;
}
  
int ftp_upload(void)
{
  CURL *curl;
  CURLcode res;
  FILE *hd_src;
  struct stat file_info;
  double speed_upload, total_time;
  curl_off_t fsize;
  // I had to add public_html as a folder, because it said there was no projects directory.  As well as add the filename to the final URL below
  char *REMOTE_URL = "ftp://iss.netii.net/public_html/projects/message/feed.zip";
  
  struct curl_slist *headerlist=NULL;
  static const char buf_1 [] = "RNFR " UPLOAD_FILE_AS;
  static const char buf_2 [] = "RNTO " RENAME_FILE_TO;
  
  /* get the file size of the local file */
  if(stat(LOCAL_FILE, &file_info)) {
    printf("Couldnt open '%s': %s\n", LOCAL_FILE, strerror(errno));
    return 1;
  }
  fsize = (curl_off_t)file_info.st_size;
  
  printf("Local file size: %" CURL_FORMAT_CURL_OFF_T " bytes.\n", fsize);
  
  /* get a FILE * of the same file */
  hd_src = fopen(LOCAL_FILE, "rb");
  
  /* In windows, this will init the winsock stuff */
  curl_global_init(CURL_GLOBAL_ALL);
  
  /* get a curl handle */
  curl = curl_easy_init();
  if(curl) {
    /* build a list of commands to pass to libcurl */
    headerlist = curl_slist_append(headerlist, buf_1);
    headerlist = curl_slist_append(headerlist, buf_2);
  
    /* we want to use our own read function */
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
  
    /* enable uploading */
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
  
    /* specify target */
    curl_easy_setopt(curl,CURLOPT_URL, REMOTE_URL);
  
    /* pass in that last of FTP commands to run after the transfer */
    curl_easy_setopt(curl, CURLOPT_POSTQUOTE, headerlist);
  
    /* now specify which file to upload */
    curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);
     
    /* specify username and password for ftp */
    curl_easy_setopt(curl, CURLOPT_USERNAME, "username");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "password");
  
    /* Set the size of the file to upload (optional).  If you give a *_LARGE
       option you MUST make sure that the type of the passed-in argument is a
       curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
       make sure that to pass in a type 'long' argument. */
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                     (curl_off_t)fsize);
                      
    /* enable verbose for easier tracing */
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
  
    /* Now run off and do what you've been told! */
    res = curl_easy_perform(curl);
     
    /* now extract transfer info */
    curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &speed_upload);
    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
  
    printf("Speed: %.3f bytes/sec during %.3f seconds\n", speed_upload, total_time);
  
    /* clean up the FTP commands list */
    curl_slist_free_all (headerlist);
  
    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  fclose(hd_src); /* close the local file */
  getchar();
  curl_global_cleanup();
  return 0;
}


int modify_file(const char *file_name, char *sn)
{
    /*
    *linebuffer：读取文件中的一行存入的缓冲
    *buffer1：一行中第一个字段的存入缓冲
    *buffer2：一行中第二个字段的存入缓冲
    */
    char linebuffer[512] = {0};
    char buffer1[512] = {0};
    char buffer2[512] = {0};

    int line_len = 0;
    int len = 0;
    int res;

    FILE *fp = fopen(file_name, "r+");
    if (fp == NULL)
    {
        printf("open error");
        return -1;
    }

    while (fgets(linebuffer, 512, fp))
    {
        line_len = strlen(linebuffer);
        len += line_len;

        /*
        * buffer1=wireless.1.current_state
        * buffer2=1
        */
        
        sscanf(linebuffer, "%[^=]=%[^=]", buffer1, buffer2);
        
        if (!strcmp("SSID", buffer1))
        {
            /*
            * 由于已经找到所需要写的位置，所以需要写位置的“头”
            */
            len -= strlen(linebuffer);

            /*
            * 实现文件位置的偏移，为写文件做准备
            */
            res = fseek(fp, len, SEEK_SET);
            if(res < 0)
            {
                perror("fseek");
                return -1;
            }
            
            strcpy(buffer2, "=iermu888999");
            /*strcat(buffer1, "=");*/
            strcat(buffer1, buffer2);

            printf("%d", strlen(buffer1));
            printf("%s \r\n", buffer1);

            /*
            * 写文件，存入所需的内容
            */
            fprintf(fp, "%s", buffer1);
            
            fclose(fp);

            return 0;
        }
    }
    
    return 0;
}

int g_downloadSize = 0;
double g_fileSize = 0;

static size_t ParaseResponseUpgrade(void *buffer, size_t size, size_t nmemb, void *user_p) 
{ 	
    FILE *fp = (FILE *)user_p; 	
    size_t return_size = 0;
    
    return_size = fwrite(buffer, size, nmemb, fp);
    g_downloadSize += return_size;
    
    printf("======== down bytes %d(%d, %d) download size(%d), filesize:(%.0f)\r\n", size*nmemb, size, nmemb, g_downloadSize, g_fileSize);
    
    return return_size; 
}



double DownloadGetSize(const char *downUrl)
{
    double downloadSize = 0.0;
    CURL *curl = NULL;
    CURLcode code;

    code = curl_global_init(CURL_GLOBAL_NOTHING);
    if (code != CURLE_OK)
    {
        printf("curl_global_init error! \r\n");
        return -1;
    }

    curl = curl_easy_init();
    if (curl == NULL)
    {
        printf("get a easy handle failed. \r\n");  
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, downUrl);
    curl_easy_setopt(curl, CURLOPT_HEADER, 1); 
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 600L);          /* 升级文件下载最长时间600s */
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);

    code = curl_easy_perform(curl);
    if (code != CURLE_OK)
    {
        printf("curl_easy_perform() failed: %s %s %d\n", curl_easy_strerror(code), __FUNCTION__, __LINE__);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return -1;
    }

    code = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &downloadSize);

    printf("downloadSize: %.0f \r\n", downloadSize);
    
    return downloadSize;
}


int DownloadResume(const char *downUrl, const char *filePath)
{
    CURL *curl = NULL;
    CURLcode code;
    char url[128] = {0};
    struct stat st;
    long offset = 0;
    FILE *fp = NULL;
    
    if (downUrl == NULL || filePath == NULL)
    {
        return -1;
    }

    /* 文件已经存在，则进行断点续传 */
    if (stat(filePath, &st) == 0 
        && st.st_size > 0 
        && st.st_size < g_fileSize)
    {
        fp = fopen(filePath, "ab+");
        if (fp == NULL)
        {
            printf("fopen %s(%s) fail! %s %d\r\n", filePath, strerror(errno), __FUNCTION__, __LINE__);
            return -1;
        }
        
        offset = st.st_size;
        g_downloadSize += offset;       /* 已经下载的文件大小 */
        printf("Download Resume, Already Download Size: %d %s %d\r\n", g_downloadSize, __FUNCTION__, __LINE__);
    }
    else
    {
        fp = fopen(filePath, "wb+");
        if (NULL == fp)
        {
            printf("fopen %s(%s) fail! %s %d\r\n", filePath, strerror(errno), __FUNCTION__, __LINE__);
            return -1;
        }

        offset = 0;
    }

    code = curl_global_init(CURL_GLOBAL_NOTHING);
    if (code != CURLE_OK)
    {
        return -1;    
    }

    curl = curl_easy_init();
    if (curl == NULL)
    {
        printf("get a easy handle failed. \r\n");  
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return -1;
    }

    snprintf(url, sizeof(url) - 1, downUrl);
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ParaseResponseUpgrade);  
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);  
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 600L);          /* 升级文件下载最长时间600s */
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_RESUME_FROM, offset);    /* 从offset处开始断点续传 */
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

    code = curl_easy_perform(curl);
    if (code != CURLE_OK)
    {
        printf("curl_easy_perform() failed: %s %s %d\n", curl_easy_strerror(code), __FUNCTION__, __LINE__);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        fclose(fp);
        return -1;
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();
    fclose(fp);

    return 0;
}  


int checkJson(char *string)
{
    Value jsonReceive;    //表示一个json格式的对象
    int iRet = StringToJson(string, jsonReceive);
    if (iRet != 0) {
        printf("string to json object fail. \r\n");
        return (-1);
    }

    if (jsonReceive["success"].isNull()) {
        printf("receive json element success is null. \r\n");
        return (-1);
    }

#if 1
    if (!jsonReceive["success"].isString()) {
        printf("Json element success (is null/not string). %s %d\r\n", 
            __FUNCTION__, __LINE__);
        return (-1);
    }
#endif

    int returnCode = atoi(jsonReceive["success"].asCString());
    printf("returnCode: %d. \r\n", returnCode);

    return (0);
}



int main(void)
{
    int iReturn = -1;
    int iResult = -1;

    
    //const char *url = "http://192.168.1.101/download/vieli";
    //const char *file_name = "vieli";
    //down_file(url, file_name);


    //const char *url = "http://192.168.1.239/download/test.bin";
    //const char *file_name = "test.bin";

    //g_fileSize = DownloadGetSize(url);
    //printf("download size: %.0f \r\n", g_fileSize);

    //if (g_fileSize != 1430879)
    //    g_fileSize = 1430879;
    
    //DownloadResume(url, file_name);
    
    //upload_file(url, file_name);
    //modify_file("RT2870AP.dat");

    
    char str[256] = {0};
    const char *string = "{\"success\":10005,\"feedbak\":\"\u6ca1\u6709\u65b0\u7684\u914d\u7f6e\u4fe1\u606f\"}";

    printf("string: %s \r\n", string);
    
    snprintf(str, sizeof(str), "%s", string);
    checkJson(str);
    

#if 0
    string ss;
    GetAuthContent("44444444", ss);
    cout << ss << endl;
   
    CreateUpload(3);
    string s = CreateUploadJsonString();
    cout << s << endl;
#endif

#if 0
    iReturn = SendAuthToPlatform(&iResult);

    printf("iReturn: %d, iResult: %d \r\n", iReturn, iResult);
    
    if (0 == iReturn && 0 == iResult)
    {
        printf("SendAuthToPlatform Success. \r\n");     
    }
#endif

    return 0;
}

