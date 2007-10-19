/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 *  skl_dct16_LLM.cpp
 *
 *  "Fast and precise" LLM implementation of FDCT/IDCT, where
 *  rotations are decomposed using:
 *    tmp = (x+y).cos t
 *    x' = tmp + y.(sin t - cos t)
 *    y' = tmp - x.(sin t + cos t)
 *
 *  See details at http://skl.planet-d.net/coding/dct.html
 *  and at the end of this file...
 *
 * Reference (e.g.):
 *  Loeffler C., Ligtenberg A., and Moschytz C.S.:
 *    Practical Fast 1D DCT Algorithm with Eleven Multiplications,
 *  Proc. ICASSP 1989, 988-991.
 *
 *  IEEE-1180-like error specs for FDCT:
 * Peak error:   1.0000
 * Peak MSE:     0.0340
 * Overall MSE:  0.0200
 * Peak ME:      0.0191
 * Overall ME:   -0.0033
 *
 *  error specs for IDCT:
 * Peak error:   1.0000
 * Peak MSE:     0.0065
 * Overall MSE:  0.0051
 * Peak ME:      0.0015
 * Overall ME:   0.0000
 *
 ********************************************************/

//#include "skl.h"
#include <math.h>
#include "dsputil.h"
#include "skl_dct.h"

typedef DCTELEM TYPE;
typedef uint8_t SKL_BYTE;

//////////////////////////////////////////////////////////

#define LOAD_BUTF(m1, m2, a, b, tmp, S) \
  (m1) = (S)[(a)] + (S)[(b)]; \
  (m2) = (S)[(a)] - (S)[(b)]

#define BUTF(a, b, tmp) \
  (tmp) = (a)+(b); \
  (b) = (a)-(b);   \
  (a) = (tmp)

#define ROTATE(m1,m2,c,k1,k2,tmp,Fix,Rnd) \
  (tmp) = ( (m1) + (m2) )*(c); \
  (m1) *= k1; \
  (m2) *= k2; \
  (tmp) += (Rnd); \
  (m1) = ((m1)+(tmp))>>(Fix);             \
  (m2) = ((m2)+(tmp))>>(Fix);

#define ROTATE2(m1,m2,c,k1,k2,tmp) \
  (tmp) = ( (m1) + (m2) )*(c); \
  (m1) *= k1; \
  (m2) *= k2; \
  (m1) = (m1)+(tmp); \
  (m2) = (m2)+(tmp);

#define ROTATE0(m1,m2,c,k1,k2,tmp) \
  (m1) = ( (m2) )*(c); \
  (m2) = (m2)*k2+(m1);

#define SHIFTL(x,n)   ((x)<<(n))
#define SHIFTR(x, n)  ((x)>>(n))
#define HALF(n)       (1<<((n)-1))

#define IPASS 3
#define FPASS 2
#define FIX  16

#if 1

#define ROT6_C     35468
#define ROT6_SmC   50159
#define ROT6_SpC  121095
#define ROT17_C    77062
#define ROT17_SmC  25571
#define ROT17_SpC 128553
#define ROT37_C    58981
#define ROT37_SmC  98391
#define ROT37_SpC  19571
#define ROT13_C   167963
#define ROT13_SmC 134553
#define ROT13_SpC 201373

#else

#define FX(x) ( (int)floor((x)*(1<<FIX) + .5 ) )

static const double c1 = cos(1.*M_PI/16);
static const double c2 = cos(2.*M_PI/16);
static const double c3 = cos(3.*M_PI/16);
static const double c4 = cos(4.*M_PI/16);
static const double c5 = cos(5.*M_PI/16);
static const double c6 = cos(6.*M_PI/16);
static const double c7 = cos(7.*M_PI/16);

static const int ROT6_C   = FX(c2-c6);  // 0.541
static const int ROT6_SmC = FX(2*c6);   // 0.765
static const int ROT6_SpC = FX(2*c2);   // 1.847

static const int ROT17_C   = FX(c1+c7);  // 1.175
static const int ROT17_SmC = FX(2*c7);   // 0.390
static const int ROT17_SpC = FX(2*c1);   // 1.961

static const int ROT37_C   = FX((c3-c7)/c4);  // 0.899
static const int ROT37_SmC = FX(2*(c5+c7));   // 1.501
static const int ROT37_SpC = FX(2*(c1-c3));   // 0.298

