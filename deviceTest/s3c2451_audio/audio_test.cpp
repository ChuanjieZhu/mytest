

#include <stdlib.h>
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

#include "audio_test.h"
#include "./2451/ivTTS.h"

#define SOUND_DEVICE "/dev/dsp"
#define SOUND_MIXER	"/dev/mixer"

#define IO_CFG_AUDIO_EX      _IOWR('E', 0x3f, int)     
#define SOUND_OUTPUT_CLOSE MIXER_WRITE(14)
#define SOUND_OUTPUT_OPEN MIXER_WRITE(15)

#ifndef PACK_ALIGN
#define PACK_ALIGN	__attribute__((packed))
#endif

class CAudio* g_pAudio = NULL;

/*------------------------------------------------------------------------------------*/
#define ivTTS_HEAP_SIZE		70000 /* ��ϣ���Ч */

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

void TtsFree();
/* Ѷ�������ӿ� */
int xfMakePlayVoice(const char *voiceText);
/* Ѷ���������������ʼ�� */
int xfVoiceInit();
/*------------------------------------------------------------------------------------*/

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

void AudioPlay(char* pAudioData, int nDataLen, int bAppend)
{
	if(g_pAudio)
	{
		g_pAudio->Play(pAudioData,nDataLen,bAppend);
	}
}

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
	m_nDataBufLen = 8000*2*1*5; /* 5 seconds */	/*  ���ò��Ż���Ĵ�С */
	m_pDataBuf = NULL;

	/*  �������豸 */
	m_hMixer = open( SOUND_MIXER, O_RDWR );	
	if( m_hMixer == -1 )
	{
		printf("Failed to open /dev/mixer\r\n");
	}
	else
	{
		Mute(1);
	}

	SetVolume(0);
	
	/*  ����Ƶ�豸 */
	m_hAudio = open(SOUND_DEVICE, O_WRONLY);	
	if( m_hAudio == -1 )
	{
		printf("Open sound device %s Failed\r\n", SOUND_DEVICE);
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
			printf("Set AFMT_S16_LE Failed\r\n");
		}

		/* set samplerate 8000 or 32000 */
		val = 32000;
		if( ioctl(m_hAudio, SNDCTL_DSP_SPEED, &val) == -1 )
		{
			printf("Set SPEED %d Failed\r\n", val);
		}

		/* force set channel to STEREO */
		val = 1;
		if( ioctl(m_hAudio, SNDCTL_DSP_STEREO, &val) == -1 )
		{
			printf("Set %s Failed\r\n", val?"STEREO":"MONO");
		}

		m_bClose = 0;
		m_bExit = 0;
	}

	m_nVoiceIcPlayNo = -1;

	/*  ���벥�Ż��� */
	if(m_bExit == 0)
	{
		m_pDataBuf = NULL;
		m_pDataBuf = (char*)malloc(m_nDataBufLen);
		if(m_pDataBuf == NULL)
		{
			printf("Set %s Failed\r\n", val?"STEREO":"MONO");
		}
	}

	pthread_mutex_init(&m_mutex, NULL);

	m_thread = 0;

	/* lph Ѷ��������ʼ������ */
	ret=xfVoiceInit();
	if(ret == -1)
	{
		return;
	}
	
	if(m_bExit==0)
		pthread_create(&m_thread, NULL, process, this);		//���������߳�
}

CAudio::~CAudio()
{
	if(m_bExit==0)
	{
		m_bExit = 1;
		sleep(1);
		if(m_thread)
		{
			usleep(100*1000);
			pthread_join(m_thread, NULL);		/*  �ȴ�process�߳��˳��� */
		}
	}

	pthread_mutex_destroy(&m_mutex);	/*  ���ٻ����� */

	//Mute(1);	/*  ��������Ϊ0 */

	/*  �ͷŲ��ŵĻ��� */
	if(m_pDataBuf != NULL)
	{
		free(m_pDataBuf);
		m_pDataBuf = NULL;
	}

	/*  �ͷŲ����ļ��Ļ��� */
	if(m_pMapBuf != NULL)
	{
		munmap(m_pMapBuf, m_nMapLen);
		m_pMapBuf = NULL;
	}
	
	/*  �ر���Ƶ */
	if(m_hMixer != -1)
	{
		close(m_hMixer);
		m_hMixer = -1;
	}

	/*  ���ڲ��ŵ����� */
	if(m_hAudio != -1)
	{
		close(m_hAudio);
		m_hAudio = -1;
	}

	/* Ѷ�������ͷ� */
	TtsFree();
}

