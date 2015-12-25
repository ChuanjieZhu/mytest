/********************************************************************************
**  Copyright (c) 2012, 深圳市飞瑞斯科技有限公司
**  All rights reserved.
**
**  文件名称： AudioControl.cpp
**  参考：
**
**  当前版本：1.0
**
**  创建作者： 
**  创建日期:  
**
**  修改作者： 张祖异在之前版本的基础上，调整版面，添加注释。，注释之前加有""
**  修改日期： 2012.5.21
***************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>
#include <error.h>
#include <getopt.h>
#include <linux/soundcard.h>

#include "2451_test.h"
#include "devLib.h"
#include "g711.h"
//#include "publicLib.h"
#include "Audio.h"
//#include "appLib.h"

/******* 讯飞语音 **********/
#ifdef PLAT_S3C2440 
//#include  "./2451/ivTTS.h"
#else
//#include  "./6441/ivTTS.h"
#endif

#include "eJTTS.h"

#define SOUND_DEVICE "/dev/dsp"
#define SOUND_MIXER	"/dev/mixer"

#define MAX_SOUND_BUF	8000*2*4
#define RESTART_TIMES   100

#ifdef PLAT_S3C2440
/*2440 语音播放*/
#define IO_CFG_AUDIO_EX      _IOWR('E', 0x3f, int)     
#define SOUND_OUTPUT_CLOSE MIXER_WRITE(14)
#define SOUND_OUTPUT_OPEN MIXER_WRITE(15)
#endif

#ifndef PACK_ALIGN
#define PACK_ALIGN	__attribute__((packed))
#endif

int hMapFile = -1;	/*  播放的文件时打开的文件的描述符，并用map */

extern pthread_attr_t gtAttr;
class CAudio* g_pAudio = NULL;

typedef struct WAV_RIFF
{
	// must be "RIFF"
	char riff_label[4];
	// riff_len+8 is size of WAVE file
	unsigned long riff_len;
	// must be "WAVE"
	char type[4];
} PACK_ALIGN WAV_RIFF, *LPWAV_RIFF;

typedef struct WAV_FMT
{
	// must be "fmt "
	char fmt_label[4];
	// fmt_len+8 is size of FORMAT chunk
	unsigned long fmt_len;
	unsigned short format;
	unsigned short channels;
	unsigned long samplerate;
	unsigned long bytes_per_sec;
	unsigned short bytes_align;
	unsigned short bits_per_sample;
} PACK_ALIGN WAV_FMT, *LPWAV_FMT;

typedef struct WAV_FACT
{
	// must be "fact"
	char fact_label[4]; 
	// fact_len+8 si sizeof FACT chunk
	unsigned long fact_len; 
} PACK_ALIGN WAV_FACT, *LPWAV_FACT;

typedef struct WAV_DATA
{
	// DATA chunk
	char data_label[4];
	unsigned long data_len;
} PACK_ALIGN WAV_DATA, *LPWAV_DATA;

typedef struct WAVEHEADER
{
	// RIFF chunk
	WAV_RIFF riff;
	// FORMAT chunk
	WAV_FMT fmt;
	// DATA chunk
	WAV_DATA data;
} PACK_ALIGN WAVEHEADER, *LPWAVEHEADER;

#undef PACK_ALIGN

#define ivTTS_HEAP_SIZE		70000 /* 混合，音效 */

unsigned long		hTTS;				// 引擎句柄
unsigned char *		pHeap;				// 堆空间地址	
#if 0
ivHTTS			hTTS;
ivPByte			pHeap;
ivTResPackDescExt	tResPackDesc;

/* Message */
ivTTSErrID DoMessage();

/* output callback */
ivTTSErrID OnOutput(
		ivUInt16		nCode,				/* [in] output data code */
		ivCPointer		pcData,				/* [in] output data buffer */
		ivSize			nSize );			/* [in] output data size */

/* read resource callback */
ivBool ivCall ReadResCB(
		ivPointer		pParameter,			/* [in] user callback parameter */
		ivPointer		pBuffer,			/* [out] read resource buffer */
		ivResAddress	iPos,				/* [in] read start position */
		ivResSize		nSize );			/* [in] read size */

/* output callback */
ivTTSErrID ivCall OutputCB(
		ivPointer		pParameter,		/* [in] user callback parameter */
		ivUInt16		nCode,			/* [in] output data code */
		ivCPointer		pcData,			/* [in] output data buffer */
		ivSize			nSize );		/* [in] output data size */

/* parameter change callback */
ivTTSErrID ivCall ParamChangeCB(
		ivPointer       pParameter,		/* [in] user callback parameter */
		ivUInt32		nParamID,		/* [in] parameter id */
		ivUInt32		nParamValue );	/* [in] parameter value */

/* progress callback */
ivTTSErrID ivCall ProgressCB(
		ivPointer       pParameter,		/* [in] user callback parameter */
		ivUInt32		iProcPos,		/* [in] current processing position */
		ivUInt32		nProcLen );		/* [in] current processing length */
#endif

void TtsFree();
/* 讯飞语音接口 */
int xfMakePlayVoice(const char *voiceText);
/* 讯飞语音所需变量初始化 */
int xfVoiceInit();

int jtMakePlayVoice(const char *voiceText);

int jtVoiceInit();

int jtVoiceDeInit();

/**************************************/

/**************************************************************\
** 函数名称： Rate8KMono_To_Rate32KStereo
** 功能： 8K转32K
** 参数： 
** 返回： 无
** 创建作者： 
** 创建日期： 
** 修改作者： 张祖异在之前版本的基础上，调整版面，添加注释，注释之前加有""
** 修改日期： 2012.5.21
***************************************************************/
void Rate8KMono_To_Rate32KStereo(unsigned short* pData, int nSamples)
{
	int i = 0;
	int j = 0;
	unsigned short *pSrc = pData;
	unsigned short *pDst = pSrc;

	pSrc += nSamples - 1;
	pDst += (nSamples - 1) * 8;
	
	for(i = 0; i < nSamples; i++)
	{
		unsigned short v = *pSrc;
		
		for(j = 0; j < 8; j++) 
		{
			*pDst++ = v;
		}
		
		pSrc--;
		pDst -= 8 * 2;
	}
}

void AudioInit()
{
   if(NULL == g_pAudio)
   {
   		g_pAudio = new class CAudio;	
   }
}

