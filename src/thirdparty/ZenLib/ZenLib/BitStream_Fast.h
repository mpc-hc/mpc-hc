/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Read a stream bit per bit
// Can read up to 32 bits at once
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenBitStream_FastH
#define ZenBitStream_FastH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Conf.h"
//---------------------------------------------------------------------------

namespace ZenLib
{

#ifndef MIN
    #define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif

class BitStream_Fast
{
public:
    BitStream_Fast ()                                                           {Buffer=NULL;
                                                                                 Buffer_Size=Buffer_Size_Init=0;
                                                                                 BufferUnderRun=false;}
    BitStream_Fast (const int8u* Buffer_, size_t Size_)                         {Buffer=Buffer_;
                                                                                 Buffer_Size=Buffer_Size_Init=Size_*8; //Size is in bits
                                                                                 BufferUnderRun=false;}
    ~BitStream_Fast ()                                                          {}

    void Attach(const int8u* Buffer_, size_t Size_)
    {
        Buffer=Buffer_;
        Buffer_Size=Buffer_Size_Init=Size_*8; //Size is in bits
        BufferUnderRun=false;
    }

    bool  GetB ()
    {
        if (Buffer_Size%8)
        {
            Buffer_Size--;
            return ((LastByte>>(Buffer_Size%8))&0x1)?true:false;
        }

        if (!Buffer_Size)
        {
            Buffer_Size=0;
            BufferUnderRun=true;
            return false;
        }

        LastByte=*Buffer;
        Buffer++;
        Buffer_Size--;
        return (LastByte&0x80)?true:false;
    }

    int8u  Get1 (int8u HowMany)
    {
        int8u ToReturn;
        static const int8u Mask[9]=
        {
            0x00,
            0x01, 0x03, 0x07, 0x0f,
            0x1f, 0x3f, 0x7f, 0xff,
        };

        if (HowMany<=(Buffer_Size%8))
        {
            Buffer_Size-=HowMany;
            return (LastByte>>(Buffer_Size%8))&Mask[HowMany];
        }

        if (HowMany>Buffer_Size)
        {
            Buffer_Size=0;
            BufferUnderRun=true;
            return 0;
        }

        int8u NewBits=HowMany-(Buffer_Size%8);
        if (NewBits==8)
            ToReturn=0;
        else
            ToReturn=LastByte<<NewBits;
        LastByte=*Buffer;
        Buffer++;
        Buffer_Size-=HowMany;
        ToReturn|=(LastByte>>(Buffer_Size%8))&Mask[NewBits];
        return ToReturn&Mask[HowMany];
    }

    int16u Get2 (int8u HowMany)
    {
        int16u ToReturn;
        static const int16u Mask[17]=
        {
            0x0000,
            0x0001, 0x0003, 0x0007, 0x000f,
            0x001f, 0x003f, 0x007f, 0x00ff,
            0x01ff, 0x03ff, 0x07ff, 0x0fff,
            0x1fff, 0x3fff, 0x7fff, 0xffff,
        };

        if (HowMany<=(Buffer_Size%8))
        {
            Buffer_Size-=HowMany;
            return (LastByte>>(Buffer_Size%8))&Mask[HowMany];
        }

        if (HowMany>Buffer_Size)
        {
            Buffer_Size=0;
            BufferUnderRun=true;
            return 0;
        }

        int8u NewBits=HowMany-(Buffer_Size%8);
        if (NewBits==16)
            ToReturn=0;
        else
            ToReturn=LastByte<<NewBits;
        if ((NewBits-1)>>3)
        {
            NewBits-=8;
            ToReturn|=*Buffer<<NewBits;
            Buffer++;
        }
        LastByte=*Buffer;
        Buffer++;
        Buffer_Size-=HowMany;
        ToReturn|=(LastByte>>(Buffer_Size%8))&Mask[NewBits];
        return ToReturn&Mask[HowMany];
    }

    int32u Get4 (int8u HowMany)
    {
        int32u ToReturn;
        static const int32u Mask[33]=
        {
            0x00000000,
            0x00000001, 0x00000003, 0x00000007, 0x0000000f,
            0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
            0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
            0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,
            0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff,
            0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
            0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff,
            0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff,
        };

        if (HowMany<=(Buffer_Size%8))
        {
            Buffer_Size-=HowMany;
            return (LastByte>>(Buffer_Size%8))&Mask[HowMany];
        }

        if (HowMany>Buffer_Size)
        {
            Buffer_Size=0;
            BufferUnderRun=true;
            return 0;
        }

        int8u NewBits=HowMany-(Buffer_Size%8);
        if (NewBits==32)
            ToReturn=0;
        else
            ToReturn=LastByte<<NewBits;
        switch ((NewBits-1)>>3)
        {
            case 3 :    NewBits-=8;
                        ToReturn|=*Buffer<<NewBits;
                        Buffer++;
            case 2 :    NewBits-=8;
                        ToReturn|=*Buffer<<NewBits;
                        Buffer++;
            case 1 :    NewBits-=8;
                        ToReturn|=*Buffer<<NewBits;
                        Buffer++;
            default:    ;
        }
        LastByte=*Buffer;
        Buffer++;
        Buffer_Size-=HowMany;
        ToReturn|=(LastByte>>(Buffer_Size%8))&Mask[NewBits];
        return ToReturn&Mask[HowMany];
    }