void CAudio::Mute(int bMute)
{
	return;
	if(bMute)
		SetVolume(-1);
	else
		SetVolume(m_nVolume);
}

void CAudio::SetVolume(int nVolume)
{
	if(m_bExit==0)
	{
		int bModifyVolume = (nVolume >= 0);

		/*  ���С�ڵ���0���Ǿ�����������Ϊ0 */
		if(nVolume<=0)
		{
			nVolume = 0;
		}
		/*  ���С�ڵ���0���Ǿ�����������Ϊ0 */
		else if(nVolume>100)
		{
			nVolume = 100;
		}
		
		/*  �������0���Ǿ����������� */
		if(bModifyVolume)
		{
			m_nVolume = nVolume;
		}

		/*  �������� */
		if(m_hMixer!=-1)
		{
			nVolume &= 0xFF;
			nVolume = nVolume | (nVolume << 8);

			if( ioctl(m_hMixer, SOUND_MIXER_WRITE_VOLUME, &nVolume) == -1 )
			{
				printf("Failed to set the volume.\r\n");
			}
		}
	}
}

void CAudio::Play(char* pAudioData, int nDataLen, int bAppend)
{
	pthread_mutex_lock(&m_mutex);
	if(bAppend == 0)
		m_nDataLen = 0;
	if(m_bClose == 0 && m_nVolume > 0 && m_pDataBuf && pAudioData && nDataLen > 0)
	{
		while(nDataLen)
		{
			int nLen = nDataLen;
            /*
             *  ������ڴ����ŵĳ���+Ҫ���ŵĳ��� ���� ����Ļ�����󳤶ȣ�
             *  ��ô��ȡ���廹�пռ�ʣ��ĳ��� 
             */
			if(m_nDataLen + nLen > m_nDataBufLen)	
				nLen = m_nDataBufLen - m_nDataLen;

			memcpy(m_pDataBuf+m_nDataLen, pAudioData, nLen);
			m_nDataLen += nLen;
			nDataLen -= nLen;
            
			/*  sleep�ȴ����ڲ��ŵ����� */
			if(nDataLen)
			{
				pthread_mutex_unlock(&m_mutex);
				sleep(1);
				pthread_mutex_lock(&m_mutex);
			}
		}
	    Mute(0);
	}
	pthread_mutex_unlock(&m_mutex);
}