void AudioUninit()
{	
	if(g_pAudio)
	{
		delete g_pAudio;
		g_pAudio = NULL;
	}
}

void AudioSetVolume(int nVolume)
{
	if(g_pAudio)
	{
		g_pAudio->SetVolume(nVolume);
	}
}

int AudioGetVolume()
{	
	int rtn = 0;
	
	if(g_pAudio)
	{
		rtn = g_pAudio->GetVolume();
	}

	return rtn;
}

void AudioSetMicVolume(int nVolume)
{
	if(g_pAudio)
	{
		g_pAudio->SetMicVolume(nVolume);
	}
}

int AudioGetMicVolume()
{
	int rtn = 0;
	
	if(g_pAudio)
	{
		rtn = g_pAudio->GetMicVolume();
	}

	return rtn;
}

void AudioCloseSound(int bClose)
{
	if(g_pAudio)
	{
		g_pAudio->CloseSound(bClose);
	}
}

void AudioMute(int bMute)
{
	if(g_pAudio)
	{
		g_pAudio->Mute(bMute);
	}
}

void AudioPlay(char* pAudioData, int nDataLen, int bAppend)
{
	if(g_pAudio)
	{
		g_pAudio->Play(pAudioData,nDataLen,bAppend);
	}
}

void AudioPlayFile(char* pFile, int bAppend)
{
	if(g_pAudio)
	{
		g_pAudio->PlayFile(pFile,bAppend);
	}
}

void AudioStopVoice()
{
	if(g_pAudio)
	{
		g_pAudio->StopVoice();
	}
}

void AudioRestartDsp()
{
	if(g_pAudio)
	{
		g_pAudio->RestartDsp();
	}
}

/**************************************************************\
** 函数名称： CAudio
** 功能： 构造函数
** 参数： 
** 返回： 无
** 创建作者： 
** 创建日期： 
** 修改作者： 张祖异在之前版本的基础上，调整版面，添加注释，注释之前加有""
** 修改日期： 2012.5.21
***************************************************************/
CAudio::CAudio()
{
	int val;
	int ret=-1;

	m_hAudio = -1;
	m_hMixer = -1;

	m_bFormatG711 = 0;
	m_bClose = 1;
	m_bMute = 0;
	m_bExit = 1;
	m_bMuteAlarm = 0;

	m_pMapFileName = NULL;
	m_pMapBuf = NULL;
	m_nMapLen = 0;

	m_nDataLen = 0;
	m_nDataBufLen = 8000*2*1*5; /* 5 seconds */	/*  设置播放缓存的大小 */
	m_pDataBuf = NULL;

	/*  打开音量设备 */
	m_hMixer = open( SOUND_MIXER, O_RDWR );	
	if( m_hMixer == -1 )
	{
		TRACE("Failed to open /dev/mixer\r\n");
	}
	else
	{
		Mute(1);
	}

	SetVolume(0);
	
	/*  打开音频设备 */
	m_hAudio = open(SOUND_DEVICE, O_WRONLY);	
	if( m_hAudio == -1 )
	{
		TRACE("Open sound device %s Failed\r\n", SOUND_DEVICE);
		if(m_hMixer!=-1)
			close(m_hMixer);
		m_hMixer = -1;
	}
	else
	{
		ioctl(m_hAudio, SNDCTL_DSP_SETDUPLEX, NULL);

		/* set 16 bits */
		val = AFMT_S16_LE;
		if( ioctl(m_hAudio, SNDCTL_DSP_SETFMT, &val) == -1 )
		{
			TRACE("Set AFMT_S16_LE Failed\r\n");
		}

		/* set samplerate 8000 or 32000 */
		val = 32000;
		if( ioctl(m_hAudio, SNDCTL_DSP_SPEED, &val) == -1 )
		{
			TRACE("Set SPEED %d Failed\r\n", val);
		}

		/* force set channel to STEREO */
		val = 1;
		if( ioctl(m_hAudio, SNDCTL_DSP_STEREO, &val) == -1 )
		{
			TRACE("Set %s Failed\r\n", val?"STEREO":"MONO");
		}

		m_bClose = 0;
		m_bExit = 0;
	}

	m_nVoiceIcPlayNo = -1;

	/*  申请播放缓存 */
	if(m_bExit == 0)
	{
		m_pDataBuf = NULL;
		m_pDataBuf = (char*)Malloc(m_nDataBufLen);
		if(m_pDataBuf == NULL)
		{
			TRACE("Set %s Failed\r\n", val?"STEREO":"MONO");
		}
	}

	pthread_mutex_init(&m_mutex, NULL);
    pthread_mutex_init(&m_DspMutex, NULL);

	m_thread = 0;

	/* lph 讯飞语音初始化参数 */
	//ret=xfVoiceInit();
    ret=jtVoiceInit();
    if(ret == -1)
	{
		return;
	}

    //pthread_attr_t *pAttr = (pthread_attr_t *)GetThreadAttr();
        
	if(m_bExit==0)
		pthread_create(&m_thread, &gtAttr, process, this);		//创建播放线程
}

/**************************************************************\
** 函数名称： CAudio
** 功能： 卸构函数
** 参数： 
** 返回： 无
** 创建作者： 
** 创建日期： 
** 修改作者： 张祖异在之前版本的基础上，调整版面，添加注释，注释之前加有""
** 修改日期： 2012.5.21
***************************************************************/
CAudio::~CAudio()
{
	if(m_bExit==0)
	{
		m_bExit = 1;
		sleep(1);
		if(m_thread)
		{
			usleep(100*1000);
			pthread_join(m_thread, NULL);		/*  等待process线程退出。 */
		}
	}

	pthread_mutex_destroy(&m_mutex);	/*  销毁互斥锁 */
    pthread_mutex_destroy(&m_DspMutex);	/*  销毁互斥锁 */

	Mute(1);	/*  设置音量为0 */

	/*  释放播放的缓存 */
	if(m_pDataBuf != NULL)
	{
		Free(m_pDataBuf);
		m_pDataBuf = NULL;
	}

	/*  释放播放文件的缓存 */
	if(m_pMapBuf != NULL)
	{
		munmap(m_pMapBuf, m_nMapLen);
		m_pMapBuf = NULL;
	}
	
	/*  关闭音频 */
	if(m_hMixer != -1)
	{
		close(m_hMixer);
		m_hMixer = -1;
	}

	/*  用于播放的声音 */
	if(m_hAudio != -1)
	{
		close(m_hAudio);
		m_hAudio = -1;
	}

	/* 讯飞语音释放 */
	//TtsFree();
	jtVoiceDeInit();
}

