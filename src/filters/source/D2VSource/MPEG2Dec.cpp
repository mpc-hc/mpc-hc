#include "stdafx.h"
#include "MPEG2Dec.h"

int testint;

struct CPU {
	BOOL					mmx;
	BOOL					_3dnow;
	BOOL					ssemmx;
	BOOL					ssefpu;
} cpu;

void CheckCPU()
{
	__asm
	{
		mov			eax, 1
		cpuid
		test		edx, 0x00800000		// STD MMX
		jz			TEST_SSE
		mov			[cpu.mmx], 1
TEST_SSE:
		test		edx, 0x02000000		// STD SSE
		jz			TEST_3DNOW
		mov			[cpu.ssemmx], 1
		mov			[cpu.ssefpu], 1
TEST_3DNOW:
		mov			eax, 0x80000001
		cpuid
		test		edx, 0x80000000		// 3D NOW
		jz			TEST_SSEMMX
		mov			[cpu._3dnow], 1
TEST_SSEMMX:
		test		edx, 0x00400000		// SSE MMX
		jz			TEST_END
		mov			[cpu.ssemmx], 1
TEST_END:
	}
}

#pragma warning(disable:4799)	// no EMMS
#pragma warning(disable:4731)	// ebp modified
// idct
extern "C" void __fastcall MMX_IDCT(short *block);
extern "C" void __fastcall SSEMMX_IDCT(short *block);
extern void Initialize_FPU_IDCT(void);
extern void FPU_IDCT(short *block);
extern void Initialize_REF_IDCT(void);
extern void REF_IDCT(short *block);

/* default intra quantization matrix */
static unsigned char default_intra_quantizer_matrix[64] =
{
	8, 16, 19, 22, 26, 27, 29, 34,
	16, 16, 22, 24, 27, 29, 34, 37,
	19, 22, 26, 27, 29, 34, 34, 38,
	22, 22, 26, 27, 29, 34, 37, 40,
	22, 26, 27, 29, 32, 35, 40, 48,
	26, 27, 29, 32, 35, 40, 48, 58,
	26, 27, 29, 34, 38, 46, 56, 69,
	27, 29, 35, 38, 46, 56, 69, 83
};

/* zig-zag and alternate scan patterns */
static unsigned char scan[2][64] =
{
	{ /* Zig-Zag scan pattern  */
		0,  1,  8, 16,  9,  2,  3, 10,
	   17, 24, 32, 25, 18, 11,  4,  5,
	   12, 19, 26, 33, 40, 48, 41, 34,
	   27, 20, 13,  6,  7, 14, 21, 28,
	   35, 42, 49, 56, 57, 50, 43, 36,
	   29, 22, 15, 23, 30, 37, 44, 51,
	   58, 59, 52, 45, 38, 31, 39, 46,
	   53, 60, 61, 54, 47, 55, 62, 63
	}
	,
	{ /* Alternate scan pattern */
		0,  8, 16, 24,  1,  9,  2, 10,
	   17, 25, 32, 40, 48, 56, 57, 49,
	   41, 33, 26, 18,  3, 11, 4,  12,
	   19, 27, 34, 42, 50, 58, 35, 43,
	   51, 59, 20, 28,  5, 13,  6, 14,
	   21, 29, 36, 44, 52, 60, 37, 45,
	   53, 61, 22, 30,  7, 15, 23, 31,
	   38, 46, 54, 62, 39, 47, 55, 63
	}
};

/* non-linear quantization coefficient table */
static unsigned char Non_Linear_quantizer_scale[32] =
{
	0, 1, 2, 3, 4, 5, 6, 7,
	8, 10, 12, 14, 16, 18, 20, 22,
	24, 28, 32, 36, 40, 44, 48, 52,
	56, 64, 72, 80, 88, 96, 104, 112
};

#define ERROR_VALUE	(-1)

typedef struct {
	char run, level, len;
} DCTtab;

typedef struct {
	char val, len;
} VLCtab;

/* Table B-10, motion_code, codes 0001 ... 01xx */
static VLCtab MVtab0[8] =
{
	{ERROR_VALUE,0}, {3,3}, {2,2}, {2,2}, {1,1}, {1,1}, {1,1}, {1,1}
};

/* Table B-10, motion_code, codes 0000011 ... 000011x */
static VLCtab MVtab1[8] =
{
	{ERROR_VALUE,0}, {ERROR_VALUE,0}, {ERROR_VALUE,0}, {7,6}, {6,6}, {5,6}, {4,5}, {4,5}
};

/* Table B-10, motion_code, codes 0000001100 ... 000001011x */
static VLCtab MVtab2[12] =
{
	{16,9}, {15,9}, {14,9}, {13,9},
	{12,9}, {11,9}, {10,8}, {10,8},
	{9,8},  {9,8},  {8,8},  {8,8}
};

/* Table B-9, coded_block_pattern, codes 01000 ... 111xx */
static VLCtab CBPtab0[32] =
{
	{ERROR_VALUE,0}, {ERROR_VALUE,0}, {ERROR_VALUE,0}, {ERROR_VALUE,0},
	{ERROR_VALUE,0}, {ERROR_VALUE,0}, {ERROR_VALUE,0}, {ERROR_VALUE,0},
	{62,5}, {2,5},  {61,5}, {1,5},  {56,5}, {52,5}, {44,5}, {28,5},
	{40,5}, {20,5}, {48,5}, {12,5}, {32,4}, {32,4}, {16,4}, {16,4},
	{8,4},  {8,4},  {4,4},  {4,4},  {60,3}, {60,3}, {60,3}, {60,3}
};

/* Table B-9, coded_block_pattern, codes 00000100 ... 001111xx */
static VLCtab CBPtab1[64] =
{
	{ERROR_VALUE,0}, {ERROR_VALUE,0}, {ERROR_VALUE,0}, {ERROR_VALUE,0},
	{58,8}, {54,8}, {46,8}, {30,8},
	{57,8}, {53,8}, {45,8}, {29,8}, {38,8}, {26,8}, {37,8}, {25,8},
	{43,8}, {23,8}, {51,8}, {15,8}, {42,8}, {22,8}, {50,8}, {14,8},
	{41,8}, {21,8}, {49,8}, {13,8}, {35,8}, {19,8}, {11,8}, {7,8},
	{34,7}, {34,7}, {18,7}, {18,7}, {10,7}, {10,7}, {6,7},  {6,7},
	{33,7}, {33,7}, {17,7}, {17,7}, {9,7},  {9,7},  {5,7},  {5,7},
	{63,6}, {63,6}, {63,6}, {63,6}, {3,6},  {3,6},  {3,6},  {3,6},
	{36,6}, {36,6}, {36,6}, {36,6}, {24,6}, {24,6}, {24,6}, {24,6}
};

/* Table B-9, coded_block_pattern, codes 000000001 ... 000000111 */
static VLCtab CBPtab2[8] =
{
	{ERROR_VALUE,0}, {0,9}, {39,9}, {27,9}, {59,9}, {55,9}, {47,9}, {31,9}
};

/* Table B-1, macroblock_address_increment, codes 00010 ... 011xx */
static VLCtab MBAtab1[16] =
{
	{ERROR_VALUE,0}, {ERROR_VALUE,0}, {7,5}, {6,5}, {5,4}, {5,4}, {4,4},
	{4,4}, {3,3}, {3,3}, {3,3}, {3,3}, {2,3}, {2,3}, {2,3}, {2,3}
};

/* Table B-1, macroblock_address_increment, codes 00000011000 ... 0000111xxxx */
static VLCtab MBAtab2[104] =
{
	{33,11}, {32,11}, {31,11}, {30,11}, {29,11}, {28,11}, {27,11}, {26,11},
	{25,11}, {24,11}, {23,11}, {22,11}, {21,10}, {21,10}, {20,10}, {20,10},
	{19,10}, {19,10}, {18,10}, {18,10}, {17,10}, {17,10}, {16,10}, {16,10},
	{15,8},  {15,8},  {15,8},  {15,8},  {15,8},  {15,8},  {15,8},  {15,8},
	{14,8},  {14,8},  {14,8},  {14,8},  {14,8},  {14,8},  {14,8},  {14,8},
	{13,8},  {13,8},  {13,8},  {13,8},  {13,8},  {13,8},  {13,8},  {13,8},
	{12,8},  {12,8},  {12,8},  {12,8},  {12,8},  {12,8},  {12,8},  {12,8},
	{11,8},  {11,8},  {11,8},  {11,8},  {11,8},  {11,8},  {11,8},  {11,8},
	{10,8},  {10,8},  {10,8},  {10,8},  {10,8},  {10,8},  {10,8},  {10,8},
	{9,7},   {9,7},   {9,7},   {9,7},   {9,7},   {9,7},   {9,7},   {9,7},
	{9,7},   {9,7},   {9,7},   {9,7},   {9,7},   {9,7},   {9,7},   {9,7},
	{8,7},   {8,7},   {8,7},   {8,7},   {8,7},   {8,7},   {8,7},   {8,7},
	{8,7},   {8,7},   {8,7},   {8,7},   {8,7},   {8,7},   {8,7},   {8,7}
};

/* Table B-12, dct_dc_size_luminance, codes 00xxx ... 11110 */
static VLCtab DClumtab0[32] =
{
	{1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2},
	{2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2},
	{0, 3}, {0, 3}, {0, 3}, {0, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 3},
	{4, 3}, {4, 3}, {4, 3}, {4, 3}, {5, 4}, {5, 4}, {6, 5}, {ERROR_VALUE, 0}
};

/* Table B-12, dct_dc_size_luminance, codes 111110xxx ... 111111111 */
static VLCtab DClumtab1[16] =
{
	{7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6},
	{8, 7}, {8, 7}, {8, 7}, {8, 7}, {9, 8}, {9, 8}, {10,9}, {11,9}
};

/* Table B-13, dct_dc_size_chrominance, codes 00xxx ... 11110 */
static VLCtab DCchromtab0[32] =
{
	{0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2},
	{1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2},
	{2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2},
	{3, 3}, {3, 3}, {3, 3}, {3, 3}, {4, 4}, {4, 4}, {5, 5}, {ERROR_VALUE, 0}
};

/* Table B-13, dct_dc_size_chrominance, codes 111110xxxx ... 1111111111 */
static VLCtab DCchromtab1[32] =
{
	{6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6},
	{6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6},
	{7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7},
	{8, 8}, {8, 8}, {8, 8}, {8, 8}, {9, 9}, {9, 9}, {10,10}, {11,10}
};

/* Table B-14, DCT coefficients table zero,
 * codes 0100 ... 1xxx (used for first (DC) coefficient)
 */
static DCTtab DCTtabfirst[12] =
{
	{0,2,4}, {2,1,4}, {1,1,3}, {1,1,3},
	{0,1,1}, {0,1,1}, {0,1,1}, {0,1,1},
	{0,1,1}, {0,1,1}, {0,1,1}, {0,1,1}
};

/* Table B-14, DCT coefficients table zero,
 * codes 0100 ... 1xxx (used for all other coefficients)
 */
static DCTtab DCTtabnext[12] =
{
	{0,2,4},  {2,1,4},  {1,1,3},  {1,1,3},
	{64,0,2}, {64,0,2}, {64,0,2}, {64,0,2}, /* EOB */
	{0,1,2},  {0,1,2},  {0,1,2},  {0,1,2}
};

/* Table B-14, DCT coefficients table zero,
 * codes 000001xx ... 00111xxx
 */
static DCTtab DCTtab0[60] =
{
	{65,0,6}, {65,0,6}, {65,0,6}, {65,0,6}, /* Escape */
	{2,2,7}, {2,2,7}, {9,1,7}, {9,1,7},
	{0,4,7}, {0,4,7}, {8,1,7}, {8,1,7},
	{7,1,6}, {7,1,6}, {7,1,6}, {7,1,6},
	{6,1,6}, {6,1,6}, {6,1,6}, {6,1,6},
	{1,2,6}, {1,2,6}, {1,2,6}, {1,2,6},
	{5,1,6}, {5,1,6}, {5,1,6}, {5,1,6},
	{13,1,8}, {0,6,8}, {12,1,8}, {11,1,8},
	{3,2,8}, {1,3,8}, {0,5,8}, {10,1,8},
	{0,3,5}, {0,3,5}, {0,3,5}, {0,3,5},
	{0,3,5}, {0,3,5}, {0,3,5}, {0,3,5},
	{4,1,5}, {4,1,5}, {4,1,5}, {4,1,5},
	{4,1,5}, {4,1,5}, {4,1,5}, {4,1,5},
	{3,1,5}, {3,1,5}, {3,1,5}, {3,1,5},
	{3,1,5}, {3,1,5}, {3,1,5}, {3,1,5}
};

