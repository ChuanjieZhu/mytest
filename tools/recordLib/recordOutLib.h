#ifndef __RECORD_OUT_LIB_H__
#define __RECORD_OUT_LIB_H__

//查询记录的条件
#define RECORD_CONDITION_ID					0X01		//根据用户id
#define RECORD_CONDITION_USER_TYPE			0X02		//根据用户id
#define RECORD_CONDITION_RECOG_TYPE			0X04		//根据用户识别的类型，（刷卡、一对一、等）
#define RECORD_CONDITION_RECOG_STATUS		0X08		//根据用户识别的结果
#define RECORD_CONDITION_RECOG_SCORES		0X10		//根据用户识别分数
#define RECORD_CONDITION_RECOG_REASON		0X20		//根据用户识别方式

typedef struct __search_condition_t_
{
	unsigned int userId;			//用户id。
	int userType;					//用户类型 （）
	int recogType;					//识别类型 （一对一、刷卡、等）
	int recogStatus;				//识别结果 （）
	int recogMinScores;				//识别分数 （大于该分数）
	int recogMaxScores;				//识别分数 （小于该分数）
	int recogReason;				//识别原因 （快捷键识别）

	unsigned int startTime;			//起始时间
	unsigned int endTime;			//结束时间
	unsigned int startAddr;			//起始地址
	int searchNum;					//要查询的条目数
}search_condition_t;

#ifdef __cplusplus 
extern "C" { 
#endif

/******************************************************************************
 * 函数名称： searchRecord
 * 功能：查找记录，查询完毕后需调用freeSearchRecord()函数进行释放,或自行释放
 * 参数：searchConditionFlag:	查询条件标记 通过searchConditionFlag来控制查询内容
         searchCondition:	查询条件的具体内容  其中的起始时间、结束时间是必须的
         pRecordInfo:		返回的查询结果链表
 * 返回： 查询结果链表
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
int searchRecord(unsigned int searchConditionFlag, search_condition_t *pSearchCondition, record_list_t *pRecordList);

/******************************************************************************
 * 函数名称： freeSearchRecord
 * 功能：释放查询的记录
 * 参数：pRecordList:	查询的记录链表
 * 返回： 无
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
void freeSearchRecord(record_list_t *pRecordList);

/******************************************************************************
 * 函数名称： searchRecordTotal
 * 功能： 获取记录库中总记录的条数
 * 参数：无
 * 返回：条数
 * 创建作者： Jason
 * 创建日期： 2012-10-15
 * 修改作者：
 * 修改日期：
 ******************************************************************************/
int searchRecordTotal();










#ifdef __cplusplus 
}
#endif


#endif
