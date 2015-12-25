#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "myXml/tinyxml.h"
#include "sysConfig.h"

CONFIG_SYS_STR gstrConfigSys;
CONFIG_CAR_STR gstrConfigCar;
CONFIG_GPS_STR gstrConfigGps;

/**************************************************************\
** 函数名称： CheckFile
** 功能： 检测文件是否存在
** 参数： pcFileName: 输入文件路径
          piFileType: 返回文件类型
          piFileLen:  返回文件大小
** 返回： 0成功；-1失败
** 创建作者： 朱坚
** 创建日期： 2012-06-20
** 修改作者： 
** 修改日期： 
\**************************************************************/
int CheckFile(char *pcFileName, int *piFileType, int *piFileLen)
{	
    int ret = -1;
    int iFileType = 2;
    int iFileLen = 0;
    struct stat st;

    if(pcFileName == NULL || *pcFileName == '\0') 
    {
        ret = -1;
    }
    else 
    {
        if(stat(pcFileName, &st) == 0) 
        {
            iFileLen = st.st_size;
        }

        if(iFileLen)
        {
            ret = 0;
            if(strstr(pcFileName, ".jpg"))
            {
                iFileType = 1;
            }
            else if(strstr(pcFileName, ".bmp")) 
            {
                iFileType = 0;
            }
        }
    }
	
    if(ret == 0)
    {
        if(piFileType)
        {
            *piFileType = iFileType;
        }
        if(piFileLen)
        {
            *piFileLen = iFileLen;
        }
    }	

    return ret;
    
}

void InitDefaultConfig()
{
    memset(&gstrConfigSys, 0, sizeof(CONFIG_SYS_STR));
    memset(&gstrConfigCar, 0, sizeof(CONFIG_CAR_STR));
    memset(&gstrConfigGps, 0, sizeof(CONFIG_GPS_STR));

    /*--------------- sys ------------*/
    gstrConfigSys.thresholdOne2One  = 50;
    gstrConfigSys.thresholdOne2More = 55;
    gstrConfigSys.exportFormat      = 0;
    gstrConfigSys.encryptAlgo       = 1;
    gstrConfigSys.updateFlag        = 0;
    gstrConfigSys.digitalFlag       = 1;
    gstrConfigSys.rakeTime          = 5;
    gstrConfigSys.picCheckSeq       = 2;
    gstrConfigSys.teacherCheckFlag  = 1;
    gstrConfigSys.teacherCheckNum   = 10;
    gstrConfigSys.studyTimeLength   = 30;
    sprintf(gstrConfigSys.cfgSN,    "%s", "0000000000000001");
    sprintf(gstrConfigSys.dsapSN,   "%s", "2013120100000002");
    sprintf(gstrConfigSys.deviceSN, "%s", "3013125121220");
    
    /*--------------- GPS ------------*/
    gstrConfigGps.controlFlag       = 0;
    gstrConfigGps.typeFlag          = 1;
    gstrConfigGps.voiceFlag         = 0;
    gstrConfigGps.intervalTime      = 10;
    gstrConfigGps.startIntervalTime = 2;
    gstrConfigGps.startIntervalNum  = 2;
    gstrConfigGps.speedThreshold    = 20;

    /*--------------- CAR ------------*/
    gstrConfigCar.type              = 0;
}


void GetConfigSys(CONFIG_SYS_STR *pstrConfigSys)
{
    memcpy(pstrConfigSys, &gstrConfigSys, sizeof(CONFIG_SYS_STR));
}

void GetConfigGps(CONFIG_GPS_STR *pstrConfigGps)
{
    memcpy(pstrConfigGps, &gstrConfigGps, sizeof(CONFIG_GPS_STR));
}

void GetConfigCar(CONFIG_CAR_STR *pstrConfigCar)
{
    memcpy(pstrConfigCar, &gstrConfigCar, sizeof(CONFIG_CAR_STR));
}

