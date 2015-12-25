#ifndef __MYXML_LIB_H__
#define __MYXML_LIB_H__

/* �����ͼƬ��СΪ60k�ֽ� */
#define MAX_AD_PIC_BUFFER_LEN	(60 * 1024)

/* ���ѧԱ���������ļ���СΪ100k�ֽ� */
#define MAX_FACE_BUFFER_LEN	(100 * 1024)

/* Car.xml������ */
typedef struct __car_config_t_
{
	int format;
	int emptyStudents;
	int emptyDutyDatas;
	int volume;
	char licensePlateNumber[20];	/* ���ƺ� */
	char licenseFrameNumber[100];	/* ���ܺ� */
	char teacherName[40];			/* �������� */
    char drivingSchoolName[100];    /* ��У���� */
}car_config_t;

/* Student.xml������ */
typedef struct __student_data_t_
{
	char idCard[60];				/* ѧԱ���֤�� */
	char name[20];					/* ѧԱ���� */
	char subjectName[32];				/* ѧԱ��ѧ��Ŀ������Ŀһ����Ŀ������Ŀ�����ѽ�ҵ */
	char faceBuf[MAX_FACE_BUFFER_LEN];	/* ѧԱ���������ļ����� */
    char sqcx[16];                      /* ���복�� */
	int  size;							/* ѧԱ���������ļ���С */
}student_data_t;

/* Ad.xml������ */
typedef struct __ad_config_t_
{
	char adPicBuf[MAX_AD_PIC_BUFFER_LEN];	/* ���ͼƬ���� */
	int size;								/* ���ͼƬ��С */
	int perTime;							/* ���ͼƬ���ż��ʱ�䣬��λΪ:�� */
}ad_config_t;

/* DelTime.xml������ */
typedef struct __delete_time_t_
{
	char deviceNum[32];
	int serialNum;
}delete_time_t;

/* Time.xml������ */
typedef struct __student_record_t_
{
	char deviceNum[32];				/* �豸�� */
	int serialNum;					/* ��� */
	char drivingSchoolName[100];		/* ��У�� */
	char licensePlateNum[20];		/* ���ƺ� */
	char licenseFrameNum[100];		/* ���ܺ� */
	char teacherName[40];			/* ����Ա���� */
	char studentName[20];			/* ѧԱ���� */
	char idCard[60];				/* ѧԱ���֤���� */
	char subjectName[32];			/* ѧԱ��ѧ��Ŀ�� */
	char *pPicBuf;					/* ָ��ѧԱ�ϻ�ʱ����Ƭ��ָ�� */
	int picBufSize;					/* ѧԱ�ϻ�ʱ����Ƭ�Ĵ�С */
	char startTime[32];				/* ѧԱ�ϻ�ʱ�� */
	char endTime[32];				/* ѧԱ�»�ʱ�� */
    char sqcx[16];                  /* ���복�� */
}student_record_t;

/* studentRecords��һ�µĽṹ�� */
typedef struct __student_record_table_
{
	int serialNum;					/* ��� */
	char teacherName[40];			/* ����Ա���� */
	char studentName[20];			/* ѧԱ���� */
	char idCard[60];				/* ѧԱ���֤���� */
	char subjectName[32];			/* ѧԱ��ѧ��Ŀ�� */
	char startTime[32];				/* ѧԱ�ϻ�ʱ�� */
	char endTime[32];				/* ѧԱ�»�ʱ�� */
	char picPath[256];				/* ͼƬ�����·�� */
	int status;						/* ͼƬ�ϴ���PC��״̬��0-δ�ϴ���1-���ϴ� */
    char sqcx[16];                  /* ���복�� */
}student_record_table;

int base64_enc(char *dest, const char *src, int count);
int base64_dec(char* dest,const char* src, int count);

int loadCarXml(char *pPath, car_config_t *pCarConfig);
int loadAdXml(char *pPath, ad_config_t *pAdConfig, int adNum);
int loadStudentXml(char *pPath, student_data_t *pStudentData, int studentNum);
int loadDeleteTimeXml(char *pPath, delete_time_t *pDeleteTime, int deleteNum);
int createTimeXml(char *pPath, student_record_t * pStudentRecord, int recordNum);

#endif//__MYXML_LIB_H__

