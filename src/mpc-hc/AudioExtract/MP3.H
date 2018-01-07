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
#ifndef _MP3_H_
#define _MP3_H_

#include "AudioRecord.h"
#include <string>
typedef struct _tagMP3
{
  char m_szArtist[31];
  char m_szTitle[31];
  char m_szAlbum[31];
  char m_szYear[5];
  BYTE m_bGenre;
  // init info
  WORD m_wBitrate;
  int m_nMode;
  BOOL m_bCRC;
  BOOL m_bCopyright;
  BOOL m_bPrivate;
  BOOL m_bOriginal;
  BOOL m_bID3;
  int m_iID3Level;
  BYTE m_trackNum;
} MP3TAG;

//bool LoadMP3DLL();

int MP3EncodeClose(FILE *li,char *pMp3Buffer);
DWORD MP3EncodeBuf(FILE *li,char *pInBuf,char *pOutBuf,DWORD pdwInBufSize,DWORD pdwOutBufSize);

int MP3Init(int pChannel,DWORD &pInBufSize,DWORD &pOutBufSize,char *pMp3Buffer,WaveFmt pWaveFmt);

#endif