    int64u Get8 (int8u HowMany)
    {
        if (HowMany>64)
            return 0; //Not supported
        int8u HowMany1, HowMany2;
        int64u Value1, Value2;
        if (HowMany>32)
            HowMany1=HowMany-32;
        else
            HowMany1=0;
        HowMany2=HowMany-HowMany1;
        Value1=Get4(HowMany1);
        Value2=Get4(HowMany2);
        if (BufferUnderRun)
            return 0;
        return Value1*0x100000000LL+Value2;
    }

    void Skip (size_t HowMany)
    {
        if (HowMany<=(Buffer_Size%8))
        {
            Buffer_Size-=HowMany;
            return;
        }

        if (HowMany>Buffer_Size)
        {
            Buffer_Size=0;
            BufferUnderRun=true;
            return;
        }

        Buffer+=(HowMany-(Buffer_Size%8)-1)>>3;
        LastByte=*Buffer;
        Buffer++;
        Buffer_Size-=HowMany;
    }

    bool   PeekB()
    {
        if (Buffer_Size%8)
            return ((LastByte>>((Buffer_Size-1)%8))&0x1)?true:false;

        if (!Buffer_Size)
        {
            Buffer_Size=0;
            BufferUnderRun=true;
            return false;
        }

        return ((*Buffer)&0x80)?true:false;
    }

    int8u  Peek1(int8u HowMany)
    {
        int8u ToReturn;
        static const int8u Mask[9]=
        {
            0x00,
            0x01, 0x03, 0x07, 0x0f,
            0x1f, 0x3f, 0x7f, 0xff,
        };

        if (HowMany<=(Buffer_Size%8))
            return (LastByte>>((Buffer_Size-HowMany)%8))&Mask[HowMany];

        if (HowMany>Buffer_Size)
        {
            Buffer_Size=0;
            BufferUnderRun=true;
            return 0;
        }

        int8u NewBits=HowMany-(Buffer_Size%8);
        if (NewBits==8)
            ToReturn=0;
        else
            ToReturn=LastByte<<NewBits;
        ToReturn|=((*Buffer)>>((Buffer_Size-HowMany)%8))&Mask[NewBits];

        return ToReturn&Mask[HowMany];
    }

    int16u Peek2(int8u HowMany)
    {
        int16u ToReturn;
        static const int16u Mask[17]=
        {
            0x0000,
            0x0001, 0x0003, 0x0007, 0x000f,
            0x001f, 0x003f, 0x007f, 0x00ff,
            0x01ff, 0x03ff, 0x07ff, 0x0fff,
            0x1fff, 0x3fff, 0x7fff, 0xffff,
        };

        if (HowMany<=(Buffer_Size%8))
            return (LastByte>>((Buffer_Size-HowMany)%8))&Mask[HowMany];

        if (HowMany>Buffer_Size)
        {
            Buffer_Size=0;
            BufferUnderRun=true;
            return 0;
        }

        const int8u* Buffer_Save=Buffer;

        int8u NewBits=HowMany-(Buffer_Size%8);
        if (NewBits==16)
            ToReturn=0;
        else
            ToReturn=LastByte<<NewBits;
        if ((NewBits-1)>>3)
        {
            NewBits-=8;
            ToReturn|=*Buffer<<NewBits;
            Buffer++;
        }
        ToReturn|=((*Buffer)>>((Buffer_Size-HowMany)%8))&Mask[NewBits];

        Buffer=Buffer_Save;

        return ToReturn&Mask[HowMany];
    }

    int32u Peek4(int8u HowMany)
    {
        int32u ToReturn;
        static const int32u Mask[33]=
        {
            0x00000000,
            0x00000001, 0x00000003, 0x00000007, 0x0000000f,
            0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
            0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
            0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,
            0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff,
            0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
            0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff,
            0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff,
        };

        if (HowMany<=(Buffer_Size%8))
            return (LastByte>>((Buffer_Size-HowMany)%8))&Mask[HowMany];

        if (HowMany>Buffer_Size)
        {
            Buffer_Size=0;
            BufferUnderRun=true;
            return 0;
        }

        const int8u* Buffer_Save=Buffer;

        int8u NewBits=HowMany-(Buffer_Size%8);
        if (NewBits==32)
            ToReturn=0;
        else
            ToReturn=LastByte<<NewBits;
        switch ((NewBits-1)>>3)
        {
            case 3 :    NewBits-=8;
                        ToReturn|=*Buffer<<NewBits;
                        Buffer++;
            case 2 :    NewBits-=8;
                        ToReturn|=*Buffer<<NewBits;
                        Buffer++;
            case 1 :    NewBits-=8;
                        ToReturn|=*Buffer<<NewBits;
                        Buffer++;
            default:    ;
        }
        ToReturn|=((*Buffer)>>((Buffer_Size-HowMany)%8))&Mask[NewBits];

        Buffer=Buffer_Save;

        return ToReturn&Mask[HowMany];
    }

    int64u Peek8(int8u HowMany)
    {
        return (int64u)Peek4(HowMany); //Not yet implemented
    }

    inline size_t Remain () const //How many bits remain?
    {
        return Buffer_Size;
    }

    inline void Byte_Align()
    {
        Skip (Buffer_Size%8);
    }

    inline size_t Offset_Get() const
    {
        return (Buffer_Size_Init-Buffer_Size)/8;
    }

    inline size_t BitOffset_Get() const
    {
        return Buffer_Size%8;
    }

    inline size_t OffsetBeforeLastCall_Get()  const //No more valid
    {
        return Buffer_Size%8;
    }

private :
    const int8u*    Buffer;
    size_t          Buffer_Size;
    size_t          Buffer_Size_Init;
    int8u           LastByte;
public :
    bool            BufferUnderRun;
};

} //namespace ZenLib
#endif
