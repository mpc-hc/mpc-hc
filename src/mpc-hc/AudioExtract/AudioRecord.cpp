/*

*  (C) 2017 see Authors.txt
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
#include "stdafx.h"

#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include "AudioRecord.h"
#include "MP3.h"
#include "..\MainFrm.h"


CWaveRecorder * CWaveRecorder::thisobj = nullptr;
void CWaveRecorder::WaveRecInBlock(WPARAM wParam, LPARAM lParam)
{
	double  elapsed_time;
	DWORD vbugsize;
	LPWAVEHDR lpwh = (LPWAVEHDR)lParam;

	time(&tFinish);

	vbugsize = ((LPWAVEHDR)lParam)->dwBytesRecorded;
	gdwBytesRecorded = vbugsize;

	if (!RecordStoped)              /// make sure nuot execut when stop, 
	{
		if (!RecordPaused)
			if (gWriteToFile)
			{
				if (gbMp3)
				{   
					CopyDataToMP3File(lpwh->lpData, vbugsize);
					WaitForSingleObject(MP3Done, 1000);// INFINITE);
					ResetEvent(MP3Done);
				}
				//else
				//{
				//	CopyDataToWaveFile(lpwh->lpData, (UINT)(((LPWAVEHDR)lParam)->dwBytesRecorded));
				//}
			}
	}

	if (!RecordStoped)
	{
		DShowAddNextBuffer();			// queue it again...

	}

	else
		SetEvent(recordDone);

	m_pMainFrame->SetBufferEvent();
}



int CWaveRecorder::CopyDataToMP3File(char *inbuf, long pBufSize)
{
	memcpy(gszMP3Inbuf3 + gMP3Offset, inbuf, pBufSize);
	gMp3NewThreadBuf = inbuf;
	SetEvent(MP3Ready);

	return 1;
}



//
//int CWaveRecorder::CopyDataToWaveFile(char *inbuf, UINT size)
//{
//	if (size == 0)
//		return 1;
//
//	_totalWritten += size;
//
//	g_dwTotalBytes += size;
//
//	if (WavLi != NULL)
//		fwrite(inbuf, 1, size, WavLi);
//	return 0;
//}
//



int CWaveRecorder::DShowAddNextBuffer()
{
	// queue the buffer for input...

	if (gWaveFmt.wBitsPerSample == 16)
		memset(WaveHeaderRec[bufindex].lpData, 0x00, TOTAL_WAVE_BUFSIZE);
	else
		memset(WaveHeaderRec[bufindex].lpData, 0x80, TOTAL_WAVE_BUFSIZE);

	m_pMainFrame->waveInAddBuffer(&WaveHeaderRec[bufindex], sizeof(WAVEHDR));

	bufindex = (bufindex + 1) % 3;
	return 0;
}


void CWaveRecorder::StopWaveRecord()
{
	RecordStoped = true;;
	m_pMainFrame->SetBufferEvent();
	SetEvent(MP3Ready);
	SetEvent(recordDone);
	WaitForSingleObject(RecordHandle, 1000);
}

//void CWaveRecorder::WriteWavHeader(void)
//{
//	WAVHDR wav;
//	ZeroMemory(&wav, sizeof(wav));
//
//	memcpy(wav.riff, "RIFF", 4);
//	wav.len = _totalWritten + 44 - 8;
//	memcpy(wav.cWavFmt, "WAVEfmt ", 8);
//	wav.dwHdrLen = 16;
//	wav.wFormat = PCMWaveFmtRecord.wFormatTag;//1;
//	wav.wNumChannels = PCMWaveFmtRecord.nChannels;
//	wav.dwSampleRate = PCMWaveFmtRecord.nSamplesPerSec;
//	wav.dwBytesPerSec = PCMWaveFmtRecord.nAvgBytesPerSec;//44100*2*2;
//	wav.wBlockAlign = PCMWaveFmtRecord.nBlockAlign;//4;
//	wav.wBitsPerSample = PCMWaveFmtRecord.wBitsPerSample;//16;
//	memcpy(wav.cData, "data", 4);
//	wav.dwDataLen = _totalWritten;
//	fseek(WavLi, 0, SEEK_SET);
//	fwrite(&wav, 1, sizeof(wav), WavLi);
//}

//int CWaveRecorder::CerateWaveFileHeader(LPCTSTR pFileName)
//{
//	_totalWritten = 0;
//
//	char tmp[256];
//	sprintf(tmp, "%s", pFileName);
//
//	WavLi = fopen(tmp, "w+b");
//	if (WavLi == NULL)
//	{
//		sprintf(tmp, "File %s connot be opened for writing\nIf it's used by other appliction please close it and retry", pFileName);
//		//MessageBox(NULL, tmp, "Error", MB_OK);
//		return 1;
//	}
//
//	WriteWavHeader();
//
//	return 0;
//
//}

DWORD WINAPI CWaveRecorder::DumpMP3Data(LPVOID *p)
{
	return thisobj->DumpMP3DataInThread(p);
}

DWORD CWaveRecorder::DumpMP3DataInThread(LPVOID *p)
{
	int tmpindex, i;
	char *tmpbuf;
	DWORD dwBytesWrited; 
	Mp3WriteOnce = false;
	DWORD dwBytesLeft, dwIndex = 0;
	while (1)
	{
		if (RecordStoped)
			break;

		WaitForSingleObject(MP3Ready, INFINITE);
		ResetEvent(MP3Ready);

		if (gMp3NewThreadBuf != NULL)
		{
			tmpbuf = gMp3NewThreadBuf;
			gMp3NewThreadBuf = NULL;

			tmpindex = bufindex;
			if (gMp3File != NULL)
			{
				if (gMp3Buffer != NULL)
				{
					dwBytesLeft = gdwBytesRecorded + gMP3Offset;
					dwIndex = 0;
					dwBytesWrited = 0;
					while (dwBytesLeft>gdwInBufSize * 2)
					{
						dwBytesWrited = MP3EncodeBuf(gMp3File, gszMP3Inbuf3 + dwIndex, gMp3Buffer, gdwInBufSize, gdwOutBufSize);
						dwIndex += (gdwInBufSize * 2);
						dwBytesLeft -= (gdwInBufSize * 2); 
						g_dwTotalBytes += dwBytesWrited;
					}
					if (dwBytesLeft>0)
					{
						if (dwBytesWrited>0)
							memcpy(gszMP3Inbuf3, gszMP3Inbuf3 + dwIndex, dwBytesLeft);
						gMP3Offset = dwBytesLeft;
					}
					else
						gMP3Offset = 0;
					Mp3WriteOnce = true;
				}

				SetEvent(MP3Done);
			}
		}
		Sleep(1);
	}
	return 0;
}

int  CWaveRecorder::DShowRecordWaveFile(LPCTSTR pSrcFileName, LPCTSTR pFileName, WaveFmt pWaveFmt, bool pbMp3, bool pbFirstCall, bool pbIsPreview)
{
	int pChannel;
	DWORD threadid;
	int vnTrackNr = -1;
	g_bSignal = false;         // init the signal
	
	gbMp3 = pbMp3;
	if (pFileName == NULL)
		gWriteToFile = false;
	else
		gWriteToFile = true;

	gWaveFmt = pWaveFmt;			

	if (pbMp3)
	{
		// if it's mP3 , force to use the 44100, stereo and 16 bit
		//gWaveFmt.nChannels = 2;
		gWaveFmt.nSamplesPerSec = 44100;
		gWaveFmt.wBitsPerSample = 16;
        //gWaveFmt.wBitrate = 128;//320;

		MP3Init(gWaveFmt.nChannels, gdwInBufSize, gdwOutBufSize, gMp3Buffer, gWaveFmt);

		gMp3Buffer = new char[gdwOutBufSize];

		if (gMp3Buffer == NULL)
			return 1;                // memory error

		WAVE_BUFSIZE = TOTAL_WAVE_BUFSIZE;
		gMP3Offset = 0;

		if (WAVE_BUFSIZE>TOTAL_WAVE_BUFSIZE)
			WAVE_BUFSIZE = TOTAL_WAVE_BUFSIZE;

		MP3Done = CreateEvent(0, FALSE, FALSE, 0);
		ResetEvent(MP3Done);
	}
	else
		WAVE_BUFSIZE = TOTAL_WAVE_BUFSIZE;

	recordDone = CreateEvent(0, FALSE, FALSE, 0);
	MP3Ready = CreateEvent(0, FALSE, FALSE, 0);
	ResetEvent(recordDone);
	SetEvent(MP3Ready);
	InitPCMWaveFmt(gWaveFmt);
	m_pMainFrame->SetWavParameters(gWaveFmt.nChannels, gWaveFmt.nSamplesPerSec, gWaveFmt.wBitsPerSample);   //10.08.19
	RecordStoped = false;
	RecordPaused = false;
	if (gWriteToFile)
	{
		if (pbMp3)
		{
			gMp3File = _tfopen(pFileName, _T("wb"));		// MP3 File
			if (gMp3File == NULL)
				return 2;//file error
			gMp3NewThreadBuf = NULL;
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DumpMP3Data, NULL, 0, &threadid);
		}
		//else
		//{
		//	if (CerateWaveFileHeader(pFileName))		// Wave File return !=0 error
		//		return 2;
		//}
	}

	int vRc = DShowopenWaveDev2(pSrcFileName, pFileName, gWaveFmt,  pbFirstCall, pbIsPreview, vnTrackNr);
	if (vRc == 0)
	{
		ResetEvent(recordDone);
		RecordHandle = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)Recorder, NULL, 0,
			&threadid);
		ResetEvent(recordDone);
		return 0;
	}
	else if (vRc == 1)
		return 6;  // WaveInOpen Fail
	else return 7;// other fail
}

int CWaveRecorder::DShowopenWaveDev2(LPCTSTR pSrcFileName, LPCTSTR pFileName, WaveFmt pWaveFmt,bool pbFirstCall, bool pbIsPreview, int pnTrackNr)
{
	MMRESULT Rc;

	bool vbIsAudioOnly;

	dwTotalwavesize = dwBytedatasize = bufindex = 0;

	InitPCMWaveFmt(pWaveFmt);

	WaveHeaderRec[0].dwBufferLength = WAVE_BUFSIZE;
	WaveHeaderRec[0].lpData = (LPSTR)(gszInbuf1);
	WaveHeaderRec[0].dwFlags = WaveHeaderRec[0].reserved = 0;
	WaveHeaderRec[0].dwLoops = 0;
	WaveHeaderRec[0].lpNext = 0;

	WaveHeaderRec[1].dwBufferLength = WAVE_BUFSIZE;
	WaveHeaderRec[1].lpData = (LPSTR)(gszInbuf2);
	WaveHeaderRec[1].dwFlags = WaveHeaderRec[1].reserved = 0;
	WaveHeaderRec[1].dwLoops = 0;
	WaveHeaderRec[1].lpNext = 0;

	WaveHeaderRec[2].dwBufferLength = WAVE_BUFSIZE;
	WaveHeaderRec[2].lpData = (LPSTR)(gszInbuf3);
	WaveHeaderRec[2].dwFlags = WaveHeaderRec[2].reserved = 0;
	WaveHeaderRec[2].dwLoops = 0;
	WaveHeaderRec[2].lpNext = 0;

	pwavemem[0] = (LPSTR)(gszInbuf1);
	pwavemem[1] = (LPSTR)(gszInbuf2);
	pwavemem[2] = (LPSTR)(gszInbuf3);


	ResetEvent(recordDone);
	DShowAddNextBuffer();

	DShowAddNextBuffer();

	return 0;

}

void CWaveRecorder::InitPCMWaveFmt(WaveFmt pWaveFmt)
{
	ZeroMemory(&PCMWaveFmtRecord, sizeof(WAVEFORMATEX));

	PCMWaveFmtRecord.wFormatTag = WAVE_FORMAT_PCM;
	PCMWaveFmtRecord.nChannels = pWaveFmt.nChannels;//1;
	PCMWaveFmtRecord.nSamplesPerSec = pWaveFmt.nSamplesPerSec;//8000;
	PCMWaveFmtRecord.wBitsPerSample = pWaveFmt.wBitsPerSample;//8;
	PCMWaveFmtRecord.nBlockAlign = PCMWaveFmtRecord.nChannels *	PCMWaveFmtRecord.wBitsPerSample / 8;
	PCMWaveFmtRecord.nAvgBytesPerSec = PCMWaveFmtRecord.nSamplesPerSec*PCMWaveFmtRecord.nBlockAlign;
	PCMWaveFmtRecord.cbSize = 0;

}


DWORD WINAPI CWaveRecorder::Recorder(LPVOID *p)
{
	return thisobj->RecorderInThread(p);
}


DWORD CWaveRecorder::RecorderInThread(LPVOID *p)
{
	int i; // number of BUFSIZ buffers to record before reaching MAXBUF
	BOOL success;
	DWORD numWrite, EventResult;
	EventResult = WaitForSingleObject(recordDone, INFINITE);
	i = 0;
	switch (EventResult)
	{
	case WAIT_ABANDONED:
		i = 1;
		break;
	case WAIT_OBJECT_0:
		i = 2;
		break;
	case WAIT_TIMEOUT:
		i = 3;
		break;
	case WAIT_FAILED:
		i = 4;
		break;
	}

	if (gWriteToFile)
	{
		if (gbMp3)
		{
			Sleep(100);
			if (!Mp3WriteOnce)
				MP3EncodeBuf(gMp3File, pwavemem[0], gMp3Buffer, gdwInBufSize, gdwOutBufSize);

			MP3EncodeClose(gMp3File, gMp3Buffer);
		}
		//else
		//	CloseWaveFile();
	}


	if (gMp3Buffer != NULL)
	{
		delete[]gMp3Buffer;
		gMp3Buffer = NULL;
	}

	ResetEvent(recordDone);
	CloseHandle(recordDone);


	return 0;
}



//int CWaveRecorder::CloseWaveFile()
//{
//
//	WriteWavHeader();
//	fclose(WavLi);
//	WavLi = NULL;
//
//	return 0;
//
//}


