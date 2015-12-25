#ifndef __RECORD_INFO_H__
#define __RECORD_INFO_H__

#define MAX_OPERATE_STORAGE_INTERVAL_TIME	300//�������ݵ��洢�豸�ϵļ��ʱ�� ��λ��
#define MAX_OPERATE_QUEUE			30//��󻺳�ļ�¼��Ŀ��
#define MAX_FILE_INDEX_SIZE			64//����¼������Ϣ����
#define MAX_RECORD_FILE_NAME_LEN		16//�����¼��Ƭ���ļ�������󳤶�
#define MAX_RECORD_FILE_PATH_LEN		128//�洢��Ƭ���ļ���·��

/* ���ݻ��� */
typedef struct __record_buffer_t_
{
	unsigned int time;//�����ʱ��  ֻȡ������

	char picFileName[MAX_RECORD_FILE_PATH_LEN];//��Ƭ�洢���ļ�
	int startAddr;//Ҫд�����Ƭ���ļ�����ʼ��ַ
	int picOffset;//��Ƭƫ�ƵĴ�С
	char *pPicBuffer;//��Ƭ����

	char indexFileName[MAX_RECORD_FILE_PATH_LEN];//�����洢���ļ�
	int startIndexNo;//Ҫд�����������ʼ���
	int indexOffset;//����ƫ�ƵĴ�С
	char indexBuffer[MAX_OPERATE_QUEUE*MAX_FILE_INDEX_SIZE];//��������
}record_buffer_t;

#if 0
/* ��ȡ�����ܴ�С */
int getStorageTotalSize(int *pSdTotalSize, int *pFlashTotalSize);

/* ��ȡ���̿��пռ��С */
int getStorageFreeSize(int *pSdTotalSize, int *pFlashTotalSize);
#endif

#endif//__RECORD_INFO_H__

