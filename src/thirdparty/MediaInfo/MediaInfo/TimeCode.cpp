// TimeCode - Time code operations with drop frame support
// Copyright (C) 2011-2012 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Pre-compilation
#include "MediaInfo/PreComp.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Setup.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/TimeCode.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
TimeCode::TimeCode ()
{
    Hours=(int8u)-1;
    Minutes=(int8u)-1;
    Seconds=(int8u)-1;
    Frames=(int8u)-1;
    FramesPerSecond=0;
    DropFrame=false;
    MustUseSecondField=false;
    IsSecondField=false;
}

//---------------------------------------------------------------------------
TimeCode::TimeCode (int8u Hours_, int8u Minutes_, int8u Seconds_, int8u Frames_, int8u FramesPerSecond_, bool DropFrame_, bool MustUseSecondField_, bool IsSecondField_)
{
    Hours=Hours_;
    Minutes=Minutes_;
    Seconds=Seconds_;
    Frames=Frames_;
    FramesPerSecond=FramesPerSecond_;
    DropFrame=DropFrame_;
    MustUseSecondField=MustUseSecondField_;
    IsSecondField=IsSecondField_;
}

//---------------------------------------------------------------------------
TimeCode::TimeCode (int64u Frames_, int8u FramesPerSecond_, bool DropFrame_, bool MustUseSecondField_, bool IsSecondField_)
{
    int8u Dropped=0;
    if (DropFrame_)
    {
        Dropped=2;
        if (FramesPerSecond_>30)
            Dropped+=2;
        if (FramesPerSecond_>60)
            Dropped+=2;
        if (FramesPerSecond_>90)
            Dropped+=2;
        if (FramesPerSecond_>120)
            Dropped+=2;
    }

    int64u Minutes_Tens = Frames_/(600*FramesPerSecond_-Dropped*9); //Count of 10 minutes
    int64u Minutes_Units = (Frames_-Minutes_Tens*(600*FramesPerSecond_-Dropped*9))/(60*FramesPerSecond_-Dropped);

    Frames_ += 9*Dropped*Minutes_Tens+Dropped*Minutes_Units;
    if (Minutes_Units && ((Frames_/FramesPerSecond_)%60)==0 && (Frames_%FramesPerSecond_)<Dropped) // If Minutes_Tens is not 0 (drop) but count of remaining seconds is 0 and count of remaining frames is less than 2, 1 additional drop was actually counted, removing it
        Frames_-=Dropped;

    Frames  =    Frames_ % FramesPerSecond_;
    Seconds =   (Frames_ / FramesPerSecond_) % 60;
    Minutes =  ((Frames_ / FramesPerSecond_) / 60) % 60;
    Hours   = (((Frames_ / FramesPerSecond_) / 60) / 60) % 24;

    DropFrame=DropFrame_;
    MustUseSecondField=MustUseSecondField_;
    IsSecondField=IsSecondField_;
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
void TimeCode::PlusOne()
{
    if (FramesPerSecond==0)
        return;
    if (MustUseSecondField)
    {
        if (IsSecondField)
        {
            Frames++;
            IsSecondField=false;
        }
        else
            IsSecondField=true;
    }
    else
        Frames++;
    if (Frames>=FramesPerSecond)
    {
        Seconds++;
        Frames=0;
        if (Seconds>=60)
        {
            Seconds=0;
            Minutes++;

            if (DropFrame && Minutes%10)
                Frames=2; //frames 0 and 1 are dropped for every minutes except 00 10 20 30 40 50

            if (Minutes>=60)
            {
                Minutes=0;
                Hours++;
                if (Hours>=24)
                {
                    Hours=0;
                }
            }
        }
    }
}

//---------------------------------------------------------------------------
void TimeCode::MinusOne()
{
    if (FramesPerSecond==0)
        return;
    if (MustUseSecondField && IsSecondField)
        IsSecondField=false;
    else
    {
        if (Frames==0 || (DropFrame && Minutes%10 && Frames<=2))
        {
            Frames=FramesPerSecond;
            if (Seconds==0)
            {
                Seconds=60;
                if (Minutes==0)
                {
                    Minutes=60;
                    if (Hours==0)
                        Hours=24;
                    Hours--;
                }
                Minutes--;
            }
            Seconds--;
        }
        Frames--;

        if (MustUseSecondField)
            IsSecondField=true;
    }
}

//---------------------------------------------------------------------------
string TimeCode::ToString()
{
    string TC;
    TC+=('0'+Hours/10);
    TC+=('0'+Hours%10);
    TC+=':';
    TC+=('0'+Minutes/10);
    TC+=('0'+Minutes%10);
    TC+=':';
    TC+=('0'+Seconds/10);
    TC+=('0'+Seconds%10);
    TC+=DropFrame?';':':';
    TC+=('0'+Frames/10);
    TC+=('0'+Frames%10);

    return TC;
}

//***************************************************************************
//
//***************************************************************************

} //NameSpace
