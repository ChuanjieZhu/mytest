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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include "cpu.h"
#include "mac.h"
#include "des.h"

/* transfer key ��Ƭ������Կ */
unsigned char s_TKey[] = {
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff
};

/* MF name */
unsigned char ucMfName[] = {
	0x31, 0x50, 0x41, 0x59, 
	0x2E, 0x53, 0x59, 0x53, 
	0x2E, 0x44, 0x44, 0x46, 
	0x30, 0x31
};

/* Main key MF������Կ */
unsigned char s_MF_MKey[] = {
	0x46, 0x49, 0x52, 0x53,
	0x2D, 0x2D, 0x74, 0x65,
	0x63, 0x68, 0x6E, 0x6F,
	0x6C, 0x6F, 0x67, 0x79
};

/* Maintain key MFά����Կ */
unsigned char s_MF_MaintainKey[] = {
	0x46, 0x49, 0x52, 0x53,
	0x2D, 0x2D, 0x74, 0x65,
	0x63, 0x68, 0x6E, 0x6F,
	0x6C, 0x6F, 0x67, 0x79
};

/* EF name */
unsigned char ucEfName[] = {
	0x12, 0x35
};

/* M1�������� */
unsigned int ICCardSectorNo = 4;

/* ��������� */
unsigned char gacChallenge[8] = {0};								
char g_readFeatureBuf[MAX_FILE_SIZE] = {0};			    /* �������� */
unsigned char g_writeFeatureBuf[MAX_FILE_SIZE] = {0};			/* д������ */
int g_iOffset = 0;										/* �洢��������ʱ����������ƫ�Ʊ��� */
int g_iWriteCardRetrueCode = -2;						/* д�������У����صĽ���룬���ݸý���룬������ͬ�Ĵ�����ʾ */
int g_writeOffset = 0;									/* �ñ�����д��ʱ��ʾд�������� */
int g_readOffset = 0;									/* ����������ƫ����ʾ */

static int ReadFile(unsigned char *pBuf, int nBufSize, char *pFileName)
{
	int filefd = -1;
	int nfilelen = 0;
	int nreadlen = 0;
	struct stat st;

	if (pBuf == NULL || pFileName == NULL)
	{

		return -1;
	}

	filefd = open(pFileName, O_RDONLY);
	if (filefd < 0)
	{
		printf("open %s error! %s %d\r\n", pFileName, __FUNCTION__, __LINE__);
		return -1;
	}

	if (stat(pFileName, &st) == 0)
	{
		nfilelen = st.st_size;
	}

	if (nfilelen > nBufSize)
	{
		nfilelen = nBufSize;
	}

	nreadlen = read(filefd, pBuf, nfilelen);

	close(filefd);

	return nreadlen;
}

/********************************************************************************************
�� �� �� : print_hex
�������� : ��ʮ�����Ƶ���ʽ��ӡ�������е�ָ������λ 
�� �� ֵ : 
����˵�� :	char * tip		��ʾ��Ϣ��ASCII�� ��0x00����
			unsigned char * buff	Ҫ��ӡ���ݵĻ�����ָ��
			unsigned char bytes		Ҫ��ӡ�Ļ��������ֽ���
********************************************************************************************/
static void print_hex(char *tip, unsigned char * buff, short bytes, const char *pFuntion, int iLineNum)
{
	unsigned char ucindex = 0;
	
	printf("%s: \r\n", tip);

	for(ucindex = 0; ucindex < bytes; ucindex++)
	{
		if (ucindex != 0 && ucindex % 16 == 0)
		{
			printf("\r\n");
		}
		printf("%02X ", buff[ucindex]);
	}

	printf("\r\n");
	
	if (pFuntion != NULL && iLineNum != 0)
	{
		printf("%s %d\r\n", pFuntion, iLineNum);
	}
}


