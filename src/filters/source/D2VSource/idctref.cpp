#include "stdafx.h"

/* idctref_miha.c, Inverse Discrete Fourier Transform, double precision */

/*************************************************************/
/*                                                           */
/* x87 hand-optimized assembly by Miha Peternel              */
/*                                     27.11. - 20.1.2001    */
/*                                                           */
/* You are free to use this code in your project if:         */
/* - no changes are made to this message                     */
/* - any changes to this code are publicly available         */
/* - your project documentation contains the following text: */
/*   "This software contains fast high-quality IDCT decoder  */
/*    by Miha Peternel."                                     */
/*                                                           */
/*************************************************************/

/*  Perform IEEE 1180 reference (64-bit floating point, separable 8x1
 *  direct matrix multiply) Inverse Discrete Cosine Transform
*/

#define ModelX 123 // enable C-level optimizations by Miha Peternel

/* Here we use math.h to generate constants.  Compiler results may
   vary a little */

#include <math.h>

#define M_PI	3.1415926535897932384626433832795
const static double HALF = 0.5;

/* private data */
static short iclip[1024+1024]; /* clipping table */
static short *iclp;

/* cosine transform matrix for 8x1 IDCT */
static double c[8][8];

/* initialize DCT coefficient matrix */
void Initialize_REF_IDCT()
{
    int freq, time, i;
    double scale;

    for(freq = 0; freq < 8; freq++)
    {
        scale = (freq == 0) ? sqrt(0.125) : 0.5;
        for(time = 0; time < 8; time++)
            c[freq][time] = scale * cos((M_PI / 8.0) * freq * (time + 0.5));
    }

#ifdef ModelX
    iclp = iclip + 1024;
    for(i = -1024; i < 1024; i++)
        iclp[i] = (i < -256) ? -256 : ((i > 255) ? 255 : i);
#endif
}

