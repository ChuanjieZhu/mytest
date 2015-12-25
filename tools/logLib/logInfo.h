#ifndef __LOG_INFO_H_
#define __LOG_INFO_H_

#define MAX_OPERATE_STORAGE_INTERVAL_TIME	300//�������ݵ��洢�豸�ϵļ��ʱ�� ��λ��
#define MAX_OPERATE_QUEUE			10          //��󻺳�ļ�¼��Ŀ��
#define MAX_FILE_INDEX_SIZE			256          //����¼������Ϣ����
#define MAX_RECORD_FILE_NAME_LEN		16      //�����¼��Ƭ���ļ�������󳤶�
#define MAX_RECORD_FILE_PATH_LEN		128     //�洢��Ƭ���ļ���·��

typedef struct __syslog_buffer_t_
{
	unsigned int time;//�����ʱ��  ֻȡ������
	
	char indexFileName[MAX_RECORD_FILE_PATH_LEN];//�����洢���ļ�
	int startIndexNo;//Ҫд�����������ʼ���
	int indexOffset;//����ƫ�ƵĴ�С
	char indexBuffer[MAX_OPERATE_QUEUE * MAX_FILE_INDEX_SIZE];//��������
} syslog_buffer_t;

typedef struct __manlog_buffer_t_
{
	unsigned int time;//�����ʱ��  ֻȡ������
	
	char indexFileName[MAX_RECORD_FILE_PATH_LEN];//�����洢���ļ�
	int startIndexNo;//Ҫд�����������ʼ���
	int indexOffset;//����ƫ�ƵĴ�С
	char indexBuffer[MAX_OPERATE_QUEUE * MAX_FILE_INDEX_SIZE];//��������
} manlog_buffer_t;

typedef struct __accesslog_buffer_t_
{
    unsigned int time;//�����ʱ��  ֻȡ������
	
	char indexFileName[MAX_RECORD_FILE_PATH_LEN];//�����洢���ļ�
	int startIndexNo;//Ҫд�����������ʼ���
	int indexOffset;//����ƫ�ƵĴ�С
	char indexBuffer[MAX_OPERATE_QUEUE * MAX_FILE_INDEX_SIZE];//��������
} accesslog_buffer_t;

#endif //__LOG_INFO_H_
