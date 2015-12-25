/*******************************************************************************
**  Copyright (c) 2014, Company
**  All rights reserved.
**	
**  文件说明: cpu 卡操作接口
**  创建日期: 2014.02.26
**
**  当前版本：1.0
**  作者：
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

/* transfer key 卡片传输密钥 */
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

/* Main key MF主控密钥 */
unsigned char s_MF_MKey[] = {
	0x46, 0x49, 0x52, 0x53,
	0x2D, 0x2D, 0x74, 0x65,
	0x63, 0x68, 0x6E, 0x6F,
	0x6C, 0x6F, 0x67, 0x79
};

/* Maintain key MF维护密钥 */
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

/* M1卡扇区号 */
unsigned int ICCardSectorNo = 4;

/* 随机数缓存 */
unsigned char gacChallenge[8] = {0};								
char g_readFeatureBuf[MAX_FILE_SIZE] = {0};			    /* 读卡缓存 */
unsigned char g_writeFeatureBuf[MAX_FILE_SIZE] = {0};			/* 写卡缓存 */
int g_iOffset = 0;										/* 存储读卡特征时，读卡缓存偏移变量 */
int g_iWriteCardRetrueCode = -2;						/* 写卡过程中，返回的结果码，根据该结果码，给出不同的错误提示 */
int g_writeOffset = 0;									/* 该变量在写卡时显示写卡进度条 */
int g_readOffset = 0;									/* 读卡进度条偏移显示 */

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
函 数 名 : print_hex
函数功能 : 以十六进制的形式打印缓冲区中的指定个数位 
返 回 值 : 
参数说明 :	char * tip		提示信息，ASCII串 以0x00结束
			unsigned char * buff	要打印内容的缓冲区指针
			unsigned char bytes		要打印的缓冲区的字节数
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
函数名称：CpuCardRead
函数功能: 读取cpu卡返指令返回结果
入口参数：
		: 
		: 
