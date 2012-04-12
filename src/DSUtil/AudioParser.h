
#pragma once

#include <WinDef.h>

#define RIFF_DWORD          0x46464952

#define AC3_SYNC_WORD           0x770b
#define TRUEHD_SYNC_WORD    0xba6f72f8
#define MLP_SYNC_WORD       0xbb6f72f8
#define IEC61937_SYNC_WORD  0x4e1ff872

#define DTS_SYNC_WORD       0x0180fe7f
#define DTSHD_SYNC_WORD     0x25205864

#define EAC3_FRAME_TYPE_INDEPENDENT  0
#define EAC3_FRAME_TYPE_DEPENDENT    1
#define EAC3_FRAME_TYPE_AC3_CONVERT  2
#define EAC3_FRAME_TYPE_RESERVED     3

int GetAC3FrameSize(const BYTE *buf); // for AC3 and E-AC3
int GetMLPFrameSize(const BYTE *buf); // for TrueHD and MLP

int ParseAC3Header     (const BYTE *buf, int *samplerate, int *channels, int *framelength, int *bitrate);
int ParseEAC3Header    (const BYTE *buf, int *samplerate, int *channels, int *framelength, int *frametype);
int ParseTrueHDHeader  (const BYTE *buf, int *samplerate, int *channels, int *framelength);
int ParseHdmvLPCMHeader(const BYTE *buf, int *samplerate, int *channels);