/*****************************************************************\
�������ƣ�CpuCardRead
��������: ��ȡcpu����ָ��ؽ��
��ڲ�����
		: 
		: 
����ֵ  ��2  - CPU��
		  1  - M1��
		  0  - ָ��ִ�гɹ�
		  -1 - 2��cpu��Ѱ��ʱ����δ����ָ��ʱ��
		  -2 - cpu����ȡ�����ָ��ִ��ʧ��
		  -3 - cpu���ⲿ��ָ֤��ִ��ʧ��
		  -4 - cpu�������ļ�ָ��ִ��ʧ��
		  -5 - 
\*****************************************************************/
static int CpuCardRead(int hCard, unsigned char cmd)
{
	static time_t tCurTime = 0;
	static time_t tLastTime = 0;
	unsigned char buf[512];
	char tmp_buf[1024];
	int ret = -1;
	int rtn = -1;
	int j = 0;
	int i = 0;
	int nCardLen = 0;
	
	fd_set readfds;
	struct timeval tv;
    
	int nCardNo = 0;
	unsigned char ucReadLen = 0;      /* ÿ�ζ�ȡ�ļ��ĳ��� */
	
	memset(buf, 0, sizeof(buf));
	memset(tmp_buf, 0, sizeof(tmp_buf));

    tv.tv_sec=0;
	tv.tv_usec=200 * 1000;
	FD_ZERO(&readfds);
	FD_SET(hCard, &readfds);
    
	ret = select(hCard+1,&readfds,NULL,NULL,&tv);
	if(ret>0 && FD_ISSET(hCard, &readfds))
	{
		/* i��ҪС��5,����ᵼ������û��ȫ */
		for(i = 0; i < 5; i++)
		{   
			ret = read(hCard, tmp_buf+nCardLen, 64);
			nCardLen += ret;
			if (((tmp_buf[0] == 0x02) && (tmp_buf[nCardLen - 1] == 0x03))
			|| ((tmp_buf[0] == 0x13) && (tmp_buf[nCardLen - 1] == 0x03)))
			{
				break;
			}
		}
#if 0
			TRACE("\r\nREAD %d BYTES %s %d\r\n", nCardLen, __FUNCTION__, __LINE__);
			/* ��ӡ��tmp_buf�е����� */
			for (j = 0; j < nCardLen; j++)
			{
				TRACE("buf[%d] 0x%02x\t", j, tmp_buf[j]);
			}
			TRACE("\r\n");
#endif

#if 0
		if(tmp_buf[4] == READ_BLOCK || (tmp_buf[5] == READ_BLOCK && tmp_buf[0] == 0x13))
		{

			TRACE("\r\nREAD %d BYTES %s %d\r\n", nCardLen, __FUNCTION__, __LINE__);

			/* ��ӡ��tmp_buf�е����� */
			for (j = 0; j < nCardLen; j++)
			{
				TRACE("buf[%d] 0x%02x\t", j, tmp_buf[j]);
			}
			TRACE("\r\n");

		}
#endif
		if(tmp_buf[0] == 0x13)
		{
			i = 1;
		}
		else
		{
			i = 0;
		}
		
		if (tmp_buf[i] == FRAME_HEAD && tmp_buf[nCardLen - 1] == FRAME_END)
		{
			j = 0;
			buf[j++] = tmp_buf[i++];

			/* ת�� */
			for(; i < nCardLen;)
			{
				if(tmp_buf[i] == 0x10)
				{
					i++;
					buf[j++] = tmp_buf[i++];
				}
				else
				{
					buf[j++] = tmp_buf[i++];
				}
			}

			if(buf[4] == cmd)
			{
				if(buf[5] == 0x00)
				{
					if ((buf[4] == ESCAPE_CONFLICT) || (buf[4] == READ_BLOCK))
					{
                        /* �������ƣ����ֽ�λ���滻����ȡ��24��bit������ */
                        /*  
                         * ������ԭ��4�ֽ������Ž�ȡ��3�ֽ���ȡ����Ϊֱ����
                         * 4�ֽ�������ȡ��
                         */
						nCardNo = (buf[9] << 24 | buf[8] << 16) | (buf[7] << 8) | buf[6];  
                        
                        //TRACE("nCardNo %u %s %d\r\n", (unsigned int)nCardNo, __FUNCTION__, __LINE__);

                        nCardNo = (~nCardNo) & 0xffffff;

                        printf("nCardNo %u %s %d\r\n", (unsigned int)nCardNo, __FUNCTION__, __LINE__);
						
						if(nCardNo) 
						{
                            //SetCardVal(nCardNo);
							//SetKey(VK_CARD);
						}

						if(buf[4] == ESCAPE_CONFLICT)
						{
							//m_uiCarNo = nCardNo;
						}
					}

					//TRACE("Command 0x%x execute successful %s %d\r\n",buf[4], __FUNCTION__, __LINE__);
					
					rtn = 0;
				}
				
			}
			else
			{
				printf("Respond Error: Receive cmd 0x%x, Need cmd 0x%x\r\n", buf[4], cmd);
			}
		}
	}
	return rtn;
}

int CpuCardSetCmd(int hCard, unsigned char * Cmd, int DataLen)
{
	unsigned char buf[512];
	unsigned char send_buf[512];
	int j = 0;
	int i = 0;
	int ret = -1;
	int index = 0;
	
	memset(buf, 0, sizeof(buf));
	memset(send_buf, 0, sizeof(send_buf));
	
	/* ֡ͷ */
	buf[index++] = FRAME_HEAD;		/* ֡ͷ 0x02 */

	/* ģ���ַ */
	buf[index++] = 0x00;
	buf[index++] = 0x00;

	/* ������ */
	buf[index++] = (char)DataLen;

	/* ���� + ���� */
	for (j = 0; j < (DataLen - 2); j++)
	{
		buf[index++] = Cmd[j];
	}
	
	/* У����-��ģ���ַ�����������һ���ֽڵ����ֽ��ۼ� */
	for (j = 1; j < index; j++)
	{
		buf[index] += buf[j];
	}
	
	index++;
	buf[index] = FRAME_END;
	i = 0;
	j = 0;
	send_buf[i++] = buf[j];
	
	/* ������ݰ�����Ϊ0x02��0x03��0x10�������ӱ�ʶ�ַ�0x10 */
	for (j = 1; j < index; j++)
	{
		if ((buf[j] == 0x02) || (buf[j] == 0x03) || (buf[j] == 0x10))
		{
			send_buf[i++] = 0x10;
			send_buf[i++] = buf[j];
		}
		else
		{
			send_buf[i++] = buf[j];
		}
	}	
	
	send_buf[i++] = buf[j];
	index = i;

	if (send_buf[4] != 0x46)
	{
		print_hex("send buf", send_buf, index, __FUNCTION__, __LINE__);
	}

	ret = write(hCard, send_buf, index);
	if (ret == index)
	{		
		ret = CpuCardRead(hCard, Cmd[0]);	 /* Cmd[0]Ϊָ������ */
	}
	else
	{
		ret = -1;
	}

	return ret;
}