返回值  ：2  - CPU卡
		  1  - M1卡
		  0  - 指令执行成功
		  -1 - 2次cpu卡寻卡时间间隔未超过指定时间
		  -2 - cpu卡获取随机数指令执行失败
		  -3 - cpu卡外部认证指令执行失败
		  -4 - cpu卡创建文件指令执行失败
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
	unsigned char ucReadLen = 0;      /* 每次读取文件的长度 */
	
	memset(buf, 0, sizeof(buf));
	memset(tmp_buf, 0, sizeof(tmp_buf));

    tv.tv_sec=0;
	tv.tv_usec=200 * 1000;
	FD_ZERO(&readfds);
	FD_SET(hCard, &readfds);
    
	ret = select(hCard+1,&readfds,NULL,NULL,&tv);
	if(ret>0 && FD_ISSET(hCard, &readfds))
	{
		/* i不要小于5,否则会导致数据没读全 */
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
			/* 打印出tmp_buf中的内容 */
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

			/* 打印出tmp_buf中的内容 */
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

			/* 转义 */
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
                        /* 德生定制，四字节位置替换，并取低24个bit的数据 */
                        /*  
                         * 卡号由原来4字节物理卡号截取低3字节再取反变为直接用
                         * 4字节物理卡号取反
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
	
	/* 帧头 */
	buf[index++] = FRAME_HEAD;		/* 帧头 0x02 */

	/* 模块地址 */
	buf[index++] = 0x00;
	buf[index++] = 0x00;

	/* 长度字 */
	buf[index++] = (char)DataLen;

	/* 命令 + 数据 */
	for (j = 0; j < (DataLen - 2); j++)
	{
		buf[index++] = Cmd[j];
	}
	
	/* 校验字-从模块地址到数据域最后一个字节的逐字节累加 */
	for (j = 1; j < index; j++)
	{
		buf[index] += buf[j];
	}
	
	index++;
	buf[index] = FRAME_END;
	i = 0;
	j = 0;
	send_buf[i++] = buf[j];
	
	/* 如果数据包数据为0x02或0x03或0x10，则增加辨识字符0x10 */
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
		ret = CpuCardRead(hCard, Cmd[0]);	 /* Cmd[0]为指令命令 */
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
	复位cpu卡接口
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


/* 获取随机数 */
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

	/* 加密方式 */
	if (iMode == 1)
	{
		cmd[0] = CPU_CARD_COS;

		/* -----命令头, 计算mac时从cmd[1]开始计算长度-------------- */

		cmd[1] = 0x04;				//CLA
		cmd[2] = COS_CREATE_FILE;	//INS
		cmd[3] = 0x00;				//P1	 默认0x00

		if (uFileType == COS_MF_FILE) 		/* Create MF */
		{
			cmd[4] = uFileType;			//P2	 filetype
			cmd[5] = 0x1e;				//LC		
			memcpy(cmd + 6, pFileMark, 2);  // 3F00文件标识
			cmd[8] = uFileType;				// 38文件类型
			memcpy(cmd + 9, pFileLen, 2);	//文件长度
			cmd[11] = 0x5e;				// 01011110 文件属性 //0x42
			cmd[12] = 0x00;				// RFU
			cmd[13] = 0x00;				// RFU
			cmd[14] = 0x00;				// RFU
			cmd[15] = 0x00;				/* 封锁权限 */
			cmd[16] = 0x91;				/* 创建权限 */
			cmd[17] = ucNameLen;		/* 文件名长度 */
			memcpy(cmd + 18, pFileName, ucNameLen); 				/* 文件名 */

			/* 创建MF时计算MAC用传输密钥 */
			pucKey = s_TKey;

			/* MAC计算字符串长度 */
			iInputLen = 18 - 1 + ucNameLen;
		}
		else if (uFileType == COS_KEY_FILE) /* create key file */
		{
			cmd[4] = uFileType;			//P2	  filetype
			cmd[5] = 0x0F;				//LC
			
			cmd[6] = 0xFF;				// FF02 key/pin文件标识
			cmd[7] = 0x02;				//

			cmd[8] = uFileType;			// 0x08文件类型
			cmd[9] = 0x04;				// key个数
			cmd[10] = 0x14;				// key记录长度

			cmd[11] = 0xC0;				// 11000000 文件属性
			/* RFU */
			cmd[12] = 0x00;				// RFU
			cmd[13] = 0x00;				// RFU
			cmd[14] = 0x00;				// RFU

			/* 封锁权限 */
			cmd[15] = 0x01;
			/* 创建权限 */
			cmd[16] = 0x01;

			/* 创建MF下key文件时计算MAC用传输密钥 */
			pucKey = s_TKey;

			/* MAC计算字符串长度 */
			iInputLen = 16;
		}
		else if (uFileType == COS_EF_FILE)
		{
			cmd[4]  = uFileType;
			cmd[5]  = 0x0f;					/* 数据长度 */
			
			memcpy(cmd + 6, pFileMark, 2);	/* 2字节文件标识 */

			cmd[8]  = uFileType;			/* 文件类型 */

			memcpy(cmd + 9, pFileLen, 2);	/* 2字节文件长度 */	
			
			cmd[11] = 0x00;			/* 文件属性0x00,读写都不使用加密保护 */
			
			cmd[12] = 0x00;			/* FRU */
			cmd[13] = 0x00;
			cmd[14] = 0x00;
			
			cmd[15] = 0x00;			/* 读权限 */
			cmd[16] = 0x00;			/* 写权限 */
			
			/* 创建MF下EF文件时用MF主控密钥计算MAC */
			pucKey = s_MF_MKey;

			/* MAC计算字符串长度 */
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
	/* 非加密方式 */
	else
	{
		cmd[0] = CPU_CARD_COS;
		cmd[1] = 0x00;				//CLA
		cmd[2] = COS_CREATE_FILE;	//INS
		cmd[3] = 0x00;	
		
		if (uFileType == COS_EF_FILE)
		{
			cmd[4]  = uFileType;
			cmd[5]  = 0x0b;					/* 数据长度 */
			memcpy(cmd + 6, pFileMark, 2);	/* 2字节文件标识 */
			cmd[8]  = uFileType;			/* 文件类型 */
			memcpy(cmd + 9, pFileLen, 2);	/* 2字节文件长度 */			

			cmd[11] = 0x00;			/* 文件属性 */
			cmd[12] = 0x00;			/* FRU */
			cmd[13] = 0x00;
			cmd[14] = 0x00;
			cmd[15] = 0x00;			/* 读权限 */
			cmd[16] = 0x00;			/* 写权限 */
		}

		iCmdLen = 19;
	}

	iret = CpuCardSetCmd(fd, cmd, iCmdLen);
	
	return iret;
}

/*
	ucParaOne: p1参数
	ucParaTwo: p2参数
	ucReadSize: 每次读取文件大小
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
	cos选择文件，pFileName: 由ucType决定，ucNameLen: 文件名长度
	ucParamOne: p1参数 
	0x00: 用文件标识符选择MF、DF或EF(XX=文件标识符或为空)
	0x02: 用文件标识符在当前DF 下选择EF(XX=EF 的文件标识符)
	0x04: 用DF 文件名直接选择DF(XX=DF 的文件名)
	0x03: 选当前DF 的父DF（Data 为空）

	ucParamTwo: p2参数
	0x00: 第一个或唯一的文件实例
	0x20: 下一个文件实例
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
	
	ucCmd[3] = ucParamOne;  	/* p1-文件选择方式 */
	ucCmd[4] = ucParamTwo;		/* p2-选择第一个或唯一的文件实例 */
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
	
	unsigned char ucTDesInput[64];		/* 20 字节待加密数据 */
	unsigned char ucTDesOutput[64];		/* 24 字节3des输出，输入20字节，分3段，每段8字节做3des运算 */
	int iTDesOutputLen = 0;		/* TDes输出长度 */
	
	unsigned char ucMacInput[29];		/* mac 输入 */	
	unsigned char ucMacOutput[4];		/* mac 输出4字节 */
	int iMacOutputLen = 0;		/* mac 输出长度 */
	
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
			ucTDesInput[0] = 0x14;	/* 华虹cpu卡要求 首字节补0x14 */
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
			cmd[4] = 0x01;				/* p2: Key/Pin 记录索引号 */
			cmd[5] = 0x1C;				/* Lc: 数据长度 */

			memcpy(cmd + 6, ucTDesOutput, iTDesOutputLen);	
			memcpy(cmd + 6 + iTDesOutputLen, ucMacOutput, iMacOutputLen);

			iCmdLen = 6 + iTDesOutputLen + iMacOutputLen + 2;
		}
		/* 写密钥索引号为02的维护密钥 */
		else if (ucKeyIndex == 0x02)
		{
			/* get 3des */
			ucTDesInput[0] = 0x14;	/* 前面补0x14 */
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
			cmd[4] = 0x02;				/* p2: Key/Pin 记录索引号 */
			cmd[5] = 0x1c;				/* Lc: 数据长度 */

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
	认证传输密钥, pkey is 16 bytes key
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
	写二进制透明文件
	ucParaOne:	p1参数，写偏移
	ucParaTwo:	p2参数，写偏移
	pWriteBuff: 写入数据缓存
	ucWriteLen:	写入数据长度
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
	cpu卡更新(写)二进制文件，sFilesSize: 文件大小
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
	用传输密钥擦除cos文件结构，
	pTkey: 传输密钥
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
        /* 复位cpu卡 */
		ret = CpuResetCard(fd);
		printf("reset card ret: %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
	
		/* 选择MF */
		if (0 == ret)
		{
			uParaOne = 0x00;
			uParaTwo = 0x00;
			ucFileMark[0] = 0x3f;
			ucFileMark[1] = 0x00;
			ret = CosSelectFile(fd, uParaOne, uParaTwo, ucFileMark, sizeof(ucFileMark));
            printf("select MF ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);

			/* 选择MF成功，ret = 3删除MF */
			if (0 == ret)
			{
				ret = 3;
			}
			/* 选择MF失败, 跳到下一步 */
			else
			{
				ret = 0;
			}
		}

		/* 选择MF成功，则先删除卡文件结构，重新写卡 */
		if (3 == ret)
		{
			ret = CosEraseDF(fd, s_TKey, sizeof(s_TKey));
			printf("erase MF ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		}

        /* 认证传输密钥 */
		if (0 == ret)
		{
			ret = CosExternalAuth(fd, s_TKey);
			printf("external auth ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		}
        
		/* 创建MF */
		if (0 == ret)
		{	
			ucFileMark[0] = 0x3F;
			ucFileMark[1] = 0x00;
			ucFileLen[0] = 0x1c;
			ucFileLen[1] = 0xD0;
			ret = CosCreateFile(fd, ucFileMark, COS_MF_FILE, ucFileLen, ucMfName, sizeof(ucMfName), 1);
			printf("create MF ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		}

		/* 选择MF */
		if (0 == ret)
		{
			uParaOne = 0x04;		/* 用MF名称直接选择MF */
			uParaTwo = 0x00;
			ret = CosSelectFile(fd, uParaOne, uParaTwo, ucMfName, sizeof(ucMfName));
			printf("select MF ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		}

		/* 认证传输密钥 */
		if (0 == ret)
		{
			ret = CosExternalAuth(fd, s_TKey);
			printf("external auth ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		}

		/* 创建MF下key文件 */
		if (0 == ret)
		{
			ucFileMark[0] = 0xff;  /* 0xff02 key文件标识 */
			ucFileMark[1] = 0x02;
			ret = CosCreateFile(fd, ucFileMark, COS_KEY_FILE, NULL, NULL, 0, 1);
			printf("create key ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		}

		/* 写主控密钥, 密钥号0x01 */
		if (0 == ret)
		{
			uKeyIndex = 0x01; 		/* 密钥索引号 0x01 */
			ret = CosWriteKeyPin(fd, uKeyIndex);
			printf("write key1 ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		}

		/* 写维护密钥，密钥号0x02 */
		if (0 == ret)
		{
			uKeyIndex = 0x02;
			ret = CosWriteKeyPin(fd, uKeyIndex);
			printf("write key2 ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		}

		/* 在MF下建立EF文件 */
		if (0 == ret)
		{	
			ucFileMark[0] = 0x12;	/* EF文件标识 */
			ucFileMark[1] = 0x35;
			ucFileLen[0] = (fileSize >> 8) & 0x00ff;	/* 文件长度,高8位 */	
			ucFileLen[1] = fileSize & 0x00ff;			/* 文件长度,低8位 */
			ret = CosCreateFile(fd, ucFileMark, COS_EF_FILE, ucFileLen, NULL, 0, 1);

			printf("create EF ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		}

		/* 选择MF下EF */
		if (0 == ret)
		{
			uParaOne = 0x00;
			uParaTwo = 0x00;
			ret = CosSelectFile(fd, uParaOne, uParaTwo, ucEfName, sizeof(ucEfName));

			printf("select EF ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
		}

		/* 写MF下EF文件 */
		if (0 == ret)
		{
			ret = CosWriteBinaryFile(fd, fileSize);
			printf("write EF ret %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
			
            if (0 == ret)
            {
				printf("write card success. %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
            }
		}

		/* 写卡出错 */
        if (ret != 0)
        {
           printf("write card fail. %d %s %d\r\n", ret, __FUNCTION__, __LINE__);
        }
    }

    return ret;
}

