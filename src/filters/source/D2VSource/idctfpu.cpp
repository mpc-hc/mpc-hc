#include "stdafx.h"

/* idct.c, inverse fast discrete cosine transform                           */


/*************************************************************/
/* inverse two dimensional DCT, Chen-Wang algorithm          */
/* (cf. IEEE ASSP-32, pp. 803-816, Aug. 1984)                */
/*                                                           */
/* floating point conversion by Miha Peternel                */
/* x87 hand-optimized assembly by Miha Peternel              */
/*                                    27.11. - 11.12.2000    */
/*                                                           */
/* You are free to use this code in your project if:         */
/* - no changes are made to this message                     */
/* - any changes to this code are publicly available         */
/* - your project documentation contains the following text: */
/*   "This software contains fast high-quality IDCT decoder  */
/*    by Miha Peternel."                                     */
/*                                                           */
/*************************************************************/


/////////////////////////////////////////////////////
//
// TODO:
// - loops can be easily vectorized for SIMD
//
/////////////////////////////////////////////////////

#include <math.h>
#  define PI 3.1415926535897932384626433832795

#define FLOAT double

const static double RC = 1.0 * 1024 * 1024 * 1024 * 1024 * 256 * 16 + 1024; // magic + clip center

static FLOAT W1; // /* sqrt(2)*cos(1*pi/16) */
static FLOAT W2; // /* sqrt(2)*cos(2*pi/16) */
static FLOAT W5; // /* sqrt(2)*cos(5*pi/16) */

static FLOAT W1_8;
static FLOAT W2_8;
static FLOAT W5_8;

static FLOAT W7; // /* sqrt(2)*cos(7*pi/16) */
static FLOAT W1mW7; // W1-W7
static FLOAT W1pW7; // W1+W7

static FLOAT W3; // /* sqrt(2)*cos(3*pi/16) */
static FLOAT W3mW5; // W3-W5
static FLOAT W3pW5; // W3+W5

static FLOAT W6; // /* sqrt(2)*cos(6*pi/16) */
static FLOAT W2mW6; // W2-W6
static FLOAT W2pW6; // W2+W6

static FLOAT S2; // 1/sqrt(2)
static FLOAT D8 = 1.0 / 8;

static FLOAT W7_8;
static FLOAT W1mW7_8;
static FLOAT W1pW7_8;

static FLOAT W3_8;
static FLOAT W3mW5_8;
static FLOAT W3pW5_8;

static FLOAT W6_8;
static FLOAT W2mW6_8;
static FLOAT W2pW6_8;

/* global declarations */

/* private data */
static short iclip[1024+1024]; /* clipping table */
static short *iclp;

void Initialize_FPU_IDCT()
{
    int i;

    S2 = sqrt(0.5); // 1.0/sqrt(2);

    W1 = sqrt(2.0) * cos(PI * (1.0 / 16));
    W1_8 = W1 / 8;
    W2 = sqrt(2.0) * cos(PI * (2.0 / 16));
    W2_8 = W2 / 8;
    W3 = sqrt(2.0) * cos(PI * (3.0 / 16));
    W3_8 = W3 / 8;
    W5 = sqrt(2.0) * cos(PI * (5.0 / 16));
    W5_8 = W5 / 8;
    W6 = sqrt(2.0) * cos(PI * (6.0 / 16));
    W6_8 = W6 / 8;
    W7 = sqrt(2.0) * cos(PI * (7.0 / 16));
    W7_8 = W7 / 8;

    W1mW7 = W1 - W7;
    W1mW7_8 = W1mW7 / 8;
    W1pW7 = W1 + W7;
    W1pW7_8 = W1pW7 / 8;
    W3mW5 = W3 - W5;
    W3mW5_8 = W3mW5 / 8;
    W3pW5 = W3 + W5;
    W3pW5_8 = W3pW5 / 8;
    W2mW6 = W2 - W6;
    W2mW6_8 = W2mW6 / 8;
    W2pW6 = W2 + W6;
    W2pW6_8 = W2pW6 / 8;

    iclp = iclip + 1024;
    for(i = -1024; i < 1024; i++)
        iclp[i] = (i < -256) ? -256 : ((i > 255) ? 255 : i);
}