int CpuSearchCard(int fd)
{
	int iret = -1;
	int icmdlen = 0;
	unsigned char cmd[64];
	
	memset(cmd, 0, sizeof(cmd));
	
	cmd[0] = SEARCH_CARD;
	cmd[1] = 0x52;

	icmdlen = 4;
	
	iret = CpuCardSetCmd(fd, cmd, icmdlen);

	return iret;
}

/*
	��λcpu���ӿ�
*/
int CpuResetCard(int fd)
{
	int iret = -1;
	int icmdlen = 0;
	unsigned char cmd[64];
	
	memset(cmd, 0, sizeof(cmd));
	
	cmd[0] = RESET_CPU_CARD;
	cmd[1] = 0x26;
	
	icmdlen = 4;
	
	iret = CpuCardSetCmd(fd, cmd, icmdlen);

	return iret;
}


/* ��ȡ����� */
int CosGetChallenge(int fd, unsigned char uBytes)
{
	unsigned char cmd[64] = {0};
	int iret = -1;
	
	memset(cmd, 0, sizeof(cmd));

	cmd[0] = CPU_CARD_COS;
	cmd[1] = 0x00;
	cmd[2] = COS_GET_CHALLENGE;
	cmd[3] = 0x00;
	cmd[4] = 0x00;
	cmd[5] = uBytes;
	
	iret = CpuCardSetCmd(fd, cmd, 8);

	return iret;
}

int CosCreateFile(int fd, unsigned char *pFileMark, unsigned char uFileType, unsigned char *pFileLen, 
    unsigned char *pFileName, unsigned char ucNameLen,  int iMode)
{
	int iret = -1;
	int iMacLen;
	int iInputLen = 0;
	int iCmdLen = 0;
	unsigned char cmd[64] = {0};
	unsigned char ucMac[4];
	unsigned char *pucKey = NULL;
	
	memset(cmd, 0, sizeof(cmd));
	memset(ucMac, 0, sizeof(ucMac));

	/* ���ܷ�ʽ */
	if (iMode == 1)
	{
		cmd[0] = CPU_CARD_COS;

		/* -----����ͷ, ����macʱ��cmd[1]��ʼ���㳤��-------------- */

		cmd[1] = 0x04;				//CLA
		cmd[2] = COS_CREATE_FILE;	//INS
		cmd[3] = 0x00;				//P1	 Ĭ��0x00

		if (uFileType == COS_MF_FILE) 		/* Create MF */
		{
			cmd[4] = uFileType;			//P2	 filetype
			cmd[5] = 0x1e;				//LC		
			memcpy(cmd + 6, pFileMark, 2);  // 3F00�ļ���ʶ
			cmd[8] = uFileType;				// 38�ļ�����
			memcpy(cmd + 9, pFileLen, 2);	//�ļ�����
			cmd[11] = 0x5e;				// 01011110 �ļ����� //0x42
			cmd[12] = 0x00;				// RFU
			cmd[13] = 0x00;				// RFU
			cmd[14] = 0x00;				// RFU
			cmd[15] = 0x00;				/* ����Ȩ�� */
			cmd[16] = 0x91;				/* ����Ȩ�� */
			cmd[17] = ucNameLen;		/* �ļ������� */
			memcpy(cmd + 18, pFileName, ucNameLen); 				/* �ļ��� */

			/* ����MFʱ����MAC�ô�����Կ */
			pucKey = s_TKey;

			/* MAC�����ַ������� */
			iInputLen = 18 - 1 + ucNameLen;
		}
		else if (uFileType == COS_KEY_FILE) /* create key file */
		{
			cmd[4] = uFileType;			//P2	  filetype
			cmd[5] = 0x0F;				//LC
			
			cmd[6] = 0xFF;				// FF02 key/pin�ļ���ʶ
			cmd[7] = 0x02;				//

			cmd[8] = uFileType;			// 0x08�ļ�����
			cmd[9] = 0x04;				// key����
			cmd[10] = 0x14;				// key��¼����

			cmd[11] = 0xC0;				// 11000000 �ļ�����
			/* RFU */
			cmd[12] = 0x00;				// RFU
			cmd[13] = 0x00;				// RFU
			cmd[14] = 0x00;				// RFU

			/* ����Ȩ�� */
			cmd[15] = 0x01;
			/* ����Ȩ�� */
			cmd[16] = 0x01;

			/* ����MF��key�ļ�ʱ����MAC�ô�����Կ */
			pucKey = s_TKey;

			/* MAC�����ַ������� */
			iInputLen = 16;
		}
		else if (uFileType == COS_EF_FILE)
		{
			cmd[4]  = uFileType;
			cmd[5]  = 0x0f;					/* ���ݳ��� */
			
			memcpy(cmd + 6, pFileMark, 2);	/* 2�ֽ��ļ���ʶ */

			cmd[8]  = uFileType;			/* �ļ����� */

			memcpy(cmd + 9, pFileLen, 2);	/* 2�ֽ��ļ����� */	
			
			cmd[11] = 0x00;			/* �ļ�����0x00,��д����ʹ�ü��ܱ��� */
			
			cmd[12] = 0x00;			/* FRU */
			cmd[13] = 0x00;
			cmd[14] = 0x00;
			
			cmd[15] = 0x00;			/* ��Ȩ�� */
			cmd[16] = 0x00;			/* дȨ�� */
			
			/* ����MF��EF�ļ�ʱ��MF������Կ����MAC */
			pucKey = s_MF_MKey;

			/* MAC�����ַ������� */
			iInputLen = 16;
		}

		/* get 4 bytes random number */
		if (CosGetChallenge(fd, 0x04) == 0)
		{	
			/* get 4 bytes mac */
			TDesEncryptMac(pucKey, cmd + 1, iInputLen, ucMac, &iMacLen, gacChallenge);
		}
		
		/* total lenght add the mac result length 4bytes */
		if (uFileType == COS_MF_FILE)
		{
			memcpy(cmd + iInputLen + 1, ucMac, 4);
			iCmdLen = iInputLen + 1 + 4 + 2;
		}
		else if (uFileType == COS_KEY_FILE)
		{
			memcpy(cmd + 17, ucMac, 4);
			iCmdLen = 23;
		}
		else if (uFileType == COS_EF_FILE)
		{
			memcpy(cmd + 17, ucMac, 4);
			iCmdLen = 23;
		}
	}
	/* �Ǽ��ܷ�ʽ */
	else
	{
		cmd[0] = CPU_CARD_COS;
		cmd[1] = 0x00;				//CLA
		cmd[2] = COS_CREATE_FILE;	//INS
		cmd[3] = 0x00;	
		
		if (uFileType == COS_EF_FILE)
		{
			cmd[4]  = uFileType;
			cmd[5]  = 0x0b;					/* ���ݳ��� */
			memcpy(cmd + 6, pFileMark, 2);	/* 2�ֽ��ļ���ʶ */
			cmd[8]  = uFileType;			/* �ļ����� */
			memcpy(cmd + 9, pFileLen, 2);	/* 2�ֽ��ļ����� */			

			cmd[11] = 0x00;			/* �ļ����� */
			cmd[12] = 0x00;			/* FRU */
			cmd[13] = 0x00;
			cmd[14] = 0x00;
			cmd[15] = 0x00;			/* ��Ȩ�� */
			cmd[16] = 0x00;			/* дȨ�� */
		}

		iCmdLen = 19;
	}

	iret = CpuCardSetCmd(fd, cmd, iCmdLen);
	
	return iret;
}

