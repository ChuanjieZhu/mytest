
#ifndef __LOG_LIB_H_
#define __LOG_LIB_H_

#include "com.h"

#define GENERAL_PATH_LEN			128      			//·���ĳ���

#define SYSLOG_INDEX_DIR_NAME		"sysIndex"			//��¼��������·��
#define MANLOG_INDEX_DIR_NAME       "manIndex"

//��־����
enum __log_type_e
{
    eSysLog,        //ϵͳ��־
    eManLog,        //������־
    eUserLog,       //�û���־
    eAccessLog        //������־
};

//��־������
enum __log_sub_type_e
{
    eNormal,       //�޹���Ա�豸����
    eAdmin,         //����Ա�豸����
    eNetSdk,       //����sdk���� 
    eUsb,           //usb����
    eUseDevice,     //usbdevice����
    eRs232,         //Rs232����
    eRs485,         //Rs485����
    eNetSync        //����ͬ������
};

enum __log_result_e
{
    eSucc,
    eFail
};

/* �������Ͷ��� */

/* �û�������� */
#define LOG_USER_ADD                0x00000001
#define LOG_USER_EDIT               0x00000002
#define LOG_USER_DEL_               0x00000003
#define LOG_USER_DEL_ALL            0x00000004

/* ����Ա���� */
#define LOG_ADMIN_ADD               0x0000000A
#define LOG_ADMIN_EDIT              0x0000000B
#define LOG_ADMIN_DEL               0x0000000C

/* ��¼���� */
#define LOG_RECORD_QUERY            0x00000010
#define LOG_RECORD_CLEAN            0x00000011
#define LOG_RECORD_SYSLOG_QUERY     0x00000012
#define LOG_RECORD_MANLOG_QUERY1    0x00000013
#define LOG_RECORD_DOORLOG_QUERY    0x00000014

/* u�̹��� */
#define LOG_USB_EXPORT_USER         0x00000020
#define LOG_USB_EXPORT_ALL_USER     0x00000021
#define LOG_USB_IMPORT_USER_TEMP    0x00000022
#define LOG_USB_IMPORT_USER_LIST    0x00000023
#define LOG_USB_SYSTEM_UPDATE       0x00000024

/* ϵͳ���� */
#define LOG_SYS_TIME_ZONE           0x00000030
#define LOG_SYS_TIME_DATE           0x00000031
#define LOG_SYS_TIME_TIME           0x00000032
#define LOG_SYS_TIME_FORMAT         0x00000033
#define LOG_SYS_TIME_NET_TIME       0x00000034
#define LOG_SYS_TIME_SYNC_TIME      0x00000035

#define LOG_SYS_SOUND_TOUCH         0x00000036
#define LOG_SYS_SOUND_PLAY          0x00000037

#define LOG_SYS_SLEEP_SETTING       0x00000038

#define LOG_SYS_RING_SETTING        0x00000039
#define LOG_SYS_RING_COUNT          0x0000003A
        
#define LOG_SYS_IO_SWITCH_OUT       0x0000003B   
#define LOG_SYS_IO_DURATION_RELAY   0x0000003C
#define LOG_SYS_DOOR_BELL           0x0000003D
#define LOG_SYS_LANGUAGE            0x0000003E
#define LOG_SYS_TIME_INTERVAL       0x0000003F
#define LOG_SYS_VERIFY_MANAGE       0x00000040
#define LOG_SYS_SECURITY            0x00000041
#define LOG_SYS_SCREEN_CALIBRATION  0x00000042
#define LOG_SYS_NETWORK             0x00000043

typedef struct __syslog_lib_t_
{
    int storageStatus;						//״̬ 0������

	unsigned int sdCardTotalSize;			//�洢�ռ��ܴ�С
	unsigned int sdCardFreeSize;			//ʣ��ռ�
	char sdCardPath[GENERAL_PATH_LEN];		//

	unsigned int flashTotalSize;			//�洢�ռ��ܴ�С
	unsigned int flashFreeSize;				//ʣ��ռ�
	char flashPath[GENERAL_PATH_LEN];		//

	char sysLogIndexPath[GENERAL_PATH_LEN];	//��¼��������·��

	int curOperateStatus;					//��ǰ����״̬

	int sysLogMaxBufNum;					//��¼��󻺳�����
	int sysLogSaveInterval;					//��¼������ʱ��
	int sysLogCurBufNum;					//��ǰ��¼��������
} syslog_lib_t;
    
typedef struct __manlog_lib_t_
{
    int storageStatus;						//״̬ 0������

	unsigned int sdCardTotalSize;			//�洢�ռ��ܴ�С
	unsigned int sdCardFreeSize;			//ʣ��ռ�
	char sdCardPath[GENERAL_PATH_LEN];		//

	unsigned int flashTotalSize;			//�洢�ռ��ܴ�С
	unsigned int flashFreeSize;				//ʣ��ռ�
	char flashPath[GENERAL_PATH_LEN];		//

	char manLogIndexPath[GENERAL_PATH_LEN];	//��¼��������·��

	int curOperateStatus;					//��ǰ����״̬

	int manLogMaxBufNum;					//��¼��󻺳�����
	int manLogSaveInterval;					//��¼������ʱ��
	int manLogCurBufNum;					//��ǰ��¼��������    
} manlog_lib_t;