int CreateConfigGps(const char *pcPath)
{
    int ret = 0;
    char tmpBuf[256] = {0};

    TiXmlDocument *configGpsXml = new TiXmlDocument();
    TiXmlDeclaration Declaration( "1.0", "gb18030", "no" );
	configGpsXml->InsertEndChild( Declaration );

    TiXmlElement *rootElement = new TiXmlElement("HHProtocol");
	configGpsXml->LinkEndChild(rootElement);

    TiXmlElement *configGpsElement = new TiXmlElement("configGps");
    rootElement->LinkEndChild(configGpsElement);

    /* GPS上机学习控制开关 */
    TiXmlElement *controlFlagElement = new TiXmlElement("controlFlag");
    configGpsElement->LinkEndChild(controlFlagElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", gstrConfigGps.controlFlag);
    TiXmlText *controlFlagContent = new TiXmlText(tmpBuf);
    controlFlagElement->LinkEndChild(controlFlagContent);

    /* GPS数据类型切换开关 */
    TiXmlElement *typeFlagElement = new TiXmlElement("typeFlag");
    configGpsElement->LinkEndChild(typeFlagElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", gstrConfigGps.typeFlag);
    TiXmlText *typeFlagContent = new TiXmlText(tmpBuf);
    typeFlagElement->LinkEndChild(typeFlagContent);

    /* GPS语音播放开关 */
    TiXmlElement *voiceFlagElement = new TiXmlElement("voiceFlag");
    configGpsElement->LinkEndChild(voiceFlagElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", gstrConfigGps.voiceFlag);
    TiXmlText *voiceFlagContent = new TiXmlText(tmpBuf);
    voiceFlagElement->LinkEndChild(voiceFlagContent); 

    /* GPS采集时间间隔 */
    TiXmlElement *intervalTimeElement = new TiXmlElement("intervalTime");
    configGpsElement->LinkEndChild(intervalTimeElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", gstrConfigGps.intervalTime);
    TiXmlText *intervalTimeContent = new TiXmlText(tmpBuf);
    intervalTimeElement->LinkEndChild(intervalTimeContent);

    /* 启动轨迹采集时间间隔 */
    TiXmlElement *startIntervalTimeElement = new TiXmlElement("startIntervalTime");
    configGpsElement->LinkEndChild(startIntervalTimeElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", gstrConfigGps.startIntervalTime);
    TiXmlText *startIntervalTimeContent = new TiXmlText(tmpBuf);
    startIntervalTimeElement->LinkEndChild(startIntervalTimeContent);

    TiXmlElement *speedThresholdElement = new TiXmlElement("speedThreshold");
    configGpsElement->LinkEndChild(speedThresholdElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", gstrConfigGps.speedThreshold);
    TiXmlText *speedThresholdContent = new TiXmlText(tmpBuf);
    speedThresholdElement->LinkEndChild(speedThresholdContent);
        
    //configBaseXml->Print();
    
    configGpsXml->SaveFile(pcPath);

	configGpsXml->Clear();
	delete configGpsXml;
    return ret;
}

int CreateConfigSys(const char *pcPath)
{
    int ret = 0;
    char tmpBuf[256] = {0};

    TiXmlDocument *configSysXml = new TiXmlDocument();
    TiXmlDeclaration Declaration( "1.0", "gb18030", "no" );
	configSysXml->InsertEndChild( Declaration );

	TiXmlElement *rootElement = new TiXmlElement("HHProtocol");
	configSysXml->LinkEndChild(rootElement);

    TiXmlElement *configSysElement = new TiXmlElement("configSys");
    rootElement->LinkEndChild(configSysElement);

    TiXmlElement *exportFormatElement = new TiXmlElement("exportFormat");
    configSysElement->LinkEndChild(exportFormatElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", gstrConfigSys.exportFormat);
    TiXmlText *exportFormatContent = new TiXmlText(tmpBuf);
    exportFormatElement->LinkEndChild(exportFormatContent);

    TiXmlElement *updateFlagElement = new TiXmlElement("updateFlag");
    configSysElement->LinkEndChild(updateFlagElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", gstrConfigSys.updateFlag);
    TiXmlText *updateFlagContent = new TiXmlText(tmpBuf);
    updateFlagElement->LinkEndChild(updateFlagContent);

    TiXmlElement *encryptAlgoElement = new TiXmlElement("encryptAlgo");
    configSysElement->LinkEndChild(encryptAlgoElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", gstrConfigSys.encryptAlgo);
    TiXmlText *encryptAlgoContent = new TiXmlText(tmpBuf);
    encryptAlgoElement->LinkEndChild(encryptAlgoContent);

    TiXmlElement *digitalFlagElement = new TiXmlElement("digitalFlag");
    configSysElement->LinkEndChild(digitalFlagElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", gstrConfigSys.digitalFlag);
    TiXmlText *digitalFlagContent = new TiXmlText(tmpBuf);
    digitalFlagElement->LinkEndChild(digitalFlagContent);

    TiXmlElement *rakeTimeElement = new TiXmlElement("rakeTime");
    configSysElement->LinkEndChild(rakeTimeElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", gstrConfigSys.rakeTime);
    TiXmlText *rakeTimeContent = new TiXmlText(tmpBuf);
    rakeTimeElement->LinkEndChild(rakeTimeContent);

    TiXmlElement *picCheckSeqElement = new TiXmlElement("picCheckSeq");
    configSysElement->LinkEndChild(picCheckSeqElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", gstrConfigSys.picCheckSeq);
    TiXmlText *picCheckSeqContent = new TiXmlText(tmpBuf);
    picCheckSeqElement->LinkEndChild(picCheckSeqContent);

    TiXmlElement *teacherCheckFlagElement = new TiXmlElement("teacherCheckFlag");
    configSysElement->LinkEndChild(teacherCheckFlagElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", gstrConfigSys.teacherCheckFlag);
    TiXmlText *teacherCheckFlagContent = new TiXmlText(tmpBuf);
    teacherCheckFlagElement->LinkEndChild(teacherCheckFlagContent);

    TiXmlElement *teacherCheckNumElement = new TiXmlElement("teacherCheckNum");
    configSysElement->LinkEndChild(teacherCheckNumElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", gstrConfigSys.teacherCheckNum);
    TiXmlText *teacherCheckNumContent = new TiXmlText(tmpBuf);
    teacherCheckNumElement->LinkEndChild(teacherCheckNumContent);

    TiXmlElement *studyTimeLengthElement = new TiXmlElement("studyTimeLength");
    configSysElement->LinkEndChild(studyTimeLengthElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", gstrConfigSys.studyTimeLength);
    TiXmlText *studyTimeLengthContent = new TiXmlText(tmpBuf);
    studyTimeLengthElement->LinkEndChild(studyTimeLengthContent);

    TiXmlElement *cfgSNElement = new TiXmlElement("cfgSN");
    configSysElement->LinkEndChild(cfgSNElement);   
    TiXmlText *cfgSNContent = new TiXmlText(gstrConfigSys.cfgSN);
    cfgSNElement->LinkEndChild(cfgSNContent);

    TiXmlElement *dsapSNElement = new TiXmlElement("dsapSN");
    configSysElement->LinkEndChild(dsapSNElement);   
    TiXmlText *dsapSNContent = new TiXmlText(gstrConfigSys.dsapSN);
    dsapSNElement->LinkEndChild(dsapSNContent);

    TiXmlElement *deviceSNElement = new TiXmlElement("deviceSN");
    configSysElement->LinkEndChild(deviceSNElement);   
    TiXmlText *deviceSNContent = new TiXmlText(gstrConfigSys.deviceSN);
    deviceSNElement->LinkEndChild(deviceSNContent);
    
    configSysXml->SaveFile(pcPath);
    configSysXml->Clear();
    delete configSysXml;

    return ret;
}

int CreateConfigCar(const char *pcPath)
{
    int ret = 0;
    char tmpBuf[256] = {0};

    TiXmlDocument *configCarXml = new TiXmlDocument();
    TiXmlDeclaration Declaration( "1.0", "gb18030", "no" );
	configCarXml->InsertEndChild( Declaration );

	TiXmlElement *rootElement = new TiXmlElement("HHProtocol");
	configCarXml->LinkEndChild(rootElement);

    TiXmlElement *configCarElement = new TiXmlElement("configCar");
    rootElement->LinkEndChild(configCarElement);

    TiXmlElement *typeElement = new TiXmlElement("type");
    configCarElement->LinkEndChild(typeElement);
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "%d", gstrConfigCar.type);
    TiXmlText *typeContent = new TiXmlText(tmpBuf);
    typeElement->LinkEndChild(typeContent);

    TiXmlElement *schoolIdElement = new TiXmlElement("schoolId");
    configCarElement->LinkEndChild(schoolIdElement);
    TiXmlText *schoolIdContent = new TiXmlText(gstrConfigCar.schoolId);
    schoolIdElement->LinkEndChild(schoolIdContent);

    TiXmlElement *schoolNameElement = new TiXmlElement("schoolName");
    configCarElement->LinkEndChild(schoolNameElement);
    TiXmlText *schoolNameContent = new TiXmlText(gstrConfigCar.schoolName);
    schoolNameElement->LinkEndChild(schoolNameContent);

    TiXmlElement *carIdElement = new TiXmlElement("carId");
    configCarElement->LinkEndChild(carIdElement);
    TiXmlText *carIdContent = new TiXmlText(gstrConfigCar.carId);
    carIdElement->LinkEndChild(carIdContent);

    TiXmlElement *carPlateNumElement = new TiXmlElement("carPlateNum");
    configCarElement->LinkEndChild(carPlateNumElement);
    TiXmlText *carPlateNumContent = new TiXmlText(gstrConfigCar.carPlateNum);
    carPlateNumElement->LinkEndChild(carPlateNumContent);

    TiXmlElement *carFrameNumElement = new TiXmlElement("carFrameNum");
    configCarElement->LinkEndChild(carFrameNumElement);
    TiXmlText *carFrameNumContent = new TiXmlText(gstrConfigCar.carFrameNum);
    carFrameNumElement->LinkEndChild(carFrameNumContent);
    
    configCarXml->SaveFile(pcPath);
    configCarXml->Clear();
    delete configCarXml;

    return ret;
}

void SetConfigSys(CONFIG_SYS_STR *pstrConfigSys)
{
    int ret = -1;
    char caPath[256];

    if(memcmp(&gstrConfigSys, pstrConfigSys, sizeof(CONFIG_SYS_STR)))
    {
        memcpy(&gstrConfigSys, pstrConfigSys, sizeof(CONFIG_SYS_STR));
        memset(caPath, 0, sizeof(caPath));
        sprintf(caPath, "%s/%s", STORAGE_PATH, CONFIG_SYS_PATH);
        ret = CreateConfigSys(caPath);
        if(ret)
        {
            TRACE("set %s failed. %s %d\r\n", caPath, __FUNCTION__, __LINE__);
        }
    }
}

void SetConfigGps(CONFIG_GPS_STR *pstrConfigGps)
{
    int ret = -1;
    char caPath[256];

    if (memcmp(&gstrConfigGps, pstrConfigGps, sizeof(CONFIG_GPS_STR)))
    {
        memcpy(&gstrConfigGps, pstrConfigGps, sizeof(CONFIG_GPS_STR));
        memset(caPath, 0, sizeof(caPath));
        sprintf(caPath, "%s/%s", STORAGE_PATH, CONFIG_GPS_PATH);
        ret = CreateConfigGps(caPath);
        if (ret)
        {
            TRACE("set %s failed. %s %d\r\n", caPath, __FUNCTION__, __LINE__);
        }
    }
}

void SetConfigCar(CONFIG_CAR_STR *pstrConfigCar)
{
    int ret = -1;
    char caPath[256];

    if (memcmp(&gstrConfigCar, pstrConfigCar, sizeof(CONFIG_CAR_STR)))
    {
        memcpy(&gstrConfigCar, pstrConfigCar, sizeof(CONFIG_CAR_STR));
        memset(caPath, 0, sizeof(caPath));
        sprintf(caPath, "%s/%s", STORAGE_PATH, CONFIG_CAR_PATH);
        ret = CreateConfigCar(caPath);
        if (ret)
        {
            TRACE("set %s failed. %s %d\r\n", caPath, __FUNCTION__, __LINE__);
        }
    }
}

int LoadConfigSysXml(char *pcPath, CONFIG_SYS_STR *pstrConfigSys)
{
    using namespace std;
	const char * xmlFile = pcPath;
	TiXmlDocument doc;                              

	if (doc.LoadFile(xmlFile))
	{
        //doc.Print();
	}
	else
	{
		cout << "can not parse xml conf/ConfigSys.xml" << endl;
		
		return -1;
	}

    TiXmlElement* rootElement = doc.RootElement();
	TiXmlElement* ConfigSecurityElement = rootElement->FirstChildElement();

    for(; ConfigSecurityElement != NULL; ConfigSecurityElement = ConfigSecurityElement->NextSiblingElement())
	{
		TiXmlElement* contactElement = ConfigSecurityElement->FirstChildElement();
		
		for(; contactElement != NULL; contactElement = contactElement->NextSiblingElement())
		{
			const char * contactType = contactElement->Value();
			if(contactType != NULL)
			{
				const char * contactValue = contactElement->GetText();
				if(contactValue != NULL)
				{
					if(strncmp(contactType, "exportFormat", strlen("exportFormat")) == 0)
					{
                        pstrConfigSys->exportFormat = atoi(contactValue);
					}
					else if(strncmp(contactType, "updateFlag", strlen("updateFlag")) == 0)
					{
						pstrConfigSys->updateFlag = atoi(contactValue); 
					}
					else if(strncmp(contactType, "encryptAlgo", strlen("encryptAlgo")) == 0)
					{
						pstrConfigSys->encryptAlgo = atoi(contactValue); 
					}
					else if(strncmp(contactType, "digitalFlag", strlen("digitalFlag")) == 0)
					{
						pstrConfigSys->digitalFlag = atoi(contactValue); 
					}
					else if(strncmp(contactType, "rakeTime", strlen("rakeTime")) == 0)
					{
						pstrConfigSys->rakeTime = atoi(contactValue);
					}
                    else if(strncmp(contactType, "picCheckSeq", strlen("picCheckSeq")) == 0)
					{
						pstrConfigSys->picCheckSeq = atoi(contactValue);
					}
                    else if(strncmp(contactType, "teacherCheckFlag", strlen("teacherCheckFlag")) == 0)
					{
						pstrConfigSys->teacherCheckFlag = atoi(contactValue);
					}
                    else if(strncmp(contactType, "teacherCheckNum", strlen("teacherCheckNum")) == 0)
					{
						pstrConfigSys->teacherCheckNum = atoi(contactValue);
					}
                    else if(strncmp(contactType, "studyTimeLength", strlen("studyTimeLength")) == 0)
					{
						pstrConfigSys->studyTimeLength = atoi(contactValue);
					}
                    else if(strncmp(contactType, "cfgSN", strlen("cfgSN")) == 0)
					{
						strncpy(pstrConfigSys->cfgSN, contactValue, sizeof(pstrConfigSys->cfgSN));
					}
                    else if(strncmp(contactType, "dsapSN", strlen("dsapSN")) == 0)
					{
						strncpy(pstrConfigSys->dsapSN, contactValue, sizeof(pstrConfigSys->dsapSN));
					}
                    else if(strncmp(contactType, "deviceSN", strlen("deviceSN")) == 0)
					{
						strncpy(pstrConfigSys->deviceSN, contactValue, sizeof(pstrConfigSys->deviceSN));
					}
 				}
			}
		}
	} 

	return 0;
}

int LoadConfigGpsXml(char *pcPath, CONFIG_GPS_STR *pstrConfigGps)
{
    using namespace std;
	const char * xmlFile = pcPath;
	TiXmlDocument doc;                              

	if (doc.LoadFile(xmlFile))
	{
        //doc.Print();
	}
	else
	{
		cout << "can not parse xml conf/ConfigGps.xml" << endl;
		
		return -1;
	}

    TiXmlElement* rootElement = doc.RootElement();
	TiXmlElement* ConfigSecurityElement = rootElement->FirstChildElement();

    for(; ConfigSecurityElement != NULL; ConfigSecurityElement = ConfigSecurityElement->NextSiblingElement())
	{
		TiXmlElement* contactElement = ConfigSecurityElement->FirstChildElement();
		
		for(; contactElement != NULL; contactElement = contactElement->NextSiblingElement())
		{
			const char * contactType = contactElement->Value();
			if(contactType != NULL)
			{
				const char * contactValue = contactElement->GetText();
				if(contactValue != NULL)
				{
					if(strncmp(contactType, "controlFlag", strlen("controlFlag")) == 0)
					{
                        pstrConfigGps->controlFlag = atoi(contactValue);
					}
					else if(strncmp(contactType, "typeFlag", strlen("typeFlag")) == 0)
					{
						pstrConfigGps->typeFlag = atoi(contactValue); 
					}
					else if(strncmp(contactType, "voiceFlag", strlen("voiceFlag")) == 0)
					{
						pstrConfigGps->voiceFlag = atoi(contactValue); 
					}
					else if(strncmp(contactType, "intervalTime", strlen("intervalTime")) == 0)
					{
						pstrConfigGps->intervalTime = atoi(contactValue); 
					}
					else if(strncmp(contactType, "startIntervalTime", strlen("startIntervalTime")) == 0)
					{
						pstrConfigGps->startIntervalTime = atoi(contactValue);
					}
                    else if(strncmp(contactType, "startIntervalNum", strlen("startIntervalNum")) == 0)
					{
						pstrConfigGps->startIntervalNum = atoi(contactValue);
					}
                    else if(strncmp(contactType, "speedThreshold", strlen("speedThreshold")) == 0)
					{
						pstrConfigGps->speedThreshold = atoi(contactValue);
					}
 				}
			}
		}
	} 

	return 0;
}


int LoadConfigCarXml(char *pcPath, CONFIG_CAR_STR *pstrConfigCar)
{
    using namespace std;
	const char * xmlFile = pcPath;
	TiXmlDocument doc;                              

	if (doc.LoadFile(xmlFile))
	{
        //doc.Print();
	}
	else
	{
		cout << "can not parse xml conf/ConfigCar.xml" << endl;
		
		return -1;
	}

    TiXmlElement* rootElement = doc.RootElement();
	TiXmlElement* ConfigSecurityElement = rootElement->FirstChildElement();

    for(; ConfigSecurityElement != NULL; ConfigSecurityElement = ConfigSecurityElement->NextSiblingElement())
	{
		TiXmlElement* contactElement = ConfigSecurityElement->FirstChildElement();
		
		for(; contactElement != NULL; contactElement = contactElement->NextSiblingElement())
		{
			const char * contactType = contactElement->Value();
			if(contactType != NULL)
			{
				const char * contactValue = contactElement->GetText();
				if(contactValue != NULL)
				{
					if(strncmp(contactType, "type", strlen("type")) == 0)
					{
                        pstrConfigCar->type = atoi(contactValue);
					}
					else if(strncmp(contactType, "schoolId", strlen("schoolId")) == 0)
					{
						strncpy(pstrConfigCar->schoolId, contactValue, sizeof(pstrConfigCar->schoolId));
					}
					else if(strncmp(contactType, "schoolName", strlen("schoolName")) == 0)
					{
						strncpy(pstrConfigCar->schoolName, contactValue, sizeof(pstrConfigCar->schoolName));
					}
					else if(strncmp(contactType, "carId", strlen("carId")) == 0)
					{
						strncpy(pstrConfigCar->carId, contactValue, sizeof(pstrConfigCar->carId));
					}
					else if(strncmp(contactType, "carPlateNum", strlen("carPlateNum")) == 0)
					{
						strncpy(pstrConfigCar->carPlateNum, contactValue, sizeof(pstrConfigCar->carPlateNum));
					}
                    else if(strncmp(contactType, "carFrameNum", strlen("carFrameNum")) == 0)
					{
						strncpy(pstrConfigCar->carFrameNum, contactValue, sizeof(pstrConfigCar->carFrameNum));
					}
 				}
			}
		}
	} 

	return 0;
}

int ReadConfigSys()
{
    int ret = -1;
    char caPath[256];

    memset(caPath, 0, sizeof(caPath));
    sprintf(caPath, "%s/%s", STORAGE_PATH, CONFIG_SYS_PATH);

    ret = CheckFile(caPath, NULL, NULL);

    if(ret)
    {
        TRACE("%s not exist %s %d\r\n", caPath, __FUNCTION__, __LINE__);
        ret = CreateConfigSys(caPath);
    }
    else
    {
        ret = LoadConfigSysXml(caPath, &gstrConfigSys);
        if(ret)
        {
           TRACE("load %s failed %s %d\r\n", caPath, __FUNCTION__, __LINE__); 
        }
    }
    
    return ret; 
}

int ReadConfigGps()
{
    int ret = -1;
    char caPath[256];

    memset(caPath, 0, sizeof(caPath));
    sprintf(caPath, "%s/%s", STORAGE_PATH, CONFIG_GPS_PATH);

    ret = CheckFile(caPath, NULL, NULL);

    if(ret)
    {
        TRACE("%s not exist %s %d\r\n", caPath, __FUNCTION__, __LINE__);
        ret = CreateConfigGps(caPath);
    }
    else
    {
        ret = LoadConfigGpsXml(caPath, &gstrConfigGps);
        if(ret)
        {
           TRACE("load %s failed %s %d\r\n", caPath, __FUNCTION__, __LINE__); 
        }
    }
    
    return ret; 
}

int ReadConfigCar()
{
    int ret = -1;
    char caPath[256];

    memset(caPath, 0, sizeof(caPath));
    sprintf(caPath, "%s/%s", STORAGE_PATH, CONFIG_CAR_PATH);

    ret = CheckFile(caPath, NULL, NULL);

    if(ret)
    {
        TRACE("%s not exist %s %d\r\n", caPath, __FUNCTION__, __LINE__);
        ret = CreateConfigCar(caPath);
    }
    else
    {
        ret = LoadConfigCarXml(caPath, &gstrConfigCar);
        if(ret)
        {
           TRACE("load %s failed %s %d\r\n", caPath, __FUNCTION__, __LINE__); 
        }
    }
    
    return ret; 
}


int ReadConfig()
{
    int ret = -1;

    ret = ReadConfigSys();
    if (ret)
    {
        TRACE("read config failed %s %d\r\n", __FUNCTION__, __LINE__); 
    }

    ret = ReadConfigGps();
    if (ret)
    {
        TRACE("read config failed %s %d\r\n", __FUNCTION__, __LINE__); 
    }

    ret = ReadConfigCar();
    if (ret)
    {
        TRACE("read config failed %s %d\r\n", __FUNCTION__, __LINE__); 
    }
    
    return ret;
}

int SysConfigInit()
{
    int ret = -1;
	char capath[64];

    InitDefaultConfig();

    memset(capath, 0, sizeof(capath));
	sprintf(capath, "%s/config", STORAGE_PATH);

	mkdir(capath, 0777);
	
    ret = ReadConfig();

    if(ret)
    {
        TRACE("read config failed %s %d\r\n", __FUNCTION__, __LINE__); 
    }

    return ret;
}

int main()
{
    int ret = -1;

    ret = SysConfigInit();
    if (ret)
    {
        TRACE("$$$ SysConfigInit failed!!! %s %d\r\n", __FUNCTION__, __LINE__);
    }
    else
    {
        TRACE("$$$ SysConfigInit succeed!!! %s %d\r\n", __FUNCTION__, __LINE__);
    }

    CONFIG_CAR_STR strConfigCar;
    CONFIG_GPS_STR strConfigGps;
    CONFIG_SYS_STR strConfigSys;

    GetConfigCar(&strConfigCar);
    GetConfigGps(&strConfigGps);
    GetConfigSys(&strConfigSys); 

    TRACE(" %d, %d, %d \r\n", strConfigGps.controlFlag, strConfigGps.intervalTime, strConfigGps.speedThreshold);
    
    return 0;
}