/*
	ucParaOne: p1����
	ucParaTwo: p2����
	ucReadSize: ÿ�ζ�ȡ�ļ���С
*/
int CosReadBinary(int fd, unsigned char uParaOne, unsigned char uParaTwo, unsigned char uReadSize)
{
	int iRet = -1;
	int iCmdLen = 0;
	unsigned char ucCmd[64];
	
	memset(ucCmd, 0, sizeof(ucCmd));
	ucCmd[0] = CPU_CARD_COS;
	ucCmd[1] = 0x00;
	ucCmd[2] = COS_READ_BINARY;
	ucCmd[3] = uParaOne;  		/* p1 */
	ucCmd[4] = uParaTwo;		/* p2 */
	ucCmd[5] = uReadSize;		/* Le */

	iCmdLen = 6 + 2;
		
	iRet = CpuCardSetCmd(fd, ucCmd, iCmdLen);

	return iRet;
	
}


int CosReadBinaryFile(int fd, short ifilesize)
{
    int iret = -1;
	short ireadoffset = 0;
    
	unsigned char ureadsize = 0;
    unsigned char uparaone = 0;
    unsigned char uparatwo = 0;
    
	int icount = 0;
	int i = 0;

	icount = ifilesize / COS_MAX_READ_SIZE;
	if ((ifilesize - (icount * COS_MAX_READ_SIZE)) > 0)
	{
		icount++;
	}

	for (i = 0; i < icount; i++)
	{
		if (i < icount - 1)
		{
			ureadsize = COS_MAX_READ_SIZE;
		}
		else
		{
			ureadsize = ifilesize - (i * COS_MAX_READ_SIZE);
		}

		ireadoffset = i * COS_MAX_READ_SIZE;
		uparaone = (ireadoffset >> 8) & 0x00FF;
		uparatwo = ireadoffset & 0x00FF;

		iret = CosReadBinary(fd, uparaone, uparatwo, ureadsize);

		if (iret < 0)
		{
			break;
		}
	}

    return iret;
}
/*
	cosѡ���ļ���pFileName: ��ucType������ucNameLen: �ļ�������
	ucParamOne: p1���� 
	0x00: ���ļ���ʶ��ѡ��MF��DF��EF(XX=�ļ���ʶ����Ϊ��)
	0x02: ���ļ���ʶ���ڵ�ǰDF ��ѡ��EF(XX=EF ���ļ���ʶ��)
	0x04: ��DF �ļ���ֱ��ѡ��DF(XX=DF ���ļ���)
	0x03: ѡ��ǰDF �ĸ�DF��Data Ϊ�գ�

	ucParamTwo: p2����
	0x00: ��һ����Ψһ���ļ�ʵ��
	0x20: ��һ���ļ�ʵ��
*/
int CosSelectFile(int fd, unsigned char ucParamOne, unsigned char ucParamTwo, 
                                unsigned char *pFileName, unsigned char ucNameLen)
{
	int iRet = -1;
	int iCmdLen = 0;
	unsigned char ucCmd[64];
	
	memset(ucCmd, 0, sizeof(ucCmd));
	
	ucCmd[0] = CPU_CARD_COS;
	ucCmd[1] = 0x00;
	ucCmd[2] = COS_SELECT_FILE;
	
	ucCmd[3] = ucParamOne;  	/* p1-�ļ�ѡ��ʽ */
	ucCmd[4] = ucParamTwo;		/* p2-ѡ���һ����Ψһ���ļ�ʵ�� */
	ucCmd[5] = ucNameLen;		/* Lc */

	memcpy(ucCmd + 6, pFileName, ucNameLen);

	iCmdLen = 6 + ucNameLen + 2;
		
	iRet = CpuCardSetCmd(fd, ucCmd, iCmdLen);

	return iRet;
}

