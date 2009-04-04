/*
 * Based on ttadec.c
 *
 * Description:	 TTAv1 decoder library for HW players
 * Developed by: Alexander Djourik <sasha@iszf.irk.ru>
 *               Pavel Zhilin <pzh@iszf.irk.ru>
 *
 * Copyright (c) 2004 True Audio Software. All rights reserved.
 *
 */

/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the True Audio Software nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "avcodec.h"

#define ISO_BUFFER_LENGTH       (1024*32)
#define ISO_NBUFFERS            (8)
#define ISO_BUFFERS_SIZE        (ISO_BUFFER_LENGTH*ISO_NBUFFERS)
#define PCM_BUFFER_LENGTH       (4608)

#define FRAME_TIME	1.04489795918367346939

#define MAX_ORDER	8
#define MAX_NCH         8	// Max supported number of channels

typedef struct {
	int shift;
	int round;
	int error;
	int mutex;
	int qm[MAX_ORDER+1];
	int dx[MAX_ORDER+1];
	int dl[MAX_ORDER+1];
} fltst;

typedef struct {
	unsigned int k0;
	unsigned int k1;
	unsigned int sum0;
	unsigned int sum1;
} adapt;

typedef struct {
	fltst fst;
	adapt rice;
	int last;
} decoder;

typedef struct
{
 int data_cur;
 int framelen,FRAMELEN;

 int frame_crc32;
 unsigned char isobuffers[ISO_BUFFERS_SIZE + 4];
 unsigned char *iso_buffers_end;

 unsigned int data_pos;

 unsigned int bit_count;
 unsigned int bit_cache;
 unsigned char *bitpos;

#define READ_ERROR      5	// Can't read from file
 int STATE;

 decoder tta[MAX_NCH];	// decoder state
 int cache[MAX_NCH];

 int maxvalue;		// output data max value
 int pcm_buffer_size;

 uint8_t *src;size_t srcsize;size_t readsize;
} TTAstate;

static int read(TTAstate *s,void *buf, size_t size1, size_t size2)
{
 if (s->srcsize==0) return 0;
 s->readsize=FFMIN(s->srcsize,size1*size2);
 memcpy(buf,s->src,s->readsize);
 s->iso_buffers_end=s->isobuffers+s->readsize;
 s->src+=s->readsize;s->srcsize-=s->readsize;
 return s->readsize;
}

static void init_buffer_read(TTAstate *s) {
	s->frame_crc32 = 0xFFFFFFFFUL;
	s->bit_count = s->bit_cache = 0;
	s->bitpos = s->iso_buffers_end;
}

static int tta_decode_init(AVCodecContext * avctx)
{
    TTAstate *s = avctx->priv_data;

    memset(s,0,sizeof(TTAstate));
    s->iso_buffers_end = s->isobuffers + ISO_BUFFERS_SIZE;
    s->maxvalue = (1UL << avctx->bits_per_coded_sample) - 1;
    s->pcm_buffer_size = PCM_BUFFER_LENGTH * (avctx->bits_per_coded_sample>>3) * avctx->channels;
    s->FRAMELEN=(int) (FRAME_TIME * avctx->sample_rate);
    init_buffer_read(s);

    return 0;
}

#ifdef WORDS_BIGENDIAN
#define	ENDSWAP_INT16(x)	(((((x)>>8)&0xFF)|(((x)&0xFF)<<8)))
#define	ENDSWAP_INT32(x)	(((((x)>>24)&0xFF)|(((x)>>8)&0xFF00)|(((x)&0xFF00)<<8)|(((x)&0xFF)<<24)))
#define WRITE_BUFFER(x, bsize, out) { \
	if (bsize > 2) *out++ = (uint8_t)(*x >> 16); \
	if (bsize > 1) *out++ = (uint8_t)(*x >> 8); \
	*out++ = (uint8_t) *x; }
#else
#define	ENDSWAP_INT16(x)	(x)
#define	ENDSWAP_INT32(x)	(x)
#define WRITE_BUFFER(x, bsize, out) { \
	*out++ = (uint8_t) *x; \
	if (bsize > 1) *out++ = (uint8_t)(*x >> 8); \
	if (bsize > 2) *out++ = (uint8_t)(*x >> 16); }