static const int ROT13_C   = FX((c1+c3)/c4);  // 2.562
static const int ROT13_SmC = FX(2*(c3+c7));   // 2.053
static const int ROT13_SpC = FX(2*(c1+c5));   // 3.072

#endif

//////////////////////////////////////////////////////////

void Skl_Dct16_C( TYPE *In )
{
  TYPE *pIn;
  int i;

  pIn = In;
  for(i=8; i>0; --i)
  {
    int mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7, Spill;

      // odd

    LOAD_BUTF(mm1,mm6, 1, 6, mm0, pIn);
    LOAD_BUTF(mm2,mm5, 2, 5, mm0, pIn);
    LOAD_BUTF(mm3,mm4, 3, 4, mm0, pIn);
    LOAD_BUTF(mm0,mm7, 0, 7, Spill, pIn);

    BUTF(mm1, mm2, Spill);
    BUTF(mm0, mm3, Spill);

    ROTATE(mm3, mm2, ROT6_C, ROT6_SmC, -ROT6_SpC, Spill, FIX-FPASS, HALF(FIX-FPASS));
    pIn[2] = mm3;
    pIn[6] = mm2;

    BUTF(mm0, mm1, Spill);
    pIn[0] = SHIFTL(mm0, FPASS);
    pIn[4] = SHIFTL(mm1, FPASS);


      // even

    mm3 = mm5 + mm7;
    mm2 = mm4 + mm6;
    ROTATE(mm2, mm3,  ROT17_C, -ROT17_SpC, -ROT17_SmC, mm0, FIX-FPASS, HALF(FIX-FPASS));
    ROTATE(mm4, mm7, -ROT37_C,  ROT37_SpC,  ROT37_SmC, mm0, FIX-FPASS, HALF(FIX-FPASS));
    mm7 += mm3;
    mm4 += mm2;
    pIn[1] = mm7;
    pIn[7] = mm4;

    ROTATE(mm5, mm6, -ROT13_C,  ROT13_SmC,  ROT13_SpC, mm0, FIX-FPASS, HALF(FIX-FPASS));
    mm5 += mm3;
    mm6 += mm2;
    pIn[3] = mm6;
    pIn[5] = mm5;

    pIn  += 8;
  }

  pIn = In;
  for(i=8; i>0; --i)
  {
    int mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7, Spill;

      // odd

    LOAD_BUTF(mm1,mm6, 1*8, 6*8, mm0, pIn);
    LOAD_BUTF(mm2,mm5, 2*8, 5*8, mm0, pIn);
    BUTF(mm1, mm2, mm0);

    LOAD_BUTF(mm3,mm4, 3*8, 4*8, mm0, pIn);
    LOAD_BUTF(mm0,mm7, 0*8, 7*8, Spill, pIn);
    BUTF(mm0, mm3, Spill);

    ROTATE(mm3, mm2, ROT6_C, ROT6_SmC, -ROT6_SpC, Spill, 0,  HALF(FIX+FPASS+3));
    pIn[2*8] = (TYPE)SHIFTR(mm3,FIX+FPASS+3);
    pIn[6*8] = (TYPE)SHIFTR(mm2,FIX+FPASS+3);

    mm0 += HALF(FPASS+3) - 1;
    BUTF(mm0, mm1, Spill);
    pIn[0*8] = (TYPE)SHIFTR(mm0, FPASS+3);
    pIn[4*8] = (TYPE)SHIFTR(mm1, FPASS+3);

      // even

    mm3 = mm5 + mm7;
    mm2 = mm4 + mm6;

    ROTATE(mm2, mm3,  ROT17_C, -ROT17_SpC, -ROT17_SmC, mm0, 0, HALF(FIX+FPASS+3));
    ROTATE2(mm4, mm7, -ROT37_C,  ROT37_SpC,  ROT37_SmC, mm0);
    mm7 += mm3;
    mm4 += mm2;
    pIn[7*8] = (TYPE)SHIFTR(mm4,FIX+FPASS+3);
    pIn[1*8] = (TYPE)SHIFTR(mm7,FIX+FPASS+3);

    ROTATE2(mm5, mm6, -ROT13_C,  ROT13_SmC,  ROT13_SpC, mm0);
    mm5 += mm3;
    mm6 += mm2;
    pIn[5*8] = (TYPE)SHIFTR(mm5,FIX+FPASS+3);
    pIn[3*8] = (TYPE)SHIFTR(mm6,FIX+FPASS+3);

    pIn++;
  }
}