int CosWriteKeyPin(int fd, unsigned char ucKeyIndex)
{
	int iret = -1;
		
	int iCmdLen = 0;
	unsigned char cmd[64];
	
	unsigned char ucTDesInput[64];		/* 20 �ֽڴ��������� */
	unsigned char ucTDesOutput[64];		/* 24 �ֽ�3des���������20�ֽڣ���3�Σ�ÿ��8�ֽ���3des���� */
	int iTDesOutputLen = 0;		/* TDes������� */
	
	unsigned char ucMacInput[29];		/* mac ���� */	
	unsigned char ucMacOutput[4];		/* mac ���4�ֽ� */
	int iMacOutputLen = 0;		/* mac ������� */
	
	memset(cmd, 0, sizeof(cmd));
	memset(ucTDesInput, 0, sizeof(ucTDesInput));
	memset(ucTDesOutput, 0, sizeof(ucTDesOutput));
	memset(ucMacInput, 0, sizeof(ucMacInput));
	memset(ucMacOutput, 0, sizeof(ucMacOutput));

	if (CosGetChallenge(fd, 0x04) >= 0)
	{
		print_hex("==============RANDOM", gacChallenge, 8, __FUNCTION__, __LINE__);
		
		/* key index is 0x01 */
		if (ucKeyIndex == 0x01)
		{
			/* get 3des (DATA_ENC) */
			ucTDesInput[0] = 0x14;	/* ����cpu��Ҫ�� ���ֽڲ�0x14 */
			ucTDesInput[1] = 0x10;
			ucTDesInput[2] = 0x01;
			ucTDesInput[3] = 0x01;
			ucTDesInput[4] = 0xff;
			
			memcpy(ucTDesInput + 5, s_MF_MKey, sizeof(s_MF_MKey));
			
			print_hex("==============DATA_ENC INPUT", ucTDesInput, sizeof(s_MF_MKey) + 5, __FUNCTION__, __LINE__);
			
			TDesRun(ucTDesInput, 21, s_TKey, ucTDesOutput, 64, &iTDesOutputLen, DES_ENCRYPT);
				
			print_hex("==============DATA_ENC OUTPUT", ucTDesOutput, iTDesOutputLen, __FUNCTION__, __LINE__);

			/* get mac */
			ucMacInput[0] = 0x84;
			ucMacInput[1] = 0xf0;
			ucMacInput[2] = 0x00;
			ucMacInput[3] = 0x01;
			ucMacInput[4] = 0x1c;

			memcpy(ucMacInput + 5, ucTDesOutput, iTDesOutputLen);
			
			print_hex("==============MAC INPUT", ucMacInput, 5 + iTDesOutputLen, __FUNCTION__, __LINE__);

			TDesEncryptMac(s_TKey, ucMacInput, 5 + iTDesOutputLen, ucMacOutput, &iMacOutputLen, gacChallenge);

			print_hex("==============MAC OUTPUT", ucMacOutput, iMacOutputLen, __FUNCTION__, __LINE__);

			/* cos command  */
			cmd[0] = CPU_CARD_COS;	
			cmd[1] = 0x84;				/* CLA: 0x84 */	
			cmd[2] = COS_WRITE_KEY;		/* INS: 0xF0 */
			cmd[3] = 0x00;				/* p1: 0x00 */
			cmd[4] = 0x01;				/* p2: Key/Pin ��¼������ */
			cmd[5] = 0x1C;				/* Lc: ���ݳ��� */

			memcpy(cmd + 6, ucTDesOutput, iTDesOutputLen);	
			memcpy(cmd + 6 + iTDesOutputLen, ucMacOutput, iMacOutputLen);

			iCmdLen = 6 + iTDesOutputLen + iMacOutputLen + 2;
		}
		/* д��Կ������Ϊ02��ά����Կ */
		else if (ucKeyIndex == 0x02)
		{
			/* get 3des */
			ucTDesInput[0] = 0x14;	/* ǰ�油0x14 */
			ucTDesInput[1] = 0x30;
			ucTDesInput[2] = 0x01;
			ucTDesInput[3] = 0x00;
			ucTDesInput[4] = 0xff;
			
			memcpy(ucTDesInput + 5, s_MF_MaintainKey, sizeof(s_MF_MaintainKey));
			
			print_hex("==============DATA_ENC INPUT", ucTDesInput, sizeof(s_MF_MaintainKey) + 5, __FUNCTION__, __LINE__);

			/* get 3des */
			TDesRun(ucTDesInput, 21  , s_MF_MKey, ucTDesOutput, 64, &iTDesOutputLen, DES_ENCRYPT);

			print_hex("==============DATA_ENC OUTPUT", ucTDesOutput, iTDesOutputLen, __FUNCTION__, __LINE__);

			/* get mac */
			ucMacInput[0] = 0x84;
			ucMacInput[1] = 0xf0;
			ucMacInput[2] = 0x00;
			ucMacInput[3] = 0x02;
			ucMacInput[4] = 0x1c;

			memcpy(ucMacInput + 5, ucTDesOutput, iTDesOutputLen);

			print_hex("==============MAC INPUT", ucMacInput, 5 + iTDesOutputLen, __FUNCTION__, __LINE__);

			TDesEncryptMac(s_MF_MKey, ucMacInput, 5 + iTDesOutputLen, ucMacOutput, &iMacOutputLen, gacChallenge);

			print_hex("==============MAC OUTPUT", ucMacOutput, iMacOutputLen, __FUNCTION__, __LINE__);

			/* cos command  */
			cmd[0] = CPU_CARD_COS;	
			cmd[1] = 0x84;				/* CLA: 0x84 */	
			cmd[2] = COS_WRITE_KEY;		/* INS: 0xF0 */
			cmd[3] = 0x00;				/* p1: 0x00 */
			cmd[4] = 0x02;				/* p2: Key/Pin ��¼������ */
			cmd[5] = 0x1c;				/* Lc: ���ݳ��� */

			/* add 3des encryt result */
			memcpy(cmd + 6, ucTDesOutput, iTDesOutputLen);	

			/* add mac encryt result */
			memcpy(cmd + 6 + iTDesOutputLen, ucMacOutput, iMacOutputLen);

			/* cmd total length */
			iCmdLen = 6 + iTDesOutputLen + iMacOutputLen + 2;
		}
		
		iret = CpuCardSetCmd(fd, cmd, iCmdLen);
	}
	
	return iret;
}

