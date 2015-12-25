#ifndef __SYS_CONFIG_H__
#define __SYS_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define TRACE printf
#define Malloc malloc
#define Free free

#define STORAGE_PATH         "/root"
#define CONFIG_CAR_PATH      "config/ConfigCar.xml"
#define CONFIG_SYS_PATH      "config/ConfigSys.xml"
#define CONFIG_AD_PATH       "config/ConfigAd.xml"
#define CONFIG_GPS_PATH      "config/ConfigGps.xml"

typedef struct ConfigSys
{
    char thresholdOne2One;              //一对一识别门限
    char thresholdOne2More;             //一对多识别门限
    char exportFormat;                  //导出文件类型
    char updateFlag;                    //软件升级控制开关，
    char encryptAlgo;                   //加密算法
    char digitalFlag;                   //是否数字签名
    char rakeTime;                      //拍照随机照片时间,距离上机之后的时间间隔
    char picCheckSeq;                   //进行MD5校验的照片序号
    char teacherCheckFlag;              //教练是否需要打卡,yes-需要,no-不需要
    char teacherCheckNum;               //经过多少次学时后，教练需再次识别
    char studyTimeLength;               //学习时间长度
    char cfgSN[16 + 1];					//网影配置文件版本号
    char dsapSN[64 + 1];                //网影序列号
    char deviceSN[16];                  //firs设备序列号
} CONFIG_SYS_STR;

typedef struct ConfigGps
{
    char controlFlag;                   //GPS控制开关, 0-GPS控制上机，必须有GPS才能上机，1-GPS不控制上机
    char typeFlag;                      //GPS数据类型切换开关，1-启动数据类型切换，0-关闭数据类型切换
    char voiceFlag;                     //GPS语音提醒开关，1-开启语音提醒，0-关闭语音提醒
    unsigned char intervalTime;         //GPS数据采集时间间隔
    unsigned char startIntervalTime;    //GPS启动数据采集时间间隔
    unsigned char startIntervalNum;     //GPS启动数据采集时长(采集几个GPS采集时间间隔)
    unsigned char speedThreshold;	    //判断是否进行“车辆启动轨迹”速度，20公里/小时 */
} CONFIG_GPS_STR;

typedef struct ConfigCar
{
    int type;                           //车辆类型
    char schoolId[16 + 1];              //驾校id
    char schoolName[128];               //驾校名
    char carId[16 + 1];                 //教练车id
    char carPlateNum[64 + 1];           //教练车车牌号
    char carFrameNum[64 + 1];           //教练车车架号
} CONFIG_CAR_STR;


void GetConfigSys(CONFIG_SYS_STR *pstrConfigSys);
void GetConfigCar(CONFIG_CAR_STR *pstrConfigCar);
void SetConfigSys(CONFIG_SYS_STR *pstrConfigSys);
void SetConfigCar(CONFIG_CAR_STR *pstrConfigCar);

#ifdef __cplusplus 
}
#endif

#endif //end __SYS_CONFIG_H__