//////////////////////////////////////////////////////////

void Skl_IDct16_C( TYPE *In )
{
  TYPE *pIn;
  int i;

  pIn = In;
  for (i=8; i>0; --i)
  {
    int mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7, Spill;

      // odd

    mm4 = (int)pIn[7];
    mm5 = (int)pIn[5];
    mm6 = (int)pIn[3];
    mm7 = (int)pIn[1];

    mm2 = mm4 + mm6;
    mm3 = mm5 + mm7;
    ROTATE2(mm2, mm3,  ROT17_C, -ROT17_SpC, -ROT17_SmC, mm1);
    ROTATE2(mm4, mm7, -ROT37_C,  ROT37_SpC,  ROT37_SmC, mm1);
    ROTATE2(mm5, mm6, -ROT13_C,  ROT13_SmC,  ROT13_SpC, mm1);

    mm4 += mm2;
    mm5 += mm3;
    mm6 += mm2;
    mm7 += mm3;

      // even

    mm3 = (int)pIn[2];
    mm2 = (int)pIn[6];

    ROTATE2(mm3, mm2, ROT6_C, ROT6_SmC, -ROT6_SpC, mm1);

    LOAD_BUTF(mm0, mm1, 0, 4, Spill, pIn);
    mm0 = SHIFTL(mm0, FIX) + HALF(FIX-IPASS);
    mm1 = SHIFTL(mm1, FIX) + HALF(FIX-IPASS);
    BUTF(mm0, mm3, Spill);
    BUTF(mm1, mm2, Spill);


    BUTF(mm0, mm7, Spill);
    pIn[0] = SHIFTR(mm0, FIX-IPASS);
    pIn[7] = SHIFTR(mm7, FIX-IPASS);
    BUTF(mm1, mm6, mm0);
    pIn[1] = SHIFTR(mm1, FIX-IPASS);
    pIn[6] = SHIFTR(mm6, FIX-IPASS);
    BUTF(mm2, mm5, mm0);
    pIn[2] = SHIFTR(mm2, FIX-IPASS);
    pIn[5] = SHIFTR(mm5, FIX-IPASS);
    BUTF(mm3, mm4, mm0);
    pIn[3] = SHIFTR(mm3, FIX-IPASS);
    pIn[4] = SHIFTR(mm4, FIX-IPASS);

    pIn += 8;
  }


  pIn = In;
  for (i=8; i>0; --i)
  {
    int mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7, Spill;

      // odd

    mm4 = (int)pIn[7*8];
    mm5 = (int)pIn[5*8];
    mm6 = (int)pIn[3*8];
    mm7 = (int)pIn[1*8];


    mm2 = mm4 + mm6;
    mm3 = mm5 + mm7;
    ROTATE2(mm2, mm3,  ROT17_C, -ROT17_SpC, -ROT17_SmC, mm1);
    ROTATE2(mm4, mm7, -ROT37_C,  ROT37_SpC,  ROT37_SmC, mm1);
    ROTATE2(mm5, mm6, -ROT13_C,  ROT13_SmC,  ROT13_SpC, mm1);

    mm4 += mm2;
    mm5 += mm3;
    mm6 += mm2;
    mm7 += mm3;

      // even

    mm3 = (int)pIn[2*8];
    mm2 = (int)pIn[6*8];

    ROTATE2(mm3, mm2, ROT6_C, ROT6_SmC, -ROT6_SpC, mm1);

    LOAD_BUTF(mm0, mm1, 0*8, 4*8, Spill, pIn);
    mm0 = SHIFTL(mm0, FIX) + HALF(FIX+IPASS+3);
    mm1 = SHIFTL(mm1, FIX) + HALF(FIX+IPASS+3);
    BUTF(mm0, mm3, Spill);
    BUTF(mm1, mm2, Spill);

    BUTF(mm0, mm7, Spill);
    pIn[8*0] = (TYPE) SHIFTR(mm0, FIX+IPASS+3);
    pIn[8*7] = (TYPE) SHIFTR(mm7, FIX+IPASS+3);
    BUTF(mm1, mm6, mm0);
    pIn[8*1] = (TYPE) SHIFTR(mm1, FIX+IPASS+3);
    pIn[8*6] = (TYPE) SHIFTR(mm6, FIX+IPASS+3);
    BUTF(mm2, mm5, mm0);
    pIn[8*2] = (TYPE) SHIFTR(mm2, FIX+IPASS+3);
    pIn[8*5] = (TYPE) SHIFTR(mm5, FIX+IPASS+3);
    BUTF(mm3, mm4, mm0);
    pIn[8*3] = (TYPE) SHIFTR(mm3, FIX+IPASS+3);
    pIn[8*4] = (TYPE) SHIFTR(mm4, FIX+IPASS+3);

    pIn++;
  }
}