void* CAudio::process(void* pArg)
{
	printf("*** entry thread %s %d\r\n", __FUNCTION__, __LINE__);

	CAudio* pThis = (CAudio*)pArg;
	if(pThis)
	{
		static char databuf[(int)(8000*2*4)]; /* 0.5 seconds data buffer of 8K Mono audio to 32K Stereo*/
		int hAudio = pThis->m_hAudio;
		int nMapPlayOffset=0, nNeedMute = 0, nMaxNeedMute = 10, bLock = 0;

		/* ��ϵͳ�����״η���֮ǰ��Ҫ���������豸��д��64k�հ����ݣ������״η�����ʧ�� */
		memset(databuf, 0, 8000 * 2 * 4);
		write(hAudio, databuf, 8000 * 2 * 4);

        //printf("----------------------->pThis->m_bExit %d %s %d\r\n", pThis->m_bExit, __FUNCTION__, __LINE__);
        
		/*  nMapPlayOffset-����ʱmap�ļ��ı��˵�ַ�� */
		while(pThis->m_bExit==0)
		{
			bLock = 1;

			pthread_mutex_lock(&pThis->m_mutex);

            //printf("----------------------->pThis->m_nDataLen %d %s %d\r\n", pThis->m_nDataLen, __FUNCTION__, __LINE__);
			/*  ���ŵ������� */
			if(pThis->m_nDataLen)
			{
				int nLen = 0;
				
				/*  ���ŵ������� */
				if(pThis->m_nDataLen)
				{
					nLen = pThis->m_nDataLen;
					/*  ������ŵ����ݳ��ȴ���ÿ�ε���󲥷ſ飨databuf������ô�Ͱ�nLen��Ϊ��󲥷ſ� */
					if( nLen > (int)(sizeof(databuf)/8))
					{
						nLen = (int)sizeof(databuf)/8;
					}

					memcpy(databuf, pThis->m_pDataBuf, nLen);	/*  �������ŵ����� */
					pThis->m_nDataLen -= nLen;	/*  ��ʣ�����ŵ����� */

					/*  �Ѻ���������Ƶ�ǰ�棬׼�����š� */
					if(pThis->m_nDataLen)
					{
						memmove(pThis->m_pDataBuf, pThis->m_pDataBuf+nLen, pThis->m_nDataLen);
					}
				}

				pthread_mutex_unlock(&pThis->m_mutex);
				bLock = 0;

				/*  ��������ʱ������ת�� */
				if(nLen>0)
				{
					Rate8KMono_To_Rate32KStereo((unsigned short*)databuf, nLen/sizeof(short));

					nLen*=8;

					nLen = write(hAudio, databuf, nLen);/*  ���� */

                    //printf("-----------------------> %s %d\r\n", __FUNCTION__, __LINE__);
                    
					if(nLen<=0)
					{
						printf("Play audio Failed\r\n");
						pThis->m_bExit = 1;
					}

					nNeedMute = nMaxNeedMute;
				}
				
			}
			else
			{
				if(nNeedMute)
				{
					if(nNeedMute==nMaxNeedMute)
					{
						int len = sizeof(databuf);
						if(len>8000*2*4)
						{
							len = 8000*2*4; /*�1����*/
						}

						memset(databuf, 0, len);
						write(hAudio, databuf, len);
					}

					nNeedMute--;

					if(nNeedMute==0)
					{
						pThis->Mute(1);
					}
				}
			}

			//int nVoiceType = pThis->m_nVoiceIcPlayNo;
			pThis->m_nVoiceIcPlayNo = -1;

			if(bLock)
			{
				pthread_mutex_unlock(&pThis->m_mutex);
				bLock = 0;
			}

			usleep(50*1000);
		}
	}

	pthread_exit(NULL);
}

