/**/

#ifndef __COM_H_
#define __COM_H_

#define MAX_MSG_LEN 64
#define MAX_USER_NAME_LEN   16

         
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


/*
 * 系统日志
 */
typedef struct __syslog_event_t_
{
    char cType;                     //日志类型
    char cSubType;                  //日志子类型
    unsigned int uiTime;            //日志时间
    int  iOpType;                   //操作类型
    char cResult;                   //操作结果
    char acMsg[MAX_MSG_LEN];        //操作详细结果
    char unused[13];                //保留 
} syslog_event_t;

/*
 * 管理日志
 */
typedef struct __manlog_event_t_
{
    char cType;                     //日志类型
    char cSubType;                  //日志子类型
    unsigned int uiTime;            //日志时间
    char acUserName[MAX_USER_NAME_LEN]; //操作者姓名
    unsigned int uiUserNo;          //用户工号
    unsigned int uiUserId;          //用户id号
    int  iOpType;                   //操作类型
    char cResult;                   //操作结果
    char acMsg[MAX_MSG_LEN];        //操作详细结果
    char unused[13];               //保留 
} manlog_event_t;


/*
 * 门禁日志
 */
typedef struct __accesslog_event_t_
{
    char cType;                  //日志类型
    char cRecogMode;            //识别模式
	int iAccessMode;				//开门模式
	int iUserNum;					//开门用户数
	unsigned int uiUserId[5];		//开门用户id
	int iGroupNum;					//开门组数
	unsigned int uiGroupId[5];		//开门组id
	unsigned int uiTime;			//时间
	int iResult;					//结果
	int iPhotoNum;					//开门过程中抓拍照片数量
	unsigned int uiPhotoSize[5];	//抓拍照片长度
	unsigned int uiPhotoTime[5];    //照片生成时间，用于确定照片所在索引文件
	unsigned int uiPhotoStartAddr[5];			//抓拍照片在在日志文件中位置偏远
} accesslog_event_t;

#endif //__COM_H_
    