void Skl_IDct16_Sparse_C( TYPE *In )
{
  TYPE *pIn;
  int i;
  int Rows = 8;

  pIn = In;
  while(Rows-->0)
  {
    const int Left  = pIn[0]|pIn[1]|pIn[2]|pIn[3];
    const int Right = pIn[4]|pIn[5]|pIn[6]|pIn[7];
    if (!Right) {
      if (Left) {
        int mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7, Spill;

          // odd

        mm6 = (int)pIn[3];
        mm7 = (int)pIn[1];

        mm2 = mm6;
        mm3 = mm7;
        ROTATE2(mm2, mm3,  ROT17_C, -ROT17_SpC, -ROT17_SmC, mm1);
        ROTATE0(mm4, mm7, -ROT37_C,  ROT37_SpC,  ROT37_SmC, mm1);
        ROTATE0(mm5, mm6, -ROT13_C,  ROT13_SmC,  ROT13_SpC, mm1);

        mm4 += mm2;
        mm5 += mm3;
        mm6 += mm2;
        mm7 += mm3;

          // even

        mm3 = (int)pIn[2];
        mm2 = 0;

        ROTATE2(mm3, mm2, ROT6_C, ROT6_SmC, -ROT6_SpC, mm1);

        mm0 = (int)pIn[0];
        mm0 = SHIFTL(mm0, FIX) + HALF(FIX-IPASS);
        mm1 = mm0;
        BUTF(mm0, mm3, Spill);
        BUTF(mm1, mm2, Spill);


        BUTF(mm0, mm7, Spill);
        pIn[0] = SHIFTR(mm0, FIX-IPASS);
        pIn[7] = SHIFTR(mm7, FIX-IPASS);
        BUTF(mm1, mm6, mm0);
        pIn[1] = SHIFTR(mm1, FIX-IPASS);
        pIn[6] = SHIFTR(mm6, FIX-IPASS);
        BUTF(mm2, mm5, mm0);
        pIn[2] = SHIFTR(mm2, FIX-IPASS);
        pIn[5] = SHIFTR(mm5, FIX-IPASS);
        BUTF(mm3, mm4, mm0);
        pIn[3] = SHIFTR(mm3, FIX-IPASS);
        pIn[4] = SHIFTR(mm4, FIX-IPASS);
      }
    }
    else
    {
      int mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7, Spill;

        // odd

      mm4 = (int)pIn[7];
      mm5 = (int)pIn[5];
      mm6 = (int)pIn[3];
      mm7 = (int)pIn[1];

      mm2 = mm4 + mm6;
      mm3 = mm5 + mm7;
      ROTATE2(mm2, mm3,  ROT17_C, -ROT17_SpC, -ROT17_SmC, mm1);
      ROTATE2(mm4, mm7, -ROT37_C,  ROT37_SpC,  ROT37_SmC, mm1);
      ROTATE2(mm5, mm6, -ROT13_C,  ROT13_SmC,  ROT13_SpC, mm1);

      mm4 += mm2;
      mm5 += mm3;
      mm6 += mm2;
      mm7 += mm3;

        // even

      mm3 = (int)pIn[2];
      mm2 = (int)pIn[6];

      ROTATE2(mm3, mm2, ROT6_C, ROT6_SmC, -ROT6_SpC, mm1);

      LOAD_BUTF(mm0, mm1, 0, 4, Spill, pIn);
      mm0 = SHIFTL(mm0, FIX) + HALF(FIX-IPASS);
      mm1 = SHIFTL(mm1, FIX) + HALF(FIX-IPASS);
      BUTF(mm0, mm3, Spill);
      BUTF(mm1, mm2, Spill);


      BUTF(mm0, mm7, Spill);
      pIn[0] = SHIFTR(mm0, FIX-IPASS);
      pIn[7] = SHIFTR(mm7, FIX-IPASS);
      BUTF(mm1, mm6, mm0);
      pIn[1] = SHIFTR(mm1, FIX-IPASS);
      pIn[6] = SHIFTR(mm6, FIX-IPASS);
      BUTF(mm2, mm5, mm0);
      pIn[2] = SHIFTR(mm2, FIX-IPASS);
      pIn[5] = SHIFTR(mm5, FIX-IPASS);
      BUTF(mm3, mm4, mm0);
      pIn[3] = SHIFTR(mm3, FIX-IPASS);
      pIn[4] = SHIFTR(mm4, FIX-IPASS);
    }
    pIn += 8;
  }

  pIn = In;
  for (i=8; i>0; --i)
  {
    int mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7, Spill;

      // odd

    mm4 = (int)pIn[7*8];
    mm5 = (int)pIn[5*8];
    mm6 = (int)pIn[3*8];
    mm7 = (int)pIn[1*8];


    mm2 = mm4 + mm6;
    mm3 = mm5 + mm7;
    ROTATE2(mm2, mm3,  ROT17_C, -ROT17_SpC, -ROT17_SmC, mm1);
    ROTATE2(mm4, mm7, -ROT37_C,  ROT37_SpC,  ROT37_SmC, mm1);
    ROTATE2(mm5, mm6, -ROT13_C,  ROT13_SmC,  ROT13_SpC, mm1);

    mm4 += mm2;
    mm5 += mm3;
    mm6 += mm2;
    mm7 += mm3;

      // even

    mm3 = (int)pIn[2*8];
    mm2 = (int)pIn[6*8];

    ROTATE2(mm3, mm2, ROT6_C, ROT6_SmC, -ROT6_SpC, mm1);

    LOAD_BUTF(mm0, mm1, 0*8, 4*8, Spill, pIn);
    mm0 = SHIFTL(mm0, FIX) + HALF(FIX+IPASS+3);
    mm1 = SHIFTL(mm1, FIX) + HALF(FIX+IPASS+3);
    BUTF(mm0, mm3, Spill);
    BUTF(mm1, mm2, Spill);

    BUTF(mm0, mm7, Spill);
    pIn[8*0] = (TYPE) SHIFTR(mm0, FIX+IPASS+3);
    pIn[8*7] = (TYPE) SHIFTR(mm7, FIX+IPASS+3);
    BUTF(mm1, mm6, mm0);
    pIn[8*1] = (TYPE) SHIFTR(mm1, FIX+IPASS+3);
    pIn[8*6] = (TYPE) SHIFTR(mm6, FIX+IPASS+3);
    BUTF(mm2, mm5, mm0);
    pIn[8*2] = (TYPE) SHIFTR(mm2, FIX+IPASS+3);
    pIn[8*5] = (TYPE) SHIFTR(mm5, FIX+IPASS+3);
    BUTF(mm3, mm4, mm0);
    pIn[8*3] = (TYPE) SHIFTR(mm3, FIX+IPASS+3);
    pIn[8*4] = (TYPE) SHIFTR(mm4, FIX+IPASS+3);

    pIn++;
  }
}