#endif

#define UPDATE_CRC32(x, crc) crc = \
	(((crc>>8) & 0x00FFFFFF) ^ crc32_table[(crc^x) & 0xFF])

static int done_buffer_read(TTAstate *s) {
	unsigned int crc32, rbytes;

	s->frame_crc32 ^= 0xFFFFFFFFUL;
	rbytes = s->iso_buffers_end - s->bitpos;

	if (rbytes < sizeof(int)) {
	    memcpy(s->isobuffers, s->bitpos, 4);
	    if (!read(s,s->isobuffers + rbytes, 1, ISO_BUFFERS_SIZE - rbytes))
		return -1;
	    s->bitpos = s->isobuffers;
	}

	memcpy(&crc32, s->bitpos, 4);
	crc32 = ENDSWAP_INT32(crc32);
	s->bitpos += sizeof(int);

	if (crc32 != s->frame_crc32)
	 return -1;

	s->bit_cache = s->bit_count = 0;
	s->frame_crc32 = 0xFFFFFFFFUL;

	return 0;
}

static const unsigned int crc32_table[256] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
	0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
	0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
	0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
	0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
	0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
	0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
	0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
	0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
	0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
	0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
	0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
	0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
	0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
	0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
	0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
	0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
	0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
	0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
	0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

static const unsigned int bit_shift[] = {
	0x00000001, 0x00000002, 0x00000004, 0x00000008,
	0x00000010, 0x00000020, 0x00000040, 0x00000080,
	0x00000100, 0x00000200, 0x00000400, 0x00000800,
	0x00001000, 0x00002000, 0x00004000, 0x00008000,
	0x00010000, 0x00020000, 0x00040000, 0x00080000,
	0x00100000, 0x00200000, 0x00400000, 0x00800000,
	0x01000000, 0x02000000, 0x04000000, 0x08000000,
	0x10000000, 0x20000000, 0x40000000, 0x80000000,
	0x80000000, 0x80000000, 0x80000000, 0x80000000,
	0x80000000, 0x80000000, 0x80000000, 0x80000000
};

static const unsigned int bit_mask[] = {
	0x00000000, 0x00000001, 0x00000003, 0x00000007,
	0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
	0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
	0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
	0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff,
	0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
	0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff,
	0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff,
	0xffffffff
};

static const unsigned int *shift_16 = bit_shift + 4;

static void rice_init(adapt *rice, unsigned int k0, unsigned int k1) {
	rice->k0 = k0;
	rice->k1 = k1;
	rice->sum0 = shift_16[k0];
	rice->sum1 = shift_16[k1];
}

static const int flt_set[3] = {10, 9, 10};

static void filter_init (fltst *fs, int shift) {
	memset (fs, 0, sizeof(fltst));
	fs->shift = shift;
	fs->round = 1 << (shift - 1);
}

static __inline void memshl (register int *pA, register int *pB) {
	*pA++ = *pB++;
	*pA++ = *pB++;
	*pA++ = *pB++;
	*pA++ = *pB++;
	*pA++ = *pB++;
	*pA++ = *pB++;
	*pA++ = *pB++;
	*pA   = *pB;
}