/**************************************************************\
** 函数名称： SetVolume
** 功能： 设置音量
** 参数： 
** 返回： 无
** 创建作者： 
** 创建日期： 
** 修改作者： 张祖异在之前版本的基础上，调整版面，添加注释，注释之前加有""
** 修改日期： 2012.5.21
***************************************************************/
void CAudio::SetVolume(int nVolume)
{
	if(m_bExit==0)
	{
		int bModifyVolume = (nVolume >= 0);

		/*  如果小于等于0，那就是设置音量为0 */
		if(nVolume<=0)
		{
			nVolume = 0;
		}
		/*  如果小于等于0，那就是设置音量为0 */
		else if(nVolume>100)
		{
			nVolume = 100;
		}
		
		/*  如果大于0，那就是设置音量 */
		if(bModifyVolume)
		{
			m_nVolume = nVolume;
		}

		/*  设置音量 */
		if(m_hMixer!=-1)
		{
			nVolume &= 0xFF;
			nVolume = nVolume | (nVolume << 8);

			if( ioctl(m_hMixer, SOUND_MIXER_WRITE_VOLUME, &nVolume) == -1 )
			{
				TRACE("Failed to set the volume.\r\n");
			}
		}
	}
}

/**************************************************************\
** 函数名称： GetVolume
** 功能： 获得音量的大小
** 参数： 
** 返回： 无
** 创建作者： 
** 创建日期： 
** 修改作者： 张祖异在之前版本的基础上，调整版面，添加注释，注释之前加有""
** 修改日期： 2012.5.21
***************************************************************/
int CAudio::GetVolume()
{
	int nVolume = -1;

	if(m_bExit==0)
	{
		if(m_hMixer!=-1)
		{
			/* added by ytj 20110613，读取音量应该用SOUND_MIXER_READ_VOLUME参数 */
			//if( ioctl(m_hMixer, SOUND_MIXER_WRITE_VOLUME, &nVolume) == -1 )
			if( ioctl(m_hMixer, SOUND_MIXER_READ_VOLUME, &nVolume) == -1 )
			{
				TRACE("Failed to get the volume.\r\n");
				nVolume = -1;
			}
			else
			{
				nVolume &= 0xFF;
			}
		}

		m_nVolume = nVolume;
	}

	return nVolume;
}

/**************************************************************\
** 函数名称： SetMicVolume
** 功能： 设置增益
** 参数： 
** 返回： 无
** 创建作者： 
** 创建日期： 
** 修改作者： 张祖异在之前版本的基础上，调整版面，添加注释，注释之前加有""
** 修改日期： 2012.5.21
***************************************************************/
void CAudio::SetMicVolume(int nVolume)
{
	if(m_bExit==0)
	{
		if(m_hMixer!=-1)
		{
			nVolume &= 0xFF;
			nVolume = nVolume | (nVolume<<8);
			if( ioctl(m_hMixer, SOUND_MIXER_WRITE_IGAIN, &nVolume) == -1 )
				TRACE("Failed to set the MIC volume.\r\n");
		}
	}
}

/**************************************************************\
** 函数名称： GetMicVolume
** 功能： 获得增益
** 参数： 
** 返回： 无
** 创建作者： 
** 创建日期： 
** 修改作者： 张祖异在之前版本的基础上，调整版面，添加注释，注释之前加有""
** 修改日期： 2012.5.21
***************************************************************/
int CAudio::GetMicVolume()
{
	int nVolume = -1;
	if(m_bExit==0)
	{
		if(m_hMixer!=-1)
		{
			if( ioctl(m_hMixer, SOUND_MIXER_READ_IGAIN, &nVolume) == -1 )
			{
				TRACE("Failed to get the MIC volume.\r\n");
				nVolume = -1;
			}
			else
				nVolume &= 0xFF;
		}
		m_nVolume = nVolume;
	}
	return nVolume;
}

/**************************************************************\
** 函数名称： CloseSound
** 功能： 关闭声音
** 参数： 如果bClose大于或为真就是关闭声音
** 返回： 无
** 创建作者： 
** 创建日期： 
** 修改作者： 张祖异在之前版本的基础上，调整版面，添加注释，注释之前加有""
** 修改日期： 2012.5.21
***************************************************************/
void CAudio::CloseSound(int bClose)
{
	if(m_hMixer!=-1)
	{
		m_bClose = bClose;
	}
}

/**************************************************************\
** 函数名称： Mute
** 功能： 关闭混音器
** 参数： 如果bMute大于或为真就是关闭混音器
** 返回： 无
** 创建作者： 
** 创建日期： 
** 修改作者： 张祖异在之前版本的基础上，调整版面，添加注释，注释之前加有""
** 修改日期： 2012.5.21
***************************************************************/
void CAudio::Mute(int bMute)
{
	return;
	if(bMute)
		SetVolume(-1);
	else
		SetVolume(m_nVolume);
}

/**************************************************************\
** 函数名称： Play
** 功能： 关闭混音器
** 参数： pAudioData-播放的数据, nDataLen-播放的数据长度, bAppend--附加，如果为0，那么m_nDataLen就为0
** 返回： 无
** 创建作者： 
** 创建日期： 
** 修改作者： 张祖异在之前版本的基础上，调整版面，添加注释，注释之前加有""
** 修改日期： 2012.5.21
***************************************************************/
void CAudio::Play(char* pAudioData, int nDataLen, int bAppend)
{
	pthread_mutex_lock(&m_mutex);
	if(bAppend == 0)
		m_nDataLen = 0;
	if(m_bClose==0 && m_nVolume>0 && m_pDataBuf && pAudioData && nDataLen>0)
	{
		while(nDataLen)
		{
			int nLen = nDataLen;
			if(m_nDataLen+nLen > m_nDataBufLen)	/*  如果现在待播放的长度+要播放的长度 大于 申请的缓冲最大长度，那么就取缓冲还有空间剩余的长度 */
				nLen = m_nDataBufLen-m_nDataLen;

			memcpy(m_pDataBuf+m_nDataLen, pAudioData, nLen);
			m_nDataLen += nLen;
			nDataLen -= nLen;
			/*  sleep等待正在播放的数据 */
			if(nDataLen)
			{
                nDataLen = 0;
#if 1                
				pthread_mutex_unlock(&m_mutex);
				sleep(1);
				pthread_mutex_lock(&m_mutex);
#endif                
			}
		}
		Mute(0);
	}
	pthread_mutex_unlock(&m_mutex);
}

