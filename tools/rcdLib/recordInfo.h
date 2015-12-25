#ifndef __RECORD_INFO_H__
#define __RECORD_INFO_H__

#define MAX_OPERATE_STORAGE_INTERVAL_TIME	120//�������ݵ��洢�豸�ϵļ��ʱ�� ��λ��
#define MAX_OPERATE_QUEUE			        10//��󻺳�ļ�¼��Ŀ��
#define MAX_FILE_INDEX_SIZE			        48//����¼������Ϣ����
#define MAX_RECORD_FILE_PATH_LEN		    128//�洢��Ƭ���ļ���·��

/* ���ݻ��� */
typedef struct __record_buffer_t_
{
	unsigned int time;                                          //�����ʱ��
	char indexFileName[MAX_RECORD_FILE_PATH_LEN];               //�����洢���ļ�
	unsigned int startIndexNo;                                           //Ҫд�����������ʼ���
	int indexOffset;                                            //����ƫ�ƵĴ�С
	char indexBuffer[MAX_OPERATE_QUEUE * MAX_FILE_INDEX_SIZE];  //��������
}record_buffer_t;

#if 0
/* ��ȡ�����ܴ�С */
int getStorageTotalSize(int *pSdTotalSize, int *pFlashTotalSize);

/* ��ȡ���̿��пռ��С */
int getStorageFreeSize(int *pSdTotalSize, int *pFlashTotalSize);
#endif

#endif//__RECORD_INFO_H__

