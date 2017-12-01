/*

* (C) 2017 see Authors.txt
*
* This file is part of MPC-HC.
*
* MPC-HC is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* MPC-HC is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
// Header file for Audio Services
#ifndef _WAVERECORD_H_
#define _WAVERECORD_H_
#include <Windows.h>
 
#define TOTAL_WAVE_BUFSIZE 262144//60000		// buffer about 256K

class CMainFrame;
typedef struct tagWaveFmt
{

	WORD nChannels; //= 1;
	DWORD nSamplesPerSec;// = 8000;
	WORD wBitsPerSample;// = 8;
	WORD wBitrate;
} WaveFmt;

typedef struct _tagSEGMENTFRAME
{
	unsigned long lStartFrame;
	unsigned long lEndFrame;
} SEGMENTFRAME;


typedef struct _tagWAVHDR
{
	BYTE  riff[4];            /* must be "RIFF"                */
	DWORD len;                /* #bytes + 44 - 8               */
	BYTE  cWavFmt[8];         /* must be "WAVEfmt"             */
	DWORD dwHdrLen;
	WORD  wFormat;
	WORD  wNumChannels;
	DWORD dwSampleRate;
	DWORD dwBytesPerSec;
	WORD  wBlockAlign;
	WORD  wBitsPerSample;
	BYTE  cData[4];            /* must be "data"               */
	DWORD dwDataLen;           /* #bytes                       */
} WAVHDR, *PWAVHDR, *LPWAVHDR;



class CWaveRecorder {
public:
	CWaveRecorder()
	{
		thisobj = this;
	}
	int  DShowRecordWaveFile(LPCTSTR pSrcFileName, LPCTSTR pFileName, WaveFmt pWaveFmt, bool pbMp3, bool pbFirstCall, bool pbIsPreview);
	void StopWaveRecord();
	static CWaveRecorder *thisobj ;
	void setMainFrm(CMainFrame *pMainFrame) { m_pMainFrame = pMainFrame; }
	void WaveRecInBlock(WPARAM wParam, LPARAM lParam);
private:
	CMainFrame *m_pMainFrame;
	
//functions
	
	int CopyDataToMP3File(char *inbuf, long pBufSize);
	//int CopyDataToWaveFile(char *inbuf, UINT size);
	int DShowAddNextBuffer();
	int DShowopenWaveDev2(LPCTSTR pSrcFileName, LPCTSTR pFileName, WaveFmt pWaveFmt,  bool pbFirstCall, bool pbIsPreview, int pnTrackNr);
	
	//int CerateWaveFileHeader(LPCTSTR pFileName);
	//void WriteWavHeader(void);
	static DWORD WINAPI DumpMP3Data(LPVOID *p);
	static DWORD WINAPI Recorder(LPVOID *p);
	DWORD RecorderInThread(LPVOID *p);
	DWORD DumpMP3DataInThread(LPVOID *p);
	void InitPCMWaveFmt(WaveFmt pWaveFmt);

	//int CloseWaveFile();

// members:
	bool g_bSignal;  // if true has signal else no signal
	bool USEDSHOW = true;
	bool RecordStoped;
	bool RecordPaused;

	bool gWriteToFile; // the flag for write file or not
	bool gbMp3;  // if the dest file is MP3 or not

	bool Mp3WriteOnce;//       has been write to file once
	

	DWORD gdwBytesRecorded;


	time_t  tStart, tFinish;   //

// Event Handle
	HANDLE MP3Done;		// event of audio input completion
	HANDLE recordDone;		// event of audio input completion 
	HANDLE MP3Ready;      
	HWAVEIN hWaveIn;		// onput WAVE audio device handle 
	
	HANDLE RecordHandle;    // the handle of record  thread

// buffers and related:
	WAVEFORMATEX PCMWaveFmtRecord;
	WaveFmt gWaveFmt;      // wave Format

	WAVEHDR WaveHeaderRec[3];
	int bufindex;

	DWORD _totalWritten;
	DWORD g_dwTotalBytes;

	DWORD dwBytedatasize = 0;	// accumulative size of recorded data
	DWORD dwTotalwavesize;	// total number of samples recorded
	DWORD WAVE_BUFSIZE;

	DWORD gdwInBufSize;    // MP3 encode input buffer size
	DWORD gdwOutBufSize;  // MP3 encode output buffer size

	char *gMp3NewThreadBuf;  // MP3 buffer Pointer
	char *gMp3Buffer = NULL;
	char *gMp3Buffer2 = NULL;

	char gszInbuf1[TOTAL_WAVE_BUFSIZE];	// WAVE input data buffer area 
	char gszInbuf2[TOTAL_WAVE_BUFSIZE];	// WAVE input data buffer area 
	char gszInbuf3[TOTAL_WAVE_BUFSIZE];	// WAVE input data buffer area 
	char gszMP3Inbuf3[TOTAL_WAVE_BUFSIZE * 2];   // used for MP3
	DWORD gMP3Offset;   // offset in the MP3 buffer
	LPSTR pwavemem[3];  // pointer of Wav buffer

// File to write
	FILE *WavLi;
	FILE *gMp3File=NULL;



};

#endif