/**************************************************************\
** 函数名称： PlayFile
** 功能： 播放文件
** 参数： pFile-播放的文件, bAppend--附加，如果为0证明之前有打开，应该close和munmap后才用，否则有内存泄露
** 返回： 无
** 创建作者： 
** 创建日期： 
** 修改作者： 张祖异 在之前版本的基础上，调整版面，添加注释，注释之前加有""
** 修改日期： 2012.5.21
***************************************************************/
void CAudio::PlayFile(char* pFile, int bAppend)
{
	pthread_mutex_lock(&m_mutex);

	/*  如果为0证明之前有打开，应该close和munmap后才用，否则有内存泄露 */
	if(bAppend == 0)
	{
		if(m_pMapFileName)
		{
			if(hMapFile != -1)
			{
				close(hMapFile);
				hMapFile = -1;
			}
			m_pMapFileName = NULL;
		}

		if(m_pMapBuf)
		{

			munmap(m_pMapBuf, m_nMapLen);
			m_pMapBuf = NULL;
		}
	}

	if(m_bClose==0 && m_nVolume>0 && pFile)
	{
		while(1)
		{
			if(m_pMapFileName==NULL)
			{
				static char filename[255+1];
				strncpy(filename, pFile, 255);
				filename[255] = '\0';
				m_pMapFileName = filename;

				break;
			}

			pthread_mutex_unlock(&m_mutex);
			usleep(100*1000);
			pthread_mutex_lock(&m_mutex);
		}

		Mute(0);
	}

	pthread_mutex_unlock(&m_mutex);
}

/**************************************************************\
** 函数名称： StopVoice
** 功能： 停止播放
** 参数： 无
** 返回： 无
** 创建作者： 
** 创建日期： 
** 修改作者： 张祖异 在之前版本的基础上，调整版面，添加注释，注释之前加有""
** 修改日期： 2012.5.21
***************************************************************/
void CAudio::StopVoice()
{
	pthread_mutex_lock(&m_mutex);

	if(m_nDataLen)
		m_nDataLen = 0;

	/*  卸载 */
	if(m_pMapBuf && m_nMapLen)
	{
		munmap(m_pMapBuf, m_nMapLen);
		m_nMapLen = 0;
	}

	if(m_pMapFileName)
		m_pMapFileName = NULL;

	pthread_mutex_unlock(&m_mutex);
}

/**************************************************************\
** 函数名称： RestartDsp
** 功能： 停止播放
** 参数： 无
** 返回： 无
** 创建作者： 
** 创建日期： 
** 修改作者： 在之前版本的基础上，调整版面，添加注释，注释之前加有""
** 修改日期： 2012.5.21
***************************************************************/
void CAudio::RestartDsp()
{
    int val = 0;

    TRACE("RestartDsp() %s %d\r\n", __FUNCTION__, __LINE__);
    
	pthread_mutex_lock(&m_DspMutex);

    if( m_hAudio != -1 )
    {
        close(m_hAudio);
    }

    m_hAudio = open(SOUND_DEVICE, O_WRONLY);	
	if( m_hAudio == -1 )
	{
		TRACE("Open sound device %s Failed\r\n", SOUND_DEVICE);
		if(m_hMixer!=-1)
			close(m_hMixer);
		m_hMixer = -1;
	}
	else
	{
        if (ioctl(m_hAudio,SNDCTL_DSP_RESET,(char *)&val) != 0) 
        { 
            printf("ioctl SNDCTL_DSP_RESET %s %d\r\n", __FUNCTION__, __LINE__); 
        }
        
		ioctl(m_hAudio, SNDCTL_DSP_SETDUPLEX, NULL);

		/* set 16 bits */
		val = AFMT_S16_LE;
		if( ioctl(m_hAudio, SNDCTL_DSP_SETFMT, &val) == -1 )
		{
			TRACE("Set AFMT_S16_LE Failed\r\n");
		}

		/* set samplerate 8000 or 32000 */
		val = 32000;
		if( ioctl(m_hAudio, SNDCTL_DSP_SPEED, &val) == -1 )
		{
			TRACE("Set SPEED %d Failed\r\n", val);
		}

		/* force set channel to STEREO */
		val = 1;
		if( ioctl(m_hAudio, SNDCTL_DSP_STEREO, &val) == -1 )
		{
			TRACE("Set %s Failed\r\n", val?"STEREO":"MONO");
		}

		m_bClose = 0;
		m_bExit = 0;
	}

	pthread_mutex_unlock(&m_DspMutex);
}


