#ifndef __MYXML_LIB_H__
#define __MYXML_LIB_H__

/* 最大广告图片大小为60k字节 */
#define MAX_AD_PIC_BUFFER_LEN	(60 * 1024)

/* 最大学员人脸特征文件大小为100k字节 */
#define MAX_FACE_BUFFER_LEN	(100 * 1024)

/* Car.xml的配置 */
typedef struct __car_config_t_
{
	int format;
	int emptyStudents;
	int emptyDutyDatas;
	int volume;
	char licensePlateNumber[20];	/* 车牌号 */
	char licenseFrameNumber[100];	/* 车架号 */
	char teacherName[40];			/* 教练姓名 */
    char drivingSchoolName[100];    /* 驾校名称 */
}car_config_t;

/* Student.xml的配置 */
typedef struct __student_data_t_
{
	char idCard[60];				/* 学员身份证号 */
	char name[20];					/* 学员姓名 */
	char subjectName[32];				/* 学员所学科目名，科目一，科目二，科目三，已结业 */
	char faceBuf[MAX_FACE_BUFFER_LEN];	/* 学员人脸特征文件内容 */
    char sqcx[16];                      /* 申请车型 */
	int  size;							/* 学员人脸特征文件大小 */
}student_data_t;

/* Ad.xml的配置 */
typedef struct __ad_config_t_
{
	char adPicBuf[MAX_AD_PIC_BUFFER_LEN];	/* 广告图片内容 */
	int size;								/* 广告图片大小 */
	int perTime;							/* 广告图片播放间隔时间，单位为:秒 */
}ad_config_t;

/* DelTime.xml的配置 */
typedef struct __delete_time_t_
{
	char deviceNum[32];
	int serialNum;
}delete_time_t;

/* Time.xml的配置 */
typedef struct __student_record_t_
{
	char deviceNum[32];				/* 设备号 */
	int serialNum;					/* 序号 */
	char drivingSchoolName[100];		/* 驾校名 */
	char licensePlateNum[20];		/* 车牌号 */
	char licenseFrameNum[100];		/* 车架号 */
	char teacherName[40];			/* 教练员姓名 */
	char studentName[20];			/* 学员姓名 */
	char idCard[60];				/* 学员身份证号码 */
	char subjectName[32];			/* 学员所学科目名 */
	char *pPicBuf;					/* 指向学员上机时刻照片的指针 */
	int picBufSize;					/* 学员上机时刻照片的大小 */
	char startTime[32];				/* 学员上机时刻 */
	char endTime[32];				/* 学员下机时刻 */
    char sqcx[16];                  /* 申请车型 */
}student_record_t;

/* studentRecords表一致的结构体 */
typedef struct __student_record_table_
{
	int serialNum;					/* 序号 */
	char teacherName[40];			/* 教练员姓名 */
	char studentName[20];			/* 学员姓名 */
	char idCard[60];				/* 学员身份证号码 */
	char subjectName[32];			/* 学员所学科目名 */
	char startTime[32];				/* 学员上机时刻 */
	char endTime[32];				/* 学员下机时刻 */
	char picPath[256];				/* 图片保存的路径 */
	int status;						/* 图片上传到PC的状态，0-未上传，1-已上传 */
    char sqcx[16];                  /* 申请车型 */
}student_record_table;

int base64_enc(char *dest, const char *src, int count);
int base64_dec(char* dest,const char* src, int count);

int loadCarXml(char *pPath, car_config_t *pCarConfig);
int loadAdXml(char *pPath, ad_config_t *pAdConfig, int adNum);
int loadStudentXml(char *pPath, student_data_t *pStudentData, int studentNum);
int loadDeleteTimeXml(char *pPath, delete_time_t *pDeleteTime, int deleteNum);
int createTimeXml(char *pPath, student_record_t * pStudentRecord, int recordNum);

#endif//__MYXML_LIB_H__