/* Table B-15, DCT coefficients table one,
 * codes 000001xx ... 11111111
*/
static DCTtab DCTtab0a[252] =
{
	{65,0,6}, {65,0,6}, {65,0,6}, {65,0,6}, /* Escape */
	{7,1,7}, {7,1,7}, {8,1,7}, {8,1,7},
	{6,1,7}, {6,1,7}, {2,2,7}, {2,2,7},
	{0,7,6}, {0,7,6}, {0,7,6}, {0,7,6},
	{0,6,6}, {0,6,6}, {0,6,6}, {0,6,6},
	{4,1,6}, {4,1,6}, {4,1,6}, {4,1,6},
	{5,1,6}, {5,1,6}, {5,1,6}, {5,1,6},
	{1,5,8}, {11,1,8}, {0,11,8}, {0,10,8},
	{13,1,8}, {12,1,8}, {3,2,8}, {1,4,8},
	{2,1,5}, {2,1,5}, {2,1,5}, {2,1,5},
	{2,1,5}, {2,1,5}, {2,1,5}, {2,1,5},
	{1,2,5}, {1,2,5}, {1,2,5}, {1,2,5},
	{1,2,5}, {1,2,5}, {1,2,5}, {1,2,5},
	{3,1,5}, {3,1,5}, {3,1,5}, {3,1,5},
	{3,1,5}, {3,1,5}, {3,1,5}, {3,1,5},
	{1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
	{1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
	{1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
	{1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
	{1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
	{1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
	{1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
	{1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
	{64,0,4}, {64,0,4}, {64,0,4}, {64,0,4}, /* EOB */
	{64,0,4}, {64,0,4}, {64,0,4}, {64,0,4},
	{64,0,4}, {64,0,4}, {64,0,4}, {64,0,4},
	{64,0,4}, {64,0,4}, {64,0,4}, {64,0,4},
	{0,3,4}, {0,3,4}, {0,3,4}, {0,3,4},
	{0,3,4}, {0,3,4}, {0,3,4}, {0,3,4},
	{0,3,4}, {0,3,4}, {0,3,4}, {0,3,4},
	{0,3,4}, {0,3,4}, {0,3,4}, {0,3,4},
	{0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
	{0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
	{0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
	{0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
	{0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
	{0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
	{0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
	{0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
	{0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
	{0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
	{0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
	{0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
	{0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
	{0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
	{0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
	{0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
	{0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
	{0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
	{0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
	{0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
	{0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
	{0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
	{0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
	{0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
	{0,4,5}, {0,4,5}, {0,4,5}, {0,4,5},
	{0,4,5}, {0,4,5}, {0,4,5}, {0,4,5},
	{0,5,5}, {0,5,5}, {0,5,5}, {0,5,5},
	{0,5,5}, {0,5,5}, {0,5,5}, {0,5,5},
	{9,1,7}, {9,1,7}, {1,3,7}, {1,3,7},
	{10,1,7}, {10,1,7}, {0,8,7}, {0,8,7},
	{0,9,7}, {0,9,7}, {0,12,8}, {0,13,8},
	{2,3,8}, {4,2,8}, {0,14,8}, {0,15,8}
};

/* Table B-14, DCT coefficients table zero,
 * codes 0000001000 ... 0000001111
 */
static DCTtab DCTtab1[8] =
{
	{16,1,10}, {5,2,10}, {0,7,10}, {2,3,10},
	{1,4,10}, {15,1,10}, {14,1,10}, {4,2,10}
};

/* Table B-15, DCT coefficients table one,
 * codes 000000100x ... 000000111x
 */
static DCTtab DCTtab1a[8] =
{
	{5,2,9}, {5,2,9}, {14,1,9}, {14,1,9},
	{2,4,10}, {16,1,10}, {15,1,9}, {15,1,9}
};

/* Table B-14/15, DCT coefficients table zero / one,
 * codes 000000010000 ... 000000011111
 */
static DCTtab DCTtab2[16] =
{
	{0,11,12}, {8,2,12}, {4,3,12}, {0,10,12},
	{2,4,12}, {7,2,12}, {21,1,12}, {20,1,12},
	{0,9,12}, {19,1,12}, {18,1,12}, {1,5,12},
	{3,3,12}, {0,8,12}, {6,2,12}, {17,1,12}
};

/* Table B-14/15, DCT coefficients table zero / one,
 * codes 0000000010000 ... 0000000011111
 */
static DCTtab DCTtab3[16] =
{
	{10,2,13}, {9,2,13}, {5,3,13}, {3,4,13},
	{2,5,13}, {1,7,13}, {1,6,13}, {0,15,13},
	{0,14,13}, {0,13,13}, {0,12,13}, {26,1,13},
	{25,1,13}, {24,1,13}, {23,1,13}, {22,1,13}
};

/* Table B-14/15, DCT coefficients table zero / one,
 * codes 00000000010000 ... 00000000011111
 */
static DCTtab DCTtab4[16] =
{
	{0,31,14}, {0,30,14}, {0,29,14}, {0,28,14},
	{0,27,14}, {0,26,14}, {0,25,14}, {0,24,14},
	{0,23,14}, {0,22,14}, {0,21,14}, {0,20,14},
	{0,19,14}, {0,18,14}, {0,17,14}, {0,16,14}
};

/* Table B-14/15, DCT coefficients table zero / one,
 * codes 000000000010000 ... 000000000011111
 */
static DCTtab DCTtab5[16] =
{
	{0,40,15}, {0,39,15}, {0,38,15}, {0,37,15},
	{0,36,15}, {0,35,15}, {0,34,15}, {0,33,15},
	{0,32,15}, {1,14,15}, {1,13,15}, {1,12,15},
	{1,11,15}, {1,10,15}, {1,9,15}, {1,8,15}
};

/* Table B-14/15, DCT coefficients table zero / one,
 * codes 0000000000010000 ... 0000000000011111
 */
static DCTtab DCTtab6[16] =
{
	{1,18,16}, {1,17,16}, {1,16,16}, {1,15,16},
	{6,3,16}, {16,2,16}, {15,2,16}, {14,2,16},
	{13,2,16}, {12,2,16}, {11,2,16}, {31,1,16},
	{30,1,16}, {29,1,16}, {28,1,16}, {27,1,16}
};

/* Table B-3, macroblock_type in P-pictures, codes 001..1xx */
static VLCtab PMBtab0[8] =
{
	{ERROR_VALUE,0},
	{MACROBLOCK_MOTION_FORWARD,3},
	{MACROBLOCK_PATTERN,2}, {MACROBLOCK_PATTERN,2},
	{MACROBLOCK_MOTION_FORWARD|MACROBLOCK_PATTERN,1}, 
	{MACROBLOCK_MOTION_FORWARD|MACROBLOCK_PATTERN,1},
	{MACROBLOCK_MOTION_FORWARD|MACROBLOCK_PATTERN,1}, 
	{MACROBLOCK_MOTION_FORWARD|MACROBLOCK_PATTERN,1}
};

/* Table B-3, macroblock_type in P-pictures, codes 000001..00011x */
static VLCtab PMBtab1[8] =
{
	{ERROR_VALUE,0},
	{MACROBLOCK_QUANT|MACROBLOCK_INTRA,6},
	{MACROBLOCK_QUANT|MACROBLOCK_PATTERN,5}, {MACROBLOCK_QUANT|MACROBLOCK_PATTERN,5},
	{MACROBLOCK_QUANT|MACROBLOCK_MOTION_FORWARD|MACROBLOCK_PATTERN,5}, {MACROBLOCK_QUANT|MACROBLOCK_MOTION_FORWARD|MACROBLOCK_PATTERN,5},
	{MACROBLOCK_INTRA,5}, {MACROBLOCK_INTRA,5}
};

/* Table B-4, macroblock_type in B-pictures, codes 0010..11xx */
static VLCtab BMBtab0[16] =
{
	{ERROR_VALUE,0}, 
	{ERROR_VALUE,0},
	{MACROBLOCK_MOTION_FORWARD,4},
	{MACROBLOCK_MOTION_FORWARD|MACROBLOCK_PATTERN,4},
	{MACROBLOCK_MOTION_BACKWARD,3}, 
	{MACROBLOCK_MOTION_BACKWARD,3},
	{MACROBLOCK_MOTION_BACKWARD|MACROBLOCK_PATTERN,3}, 
	{MACROBLOCK_MOTION_BACKWARD|MACROBLOCK_PATTERN,3},
	{MACROBLOCK_MOTION_FORWARD|MACROBLOCK_MOTION_BACKWARD,2}, 
	{MACROBLOCK_MOTION_FORWARD|MACROBLOCK_MOTION_BACKWARD,2},
	{MACROBLOCK_MOTION_FORWARD|MACROBLOCK_MOTION_BACKWARD,2}, 
	{MACROBLOCK_MOTION_FORWARD|MACROBLOCK_MOTION_BACKWARD,2},
	{MACROBLOCK_MOTION_FORWARD|MACROBLOCK_MOTION_BACKWARD|MACROBLOCK_PATTERN,2},
	{MACROBLOCK_MOTION_FORWARD|MACROBLOCK_MOTION_BACKWARD|MACROBLOCK_PATTERN,2},
	{MACROBLOCK_MOTION_FORWARD|MACROBLOCK_MOTION_BACKWARD|MACROBLOCK_PATTERN,2},
	{MACROBLOCK_MOTION_FORWARD|MACROBLOCK_MOTION_BACKWARD|MACROBLOCK_PATTERN,2}
};

/* Table B-4, macroblock_type in B-pictures, codes 000001..00011x */
static VLCtab BMBtab1[8] =
{
	{ERROR_VALUE,0},
	{MACROBLOCK_QUANT|MACROBLOCK_INTRA,6},
	{MACROBLOCK_QUANT|MACROBLOCK_MOTION_BACKWARD|MACROBLOCK_PATTERN,6},
	{MACROBLOCK_QUANT|MACROBLOCK_MOTION_FORWARD|MACROBLOCK_PATTERN,6},
	{MACROBLOCK_QUANT|MACROBLOCK_MOTION_FORWARD|MACROBLOCK_MOTION_BACKWARD|MACROBLOCK_PATTERN,5},
	{MACROBLOCK_QUANT|MACROBLOCK_MOTION_FORWARD|MACROBLOCK_MOTION_BACKWARD|MACROBLOCK_PATTERN,5},
	{MACROBLOCK_INTRA,5}, 
	{MACROBLOCK_INTRA,5}
};

//
// getbit
//

void CMPEG2Dec::Initialize_Buffer()
{
	Rdptr = Rdbfr + BUFFER_SIZE;
	Rdmax = Rdptr;

	if (SystemStream_Flag)
	{
		if (Rdptr >= Rdmax)
			Next_Packet();
		CurrentBfr = *Rdptr++ << 24;

		if (Rdptr >= Rdmax)
			Next_Packet();
		CurrentBfr += *Rdptr++ << 16;

		if (Rdptr >= Rdmax)
			Next_Packet();
		CurrentBfr += *Rdptr++ << 8;

		if (Rdptr >= Rdmax)
			Next_Packet();
		CurrentBfr += *Rdptr++;

		Fill_Next();
	}
	else
	{
		Fill_Buffer();

		CurrentBfr = (*Rdptr << 24) + (*(Rdptr+1) << 16) + (*(Rdptr+2) << 8) + *(Rdptr+3);
		Rdptr += 4;

		Fill_Next();
	}

	BitsLeft = 32;
}

unsigned int CMPEG2Dec::Get_Bits_All(unsigned int N)
{
	N -= BitsLeft;
	Val = (CurrentBfr << (32 - BitsLeft)) >> (32 - BitsLeft);

	if (N != 0)
		Val = (Val << N) + (NextBfr >> (32 - N));

	CurrentBfr = NextBfr;
	BitsLeft = 32 - N;
	Fill_Next();

	return Val;
}

void CMPEG2Dec::Flush_Buffer_All(unsigned int N)
{
	CurrentBfr = NextBfr;
	BitsLeft = BitsLeft + 32 - N;
	Fill_Next();
}

void CMPEG2Dec::Next_Packet()
{
	unsigned int code, Packet_Length, Packet_Header_Length;

	for (;;)
	{
		code = Get_Short();
		code = (code<<16) + Get_Short();

		// remove system layer byte stuffing
		while ((code & 0xffffff00) != 0x00000100)
			code = (code<<8) + Get_Byte();

		switch (code)
		{
			case PACK_START_CODE:
				Rdptr += 8;
				break;

			case VIDEO_ELEMENTARY_STREAM:   
				Packet_Length = Get_Short();
				Rdmax = Rdptr + Packet_Length;

				code = Get_Byte();

				if ((code & 0xc0)==0x80)
				{
					code = Get_Byte();
					Packet_Header_Length = Get_Byte();

					Rdptr += Packet_Header_Length;
					return;
				}
				else
					Rdptr += Packet_Length-1;
				break;

			default:
				if (code>=SYSTEM_START_CODE)
				{
					code = Get_Short();
					Rdptr += code;
				}
				break;
		}
	}
}

void CMPEG2Dec::Fill_Buffer()
{
	Read = _read(Infile[File_Flag], Rdbfr, BUFFER_SIZE);

	if (Read < BUFFER_SIZE)
		Next_File();

	if (KeyOp_Flag && (Rdbfr[20] & 0x10))
	{
		BufferOp(Rdbfr, lfsr0, lfsr1);
		Rdbfr[20] &= ~0x10;
	}

	Rdptr = Rdbfr;

	if (SystemStream_Flag)
		Rdmax -= BUFFER_SIZE;
}

void CMPEG2Dec::Next_File()
{
	if (File_Flag < File_Limit-1)
		File_Flag ++;

	_lseeki64(Infile[File_Flag], 0, SEEK_SET);
	_read(Infile[File_Flag], Rdbfr + Read, BUFFER_SIZE - Read);
}

unsigned int CMPEG2Dec::Show_Bits(unsigned int N)
{
	if (N <= BitsLeft)
		return (CurrentBfr << (32 - BitsLeft)) >> (32 - N);
	else
	{
		N -= BitsLeft;
		return (((CurrentBfr << (32 - BitsLeft)) >> (32 - BitsLeft)) << N) + (NextBfr >> (32 - N));
	}
}

unsigned int CMPEG2Dec::Get_Bits(unsigned int N)
{
	if (N < BitsLeft)
	{
		Val = (CurrentBfr << (32 - BitsLeft)) >> (32 - N);
		BitsLeft -= N;
		return Val;
	}
	else
		return Get_Bits_All(N);
}

void CMPEG2Dec::Flush_Buffer(unsigned int N)
{
	if (N < BitsLeft)
		BitsLeft -= N;
	else
		Flush_Buffer_All(N);	
}

void CMPEG2Dec::Fill_Next()
{
	if (SystemStream_Flag && Rdptr>=Rdmax-4)
	{
		if (Rdptr >= Rdmax)
			Next_Packet();
		NextBfr = Get_Byte() << 24;

		if (Rdptr >= Rdmax)
			Next_Packet();
		NextBfr += Get_Byte() << 16;

		if (Rdptr >= Rdmax)
			Next_Packet();
		NextBfr += Get_Byte() << 8;

		if (Rdptr >= Rdmax)
			Next_Packet();
		NextBfr += Get_Byte();
	}
	else if (Rdptr < Rdbfr+BUFFER_SIZE-4)
	{
		NextBfr = (*Rdptr << 24) + (*(Rdptr+1) << 16) + (*(Rdptr+2) << 8) + *(Rdptr+3);
		Rdptr += 4;
	}
	else
	{
		if (Rdptr >= Rdbfr+BUFFER_SIZE)
			Fill_Buffer();
		NextBfr = *Rdptr++ << 24;

		if (Rdptr >= Rdbfr+BUFFER_SIZE)
			Fill_Buffer();
		NextBfr += *Rdptr++ << 16;

		if (Rdptr >= Rdbfr+BUFFER_SIZE)
			Fill_Buffer();
		NextBfr += *Rdptr++ << 8;

		if (Rdptr >= Rdbfr+BUFFER_SIZE)
			Fill_Buffer();
		NextBfr += *Rdptr++;
	}
}

unsigned int CMPEG2Dec::Get_Byte()
{
	while (Rdptr >= (Rdbfr + BUFFER_SIZE))
	{
		Read = _read(Infile[File_Flag], Rdbfr, BUFFER_SIZE);

		if (Read < BUFFER_SIZE)
			Next_File();

		if (KeyOp_Flag && (Rdbfr[20] & 0x10))
		{
			BufferOp(Rdbfr, lfsr0, lfsr1);
			Rdbfr[20] &= ~0x10;
		}

		Rdptr -= BUFFER_SIZE;
		Rdmax -= BUFFER_SIZE;
	}

	return *Rdptr++;
}

unsigned int CMPEG2Dec::Get_Short()
{
	unsigned int i = Get_Byte();
	return (i<<8) + Get_Byte();
}

void CMPEG2Dec::next_start_code()
{
	Flush_Buffer(BitsLeft & 7);

	while (Show_Bits(24) != 1)
		Flush_Buffer(8);
}

//
// gethdr
//

int CMPEG2Dec::Get_Hdr()
{
	for (;;)
	{
		/* look for next_start_code */
		next_start_code();

		switch (Get_Bits(32))
		{
			case SEQUENCE_HEADER_CODE:
				sequence_header();
				break;

			case GROUP_START_CODE:
				group_of_pictures_header();
				break;

			case PICTURE_START_CODE:
				picture_header();
				return 1;
		}
	}
}

/* decode group of pictures header */
/* ISO/IEC 13818-2 section 6.2.2.6 */
void CMPEG2Dec::group_of_pictures_header()
{
	int gop_hour;
	int gop_minute;
	int gop_sec;
	int gop_frame;

	int drop_flag;
	int closed_gop;
	int broken_link;

	drop_flag   = Get_Bits(1);
	gop_hour    = Get_Bits(5);
	gop_minute  = Get_Bits(6);
	Flush_Buffer(1);	// marker bit
	gop_sec     = Get_Bits(6);
	gop_frame	= Get_Bits(6);
	closed_gop  = Get_Bits(1);
	broken_link = Get_Bits(1);

	extension_and_user_data();
}

/* decode picture header */
/* ISO/IEC 13818-2 section 6.2.3 */
void CMPEG2Dec::picture_header()
{
	int vbv_delay;
	int full_pel_forward_vector;
	int forward_f_code;
	int full_pel_backward_vector;
	int backward_f_code;
	int Extra_Information_Byte_Count;

	temporal_reference  = Get_Bits(10);
	picture_coding_type = Get_Bits(3);
	vbv_delay           = Get_Bits(16);

	if (picture_coding_type==P_TYPE || picture_coding_type==B_TYPE)
	{
		full_pel_forward_vector = Get_Bits(1);
		forward_f_code = Get_Bits(3);
	}

	if (picture_coding_type==B_TYPE)
	{
		full_pel_backward_vector = Get_Bits(1);
		backward_f_code = Get_Bits(3);
	}

	Extra_Information_Byte_Count = extra_bit_information();
	extension_and_user_data();
}

/* decode sequence header */
void CMPEG2Dec::sequence_header()
{
	int frame_rate_code;
	int vbv_buffer_size;
	int aspect_ratio_information;
	int bit_rate_value;

	int constrained_parameters_flag;
	int i;

	horizontal_size             = Get_Bits(12);
	vertical_size               = Get_Bits(12);
	aspect_ratio_information    = Get_Bits(4);
	frame_rate_code             = Get_Bits(4);
	bit_rate_value              = Get_Bits(18);
	Flush_Buffer(1);	// marker bit
	vbv_buffer_size             = Get_Bits(10);
	constrained_parameters_flag = Get_Bits(1);

	load_intra_quantizer_matrix = Get_Bits(1);
	if (load_intra_quantizer_matrix)
	{
		for (i=0; i<64; i++)
			intra_quantizer_matrix[scan[ZIG_ZAG][i]] = Get_Bits(8);
	}
	else
	{
		for (i=0; i<64; i++)
			intra_quantizer_matrix[i] = default_intra_quantizer_matrix[i];
	}

	load_non_intra_quantizer_matrix = Get_Bits(1);
	if (load_non_intra_quantizer_matrix)
	{
		for (i=0; i<64; i++)
			non_intra_quantizer_matrix[scan[ZIG_ZAG][i]] = Get_Bits(8);
	}
	else
	{
		for (i=0; i<64; i++)
			non_intra_quantizer_matrix[i] = 16;
	}

	/* copy luminance to chrominance matrices */
	for (i=0; i<64; i++)
	{
		chroma_intra_quantizer_matrix[i] = intra_quantizer_matrix[i];
		chroma_non_intra_quantizer_matrix[i] = non_intra_quantizer_matrix[i];
	}
	extension_and_user_data();
}

/* decode slice header */
/* ISO/IEC 13818-2 section 6.2.4 */
int CMPEG2Dec::slice_header()
{
	int slice_vertical_position_extension;
	int quantizer_scale_code;
	int slice_picture_id_enable = 0;
	int slice_picture_id = 0;
	int extra_information_slice = 0;

	slice_vertical_position_extension = vertical_size>2800 ? Get_Bits(3) : 0;

	quantizer_scale_code = Get_Bits(5);
	quantizer_scale = q_scale_type ? Non_Linear_quantizer_scale[quantizer_scale_code] : quantizer_scale_code<<1;

	/* slice_id introduced in March 1995 as part of the video corridendum
	   (after the IS was drafted in November 1994) */
	if (Get_Bits(1))
	{
		Get_Bits(1);	// intra slice

		slice_picture_id_enable = Get_Bits(1);
		slice_picture_id = Get_Bits(6);

		extra_information_slice = extra_bit_information();
	}

	return slice_vertical_position_extension;
}

/* decode extension and user data */
/* ISO/IEC 13818-2 section 6.2.2.2 */
void CMPEG2Dec::extension_and_user_data()
{
	int code, ext_ID;

	next_start_code();

	while ((code = Show_Bits(32))==EXTENSION_START_CODE || code==USER_DATA_START_CODE)
	{
		if (code==EXTENSION_START_CODE)
		{
			Flush_Buffer(32);
			ext_ID = Get_Bits(4);

			switch (ext_ID)
			{
				case SEQUENCE_EXTENSION_ID:
					sequence_extension();
					break;

				case SEQUENCE_DISPLAY_EXTENSION_ID:
					sequence_display_extension();
					break;

				case QUANT_MATRIX_EXTENSION_ID:
					quant_matrix_extension();
					break;

				case PICTURE_DISPLAY_EXTENSION_ID:
					picture_display_extension();
					break;

				case PICTURE_CODING_EXTENSION_ID:
					picture_coding_extension();
					break;

				case COPYRIGHT_EXTENSION_ID:
					copyright_extension();
					break;
			}
			next_start_code();
		}
		else
		{
			Flush_Buffer(32);
			next_start_code();
		}
	}
}

/* decode sequence extension */
/* ISO/IEC 13818-2 section 6.2.2.3 */
void CMPEG2Dec::sequence_extension()
{
	int profile_and_level_indication;
	int low_delay;
	int frame_rate_extension_n;
	int frame_rate_extension_d;

	int horizontal_size_extension;
	int vertical_size_extension;
	int bit_rate_extension;
	int vbv_buffer_size_extension;

	profile_and_level_indication = Get_Bits(8);
	progressive_sequence         = Get_Bits(1);
	chroma_format                = Get_Bits(2);
	horizontal_size_extension    = Get_Bits(2);
	vertical_size_extension      = Get_Bits(2);
	bit_rate_extension           = Get_Bits(12);
	Flush_Buffer(1);	// marker bit
	vbv_buffer_size_extension    = Get_Bits(8);
	low_delay                    = Get_Bits(1);
 
	frame_rate_extension_n       = Get_Bits(2);
	frame_rate_extension_d       = Get_Bits(5);

	horizontal_size = (horizontal_size_extension<<12) | (horizontal_size&0x0fff);
	vertical_size = (vertical_size_extension<<12) | (vertical_size&0x0fff);
}

/* decode sequence display extension */
void CMPEG2Dec::sequence_display_extension()
{
	int video_format;  
	int color_description;
	int color_primaries;
	int transfer_characteristics;
	int matrix_coefficients;
	int display_horizontal_size;
	int display_vertical_size;

	video_format      = Get_Bits(3);
	color_description = Get_Bits(1);

	if (color_description)
	{
		color_primaries          = Get_Bits(8);
		transfer_characteristics = Get_Bits(8);
		matrix_coefficients      = Get_Bits(8);
	}

	display_horizontal_size = Get_Bits(14);
	Flush_Buffer(1);	// marker bit
	display_vertical_size   = Get_Bits(14);
}

/* decode quant matrix entension */
/* ISO/IEC 13818-2 section 6.2.3.2 */
void CMPEG2Dec::quant_matrix_extension()
{
	int i;

	load_intra_quantizer_matrix = Get_Bits(1);
	if (load_intra_quantizer_matrix)
		for (i=0; i<64; i++)
			chroma_intra_quantizer_matrix[scan[ZIG_ZAG][i]]
				= intra_quantizer_matrix[scan[ZIG_ZAG][i]] = Get_Bits(8);

	load_non_intra_quantizer_matrix = Get_Bits(1);
	if (load_non_intra_quantizer_matrix)
		for (i=0; i<64; i++)
			chroma_non_intra_quantizer_matrix[scan[ZIG_ZAG][i]]
				= non_intra_quantizer_matrix[scan[ZIG_ZAG][i]] = Get_Bits(8);
	
	load_chroma_intra_quantizer_matrix = Get_Bits(1);
	if (load_chroma_intra_quantizer_matrix)
		for (i=0; i<64; i++)
			chroma_intra_quantizer_matrix[scan[ZIG_ZAG][i]] = Get_Bits(8);

	load_chroma_non_intra_quantizer_matrix = Get_Bits(1);
	if (load_chroma_non_intra_quantizer_matrix)
		for (i=0; i<64; i++)
			chroma_non_intra_quantizer_matrix[scan[ZIG_ZAG][i]] = Get_Bits(8);
}

/* decode picture display extension */
/* ISO/IEC 13818-2 section 6.2.3.3. */
void CMPEG2Dec::picture_display_extension()
{
	int frame_center_horizontal_offset[3];
	int frame_center_vertical_offset[3];

	int i;
	int number_of_frame_center_offsets;

	/* based on ISO/IEC 13818-2 section 6.3.12 
	   (November 1994) Picture display extensions */

	/* derive number_of_frame_center_offsets */
	if (progressive_sequence)
	{
		if (repeat_first_field)
		{
			if (top_field_first)
				number_of_frame_center_offsets = 3;
			else
				number_of_frame_center_offsets = 2;
		}
		else
			number_of_frame_center_offsets = 1;
	}
	else
	{
		if (picture_structure!=FRAME_PICTURE)
			number_of_frame_center_offsets = 1;
		else
		{
			if (repeat_first_field)
				number_of_frame_center_offsets = 3;
			else
				number_of_frame_center_offsets = 2;
		}
	}

	/* now parse */
	for (i=0; i<number_of_frame_center_offsets; i++)
	{
		frame_center_horizontal_offset[i] = Get_Bits(16);
		Flush_Buffer(1);	// marker bit

		frame_center_vertical_offset[i] = Get_Bits(16);
		Flush_Buffer(1);	// marker bit
	}
}

/* decode picture coding extension */
void CMPEG2Dec::picture_coding_extension()
{
	int chroma_420_type;
	int composite_display_flag;
	int v_axis;
	int field_sequence;
	int sub_carrier;
	int burst_amplitude;
	int sub_carrier_phase;

	f_code[0][0] = Get_Bits(4);
	f_code[0][1] = Get_Bits(4);
	f_code[1][0] = Get_Bits(4);
	f_code[1][1] = Get_Bits(4);

	intra_dc_precision         = Get_Bits(2);
	picture_structure          = Get_Bits(2);
	top_field_first            = Get_Bits(1);
	frame_pred_frame_dct       = Get_Bits(1);
	concealment_motion_vectors = Get_Bits(1);
	q_scale_type			   = Get_Bits(1);
	intra_vlc_format           = Get_Bits(1);
	alternate_scan			   = Get_Bits(1);
	repeat_first_field         = Get_Bits(1);
	chroma_420_type            = Get_Bits(1);
	progressive_frame          = Get_Bits(1);
	composite_display_flag     = Get_Bits(1);

	pf_current = progressive_frame;

	if (composite_display_flag)
	{
		v_axis            = Get_Bits(1);
		field_sequence    = Get_Bits(3);
		sub_carrier       = Get_Bits(1);
		burst_amplitude   = Get_Bits(7);
		sub_carrier_phase = Get_Bits(8);
	}
}

/* decode extra bit information */
/* ISO/IEC 13818-2 section 6.2.3.4. */
int CMPEG2Dec::extra_bit_information()
{
	int Byte_Count = 0;

	while (Get_Bits(1))
	{
		Flush_Buffer(8);
		Byte_Count ++;
	}

	return(Byte_Count);
}

/* Copyright extension */
/* ISO/IEC 13818-2 section 6.2.3.6. */
/* (header added in November, 1994 to the IS document) */
void CMPEG2Dec::copyright_extension()
{
	int copyright_flag;
	int copyright_identifier;
	int original_or_copy;
	int copyright_number_1;
	int copyright_number_2;
	int copyright_number_3;

	int reserved_data;

	copyright_flag =       Get_Bits(1); 
	copyright_identifier = Get_Bits(8);
	original_or_copy =     Get_Bits(1);
  
	/* reserved */
	reserved_data = Get_Bits(7);

	Flush_Buffer(1); // marker bit
	copyright_number_1 =   Get_Bits(20);
	Flush_Buffer(1); // marker bit
	copyright_number_2 =   Get_Bits(22);
	Flush_Buffer(1); // marker bit
	copyright_number_3 =   Get_Bits(22);
}

//
// getpic
//

static const unsigned char cc_table[12] = {
	0, 0, 0, 0, 1, 2, 1, 2, 1, 2, 1, 2
};

void CMPEG2Dec::Decode_Picture(int ref, unsigned char *dst, int pitch)
{
	if (picture_structure==FRAME_PICTURE && Second_Field)
		Second_Field = 0;

	if (picture_coding_type!=B_TYPE)
	{
		pf_forward = pf_backward;
		pf_backward = pf_current;
	}

	Update_Picture_Buffers();

	picture_data();

	if (ref && (picture_structure==FRAME_PICTURE || Second_Field))
	{
		if (picture_coding_type==B_TYPE)
			assembleFrame(auxframe, pf_current, dst, pitch);
		else
			assembleFrame(forward_reference_frame, pf_forward, dst, pitch);
	}

	if (picture_structure!=FRAME_PICTURE)
		Second_Field = !Second_Field;
}

/* reuse old picture buffers as soon as they are no longer needed */
void CMPEG2Dec::Update_Picture_Buffers()
{                           
	int cc;              /* color component index */
	unsigned char *tmp;  /* temporary swap pointer */

	for (cc=0; cc<3; cc++)
	{
		/* B pictures  do not need to be save for future reference */
		if (picture_coding_type==B_TYPE)
			current_frame[cc] = auxframe[cc];
		else
		{
			if (!Second_Field)
			{
				/* only update at the beginning of the coded frame */
				tmp = forward_reference_frame[cc];

				/* the previously decoded reference frame is stored coincident with the 
				   location where the backward reference frame is stored (backwards 
				   prediction is not needed in P pictures) */
				forward_reference_frame[cc] = backward_reference_frame[cc];

				/* update pointer for potential future B pictures */
				backward_reference_frame[cc] = tmp;
			}

			/* can erase over old backward reference frame since it is not used
			   in a P picture, and since any subsequent B pictures will use the 
			   previously decoded I or P frame as the backward_reference_frame */
			current_frame[cc] = backward_reference_frame[cc];
		}

	    if (picture_structure==BOTTOM_FIELD)
			current_frame[cc] += (cc==0) ? Coded_Picture_Width : Chroma_Width;
	}
}

/* decode all macroblocks of the current picture */
/* stages described in ISO/IEC 13818-2 section 7 */
void CMPEG2Dec::picture_data()
{
	int MBAmax;

	/* number of macroblocks per picture */
	MBAmax = mb_width*mb_height;

	if (picture_structure!=FRAME_PICTURE)
		MBAmax>>=1;

	for (;;)
		if (slice(MBAmax)<0)
			return;
}

/* decode all macroblocks of the current picture */
/* ISO/IEC 13818-2 section 6.3.16 */
/* return 0 : go to next slice */
/* return -1: go to next picture */
int CMPEG2Dec::slice(int MBAmax)
{
	int MBA = 0, MBAinc =0, macroblock_type, motion_type, dct_type = -1, ret;
	int dc_dct_pred[3], PMV[2][2][2], motion_vertical_field_select[2][2], dmvector[2];

	if ((ret=start_of_slice(&MBA, &MBAinc, dc_dct_pred, PMV))!=1)
		return ret;

	for (;;)
	{
		/* this is how we properly exit out of picture */
		if (MBA>=MBAmax) return -1;		// all macroblocks decoded

		if (MBAinc==0)
		{
			if (!Show_Bits(23) || Fault_Flag)	// next_start_code or fault
			{
resync:
				Fault_Flag = 0;
				return 0;	// trigger: go to next slice
			}
			else /* neither next_start_code nor Fault_Flag */
			{
				/* decode macroblock address increment */
				MBAinc = Get_macroblock_address_increment();
				if (Fault_Flag) goto resync;
			}
		}

		if (MBAinc==1) /* not skipped */
		{
			if (!decode_macroblock(&macroblock_type, &motion_type, &dct_type, PMV,
				dc_dct_pred, motion_vertical_field_select, dmvector))
				goto resync;
		}
		else /* MBAinc!=1: skipped macroblock */
			/* ISO/IEC 13818-2 section 7.6.6 */
			skipped_macroblock(dc_dct_pred, PMV, &motion_type, motion_vertical_field_select, &macroblock_type);

		_ASSERTE(dct_type != -1); // This should never happened 
		/* ISO/IEC 13818-2 section 7.6 */
		motion_compensation(MBA, macroblock_type, motion_type, PMV,
							motion_vertical_field_select, dmvector, dct_type);

		/* advance to next macroblock */
		MBA++; MBAinc--;

		if (MBA>=MBAmax) return -1;		// all macroblocks decoded
	}
}

/* ISO/IEC 13818-2 section 6.3.17.1: Macroblock modes */
void CMPEG2Dec::macroblock_modes(int *pmacroblock_type, int *pmotion_type,
							 int *pmotion_vector_count, int *pmv_format,
							 int *pdmv, int *pmvscale, int *pdct_type)
{
	int macroblock_type, motion_type = 0, motion_vector_count;
	int mv_format, dmv, mvscale, dct_type;

	/* get macroblock_type */
	macroblock_type = Get_macroblock_type();
	if (Fault_Flag) return;

	/* get frame/field motion type */
	if (macroblock_type & (MACROBLOCK_MOTION_FORWARD|MACROBLOCK_MOTION_BACKWARD))
	{
		if (picture_structure==FRAME_PICTURE)
			motion_type = frame_pred_frame_dct ? MC_FRAME : Get_Bits(2);
		else
			motion_type = Get_Bits(2);
    }
	else if ((macroblock_type & MACROBLOCK_INTRA) && concealment_motion_vectors)
		motion_type = (picture_structure==FRAME_PICTURE) ? MC_FRAME : MC_FIELD;

	/* derive motion_vector_count, mv_format and dmv, (table 6-17, 6-18) */
	if (picture_structure==FRAME_PICTURE)
	{
		motion_vector_count = (motion_type==MC_FIELD) ? 2 : 1;
		mv_format = (motion_type==MC_FRAME) ? MV_FRAME : MV_FIELD;
	}
	else
	{
		motion_vector_count = (motion_type==MC_16X8) ? 2 : 1;
		mv_format = MV_FIELD;
	}
	
	dmv = (motion_type==MC_DMV); /* dual prime */

	/*
	   field mv predictions in frame pictures have to be scaled
	   ISO/IEC 13818-2 section 7.6.3.1 Decoding the motion vectors
	*/
	mvscale = (mv_format==MV_FIELD && picture_structure==FRAME_PICTURE);

	/* get dct_type (frame DCT / field DCT) */
	dct_type = (picture_structure==FRAME_PICTURE) && (!frame_pred_frame_dct)
				&& (macroblock_type & (MACROBLOCK_PATTERN|MACROBLOCK_INTRA)) ? Get_Bits(1) : 0;

	/* return values */
	*pmacroblock_type = macroblock_type;
	*pmotion_type = motion_type;
	*pmotion_vector_count = motion_vector_count;
	*pmv_format = mv_format;
	*pdmv = dmv;
	*pmvscale = mvscale;
	*pdct_type = dct_type;
}

/* move/add 8x8-Block from block[comp] to backward_reference_frame */
/* copy reconstructed 8x8 block from block[comp] to current_frame[]
   ISO/IEC 13818-2 section 7.6.8: Adding prediction and coefficient data
   This stage also embodies some of the operations implied by:
   - ISO/IEC 13818-2 section 7.6.7: Combining predictions
   - ISO/IEC 13818-2 section 6.1.3: Macroblock
*/
void CMPEG2Dec::Add_Block(int count, int bx, int by, int dct_type, int addflag)
{
	static const __int64 mmmask_128 = 0x0080008000800080;

	int comp, cc, iincr, bxh, byh;
	unsigned char *rfp;
	short *Block_Ptr;

	for (comp=0; comp<count; comp++)
	{
		Block_Ptr = block[comp];
		cc = cc_table[comp];

		bxh = bx; byh = by;

		if (cc==0)
		{
			if (picture_structure==FRAME_PICTURE)
			{
				if (dct_type)
				{
					rfp = current_frame[0] + Coded_Picture_Width*(by+((comp&2)>>1)) + bx + ((comp&1)<<3);
					iincr = Coded_Picture_Width<<1;
				}
				else
				{
					rfp = current_frame[0] + Coded_Picture_Width*(by+((comp&2)<<2)) + bx + ((comp&1)<<3);
					iincr = Coded_Picture_Width;
				}
			}
			else
			{
				rfp = current_frame[0] + (Coded_Picture_Width<<1)*(by+((comp&2)<<2)) + bx + ((comp&1)<<3);
				iincr = Coded_Picture_Width<<1;
			}
		}
		else
		{
			if (chroma_format!=CHROMA444)
				bxh >>= 1;
			if (chroma_format==CHROMA420)
				byh >>= 1;

			if (picture_structure==FRAME_PICTURE)
			{
				if (dct_type && chroma_format!=CHROMA420)
				{
					/* field DCT coding */
					rfp = current_frame[cc] + Chroma_Width*(byh+((comp&2)>>1)) + bxh + (comp&8);
					iincr = Chroma_Width<<1;
				}
				else
				{
					/* frame DCT coding */
					rfp = current_frame[cc] + Chroma_Width*(byh+((comp&2)<<2)) + bxh + (comp&8);
					iincr = Chroma_Width;
				}
			}
			else
			{
				/* field picture */
				rfp = current_frame[cc] + (Chroma_Width<<1)*(byh+((comp&2)<<2)) + bxh + (comp&8);
				iincr = Chroma_Width<<1;
			}
		}

		if (addflag)
		{
			__asm
			{
				pxor		mm0, mm0
				mov			eax, [rfp]
				mov			ebx, [Block_Ptr]
				mov			edi, 8
addon:
				movq		mm2, [ebx+8]

				movq		mm3, [eax]
				movq		mm4, mm3

				movq		mm1, [ebx]
				punpckhbw	mm3, mm0

				paddsw		mm3, mm2
				packuswb	mm3, mm0

				punpcklbw	mm4, mm0
				psllq		mm3, 32

				paddsw		mm4, mm1
				packuswb	mm4, mm0

				por			mm3, mm4			
				add			ebx, 16

				dec			edi
				movq		[eax], mm3

				add			eax, [iincr]
				cmp			edi, 0x00
				jg			addon
			}
		}
		else
		{
			__asm
			{
				mov			eax, [rfp]
				mov			ebx, [Block_Ptr]
				mov			edi, 8

				pxor		mm0, mm0
				movq		mm7, [mmmask_128]
addoff:
				movq		mm3, [ebx+8]
				movq		mm4, [ebx]

				paddsw		mm3, mm7
				paddsw		mm4, mm7

				packuswb	mm3, mm0
				packuswb	mm4, mm0

				psllq		mm3, 32
				por			mm3, mm4
			
				add			ebx, 16
				dec			edi

				movq		[eax], mm3

				add			eax, [iincr]
				cmp			edi, 0x00
				jg			addoff
			}
		}
	}
}

/* set scratch pad macroblock to zero */
void CMPEG2Dec::Clear_Block(int count)
{
	int comp;
	short *Block_Ptr;

	for (comp=0; comp<count; comp++)
	{
		Block_Ptr = block[comp];

		__asm
		{
			mov			eax, [Block_Ptr];
			pxor		mm0, mm0;
			movq		[eax+0 ], mm0;
			movq		[eax+8 ], mm0;
			movq		[eax+16], mm0;
			movq		[eax+24], mm0;
			movq		[eax+32], mm0;
			movq		[eax+40], mm0;
			movq		[eax+48], mm0;
			movq		[eax+56], mm0;
			movq		[eax+64], mm0;
			movq		[eax+72], mm0;
			movq		[eax+80], mm0;
			movq		[eax+88], mm0;
			movq		[eax+96], mm0;
			movq		[eax+104],mm0;
			movq		[eax+112],mm0;
			movq		[eax+120],mm0;
		}
	}
}

/* ISO/IEC 13818-2 section 7.6 */
void CMPEG2Dec::motion_compensation(int MBA, int macroblock_type, int motion_type, 
								int PMV[2][2][2], int motion_vertical_field_select[2][2],
								int dmvector[2], int dct_type)
{
	int bx, by;
	int comp;

	/* derive current macroblock position within picture */
	/* ISO/IEC 13818-2 section 6.3.1.6 and 6.3.1.7 */
	bx = 16*(MBA%mb_width);
	by = 16*(MBA/mb_width);

	/* motion compensation */
	if (!(macroblock_type & MACROBLOCK_INTRA))
		form_predictions(bx, by, macroblock_type, motion_type, PMV, 
			motion_vertical_field_select, dmvector);

	switch (IDCT_Flag)
	{
		case IDCT_MMX:
			for (comp=0; comp<block_count; comp++)
				MMX_IDCT(block[comp]);
			break;

		case IDCT_SSEMMX:
			for (comp=0; comp<block_count; comp++)
				SSEMMX_IDCT(block[comp]);
			break;

		case IDCT_FPU:
			__asm emms;
			for (comp=0; comp<block_count; comp++)
				FPU_IDCT(block[comp]);
			break;

		case IDCT_REF:
			__asm emms;
			for (comp=0; comp<block_count; comp++)
				REF_IDCT(block[comp]);
			break;
	}

	Add_Block(block_count, bx, by, dct_type, (macroblock_type & MACROBLOCK_INTRA)==0);
}

/* ISO/IEC 13818-2 section 7.6.6 */
void CMPEG2Dec::skipped_macroblock(int dc_dct_pred[3], int PMV[2][2][2], int *motion_type, 
							   int motion_vertical_field_select[2][2], int *macroblock_type)
{
	Clear_Block(block_count);

	/* reset intra_dc predictors */
	/* ISO/IEC 13818-2 section 7.2.1: DC coefficients in intra blocks */
	dc_dct_pred[0]=dc_dct_pred[1]=dc_dct_pred[2]=0;

	/* reset motion vector predictors */
	/* ISO/IEC 13818-2 section 7.6.3.4: Resetting motion vector predictors */
	if (picture_coding_type==P_TYPE)
		PMV[0][0][0]=PMV[0][0][1]=PMV[1][0][0]=PMV[1][0][1]=0;

	/* derive motion_type */
	if (picture_structure==FRAME_PICTURE)
		*motion_type = MC_FRAME;
	else
	{
		*motion_type = MC_FIELD;
		motion_vertical_field_select[0][0] = motion_vertical_field_select[0][1] = 
			(picture_structure==BOTTOM_FIELD);
	}

	/* clear MACROBLOCK_INTRA */
	*macroblock_type&= ~MACROBLOCK_INTRA;
}

/* return==-1 means go to next picture */
/* the expression "start of slice" is used throughout the normative
   body of the MPEG specification */
int CMPEG2Dec::start_of_slice(int *MBA, int *MBAinc,
						  int dc_dct_pred[3], int PMV[2][2][2])
{
	unsigned int code;
	int slice_vert_pos_ext;

	next_start_code();
	code = Get_Bits(32);

	if (code<SLICE_START_CODE_MIN || code>SLICE_START_CODE_MAX)
	{
		// only slice headers are allowed in picture_data
		Fault_Flag = 10;
		return -1;
	}

	/* decode slice header (may change quantizer_scale) */
	slice_vert_pos_ext = slice_header();

	/* decode macroblock address increment */
	*MBAinc = Get_macroblock_address_increment();
	if (Fault_Flag) return -1;

	/* set current location */
	/* NOTE: the arithmetic used to derive macroblock_address below is
	   equivalent to ISO/IEC 13818-2 section 6.3.17: Macroblock */
	*MBA = ((slice_vert_pos_ext<<7) + (code&255) - 1)*mb_width + *MBAinc - 1;
	*MBAinc = 1;	// first macroblock in slice: not skipped

	/* reset all DC coefficient and motion vector predictors */
	/* ISO/IEC 13818-2 section 7.2.1: DC coefficients in intra blocks */
	dc_dct_pred[0]=dc_dct_pred[1]=dc_dct_pred[2]=0;
  
	/* ISO/IEC 13818-2 section 7.6.3.4: Resetting motion vector predictors */
	PMV[0][0][0]=PMV[0][0][1]=PMV[1][0][0]=PMV[1][0][1]=0;
	PMV[0][1][0]=PMV[0][1][1]=PMV[1][1][0]=PMV[1][1][1]=0;

	/* successfull: trigger decode macroblocks in slice */
	return 1;
}

/* ISO/IEC 13818-2 sections 7.2 through 7.5 */
int CMPEG2Dec::decode_macroblock(int *macroblock_type, int *motion_type, int *dct_type,
							 int PMV[2][2][2], int dc_dct_pred[3], 
							 int motion_vertical_field_select[2][2], int dmvector[2])
{
	int quantizer_scale_code, comp, motion_vector_count, mv_format; 
	int dmv, mvscale, coded_block_pattern;

	/* ISO/IEC 13818-2 section 6.3.17.1: Macroblock modes */
	macroblock_modes(macroblock_type, motion_type, &motion_vector_count, &mv_format,
					 &dmv, &mvscale, dct_type);
	if (Fault_Flag) return 0;	// trigger: go to next slice

	if (*macroblock_type & MACROBLOCK_QUANT)
	{
		quantizer_scale_code = Get_Bits(5);

		/* ISO/IEC 13818-2 section 7.4.2.2: Quantizer scale factor */
		quantizer_scale = q_scale_type ?
		Non_Linear_quantizer_scale[quantizer_scale_code] : (quantizer_scale_code << 1);
	}

	/* ISO/IEC 13818-2 section 6.3.17.2: Motion vectors */
	/* decode forward motion vectors */
	if ((*macroblock_type & MACROBLOCK_MOTION_FORWARD) 
		|| ((*macroblock_type & MACROBLOCK_INTRA) && concealment_motion_vectors))
		motion_vectors(PMV, dmvector, motion_vertical_field_select, 0,
		motion_vector_count, mv_format, f_code[0][0]-1, f_code[0][1]-1, dmv, mvscale);
	if (Fault_Flag) return 0;	// trigger: go to next slice

	/* decode backward motion vectors */
	if (*macroblock_type & MACROBLOCK_MOTION_BACKWARD)
		motion_vectors(PMV, dmvector, motion_vertical_field_select, 1,
		motion_vector_count,mv_format, f_code[1][0]-1, f_code[1][1]-1, 0, mvscale);
	if (Fault_Flag) return 0;  // trigger: go to next slice

	if ((*macroblock_type & MACROBLOCK_INTRA) && concealment_motion_vectors)
		Flush_Buffer(1);	// marker bit

	/* macroblock_pattern */
	/* ISO/IEC 13818-2 section 6.3.17.4: Coded block pattern */
	if (*macroblock_type & MACROBLOCK_PATTERN)
	{
		coded_block_pattern = Get_coded_block_pattern();

		if (chroma_format==CHROMA422)
			coded_block_pattern = (coded_block_pattern<<2) | Get_Bits(2);
		else if (chroma_format==CHROMA444)
			coded_block_pattern = (coded_block_pattern<<6) | Get_Bits(6);
	}
	else
	    coded_block_pattern = (*macroblock_type & MACROBLOCK_INTRA) ? (1<<block_count)-1 : 0;

	if (Fault_Flag) return 0;	// trigger: go to next slice

	Clear_Block(block_count);

	/* decode blocks */
	for (comp=0; comp<block_count; comp++)
	{
		if (coded_block_pattern & (1<<(block_count-1-comp)))
		{
			if (*macroblock_type & MACROBLOCK_INTRA)
				Decode_MPEG2_Intra_Block(comp, dc_dct_pred);
			else
				Decode_MPEG2_Non_Intra_Block(comp);
			if (Fault_Flag) return 0;	// trigger: go to next slice
		}
	}

	/* reset intra_dc predictors */
	/* ISO/IEC 13818-2 section 7.2.1: DC coefficients in intra blocks */
	if (!(*macroblock_type & MACROBLOCK_INTRA))
		dc_dct_pred[0]=dc_dct_pred[1]=dc_dct_pred[2]=0;

	/* reset motion vector predictors */
	if ((*macroblock_type & MACROBLOCK_INTRA) && !concealment_motion_vectors)
	{
		/* intra mb without concealment motion vectors */
		/* ISO/IEC 13818-2 section 7.6.3.4: Resetting motion vector predictors */
		PMV[0][0][0]=PMV[0][0][1]=PMV[1][0][0]=PMV[1][0][1]=0;
		PMV[0][1][0]=PMV[0][1][1]=PMV[1][1][0]=PMV[1][1][1]=0;
	}

	/* special "No_MC" macroblock_type case */
	/* ISO/IEC 13818-2 section 7.6.3.5: Prediction in P pictures */
	if ((picture_coding_type==P_TYPE) 
		&& !(*macroblock_type & (MACROBLOCK_MOTION_FORWARD|MACROBLOCK_INTRA)))
	{
		/* non-intra mb without forward mv in a P picture */
		/* ISO/IEC 13818-2 section 7.6.3.4: Resetting motion vector predictors */
		PMV[0][0][0]=PMV[0][0][1]=PMV[1][0][0]=PMV[1][0][1]=0;

		/* derive motion_type */
		/* ISO/IEC 13818-2 section 6.3.17.1: Macroblock modes, frame_motion_type */
		if (picture_structure==FRAME_PICTURE)
			*motion_type = MC_FRAME;
		else
		{
			*motion_type = MC_FIELD;
			motion_vertical_field_select[0][0] = (picture_structure==BOTTOM_FIELD);
		}
	}
	/* successfully decoded macroblock */
	return 1 ;
}

/* decode one intra coded MPEG-2 block */
void CMPEG2Dec::Decode_MPEG2_Intra_Block(int comp, int dc_dct_pred[])
{
	int val, i, j, sign, *qmat;
	unsigned int code;
	DCTtab *tab;
	short *bp;

	bp = block[comp];
	qmat = (comp<4 || chroma_format==CHROMA420) 
		? intra_quantizer_matrix : chroma_intra_quantizer_matrix;

	/* ISO/IEC 13818-2 section 7.2.1: decode DC coefficients */
	switch (cc_table[comp])
	{
		case 0:
			val = (dc_dct_pred[0]+= Get_Luma_DC_dct_diff());
			break;

		case 1:
			val = (dc_dct_pred[1]+= Get_Chroma_DC_dct_diff());
			break;

		case 2:
			val = (dc_dct_pred[2]+= Get_Chroma_DC_dct_diff());
			break;

		default: // ERROR
			ASSERT(FALSE);
			val = 0;
			break;
	}

	bp[0] = val << (3-intra_dc_precision);

	/* decode AC coefficients */
	for (i=1; ; i++)
	{
		code = Show_Bits(16);

		if (code>=16384 && !intra_vlc_format)
			tab = &DCTtabnext[(code>>12)-4];
		else if (code>=1024)
		{
			if (intra_vlc_format)
				tab = &DCTtab0a[(code>>8)-4];
			else
				tab = &DCTtab0[(code>>8)-4];
		}
		else if (code>=512)
		{
			if (intra_vlc_format)
				tab = &DCTtab1a[(code>>6)-8];
			else
				tab = &DCTtab1[(code>>6)-8];
		}
		else if (code>=256)
			tab = &DCTtab2[(code>>4)-16];
		else if (code>=128)
			tab = &DCTtab3[(code>>3)-16];
		else if (code>=64)
			tab = &DCTtab4[(code>>2)-16];
		else if (code>=32)
			tab = &DCTtab5[(code>>1)-16];
		else if (code>=16)
			tab = &DCTtab6[code-16];
		else
		{
			Fault_Flag = 1;
			return;
		}

		Flush_Buffer(tab->len);

		if (tab->run<64)
		{
			i+= tab->run;
			val = tab->level;
			sign = Get_Bits(1);
		}
		else if (tab->run==64) /* end_of_block */
			return;
		else /* escape */
		{
			i+= Get_Bits(6);
			val = Get_Bits(12);

			sign = (val>=2048);
			if (sign)
				val = 4096 - val;
		}

		j = scan[alternate_scan][i];

		val = (val * quantizer_scale * qmat[j]) >> 4;
		bp[j] = sign ? -val : val;
	}
}

/* decode one non-intra coded MPEG-2 block */
void CMPEG2Dec::Decode_MPEG2_Non_Intra_Block(int comp)
{
	int val, i, j, sign, *qmat;
	unsigned int code;
	DCTtab *tab;
	short *bp;

	bp = block[comp];
	qmat = (comp<4 || chroma_format==CHROMA420) 
		? non_intra_quantizer_matrix : chroma_non_intra_quantizer_matrix;

	/* decode AC coefficients */
	for (i=0; ; i++)
	{
		code = Show_Bits(16);

		if (code>=16384)
		{
			if (i==0)
				tab = &DCTtabfirst[(code>>12)-4];
			else
				tab = &DCTtabnext[(code>>12)-4];
		}
		else if (code>=1024)
			tab = &DCTtab0[(code>>8)-4];
		else if (code>=512)
			tab = &DCTtab1[(code>>6)-8];
		else if (code>=256)
			tab = &DCTtab2[(code>>4)-16];
		else if (code>=128)
			tab = &DCTtab3[(code>>3)-16];
		else if (code>=64)
			tab = &DCTtab4[(code>>2)-16];
		else if (code>=32)
			tab = &DCTtab5[(code>>1)-16];
		else if (code>=16)
			tab = &DCTtab6[code-16];
		else
		{
			Fault_Flag = 1;
			return;
		}

		Flush_Buffer(tab->len);

		if (tab->run<64)
		{
			i+= tab->run;
			val = tab->level;
			sign = Get_Bits(1);
		}
		else if (tab->run==64) /* end_of_block */
			return;
		else /* escape */
		{
			i+= Get_Bits(6);
			val = Get_Bits(12);

			sign = (val>=2048);
			if (sign)
				val = 4096 - val;
		}

		j = scan[alternate_scan][i];

		val = (((val<<1)+1) * quantizer_scale * qmat[j]) >> 5;
		bp[j] = sign ? -val : val;
	}
}

int CMPEG2Dec::Get_macroblock_type()
{
	int macroblock_type;

	switch (picture_coding_type)
	{
		case I_TYPE:
			macroblock_type = Get_I_macroblock_type();
			break;

		case P_TYPE:
			macroblock_type = Get_P_macroblock_type();
			break;

		case B_TYPE:
			macroblock_type = Get_B_macroblock_type();
			break;

		default: //ERROR
			ASSERT(FALSE);
			macroblock_type = 0;
			break;
	}

	return macroblock_type;
}

int CMPEG2Dec::Get_I_macroblock_type()
{
	if (Get_Bits(1))
		return 1;

	if (!Get_Bits(1))
		Fault_Flag = 2;

	return 17;
}

int CMPEG2Dec::Get_P_macroblock_type()
{
	int code;

	if ((code = Show_Bits(6))>=8)
	{
		code >>= 3;
		Flush_Buffer(PMBtab0[code].len);

		return PMBtab0[code].val;
	}

	if (code==0)
	{
		Fault_Flag = 2;
		return 0;
	}

	Flush_Buffer(PMBtab1[code].len);

	return PMBtab1[code].val;
}

int CMPEG2Dec::Get_B_macroblock_type()
{
	int code;

	if ((code = Show_Bits(6))>=8)
	{
		code >>= 2;
		Flush_Buffer(BMBtab0[code].len);

		return BMBtab0[code].val;
	}

	if (code==0)
	{
		Fault_Flag = 2;
		return 0;
	}

	Flush_Buffer(BMBtab1[code].len);

	return BMBtab1[code].val;
}

int CMPEG2Dec::Get_coded_block_pattern()
{
	int code;

	if ((code = Show_Bits(9))>=128)
	{
		code >>= 4;
		Flush_Buffer(CBPtab0[code].len);

		return CBPtab0[code].val;
	}

	if (code>=8)
	{
		code >>= 1;
		Flush_Buffer(CBPtab1[code].len);

		return CBPtab1[code].val;
	}

	if (code<1)
	{
		Fault_Flag = 3;
		return 0;
	}

	Flush_Buffer(CBPtab2[code].len);

	return CBPtab2[code].val;
}

int CMPEG2Dec::Get_macroblock_address_increment()
{
	int code, val;

	val = 0;

	while ((code = Show_Bits(11))<24)
	{
		if (code!=15) /* if not macroblock_stuffing */
		{
			if (code==8) /* if macroblock_escape */
				val+= 33;
			else
			{
				Fault_Flag = 4;
				return 1;
			}
		}
		Flush_Buffer(11);
	}

	/* macroblock_address_increment == 1 */
	/* ('1' is in the MSB position of the lookahead) */
	if (code>=1024)
	{
		Flush_Buffer(1);
		return val + 1;
	}

	/* codes 00010 ... 011xx */
	if (code>=128)
	{
		/* remove leading zeros */
		code >>= 6;
		Flush_Buffer(MBAtab1[code].len);
    
		return val + MBAtab1[code].val;
	}
  
	/* codes 00000011000 ... 0000111xxxx */
	code-= 24; /* remove common base */
	Flush_Buffer(MBAtab2[code].len);

	return val + MBAtab2[code].val;
}

/*
   parse VLC and perform dct_diff arithmetic.
   MPEG-2:  ISO/IEC 13818-2 section 7.2.1 

   Note: the arithmetic here is presented more elegantly than
   the spec, yet the results, dct_diff, are the same.
*/
int CMPEG2Dec::Get_Luma_DC_dct_diff()
{
	int code, size, dct_diff;

	/* decode length */
	code = Show_Bits(5);

	if (code<31)
	{
		size = DClumtab0[code].val;
		Flush_Buffer(DClumtab0[code].len);
	}
	else
	{
		code = Show_Bits(9) - 0x1f0;
		size = DClumtab1[code].val;
		Flush_Buffer(DClumtab1[code].len);
	}

	if (size==0)
		dct_diff = 0;
	else
	{
		dct_diff = Get_Bits(size);

		if ((dct_diff & (1<<(size-1)))==0)
			dct_diff-= (1<<size) - 1;
	}

	return dct_diff;
}

int CMPEG2Dec::Get_Chroma_DC_dct_diff()
{
	int code, size, dct_diff;

	/* decode length */
	code = Show_Bits(5);

	if (code<31)
	{
		size = DCchromtab0[code].val;
		Flush_Buffer(DCchromtab0[code].len);
	}
	else
	{
		code = Show_Bits(10) - 0x3e0;
		size = DCchromtab1[code].val;
		Flush_Buffer(DCchromtab1[code].len);
	}

	if (size==0)
		dct_diff = 0;
	else
	{
		dct_diff = Get_Bits(size);

		if ((dct_diff & (1<<(size-1)))==0)
			dct_diff-= (1<<size) - 1;
	}

	return dct_diff;
}

/*
static int currentfield;
static unsigned char **predframe;
static int DMV[2][2];
static int stw;
*/

void CMPEG2Dec::form_predictions(int bx, int by, int macroblock_type, int motion_type,
					  int PMV[2][2][2], int motion_vertical_field_select[2][2],
					  int dmvector[2])
{
	int currentfield;
	unsigned char **predframe;
	int DMV[2][2];
	int stw;
	
	stw = 0;

	if ((macroblock_type & MACROBLOCK_MOTION_FORWARD) || (picture_coding_type==P_TYPE))
	{
		if (picture_structure==FRAME_PICTURE)
		{
			if ((motion_type==MC_FRAME) || !(macroblock_type & MACROBLOCK_MOTION_FORWARD))
			{
				/* frame-based prediction (broken into top and bottom halves
				   for spatial scalability prediction purposes) */
				form_prediction(forward_reference_frame, 0, current_frame, 0, Coded_Picture_Width, 
					Coded_Picture_Width<<1, 16, 8, bx, by, PMV[0][0][0], PMV[0][0][1], stw);

				form_prediction(forward_reference_frame, 1, current_frame, 1, Coded_Picture_Width, 
					Coded_Picture_Width<<1, 16, 8, bx, by, PMV[0][0][0], PMV[0][0][1], stw);
			}
			else if (motion_type==MC_FIELD) /* field-based prediction */
			{
				/* top field prediction */
				form_prediction(forward_reference_frame, motion_vertical_field_select[0][0], 
					current_frame, 0, Coded_Picture_Width<<1, Coded_Picture_Width<<1, 16, 8,
					bx, by>>1, PMV[0][0][0], PMV[0][0][1]>>1, stw);

				/* bottom field prediction */
				form_prediction(forward_reference_frame, motion_vertical_field_select[1][0], 
					current_frame, 1, Coded_Picture_Width<<1, Coded_Picture_Width<<1, 16, 8,
					bx, by>>1, PMV[1][0][0], PMV[1][0][1]>>1, stw);
			}
			else if (motion_type==MC_DMV) /* dual prime prediction */
			{
				/* calculate derived motion vectors */
				Dual_Prime_Arithmetic(DMV, dmvector, PMV[0][0][0], PMV[0][0][1]>>1);

				/* predict top field from top field */
				form_prediction(forward_reference_frame, 0, current_frame, 0, 
					Coded_Picture_Width<<1, Coded_Picture_Width<<1, 16, 8, bx, by>>1,
					PMV[0][0][0], PMV[0][0][1]>>1, 0);

				/* predict and add to top field from bottom field */
				form_prediction(forward_reference_frame, 1, current_frame, 0,
					Coded_Picture_Width<<1, Coded_Picture_Width<<1, 16, 8, bx, by>>1,
					DMV[0][0], DMV[0][1], 1);

				/* predict bottom field from bottom field */
				form_prediction(forward_reference_frame, 1, current_frame, 1,
					Coded_Picture_Width<<1, Coded_Picture_Width<<1, 16, 8, bx, by>>1,
					PMV[0][0][0], PMV[0][0][1]>>1, 0);

				/* predict and add to bottom field from top field */
				form_prediction(forward_reference_frame, 0, current_frame, 1,
					Coded_Picture_Width<<1, Coded_Picture_Width<<1, 16, 8, bx, by>>1,
					DMV[1][0], DMV[1][1], 1);
			}
			else
				Fault_Flag = 5;
		}
		else
		{
			/* field picture */
			currentfield = (picture_structure==BOTTOM_FIELD);

			/* determine which frame to use for prediction */
			if (picture_coding_type==P_TYPE && Second_Field && currentfield!=motion_vertical_field_select[0][0])
				predframe = backward_reference_frame;
			else
				predframe = forward_reference_frame;

			if ((motion_type==MC_FIELD) || !(macroblock_type & MACROBLOCK_MOTION_FORWARD))
			{
				form_prediction(predframe, motion_vertical_field_select[0][0], current_frame, 0, 
					Coded_Picture_Width<<1, Coded_Picture_Width<<1, 16, 16, bx, by,
					PMV[0][0][0], PMV[0][0][1], stw);
			}
			else if (motion_type==MC_16X8)
			{
				form_prediction(predframe, motion_vertical_field_select[0][0], current_frame, 0, 
					Coded_Picture_Width<<1, Coded_Picture_Width<<1, 16, 8, bx, by,
					PMV[0][0][0], PMV[0][0][1], stw);

				if (picture_coding_type==P_TYPE && Second_Field && currentfield!=motion_vertical_field_select[1][0])
					predframe = backward_reference_frame;
				else
					predframe = forward_reference_frame;

				form_prediction(predframe, motion_vertical_field_select[1][0], current_frame, 
					0, Coded_Picture_Width<<1, Coded_Picture_Width<<1, 16, 8, bx, by+8,
					PMV[1][0][0], PMV[1][0][1], stw);
			}
			else if (motion_type==MC_DMV)
			{
				if (Second_Field)
					predframe = backward_reference_frame;
				else
					predframe = forward_reference_frame;

				/* calculate derived motion vectors */
				Dual_Prime_Arithmetic(DMV, dmvector, PMV[0][0][0], PMV[0][0][1]);

				/* predict from field of same parity */
				form_prediction(forward_reference_frame, currentfield, current_frame, 0, 
					Coded_Picture_Width<<1, Coded_Picture_Width<<1, 16, 16, bx, by,
					PMV[0][0][0], PMV[0][0][1], 0);

				/* predict from field of opposite parity */
				form_prediction(predframe, !currentfield, current_frame, 0,
					Coded_Picture_Width<<1, Coded_Picture_Width<<1, 16, 16, bx, by,
					DMV[0][0], DMV[0][1], 1);
			}
			else
				Fault_Flag = 5;
		}

		stw = 1;
	}

	if (macroblock_type & MACROBLOCK_MOTION_BACKWARD)
	{
		if (picture_structure==FRAME_PICTURE)
		{
			if (motion_type==MC_FRAME)
			{
				/* frame-based prediction */
				form_prediction(backward_reference_frame, 0, current_frame, 0,
					Coded_Picture_Width, Coded_Picture_Width<<1, 16, 8, bx, by,
					PMV[0][1][0], PMV[0][1][1], stw);

				form_prediction(backward_reference_frame, 1, current_frame, 1,
					Coded_Picture_Width, Coded_Picture_Width<<1, 16, 8, bx, by,
					PMV[0][1][0], PMV[0][1][1], stw);
			}
			else /* field-based prediction */
			{
				/* top field prediction */
				form_prediction(backward_reference_frame, motion_vertical_field_select[0][1], 
					current_frame, 0, Coded_Picture_Width<<1, Coded_Picture_Width<<1, 16, 8,
					bx, by>>1, PMV[0][1][0], PMV[0][1][1]>>1, stw);

				/* bottom field prediction */
				form_prediction(backward_reference_frame, motion_vertical_field_select[1][1], 
					current_frame, 1, Coded_Picture_Width<<1, Coded_Picture_Width<<1, 16, 8,
					bx, by>>1, PMV[1][1][0], PMV[1][1][1]>>1, stw);
			}
		}
		else
		{
			/* field picture */
			if (motion_type==MC_FIELD)
			{
				/* field-based prediction */
				form_prediction(backward_reference_frame, motion_vertical_field_select[0][1], 
					current_frame, 0, Coded_Picture_Width<<1, Coded_Picture_Width<<1, 16, 16,
					bx, by, PMV[0][1][0], PMV[0][1][1], stw);
			}
			else if (motion_type==MC_16X8)
			{
				form_prediction(backward_reference_frame, motion_vertical_field_select[0][1],
					current_frame, 0, Coded_Picture_Width<<1, Coded_Picture_Width<<1, 16, 8,
					bx, by, PMV[0][1][0], PMV[0][1][1], stw);

				form_prediction(backward_reference_frame, motion_vertical_field_select[1][1],
					current_frame, 0, Coded_Picture_Width<<1, Coded_Picture_Width<<1, 16, 8,
					bx, by+8, PMV[1][1][0], PMV[1][1][1], stw);
			}
			else
				Fault_Flag = 5;
		}
	}
}

void CMPEG2Dec::form_prediction(unsigned char *src[], int sfield, unsigned char *dst[],
							int dfield, int lx, int lx2, int w, int h, int x, int y,
							int dx, int dy, int average_flag)
{
	form_component_prediction(src[0]+(sfield?lx2>>1:0), dst[0]+(dfield?lx2>>1:0),
		lx, lx2, w, h, x, y, dx, dy, average_flag);

	if (chroma_format!=CHROMA444)
	{
		lx>>=1; lx2>>=1; w>>=1; x>>=1; dx/=2;
	}

	if (chroma_format==CHROMA420)
	{
		h>>=1; y>>=1; dy/=2;
	}

	/* Cb */
	form_component_prediction(src[1]+(sfield?lx2>>1:0), dst[1]+(dfield?lx2>>1:0),
		lx, lx2, w, h, x, y, dx, dy, average_flag);

	/* Cr */
	form_component_prediction(src[2]+(sfield?lx2>>1:0), dst[2]+(dfield?lx2>>1:0),
		lx, lx2, w, h, x, y, dx, dy, average_flag);
}

/* ISO/IEC 13818-2 section 7.6.4: Forming predictions */
void CMPEG2Dec::form_component_prediction(unsigned char *src, unsigned char *dst,
										  int lx, int lx2, int w, int h, int x, int y,
										  int dx, int dy, int average_flag)
{
	static const __int64 mmmask_0001 = 0x0001000100010001;
	static const __int64 mmmask_0002 = 0x0002000200020002;
	static const __int64 mmmask_0003 = 0x0003000300030003;
	static const __int64 mmmask_0006 = 0x0006000600060006;

	unsigned char *s = src + lx * (y + (dy>>1)) + x + (dx>>1);
	unsigned char *d = dst + lx * y + x;
	int flag = (average_flag<<2) + ((dx & 1)<<1) + (dy & 1);

	switch (flag)
	{
		case 0:
			// d[i] = s[i];
			__asm
			{
				mov			eax, [s]
				mov			ebx, [d]
				mov			esi, 0x00
				mov			edi, [h]
mc0:
				movq		mm1, [eax+esi]
				add			esi, 0x08
				cmp			esi, [w]
				movq		[ebx+esi-8], mm1
				jl			mc0

				add			eax, [lx2]
				add			ebx, [lx2]
				dec			edi
				mov			esi, 0x00
				cmp			edi, 0x00
				jg			mc0
			}
			break;

		case 1:
			// d[i] = (s[i]+s[i+lx]+1)>>1;
			__asm
			{
				pxor		mm0, mm0
				movq		mm7, [mmmask_0001]
				mov			eax, [s]
				mov			ebx, [d]
				mov			ecx, eax
				add			ecx, [lx]
				mov			esi, 0x00
				mov			edi, [h]
mc1:
				movq		mm1, [eax+esi]
				movq		mm2, [ecx+esi]

				movq		mm3, mm1
				movq		mm4, mm2

				punpcklbw	mm1, mm0
				punpckhbw	mm3, mm0
				punpcklbw	mm2, mm0
				punpckhbw	mm4, mm0

				paddsw		mm1, mm2
				paddsw		mm3, mm4

				paddsw		mm1, mm7
				paddsw		mm3, mm7

				psrlw		mm1, 1
				psrlw		mm3, 1

				packuswb	mm1, mm0
				packuswb	mm3, mm0

				psllq		mm3, 32
				por			mm1, mm3

				add			esi, 0x08
				cmp			esi, [w]
				movq		[ebx+esi-8], mm1
				jl			mc1

				add			eax, [lx2]
				add			ebx, [lx2]
				add			ecx, [lx2]
				dec			edi
				mov			esi, 0x00
				cmp			edi, 0x00
				jg			mc1
			}
			break;

		case 2:
			// d[i] = (s[i]+s[i+1]+1)>>1;
			__asm
			{
				pxor		mm0, mm0
				movq		mm7, [mmmask_0001]
				mov			eax, [s]
				mov			ebx, [d]
				mov			esi, 0x00
				mov			edi, [h]
mc2:
				movq		mm1, [eax+esi]
				movq		mm2, [eax+esi+1]

				movq		mm3, mm1
				movq		mm4, mm2

				punpcklbw	mm1, mm0
				punpckhbw	mm3, mm0

				punpcklbw	mm2, mm0
				punpckhbw	mm4, mm0

				paddsw		mm1, mm2
				paddsw		mm3, mm4

				paddsw		mm1, mm7
				paddsw		mm3, mm7

				psrlw		mm1, 1
				psrlw		mm3, 1

				packuswb	mm1, mm0
				packuswb	mm3, mm0

				psllq		mm3, 32
				por			mm1, mm3

				add			esi, 0x08
				cmp			esi, [w]
				movq		[ebx+esi-8], mm1
				jl			mc2

				add			eax, [lx2]
				add			ebx, [lx2]
				dec			edi
				mov			esi, 0x00
				cmp			edi, 0x00
				jg			mc2
			}
			break;

		case 3:
			// d[i] = (s[i]+s[i+1]+s[i+lx]+s[i+lx+1]+2)>>2;
			__asm
			{
				pxor		mm0, mm0
				movq		mm7, [mmmask_0002]
				mov			eax, [s]
				mov			ebx, [d]
				mov			ecx, eax
				add			ecx, [lx]
				mov			esi, 0x00
				mov			edi, [h]
mc3:
				movq		mm1, [eax+esi]
				movq		mm2, [eax+esi+1]
				movq		mm3, mm1
				movq		mm4, mm2

				punpcklbw	mm1, mm0
				punpckhbw	mm3, mm0

				punpcklbw	mm2, mm0
				punpckhbw	mm4, mm0

				paddsw		mm1, mm2
				paddsw		mm3, mm4

				movq		mm5, [ecx+esi]
				paddsw		mm1, mm7

				movq		mm6, [ecx+esi+1]
				paddsw		mm3, mm7

				movq		mm2, mm5
				movq		mm4, mm6

				punpcklbw	mm2, mm0
				punpckhbw	mm5, mm0

				punpcklbw	mm4, mm0
				punpckhbw	mm6, mm0
				
				paddsw		mm2, mm4
				paddsw		mm5, mm6

				paddsw		mm1, mm2
				paddsw		mm3, mm5

				psrlw		mm1, 2
				psrlw		mm3, 2

				packuswb	mm1, mm0
				packuswb	mm3, mm0

				psllq		mm3, 32
				por			mm1, mm3

				add			esi, 0x08
				cmp			esi, [w]
				movq		[ebx+esi-8], mm1
				jl			mc3

				add			eax, [lx2]
				add			ebx, [lx2]
				add			ecx, [lx2]
				dec			edi
				mov			esi, 0x00
				cmp			edi, 0x00
				jg			mc3
			}
			break;

		case 4:
			// d[i] = (s[i]+d[i]+1)>>1;
			__asm
			{
				pxor		mm0, mm0
				movq		mm7, [mmmask_0001]
				mov			eax, [s]
				mov			ebx, [d]
				mov			esi, 0x00
				mov			edi, [h]
mc4:
				movq		mm1, [eax+esi]
				movq		mm2, [ebx+esi]
				movq		mm3, mm1
				movq		mm4, mm2

				punpcklbw	mm1, mm0
				punpckhbw	mm3, mm0

				punpcklbw	mm2, mm0
				punpckhbw	mm4, mm0

				paddsw		mm1, mm2
				paddsw		mm3, mm4

				paddsw		mm1, mm7
				paddsw		mm3, mm7

				psrlw		mm1, 1
				psrlw		mm3, 1

				packuswb	mm1, mm0
				packuswb	mm3, mm0

				psllq		mm3, 32
				por			mm1, mm3

				add			esi, 0x08
				cmp			esi, [w]
				movq		[ebx+esi-8], mm1
				jl			mc4

				add			eax, [lx2]
				add			ebx, [lx2]
				dec			edi
				mov			esi, 0x00
				cmp			edi, 0x00
				jg			mc4
			}
			break;

		case 5:
			// d[i] = ((d[i]<<1) + s[i]+s[i+lx] + 3)>>2;
			__asm
			{
				pxor		mm0, mm0
				movq		mm7, [mmmask_0003]
				mov			eax, [s]
				mov			ebx, [d]
				mov			ecx, eax
				add			ecx, [lx]
				mov			esi, 0x00
				mov			edi, [h]
mc5:
				movq		mm1, [eax+esi]
				movq		mm2, [ecx+esi]
				movq		mm3, mm1
				movq		mm4, mm2

				punpcklbw	mm1, mm0
				punpckhbw	mm3, mm0

				punpcklbw	mm2, mm0
				punpckhbw	mm4, mm0

				paddsw		mm1, mm2
				paddsw		mm3, mm4

				movq		mm5, [ebx+esi]

				paddsw		mm1, mm7
				paddsw		mm3, mm7

				movq		mm6, mm5
				punpcklbw	mm5, mm0
				punpckhbw	mm6, mm0

				psllw		mm5, 1
				psllw		mm6, 1

				paddsw		mm1, mm5
				paddsw		mm3, mm6

				psrlw		mm1, 2
				psrlw		mm3, 2

				packuswb	mm1, mm0
				packuswb	mm3, mm0

				psllq		mm3, 32
				por			mm1, mm3

				add			esi, 0x08
				cmp			esi, [w]
				movq		[ebx+esi-8], mm1
				jl			mc5

				add			eax, [lx2]
				add			ebx, [lx2]
				add			ecx, [lx2]
				dec			edi
				mov			esi, 0x00
				cmp			edi, 0x00
				jg			mc5
			}
			break;

		case 6:
			// d[i] = ((d[i]<<1) + s[i]+s[i+1] + 3) >> 2;
			__asm
			{
				pxor		mm0, mm0
				movq		mm7, [mmmask_0003]
				mov			eax, [s]
				mov			ebx, [d]
				mov			esi, 0x00
				mov			edi, [h]
mc6:
				movq		mm1, [eax+esi]
				movq		mm2, [eax+esi+1]
				movq		mm3, mm1
				movq		mm4, mm2

				punpcklbw	mm1, mm0
				punpckhbw	mm3, mm0

				punpcklbw	mm2, mm0
				punpckhbw	mm4, mm0

				paddsw		mm1, mm2
				paddsw		mm3, mm4

				movq		mm5, [ebx+esi]

				paddsw		mm1, mm7
				paddsw		mm3, mm7

				movq		mm6, mm5
				punpcklbw	mm5, mm0
				punpckhbw	mm6, mm0

				psllw		mm5, 1
				psllw		mm6, 1

				paddsw		mm1, mm5
				paddsw		mm3, mm6

				psrlw		mm1, 2
				psrlw		mm3, 2

				packuswb	mm1, mm0
				packuswb	mm3, mm0

				psllq		mm3, 32
				por			mm1, mm3

				add			esi, 0x08
				cmp			esi, [w]
				movq		[ebx+esi-8], mm1
				jl			mc6

				add			eax, [lx2]
				add			ebx, [lx2]
				dec			edi
				mov			esi, 0x00
				cmp			edi, 0x00
				jg			mc6
			}
			break;

		case 7:
			// d[i] = ((d[i]<<2) + s[i]+s[i+1]+s[i+lx]+s[i+lx+1] + 6)>>3;
			__asm
			{
				pxor		mm0, mm0
				movq		mm7, [mmmask_0006]
				mov			eax, [s]
				mov			ebx, [d]
				mov			ecx, eax
				add			ecx, [lx]
				mov			esi, 0x00
				mov			edi, [h]
mc7:
				movq		mm1, [eax+esi]
				movq		mm2, [eax+esi+1]
				movq		mm3, mm1
				movq		mm4, mm2

				punpcklbw	mm1, mm0
				punpckhbw	mm3, mm0

				punpcklbw	mm2, mm0
				punpckhbw	mm4, mm0

				paddsw		mm1, mm2
				paddsw		mm3, mm4

				movq		mm5, [ecx+esi]
				paddsw		mm1, mm7

				movq		mm6, [ecx+esi+1]
				paddsw		mm3, mm7

				movq		mm2, mm5
				movq		mm4, mm6

				punpcklbw	mm2, mm0
				punpckhbw	mm5, mm0

				punpcklbw	mm4, mm0
				punpckhbw	mm6, mm0
				
				paddsw		mm2, mm4
				paddsw		mm5, mm6

				paddsw		mm1, mm2
				paddsw		mm3, mm5

				movq		mm6, [ebx+esi]

				movq		mm4, mm6
				punpcklbw	mm4, mm0
				punpckhbw	mm6, mm0

				psllw		mm4, 2
				psllw		mm6, 2

				paddsw		mm1, mm4
				paddsw		mm3, mm6

				psrlw		mm1, 3
				psrlw		mm3, 3

				packuswb	mm1, mm0
				packuswb	mm3, mm0

				psllq		mm3, 32
				por			mm1, mm3

				add			esi, 0x08
				cmp			esi, [w]
				movq		[ebx+esi-8], mm1
				jl			mc7

				add			eax, [lx2]
				add			ebx, [lx2]
				add			ecx, [lx2]
				dec			edi
				mov			esi, 0x00
				cmp			edi, 0x00
				jg			mc7
			}
			break;
	}
}

//
// motion
//

/* ISO/IEC 13818-2 sections 6.2.5.2, 6.3.17.2, and 7.6.3: Motion vectors */
void CMPEG2Dec::motion_vectors(int PMV[2][2][2],int dmvector[2],
					int motion_vertical_field_select[2][2], int s,
					int motion_vector_count, int mv_format, int h_r_size,
					int v_r_size, int dmv, int mvscale)
{
	if (motion_vector_count==1)
	{
		if (mv_format==MV_FIELD && !dmv)
			motion_vertical_field_select[1][s] =
			motion_vertical_field_select[0][s] = Get_Bits(1);

		motion_vector(PMV[0][s],dmvector,h_r_size,v_r_size,dmv,mvscale,0);

		/* update other motion vector predictors */
		PMV[1][s][0] = PMV[0][s][0];
		PMV[1][s][1] = PMV[0][s][1];
	}
	else
	{
		motion_vertical_field_select[0][s] = Get_Bits(1);
		motion_vector(PMV[0][s],dmvector,h_r_size,v_r_size,dmv,mvscale,0);
		motion_vertical_field_select[1][s] = Get_Bits(1);
		motion_vector(PMV[1][s],dmvector,h_r_size,v_r_size,dmv,mvscale,0);
	}
}

/* ISO/IEC 13818-2 section 7.6.3.6: Dual prime additional arithmetic */
void CMPEG2Dec::Dual_Prime_Arithmetic(int DMV[][2],int *dmvector, int mvx,int mvy)
{
	if (picture_structure==FRAME_PICTURE)
	{
		if (top_field_first)
		{
			/* vector for prediction of top field from bottom field */
			DMV[0][0] = ((mvx  +(mvx>0))>>1) + dmvector[0];
			DMV[0][1] = ((mvy  +(mvy>0))>>1) + dmvector[1] - 1;

			/* vector for prediction of bottom field from top field */
			DMV[1][0] = ((3*mvx+(mvx>0))>>1) + dmvector[0];
			DMV[1][1] = ((3*mvy+(mvy>0))>>1) + dmvector[1] + 1;
		}
		else
		{
			/* vector for prediction of top field from bottom field */
			DMV[0][0] = ((3*mvx+(mvx>0))>>1) + dmvector[0];
			DMV[0][1] = ((3*mvy+(mvy>0))>>1) + dmvector[1] - 1;

			/* vector for prediction of bottom field from top field */
			DMV[1][0] = ((mvx  +(mvx>0))>>1) + dmvector[0];
			DMV[1][1] = ((mvy  +(mvy>0))>>1) + dmvector[1] + 1;
		}
	}
	else
	{
		/* vector for prediction from field of opposite 'parity' */
		DMV[0][0] = ((mvx+(mvx>0))>>1) + dmvector[0];
		DMV[0][1] = ((mvy+(mvy>0))>>1) + dmvector[1];

		/* correct for vertical field shift */
		if (picture_structure==TOP_FIELD)
			DMV[0][1]--;
		else
			DMV[0][1]++;
	}
}

/* get and decode motion vector and differential motion vector for one prediction */
void CMPEG2Dec::motion_vector(int *PMV, int *dmvector, int h_r_size, int v_r_size,
				   int dmv, int mvscale, int full_pel_vector)
{
	int motion_code, motion_residual;

	/* horizontal component */
	/* ISO/IEC 13818-2 Table B-10 */
	motion_code = Get_motion_code();

	motion_residual = (h_r_size!=0 && motion_code!=0) ? Get_Bits(h_r_size) : 0;

	decode_motion_vector(&PMV[0],h_r_size,motion_code,motion_residual,full_pel_vector);

	if (dmv)
		dmvector[0] = Get_dmvector();

	/* vertical component */
	motion_code     = Get_motion_code();
	motion_residual = (v_r_size!=0 && motion_code!=0) ? Get_Bits(v_r_size) : 0;

	if (mvscale)
		PMV[1] >>= 1; /* DIV 2 */

	decode_motion_vector(&PMV[1],v_r_size,motion_code,motion_residual,full_pel_vector);

	if (mvscale)
		PMV[1] <<= 1;

	if (dmv)
		dmvector[1] = Get_dmvector();
}

/* calculate motion vector component */
/* ISO/IEC 13818-2 section 7.6.3.1: Decoding the motion vectors */
/* Note: the arithmetic here is more elegant than that which is shown 
   in 7.6.3.1.  The end results (PMV[][][]) should, however, be the same.  */
void CMPEG2Dec::decode_motion_vector(int *pred, int r_size, int motion_code,
								 int motion_residual, int full_pel_vector)
{
	int lim, vec;

	lim = 16<<r_size;
	vec = full_pel_vector ? (*pred >> 1) : (*pred);

	if (motion_code>0)
	{
		vec+= ((motion_code-1)<<r_size) + motion_residual + 1;
		if (vec>=lim)
			vec-= lim + lim;
	}
	else if (motion_code<0)
	{
		vec-= ((-motion_code-1)<<r_size) + motion_residual + 1;
		if (vec<-lim)
			vec+= lim + lim;
	}

	*pred = full_pel_vector ? (vec<<1) : vec;
}

int CMPEG2Dec::Get_motion_code()
{
	int code;

	if (Get_Bits(1))
		return 0;

	if ((code = Show_Bits(9))>=64)
	{
		code >>= 6;
		Flush_Buffer(MVtab0[code].len);

		return Get_Bits(1)?-MVtab0[code].val:MVtab0[code].val;
	}

	if (code>=24)
	{
		code >>= 3;
		Flush_Buffer(MVtab1[code].len);

		return Get_Bits(1)?-MVtab1[code].val:MVtab1[code].val;
	}

	if ((code-=12)<0)
	{
		Fault_Flag = 10;
		return 0;
	}

	Flush_Buffer(MVtab2[code].len);

	return Get_Bits(1) ? -MVtab2[code].val : MVtab2[code].val;
}

/* get differential motion vector (for dual prime prediction) */
int CMPEG2Dec::Get_dmvector()
{
	if (Get_Bits(1))
		return Get_Bits(1) ? -1 : 1;
	else
		return 0;
}

//
// store
//

static const __int64 mmmask_0001 = 0x0001000100010001;
static const __int64 mmmask_0002 = 0x0002000200020002;
static const __int64 mmmask_0003 = 0x0003000300030003;
static const __int64 mmmask_0004 = 0x0004000400040004;
static const __int64 mmmask_0005 = 0x0005000500050005;
static const __int64 mmmask_0007 = 0x0007000700070007;
static const __int64 mmmask_0016 = 0x0010001000100010;
static const __int64 mmmask_0040 = 0x0040004000400040;
static const __int64 mmmask_0128 = 0x0080008000800080;

void CMPEG2Dec::assembleFrame(unsigned char *src[], int pf, unsigned char *dst, int pitch)
{
	unsigned char *y444;

	if (Fault_Flag)
		Fault_Flag = 0;

	if (Luminance_Flag)
	{
		Luminance_Filter(src[0], lum);
		y444 = lum;
	}
	else
		y444 = src[0];

	if (chroma_format==CHROMA420)
	{
		conv420to422(src[1], u422, pf);
		conv420to422(src[2], v422, pf);

		if (!dstYUY2())
		{
			conv422to444(u422, u444);
			conv422to444(v422, v444);
		}
	}
	else if (!dstYUY2())
	{
		conv422to444(src[1], u444);
		conv422to444(src[2], v444);
	}

	if (dstYUY2())
		conv422toYUY2(y444, u422, v422, dst, pitch);
	else
 		conv444toRGB24(y444, u444, v444, dst, pitch);
}

void CMPEG2Dec::Luminance_Filter(unsigned char *src, unsigned char *dst)
{
	src += CLIP_AREA;
	dst += CLIP_AREA;

	__asm
	{
		mov			edx, this
		mov			eax, [src]
		mov			ebx, [dst]
		mov			esi, 0x00
		mov			edi, [edx].LUM_AREA
		pxor		mm0, mm0
		movq		mm5, [edx].LumOffsetMask
		movq		mm6, [edx].LumGainMask
		movq		mm7, mmmask_0040

lumconv:
		movq		mm1, [eax+esi]
		movq		mm2, mm1

		punpcklbw	mm1, mm0
		punpckhbw	mm2, mm0

		pmullw		mm1, mm6
		pmullw		mm2, mm6

		paddw		mm1, mm7
		paddw		mm2, mm7

		psrlw		mm1, 7
		psrlw		mm2, 7

		paddw		mm1, mm5
		paddw		mm2, mm5

		packuswb	mm1, mm0
		packuswb	mm2, mm0

		add			esi, 0x08
		cmp			esi, edi
		movq		[ebx+esi-8], mm1
		movq		[ebx+esi-4], mm2
		jl			lumconv
	}
}

void CMPEG2Dec::conv422to444(unsigned char *src, unsigned char *dst)
{
	src += HALF_CLIP_AREA;
	dst += CLIP_AREA;

	__asm
	{
		mov			edx, this
		mov			eax, [src]
		mov			ebx, [dst]
		mov			edi, [edx].Clip_Height

		movq		mm1, [mmmask_0001]
		pxor		mm0, mm0

convyuv444init:
		movq		mm7, [eax]
		mov			esi, 0x00

convyuv444:
		movq		mm2, mm7
		movq		mm7, [eax+esi+8]
		movq		mm3, mm2
		movq		mm4, mm7

		psrlq		mm3, 8
		psllq		mm4, 56
		por			mm3, mm4

		movq		mm4, mm2
		movq		mm5, mm3

		punpcklbw	mm4, mm0
		punpcklbw	mm5, mm0

		movq		mm6, mm4
		paddusw		mm4, mm1
		paddusw		mm4, mm5
		psrlw		mm4, 1
		psllq		mm4, 8
		por			mm4, mm6

		punpckhbw	mm2, mm0
		punpckhbw	mm3, mm0

		movq		mm6, mm2
		paddusw		mm2, mm1
		paddusw		mm2, mm3

		movq		[ebx+esi*2], mm4

		psrlw		mm2, 1
		psllq		mm2, 8
		por			mm2, mm6

		add			esi, 0x08
		cmp			esi, [edx].HALF_WIDTH_D8
		movq		[ebx+esi*2-8], mm2
		jl			convyuv444

		movq		mm2, mm7
		punpcklbw	mm2, mm0
		movq		mm3, mm2

		psllq		mm2, 8
		por			mm2, mm3

		movq		[ebx+esi*2], mm2

		punpckhbw	mm7, mm0
		movq		mm6, mm7

		psllq		mm6, 8
		por			mm6, mm7

		movq		[ebx+esi*2+8], mm6

		add			eax, [edx].HALF_WIDTH
		add			ebx, [edx].Coded_Picture_Width
		dec			edi
		cmp			edi, 0x00
		jg			convyuv444init
	}
}

void CMPEG2Dec::conv420to422(unsigned char *src, unsigned char *dst, int frame_type)
{
	if (frame_type)
	{
		__asm
		{
			push		ebp
			mov			eax, [src]
			mov			ebx, [dst]
			mov			ebp, this
			mov			ecx, ebx
			add			ecx, ds:[ebp].HALF_WIDTH
			mov			esi, 0x00
			movq		mm3, [mmmask_0003]
			pxor		mm0, mm0
			movq		mm4, [mmmask_0002]

			mov			edx, eax
			add			edx, ds:[ebp].HALF_WIDTH
convyuv422topp:
			movd		mm1, [eax+esi]
			movd		mm2, [edx+esi]
			movd		[ebx+esi], mm1
			punpcklbw	mm1, mm0
			pmullw		mm1, mm3
			paddusw		mm1, mm4
			punpcklbw	mm2, mm0
			paddusw		mm2, mm1
			psrlw		mm2, 0x02
			packuswb	mm2, mm0

			add			esi, 0x04
			cmp			esi, ds:[ebp].HALF_WIDTH
			movd		[ecx+esi-4], mm2
			jl			convyuv422topp

			add			eax, ds:[ebp].HALF_WIDTH
			add			ebx, ds:[ebp].Coded_Picture_Width
			add			ecx, ds:[ebp].Coded_Picture_Width
			mov			esi, 0x00

			mov			edi, ds:[ebp].PROGRESSIVE_HEIGHT
convyuv422p:
			movd		mm1, [eax+esi]

			punpcklbw	mm1, mm0
			mov			edx, eax

			pmullw		mm1, mm3
			sub			edx, ds:[ebp].HALF_WIDTH

			movd		mm5, [edx+esi]
			movd		mm2, [edx+esi]

			punpcklbw	mm5, mm0
			punpcklbw	mm2, mm0
			paddusw		mm5, mm1
			paddusw		mm2, mm1
			paddusw		mm5, mm4
			paddusw		mm2, mm4
			psrlw		mm5, 0x02
			psrlw		mm2, 0x02
			packuswb	mm5, mm0
			packuswb	mm2, mm0

			mov			edx, eax
			add			edx, ds:[ebp].HALF_WIDTH
			add			esi, 0x04
			cmp			esi, ds:[ebp].HALF_WIDTH
			movd		[ebx+esi-4], mm5
			movd		[ecx+esi-4], mm2

			jl			convyuv422p

			add			eax, ds:[ebp].HALF_WIDTH
			add			ebx, ds:[ebp].Coded_Picture_Width
			add			ecx, ds:[ebp].Coded_Picture_Width
			mov			esi, 0x00
			dec			edi
			cmp			edi, 0x00
			jg			convyuv422p

			mov			edx, eax
			sub			edx, ds:[ebp].HALF_WIDTH
convyuv422bottomp:
			movd		mm1, [eax+esi]
			movd		mm5, [edx+esi]
			punpcklbw	mm5, mm0
			movd		[ecx+esi], mm1

			punpcklbw	mm1, mm0
			pmullw		mm1, mm3
			paddusw		mm5, mm1
			paddusw		mm5, mm4
			psrlw		mm5, 0x02
			packuswb	mm5, mm0

			add			esi, 0x04
			cmp			esi, ds:[ebp].HALF_WIDTH
			movd		[ebx+esi-4], mm5
			jl			convyuv422bottomp
			pop			ebp
		}
	}
	else
	{
		__asm
		{
			push		ebp
			mov			eax, [src]
			mov			ecx, [dst]
			mov			ebp, this
			mov			esi, 0x00
			pxor		mm0, mm0
			movq		mm3, [mmmask_0003]
			movq		mm4, [mmmask_0004]
			movq		mm5, [mmmask_0005]

convyuv422topi:
			movd		mm1, [eax+esi]
			mov			ebx, eax
			add			ebx, ds:[ebp].HALF_WIDTH
			movd		mm2, [ebx+esi]
			movd		[ecx+esi], mm1
			punpcklbw	mm1, mm0
			movq		mm6, mm1
			pmullw		mm1, mm3

			punpcklbw	mm2, mm0
			movq		mm7, mm2
			pmullw		mm2, mm5
			paddusw		mm2, mm1
			paddusw		mm2, mm4
			psrlw		mm2, 0x03
			packuswb	mm2, mm0

			mov			edx, ecx
			add			edx, ds:[ebp].HALF_WIDTH
			pmullw		mm6, mm5
			movd		[edx+esi], mm2

			add			ebx, ds:[ebp].HALF_WIDTH
			movd		mm2, [ebx+esi]
			punpcklbw	mm2, mm0
			pmullw		mm2, mm3
			paddusw		mm2, mm6
			paddusw		mm2, mm4
			psrlw		mm2, 0x03
			packuswb	mm2, mm0

			add			edx, ds:[ebp].HALF_WIDTH
			add			ebx, ds:[ebp].HALF_WIDTH
			pmullw		mm7, [mmmask_0007]
			movd		[edx+esi], mm2

			movd		mm2, [ebx+esi]
			punpcklbw	mm2, mm0
			paddusw		mm2, mm7
			paddusw		mm2, mm4
			psrlw		mm2, 0x03
			packuswb	mm2, mm0

			add			edx, ds:[ebp].HALF_WIDTH
			add			esi, 0x04
			cmp			esi, ds:[ebp].HALF_WIDTH
			movd		[edx+esi-4], mm2

			jl			convyuv422topi

			add			eax, ds:[ebp].Coded_Picture_Width
			add			ecx, ds:[ebp].DOUBLE_WIDTH
			mov			esi, 0x00

			mov			edi, ds:[ebp].INTERLACED_HEIGHT
convyuv422i:
			movd		mm1, [eax+esi]
			punpcklbw	mm1, mm0
			movq		mm6, mm1
			mov			ebx, eax
			sub			ebx, ds:[ebp].Coded_Picture_Width
			movd		mm3, [ebx+esi]
			pmullw		mm1, [mmmask_0007]
			punpcklbw	mm3, mm0
			paddusw		mm3, mm1
			paddusw		mm3, mm4
			psrlw		mm3, 0x03
			packuswb	mm3, mm0

			add			ebx, ds:[ebp].HALF_WIDTH
			movq		mm1, [ebx+esi]
			add			ebx, ds:[ebp].Coded_Picture_Width
			movd		[ecx+esi], mm3

			movq		mm3, [mmmask_0003]
			movd		mm2, [ebx+esi]

			punpcklbw	mm1, mm0
			pmullw		mm1, mm3
			punpcklbw	mm2, mm0
			movq		mm7, mm2
			pmullw		mm2, mm5
			paddusw		mm2, mm1
			paddusw		mm2, mm4
			psrlw		mm2, 0x03
			packuswb	mm2, mm0

			pmullw		mm6, mm5
			mov			edx, ecx
			add			edx, ds:[ebp].HALF_WIDTH
			movd		[edx+esi], mm2

			add			ebx, ds:[ebp].HALF_WIDTH
			movd		mm2, [ebx+esi]
			punpcklbw	mm2, mm0
			pmullw		mm2, mm3
			paddusw		mm2, mm6
			paddusw		mm2, mm4
			psrlw		mm2, 0x03
			packuswb	mm2, mm0

			pmullw		mm7, [mmmask_0007]
			add			edx, ds:[ebp].HALF_WIDTH
			add			ebx, ds:[ebp].HALF_WIDTH
 			movd		[edx+esi], mm2

			movd		mm2, [ebx+esi]
			punpcklbw	mm2, mm0
			paddusw		mm2, mm7
			paddusw		mm2, mm4
			psrlw		mm2, 0x03
			packuswb	mm2, mm0

			add			edx, ds:[ebp].HALF_WIDTH
			add			esi, 0x04
			cmp			esi, ds:[ebp].HALF_WIDTH
			movd		[edx+esi-4], mm2

			jl			convyuv422i
			add			eax, ds:[ebp].Coded_Picture_Width
			add			ecx, ds:[ebp].DOUBLE_WIDTH
			mov			esi, 0x00
			dec			edi
			cmp			edi, 0x00
			jg			convyuv422i

convyuv422bottomi:
			movd		mm1, [eax+esi]
			movq		mm6, mm1
			punpcklbw	mm1, mm0
			mov			ebx, eax
			sub			ebx, ds:[ebp].Coded_Picture_Width
			movd		mm3, [ebx+esi]
			punpcklbw	mm3, mm0
			pmullw		mm1, [mmmask_0007]
			paddusw		mm3, mm1
			paddusw		mm3, mm4
			psrlw		mm3, 0x03
			packuswb	mm3, mm0

			add			ebx, ds:[ebp].HALF_WIDTH
			movq		mm1, [ebx+esi]
			punpcklbw	mm1, mm0
			movd		[ecx+esi], mm3

			pmullw		mm1, [mmmask_0003]
			add			ebx, ds:[ebp].Coded_Picture_Width
			movd		mm2, [ebx+esi]
			punpcklbw	mm2, mm0
			movq		mm7, mm2
			pmullw		mm2, mm5
			paddusw		mm2, mm1
			paddusw		mm2, mm4
			psrlw		mm2, 0x03
			packuswb	mm2, mm0

			mov			edx, ecx
			add			edx, ds:[ebp].HALF_WIDTH
			pmullw		mm7, [mmmask_0007]
			movd		[edx+esi], mm2

			add			edx, ds:[ebp].HALF_WIDTH
 			movd		[edx+esi], mm6

			punpcklbw	mm6, mm0
			paddusw		mm6, mm7
			paddusw		mm6, mm4
			psrlw		mm6, 0x03
			packuswb	mm6, mm0

			add			edx, ds:[ebp].HALF_WIDTH
			add			esi, 0x04
			cmp			esi, ds:[ebp].HALF_WIDTH
			movd		[edx+esi-4], mm6

			jl			convyuv422bottomi
			pop			ebp
		}
	}
}

void CMPEG2Dec::conv444toRGB24(unsigned char *py, unsigned char *pu, unsigned char *pv, unsigned char *dst, int pitch)
{
	int PWIDTH = pitch - DSTBYTES;

	py += CLIP_STEP;
	pu += CLIP_STEP;
	pv += CLIP_STEP;

	int Clip_Height = this->Clip_Height;
	__int64 RGB_Offset = this->RGB_Offset;
	__int64 RGB_Scale = this->RGB_Scale;
	__int64 RGB_CBU = this->RGB_CBU;
	__int64 RGB_CRV = this->RGB_CRV;
	__int64 RGB_CGX = this->RGB_CGX;
	int Clip_Width = this->Clip_Width;
	int Coded_Picture_Width = this->Coded_Picture_Width;

	__asm
	{
		mov			eax, [py]
		mov			ebx, [pu]
		mov			ecx, [pv]
		mov			edx, [dst]
		mov			edi, Clip_Height
		mov			esi, 0x00
		pxor		mm0, mm0

convRGB24:
		movd		mm1, [eax+esi]
		movd		mm3, [ebx+esi]
		punpcklbw	mm1, mm0
		punpcklbw	mm3, mm0
		movd		mm5, [ecx+esi]
		punpcklbw	mm5, mm0
		movq		mm7, [mmmask_0128]
		psubw		mm3, mm7
		psubw		mm5, mm7

		psubw		mm1, RGB_Offset
		movq		mm2, mm1
		movq		mm7, [mmmask_0001]
		punpcklwd	mm1, mm7
		punpckhwd	mm2, mm7
		movq		mm7, RGB_Scale
		pmaddwd		mm1, mm7
		pmaddwd		mm2, mm7

		movq		mm4, mm3
		punpcklwd	mm3, mm0
		punpckhwd	mm4, mm0
		movq		mm7, RGB_CBU
		pmaddwd		mm3, mm7
		pmaddwd		mm4, mm7
		paddd		mm3, mm1
		paddd		mm4, mm2
		psrld		mm3, 13
		psrld		mm4, 13
		packuswb	mm3, mm0
		packuswb	mm4, mm0

		movq		mm6, mm5
		punpcklwd	mm5, mm0
		punpckhwd	mm6, mm0
		movq		mm7, RGB_CRV
		pmaddwd		mm5, mm7
		pmaddwd		mm6, mm7
		paddd		mm5, mm1
		paddd		mm6, mm2
		psrld		mm5, 13
		psrld		mm6, 13
		packuswb	mm5, mm0
		packuswb	mm6, mm0

		punpcklbw	mm3, mm5
		punpcklbw	mm4, mm6
		movq		mm5, mm3
		movq		mm6, mm4
		psrlq		mm5, 16
		psrlq		mm6, 16
		por			mm3, mm5
		por			mm4, mm6

		movd		mm5, [ebx+esi]
		movd		mm6, [ecx+esi]
		punpcklbw	mm5, mm0
		punpcklbw	mm6, mm0
		movq		mm7, [mmmask_0128]
		psubw		mm5, mm7
		psubw		mm6, mm7

		movq		mm7, mm6
		punpcklwd	mm6, mm5
		punpckhwd	mm7, mm5		
		movq		mm5, RGB_CGX
		pmaddwd		mm6, mm5
		pmaddwd		mm7, mm5
		paddd		mm6, mm1
		paddd		mm7, mm2

		psrld		mm6, 13
		psrld		mm7, 13
		packuswb	mm6, mm0
		packuswb	mm7, mm0

		punpcklbw	mm3, mm6
		punpcklbw	mm4, mm7

		movq		mm1, mm3
		movq		mm5, mm4
		movq		mm6, mm4

		psrlq		mm1, 32
		psllq		mm1, 24
		por			mm1, mm3

		psrlq		mm3, 40
		psllq		mm6, 16
		por			mm3, mm6
		movd		[edx], mm1

		psrld		mm4, 16
		psrlq		mm5, 24
		por			mm5, mm4
		movd		[edx+4], mm3

		add			edx, 0x0c
		add			esi, 0x04
		cmp			esi, Clip_Width
		movd		[edx-4], mm5

		jl			convRGB24

		add			eax, Coded_Picture_Width
		add			ebx, Coded_Picture_Width
		add			ecx, Coded_Picture_Width
		add			edx, PWIDTH
		mov			esi, 0x00
		dec			edi
		cmp			edi, 0x00
		jg			convRGB24

		emms
	}
}

// YUV 4:2:2 Format:
// YUYV YUYV ...
void CMPEG2Dec::conv422toYUY2(unsigned char *py, unsigned char *pu, unsigned char *pv, unsigned char *dst, int pitch)
{
	py += CLIP_STEP;
	pu += CLIP_STEP;
	pv += CLIP_STEP;

	int y = this->Clip_Height;
	int Clip_Width_2 = this->Clip_Width / 2;
	int Coded_Picture_Width = this->Coded_Picture_Width;
	int Coded_Picture_Width_2 = this->Coded_Picture_Width / 2;

	__asm
	{
		emms
		mov			eax, [py]
		mov			ebx, [pu]
		mov			ecx, [pv]
		mov			edx, [dst]
		mov			edi, Clip_Width_2
	yloop:
		xor			esi, esi
	xloop:
		movd		mm1, [eax+esi*2]		;0000YYYY
		movd		mm2, [ebx+esi]			;0000UUUU
		movd		mm3, [ecx+esi]			;0000VVVV
		;interleave this to VYUYVYUY
		punpcklbw	mm2, mm3				;VUVUVUVU
		punpcklbw	mm1, mm2				;VYUYVYUY
		movq		[edx+esi*4], mm1
		movd		mm1, [eax+esi*2+4]		;0000YYYY
		punpckhdq	mm2, mm2				;xxxxVUVU
		punpcklbw	mm1, mm2				;VYUYVYUY
		movq		[edx+esi*4+8], mm1
		add			esi, 4
		cmp			esi, edi
		jb			xloop
		add			edx, pitch
		add			eax, Coded_Picture_Width
		add			ebx, Coded_Picture_Width_2
		add			ecx, Coded_Picture_Width_2
		dec			y
		jnz			yloop
		emms
	}
}

//
// codec
//

static const int ChromaFormat[4] = {
	0, 6, 8, 12
};

CMPEG2Dec::CMPEG2Dec()
{
  VF_File = 0;
  VF_FrameLimit = VF_FrameBound = VF_GOPLimit = VF_GOPNow = VF_GOPSize =
	VF_OldFrame = VF_OldRef = 0;
  VF_FrameSize = VF_FrameRate = 0;
  memset(Rdbfr, 0, sizeof(Rdbfr));
  Rdptr = Rdmax = 0;
  CurrentBfr = NextBfr = BitsLeft = Val = Read = 0;
  Fault_Flag = File_Flag = File_Limit = FO_Flag = IDCT_Flag = SystemStream_Flag = 0;
  Luminance_Flag = Resize_Flag = KeyOp_Flag = lfsr0 = lfsr1 = 0;
  BufferOp = 0;
  memset(intra_quantizer_matrix, 0, sizeof(intra_quantizer_matrix));
  memset(non_intra_quantizer_matrix, 0, sizeof(non_intra_quantizer_matrix));
  memset(chroma_intra_quantizer_matrix, 0, sizeof(chroma_intra_quantizer_matrix));
  memset(chroma_non_intra_quantizer_matrix, 0, sizeof(chroma_non_intra_quantizer_matrix));
  load_intra_quantizer_matrix =
  load_non_intra_quantizer_matrix =
  load_chroma_intra_quantizer_matrix =
  load_chroma_non_intra_quantizer_matrix = 0;
  q_scale_type =
  alternate_scan =
  quantizer_scale = 0;

	int i;
	for (i=0; i<MAX_FILE_NUMBER; i++) Infilename[i] = NULL;
	for (i=0; i<8; i++) p_block[i] = block[i] = NULL;
	p_fTempArray = fTempArray = NULL;
	for (i=0; i<3; i++) backward_reference_frame[i] = forward_reference_frame[i] =  auxframe[i] = NULL;
	lum = NULL;
	u422 = v422 = u444 = v444 = dstFrame = NULL;
	hLibrary = NULL;

  CheckCPU();
}

static char* myfgets(char* buff, int len, FILE* file)
{
	char* ret = buff;

	ret[0] = 0;

	ret = fgets(buff, len, file);
	while(ret)
	{
		while(isspace(*ret)) ret++;
		if(*ret) break;
		ret = fgets(buff, len, file);
	}

	return(ret);
}

int CMPEG2Dec::Open(LPCTSTR path, DstFormat dstFormat)
{
	m_dstFormat = dstFormat;
	char ID[19], PASS[19] = "DVD2AVIProjectFile";
	DWORD i, j, size, code, type, tff, rff, film, ntsc, gop, top, bottom, mapping;
	int repeat_on, repeat_off, repeat_init;
	int Clip_Top, Clip_Bottom, Clip_Left, Clip_Right, Squeeze_Width, Squeeze_Height;

	HKEY key; DWORD value = REG_SZ; DWORD length = 256;
	char *ext, buffer[256];

	CMPEG2Dec* out = this;

	if (_tfopen_s(&out->VF_File, path, _T("r")))
		return 0;
	if (fgets(ID, 19, out->VF_File)==NULL)
		return 0;
	if (strcmp(ID, PASS))
		return 0;

	// load DLL
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\VFPlugin", 0, KEY_ALL_ACCESS, &key)==ERROR_SUCCESS)
	{
		RegQueryValueExA(key, "DVD2AVI", NULL, &value, (unsigned char*)buffer, &length);

		ext = strrchr(buffer, '\\');
		sprintf(buffer + (int)(ext-buffer) + 1, "OpenDVD.dll");
		RegCloseKey(key);
	}

	if ((hLibrary = LoadLibraryA(buffer)) != NULL)
		BufferOp = (PBufferOp) GetProcAddress(hLibrary, "BufferOp");

	for (i=0; i<MAX_FILE_NUMBER; i++)
		Infilename[i] = DNew char[_MAX_PATH];

	if(1 != fscanf_s(out->VF_File, "%d", &File_Limit))
		return 0;

	i = File_Limit;
	while (i)
	{
		if(1 != fscanf_s(out->VF_File, "%d ", &j))
			return 0;
		fgets(Infilename[File_Limit-i], j+1, out->VF_File);
		if ((Infile[File_Limit-i] = _open(Infilename[File_Limit-i], _O_RDONLY | _O_BINARY))==-1)
			return 0;
		i--;
	}

	if(3 != sscanf_s(myfgets(buffer, sizeof(buffer), out->VF_File), "\nStream_Type=%d,%X,%X\n", &SystemStream_Flag, &lfsr0, &lfsr1))
		return 0;
	if (lfsr0 || lfsr1)
		KeyOp_Flag = 1;
	else
		KeyOp_Flag = 0;

	if(1 != sscanf_s(myfgets(buffer, sizeof(buffer), out->VF_File), "iDCT_Algorithm=%d\n", &IDCT_Flag))
		return 0;

	switch (IDCT_Flag)
	{
		case IDCT_SSEMMX:
			if (!cpu.ssemmx)
				IDCT_Flag = IDCT_MMX;
			break;

		case IDCT_FPU:
			Initialize_FPU_IDCT();
			break;

		case IDCT_REF:
			Initialize_REF_IDCT();
			break;
	}

	File_Flag = 0;
	_lseeki64(Infile[0], 0, SEEK_SET);
	Initialize_Buffer();

	do
	{
		next_start_code();
		code = Get_Bits(32);
	}
	while (code!=SEQUENCE_HEADER_CODE);

	sequence_header();

	mb_width = (horizontal_size+15)/16;
	mb_height = progressive_sequence ? (vertical_size+15)/16 : 2*((vertical_size+31)/32);

	Coded_Picture_Width = 16 * mb_width;
	Coded_Picture_Height = 16 * mb_height;

	Chroma_Width = (chroma_format==CHROMA444) ? Coded_Picture_Width : Coded_Picture_Width>>1;
	Chroma_Height = (chroma_format!=CHROMA420) ? Coded_Picture_Height : Coded_Picture_Height>>1;

	block_count = ChromaFormat[chroma_format];

	for (i=0; i<8; i++)
	{
		p_block[i] = (short *)DNew BYTE[sizeof(short)*64 + 64];
		block[i]   = (short *)((long)p_block[i] + 64 - (long)p_block[i]%64);
	}

	p_fTempArray = (void *)DNew BYTE[sizeof(float)*128 + 64];
	fTempArray = (void *)((long)p_fTempArray + 64 - (long)p_fTempArray%64);

	for (i=0; i<3; i++)
	{
		if (i==0)
			size = Coded_Picture_Width * Coded_Picture_Height;
		else
			size = Chroma_Width * Chroma_Height;

		backward_reference_frame[i] = DNew unsigned char[size];
		forward_reference_frame[i] = DNew unsigned char[size];
		auxframe[i] = DNew unsigned char[size];
	}

	if(1 != sscanf_s(myfgets(buffer, sizeof(buffer), out->VF_File), "YUVRGB_Scale=%d\n", &i))
		return 0;

	if (i)
	{
		RGB_Scale = 0x1000254310002543;
		RGB_Offset = 0x0010001000100010;
		RGB_CBU = 0x0000408D0000408D;
		RGB_CGX = 0xF377E5FCF377E5FC;
		RGB_CRV = 0x0000331300003313;
	}
	else
	{
		RGB_Scale = 0x1000200010002000;
		RGB_Offset = 0x0000000000000000;
		RGB_CBU = 0x000038B4000038B4;
		RGB_CGX = 0xF4FDE926F4FDE926;
		RGB_CRV = 0x00002CDD00002CDD;
	}

	char* tmp = myfgets(buffer, sizeof(buffer), out->VF_File);
	if(2 != sscanf_s(tmp, "Luminance=%d,%d\n", &i, &j))
	{
		if(2 != sscanf_s(tmp, "Luminance_Filter=%d,%d\n", &i, &j))
			return 0;
		i=128; j=0;
	}

	if (i==128 && j==0)
		Luminance_Flag = 0;
	else
	{
		Luminance_Flag = 1;
		LumGainMask = ((__int64)i<<48) + ((__int64)i<<32) + ((__int64)i<<16) + (__int64)i;
		LumOffsetMask = ((__int64)j<<48) + ((__int64)j<<32) + ((__int64)j<<16) + (__int64)j;

		lum = DNew unsigned char[Coded_Picture_Width * Coded_Picture_Height];
	}

	if(6 != sscanf_s(myfgets(buffer, sizeof(buffer), out->VF_File), "Picture_Size=%d,%d,%d,%d,%d,%d\n", 
		&Clip_Top, &Clip_Bottom, &Clip_Left, &Clip_Right, &Squeeze_Width, &Squeeze_Height))
		return 0;

	Resize_Flag = 0;
	Resize_Width = Clip_Width = Coded_Picture_Width;
	Resize_Height = Clip_Height = Coded_Picture_Height;
	CLIP_AREA = HALF_CLIP_AREA = CLIP_STEP = 0;

	if (Clip_Top || Clip_Bottom || Clip_Left || Clip_Right)
	{
		Clip_Width -= Clip_Left+Clip_Right;
		Clip_Height -= Clip_Top+Clip_Bottom;
		Resize_Width = Clip_Width;
		Resize_Height = Clip_Height;

		CLIP_AREA = Coded_Picture_Width * Clip_Top;
		HALF_CLIP_AREA = (Coded_Picture_Width>>1) * Clip_Top;
		CLIP_STEP = Coded_Picture_Width * Clip_Top + Clip_Left;
	}

	if (Squeeze_Width || Squeeze_Height)
	{
		Resize_Flag = 1;
		Resize_Width -= Squeeze_Width;
		Resize_Height -= Squeeze_Height;
	}

	DSTBYTES = Clip_Width * (dstRGB24() ? 3 : 2);
	DSTBYTES2 = DSTBYTES * 2;
	LUM_AREA = Coded_Picture_Width * Clip_Height;
	PROGRESSIVE_HEIGHT = (Coded_Picture_Height>>1) - 2;
	INTERLACED_HEIGHT = (Coded_Picture_Height>>2) - 2;
	HALF_WIDTH = Coded_Picture_Width>>1;
	HALF_WIDTH_D8 = (Coded_Picture_Width>>1) - 8;
	DOUBLE_WIDTH = Coded_Picture_Width<<1;

	u422 = DNew unsigned char[Coded_Picture_Width * Coded_Picture_Height / 2];
	v422 = DNew unsigned char[Coded_Picture_Width * Coded_Picture_Height / 2];
	u444 = DNew unsigned char[Coded_Picture_Width * Coded_Picture_Height];
	v444 = DNew unsigned char[Coded_Picture_Width * Coded_Picture_Height];
	dstFrame = DNew unsigned char[Clip_Width * Clip_Height * 4];  // max value (super set)

	if(1 != sscanf_s(myfgets(buffer, sizeof(buffer), out->VF_File), "Field_Operation=%d\n", &FO_Flag))
		return 0;
	if(1 != sscanf_s(myfgets(buffer, sizeof(buffer), out->VF_File), "Frame_Rate=%d\n", &(out->VF_FrameRate)))
		return 0;
	if(4 != sscanf_s(myfgets(buffer, sizeof(buffer), out->VF_File), "Location=%d,%X,%d,%X\n", &i, &j, &i, &j))
		return 0;

	ntsc = film = top = bottom = gop = mapping = repeat_on = repeat_off = repeat_init = 0;

	while (1 == fscanf_s(out->VF_File, "%d", &type) && type<9)
	{
		if (type==7)	// I frame
		{
			GOPList[gop] = reinterpret_cast<GOPLIST*>(calloc(1, sizeof(GOPLIST)));
			GOPList[gop]->number = film;
			if(2 != fscanf_s(out->VF_File, "%d %X", &(GOPList[gop]->file), &j))
				break;

			GOPList[gop]->position = (__int64)j*BUFFER_SIZE;
			gop ++;

			if(1 != fscanf_s(out->VF_File, "%d", &j))
				break;

			tff = j>>1;
			rff = j & 1;
		}
		else	// P, B frame
		{
			tff = type>>1;
			rff = type & 1;
		}

		if (!film)
		{
			if (tff)
				Field_Order = 1;
			else
				Field_Order = 0;
		}

		if (FO_Flag==FO_FILM)
		{
			if (rff)
				repeat_on++;
			else
				repeat_off++;

			if (repeat_init)
			{
				if (repeat_off-repeat_on == 5)
				{
					repeat_on = repeat_off = 0;
				}
				else
				{
					FrameList[mapping] = reinterpret_cast<FRAMELIST*>(calloc(1, sizeof(FRAMELIST)));
					FrameList[mapping]->top = FrameList[mapping]->bottom = film;
					mapping ++;
				}

				if (repeat_on-repeat_off == 5)
				{
					repeat_on = repeat_off = 0;
					FrameList[mapping] = reinterpret_cast<FRAMELIST*>(calloc(1, sizeof(FRAMELIST)));
					FrameList[mapping]->top = FrameList[mapping]->bottom = film;
					mapping ++;
				}
			}
			else
			{
				if (repeat_off-repeat_on == 3)
				{
					repeat_on = repeat_off = 0;
					repeat_init = 1;
				}
				else
				{
					FrameList[mapping] = reinterpret_cast<FRAMELIST*>(calloc(1, sizeof(FRAMELIST)));
					FrameList[mapping]->top = FrameList[mapping]->bottom = film;
					mapping ++;
				}

				if (repeat_on-repeat_off == 3)
				{
					repeat_on = repeat_off = 0;
					repeat_init = 1;

					FrameList[mapping] = reinterpret_cast<FRAMELIST*>(calloc(1, sizeof(FRAMELIST)));
					FrameList[mapping]->top = FrameList[mapping]->bottom = film;
					mapping ++;
				}
			}
		}
		else
		{
			if (top)
			{
				FrameList[ntsc]->bottom = film;
				ntsc ++;
				FrameList[ntsc] = reinterpret_cast<FRAMELIST*>(calloc(1, sizeof(FRAMELIST)));
				FrameList[ntsc]->top = film;
			}
			else if (bottom)
			{
				FrameList[ntsc]->top = film;
				ntsc ++;
				FrameList[ntsc] = reinterpret_cast<FRAMELIST*>(calloc(1, sizeof(FRAMELIST)));
				FrameList[ntsc]->bottom = film;
			}
			else
			{
				FrameList[ntsc] = reinterpret_cast<FRAMELIST*>(calloc(1, sizeof(FRAMELIST)));
				FrameList[ntsc]->top = film;
				FrameList[ntsc]->bottom = film;
				ntsc ++;
			}

			if (rff)
			{
				if (!top && !bottom)
					FrameList[ntsc] = reinterpret_cast<FRAMELIST*>(calloc(1, sizeof(FRAMELIST)));

				if (tff)
				{
					FrameList[ntsc]->top = film;
					top = 1;
				}
				else
				{
					FrameList[ntsc]->bottom = film;
					bottom = 1;
				}

				if (top && bottom)
				{
					top = bottom = 0;
					ntsc ++;
				}
			}
		}

		film ++;
	}

	out->VF_FrameBound = film;
	film -= 2;

	if (FO_Flag==FO_FILM)
	{
		while (FrameList[mapping-1]->top >= film)
			mapping --;

		out->VF_FrameLimit = mapping;
	}
	else
	{
		if (FO_Flag==FO_SWAP)
		{
			Field_Order = !Field_Order;

			if (Field_Order)
				for (i=0; i<ntsc-1; i++)
					FrameList[i]->bottom = FrameList[i+1]->bottom;
			else
				for (i=0; i<ntsc-1; i++)
					FrameList[i]->top = FrameList[i+1]->top;
		}

		while ((FrameList[ntsc-1]->top >= film) || (FrameList[ntsc-1]->bottom >= film))
			ntsc --;

		out->VF_FrameLimit = ntsc;

		for (i=0; i<out->VF_FrameLimit-1; i++)
			if (FrameList[i]->top==FrameList[i+1]->top || FrameList[i]->top==FrameList[i+1]->bottom ||
				FrameList[i]->bottom==FrameList[i+1]->top || FrameList[i]->bottom==FrameList[i+1]->bottom)
			{
				FrameList[i]->forward = 1;
				FrameList[i+1]->backward = 1;
			}
	}

	Full_Frame = 1;
	for (i=0; i<out->VF_FrameLimit; i++)
		if (FrameList[i]->top!=FrameList[i]->bottom)
		{
			Full_Frame = 0;
			break;
		}

	out->VF_GOPNow = out->VF_GOPLimit = gop;
	out->VF_OldFrame = out->VF_FrameLimit;
	out->VF_FrameSize = Clip_Width * Clip_Height * 3;

	return 1;
}

void CMPEG2Dec::Decode(unsigned char *dst, DWORD frame, int pitch)
{
	DWORD i, now, size = 0, origin, ref, fo = 0;
	int remain;

	CMPEG2Dec* in = this;

	if (FO_Flag==FO_FILM)
	{
		fo = 0;
		frame = FrameList[frame]->top;
	}

	origin = frame;

	if (FO_Flag!=FO_FILM)
	{
		if (FrameList[frame]->top == FrameList[frame]->bottom)
		{
			fo = 0;
			frame = FrameList[frame]->top;
		}
		else if (FrameList[frame]->top < FrameList[frame]->bottom)
		{
			fo = 1;
			frame = FrameList[frame]->top;
		}
		else
		{
			fo = 2;
			frame = FrameList[frame]->bottom;
		}
	}

	ref = frame;

	if (frame >= GOPList[in->VF_GOPLimit-1]->number)
	{
		now = in->VF_GOPLimit-1;
		ref -= GOPList[in->VF_GOPLimit-1]->number;
		size = in->VF_FrameBound - GOPList[in->VF_GOPLimit-1]->number + 1;
	}
	else
		for (now = 0; now < (in->VF_GOPLimit-1); now++)
		{
			if (frame>=GOPList[now]->number && frame<GOPList[now+1]->number)
			{
				ref -= GOPList[now]->number;
				size = GOPList[now+1]->number - GOPList[now]->number + 1;
				break;
			}
		}

	if (fo)
		ref ++;

	if (now != in->VF_GOPNow)
	{
		if ((in->VF_OldFrame + 1)==origin)
		{
			if (Full_Frame)
			{
				Get_Hdr();
				Decode_Picture(1, dst, pitch);

				if (picture_structure!=FRAME_PICTURE)
				{
					Get_Hdr();
					Decode_Picture(1, dst, pitch);
				}
			}
			else
				switch (fo)
				{
					case 0:
						if (!FrameList[origin]->backward)
						{
							Get_Hdr();
							Decode_Picture(1, dst, pitch);

							if (picture_structure!=FRAME_PICTURE)
							{
								Get_Hdr();
								Decode_Picture(1, dst, pitch);
							}

							if (FrameList[origin]->forward)
							{
								if (Field_Order)
									Copyodd(dst, dstFrame, pitch, 1);
								else
									Copyeven(dst, dstFrame, pitch, 1);
							}
						}
						else
						{
							Copyodd(dstFrame, dst, pitch, 0);
							Copyeven(dstFrame, dst, pitch, 0);
						}
						break;

					case 1:
						Copyodd(dstFrame, dst, pitch, 0);

						Get_Hdr();
						Decode_Picture(1, dstFrame, DSTBYTES);

						if (picture_structure!=FRAME_PICTURE)
						{
							Get_Hdr();
							Decode_Picture(1, dstFrame, DSTBYTES);
						}

						Copyeven(dstFrame, dst, pitch, 0);
						break;

					case 2:	
						Copyeven(dstFrame, dst, pitch, 0);

						Get_Hdr();
						Decode_Picture(1, dstFrame, DSTBYTES);

						if (picture_structure!=FRAME_PICTURE)
						{
							Get_Hdr();
							Decode_Picture(1, dstFrame, DSTBYTES);
						}

						Copyodd(dstFrame, dst, pitch, 0);
						break;
				}

			if (in->VF_GOPSize)
			{
				for (i=0; i < in->VF_GOPSize; i++)
					free(GOPBuffer[i]);

				in->VF_GOPSize = 0;
			}

			in->VF_GOPNow = in->VF_GOPLimit;
			in->VF_OldFrame = origin;
			return;
		}

		remain = ref;
		in->VF_OldRef = ref;
		in->VF_GOPNow = now;
		Second_Field = 0;

		if (size < in->VF_GOPSize)
			for (i=0; i < (in->VF_GOPSize - size); i++)
				free(GOPBuffer[size+i]);
		else if (size > in->VF_GOPSize)
			for (i=0; i < (size - in->VF_GOPSize); i++)
				GOPBuffer[in->VF_GOPSize+i] = reinterpret_cast<unsigned char*>(malloc(in->VF_FrameSize));

		in->VF_GOPSize = size;

		File_Flag = GOPList[now]->file;
		_lseeki64(Infile[GOPList[now]->file], GOPList[now]->position, SEEK_SET);
		Initialize_Buffer();

		while (Get_Hdr() && picture_coding_type!=I_TYPE);

		Decode_Picture(0, dst, pitch);

		while (Get_Hdr() && picture_coding_type==B_TYPE);

		if (picture_structure!=FRAME_PICTURE)
		{
			Decode_Picture(0, dst, pitch);
			Get_Hdr();
		}

		Decode_Picture(1, dst, pitch);

		if (picture_structure!=FRAME_PICTURE)
		{
			Get_Hdr();
			Decode_Picture(1, dst, pitch);
		}

		Copyodd(dst, GOPBuffer[0], pitch, 1);
		Copyeven(dst, GOPBuffer[0], pitch, 1);

		while (remain && Get_Hdr())
		{
			Decode_Picture(1, dst, pitch);

			if (picture_structure!=FRAME_PICTURE)
			{
				Get_Hdr();
				Decode_Picture(1, dst, pitch);
			}

			Copyodd(dst, GOPBuffer[ref - remain + 1], pitch, 1);
			Copyeven(dst, GOPBuffer[ref - remain + 1], pitch, 1);

			remain--;
		}

		if (!Full_Frame && ref>=(size-2))
		{
			Copyodd(dst, dstFrame, pitch, 1);
			Copyeven(dst, dstFrame, pitch, 1);
		}
	}
	else
	{
		remain = ref - in->VF_OldRef;

		if (remain > 0)
		{
			in->VF_OldRef = ref;

			while (remain && Get_Hdr())
			{
				Decode_Picture(1, dst, pitch);

				if (picture_structure!=FRAME_PICTURE)
				{
					Get_Hdr();
					Decode_Picture(1, dst, pitch);
				}

				Copyodd(dst, GOPBuffer[ref - remain + 1], pitch, 1);
				Copyeven(dst, GOPBuffer[ref - remain + 1], pitch, 1);

				remain--;
			}

			if (!Full_Frame && ref>=(size-2))
			{
				Copyodd(dst, dstFrame, pitch, 1);
				Copyeven(dst, dstFrame, pitch, 1);
			}
		}
	}

	switch (fo)
	{
		case 0:
			Copyodd(GOPBuffer[ref], dst, pitch, 0);
			Copyeven(GOPBuffer[ref], dst, pitch, 0);
			break;

		case 1:
			Copyodd(GOPBuffer[ref-1], dst, pitch, 0);
			Copyeven(GOPBuffer[ref], dst, pitch, 0);
			break;

		case 2:
			Copyodd(GOPBuffer[ref], dst, pitch, 0);
			Copyeven(GOPBuffer[ref-1], dst, pitch, 0);
			break;
	}

	in->VF_OldFrame = origin;
}

void CMPEG2Dec::Close()
{
	unsigned int i;

	for(i = 0; i < VF_GOPLimit; i++) free(GOPList[i]);
	for(i = 0; i < VF_FrameLimit; i++) free(FrameList[i]);

	if (VF_File != NULL)
		fclose(VF_File);

	while (VF_GOPSize)
	{
		VF_GOPSize--;
		free(GOPBuffer[VF_GOPSize]);
	}

	while (File_Limit)
	{
		File_Limit--;
		_close(Infile[File_Limit]);
	}

	for (i=0; i<MAX_FILE_NUMBER; i++)
		delete [] Infilename[i];

	for (i=0; i<3; i++)
	{
		delete [] backward_reference_frame[i];
		delete [] forward_reference_frame[i];
		delete [] auxframe[i];
	}

	delete [] u422;
	delete [] v422;
	delete [] u444;
	delete [] v444;
	delete [] dstFrame;

	if(Luminance_Flag)
		delete [] lum;

	for (i=0; i<8; i++)
		delete [] p_block[i];

	delete [] p_fTempArray;

	if (hLibrary)
		FreeLibrary(hLibrary);
}

void CMPEG2Dec::Copyodd(unsigned char *src, unsigned char *dst, int pitch, int forward)
{
	int i;
	int PWIDTH = forward ? (pitch<<1) : DSTBYTES2;
	int QWIDTH = forward ? DSTBYTES2 : (pitch<<1);

	for (i=0; i<(Clip_Height>>1); i++)
	{
		memcpy (dst, src, DSTBYTES);
		src += PWIDTH;
		dst += QWIDTH;
	}
}

void CMPEG2Dec::Copyeven(unsigned char *src, unsigned char *dst, int pitch, int forward)
{
	int i;
	int PWIDTH = forward ? (pitch<<1) : DSTBYTES2;
	int QWIDTH = forward ? DSTBYTES2 : (pitch<<1);
	src += forward ? pitch : DSTBYTES;
	dst += forward ? DSTBYTES : pitch;

	for (i=0; i<(Clip_Height>>1); i++)
	{
		memcpy (dst, src, DSTBYTES);
		src += PWIDTH;
		dst += QWIDTH;
	}
}