static __inline void hybrid_filter (fltst *fs, int *in) {
	register int *pA = fs->dl;
	register int *pB = fs->qm;
	register int *pM = fs->dx;
	register int sum = fs->round;

	if (!fs->error) {
		sum += *pA++ * *pB, pB++;
		sum += *pA++ * *pB, pB++;
		sum += *pA++ * *pB, pB++;
		sum += *pA++ * *pB, pB++;
		sum += *pA++ * *pB, pB++;
		sum += *pA++ * *pB, pB++;
		sum += *pA++ * *pB, pB++;
		sum += *pA++ * *pB, pB++; pM += 8;
	} else if (fs->error < 0) {
		sum += *pA++ * (*pB -= *pM++), pB++;
		sum += *pA++ * (*pB -= *pM++), pB++;
		sum += *pA++ * (*pB -= *pM++), pB++;
		sum += *pA++ * (*pB -= *pM++), pB++;
		sum += *pA++ * (*pB -= *pM++), pB++;
		sum += *pA++ * (*pB -= *pM++), pB++;
		sum += *pA++ * (*pB -= *pM++), pB++;
		sum += *pA++ * (*pB -= *pM++), pB++;
	} else {
		sum += *pA++ * (*pB += *pM++), pB++;
		sum += *pA++ * (*pB += *pM++), pB++;
		sum += *pA++ * (*pB += *pM++), pB++;
		sum += *pA++ * (*pB += *pM++), pB++;
		sum += *pA++ * (*pB += *pM++), pB++;
		sum += *pA++ * (*pB += *pM++), pB++;
		sum += *pA++ * (*pB += *pM++), pB++;
		sum += *pA++ * (*pB += *pM++), pB++;
	}

	*(pM-0) = ((*(pA-1) >> 30) | 1) << 2;
	*(pM-1) = ((*(pA-2) >> 30) | 1) << 1;
	*(pM-2) = ((*(pA-3) >> 30) | 1) << 1;
	*(pM-3) = ((*(pA-4) >> 30) | 1);

	fs->error = *in;
	*in += (sum >> fs->shift);
	*pA = *in;

	*(pA-1) = *(pA-0) - *(pA-1);
	*(pA-2) = *(pA-1) - *(pA-2);
	*(pA-3) = *(pA-2) - *(pA-3);

	memshl (fs->dl, fs->dl + 1);
	memshl (fs->dx, fs->dx + 1);
}


static void decoder_init(decoder *tta, int nch, int byte_size) {
	int shift = flt_set[byte_size - 1];
	int i;

	for (i = 0; i < nch; i++) {
		filter_init(&tta[i].fst, shift);
		rice_init(&tta[i].rice, 10, 10);
		tta[i].last = 0;
	}
}

static int set_position (TTAstate *s,unsigned int pos) {
/*
	unsigned int seek_pos;
	if (pos >= fframes) return 0;
	if (!st_state) {
		ttainfo->STATE = FILE_ERROR;
		return -1;
	}

	seek_pos = ttainfo->DATAPOS + seek_table[data_pos = pos];
	if (fseek(ttainfo->HANDLE, seek_pos, SEEK_SET) < 0) {
		ttainfo->STATE = READ_ERROR;
		return -1;
	}
*/
	s->data_cur = 0;
	s->framelen = 0;

	// init bit reader
	init_buffer_read(s);

	return 0;
}

#define PREDICTOR1(x, k)	((int)((((uint64_t)x << k) - x) >> k))
#define DEC(x)			(((x)&1)?(++(x)>>1):(-(x)>>1))

