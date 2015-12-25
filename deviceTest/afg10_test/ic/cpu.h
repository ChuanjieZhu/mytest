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

#ifndef _CPU_H_
#define _CPU_H_

/* 圆志科信读头的命令 */
#define FRAME_HEAD           	0x02
#define FRAME_END           	0x03
#define SET_SERIAL_BAUD_RATE  	0x15
#define SEARCH_CARD           	0x46
#define ESCAPE_CONFLICT       	0x47
#define CHOOSE_CARD           	0x48
#define CHECK_SECRET_KEY     	0x4A
#define READ_BLOCK            	0x4B
#define WRITE_BLOCK           	0x4C

/* 圆志科信读头CPU卡命令 */
#define SET_TYPE_A_MODE 		0x3A	/* 设置读卡模块工作于ISO14443 TYPE A工作模式 */		
#define RESET_CPU_CARD			0x53	/* 复位卡指令 */
#define CPU_CARD_COS			0x54	/* 发送COS指令 */

#define COS_EXTERNAL_AUTH		0X82 	/* 保密模块验证 */
#define COS_GET_CHALLENGE		0x84	/* 获取随机数 */
#define COS_WRITE_KEY			0xF0	/* 创建、修改或重新激活密钥 */
#define COS_CREATE_FILE			0xE0	/* 华虹专有COS指令，创建文件 */
#define COS_SELECT_FILE			0xA4	/* COS指令，选择文件 */
#define COS_UPDATE_FILE			0xD6 	/* COS指令，写或修改文件 */
#define COS_READ_BINARY			0xB0	/* COS指令，读文件 */
#define COS_ERASE_DF			0x0E 	/* 删除DF/MF */

#define COS_MF_FILE				0x38
#define COS_KEY_FILE			0x08
#define COS_EF_FILE				0x01

#define COS_MAX_WRITE_SIZE		0x80	/* 每次写字节数 128字节 */
#define COS_MAX_READ_SIZE		0x28	/* 每次读字节数 40字节 */
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
