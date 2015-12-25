/*******************************************************************************
**  Copyright (c) 2014, Company
**  All rights reserved.
**	
**  �ļ�˵��: cpu �������ӿ�
**  ��������: 2014.02.26
**
**  ��ǰ�汾��1.0
**  ���ߣ�
********************************************************************************/

#ifndef _CPU_H_
#define _CPU_H_

/* Բ־���Ŷ�ͷ������ */
#define FRAME_HEAD           	0x02
#define FRAME_END           	0x03
#define SET_SERIAL_BAUD_RATE  	0x15
#define SEARCH_CARD           	0x46
#define ESCAPE_CONFLICT       	0x47
#define CHOOSE_CARD           	0x48
#define CHECK_SECRET_KEY     	0x4A
#define READ_BLOCK            	0x4B
#define WRITE_BLOCK           	0x4C

/* Բ־���Ŷ�ͷCPU������ */
#define SET_TYPE_A_MODE 		0x3A	/* ���ö���ģ�鹤����ISO14443 TYPE A����ģʽ */		
#define RESET_CPU_CARD			0x53	/* ��λ��ָ�� */
#define CPU_CARD_COS			0x54	/* ����COSָ�� */

#define COS_EXTERNAL_AUTH		0X82 	/* ����ģ����֤ */
#define COS_GET_CHALLENGE		0x84	/* ��ȡ����� */
#define COS_WRITE_KEY			0xF0	/* �������޸Ļ����¼�����Կ */
#define COS_CREATE_FILE			0xE0	/* ����ר��COSָ������ļ� */
#define COS_SELECT_FILE			0xA4	/* COSָ�ѡ���ļ� */
#define COS_UPDATE_FILE			0xD6 	/* COSָ�д���޸��ļ� */
#define COS_READ_BINARY			0xB0	/* COSָ����ļ� */
#define COS_ERASE_DF			0x0E 	/* ɾ��DF/MF */

#define COS_MF_FILE				0x38
#define COS_KEY_FILE			0x08
#define COS_EF_FILE				0x01

#define COS_MAX_WRITE_SIZE		0x80	/* ÿ��д�ֽ��� 128�ֽ� */
#define COS_MAX_READ_SIZE		0x28	/* ÿ�ζ��ֽ��� 40�ֽ� */
#define COS_FEATURE_SIZE		6184	/* 6 * 1024 + 4 + 4 + 32 */

#define MAX_FILE_SIZE 		    8 * 1024
#define WRITE_FILE_PATH         "/root/feature.dat"

#ifdef __cplusplus
extern "C" {
#endif

int CpuCardSetCmd(int hCard, unsigned char *Cmd, int DataLen);

int CpuSearchCard(int fd);

int CpuResetCard(int fd);

int CosGetChallenge(int fd, unsigned char uBytes);

int CosCreateFile(int fd, unsigned char *pFileMark, unsigned char uFileType, unsigned char *pFileLen, 
    unsigned char *pFileName, unsigned char ucNameLen,  int iMode);

int CosReadBinary(int fd, unsigned char uParaOne, unsigned char uParaTwo, unsigned char uReadSize);

int CosReadBinaryFile(int fd, short ifilesize);

int CosSelectFile(int fd, unsigned char ucParamOne, unsigned char ucParamTwo, 
                                unsigned char *pFileName, unsigned char ucNameLen);
int CosWriteKeyPin(int fd, unsigned char ucKeyIndex);

int CosExternalAuth(int fd, unsigned char *pKey);

int CosWriteBinary(int fd, unsigned char ucParaOne, unsigned char ucParaTwo, 
                                    unsigned char *pWriteBuff, unsigned char ucWriteLen);

int CosWriteBinaryFile(int fd, short sFilesSize);

int CosEraseDF(int fd, unsigned char *pTkey, int ikeylen);

int CosReadFile(int fd);

int CosWriteFile(int fd);

#ifdef __cplusplus
}
#endif

#endif //_CPU_H_
