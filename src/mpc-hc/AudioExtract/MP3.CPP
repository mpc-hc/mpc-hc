/*

*  
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
#include "MP3Enc.h"
#include "MP3.h"

HBE_STREAM hStream;
int SamplesPerformed=0;
char *Mp3Buffer1;
char *vgszInstallPath;

static HINSTANCE hinstDLL; 



int MP3Init(int pChannel,DWORD &pInBufSize,DWORD &pOutBufSize,char *pMp3Buffer,WaveFmt pWaveFmt)
{

	DWORD dwNumberOfSamples;
	DWORD dwOutputBufferLength;

	pInBufSize  = 0;
	pOutBufSize = 0;	
	BE_CONFIG beConfig;
	BE_ERR		err				=0;
	memset(&beConfig,0,sizeof(beConfig));					// clear all fields

	beConfig.dwConfig = BE_CONFIG_MP3;
	beConfig.format.mp3.dwSampleRate		= pWaveFmt.nSamplesPerSec;//44100;				// INPUT FREQUENCY
	if (pChannel==2)
		beConfig.format.mp3.byMode			= BE_MP3_MODE_STEREO;	// OUTPUT IN STREO
	else if (pChannel==1)
		beConfig.format.mp3.byMode			= BE_MP3_MODE_MONO;	// OUTPUT IN Mono
	else
		beConfig.format.mp3.byMode			= BE_MP3_MODE_DUALCHANNEL;	// OUTPUT IN DualChannel

	beConfig.format.mp3.wBitrate			= pWaveFmt.wBitrate;//192;					// MINIMUM BIT RATE
	beConfig.format.mp3.bOriginal			= false;					// SET ORIGINAL FLAG
	beConfig.format.mp3.bCRC				= false;					// INSERT CRC
	beConfig.format.mp3.bCopyright			= false;
	beConfig.format.mp3.bPrivate			= false;



	// Init the MP3 Stream
	err = beInitStream(&beConfig, &dwNumberOfSamples,&dwOutputBufferLength,&hStream);

	// Check result
	if(err != BE_ERR_SUCCESSFUL)
	{
	//	fprintf(stderr,"Error opening encoding stream (%lu)", err);
		return 0;
	}
  pInBufSize  = dwNumberOfSamples;
  pOutBufSize = dwOutputBufferLength;
 // pMp3Buffer=new char[pOutBufSize];
  return 1;


}


DWORD MP3EncodeBuf(FILE *li,char *pInBuf,char *pOutBuf,DWORD pdwInBufSize,DWORD pdwOutBufSize)
{
  // initialization of Mp3 encoder
 
  SHORT *InputBuffer=(SHORT*)pInBuf;      //SHORT=short=16 bits

  Mp3Buffer1=pOutBuf;
  DWORD dwNumberOfSamplesEncoded;
  SamplesPerformed+=pdwInBufSize;
  if(beEncodeChunk(hStream,pdwInBufSize,InputBuffer,
        (BYTE*)Mp3Buffer1,&dwNumberOfSamplesEncoded)!=BE_ERR_SUCCESSFUL)
    {

      return 0;
    }
    fwrite(Mp3Buffer1,sizeof(char),dwNumberOfSamplesEncoded,li);
  
  return dwNumberOfSamplesEncoded;//return 1;
}


int MP3EncodeClose(FILE *li,char *pMp3Buffer)
{
  DWORD dwNumberOfSamplesEncoded;
  if (pMp3Buffer!=NULL)
	beDeinitStream(hStream,(BYTE*)Mp3Buffer1,&dwNumberOfSamplesEncoded);
  beCloseStream(hStream);

  fclose(li);
  return 1;
}




