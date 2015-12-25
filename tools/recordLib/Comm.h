#ifndef _COMM_H
#define _COMM_H

/* 记录信息 */
typedef struct __operate_event_t_
{
	unsigned int operateType;//类型  bit0：数据库操作  bit1：记录照片
	unsigned int id;//用户id
	int nServerID;
	int nChannelID;
	int nThreshold;//识别门限
	int recogScores;//识别分数
	int nType;// 0:卡  1:其它
	int nStatus;//识别状态
	int nUserType;//用户类型
	int nLoginReason;//考勤原因
	unsigned int time;//事件产生的时间
	unsigned int fileSize;//记录照片文件大小 (包括开始头、索引信息、jpg照片、结束头的大小)
	char cRecogMode;//识别模式
	char *pFileData;//记录照片的图片数据
}operate_event_t;

/* G1J机型，黑白图像旋转方向为顺时针旋转90度 */
#define YUV_ROTATE_PARAM 1

#define MAX_TMP_LEN 512

#define MAX_RAW_JPG_SIZE 80 * 1024

#endif  /* _COMM_H */
