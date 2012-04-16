
#include "stdafx.h"
#include "AudioParser.h"

#define AC3_CHANNEL                  0
#define AC3_MONO                     1
#define AC3_STEREO                   2
#define AC3_3F                       3
#define AC3_2F1R                     4
#define AC3_3F1R                     5
#define AC3_2F2R                     6
#define AC3_3F2R                     7
#define AC3_CHANNEL1                 8
#define AC3_CHANNEL2                 9
#define AC3_DOLBY                   10
#define AC3_CHANNEL_MASK            15
#define AC3_LFE                     16

int GetAC3FrameSize(const BYTE *buf)
{
	if (*(WORD*)buf != AC3_SYNC_WORD) // syncword
		return 0;

	int frame_size;

	if (buf[5] >> 3 <= 10) {   // Normal AC-3
		static const int rates[] = {32,  40,  48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, 448, 512, 576, 640};

		int frmsizecod = buf[4] & 0x3F;
		if (frmsizecod >= 38)
			return 0;

		int rate  = rates[frmsizecod >> 1];
		switch (buf[4] & 0xc0) {
			case 0:
				frame_size = 4 * rate;
				break;
			case 0x40:
				frame_size = 2 * (320 * rate / 147 + (frmsizecod & 1));
				break;
			case 0x80:
				frame_size = 6 * rate;
				break;
			default:
				return 0;
		}
	} else {   /// Enhanced AC-3
		frame_size = (((buf[2] & 0x03) << 8) + buf[3] + 1) * 2;
	}
	return frame_size;
}

int GetMLPFrameSize(const BYTE *buf)
{
	DWORD sync = *(DWORD*)(buf+4);
	if (sync == TRUEHD_SYNC_WORD || sync == MLP_SYNC_WORD) {
		return (((buf[0] << 8) | buf[1]) & 0xfff) * 2;
	}
	return 0;
}


int ParseAC3Header(const BYTE *buf, int *samplerate, int *channels, int *framelength, int *bitrate)
{
	if (*(WORD*)buf != AC3_SYNC_WORD) // syncword
		return 0;

	if (buf[5] >> 3 >= 11)   // bsid
		return 0;

	static const int rates[] = {32,  40,  48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, 448, 512, 576, 640};
	static const unsigned char lfeon[8] = {0x10, 0x10, 0x04, 0x04, 0x04, 0x01, 0x04, 0x01};
	static const unsigned char halfrate[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3};

	int frmsizecod = buf[4] & 0x3F;
	if (frmsizecod >= 38)
		return 0;

	int half = halfrate[buf[5] >> 3];
	int rate  = rates[frmsizecod >> 1];
	*bitrate  = (rate * 1000) >> half;
	int frame_size;
	switch (buf[4] & 0xc0) {
		case 0:
			*samplerate = 48000 >> half;
			frame_size  = 4 * rate;
			break;
		case 0x40:
			*samplerate = 44100 >> half;
			frame_size  = 2 * (320 * rate / 147 + (frmsizecod & 1));
			break;
		case 0x80:
			*samplerate = 32000 >> half;
			frame_size  = 6 * rate;
			break;
		default:
			return 0;
	}

	unsigned char acmod    = buf[6] >> 5;
	unsigned char flags = ((((buf[6] & 0xf8) == 0x50) ? AC3_DOLBY : acmod) | ((buf[6] & lfeon[acmod]) ? AC3_LFE : 0));
	switch (flags & AC3_CHANNEL_MASK) {
		case AC3_MONO:
			*channels = 1;
			break;
		case AC3_CHANNEL:
		case AC3_STEREO:
		case AC3_CHANNEL1:
		case AC3_CHANNEL2:
		case AC3_DOLBY:
			*channels = 2;
			break;
		case AC3_2F1R:
		case AC3_3F:
			*channels = 3;
			break;
		case AC3_3F1R:
		case AC3_2F2R:
			*channels = 4;
			break;
		case AC3_3F2R:
			*channels = 5;
			break;
	}
	if (flags & AC3_LFE) (*channels)++;

	*framelength = 1536;
	return frame_size;
}

int ParseEAC3Header(const BYTE *buf, int *samplerate, int *channels, int *framelength, int *frametype)
{
	if (*(WORD*)buf != AC3_SYNC_WORD) // syncword
		return 0;

	if (buf[5] >> 3 <= 10)   // bsid
		return 0;

	static const int sample_rates[] = { 48000, 44100, 32000, 24000, 22050, 16000 };
	static const int channels_tbl[]     = { 2, 1, 2, 3, 3, 4, 4, 5 };
	static const int samples_tbl[]      = { 256, 512, 768, 1536 };

	int frame_size = (((buf[2] & 0x03) << 8) + buf[3] + 1) * 2;

	int fscod  =  buf[4] >> 6;
	int fscod2 = (buf[4] >> 4) & 0x03;

	if (fscod == 0x03 && fscod2 == 0x03)
		return 0;

	int acmod = (buf[4] >> 1) & 0x07;
	int lfeon =  buf[4] & 0x01;

	*frametype    = (buf[2] >> 6) & 0x03;
	if (*frametype == EAC3_FRAME_TYPE_RESERVED)
		return 0;
	//int sub_stream_id = (buf[2] >> 3) & 0x07;
	*samplerate       = sample_rates[fscod == 0x03 ? 3 + fscod2 : fscod];
	*channels         = channels_tbl[acmod] + lfeon;
	*framelength      = (fscod == 0x03) ? 1536 : samples_tbl[fscod2];

	return frame_size;
}

int ParseTrueHDHeader(const BYTE *buf, int *samplerate, int *channels, int *framelength)
{
	static const int sampling_rates[]  = { 48000, 96000, 192000, 0, 0, 0, 0, 0, 44100, 88200, 176400, 0, 0, 0, 0, 0 };
	static const int channel_count[13] = {//   LR    C   LFE  LRs LRvh  LRc LRrs  Cs   Ts  LRsd  LRw  Cvh  LFE2
		2,   1,   1,   2,   2,   2,   2,   1,   1,   2,   2,   1,   1
	};

	DWORD sync = *(DWORD*)(buf+4);
	if (sync != TRUEHD_SYNC_WORD) {
		return 0;
	}

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

int ParseHdmvLPCMHeader(const BYTE *buf, int *samplerate, int *channels)
{
	*samplerate = 0;
	*channels   = 0;

	int frame_size = buf[0] << 8 | buf[1];
	frame_size += 4; // add header size;

	static int channels_layout[] = {0, 1, 0, 2, 3, 3, 4, 4, 5, 6, 7, 8, 0, 0, 0, 0};
	BYTE channel_layout = buf[2] >> 4;
	*channels           = channels_layout[channel_layout];
	if (!*channels) {
		return 0;
	}

	static int bitspersample[] = {0, 16, 20, 24};
	int bits_per_sample = bitspersample[buf[3] >> 6];
	if (!(bits_per_sample == 16 || bits_per_sample == 24)) {
		return 0;
	}

	static int freq[] = {0, 48000, 0, 0, 96000, 192000};
	*samplerate = freq[buf[2] & 0x0f];
	if (!(*samplerate == 48000 || *samplerate == 96000 || *samplerate == 192000)) {
		return 0;
	}

	return frame_size;
}
