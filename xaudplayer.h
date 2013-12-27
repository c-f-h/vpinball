#pragma once

/* xaudio general includes */
#include "media\xaudio\decoder.h"

/* xaudio plug-in modules */
#include "media\xaudio\memory_input.h"
#include "media\xaudio\audio_output.h"
#include "media\xaudio\mpeg_codec.h"

class XAudPlayer
{
public:
	XAudPlayer();
	~XAudPlayer();

	int Init(char * const szFileName, const int volume);

	int Tick();

	void End();

	void Pause();
	void Unpause();

private:
	XA_DecoderInfo *m_decoder;

	FILE *file;

	int m_cDataLeft;
	int m_lastplaypos;

	bool m_fStarted;
	bool m_fEndData; // all data has been decoded - wait for buffer to play

	HRESULT CreateBuffer(const int volume);
	HRESULT CreateStreamingBuffer(WAVEFORMATEX *pwfx);

//#define NUM_PLAY_NOTIFICATIONS  16

	//LPDIRECTSOUND       m_pDS            = NULL;
	LPDIRECTSOUNDBUFFER m_pDSBuffer;
	//LPDIRECTSOUNDNOTIFY m_pDSNotify;

	//DSBPOSITIONNOTIFY   m_aPosNotify;  
	//HANDLE              m_hNotificationEvent;

	DWORD               m_dwBufferSize;
	DWORD               m_dwNextWriteOffset;
};