void REF_IDCT(short *block)
{
    double tmp[64];
    double rnd[64];
    int int0, int1, int2, int3, int4, int5, int6, int7;
    unsigned short fpold;
    unsigned short fpnew;

    int *b = (int *) block;

    if(!(b[0] | (b[31]&~0x10000)))
    {
        if(b[ 1] | b[ 2] | b[ 3] | b[ 4] | b[ 5] | b[ 6])
            goto normal;
        if(b[ 7] | b[ 8] | b[ 9] | b[10] | b[11] | b[12])
            goto normal;
        if(b[13] | b[14] | b[15] | b[16] | b[17] | b[18])
            goto normal;
        if(b[19] | b[20] | b[21] | b[22] | b[23] | b[24])
            goto normal;
        if(b[25] | b[26] | b[27] | b[28] | b[29] | b[30])
            goto normal;
        b[31] = 0;
        return;
    }
normal:

    __asm
    {
        // do the IDCT
        mov esi, [block]
        lea eax, [c]
        lea edi, [tmp]
        //mov ebx,8
        mov ebx, 8 // 0x77000000 // 8
        align 16
        __col1:
        movzx edx, [esi+1*2]
        mov   ecx, [esi+2*2]
        or    edx, [esi+4*2]
        or    ecx, [esi+6*2]
        or edx, ecx
        //mov ecx,8
        mov ecx, 8/2 // 0x77000000 // 8

        jnz __row1
        fild  word ptr [esi+0*2]
        fmul qword ptr [eax+0*8*8]
        fst  qword ptr [edi+0*8]
        fst  qword ptr [edi+1*8]
        fst  qword ptr [edi+2*8]
        fst  qword ptr [edi+3*8]
        fst  qword ptr [edi+4*8]
        fst  qword ptr [edi+5*8]
        fst  qword ptr [edi+6*8]
        fstp qword ptr [edi+7*8]
        add edi, 8*8
        jmp __next1
        align 16
        __row1:
        fild  word ptr [esi+0*2]
        fmul qword ptr [eax+0*8*8]
        fild  word ptr [esi+1*2]
        fmul qword ptr [eax+1*8*8]
        fadd
        fild  word ptr [esi+2*2]
        fmul qword ptr [eax+2*8*8]
        fadd
        fild  word ptr [esi+3*2]
        fmul qword ptr [eax+3*8*8]
        fadd
        fild  word ptr [esi+4*2]
        fmul qword ptr [eax+4*8*8]
        fadd
        fild  word ptr [esi+5*2]
        fmul qword ptr [eax+5*8*8]
        fadd
        fild  word ptr [esi+6*2]
        fmul qword ptr [eax+6*8*8]
        fadd
        fild  word ptr [esi+7*2]
        fmul qword ptr [eax+7*8*8]
        fadd

        fild  word ptr [esi+0*2]
        fmul qword ptr [eax+0*8*8+8]
        fild  word ptr [esi+1*2]
        fmul qword ptr [eax+1*8*8+8]
        fadd
        fild  word ptr [esi+2*2]
        fmul qword ptr [eax+2*8*8+8]
        fadd
        fild  word ptr [esi+3*2]
        fmul qword ptr [eax+3*8*8+8]
        fadd
        fild  word ptr [esi+4*2]
        fmul qword ptr [eax+4*8*8+8]
        fadd
        fild  word ptr [esi+5*2]
        fmul qword ptr [eax+5*8*8+8]
        fadd
        fild  word ptr [esi+6*2]
        fmul qword ptr [eax+6*8*8+8]
        fadd
        fild  word ptr [esi+7*2]
        fmul qword ptr [eax+7*8*8+8]
        fadd
        add eax, 8*2
        fxch st(1)
        fstp qword ptr [edi]//
        fstp qword ptr [edi+8]
        add edi, 8*2
        dec ecx

        jnz __row1
        add eax, -8*8
        //align 16
        __next1:
        add esi, +8*2

        sub ebx, 0x80000001 // add ebx,ebx
        js  __col1
        //align 16
        test ebx, ebx // align jump &| redo flags
        jnz __col1

        lea esi, [tmp]
        lea eax, [c]
        lea edi, [rnd]
        //mov edi,[block]
        fld qword ptr [HALF]
        mov ebx, 8
        __row2:
        mov ecx, 8/2
        align 16
        __col2:
        fld  qword ptr [esi+0*8*8]
        fmul qword ptr [eax+0*8*8]
        fld  qword ptr [esi+1*8*8]
        fmul qword ptr [eax+1*8*8]
        fadd
        fld  qword ptr [esi+2*8*8]
        fmul qword ptr [eax+2*8*8]
        fadd
        fld  qword ptr [esi+3*8*8]
        fmul qword ptr [eax+3*8*8]
        fadd
        fld  qword ptr [esi+4*8*8]
        fmul qword ptr [eax+4*8*8]
        fadd
        fld  qword ptr [esi+5*8*8]
        fmul qword ptr [eax+5*8*8]
        fadd
        fld  qword ptr [esi+6*8*8]
        fmul qword ptr [eax+6*8*8]
        fadd
        fld  qword ptr [esi+7*8*8]
        fmul qword ptr [eax+7*8*8]
        fadd
        fadd st(0), st(1)

        fxch st(1)

        fld  qword ptr [esi+0*8*8]
        fmul qword ptr [eax+0*8*8+8]
        fld  qword ptr [esi+1*8*8]
        fmul qword ptr [eax+1*8*8+8]
        fadd
        fld  qword ptr [esi+2*8*8]
        fmul qword ptr [eax+2*8*8+8]
        fadd
        fld  qword ptr [esi+3*8*8]
        fmul qword ptr [eax+3*8*8+8]
        fadd
        fld  qword ptr [esi+4*8*8]
        fmul qword ptr [eax+4*8*8+8]
        fadd
        fld  qword ptr [esi+5*8*8]
        fmul qword ptr [eax+5*8*8+8]
        fadd
        fld  qword ptr [esi+6*8*8]
        fmul qword ptr [eax+6*8*8+8]
        fadd
        fld  qword ptr [esi+7*8*8]
        fmul qword ptr [eax+7*8*8+8]
        fadd
        fadd st(0), st(1)
        add eax, 8*2

        fxch st(2)
        fstp qword ptr [edi]
        fxch st(1)
        fstp qword ptr [edi+8*8]
        add edi, 8*8*2

        dec ecx

        jnz __col2
        add eax, -8*8
        add esi, +8
        add edi, 8-8*8*8

        sub ebx, 0x80000001
        js  __row2
        //align 16
        test ebx, ebx // align jump &| redo flags
        jnz __row2
        ffree st(0) // bye bye 0.5

        // set x87 to floor mode
        fstcw [fpold]
        movzx eax, [fpold]

        or eax, 0x0400 // round down - floor
        mov [fpnew], ax
        fldcw [fpnew]

        // now floor the damn array
        lea esi, [rnd]
        mov edi, [block]
        mov ebx, -256 // clip min
        mov edx, +255 // clip max
        mov ecx, 8
        align 16
        __floor:
        fld   qword ptr [esi+0*8]
        fistp dword ptr [int0]
        mov eax, [int0]
        cmp   eax, ebx
        cmovl eax, ebx
        cmp   eax, edx
        cmovg eax, edx
        fld   qword ptr [esi+1*8]
        fistp dword ptr [int1]
        mov word ptr [edi+0*2], ax
        mov eax, [int1]
        cmp   eax, ebx
        cmovl eax, ebx
        cmp   eax, edx
        cmovg eax, edx
        fld   qword ptr [esi+2*8]
        fistp dword ptr [int2]
        mov word ptr [edi+1*2], ax
        mov eax, [int2]
        cmp   eax, ebx
        cmovl eax, ebx
        cmp   eax, edx
        cmovg eax, edx
        fld   qword ptr [esi+3*8]
        fistp dword ptr [int3]
        mov word ptr [edi+2*2], ax
        mov eax, [int3]
        cmp   eax, ebx
        cmovl eax, ebx
        cmp   eax, edx
        cmovg eax, edx
        fld   qword ptr [esi+4*8]
        fistp dword ptr [int4]
        mov word ptr [edi+3*2], ax
        mov eax, [int4]
        cmp   eax, ebx
        cmovl eax, ebx
        cmp   eax, edx
        cmovg eax, edx
        fld   qword ptr [esi+5*8]
        fistp dword ptr [int5]
        mov word ptr [edi+4*2], ax
        mov eax, [int5]
        cmp   eax, ebx
        cmovl eax, ebx
        cmp   eax, edx
        cmovg eax, edx
        fld   qword ptr [esi+6*8]
        fistp dword ptr [int6]
        mov word ptr [edi+5*2], ax
        mov eax, [int6]
        cmp   eax, ebx
        cmovl eax, ebx
        cmp   eax, edx
        cmovg eax, edx
        fld   qword ptr [esi+7*8]
        fistp dword ptr [int7]
        mov word ptr [edi+6*2], ax
        mov eax, [int7]
        cmp   eax, ebx
        cmovl eax, ebx
        cmp   eax, edx
        cmovg eax, edx
        mov word ptr [edi+7*2], ax

        add esi, 8*8
        add edi, 8*2

        sub ecx, 0x80000001
        js  __floor
        //align 16
        test ecx, ecx // align jump &| redo flags
        jnz __floor

        // set x87 to default mode
        fldcw [fpold]
    };
}
