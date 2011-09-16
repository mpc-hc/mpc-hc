
#pragma once

#include <WinDef.h>

#define RIFF_DWORD          0x46464952
#define AC3_SYNC_WORD           0x770b
#define TRUEHD_SYNC_WORD    0xba6f72f8
#define MLP_SYNC_WORD       0xbb6f72f8
#define IEC61937_SYNC_WORD  0x4e1ff872

int ParseTrueHDHeader(const BYTE *buf, int *samplerate, int *channels, int *framelength);