/*-----------------------------------------Ѷ������-------------------------------------------*/
/* Message */
ivTTSErrID DoMessage()
{
	/* ��ȡ��Ϣ���û�ʵ�� */
	if(1)
	{
		/* �����ϳ� */
		return ivTTS_ERR_OK;
	}
	else
	{
		/* �˳��ϳ� */
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
	/* ��ȡ�߳���Ϣ���Ƿ��˳��ϳ� */
	ivTTSErrID tErr = DoMessage();
	if ( tErr != ivTTS_ERR_OK ) return tErr;
	/* ������������ȥ���� */
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

void TtsFree()
{
	/* ���ʼ�� */
	ivTTS_Destroy(hTTS);

	if ( tResPackDesc.pCacheBlockIndex )
	{
		free(tResPackDesc.pCacheBlockIndex);
	}
	if ( tResPackDesc.pCacheBuffer )
	{
		free(tResPackDesc.pCacheBuffer);
	}
	if ( pHeap )
	{
		free(pHeap);
	}

	//sleep(10);
	fclose((FILE *)tResPackDesc.pCBParam);
	
}

/***** Ѷ�ɲ��������ӿ� ********
����: voiceText
����ֵ: 0 �ɹ��� -1 ���ɹ�
******/
int xfMakePlayVoice(const char *voiceText)
{
	int ret = 0;    
    
	/************************************************************************
		��ʽ�ϳ�
	************************************************************************/
	/* ���÷�����Ϊ XIAOYAN */
    
	ret = ivTTS_SetParam(hTTS, ivTTS_PARAM_ROLE, ivTTS_ROLE_XIAOYAN);
	
	ret = ivTTS_SynthText(hTTS, voiceText, (ivSize)-1);
	if(ret == ivTTS_ERR_OK)
	{
		//printf("ivTTS_SynthText   ivTTS_ERR_OK %s %d\r\n", __FUNCTION__, __LINE__);
	}
	else
	{
		ret = -1;
	}

	return ret;

}


/* Ѷ���������������ʼ�� */
int xfVoiceInit()
{
	ivTTSErrID		ivReturn;
    char acFilePath[256];

	//����� 
	pHeap = (ivPByte)malloc(ivTTS_HEAP_SIZE);
	if(pHeap == NULL)
	{
		printf("fail to malloc pHeap. %s  %d \r\n", __FUNCTION__, __LINE__);
		return -1;
	}
    
	memset(pHeap, 0, ivTTS_HEAP_SIZE);
    memset(acFilePath, 0, sizeof(acFilePath));

    sprintf(acFilePath, "%s", "Resource.irf");

	/* ��ʼ����Դ */
	/* �����ж����Դ�������Էְ�*/
	tResPackDesc.pCBParam = fopen(acFilePath, "rb");
	tResPackDesc.pfnRead = ReadResCB;
	tResPackDesc.pfnMap = NULL;
	tResPackDesc.nSize = 0;

	/* TTS�ڲ�ʹ�� */
	tResPackDesc.pCacheBlockIndex = NULL;
	tResPackDesc.pCacheBuffer = NULL;
	tResPackDesc.nCacheBlockSize = 0;
	tResPackDesc.nCacheBlockCount = 0;
	tResPackDesc.nCacheBlockExt = 0;

	if( !tResPackDesc.pCBParam )
	{
		printf("fail to open Resource.irf.  %s  %d \r\n", __FUNCTION__, __LINE__);
		return -1;
	}

	ivReturn = 0;
	/* ���� TTS ʵ�� */
	ivReturn = ivTTS_Create(&hTTS, (ivPointer)pHeap, ivTTS_HEAP_SIZE, ivNull, (ivPResPackDescExt)&tResPackDesc, (ivSize)1, NULL);

	/* ������Ƶ����ص� */
	ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_OUTPUT_CALLBACK, (ivUInt32)OutputCB);
	
	/* ���ò����ı�ص� */
	ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_PARAMCH_CALLBACK, (ivUInt32)ParamChangeCB);
	
	/* ���ô�����Ȼص� */
	ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_PROGRESS_CALLBACK, (ivUInt32)ProgressCB);

	/* ���������ı�����ҳ */
	ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_INPUT_CODEPAGE, ivTTS_CODEPAGE_GBK);

	/* �������� */
	ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_LANGUAGE, ivTTS_LANGUAGE_AUTO);	

	/* �������� */
	ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_VOLUME, ivTTS_VOLUME_NORMAL);

	return 0;
}
/*-----------------------------------------Ѷ������-------------------------------------------*/

int main(int argc, char *argv[])
{
    AudioInit();

    AudioSetVolume(50);

    time_t tt;
    struct tm stm;
    struct tm *pStm = &stm;

    tt = time(0);
    localtime_r(&tt, pStm);

    printf("---------------[%d-%02d-%02d %02d:%02d:%02d]--------------\r\n",
        pStm->tm_year + 1900, pStm->tm_mon + 1, pStm->tm_mday,
        pStm->tm_hour, pStm->tm_min, pStm->tm_sec);

    int i = 0;
    
    while (1)
    {
        if (i == 0)
        {
            xfMakePlayVoice((const char *)"�߶���������");
            i = 1;
        }
        else
        {
            xfMakePlayVoice((const char *)"����������ǿ");
            i = 0;
        }
        usleep(500 * 1000);
    }

    AudioUninit();
}

