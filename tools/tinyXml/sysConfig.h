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
    char thresholdOne2One;              //һ��һʶ������
    char thresholdOne2More;             //һ�Զ�ʶ������
    char exportFormat;                  //�����ļ�����
    char updateFlag;                    //����������ƿ��أ�
    char encryptAlgo;                   //�����㷨
    char digitalFlag;                   //�Ƿ�����ǩ��
    char rakeTime;                      //���������Ƭʱ��,�����ϻ�֮���ʱ����
    char picCheckSeq;                   //����MD5У�����Ƭ���
    char teacherCheckFlag;              //�����Ƿ���Ҫ��,yes-��Ҫ,no-����Ҫ
    char teacherCheckNum;               //�������ٴ�ѧʱ�󣬽������ٴ�ʶ��
    char studyTimeLength;               //ѧϰʱ�䳤��
    char cfgSN[16 + 1];					//��Ӱ�����ļ��汾��
    char dsapSN[64 + 1];                //��Ӱ���к�
    char deviceSN[16];                  //firs�豸���к�
} CONFIG_SYS_STR;

typedef struct ConfigGps
{
    char controlFlag;                   //GPS���ƿ���, 0-GPS�����ϻ���������GPS�����ϻ���1-GPS�������ϻ�
    char typeFlag;                      //GPS���������л����أ�1-�������������л���0-�ر����������л�
    char voiceFlag;                     //GPS�������ѿ��أ�1-�����������ѣ�0-�ر���������
    unsigned char intervalTime;         //GPS���ݲɼ�ʱ����
    unsigned char startIntervalTime;    //GPS�������ݲɼ�ʱ����
    unsigned char startIntervalNum;     //GPS�������ݲɼ�ʱ��(�ɼ�����GPS�ɼ�ʱ����)
    unsigned char speedThreshold;	    //�ж��Ƿ���С����������켣���ٶȣ�20����/Сʱ */
} CONFIG_GPS_STR;

typedef struct ConfigCar
{
    int type;                           //��������
    char schoolId[16 + 1];              //��Уid
    char schoolName[128];               //��У��
    char carId[16 + 1];                 //������id
    char carPlateNum[64 + 1];           //���������ƺ�
    char carFrameNum[64 + 1];           //���������ܺ�
} CONFIG_CAR_STR;


void GetConfigSys(CONFIG_SYS_STR *pstrConfigSys);
void GetConfigCar(CONFIG_CAR_STR *pstrConfigCar);
void SetConfigSys(CONFIG_SYS_STR *pstrConfigSys);
void SetConfigCar(CONFIG_CAR_STR *pstrConfigCar);

#ifdef __cplusplus 
}
#endif

#endif //end __SYS_CONFIG_H__