/*
	��֤������Կ, pkey is 16 bytes key
*/
int CosExternalAuth(int fd, unsigned char *pKey)
{
	unsigned char cmd[64];
	unsigned char ucTDesOutput[8];
	int iret = -1;

	/* get 8 bytes random number */
	if (CosGetChallenge(fd, 0x08) >= 0)
	{	
		memset(cmd, 0, sizeof(cmd));
		memset(ucTDesOutput, 0, sizeof(ucTDesOutput));

		/* 3des encryption, ucTDesOutput is the encrytion result */
		DesRun(gacChallenge, pKey, ucTDesOutput, DES_ENCRYPT);
		
		cmd[0] = CPU_CARD_COS;
		cmd[1] = 0x00;
		cmd[2] = COS_EXTERNAL_AUTH;
		cmd[3] = 0x00;
		cmd[4] = 0x00;
		cmd[5] = 0x08;
		
		memcpy(cmd + 6, ucTDesOutput, 8);

		iret = CpuCardSetCmd(fd, cmd, 16);
	}

	return iret;
}

/*
	д������͸���ļ�
	ucParaOne:	p1������дƫ��
	ucParaTwo:	p2������дƫ��
	pWriteBuff: д�����ݻ���
	ucWriteLen:	д�����ݳ���
*/
int CosWriteBinary(int fd, unsigned char ucParaOne, unsigned char ucParaTwo, 
                                    unsigned char *pWriteBuff, unsigned char ucWriteLen)
{
	int iRet = -1;
	unsigned char ucCmd[512];
	int iCmdLen = 0;
	
	memset(ucCmd, 0, sizeof(ucCmd));

	ucCmd[0] = CPU_CARD_COS;
	ucCmd[1] = 0x00;
	ucCmd[2] = COS_UPDATE_FILE;
	ucCmd[3] = ucParaOne;
	ucCmd[4] = ucParaTwo;
	ucCmd[5] = ucWriteLen;

	memcpy(ucCmd + 6, pWriteBuff, ucWriteLen);

	iCmdLen = 6 + ucWriteLen + 2;
	
	iRet = CpuCardSetCmd(fd, ucCmd, iCmdLen);
	
	return iRet;
}