//////////////////////////////////////////////////////////
// IDCT + Put/Add shortcuts

#define CLIP(v)  ((v)<0 ? 0 : (v)>255 ? 255 : (SKL_BYTE)(v))
void Skl_IDct16_Put_C(SKL_BYTE *Dst, int BpS, TYPE *In)
{
  int j;
  Skl_IDct16_Sparse_C( In );
  for(j=8; j>0; --j) {
    Dst[0] = CLIP(In[0]);
    Dst[1] = CLIP(In[1]);
    Dst[2] = CLIP(In[2]);
    Dst[3] = CLIP(In[3]);
    Dst[4] = CLIP(In[4]);
    Dst[5] = CLIP(In[5]);
    Dst[6] = CLIP(In[6]);
    Dst[7] = CLIP(In[7]);

    Dst += BpS;
    In += 8;
  }
}
void Skl_IDct16_Add_C(SKL_BYTE *Dst, int BpS, TYPE *In)
{
  int j;
  Skl_IDct16_Sparse_C( In );
  for(j=8; j>0; --j) {
    int v;
    v = In[0] + Dst[0]; Dst[0] = CLIP(v);
    v = In[1] + Dst[1]; Dst[1] = CLIP(v);
    v = In[2] + Dst[2]; Dst[2] = CLIP(v);
    v = In[3] + Dst[3]; Dst[3] = CLIP(v);
    v = In[4] + Dst[4]; Dst[4] = CLIP(v);
    v = In[5] + Dst[5]; Dst[5] = CLIP(v);
    v = In[6] + Dst[6]; Dst[6] = CLIP(v);
    v = In[7] + Dst[7]; Dst[7] = CLIP(v);
    Dst += BpS;
    In += 8;
  }
}
#undef CLIP


