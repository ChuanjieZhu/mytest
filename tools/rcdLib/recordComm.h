
#ifndef __RECORD_COMM_H__
#define __RECORD_COMM_H__

/* ��¼��Ϣ */
typedef struct __operate_event_t_
{
	unsigned int operateType;       //����  bit0�����ݿ����
	unsigned int recordTime;        //��¼������ʱ��
	char recordType;        //��¼���ͣ�0-������1-Сʱ���
    unsigned int inCount;     //������
    unsigned int outCount;    //������
} operate_event_t;

#define MAX_TMP_LEN 512

#endif  /* __COMM_H__ */

