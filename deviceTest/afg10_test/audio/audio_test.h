/*******************************************************************************
**  Copyright (c) 2014, Company
**  All rights reserved.
**	
**  �ļ�˵��: afg10 2451 audio���Խӿ�
**  ��������: 2014.02.26
**
**  ��ǰ�汾��1.0
**  ���ߣ�
********************************************************************************/

#ifndef AUDIO_TEST_H
#define AUDIO_TEST_H

class CAudio
{
    private:
        int m_hAudio;
        int m_hMixer;
    	int m_bFormatG711;	/*  �Ƿ�ΪG711��Ƶ��ʽ */
    	int m_bClose;	/*  �Ƿ��Ѿ��رգ�1Ϊ�رգ�0Ϊû�йرա� */
    	int m_bMute;	
    	int m_bExit;	/*  ϵͳ�Ƿ��Ѿ��˳���1Ϊ�˳���0Ϊû���˳��� */
    	int m_bMuteAlarm;	/*  ò��û���õ����� */

    	char *m_pMapFileName;	/*  �����ļ�ʱ��Ҫmap���ļ��� */
    	char *m_pMapBuf;		/*  map��open���ļ�������hMapFile */
    	char *m_pDataBuf;		/*  map����bufָ�� */

    	int m_nVolume;			/*  ����ֵ */
    	int m_nDataBufLen;		/*  m_pDataBuf�ĳ��� */
    	int m_nMapLen;			/*  �����ŵ��ļ�map�ĳ��� */
    	int m_nDataLen;			/*  �������ݵĳ��� */
    	int m_nVoiceIcPlayNo;	/*  ò������û���� */

    	pthread_mutex_t m_mutex;	/*  ���Ż���������Ϊ����ʱ�����ݷŵ�m_pDataBuf */

    	pthread_t m_thread;		/*  process�߳�ID */

    public:
        CAudio();
        ~CAudio();
        void Mute(int bMute);
        void SetVolume(int nVolume);
        void Play(char* pAudioData, int nDataLen, int bAppend=1);
        void PlayFile(char* pFile, int bAppend=1);
        static void* process(void* pArgs);	
};


#endif //AUDIO_TEST_H
