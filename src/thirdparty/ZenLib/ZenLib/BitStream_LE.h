/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Read a stream bit per bit, Little Endian version (rarely used!!!)
// Can read up to 32 bits at once
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenBitStream_LEH
#define ZenBitStream_LEH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/BitStream.h"
//---------------------------------------------------------------------------

namespace ZenLib
{

class BitStream_LE : public BitStream
{
public:
    BitStream_LE ()                                                             :BitStream() {};
    BitStream_LE (const int8u* Buffer_, size_t Size_)                           :BitStream(Buffer_, Size_) {};

    void Attach(const int8u* Buffer_, size_t Size_)
    {
        endbyte=0;
        endbit=0;
        buffer=Buffer_;
        ptr=Buffer_;
        storage=(long)Size_;
    };

    int32u Get (size_t HowMany)
    {
        ptr_BeforeLastCall=ptr;

        long ret;
        static const int32u Mask[33]={
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
        unsigned long m=Mask[HowMany];

        HowMany+=endbit;

        if(endbyte+4>=storage){
            ret=-1L;
            if(endbyte*8+(long)HowMany>storage*8){
                Attach(NULL, 0);
                goto overflow;
            }
        }

        ret=ptr[0]>>endbit;
        if(HowMany>8){
        ret|=ptr[1]<<(8-endbit);
        if(HowMany>16){
          ret|=ptr[2]<<(16-endbit);
          if(HowMany>24){
        ret|=ptr[3]<<(24-endbit);
        if(HowMany>32 && endbit){
          ret|=ptr[4]<<(32-endbit);
        }
          }
        }
        }
        ret&=m;

        ptr+=HowMany/8;
        endbyte+=(long)HowMany/8;
        endbit=(long)HowMany&7;

        overflow:

        return(ret);
    };

    void Skip(size_t bits)
    {
        Get(bits);
    }

    int32u Remain () //How many bits remain?
    {
        return storage*8-(endbyte*8+endbit);
    };

    void Byte_Align()
    {
    };

    size_t Offset_Get()
    {
        return ptr-buffer;
    };

    size_t BitOffset_Get()
    {
        return endbit;
    };

    size_t OffsetBeforeLastCall_Get()
    {
        return ptr_BeforeLastCall-buffer;
    };

private :
    long endbyte;
    int  endbit;

    const unsigned char *buffer;
    const unsigned char *ptr;
    const unsigned char *ptr_BeforeLastCall;
    long storage;
};

} //namespace ZenLib
#endif
