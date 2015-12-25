#ifndef __RECORD_OUT_LIB_H__
#define __RECORD_OUT_LIB_H__

//��ѯ��¼������
#define RECORD_CONDITION_ID					0X01		//�����û�id
#define RECORD_CONDITION_USER_TYPE			0X02		//�����û�id
#define RECORD_CONDITION_RECOG_TYPE			0X04		//�����û�ʶ������ͣ���ˢ����һ��һ���ȣ�
#define RECORD_CONDITION_RECOG_STATUS		0X08		//�����û�ʶ��Ľ��
#define RECORD_CONDITION_RECOG_SCORES		0X10		//�����û�ʶ�����
#define RECORD_CONDITION_RECOG_REASON		0X20		//�����û�ʶ��ʽ

typedef struct __search_condition_t_
{
	unsigned int userId;			//�û�id��
	int userType;					//�û����� ����
	int recogType;					//ʶ������ ��һ��һ��ˢ�����ȣ�
	int recogStatus;				//ʶ���� ����
	int recogMinScores;				//ʶ����� �����ڸ÷�����
	int recogMaxScores;				//ʶ����� ��С�ڸ÷�����
	int recogReason;				//ʶ��ԭ�� ����ݼ�ʶ��

	unsigned int startTime;			//��ʼʱ��
	unsigned int endTime;			//����ʱ��
	unsigned int startAddr;			//��ʼ��ַ
	int searchNum;					//Ҫ��ѯ����Ŀ��
}search_condition_t;

#ifdef __cplusplus 
extern "C" { 
#endif

/******************************************************************************
 * �������ƣ� searchRecord
 * ���ܣ����Ҽ�¼����ѯ��Ϻ������freeSearchRecord()���������ͷ�,�������ͷ�
 * ������searchConditionFlag:	��ѯ������� ͨ��searchConditionFlag�����Ʋ�ѯ����
         searchCondition:	��ѯ�����ľ�������  ���е���ʼʱ�䡢����ʱ���Ǳ����
         pRecordInfo:		���صĲ�ѯ�������
 * ���أ� ��ѯ�������
 * �������ߣ� Jason
 * �������ڣ� 2012-10-15
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
int searchRecord(unsigned int searchConditionFlag, search_condition_t *pSearchCondition, record_list_t *pRecordList);

/******************************************************************************
 * �������ƣ� freeSearchRecord
 * ���ܣ��ͷŲ�ѯ�ļ�¼
 * ������pRecordList:	��ѯ�ļ�¼����
 * ���أ� ��
 * �������ߣ� Jason
 * �������ڣ� 2012-10-15
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
void freeSearchRecord(record_list_t *pRecordList);

/******************************************************************************
 * �������ƣ� searchRecordTotal
 * ���ܣ� ��ȡ��¼�����ܼ�¼������
 * ��������
 * ���أ�����
 * �������ߣ� Jason
 * �������ڣ� 2012-10-15
 * �޸����ߣ�
 * �޸����ڣ�
 ******************************************************************************/
int searchRecordTotal();










#ifdef __cplusplus 
}
#endif


#endif