/*
	cpu������(д)�������ļ���sFilesSize: �ļ���С
*/
int CosWriteBinaryFile(int fd, short sFilesSize)
{
	int iret = -1;
	int icount = 0;
	int i;
	unsigned char uparaone = 0;
	unsigned char uparatwo = 0;
	unsigned char uwritesize = 0;
	unsigned char ucwritebuf[COS_MAX_WRITE_SIZE + 1];
	short swriteoffset = 0;
	short sreadoffset = 0;

    /* check the total count */
	icount = sFilesSize / COS_MAX_WRITE_SIZE;
	if ((sFilesSize - (icount * COS_MAX_WRITE_SIZE)) > 0)
	{
		icount++;
	}
		
	for (i = 0; i < icount; i++)
	{
		if (i < icount - 1)
		{
			uwritesize = COS_MAX_WRITE_SIZE;
		}
		else
		{
			uwritesize = sFilesSize - (i * COS_MAX_WRITE_SIZE);
		}

		/* read and write offset */
		swriteoffset = sreadoffset = i * COS_MAX_WRITE_SIZE;
		uparaone = (swriteoffset >> 8) & 0x00ff;
		uparatwo = (swriteoffset) & 0x00ff;

		memset(ucwritebuf, 0, sizeof(ucwritebuf));
		memcpy(ucwritebuf, g_writeFeatureBuf + sreadoffset, uwritesize);

        /* write binary */
		iret = CosWriteBinary(fd, uparaone, uparatwo, ucwritebuf, uwritesize);

		/* error, break the write process */
		if (iret < 0)
		{
			break;
		}
		else
		{	
			/* g_writeOffset get the write offset for WirteCpuCard() function */
			g_writeOffset = (swriteoffset + uwritesize);	
		}
	}

	return iret;
}

/*
	�ô�����Կ����cos�ļ��ṹ��
	pTkey: ������Կ
*/
int CosEraseDF(int fd, unsigned char *pTkey, int ikeylen)
{
	int ret = -1;
	unsigned char cmd[64];
	int cmdlen = 0;

	/* get 8 bytes random number from cpu card */
	if (CosGetChallenge(fd, 0x08) == 0)
	{
		/* s_MF_MaintainKey is MF maintain key */
		ret = CosExternalAuth(fd, s_MF_MaintainKey);
		if (ret == 0)
		{
			/* get 4 bytes random number from cpu card */
			if (CosGetChallenge(fd, 0x04) == 0)
			{
				unsigned char macinput[64];
				unsigned char macoutput[4];
				int macinputlen = 0;
				int macoutputlen = 0;
                
				memset(macinput, 0, sizeof(macinput));
				memset(macoutput, 0, sizeof(macoutput));

				macinput[0] = 0x84;
				macinput[1] = COS_ERASE_DF;
				macinput[2] = 0x00;
				macinput[3] = 0x00;
				macinput[4] = 0x18;

				macinput[5] = 0xF0;
				macinput[6] = 0x02;
				macinput[7] = 0x91;
				macinput[8] = 0xFF;

				memcpy(macinput + 9, pTkey, ikeylen);
				macinputlen = 9 + ikeylen;

				/* get 4 bytes mac value */
				TDesEncryptMac(s_MF_MaintainKey, macinput, macinputlen, macoutput, &macoutputlen, gacChallenge);

				cmd[0] = CPU_CARD_COS;
				memcpy(cmd + 1, macinput, macinputlen);
				memcpy(cmd + 1 + macinputlen, macoutput, macoutputlen);

				/* total length of the cmd, should be add 1. */
				cmdlen = 1 + macinputlen + macoutputlen + 2;

				ret = CpuCardSetCmd(fd, cmd, cmdlen);
			}
		}
	}

	return ret;
}

int CosReadFile(int fd)
{
    int ret = -1;
    unsigned char  paramOne = 0;
    unsigned char  paramTwo = 0;
    unsigned char  fileMask[2] = {0};

    ret = CpuSearchCard(fd);
    if (2 == ret)
    {
        printf("is cpu card. \r\n");

        ret = CpuResetCard(fd);

        printf("CpuResetCard %d \r\n", ret);

        if (0 == ret)
        {
            paramOne = 0x00;
    		paramTwo = 0x00;
    		fileMask[0] = 0x3f;
    		fileMask[1] = 0x00;

            ret = CosSelectFile(fd, paramOne, paramTwo, fileMask, sizeof(fileMask));
    	    printf("SELECT MF: %d %s %d\r\n", ret, __FUNCTION__, __LINE__);	
        }

        if (0 == ret)
        {
            paramOne = 0x00;
    		paramTwo = 0x00;
    		fileMask[0] = 0x12;
    		fileMask[1] = 0x35;
    		ret = CosSelectFile(fd, paramOne, paramTwo, fileMask, sizeof(fileMask));
    		printf("SELECT EF: %d %s %d\r\n", ret, __FUNCTION__, __LINE__);	
        }

        /* read file */
    	if (0 == ret)
    	{
    		short sFileSize = COS_FEATURE_SIZE;

    		g_iOffset = 0;
    		g_readOffset = 0;
    		memset(g_readFeatureBuf, 0, sizeof(g_readFeatureBuf));
            ret = CosReadBinaryFile(fd, sFileSize);

            /* read card success */
    		if (0 == ret)
    		{							
    			printf("read card success. \r\n");
            }
    	}

        if (0 != ret)
        {
            printf("read card fail. \r\n");
        }
    }
    else
    {
        printf("card is not cpu card. \r\n");
    }

    return ret;   
}