void FPU_IDCT(short *block)
{
    int *b = (int *) block;
    if(b[0] == 0 && (b[31] == 0x10000 || b[31] == 0))
    {
        if(b[ 1] | b[ 2] | b[ 3] | b[ 4] | b[ 5])
            goto normal;
        if(b[ 6] | b[ 7] | b[ 8] | b[ 9] | b[10])
            goto normal;
        if(b[11] | b[12] | b[13] | b[14] | b[15])
            goto normal;
        if(b[16] | b[17] | b[18] | b[19] | b[20])
            goto normal;
        if(b[21] | b[22] | b[23] | b[24] | b[25])
            goto normal;
        if(b[26] | b[27] | b[28] | b[29] | b[30])
            goto normal;
        b[31] = 0;
        ////empty++;
        return;
    }
normal:

#define tmp  ebx
#define tmp1 ebx-1*8
#define tmp2 ebx-2*8
#define tmp3 ebx-3*8
#define int0 ebx-3*8-1*4
#define int1 ebx-3*8-2*4
#define int2 ebx-3*8-3*4
#define int3 ebx-3*8-4*4
#define int4 ebx-3*8-5*4
#define int5 ebx-3*8-6*4
#define int6 ebx-3*8-7*4
#define int7 ebx-3*8-8*4
#define SIZE 8*8*8+3*8+8*4+16 // locals + 16-byte alignment area
    __asm
    {
        lea ebx, [esp-8*8*8]
        sub esp, SIZE
        and ebx, -16 // force 16-byte alignment of locals

// rows
        mov esi, [block]
        lea edi, [tmp]
        mov ecx, 8

        align 16
        Lrows:
        movsx eax, word ptr [esi+2]
        or    eax,         [esi+4]
        or    eax,         [esi+8]
        or    eax,         [esi+12]
        jnz L1

        fild word ptr [esi+0*2]
        fst  qword ptr [edi+7*8]
        fst  qword ptr [edi+6*8]
        fst  qword ptr [edi+5*8]
        fst  qword ptr [edi+4*8]
        fst  qword ptr [edi+3*8]
        fst  qword ptr [edi+2*8]
        fst  qword ptr [edi+1*8]
        fstp qword ptr [edi+0*8]
        jmp L2

        align 16
        L1:

        fild word ptr [esi+7*2]
        fld st(0)
        fild word ptr [esi+1*2]
        fadd st(1), st(0)
        fld qword ptr [W7]
        fxch st(1)
        fmul qword ptr [W1mW7]
        fxch st(1)
        fmulp st(2), st(0)
        fadd st(0), st(1)
        fstp qword ptr [tmp1]
        fild word ptr [esi+3*2]
        fld st(0)
        fxch st(3)
        fmul qword ptr [W1pW7]
        fild word ptr [esi+5*2]
        fadd st(4), st(0)
        fmul qword ptr [W3mW5]
        fxch st(1)
        fsubp st(3), st(0) //fsubrp
        fld qword ptr [W3]
        fmulp st(4), st(0)
        fsubr st(0), st(3)
        fstp qword ptr [tmp2]
        fmul qword ptr [W3pW5]
        fsubp st(2), st(0) //fsubrp
        fxch st(1)
        fstp qword ptr [tmp3]
        fild word ptr [esi+0*2]
        fild word ptr [esi+4*2]
        fild word ptr [esi+2*2]
        fld st(0)
        fmul qword ptr [W2mW6]
        fld st(3)
        fild word ptr [esi+6*2]
        fxch st(5)
        fsub st(0), st(4)
        fxch st(3)
        fadd st(0), st(5)
        fxch st(1)
        faddp st(4), st(0)
        fld qword ptr [W6]
        fmulp st(1), st(0)
        fxch st(4)
        fmul qword ptr [W2pW6]
        fld qword ptr [tmp1]
        fsub qword ptr [tmp2]
        fld st(5)
        fxch st(3)
        faddp st(6), st(0)
        fld qword ptr [tmp1]
        fxch st(1)
        fstp qword ptr [tmp1]
        fld st(6)
        fadd qword ptr [tmp3]
        fxch st(1)
        fadd qword ptr [tmp2]
        fxch st(7)
        fsub qword ptr [tmp3]
        fxch st(1)
        fstp qword ptr [tmp2]
        fld st(4)
        fxch st(3)
        fsubrp st(2), st(0) //fsubp
        fxch st(4)
        fsub st(0), st(5)
        fxch st(2)
        faddp st(5), st(0)
        fld st(2)
        fsub st(0), st(1)
        fxch st(5)
        fstp qword ptr [tmp3]
        fld qword ptr [tmp1]
        fld qword ptr [S2]
        fxch st(4)
        faddp st(2), st(0)
        fld st(3)
        fxch st(1)
        fadd st(0), st(5)
        fmulp st(1), st(0)

        fld qword ptr [tmp3]
        fadd st(0), st(7)
        fxch st(5)
        fsubr qword ptr [tmp1]
        fxch st(5)
        fstp qword ptr [edi+0*8]
        fxch st(6)
        fsubr qword ptr [tmp3]
        fld st(2)
        fxch st(1)
        fstp qword ptr [edi+7*8]
        fadd qword ptr [tmp2]
        fxch st(3)
        fmulp st(4), st(0)
        fxch st(2)
        fstp qword ptr [edi+3*8]
        fld st(1)
        fadd st(0), st(5)
        fxch st(1)
        fsub qword ptr [tmp2]
        fxch st(2)
        fsubrp st(5), st(0) //fsubp
        fstp qword ptr [edi+1*8]
        fld st(2)
        fxch st(1)
        fstp qword ptr [edi+4*8]
        fxch st(2)
        fsub st(0), st(1)
        fxch st(2)
        faddp st(1), st(0)
        fxch st(2)
        fstp qword ptr [edi+6*8]
        fstp qword ptr [edi+5*8]
        fstp qword ptr [edi+2*8]
        L2:
        add esi, 8*2
        add edi, 8*8
        dec ecx
        jnz Lrows

// columns
        lea esi, [tmp]
        mov edi, [block]
        lea edx, [iclip+1024*2]
        mov ecx, 8

        align 16
        Lcols:
        fld qword ptr [esi+7*8*8]
        fld st(0)
        fld qword ptr [esi+1*8*8]
        fadd st(1), st(0)
        fld qword ptr [W7_8]
        fxch st(1)
        fmul qword ptr [W1mW7_8]
        fxch st(1)
        fmulp st(2), st(0)
        fadd st(0), st(1)
        fstp qword ptr [tmp2]
        fld qword ptr [esi+3*8*8]
        fld st(0)
        fxch st(3)
        fmul qword ptr [W1pW7_8]
        fld qword ptr [esi+5*8*8]
        fadd st(4), st(0)
        fmul qword ptr [W3mW5_8]
        fxch st(1)
        fsubp st(3), st(0) //fsubrp
        fld qword ptr [W3_8]
        fmulp st(4), st(0)
        fsubr st(0), st(3)
        fstp qword ptr [tmp3]
        fld qword ptr [D8]
        fld qword ptr [esi+0*8*8]
        fmul st(0), st(1)
        fxch st(2)
        fmul qword ptr [W3pW5_8]
        fld qword ptr [esi+4*8*8]
        fmulp st(2), st(0)
        fld qword ptr [esi+6*8*8]
        fld st(3)
        fxch st(6)
        fsubrp st(2), st(0) //fsubp
        fld qword ptr [esi+2*8*8]
        fld st(0)
        fxch st(5)
        fsub st(0), st(4)
        fxch st(7)
        faddp st(4), st(0)
        fxch st(4)
        fadd st(0), st(1)
        fld qword ptr [W6_8]
        fxch st(2)
        fmul qword ptr [W2pW6_8]
        fxch st(2)
        fmulp st(1), st(0)
        fxch st(4)
        fmul qword ptr [W2mW6_8]
        fld qword ptr [tmp2]
        fsub qword ptr [tmp3]
        fxch st(2)
        fsubr st(0), st(5)
        fxch st(1)
        faddp st(5), st(0)
        fld qword ptr [tmp2]
        fxch st(2)
        fstp qword ptr [tmp2]
        fld st(5)
        fxch st(2)
        fadd qword ptr [tmp3]
        fxch st(6)
        fsub st(0), st(3)
        fxch st(2)
        faddp st(3), st(0)
        fld st(3)
        fsub st(0), st(5)
        fxch st(3)
        fstp qword ptr [tmp3]
        fxch st(3)
        faddp st(4), st(0)
        fld st(5)
        fld qword ptr [tmp2]
        fxch st(7)
        fsub st(0), st(4)
        fxch st(7)
        fadd st(0), st(2)
        fxch st(1)
        faddp st(4), st(0)
        fld qword ptr [S2]
        fmul st(1), st(0)
        fxch st(1)
        fstp qword ptr [tmp1]
        fld st(4)
        fadd st(0), st(6)
        fxch st(2)
        fsubr qword ptr [tmp2]
        fxch st(5)
        fsubrp st(6), st(0) //fsubp
        fxch st(1)
        fistp dword ptr [int0]
        fxch st(4)
        mov eax, [int0]
        movsx eax, word ptr [edx+2*eax]
        mov [edi+0*8*2], ax
        fistp dword ptr [int7]
        mov eax, [int7]
        fld st(0)
        movsx eax, word ptr [edx+2*eax]
        mov [edi+7*8*2], ax
        fadd qword ptr [tmp3]
        fistp dword ptr [int3]
        mov eax, [int3]
        movsx eax, word ptr [edx+2*eax]
        mov [edi+3*8*2], ax
        fsub qword ptr [tmp3]
        fld st(1)
        fxch st(1)
        fistp dword ptr [int4]
        mov eax, [int4]
        movsx eax, word ptr [edx+2*eax]
        mov [edi+4*8*2], ax
        fadd qword ptr [tmp1]
        fxch st(3)
        fmulp st(2), st(0)
        fxch st(2)
        fistp dword ptr [int1]
        fxch st(1)
        mov eax, [int1]
        movsx eax, word ptr [edx+2*eax]
        mov [edi+1*8*2], ax
        fsub qword ptr [tmp1]
        fld st(2)
        fsub st(0), st(2)
        fxch st(1)
        fistp dword ptr [int6]
        fxch st(2)
        mov eax, [int6]
        faddp st(1), st(0)
        movsx eax, word ptr [edx+2*eax]
        mov [edi+6*8*2], ax
        fistp dword ptr [int2]
        mov eax, [int2]
        movsx eax, word ptr [edx+2*eax]
        mov [edi+2*8*2], ax
        fistp dword ptr [int5]
        mov eax, [int5]
        movsx eax, word ptr [edx+2*eax]
        mov [edi+5*8*2], ax

        add esi, 8
        add edi, 2
        dec ecx
        jnz Lcols

        add esp, SIZE
    }
}