typedef struct __accesslog_lib_t_
{
    int storageStatus;						//״̬ 0������

	unsigned int sdCardTotalSize;			//�洢�ռ��ܴ�С
	unsigned int sdCardFreeSize;			//ʣ��ռ�
	char sdCardPath[GENERAL_PATH_LEN];		//

	unsigned int flashTotalSize;			//�洢�ռ��ܴ�С
	unsigned int flashFreeSize;				//ʣ��ռ�
	char flashPath[GENERAL_PATH_LEN];		//

	char accessLogIndexPath[GENERAL_PATH_LEN];	//��¼��������·��

	int curOperateStatus;					//��ǰ����״̬

	int accessLogMaxBufNum;					//��¼��󻺳�����
	int accessLogSaveInterval;					//��¼������ʱ��
	int accessLogCurBufNum;					//��ǰ��¼��������    
} accesslog_lib_t;

/* �����ṹ�� */
typedef struct __syslog_index_t_
{
	unsigned int logNo;			//��0��ʼ����
	char logFileStatus;			//��־״̬  0������  1��ɾ��
	unsigned int logStartAddr;	//ָ����ļ�����ʼ��ַ
	int logFileSize;			//ָ����ļ��Ĵ�С 
	char logType;               //��־����
    char logSubType;            //��־������
    unsigned int logTime;       //��־ʱ��
    int  logOpType;             //��������
    char logResult;             //�������
    char logMsg[MAX_MSG_LEN];   //������ϸ���
    char unused[64];            //���� 
} syslog_index_t;

typedef struct __syslog_info_t_
{
    int logNo;
    syslog_index_t logIndex;
    struct __syslog_info_t_ *next;
    struct __syslog_info_t_ *cur;
    
} syslog_info_t;

typedef struct __syslog_list_t_
{
    int logNum;
    syslog_info_t head;
} syslog_list_t;

/* �����ṹ�� */
typedef struct __manlog_index_t_
{
	unsigned int logNo;			//��0��ʼ����
	char logFileStatus;			//�ļ�״̬  0������  1��ɾ��
	unsigned int logStartAddr;	//ָ����ļ�����ʼ��ַ
	int logFileSize;			//ָ����ļ��Ĵ�С 
	char logType;               //��־����
    char logSubType;            //��־������
    unsigned int logTime;       //��־ʱ��
    char logUserName[MAX_USER_NAME_LEN]; //����������
    unsigned int logUserNo;          //�û�����
    unsigned int logUserId;          //�û�id��
    int  logOpType;             //��������
    char logResult;             //�������
    char logMsg[MAX_MSG_LEN];   //������ϸ���
    char unused[64];            //���� 
} manlog_index_t;

typedef struct __manlog_info_t_
{
    int logNo;
    manlog_index_t logIndex;
    struct __manlog_info_t_ *next;
    struct __manlog_info_t_ *cur;
    
} manlog_info_t;

typedef struct __manlog_list_t_
{
    int logNum;
    manlog_info_t head;
} manlog_list_t;

typedef struct __accesslog_index_t_
{
	unsigned int logNo;			//��0��ʼ����
	char logStatus;				//�ļ�״̬  0������  1��ɾ��
	unsigned int logStartAddr;	//ָ����ļ�����ʼ��ַ
	char logType;               //��־����
	char accessMode;               //��������
	char recogMode;				//ʶ��ģʽ
	int userNum;				//��������
	int userId[5];				//�����û�id�����5����һ����
	int groupNum;				//��������
	int groupId[5];				//������id
    unsigned int accessTime;    //����ʱ��
    int result;					//���Ž��
	int photoNum;               //��Ƭ����
	unsigned int photoSize[5];  //��Ƭ��С
	unsigned int photoTime[5];  //��Ƭ��¼����ʱ��
	unsigned int photoStartAddr[5]; //��Ƭ�ڼ�¼��ƫ��λ��
    char unused[12];            //���� 
} accesslog_index_t;

typedef struct __accesslog_info_t_
{
    int logNo;
    accesslog_index_t logIndex;
    struct __accesslog_index_t_ *next;
    struct __accesslog_index_t_ *cur;
} accesslog_info_t;

typedef struct __accesslog_list_t_
{
    int logNum;
    accesslog_info_t head;
} accesslog_list_t;


#ifdef __cplusplus
extern "C" {
#endif

#include "logOutLib.h"

void lockSysLogWriteData();
void unlockSysLogWriteData();
void lockManLogWriteData();
void unlockManLogWriteData();
int addSysLog(syslog_event_t sysLog);
int addManLog(manlog_event_t manLog);
int searchSysLogNum(unsigned int searchCondFlag, syslog_search_condition_t *pSearchCondition);
int searchManLogNum(unsigned int searchCondFlag, manlog_search_condition_t *pSearchCondition);
void showSysLogInfo();
void showManLogInfo();
int delOldLogFile(char *pPath, char cLogType);
void initLogLib(syslog_lib_t *pSysLogLib, manlog_lib_t *pManLogLib);
void deinitLogLib();

#ifdef __cplusplus
}
#endif

#endif
