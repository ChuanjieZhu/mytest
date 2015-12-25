/*******************************************************************************
**  Copyright (c) 2014, Company
**  All rights reserved.
**	
**  文件说明: afg10 2451 audio测试接口
**  创建日期: 2014.02.26
**
**  当前版本：1.0
**  作者：
********************************************************************************/

#ifndef AUDIO_TEST_H
#define AUDIO_TEST_H

class CAudio
{
    private:
        int m_hAudio;
        int m_hMixer;
    	int m_bFormatG711;	/*  是否为G711音频格式 */
    	int m_bClose;	/*  是否已经关闭，1为关闭，0为没有关闭。 */
    	int m_bMute;	
    	int m_bExit;	/*  系统是否已经退出，1为退出，0为没有退出。 */
    	int m_bMuteAlarm;	/*  貌似没有用的咚咚 */

    	char *m_pMapFileName;	/*  播放文件时，要map的文件名 */
    	char *m_pMapBuf;		/*  map后open的文件描述符hMapFile */
    	char *m_pDataBuf;		/*  map后后的buf指针 */

    	int m_nVolume;			/*  音量值 */
    	int m_nDataBufLen;		/*  m_pDataBuf的长度 */
    	int m_nMapLen;			/*  被播放的文件map的长度 */
    	int m_nDataLen;			/*  播放内容的长度 */
    	int m_nVoiceIcPlayNo;	/*  貌似现在没有用 */

    	pthread_mutex_t m_mutex;	/*  播放互斥锁，因为播放时把数据放到m_pDataBuf */

    	pthread_t m_thread;		/*  process线程ID */

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