#undef FIX
#undef FPASS
#undef IPASS

#undef BUTF
#undef LOAD_BUTF
#undef ROTATE
#undef ROTATE2
#undef SHIFTL
#undef SHIFTR
#undef TYPE

//////////////////////////////////////////////////////////
//   - Data flow schematics for FDCT -
// Output is scaled by 2.sqrt(2)
// Initial butterflies (in0/in7, etc.) are not fully depicted.
// Note: Rot6 coeffs are multiplied by sqrt(2).
//////////////////////////////////////////////////////////
/*
 <---------Stage1 =even part=----------->

 in3 mm3  +_____.___-___________.____* out6
  x              \ /            |
 in4 mm4          \             |
                 / \            |
 in0 mm0  +_____o___+__.___-___ | ___* out4
  x                     \ /     |
 in7 mm7                 \    (Rot6)
                        / \     |
 in1 mm1  +_____o___+__o___+___ | ___* out0
  x              \ /            |
 in6 mm6          /             |
                 / \            |
 in2 mm2  +_____.___-___________o____* out2
  x
 in5 mm5

 <---------Stage2 =odd part=---------------->

 mm7*___._________.___-___[xSqrt2]___* out3
        |          \ /
      (Rot3)        \
        |          / \
 mm5*__ | ___o____o___+___.___-______* out7
        |    |             \ /
        |  (Rot1)           \
        |    |             / \
 mm6*__ |____.____o___+___o___+______* out1
        |          \ /
        |           /
        |          / \
 mm4*___o_________.___-___[xSqrt2]___* out5



    Alternative schematics for stage 2:
    -----------------------------------

 mm7 *___[xSqrt2]____o___+____o_______* out1
                      \ /     |
                       /    (Rot1)
                      / \     |
 mm6 *____o___+______.___-___ | __.___* out5
           \ /                |   |
            /                 |   |
           / \                |   |
 mm5 *____.___-______.___-____.__ | __* out7
                      \ /         |
                       \        (Rot3)
                      / \         |
 mm4 *___[xSqrt2]____o___+________o___* out3

*/

//////////////////////////////////////////////////////////
//   - Data flow schematics for IDCT -
// Output is scaled by 2.sqrt(2)
// Note: Rot6 coeffs are multiplied by sqrt(2).
//////////////////////////////////////////////////////////
/*
 <---------Stage 1 =even part=---------------->

 in3 *___[xSqrt2]____.___-________o____* mm7
                      \ /         |
                       /          |
                      / \         |
 in1 *____o___+______o___+____.__ | ___* mm3
           \ /                |   |
            /               (Rot3)|
           / \                |   |
 in7 *____.___-______.___-____o__ | ___* mm1
                      \ /         |
                       \        (Rot1)
                      / \         |
 in5 *___[xSqrt2]____o___+________.____* mm5


    Alternative schematics for stage 1:
    -----------------------------------

 in1 *________.____.___-_____[xSqrt2]__* mm1
              |     \ /
            (Rot1)   \
              |     / \
 in5 *___o___ | ___o___+_____.___-_____* mm5
         |    |               \ /
         |    |                \
         |    |               / \
 in7 *__ | ___o____o___+__ __o___+_____* mm7
         |          \ /
       (Rot3)        /
         |          / \
 in3 *___._________.___-_____[xSqrt2]__* mm3

 <---------Stage2 =odd part=--------->

 in6 *__o___________o___+___* mm6 -> butterfly with mm3 -> out3/out4
 mm6    |            \ /
        |             /
        |            / \
 in4 *_ | ___o___+__.___-___* mm4 -> butf. w/ mm1 -> out7/out0
 mm4    |     \ /
      (Rot6)   /
        |     / \
 in0 *_ | ___.___-__.___-___* mm0 -> butf. w/ mm5 -> out6/out1
 mm0    |            \ /
        |             \
        |            / \
 in2 *__.___________o___+___* mm2 -> butf. w/ mm7 -> out2/out5
 mm2

*/
//////////////////////////////////////////////////////////






