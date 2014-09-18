/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef MediaInfo_TimeCodeH
#define MediaInfo_TimeCodeH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Unknown
//***************************************************************************

class TimeCode
{
public:
    //constructor/Destructor
    TimeCode ();
    TimeCode (int8u Hours, int8u Minutes, int8u Seconds, int8u Frames, int8u FramesPerSecond, bool DropFrame, bool MustUseSecondField=false, bool IsSecondField=false);
    TimeCode (int64s Frames, int8u FramesPerSecond, bool DropFrame, bool MustUseSecondField=false, bool IsSecondField_=false);

    //Operators
    TimeCode &operator ++()
    {
        PlusOne();
        return *this;
    }
    TimeCode operator ++(int)
    {
        PlusOne();
        return *this;
    }
    TimeCode &operator --()
    {
        MinusOne();
        return *this;
    }
    TimeCode operator --(int)
    {
        MinusOne();
        return *this;
    }
    bool operator== (const TimeCode &tc) const
    {
        return Hours                ==tc.Hours
            && Minutes              ==tc.Minutes
            && Seconds              ==tc.Seconds
            && Frames               ==tc.Frames
            && FramesPerSecond      ==tc.FramesPerSecond
            && DropFrame            ==tc.DropFrame
            && MustUseSecondField   ==tc.MustUseSecondField
            && IsSecondField        ==tc.IsSecondField;
    }
    bool operator!= (const TimeCode &tc) const
    {
        return Hours                !=tc.Hours
            || Minutes              !=tc.Minutes
            || Seconds              !=tc.Seconds
            || Frames               !=tc.Frames
            || FramesPerSecond      !=tc.FramesPerSecond
            || DropFrame            !=tc.DropFrame
            || MustUseSecondField   !=tc.MustUseSecondField
            || IsSecondField        !=tc.IsSecondField;
    }

    //Helpers
    bool IsValid() const
    {
        return FramesPerSecond?true:false;
    }
    void PlusOne();
    void MinusOne();
    string ToString();
    int64s ToFrames();

public:
    int8u Hours;
    int8u Minutes;
    int8u Seconds;
    int8u Frames;
    int8u FramesPerSecond;
    bool  DropFrame;
    bool  MustUseSecondField;
    bool  IsSecondField;
    bool  IsNegative;
};

} //NameSpace

#endif
