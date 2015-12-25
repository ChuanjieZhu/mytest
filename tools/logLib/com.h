/**/

#ifndef __COM_H_
#define __COM_H_

#define MAX_MSG_LEN 64
#define MAX_USER_NAME_LEN   16

         
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


/*
 * ϵͳ��־
 */
typedef struct __syslog_event_t_
{
    char cType;                     //��־����
    char cSubType;                  //��־������
    unsigned int uiTime;            //��־ʱ��
    int  iOpType;                   //��������
    char cResult;                   //�������
    char acMsg[MAX_MSG_LEN];        //������ϸ���
    char unused[13];                //���� 
} syslog_event_t;

/*
 * ������־
 */
typedef struct __manlog_event_t_
{
    char cType;                     //��־����
    char cSubType;                  //��־������
    unsigned int uiTime;            //��־ʱ��
    char acUserName[MAX_USER_NAME_LEN]; //����������
    unsigned int uiUserNo;          //�û�����
    unsigned int uiUserId;          //�û�id��
    int  iOpType;                   //��������
    char cResult;                   //�������
    char acMsg[MAX_MSG_LEN];        //������ϸ���
    char unused[13];               //���� 
} manlog_event_t;


/*
 * �Ž���־
 */
typedef struct __accesslog_event_t_
{
    char cType;                  //��־����
    char cRecogMode;            //ʶ��ģʽ
	int iAccessMode;				//����ģʽ
	int iUserNum;					//�����û���
	unsigned int uiUserId[5];		//�����û�id
	int iGroupNum;					//��������
	unsigned int uiGroupId[5];		//������id
	unsigned int uiTime;			//ʱ��
	int iResult;					//���
	int iPhotoNum;					//���Ź�����ץ����Ƭ����
	unsigned int uiPhotoSize[5];	//ץ����Ƭ����
	unsigned int uiPhotoTime[5];    //��Ƭ����ʱ�䣬����ȷ����Ƭ���������ļ�
	unsigned int uiPhotoStartAddr[5];			//ץ����Ƭ������־�ļ���λ��ƫԶ
} accesslog_event_t;

#endif //__COM_H_
    