int CosWriteFile(int fd)
{
    int ret = -1;
    int fileSize = 0;
    unsigned char ucFileMark[2];
	unsigned char ucFileLen[2];
	unsigned char uParaOne;
	unsigned char uParaTwo;
	unsigned char uKeyIndex;
    
    memset(g_writeFeatureBuf, 0, sizeof(g_writeFeatureBuf));
    fileSize = ReadFile(g_writeFeatureBuf, MAX_FILE_SIZE, (char *)WRITE_FILE_PATH);
    printf("fileSize %d \r\n", fileSize);

    if (fileSize == COS_FEATURE_SIZE)
    {
        /* ��λcpu�� */
		ret = CpuResetCard(fd);
		printf("reset card ret: %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
	
		/* ѡ��MF */
		if (0 == ret)
		{
			uParaOne = 0x00;
			uParaTwo = 0x00;
			ucFileMark[0] = 0x3f;
			ucFileMark[1] = 0x00;
			ret = CosSelectFile(fd, uParaOne, uParaTwo, ucFileMark, sizeof(ucFileMark));
            printf("select MF ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);

			/* ѡ��MF�ɹ���ret = 3ɾ��MF */
			if (0 == ret)
			{
				ret = 3;
			}
			/* ѡ��MFʧ��, ������һ�� */
			else
			{
				ret = 0;
			}
		}

		/* ѡ��MF�ɹ�������ɾ�����ļ��ṹ������д�� */
		if (3 == ret)
		{
			ret = CosEraseDF(fd, s_TKey, sizeof(s_TKey));
			printf("erase MF ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		}

        /* ��֤������Կ */
		if (0 == ret)
		{
			ret = CosExternalAuth(fd, s_TKey);
			printf("external auth ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		}
        
		/* ����MF */
		if (0 == ret)
		{	
			ucFileMark[0] = 0x3F;
			ucFileMark[1] = 0x00;
			ucFileLen[0] = 0x1c;
			ucFileLen[1] = 0xD0;
			ret = CosCreateFile(fd, ucFileMark, COS_MF_FILE, ucFileLen, ucMfName, sizeof(ucMfName), 1);
			printf("create MF ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		}

		/* ѡ��MF */
		if (0 == ret)
		{
			uParaOne = 0x04;		/* ��MF����ֱ��ѡ��MF */
			uParaTwo = 0x00;
			ret = CosSelectFile(fd, uParaOne, uParaTwo, ucMfName, sizeof(ucMfName));
			printf("select MF ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		}

		/* ��֤������Կ */
		if (0 == ret)
		{
			ret = CosExternalAuth(fd, s_TKey);
			printf("external auth ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		}

		/* ����MF��key�ļ� */
		if (0 == ret)
		{
			ucFileMark[0] = 0xff;  /* 0xff02 key�ļ���ʶ */
			ucFileMark[1] = 0x02;
			ret = CosCreateFile(fd, ucFileMark, COS_KEY_FILE, NULL, NULL, 0, 1);
			printf("create key ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		}

		/* д������Կ, ��Կ��0x01 */
		if (0 == ret)
		{
			uKeyIndex = 0x01; 		/* ��Կ������ 0x01 */
			ret = CosWriteKeyPin(fd, uKeyIndex);
			printf("write key1 ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		}

		/* дά����Կ����Կ��0x02 */
		if (0 == ret)
		{
			uKeyIndex = 0x02;
			ret = CosWriteKeyPin(fd, uKeyIndex);
			printf("write key2 ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		}

		/* ��MF�½���EF�ļ� */
		if (0 == ret)
		{	
			ucFileMark[0] = 0x12;	/* EF�ļ���ʶ */
			ucFileMark[1] = 0x35;
			ucFileLen[0] = (fileSize >> 8) & 0x00ff;	/* �ļ�����,��8λ */	
			ucFileLen[1] = fileSize & 0x00ff;			/* �ļ�����,��8λ */
			ret = CosCreateFile(fd, ucFileMark, COS_EF_FILE, ucFileLen, NULL, 0, 1);

			printf("create EF ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		}

		/* ѡ��MF��EF */
		if (0 == ret)
		{
			uParaOne = 0x00;
			uParaTwo = 0x00;
			ret = CosSelectFile(fd, uParaOne, uParaTwo, ucEfName, sizeof(ucEfName));

			printf("select EF ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		}

		/* дMF��EF�ļ� */
		if (0 == ret)
		{
			ret = CosWriteBinaryFile(fd, fileSize);
			printf("write EF ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
			
            if (0 == ret)
            {
				printf("write card success. %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
            }
		}

		/* д������ */
        if (ret != 0)
        {
           printf("write card fail. %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
        }
    }

    return ret;
}