/**************************************************************\
** 函数名称： process
** 功能： 音频处理
** 参数： 无
** 返回： 无
** 创建作者： 
** 创建日期： 
** 修改作者： 张祖异 在之前版本的基础上，调整版面，添加注释，注释之前加有""
** 修改日期： 2012.5.21
***************************************************************/
void* CAudio::process(void* pArg)
{
	TRACE("*** entry thread %s %d\r\n", __FUNCTION__, __LINE__);

    int count = 0;
    
	CAudio* pThis = (CAudio*)pArg;
	if(pThis)
	{
		static char databuf[MAX_SOUND_BUF]; /* 0.5 seconds data buffer of 8K Mono audio to 32K Stereo*/
		int hAudio = pThis->m_hAudio;
		int nMapPlayOffset=0, nNeedMute = 0, nMaxNeedMute = 10, bLock = 0;

		/* 在系统启动首次发音之前，要先向声音设备中写入64k空白数据，否则首次发音会失败 */
		memset(databuf, 0, MAX_SOUND_BUF);

	    pthread_mutex_lock(&pThis->m_DspMutex);
        
		write(pThis->m_hAudio, databuf, MAX_SOUND_BUF);

        pthread_mutex_unlock(&pThis->m_DspMutex);

		/*  nMapPlayOffset-播放时map文件的便宜地址； */
		while(pThis->m_bExit==0)
		{
			bLock = 1;
			pthread_mutex_lock(&pThis->m_mutex);
#if 1
			/*  播放的是数据 */
			if(pThis->m_nDataLen || pThis->m_pMapBuf)
			{
				int nLen = 0;
				
				/*  播放的是数据 */
				if(pThis->m_nDataLen)
				{
					nLen = pThis->m_nDataLen;
					/*  如果播放的数据长度大于每次的最大播放块（databuf），那么就把nLen置为最大播放块 */
					if( nLen > (int)(sizeof(databuf)/8))
					{
						nLen = (int)sizeof(databuf)/8;
					}

					memcpy(databuf, pThis->m_pDataBuf, nLen);	/*  拷贝播放的数据 */
					pThis->m_nDataLen -= nLen;	/*  还剩待播放的数据 */

					/*  把后面的数据移到前面，准备播放。 */
					if(pThis->m_nDataLen)
					{
						memmove(pThis->m_pDataBuf, pThis->m_pDataBuf+nLen, pThis->m_nDataLen);
					}
				}
				/*  播放的是数据，并在mmap里加载了的 */
				else if(pThis->m_pMapBuf)
				{
					/*  没有取完所有播放的数据 */
					if(nMapPlayOffset < pThis->m_nMapLen)
					{
						nLen = pThis->m_nMapLen-nMapPlayOffset;

						/*  如果是G711的格式，那么就解码 */
						if(pThis->m_bFormatG711)
						{
							/*  如果播放的数据长度大于每次的最大播放块（databuf），那么就把nLen置为最大播放块 */
							if( nLen > (int)(sizeof(databuf)/8/2))
							{
								nLen = (int)sizeof(databuf)/8/2;
							}
							/*  解码，把直接解码的数据存入databuf */
							G711ADec((unsigned char*)pThis->m_pMapBuf+nMapPlayOffset, (short*)databuf, nLen);

							nMapPlayOffset+=nLen;
							nLen*=2;
						}
						else
						{
							/*  如果播放的数据长度大于每次的最大播放块（databuf），那么就把nLen置为最大播放块 */
							if( nLen > (int)(sizeof(databuf)/8) )
							{
								nLen = (int)sizeof(databuf)/8;
							}

							memcpy(databuf, pThis->m_pMapBuf+nMapPlayOffset, nLen);	/*  取待播放的数据存入databuf */
							nMapPlayOffset+=nLen;
						}
					}
					/*  如果取完了，就munmap，并close */
					else
					{
						if(pThis->m_nMapLen)
						{
							munmap(pThis->m_pMapBuf, pThis->m_nMapLen);
						}

						pThis->m_pMapBuf = NULL;
						close(hMapFile);
						hMapFile = -1;
					}
				}

				pthread_mutex_unlock(&pThis->m_mutex);
				bLock = 0;

				/*  声音播放时采样率转换 */
				if(nLen>0)
				{
					Rate8KMono_To_Rate32KStereo((unsigned short*)databuf, nLen/sizeof(short));

					nLen*=8;

                    pthread_mutex_lock(&pThis->m_DspMutex);
                    
					nLen = write(pThis->m_hAudio, databuf, nLen);/*  播放 */

                    pthread_mutex_unlock(&pThis->m_DspMutex);

                    count++;
                    if (count >= 50)
                    {
                        pThis->RestartDsp();
                        count = 0;    
                    }
                    
                    if(nLen<=0)
					{
						TRACE("Play audio Failed\r\n");
						pThis->m_bExit = 1;
					}

					nNeedMute = nMaxNeedMute;
				}
				/*  如果取完了，就munmap，并close */
				if(pThis->m_pMapBuf==NULL)
				{
					if(hMapFile!=-1)
					{
						close(hMapFile);
						hMapFile = -1;
					}
				}
			}
			else if(pThis->m_pMapFileName)	/*  播放的是文件 */
			{
				/* modified by ytj 2011011，对于E5或A5机型，由于welcome_en.g711及welcome_hk.g711文件比较小，
				   在系统启动首次发音之前，要先向声音设备中写入64k空白数据，否则首次发音会失败 */
				if (/*(IS_E5() || IS_A5())
					&& */((strcmp(pThis->m_pMapFileName, "/usr/local/lib/welcome_en.g711") == 0)
					|| (strcmp(pThis->m_pMapFileName, "/usr/local/lib/welcome_hk.g711") == 0)))
				{
					memset(databuf, 0, MAX_SOUND_BUF);
                    
                    pthread_mutex_lock(&pThis->m_DspMutex);
					write(pThis->m_hAudio, databuf, MAX_SOUND_BUF);
                    pthread_mutex_unlock(&pThis->m_DspMutex);
                }

				/*  加载要播放的文件 */
				hMapFile = open(pThis->m_pMapFileName, O_RDONLY);
				if(hMapFile==-1)
				{
					TRACE("PlayFile: open file %s fail.\r\n", pThis->m_pMapFileName);
				}
				else
				{
					struct stat st;
					fstat(hMapFile, &st);
					
					/*  根据文件的大小来mmap数据 */
					if(st.st_size>0)
					{
						pThis->m_pMapBuf = (char*)mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, hMapFile, 0);
						if(pThis->m_pMapBuf)
						{
							pThis->m_nMapLen = st.st_size;
							nMapPlayOffset = 0;

							if(pThis->m_pMapBuf)
							{
								/* 解析WAV 或G711 文件头*/
								char* pBuf = pThis->m_pMapBuf;
								LPWAV_RIFF pRiff = (LPWAV_RIFF)pBuf;
								LPWAV_FMT pFmt = (LPWAV_FMT)(pBuf+sizeof(WAV_RIFF));
								
								/*  数据文件的播放格式头不一样。 */
								if(strncmp(pRiff->riff_label, "RIFF", 4)
									|| (8 + pRiff->riff_len) < (unsigned long)st.st_size
									|| (!((strncmp(pRiff->type, "WAVE", 4)==0
									&& strncmp(pFmt->fmt_label, "fmt ", 4)==0)
									|| strncmp(pRiff->type, "G711", 4)==0)) )
								{
									nMapPlayOffset = pThis->m_nMapLen;
								}
								else if(strncmp(pRiff->type, "WAVE", 4)==0)/*  WAVE格式的文件 */
								{
									// WAV file /*  播放文件格式检查 */
									if(pFmt->format != 1
										|| pFmt->channels != 1
										|| pFmt->samplerate != 8000
										|| pFmt->bits_per_sample != 16)
									{
										nMapPlayOffset = pThis->m_nMapLen;
									}
									else
									{
										LPWAV_FACT pFact;
										LPWAV_DATA pData;
										pBuf+=sizeof(WAV_RIFF)+8+pFmt->fmt_len;
										pFact = (LPWAV_FACT)pBuf;

										if(strncmp(pFact->fact_label, "fact", 4)==0)
										{
											pBuf+=8+pFact->fact_len;
										}

										pData = (LPWAV_DATA)pBuf;

										if(strncmp(pData->data_label, "data", 4)==0)
										{
											pBuf += 8;
											nMapPlayOffset = pBuf - pThis->m_pMapBuf;

											if(( nMapPlayOffset + pData->data_len ) > (unsigned long)st.st_size )
											{
												nMapPlayOffset = pThis->m_nMapLen;
											}
										}
										else
										{
											nMapPlayOffset = pThis->m_nMapLen;
										}
									}

									pThis->m_bFormatG711 = 0;
								}
								else
								{
									// G.711 file
									pBuf+=sizeof(WAV_RIFF);
									nMapPlayOffset = pBuf - pThis->m_pMapBuf;

									if(( nMapPlayOffset + pRiff->riff_len - 4) > (unsigned long)st.st_size )
									{
										nMapPlayOffset = pThis->m_nMapLen;
									}

									pThis->m_bFormatG711 = 1;
								}
							}
						}
					}

					if(pThis->m_pMapBuf==NULL)
					{
						close(hMapFile);
						hMapFile = -1;
					}
				}

				pThis->m_pMapFileName = NULL;
			}
			else
			{
				if(nNeedMute)
				{
					if(nNeedMute==nMaxNeedMute)
					{
						int len = sizeof(databuf);
						if(len>MAX_SOUND_BUF)
						{
							len = MAX_SOUND_BUF; /*最长1秒钟*/
						}

						memset(databuf, 0, len);
						//write(hAudio, databuf, len);
					}

					nNeedMute--;

					if(nNeedMute==0)
					{
						pThis->Mute(1);
					}
				}

				if(hMapFile!=-1)
				{
					close(hMapFile);
					hMapFile = -1;
				}
			}

			//int nVoiceType = pThis->m_nVoiceIcPlayNo;
			pThis->m_nVoiceIcPlayNo = -1;
#endif

			if(bLock)
			{
				pthread_mutex_unlock(&pThis->m_mutex);
				bLock = 0;
			}

			usleep(50*1000);
		}

		if(pThis->m_pMapBuf && pThis->m_nMapLen)
		{
			munmap(pThis->m_pMapBuf, pThis->m_nMapLen); pThis->m_pDataBuf = NULL;
		}

		if(hMapFile!=-1)
		{
			close(hMapFile);
			hMapFile = -1;
		}
	}

	pthread_exit(NULL);
}

