// TimeCode - Time code operations with drop frame support
// Copyright (C) 2011-2011 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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
    TimeCode (int8u Hours_, int8u Minutes_, int8u Seconds_, int8u Frames_, int8u FramesPerSecond_, bool DropFrame_, bool MustUseSecondField_=false, bool IsSecondField_=false);

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

public:
    int8u Hours;
    int8u Minutes;
    int8u Seconds;
    int8u Frames;
    int8u FramesPerSecond;
    bool  DropFrame;
    bool  MustUseSecondField;
    bool  IsSecondField;
};

} //NameSpace

#endif

