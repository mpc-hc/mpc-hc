
#include "stdafx.h"
#include "AudioParser.h"

int ParseTrueHDHeader(const BYTE *buf, int *samplerate, int *channels, int *framelength)
{
	static const int sampling_rates[]  = { 48000, 96000, 192000, 0, 0, 0, 0, 0, 44100, 88200, 176400, 0, 0, 0, 0, 0 };
	static const int channel_count[13] = {//   LR    C   LFE  LRs LRvh  LRc LRrs  Cs   Ts  LRsd  LRw  Cvh  LFE2
												2,   1,   1,   2,   2,   2,   2,   1,   1,   2,   2,   1,   1 };
	
	DWORD sync = *(DWORD*)(buf+4);
	if (sync != TRUEHD_SYNC_WORD)  // syncword
		return 0;
	int frame_size  = (((buf[0] << 8) | buf[1]) & 0xfff) * 2;
	
	*samplerate             = sampling_rates[buf[8] >> 4];
	*framelength            = 40 << ((buf[8] >> 4) & 0x07);
	int chanmap_substream_1 = ((buf[ 9] & 0x0f) << 1) | (buf[10] >> 7);
	int chanmap_substream_2 = ((buf[10] & 0x1f) << 8) |  buf[11];
	int channel_map         = chanmap_substream_2 ? chanmap_substream_2 : chanmap_substream_1;
	*channels = 0;
	for (int i = 0; i < 13; ++i)
		*channels += channel_count[i] * ((channel_map >> i) & 1);

	return frame_size;
}