/**************************************************************\
** 函数名称： Wav2G711File
** 功能： 音频文件转换
** 参数： pWavFile  输入文件路径 pG711File 输出文件路径
** 返回： 无
** 创建作者： 
** 创建日期： 2012-07-11
** 修改作者：
** 修改日期：
\**************************************************************/
int Wav2G711File(char* pWavFile, char* pG711File)
{
	int ret = -255;
	int hMapFile = open(pWavFile, O_RDONLY);
	if(hMapFile==-1) 
	{
		TRACE("Wav2G711: open wav file %s fail.\r\n", pWavFile);
		ret = -1;
	}
	else 
	{
		struct stat st;
		fstat(hMapFile, &st);
		if(st.st_size>0) 
		{
			char* pMapBuf = (char*)mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, hMapFile, 0);
			if(pMapBuf) 
			{
				/* 解析WAV 生成G711 文件头*/
				char* pBuf = pMapBuf;
				LPWAV_RIFF pRiff = (LPWAV_RIFF)pBuf;
				LPWAV_FMT pFmt = (LPWAV_FMT)(pBuf+sizeof(WAV_RIFF));

				if(strncmp(pRiff->riff_label, "RIFF", 4) 
					|| (8+pRiff->riff_len)<(unsigned long)st.st_size  
					|| strncmp(pRiff->type, "WAVE", 4) 
					|| strncmp(pFmt->fmt_label, "fmt ", 4) )
				{
					TRACE("Wav2G711: %s not a WAV file\r\n", pWavFile);
				}
				else if(pFmt->format!=1 
					|| pFmt->channels!=1
					|| pFmt->samplerate!=8000 
					|| pFmt->bits_per_sample!=16)
				{
					TRACE("Wav2G711: file %s format error. Only samplerate 8000, 16 bits, "
						"mono is supported.\r\n", pWavFile);
				}
				else 
				{
					LPWAV_FACT pFact;
					LPWAV_DATA pData;
					pBuf+=sizeof(WAV_RIFF)+8+pFmt->fmt_len;
					pFact = (LPWAV_FACT)pBuf;

					if(strncmp(pFact->fact_label, "fact", 4)==0) 
					{
						pBuf+=8+pFact->fact_len;
					}
					
					pData = (LPWAV_DATA)pBuf;

					if(strncmp(pData->data_label, "data", 4)==0) 
					{
						pBuf += 8;

						if(( pBuf - pMapBuf + pData->data_len ) > (unsigned long)st.st_size ) 
						{
							TRACE("Wav2G711: file %s format error. filesize invalid.\r\n", pWavFile);
						}
						else 
						{
							/* Write G711 file */
							int hDstFile = open(pG711File, O_CREAT|O_WRONLY|O_TRUNC, 0777);
							if(hDstFile==-1)
							{
								TRACE("Wav2G711: Open G711 file %s fail\r\n", pG711File);
							}
							else 
							{
								int nDataLen = pData->data_len/2;
								char* pWavData = (char*)Malloc(sizeof(WAV_RIFF)+nDataLen*2);
								if(pWavData) 
								{
									LPWAV_RIFF pRiff = (LPWAV_RIFF)pWavData;
									memset(pRiff, 0, sizeof(WAV_RIFF));
									memcpy(pRiff->riff_label, "RIFF", 4);
									pRiff->riff_len = 4+nDataLen;
									memcpy(pRiff->type, "G711", 4);
									memcpy(pWavData+sizeof(WAV_RIFF), pBuf, nDataLen*2);

									G711AEnc((short*)(pWavData+sizeof(WAV_RIFF)), NULL, nDataLen);

									write(hDstFile, pWavData, sizeof(WAV_RIFF)+nDataLen);
									close(hDstFile);
									Free(pWavData);
									ret = 0;
								}
							}
						}
					}
					else
					{
						TRACE("Wav2G711: file %s format error. \"data\" chunk not found.\r\n", pWavFile);
					}
				}
				
				munmap(pMapBuf, st.st_size);
			}
		}
		
		close(hMapFile);
	}
	
	return ret;
}

