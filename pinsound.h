#pragma once
// PinSound.h: interface for the PinSound class.
//
//////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>
using namespace std;

#if !defined(AFX_PINSOUND_H__61491D0B_9950_480C_B453_911B3A2CDB8E__INCLUDED_)
#define AFX_PINSOUND_H__61491D0B_9950_480C_B453_911B3A2CDB8E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


struct DSAudioDevice
{
    LPGUID guid;
    string description;
    string module;
    DSAudioDevice () {
        guid = NULL;
    }
    ~DSAudioDevice () {
	    if(guid)
            delete guid;
    }
};

typedef vector<DSAudioDevice*> DSAudioDevices;

BOOL CALLBACK DSEnumCallBack(LPGUID guid, LPCSTR desc, LPCSTR mod, LPVOID list);


class PinSound
{
public:
	PinSound();
	virtual ~PinSound();

	LPDIRECTSOUNDBUFFER m_pDSBuffer;

	char m_szName[MAXTOKEN];
	char m_szInternalName[MAXTOKEN];
	char m_szPath[MAX_PATH];

	char *m_pdata; // Copy of the buffer data so we can save it out
	int m_cdata;
};

class PinSoundCopy
	{
public:
	LPDIRECTSOUNDBUFFER m_pDSBuffer;
	PinSound *m_ppsOriginal;
	};

class PinDirectSound
{
public:
	PinDirectSound();
	~PinDirectSound();

	void InitDirectSound(HWND hwnd);
	PinSound *LoadWaveFile( TCHAR* strFileName );
	HRESULT CreateStaticBuffer(TCHAR* strFileName, PinSound *pps);
	HRESULT FillBuffer(PinSound *pps);
	HRESULT CreateDirectFromNative(PinSound *pps, WAVEFORMATEX *pwfx);
	HRESULT RestoreBuffers();

	LPDIRECTSOUND       m_pDS;
	//LPDIRECTSOUNDBUFFER m_pDSBuffer;
	CWaveSoundRead*     m_pWaveSoundRead;
	DWORD               m_dwBufferBytes;
};

#endif // !defined(AFX_PINSOUND_H__61491D0B_9950_480C_B453_911B3A2CDB8E__INCLUDED_)