static int tta_decode_frame(AVCodecContext *avctx,
			    void *data, int *data_size,
			    const uint8_t *buf, int buf_size)
{
    TTAstate *s = avctx->priv_data;
    unsigned int k, depth, unary, binary;
    uint8_t *p = data;
    int value, res;
    int *prev = s->cache;
    decoder *dec = s->tta;

    s->src=buf;s->srcsize=buf_size;s->readsize=0;

	for (res = 0; p < (uint8_t*)data + s->pcm_buffer_size;) {
		fltst *fst = &dec->fst;
		adapt *rice = &dec->rice;
		int *last = &dec->last;

		if (s->data_cur == s->framelen) {
			//if (data_pos == fframes) break;
			if (s->framelen && done_buffer_read(s)) {
			    if (set_position(s,s->data_pos))
			     return -1;
			    if (res)
			     break;
			}

			/*if (s->data_pos == fframes - 1 && lastlen)
				framelen = lastlen;
			else*/ s->framelen = s->FRAMELEN;

			decoder_init(s->tta, avctx->channels, avctx->bits_per_coded_sample>>3);
			s->data_pos++; s->data_cur = 0;
		}

		// decode Rice unsigned
		//GET_UNARY(unary);
	        unary = 0;
	        while (!(s->bit_cache ^ bit_mask[s->bit_count])) {
		        if (s->bitpos == s->iso_buffers_end) {
			        if (!read(s,s->isobuffers, 1, ISO_BUFFERS_SIZE)) {
			            s->STATE = READ_ERROR;
			            return -1; }
			        s->bitpos = s->isobuffers; }
		        unary += s->bit_count;
		        s->bit_cache = *s->bitpos++;
		        UPDATE_CRC32(s->bit_cache, s->frame_crc32);
		        s->bit_count = 8; }
	        while (s->bit_cache & 1) {
		        unary++;
		        s->bit_cache >>= 1;
		        s->bit_count--; }
	        s->bit_cache >>= 1;
	        s->bit_count--;

		switch (unary) {
		case 0: depth = 0; k = rice->k0; break;
		default:
			depth = 1; k = rice->k1;
			unary--;
		}

		if (k) {
//			GET_BINARY(binary, k);
	                while (s->bit_count < k) {
		                if (s->bitpos == s->iso_buffers_end) {
			                if (!read(s,s->isobuffers, 1, ISO_BUFFERS_SIZE)) {
			                    s->STATE = READ_ERROR;
			                    return -1; }
			                s->bitpos = s->isobuffers; }
		                UPDATE_CRC32(*s->bitpos, s->frame_crc32);
		                s->bit_cache |= *s->bitpos << s->bit_count;
		                s->bit_count += 8;
		                s->bitpos++; }
	                binary = s->bit_cache & bit_mask[k];
	                s->bit_cache >>= k;
	                s->bit_count -= k;
	                s->bit_cache &= bit_mask[s->bit_count];

			value = (unary << k) + binary;
		} else value = unary;

		switch (depth) {
		case 1:
			rice->sum1 += value - (rice->sum1 >> 4);
			if (rice->k1 > 0 && rice->sum1 < shift_16[rice->k1])
				rice->k1--;
			else if (rice->sum1 > shift_16[rice->k1 + 1])
				rice->k1++;
			value += bit_shift[rice->k0];
		default:
			rice->sum0 += value - (rice->sum0 >> 4);
			if (rice->k0 > 0 && rice->sum0 < shift_16[rice->k0])
				rice->k0--;
			else if (rice->sum0 > shift_16[rice->k0 + 1])
			rice->k0++;
		}

		value = DEC(value);

		// decompress stage 1: adaptive hybrid filter
		hybrid_filter(fst, &value);

		// decompress stage 2: fixed order 1 prediction
		switch (avctx->bits_per_coded_sample>>3) {
		case 1: value += PREDICTOR1(*last, 4); break;	// bps 8
		case 2: value += PREDICTOR1(*last, 5); break;	// bps 16
		case 3: value += PREDICTOR1(*last, 5); break;	// bps 24
		} *last = value;

		// check for errors
		if (FFABS(value) > s->maxvalue) {
			unsigned int tail =
				s->pcm_buffer_size / ((avctx->bits_per_coded_sample>>3)* avctx->channels) - res;
			memset(data, 0, s->pcm_buffer_size);
			s->data_cur += tail; res += tail;
			break;
		}

		if (dec < s->tta + (avctx->channels - 1)) {
			*prev++ = value; dec++;
		} else {
			*prev = value;
			if (avctx->channels > 1) {
				int *r = prev - 1;
				for (*prev += *r/2; r >= s->cache; r--)
					*r = *(r + 1) - *r;
				for (r = s->cache; r < prev; r++)
					WRITE_BUFFER(r, (avctx->bits_per_coded_sample>>3), p)
			}
			WRITE_BUFFER(prev, (avctx->bits_per_coded_sample>>3), p)
			prev = s->cache;
			s->data_cur++; res++;
			dec = s->tta;
		}
        }
    *data_size=res*avctx->channels*(avctx->bits_per_coded_sample>>3);
    return s->readsize; //buf_size-s->srcsize;
}

static void tta_flush(AVCodecContext *avctx)
{
    TTAstate *s = avctx->priv_data;
    set_position(s,0);
}

AVCodec tta_decoder = {
    "tta",
    CODEC_TYPE_AUDIO,
    CODEC_ID_TTA,
    sizeof(TTAstate),
    /*.init = */tta_decode_init,
    /*.encode = */NULL,
    /*.close = */NULL,
    /*.decode = */tta_decode_frame,
    /*.capabilities = */0,
    /*.next = */NULL,
    /*.flush = */tta_flush,
    /*.supported_framerates = */NULL,
    /*.pix_fmts = */NULL,
    /*.long_name = */"True Audio",
};