/**** 语音开关: 0-语音开，1-语音关 ****/
void MuteVoice(int bMute)
{
	AudioCloseSound(bMute);
}

/* 讯飞语音函数实现部分 */

#if 0
/* Message */
ivTTSErrID DoMessage()
{
	/* 获取消息，用户实现 */
	if(1)
	{
		/* 继续合成 */
		return ivTTS_ERR_OK;
	}
	else
	{
		/* 退出合成 */
		return ivTTS_ERR_EXIT;
	}
}

/* output callback */
ivTTSErrID OnOutput(
		ivUInt16		nCode,			/* [in] output data code */
		ivCPointer		pcData,			/* [in] output data buffer */
		ivSize			nSize )			/* [in] output data size */
{

	AudioPlay((char *)pcData, nSize, 1);

	return ivTTS_ERR_OK;
}

/* read resource callback */
ivBool ivCall ReadResCB(
		ivPointer		pParameter,		/* [in] user callback parameter */
		ivPointer		pBuffer,		/* [out] read resource buffer */
		ivResAddress	iPos,			/* [in] read start position */
		ivResSize		nSize )			/* [in] read size */
{
	FILE* pFile = (FILE*)pParameter;
    int err;

	fseek(pFile, iPos, SEEK_SET);
	err = fread(pBuffer, 1, nSize, pFile);
    if ( err == (int)nSize )
	    return ivTrue;
    else
        return ivFalse;
}

/* output callback */
ivTTSErrID ivCall OutputCB(
		ivPointer		pParameter,		/* [in] user callback parameter */
		ivUInt16		nCode,			/* [in] output data code */
		ivCPointer		pcData,			/* [in] output data buffer */
		ivSize			nSize )			/* [in] output data size */
{
	/* 获取线程消息，是否退出合成 */
	ivTTSErrID tErr = DoMessage();
	if ( tErr != ivTTS_ERR_OK ) return tErr;
	/* 把语音数据送去播音 */
	return OnOutput(nCode, pcData, nSize);
}

/* parameter change callback */
ivTTSErrID ivCall ParamChangeCB(
		ivPointer       pParameter,		/* [in] user callback parameter */
		ivUInt32		nParamID,		/* [in] parameter id */
		ivUInt32		nParamValue )	/* [in] parameter value */
{
	return ivTTS_ERR_OK;
}

/* progress callback */
ivTTSErrID ivCall ProgressCB(
		ivPointer       pParameter,		/* [in] user callback parameter */
		ivUInt32		iProcPos,		/* [in] current processing position */
		ivUInt32		nProcLen )		/* [in] current processing length */
{
	return ivTTS_ERR_OK;
}
#endif

void TtsFree()
{
#if 0    
	/* 逆初始化 */
	ivTTS_Destroy(hTTS);

	if ( tResPackDesc.pCacheBlockIndex )
	{
		Free(tResPackDesc.pCacheBlockIndex);
	}
	if ( tResPackDesc.pCacheBuffer )
	{
		Free(tResPackDesc.pCacheBuffer);
	}
	if ( pHeap )
	{
		Free(pHeap);
	}

	//sleep(10);
	fclose((FILE *)tResPackDesc.pCBParam);
#endif
}

/***** 讯飞播放语音接口 ********
参数: voiceText
返回值: 0 成功； -1 不成功
******/
int xfMakePlayVoice(const char *voiceText)
{

    return jtMakePlayVoice(voiceText);
    
#if 0    
    int ret =0;

	/************************************************************************
		块式合成
	************************************************************************/
	/* 设置发音人为 XIAOYAN */
	ret = ivTTS_SetParam(hTTS, ivTTS_PARAM_ROLE, ivTTS_ROLE_XIAOYAN);
	
	ret = ivTTS_SynthText(hTTS, voiceText, (ivSize)-1);
	if(ret == ivTTS_ERR_OK)
	{
		TRACE("ivTTS_SynthText   ivTTS_ERR_OK %s  %d \r\n", __FUNCTION__, __LINE__);
	}
	else
	{
		ret = -1;
	}
    
	return ret;
#endif
}


/* 讯飞语音所需变量初始化 */
int xfVoiceInit()
{
#if 0
	ivTTSErrID		ivReturn;

	//分配堆 
	pHeap = (ivPByte)Malloc(ivTTS_HEAP_SIZE);
	if(pHeap == NULL)
	{
		TRACE("fail to Malloc pHeap. %s  %d \r\n", __FUNCTION__, __LINE__);
		return -1;
	}
	memset(pHeap, 0, ivTTS_HEAP_SIZE);

	/* 初始化资源 */
	/* 可以有多个资源包，可以分包*/
	tResPackDesc.pCBParam = fopen("/root/res/Resource.irf", "rb");
	tResPackDesc.pfnRead = ReadResCB;
	tResPackDesc.pfnMap = NULL;
	tResPackDesc.nSize = 0;

	/* TTS内部使用 */
	tResPackDesc.pCacheBlockIndex = NULL;
	tResPackDesc.pCacheBuffer = NULL;
	tResPackDesc.nCacheBlockSize = 0;
	tResPackDesc.nCacheBlockCount = 0;
	tResPackDesc.nCacheBlockExt = 0;

	if( !tResPackDesc.pCBParam )
	{
		TRACE("fail to open Resource.irf.  %s  %d \r\n", __FUNCTION__, __LINE__);
		return -1;
	}

	ivReturn = 0;
	/* 创建 TTS 实例 */
	ivReturn = ivTTS_Create(&hTTS, (ivPointer)pHeap, ivTTS_HEAP_SIZE, ivNull, (ivPResPackDescExt)&tResPackDesc, (ivSize)1, NULL);

	/* 设置音频输出回调 */
	ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_OUTPUT_CALLBACK, (ivUInt32)OutputCB);
	
	/* 设置参数改变回调 */
	ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_PARAMCH_CALLBACK, (ivUInt32)ParamChangeCB);
	
	/* 设置处理进度回调 */
	ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_PROGRESS_CALLBACK, (ivUInt32)ProgressCB);

	/* 设置输入文本代码页 */
	ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_INPUT_CODEPAGE, ivTTS_CODEPAGE_GBK);

	/* 设置语种 */
	ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_LANGUAGE, ivTTS_LANGUAGE_AUTO);	

	/* 设置音量 */
	ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_VOLUME, ivTTS_VOLUME_NORMAL);
