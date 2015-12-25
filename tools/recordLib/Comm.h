#ifndef _COMM_H
#define _COMM_H

/* ��¼��Ϣ */
typedef struct __operate_event_t_
{
	unsigned int operateType;//����  bit0�����ݿ����  bit1����¼��Ƭ
	unsigned int id;//�û�id
	int nServerID;
	int nChannelID;
	int nThreshold;//ʶ������
	int recogScores;//ʶ�����
	int nType;// 0:��  1:����
	int nStatus;//ʶ��״̬
	int nUserType;//�û�����
	int nLoginReason;//����ԭ��
	unsigned int time;//�¼�������ʱ��
	unsigned int fileSize;//��¼��Ƭ�ļ���С (������ʼͷ��������Ϣ��jpg��Ƭ������ͷ�Ĵ�С)
	char cRecogMode;//ʶ��ģʽ
	char *pFileData;//��¼��Ƭ��ͼƬ����
}operate_event_t;

/* G1J���ͣ��ڰ�ͼ����ת����Ϊ˳ʱ����ת90�� */
#define YUV_ROTATE_PARAM 1

#define MAX_TMP_LEN 512

#define MAX_RAW_JPG_SIZE 80 * 1024

#endif  /* _COMM_H */