#endif
	return 0;
}

/* 捷通语音实现 */	
// 合成语音数据输出回调函数
// 当合成一段语音时,此函数会被多次调用,每次输出一定量的语音数据
// 用户可在此函数内将语音数据送入音频播放接口
jtErrCode jtOutputVoiceProc(void* pParameter, 
	long iOutputFormat, void* pData, long iSize)
{
	// 如果iSize为-1，则是当前文本的
	// 所有合成数据均已输出，如果需要停止引擎，
	// 则可以在这里进行

    TRACE("iSize %ld %s %d\r\n", iSize, __FUNCTION__, __LINE__);

    if (iSize == 0)
    {
        sleep(1);
    }
    
    if(iSize == -1)
	{
		// jtTTS_SynthesizeText和jtTTS_Synthesize
		// 将输入的文本合成完毕后，会自动退出，
		// 可以不调用jtTTS_SynthStop,而
		// jtTTS_SynthStart合成完毕后，并不主动退出，
		// 需调用jtTTS_SynthStop使之退出
		jtTTS_SynthStop((void*)hTTS);
		return jtTTS_ERR_NONE;
	}
#if 1
    AudioPlay((char *)pData, iSize, 1);

#else

	fwrite(pData, 1, iSize, fpOutPCM);
#endif

	return jtTTS_ERR_NONE;
}


/* 捷通语音所需变量初始化 */
int jtVoiceInit()
{
	jtErrCode dwError;			// 错误码
	long nSize;				// 堆空间大小
	char strCNFileName[] = "/root/Data/CNPackage.dat";
	char strENFileName[] = "/root/Data/ENPackage.dat";

	dwError = jtTTS_GetExtBufSize((const signed char*)strCNFileName, (const signed char*)strENFileName, NULL, &nSize);	
	if(dwError != jtTTS_ERR_NONE)
	{
        TRACE("dwError %d %s  %d \r\n", dwError, __FUNCTION__, __LINE__);

		return -1;
	}

    TRACE("dwError %d %s  %d \r\n", dwError, __FUNCTION__, __LINE__);

	pHeap = (unsigned char *)Malloc(nSize);
    if (pHeap == NULL)
    {
        return -1;
    }

    TRACE("pHeap %p %s  %d \r\n", pHeap, __FUNCTION__, __LINE__);

	memset(pHeap, 0, nSize);

	// 文件读取方式
	dwError = jtTTS_Init((const signed char*)strCNFileName, (const signed char*)strENFileName, NULL, (void **)&hTTS, pHeap);	
	if (dwError != jtTTS_ERR_NONE)
	{
        TRACE("dwError %u hTTS %lu %s  %d \r\n", dwError, hTTS, __FUNCTION__, __LINE__);
        
		return -1;
	}

	/* 设置输入文本代码页 */
#if 0//defined(UNICODE) || defined(_UNICODE)
	dwError = jtTTS_SetParam((void*)hTTS, jtTTS_PARAM_CODEPAGE, (void*)jtTTS_CODEPAGE_UNICODE);
#else
	dwError = jtTTS_SetParam((void*)hTTS, jtTTS_PARAM_CODEPAGE, (void*)jtTTS_CODEPAGE_GBK);
#endif

    TRACE("dwError %d %s  %d \r\n", dwError, __FUNCTION__, __LINE__);

	/* 设置直接文本输入 */
	dwError = jtTTS_SetParam((void*)hTTS, jtTTS_PARAM_INPUTTXT_MODE, 
		(void*)jtTTS_INPUT_TEXT_DIRECT);

    TRACE("dwError %d %s  %d \r\n", dwError, __FUNCTION__, __LINE__);

	/* 设置音频输出回调 */
	dwError = jtTTS_SetParam((void*)hTTS, jtTTS_PARAM_OUTPUT_CALLBACK, 
		(void *)jtOutputVoiceProc);

    TRACE("dwError %d %s  %d \r\n", dwError, __FUNCTION__, __LINE__);
    
    //fpOutPCM = fopen("./Output.pcm", "wb");

	jtTTS_SetParam((void*)hTTS, jtTTS_PARAM_VOLUME, (void*)jtTTS_VOLUME_NORMAL);
	jtTTS_SetParam((void*)hTTS, jtTTS_PARAM_SPEED, (void*)jtTTS_SPEED_MID);
	//jtTTS_SetParam((void*)hTTS, jtTTS_PARAM_SPEED, (void*)jtTTS_SPEED_NORMAL);
	jtTTS_SetParam((void*)hTTS, jtTTS_PARAM_PITCH, (void*)jtTTS_PITCH_NORMAL);
    jtTTS_SetParam((void*)hTTS, jtTTS_PARAM_WAV_FORMAT, (void*)jtTTS_FORMAT_PCM_8K16B);

	return 0;
}


/***** 捷通播放语音接口 ********
参数: voiceText
返回值: 0 成功； -1 不成功
******/
int jtMakePlayVoice(const char *voiceText)
{
	jtErrCode dwError;			// 错误码

	dwError = jtTTS_SynthesizeText((void*)hTTS, 
		voiceText, strlen(voiceText) * sizeof(char));

    TRACE("jtTTS_SynthesizeText %d %s  %d \r\n", dwError, __FUNCTION__, __LINE__);

    if (dwError != jtTTS_ERR_NONE)
	{
		return -1;
	}

	return 0;
}

/* 捷通语音所需变量初始化 */
int jtVoiceDeInit()
{
	/* 逆初始化 */
	jtTTS_End((void*)hTTS);		

	if (pHeap != NULL)
	{
		Free(pHeap);
	}

    //if (fpOutPCM != NULL )
	//	fclose(fpOutPCM);

    return 0;
}

	
