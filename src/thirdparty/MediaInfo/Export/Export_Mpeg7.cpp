// File__Analyze - Base for analyze files
// Copyright (C) 2009-2009 Jerome Martinez, Zen@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Export/Export_Mpeg7.h"
#include "MediaInfo/File__Analyse_Automatic.h"
#include <ctime>
#ifdef SS
   #undef SS //Solaris defines this somewhere
#endif
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
extern MediaInfo_Config Config;
//---------------------------------------------------------------------------

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const Char* Mpeg7_Type(MediaInfo_Internal &MI) //TO ADAPT
{
    if (MI.Count_Get(Stream_Image))
    {
        if (MI.Count_Get(Stream_Video) || MI.Count_Get(Stream_Audio))
            return _T("Multimedia");
        else
            return _T("Image");
    }
    else if (MI.Count_Get(Stream_Video))
    {
        if (MI.Count_Get(Stream_Audio))
            return _T("AudioVisual");
        else
            return _T("Video");
    }
    else if (MI.Count_Get(Stream_Audio))
        return _T("Audio");

    //Not known
    const Ztring &Format=MI.Get(Stream_General, 0, General_Format);
    if (Format==_T("AVI") || Format==_T("DV") || Format==_T("MPEG-4") || Format==_T("MPEG-PS") || Format==_T("MPEG-TS") || Format==_T("QuickTime") || Format==_T("Windows Media"))
        return _T("Video");
    if (Format==_T("MPEG Audio") || Format==_T("Wave"))
        return _T("Audio");
    if (Format==_T("BMP") || Format==_T("GIF") || Format==_T("JPEG") || Format==_T("JPEG 2000") || Format==_T("M-JPEG 2000") || Format==_T("PNG") || Format==_T("TIFF"))
        return _T("Image");
    return _T("Multimedia");
}

//---------------------------------------------------------------------------
int32u Mpeg7_ContentCS_termID(MediaInfo_Internal &MI)
{
    if (MI.Count_Get(Stream_Image))
    {
        if (MI.Count_Get(Stream_Video) || MI.Count_Get(Stream_Audio))
            return 20000;
        else
            return 40100;
    }
    else if (MI.Count_Get(Stream_Video))
    {
        if (MI.Count_Get(Stream_Audio))
            return 20000;
        else
            return 40200;
    }
    else if (MI.Count_Get(Stream_Audio))
        return 10000;

    //Not known
    const Ztring &Format=MI.Get(Stream_General, 0, General_Format);
    if (Format==_T("AVI") || Format==_T("DV") || Format==_T("MPEG-4") || Format==_T("MPEG-PS") || Format==_T("MPEG-TS") || Format==_T("QuickTime") || Format==_T("Windows Media"))
        return 40200;
    if (Format==_T("MPEG Audio") || Format==_T("Wave"))
        return 10000;
    if (Format==_T("BMP") || Format==_T("GIF") || Format==_T("JPEG") || Format==_T("JPEG 2000") || Format==_T("M-JPEG 2000") || Format==_T("PNG") || Format==_T("TIFF"))
        return 40100;
    return 500000;
}

Ztring Mpeg7_ContentCS_Name(int32u termID, MediaInfo_Internal &) //xxyyzz: xx=main number, yy=sub-number, zz=sub-sub-number
{
    switch (termID/10000)
    {
        case  1 : return _T("Audio");
        case  2 : return _T("Audiovisual");
        case  3 : return _T("Scene");
        case  4 :   switch ((termID%10000)/100)
                    {
                        case 1 : return _T("Image");
                        case 2 : return _T("Video");
                        case 3 : return _T("Graphics");
                    }
        case 50 : return Ztring(); //Unknown
        default : return Ztring();
    }
}

//---------------------------------------------------------------------------
int32u Mpeg7_FileFormatCS_termID_MediaInfo(MediaInfo_Internal &MI)
{
    const Ztring &Format=MI.Get(Stream_General, 0, General_Format);

    if (Format==_T("MPEG Audio"))
    {
        if (MI.Get(Stream_Audio, 0, Audio_Format_Profile).find(_T("2"))!=string::npos)
            return 500000; //mp2
        if (MI.Get(Stream_Audio, 0, Audio_Format_Profile).find(_T("1"))!=string::npos)
            return 510000; //mp1
        return 0;
    }
    if (Format==_T("Wave") && MI.Get(Stream_Audio, 0, Audio_Format_Profile)==_T("RF64"))
        return 520000; //Wav (RF64)
    if (Format==_T("Wave64"))
        return 530000;
    return 0;
}

//---------------------------------------------------------------------------
int32u Mpeg7_FileFormatCS_termID(MediaInfo_Internal &MI)
{
    const Ztring &Format=MI.Get(Stream_General, 0, General_Format);

    if (Format==_T("AVI"))
        return 70000;
    if (Format==_T("BMP"))
        return 110000;
    if (Format==_T("GIF"))
        return 120000;
    if (Format==_T("DV"))
        return 60000;
    if (Format==_T("JPEG"))
        return 10000;
    if (Format==_T("JPEG 2000") || Format==_T("M-JPEG 2000"))
        return 20000;
    if (Format==_T("MPEG Audio"))
        return (MI.Get(Stream_Audio, 0, Audio_Format_Profile).find(_T("3"))!=string::npos)?40000:0;
    if (Format==_T("MPEG-4"))
        return 50000;
    if (Format==_T("MPEG-PS"))
        return 30100;
    if (Format==_T("MPEG-TS"))
        return 30200;
    if (Format==_T("PNG"))
        return 150000;
    if (Format==_T("QuickTime"))
        return 160000;
    if (Format==_T("TIFF"))
        return 180000;
    if (Format==_T("Wave"))
    {
        if (!MI.Get(Stream_Audio, 0, Audio_Format_Profile).empty())
            return 00000;
        else
            return 90000;
    }
    if (Format==_T("Windows Media"))
        return 190000;
    if (Format==_T("ZIP"))
        return 100000;

    //Out of specs
    return Mpeg7_FileFormatCS_termID_MediaInfo(MI);
}

Ztring Mpeg7_FileFormatCS_Name(int32u termID, MediaInfo_Internal &MI) //xxyyzz: xx=main number, yy=sub-number, zz=sub-sub-number
{
    switch (termID/10000)
    {
        case  1 : return _T("jpeg");
        case  2 : return _T("JPEG 2000");
        case  3 :   switch ((termID%10000)/100)
                    {
                        case 1 : return _T("mpeg-ps");
                        case 2 : return _T("mpeg-ts");
                        default: return _T("mpeg");
                    }
        case  4 : return _T("mp3");
        case  5 : return _T("mp4");
        case  6 : return _T("dv");
        case  7 : return _T("avi");
        case  8 : return _T("bdf");
        case  9 : return _T("wav");
        case 10 : return _T("zip");
        case 11 : return _T("bmp");
        case 12 : return _T("gif");
        case 13 : return _T("photocd");
        case 14 : return _T("ppm");
        case 15 : return _T("png");
        case 16 : return _T("quicktime");
        case 17 : return _T("spiff");
        case 18 : return _T("tiff");
        case 19 : return _T("asf");
        case 20 : return _T("iff");
        case 21 : return _T("miff");
        case 22 : return _T("pcx");
        //Out of specs --> MediaInfo CS
        case 50 : return _T("mp1");
        case 51 : return _T("mp2");
        case 52 : return _T("wav-rf64)");
        case 53 : return _T("wave64");
        default : return MI.Get(Stream_General, 0, General_Format);
    }
}

//---------------------------------------------------------------------------
int32u Mpeg7_VisualCodingFormatCS_termID(MediaInfo_Internal &MI, size_t StreamPos)
{
    const Ztring &Format=MI.Get(Stream_Video, StreamPos, Video_Format);
    const Ztring &Version=MI.Get(Stream_Video, StreamPos, Video_Format_Version);
    const Ztring &Profile=MI.Get(Stream_Video, StreamPos, Video_Format_Profile);

    if (Format==_T("MPEG Video"))
    {
        if (Version.find(_T("1"))!=string::npos)
            return 10000;
        if (Version.find(_T("2"))!=string::npos)
        {
            if (Profile.find(_T("Simple"))!=string::npos)
            {
                if (Profile.find(_T("Main"))!=string::npos)
                    return 20101;
                return 20100;
            }
            if (Profile.find(_T("Main@"))!=string::npos)
            {
                if (Profile.find(_T("Low"))!=string::npos)
                    return 20201;
                if (Profile.find(_T("@Main"))!=string::npos)
                    return 20202;
                if (Profile.find(_T("High 1440"))!=string::npos)
                    return 20203;
                if (Profile.find(_T("High"))!=string::npos)
                    return 20204;
                return 20200;
            }
            if (Profile.find(_T("SNR Scalable"))!=string::npos)
            {
                if (Profile.find(_T("Low"))!=string::npos)
                    return 20301;
                if (Profile.find(_T("Main"))!=string::npos)
                    return 20302;
                return 20300;
            }
            if (Profile.find(_T("Spatial Sclable"))!=string::npos)
            {
                if (Profile.find(_T("Main"))!=string::npos)
                    return 20401;
                if (Profile.find(_T("High 1440"))!=string::npos)
                    return 20402;
                if (Profile.find(_T("High"))!=string::npos)
                    return 20403;
                return 20400;
            }
            if (Profile.find(_T("High"))!=string::npos)
            {
                if (Profile.find(_T("Main"))!=string::npos)
                    return 20501;
                if (Profile.find(_T("High 1440"))!=string::npos)
                    return 20502;
                if (Profile.find(_T("High"))!=string::npos)
                    return 20503;
                return 20500;
            }
        }
    }

    return 0;
}

Ztring Mpeg7_VisualCodingFormatCS_Name(int32u termID, MediaInfo_Internal &MI, size_t StreamPos) //xxyyzz: xx=main number, yy=sub-number, zz=sub-sub-number
{
    switch (termID/10000)
    {
        case 1 : return _T("MPEG-1 Video");
        case 2 :    switch ((termID%10000)/100)
                    {
                        case 1 :    switch (termID%100)
                                    {
                                        case 1 : return _T("MPEG-2 Video Simple Profile @ Main Level");
                                        default: return _T("MPEG-2 Video Simple Profile");
                                    }
                        case 2 :    switch (termID%100)
                                    {
                                        case 1 : return _T("MPEG-2 Video Main Profile @ Low Level");
                                        case 2 : return _T("MPEG-2 Video Main Profile @ Main Level");
                                        case 3 : return _T("MPEG-2 Video Main Profile @ High 1440 Level");
                                        case 4 : return _T("MPEG-2 Video Main Profile @ High Level");
                                        default: return _T("MPEG-2 Video Main Profile");
                                    }
                        case 3 :    switch (termID%100)
                                    {
                                        case 1 : return _T("MPEG-2 Video SNR Scalable Profile @ Low Level");
                                        case 2 : return _T("MPEG-2 Video SNR Scalable Profile @ Main Level");
                                        default: return _T("MPEG-2 Video SNR Scalable Profile");
                                    }
                        case 4 :    switch (termID%100)
                                    {
                                        case 1 : return _T("MPEG-2 Video Spatial Scalable Profile @ Main Level");
                                        case 2 : return _T("MPEG-2 Video Spatial Scalable Profile @ High 1440 Level");
                                        case 3 : return _T("MPEG-2 Video Spatial Scalable Profile @ High Level");
                                        default: return _T("MPEG-2 Video Spatial Scalable Profile");
                                    }
                        case 5 :    switch (termID%100)
                                    {
                                        case 1 : return _T("MPEG-2 Video High Profile @ Main Level");
                                        case 2 : return _T("MPEG-2 Video High Profile @ High 1440 Level");
                                        case 3 : return _T("MPEG-2 Video High Profile @ High Level");
                                        default: return _T("MPEG-2 Video High Profile");
                                    }
                        case 6 :    switch (termID%100)
                                    {
                                        case 1 : return _T("MPEG-2 Video Multiview Profile @ Main Level");
                                        default: return _T("MPEG-2 Video Multiview Profile");
                                    }
                        case 7 :    switch (termID%100)
                                    {
                                        case 1 : return _T("MPEG-2 Video 4:2:2 Profile @ Main Level");
                                        default: return _T("MPEG-2 Video 4:2:2 Profile");
                                    }
                        default: return _T("MPEG-2 Video");
                    }
        default: return MI.Get(Stream_Video, StreamPos, Video_Format);
    }
}

//---------------------------------------------------------------------------
Ztring Mpeg7_Visual_colorDomain(MediaInfo_Internal &MI, size_t StreamPos)
{
    const Ztring &Colorimetry=MI.Get(Stream_Video, StreamPos, Video_Colorimetry);
    if (Colorimetry.find(_T("4:"))!=string::npos)
        return _T(" colorDomain=\"color\"");
    if (Colorimetry==_T("Gray"))
        return _T(" colorDomain=\"graylevel\"");
    return _T("");
}

//---------------------------------------------------------------------------
int32u Mpeg7_SystemCS_termID(MediaInfo_Internal &MI, size_t StreamPos)
{
    if (MI.Get(Stream_Video, StreamPos, Video_Standard)==_T("PAL"))
        return 10000;
    if (MI.Get(Stream_Video, StreamPos, Video_Standard)==_T("SECAM"))
        return 20000;
    if (MI.Get(Stream_Video, StreamPos, Video_Standard)==_T("NTSC"))
        return 30000;
    return 0;
}

Ztring Mpeg7_SystemCS_Name(int32u termID) //xxyyzz: xx=main number, yy=sub-number, zz=sub-sub-number
{
    switch (termID/10000)
    {
        case  1 : return _T("PAL");
        case  2 : return _T("SECAM");
        case  3 : return _T("NTSC");
        default : return Ztring();
    }
}

//---------------------------------------------------------------------------
int32u Mpeg7_AudioCodingFormatCS_termID(MediaInfo_Internal &MI, size_t StreamPos)
{
    const Ztring &Format=MI.Get(Stream_Audio, StreamPos, Audio_Format);
    const Ztring &Version=MI.Get(Stream_Audio, StreamPos, Audio_Format_Version);
    const Ztring &Profile=MI.Get(Stream_Audio, StreamPos, Audio_Format_Profile);

    if (Format==_T("AC-3"))
        return 10000;
    if (Format==_T("DTS"))
        return 20000;
    if (Format==_T("MPEG Audio"))
    {
        if (Version.find(_T("1"))!=string::npos)
        {
            if (Profile.find(_T("1"))!=string::npos)
                return 30100;
            if (Profile.find(_T("2"))!=string::npos)
                return 30200;
            if (Profile.find(_T("3"))!=string::npos)
                return 30300;
            return 30000;
        }
        if (Version.find(_T("2"))!=string::npos)
        {
            if (Profile.find(_T("1"))!=string::npos)
                return 40100;
            if (Profile.find(_T("2"))!=string::npos)
                return 40200;
            if (Profile.find(_T("3"))!=string::npos)
                return 40300;
            return 40000;
        }
        return 0;
    }
    if (Format==_T("PCM"))
        return 80000;

    return 0;
}

Ztring Mpeg7_AudioCodingFormatCS_Name(int32u termID, MediaInfo_Internal &MI, size_t StreamPos) //xxyyzz: xx=main number, yy=sub-number, zz=sub-sub-number
{
    switch (termID/10000)
    {
        case 1 : return _T("AC3");
        case 2 : return _T("DTS");
        case 3 :    switch ((termID%10000)/100)
                    {
                        case 1 : return _T("MPEG-1 Audio Layer I");
                        case 2 : return _T("MPEG-1 Audio Layer II");
                        case 3 : return _T("MPEG-1 Audio Layer III");
                        default: return _T("MPEG-1 Audio");
                    }
        case 4 :    switch ((termID%10000)/100)
                    {
                        case 1 :    switch (termID%100)
                                    {
                                        case 1 : return _T("MPEG-2 Audio Low Sampling Rate Layer I");
                                        case 2 : return _T("MPEG-2 Audio Low Sampling Rate Layer II");
                                        case 3 : return _T("MPEG-2 Audio Low Sampling Rate Layer III");
                                        default: return _T("MPEG-2 Audio Low Sampling Rate");
                                    }
                        case 2 :    switch (termID%100)
                                    {
                                        case 1 : return _T("MPEG-2 Backward Compatible Multi-Channel Layer I");
                                        case 2 : return _T("MPEG-2 Backward Compatible Multi-Channel Layer II");
                                        case 3 : return _T("MPEG-2 Backward Compatible Multi-Channel Layer III");
                                        default: return _T("MPEG-2 Backward Compatible Multi-Channel");
                                    }
                        default: return _T("MPEG-2 Audio");
                    }
        case 8 : return _T("Linear PCM");
        default: return MI.Get(Stream_Audio, StreamPos, Video_Format);
    }
}

//---------------------------------------------------------------------------
int32u Mpeg7_AudioPresentationCS_termID(MediaInfo_Internal &MI, size_t StreamPos)
{
    const Ztring &Channels=MI.Get(Stream_Audio, StreamPos, Audio_Channel_s_);
    const Ztring &ChannelPositions2=MI.Get(Stream_Audio, StreamPos, Audio_ChannelPositions_String2);
    if (Channels==_T("6") && ChannelPositions2==_T("3/2.1"))
        return 50000;
    if (Channels==_T("8") && ChannelPositions2==_T("3/2/2.1"))
        return 60000;
    if (Channels==_T("2"))
        return 30000;
    if (Channels==_T("1"))
        return 20000;
    return 0;
}

Ztring Mpeg7_AudioPresentationCS_Name(int32u termID, MediaInfo_Internal &, size_t)
{
    switch (termID/10000)
    {
        case 2 : return _T("mono");
        case 3 : return _T("stereo");
        case 5 : return _T("Home theater 5.1");
        case 6 : return _T("Movie theater");
        default: return Ztring();
    }
}

//---------------------------------------------------------------------------
Ztring Mpeg7_AudioEmphasis(MediaInfo_Internal &MI, size_t StreamPos)
{
    const Ztring &Emphasis=MI.Get(Stream_Audio, StreamPos, Audio_Format_Settings_Emphasis);
    if (Emphasis==_T("50/15ms"))
        return _T("50over15Microseconds");
    if (Emphasis==_T("CCITT"))
        return _T("ccittJ17");
    if (Emphasis==_T("Reserved"))
        return _T("reserved");
    return _T("none");
}

//---------------------------------------------------------------------------
Ztring Mpeg7_MediaTimePoint(MediaInfo_Internal &MI)
{
    if (MI.Count_Get(Stream_Video)==1 && MI.Get(Stream_General, 0, General_Format)==_T("MPEG-PS"))
    {
        int64u Delay=(int64u)(MI.Get(Stream_Video, 0, Video_Delay).To_float64()*90);
        int64u Rate=90000;
        int64u DD=Delay/(24*60*60*Rate);
        Delay=Delay%(24*60*60*Rate);
        int64u HH=Delay/(60*60*Rate);
        Delay=Delay%(60*60*Rate);
        int64u MM=Delay/(60*Rate);
        Delay=Delay%(60*Rate);
        int64u SS=Delay/Rate;
        Delay=Delay%Rate;
        Ztring ToReturn;
        if (DD)
            ToReturn+=Ztring::ToZtring(DD);
        ToReturn+=_T('T');
        ToReturn+=(HH<10?_T("0"):_T(""))+Ztring::ToZtring(HH)+_T(':');
        ToReturn+=(MM<10?_T("0"):_T(""))+Ztring::ToZtring(MM)+_T(':');
        ToReturn+=(SS<10?_T("0"):_T(""))+Ztring::ToZtring(SS)+_T(':');
        ToReturn+=Ztring::ToZtring(Delay)+_T('F');
        ToReturn+=Ztring::ToZtring(Rate);
        return ToReturn;
    }

    //Default: In miliseconds
    int64u Milliseconds=MI.Get(Stream_Video, 0, Video_Delay).To_int64u();
    int64u DD=Milliseconds/(24*60*60*1000);
    Milliseconds=Milliseconds%(24*60*60*1000);
    int64u HH=Milliseconds/(60*60*1000);
    Milliseconds=Milliseconds%(60*60*1000);
    int64u MM=Milliseconds/(60*1000);
    Milliseconds=Milliseconds%(60*1000);
    int64u SS=Milliseconds/1000;
    int64u NN=Milliseconds%1000;
    int64u FF=1000;
    Ztring ToReturn;
    if (DD)
        ToReturn+=Ztring::ToZtring(DD);
    ToReturn+=_T('T');
    ToReturn+=(HH<10?_T("0"):_T(""))+Ztring::ToZtring(HH)+_T(':');
    ToReturn+=(MM<10?_T("0"):_T(""))+Ztring::ToZtring(MM)+_T(':');
    ToReturn+=(SS<10?_T("0"):_T(""))+Ztring::ToZtring(SS)+_T(':');
    ToReturn+=Ztring::ToZtring(NN)+_T('F');
    ToReturn+=Ztring::ToZtring(FF);
    return ToReturn;
}

//---------------------------------------------------------------------------
Ztring Mpeg7_MediaDuration(MediaInfo_Internal &MI)
{
    if (MI.Count_Get(Stream_Video)==1)
    {
        int64u FrameCount=MI.Get(Stream_Video, 0, Video_FrameCount).To_int64u();
        int64u FrameRate=MI.Get(Stream_Video, 0, Video_FrameRate).To_int64u();
        if (FrameRate==0)
            return Ztring();
        int64u DD=FrameCount/(24*60*60*FrameRate);
        FrameCount=FrameCount%(24*60*60*FrameRate);
        int64u HH=FrameCount/(60*60*FrameRate);
        FrameCount=FrameCount%(60*60*FrameRate);
        int64u MM=FrameCount/(60*FrameRate);
        FrameCount=FrameCount%(60*FrameRate);
        int64u SS=FrameCount/FrameRate;
        FrameCount=FrameCount%FrameRate;
        Ztring ToReturn;
        ToReturn+=_T('P');
        if (DD)
            ToReturn+=Ztring::ToZtring(DD)+_T('D');
        ToReturn+=_T('T');
        ToReturn+=Ztring::ToZtring(HH)+_T('H');
        ToReturn+=Ztring::ToZtring(MM)+_T('M');
        ToReturn+=Ztring::ToZtring(SS)+_T('S');
        ToReturn+=Ztring::ToZtring(FrameCount)+_T('N');
        ToReturn+=Ztring::ToZtring(FrameRate)+_T('F');
        return ToReturn;
    }

    if (MI.Count_Get(Stream_Audio)==1)
    {
        int64u SamplingCount=MI.Get(Stream_Audio, 0, Audio_SamplingCount).To_int64u();
        int64u SamplingRate=MI.Get(Stream_Audio, 0, Audio_SamplingRate).To_int64u();
        if (SamplingRate==0)
            return Ztring();
        int64u DD=SamplingCount/(24*60*60*SamplingRate);
        SamplingCount=SamplingCount%(24*60*60*SamplingRate);
        int64u HH=SamplingCount/(60*60*SamplingRate);
        SamplingCount=SamplingCount%(60*60*SamplingRate);
        int64u MM=SamplingCount/(60*SamplingRate);
        SamplingCount=SamplingCount%(60*SamplingRate);
        int64u SS=SamplingCount/SamplingRate;
        SamplingCount=SamplingCount%SamplingRate;
        Ztring ToReturn;
        ToReturn+=_T('P');
        if (DD)
            ToReturn+=Ztring::ToZtring(DD)+_T('D');
        ToReturn+=_T('T');
        ToReturn+=Ztring::ToZtring(HH)+_T('H');
        ToReturn+=Ztring::ToZtring(MM)+_T('M');
        ToReturn+=Ztring::ToZtring(SS)+_T('S');
        ToReturn+=Ztring::ToZtring(SamplingCount)+_T('N');
        ToReturn+=Ztring::ToZtring(SamplingRate)+_T('F');
        return ToReturn;
    }

    //Default: In miliseconds
    int64u Milliseconds=MI.Get(Stream_General, 0, General_Duration).To_int64u();
    int64u DD=Milliseconds/(24*60*60*1000);
    Milliseconds=Milliseconds%(24*60*60*1000);
    int64u HH=Milliseconds/(60*60*1000);
    Milliseconds=Milliseconds%(60*60*1000);
    int64u MM=Milliseconds/(60*1000);
    Milliseconds=Milliseconds%(60*1000);
    int64u SS=Milliseconds/1000;
    int64u NN=Milliseconds%1000;
    int64u FF=1000;
    Ztring ToReturn;
    ToReturn+=_T('P');
    if (DD)
        ToReturn+=Ztring::ToZtring(DD)+_T('D');
    ToReturn+=_T('T');
    ToReturn+=Ztring::ToZtring(HH)+_T('H');
    ToReturn+=Ztring::ToZtring(MM)+_T('M');
    ToReturn+=Ztring::ToZtring(SS)+_T('S');
    ToReturn+=Ztring::ToZtring(NN)+_T('N');
    ToReturn+=Ztring::ToZtring(FF)+_T('F');
    return ToReturn;
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
Export_Mpeg7::Export_Mpeg7 ()
{
}

//---------------------------------------------------------------------------
Export_Mpeg7::~Export_Mpeg7 ()
{
}

//***************************************************************************
// Input
//***************************************************************************

//---------------------------------------------------------------------------
Ztring Mpeg7_Transform_Visual(Ztring &ToReturn, MediaInfo_Internal &MI, size_t StreamPos)
{
    ToReturn+=_T("\t\t\t\t\t\t\t<mpeg7:VisualCoding>\n");

    //Format
    ToReturn+=_T("\t\t\t\t\t\t\t\t<mpeg7:Format");
    int32u VisualCodingFormatCS_termID=Mpeg7_VisualCodingFormatCS_termID(MI, StreamPos);
    if (VisualCodingFormatCS_termID)
    {
        ToReturn+=_T(" href=\"urn:mpeg:mpeg7:cs:VisualCodingFormatCS:2001:");
        ToReturn+=Ztring::ToZtring(VisualCodingFormatCS_termID/10000);
        ToReturn+=_T("\"");
    }
    ToReturn+=Mpeg7_Visual_colorDomain(MI, StreamPos); //Algo puts empty string if not known
    ToReturn+=_T(">\n");
    ToReturn+=_T("\t\t\t\t\t\t\t\t\t<mpeg7:Name xml:lang=\"en\">"); ToReturn+=Mpeg7_VisualCodingFormatCS_Name((VisualCodingFormatCS_termID/10000)*10000, MI, StreamPos); ToReturn+=_T("</mpeg7:Name>\n");
    if (VisualCodingFormatCS_termID%10000)
    {
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t<mpeg7:Term termID=\"");
        ToReturn+=Ztring::ToZtring(VisualCodingFormatCS_termID/10000);
        ToReturn+=_T(".");
        ToReturn+=Ztring::ToZtring((VisualCodingFormatCS_termID%10000)/100);
        ToReturn+=_T("\">\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t<mpeg7:Name xml:lang=\"en\">"); ToReturn+=Mpeg7_VisualCodingFormatCS_Name((VisualCodingFormatCS_termID/100)*100, MI, StreamPos); ToReturn+=_T("</mpeg7:Name>\n");
        if (VisualCodingFormatCS_termID%100)
        {
            ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t<mpeg7:Term termID=\"");
            ToReturn+=Ztring::ToZtring(VisualCodingFormatCS_termID/10000);
            ToReturn+=_T(".");
            ToReturn+=Ztring::ToZtring((VisualCodingFormatCS_termID%10000)/100);
            ToReturn+=_T(".");
            ToReturn+=Ztring::ToZtring(VisualCodingFormatCS_termID%100);
            ToReturn+=_T("\">\n");
            ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t\t<mpeg7:Name xml:lang=\"en\">"); ToReturn+=Mpeg7_VisualCodingFormatCS_Name(VisualCodingFormatCS_termID, MI, StreamPos); ToReturn+=_T("</mpeg7:Name>\n");
            ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t</mpeg7:Term>\n");
        }
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t</mpeg7:Term>\n");
    }
    ToReturn+=_T("\t\t\t\t\t\t\t\t</mpeg7:Format>\n");

    //Pixel
    if (!MI.Get(Stream_Video, 0, Video_PixelAspectRatio).empty()
     || !MI.Get(Stream_Video, 0, Video_Resolution).empty())
    {
        ToReturn+=_T("\t\t\t\t\t\t\t\t<mpeg7:Pixel");
        if (!MI.Get(Stream_Video, 0, Video_PixelAspectRatio).empty())
        {
            ToReturn+=_T(" aspectRatio=\"");
            ToReturn+=MI.Get(Stream_Video, 0, Video_PixelAspectRatio);
            ToReturn+=_T("\"");
        }
        if (!MI.Get(Stream_Video, 0, Video_Resolution).empty())
        {
            ToReturn+=_T(" bitsPer=\"");
            ToReturn+=MI.Get(Stream_Video, 0, Video_Resolution);
            ToReturn+=_T("\"");
        }
        ToReturn+=_T("/>\n");
    }

    //Frame
    ToReturn+=_T("\t\t\t\t\t\t\t\t<mpeg7:Frame aspectRatio=\"")+MI.Get(Stream_Video, 0, Video_DisplayAspectRatio); ToReturn+=_T("\" height=\""); ToReturn+=MI.Get(Stream_Video, 0, Video_Height); ToReturn+=_T("\" width=\""); ToReturn+=MI.Get(Stream_Video, 0, Video_Width); ToReturn+=_T("\" rate=\""); ToReturn+=MI.Get(Stream_Video, 0, Video_FrameRate); ToReturn+=_T("\" structure=\""); ToReturn+=MI.Get(Stream_Video, 0, Video_ScanType).MakeLowerCase(); ToReturn+=_T("\"/>\n");

    //Colorimetry
    if (MI.Get(Stream_Video, StreamPos, Video_Colorimetry).find(_T("4:2:0"))!=string::npos)
    {
        ToReturn+=_T("\t\t\t\t\t\t\t\t<mpeg7:ColorSampling> <!-- YUV 4:2:0 Interlaced -->\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t<mpeg7:Lattice height=\"720\" width=\"486\"/>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t<mpeg7:Field temporalOrder=\"0\" positionalOrder=\"0\">\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t<mpeg7:Component>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t\t<mpeg7:Name>Luminance</mpeg7:Name>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t\t<mpeg7:Offset horizontal=\"0.0\" vertical=\"0.0\"/>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t\t<mpeg7:Period horizontal=\"1.0\" vertical=\"2.0\"/>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t</mpeg7:Component>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t<mpeg7:Component>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t\t<mpeg7:Name>ChrominanceBlueDifference</mpeg7:Name>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t\t<mpeg7:Offset horizontal=\"0.0\" vertical=\"0.5\"/>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t\t<mpeg7:Period horizontal=\"2.0\" vertical=\"4.0\"/>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t</mpeg7:Component>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t<mpeg7:Component>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t\t<mpeg7:Name>ChrominanceRedDifference</mpeg7:Name>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t\t<mpeg7:Offset horizontal=\"0.0\" vertical=\"0.5\"/>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t\t<mpeg7:Period horizontal=\"2.0\" vertical=\"4.0\"/>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t</mpeg7:Component>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t</mpeg7:Field>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t<mpeg7:Field temporalOrder=\"1\" positionalOrder=\"1\">\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t<mpeg7:Component>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t\t<mpeg7:Name>Luminance</mpeg7:Name>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t\t<mpeg7:Offset horizontal=\"0.0\" vertical=\"1.0\"/>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t\t<mpeg7:Period horizontal=\"1.0\" vertical=\"2.0\"/>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t</mpeg7:Component>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t<mpeg7:Component>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t\t<mpeg7:Name>ChrominanceBlueDifference</mpeg7:Name>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t\t<mpeg7:Offset horizontal=\"0.0\" vertical=\"2.5\"/>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t\t<mpeg7:Period horizontal=\"2.0\" vertical=\"4.0\"/>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t</mpeg7:Component>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t<mpeg7:Component>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t\t<mpeg7:Name>ChrominanceRedDifference</mpeg7:Name>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t\t<mpeg7:Offset horizontal=\"0.0\" vertical=\"2.5\"/>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t\t<mpeg7:Period horizontal=\"4.0\" vertical=\"2.0\"/>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t</mpeg7:Component>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t</mpeg7:Field>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t</mpeg7:ColorSampling>\n");
    }

    ToReturn+=_T("\t\t\t\t\t\t\t</mpeg7:VisualCoding>\n");

    return ToReturn;
}

//---------------------------------------------------------------------------
Ztring Mpeg7_Transform_Audio(Ztring &ToReturn, MediaInfo_Internal &MI, size_t StreamPos)
{
    ToReturn+=_T("\t\t\t\t\t\t\t<mpeg7:AudioCoding>\n");

    //Format
    ToReturn+=_T("\t\t\t\t\t\t\t\t<mpeg7:Format");
    int32u AudioCodingFormatCS_termID=Mpeg7_AudioCodingFormatCS_termID(MI, StreamPos);
    if (AudioCodingFormatCS_termID)
    {
        ToReturn+=_T(" href=\"urn:mpeg:mpeg7:cs:AudioCodingFormatCS:2001:");
        ToReturn+=Ztring::ToZtring(AudioCodingFormatCS_termID/10000);
        ToReturn+=_T("\"");
    }
    ToReturn+=_T(">\n");
    ToReturn+=_T("\t\t\t\t\t\t\t\t\t<mpeg7:Name xml:lang=\"en\">"); ToReturn+=Mpeg7_AudioCodingFormatCS_Name((AudioCodingFormatCS_termID/10000)*10000, MI, StreamPos); ToReturn+=_T("</mpeg7:Name>\n");
    if (AudioCodingFormatCS_termID%10000)
    {
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t<mpeg7:Term termID=\"");
        ToReturn+=Ztring::ToZtring(AudioCodingFormatCS_termID/10000);
        ToReturn+=_T(".");
        ToReturn+=Ztring::ToZtring((AudioCodingFormatCS_termID%10000)/100);
        ToReturn+=_T("\">\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t<mpeg7:Name xml:lang=\"en\">"); ToReturn+=Mpeg7_AudioCodingFormatCS_Name((AudioCodingFormatCS_termID/100)*100, MI, StreamPos); ToReturn+=_T("</mpeg7:Name>\n");
        if (AudioCodingFormatCS_termID%100)
        {
            ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t<mpeg7:Term termID=\"");
            ToReturn+=Ztring::ToZtring(AudioCodingFormatCS_termID/10000);
            ToReturn+=_T(".");
            ToReturn+=Ztring::ToZtring((AudioCodingFormatCS_termID%10000)/100);
            ToReturn+=_T(".");
            ToReturn+=Ztring::ToZtring(AudioCodingFormatCS_termID%100);
            ToReturn+=_T("\">\n");
            ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t\t<mpeg7:Name xml:lang=\"en\">"); ToReturn+=Mpeg7_AudioCodingFormatCS_Name(AudioCodingFormatCS_termID, MI, StreamPos); ToReturn+=_T("</mpeg7:Name>\n");
            ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t</mpeg7:Term>\n");
        }
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t</mpeg7:Term>\n");
    }
    ToReturn+=_T("\t\t\t\t\t\t\t\t</mpeg7:Format>\n");

    //AudioChannels
    ToReturn+=_T("\t\t\t\t\t\t\t\t<mpeg7:AudioChannels>"); ToReturn+=MI.Get(Stream_Audio, StreamPos, Audio_Channel_s_); ToReturn+=_T("</mpeg7:AudioChannels>\n");

    //Sample
    ToReturn+=_T("\t\t\t\t\t\t\t\t<mpeg7:Sample rate=\""); ToReturn+=MI.Get(Stream_Audio, StreamPos, Audio_SamplingRate); ToReturn+=_T("\""); if (!MI.Get(Stream_Audio, StreamPos, Audio_Resolution).empty()) {ToReturn+=_T(" bitsPer=\""); ToReturn+=MI.Get(Stream_Audio, StreamPos, Audio_Resolution); ToReturn+=_T("\"");} ToReturn+=_T("/>\n");

    //Emphasis
    if (MI.Get(Stream_Audio, StreamPos, Audio_Format)==_T("MPEG Audio"))
        ToReturn+=_T("\t\t\t\t\t\t\t\t<mpeg7:Emphasis>")+Mpeg7_AudioEmphasis(MI, StreamPos)+_T("</mpeg7:Emphasis>\n");

    //Presentation
    int32u AudioPresentationCS_termID=Mpeg7_AudioPresentationCS_termID(MI, StreamPos);
    if (AudioPresentationCS_termID)
    {
        ToReturn+=_T("\t\t\t\t\t\t\t\t<mpeg7:Presentation href=\"urn:mpeg:mpeg7:cs:AudioPresentationCS:2001:");
        ToReturn+=Ztring::ToZtring(AudioPresentationCS_termID/10000);
        if (AudioPresentationCS_termID%10000)
        {
            ToReturn+=_T(".");
            ToReturn+=Ztring::ToZtring((AudioPresentationCS_termID%10000)/100);
            if (AudioPresentationCS_termID%100)
            {
                ToReturn+=_T(".");
                ToReturn+=Ztring::ToZtring(AudioPresentationCS_termID%100);
            }
        }
        ToReturn+=_T("\">\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t<mpeg7:Name>")+Mpeg7_AudioPresentationCS_Name(AudioPresentationCS_termID, MI, StreamPos)+_T("</mpeg7:Name>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t</mpeg7:Presentation>\n");
    }

    ToReturn+=_T("\t\t\t\t\t\t\t</mpeg7:AudioCoding>\n");
    return ToReturn;
}

//---------------------------------------------------------------------------
Ztring Export_Mpeg7::Transform(MediaInfo_Internal &MI)
{
    //Current date/time is ISO format
    time_t Time=time(NULL);
    Ztring TimeS; TimeS.Date_From_Seconds_1970((int32u)Time);
    TimeS.FindAndReplace(_T("UTC "), _T(""));
    TimeS.FindAndReplace(_T(" "), _T("T"));
    TimeS+=_T("+00:00");

    Ztring ToReturn;

    //Mpeg7
    ToReturn+=_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    ToReturn+=_T("<!-- Generated at ")+TimeS+_T(" by ")+MediaInfoLib::Config.Info_Version_Get()+_T(" -->\n");
    ToReturn+=_T("<mpeg7:Mpeg7 xmlns=\"urn:mpeg:mpeg7:schema:2004\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:mpeg7=\"urn:mpeg:mpeg7:schema:2004\" xsi:schemaLocation=\"urn:mpeg:mpeg7:schema:2004 http://standards.iso.org/ittf/PubliclyAvailableStandards/MPEG-7_schema_files/mpeg7-v2.xsd\">\n");

    //Description - DescriptionMetadata
    {
        ToReturn+=_T("\t<mpeg7:DescriptionMetadata>\n");
        Ztring FileName=MI.Get(Stream_General, 0, General_FileName);
        Ztring Extension=MI.Get(Stream_General, 0, General_FileExtension);
        if (!Extension.empty())
            FileName+=_T('.')+Extension;
        if (!FileName.empty())
            ToReturn+=_T("\t\t<mpeg7:PrivateIdentifier>")+FileName+_T("</mpeg7:PrivateIdentifier>\n");
        ToReturn+=_T("\t\t<mpeg7:CreationTime>")+TimeS+_T("</mpeg7:CreationTime>\n");
        ToReturn+=_T("\t\t<mpeg7:Instrument>\n");
        ToReturn+=_T("\t\t\t<mpeg7:Tool>\n");
        ToReturn+=_T("\t\t\t\t<mpeg7:Name>")+MediaInfoLib::Config.Info_Version_Get()+_T("</mpeg7:Name>\n");
        ToReturn+=_T("\t\t\t</mpeg7:Tool>\n");
        ToReturn+=_T("\t\t</mpeg7:Instrument>\n");
        ToReturn+=_T("\t</mpeg7:DescriptionMetadata>\n");
    }

    //Description - CreationDescription
    if (!MI.Get(Stream_General, 0, General_Movie).empty()
     || !MI.Get(Stream_General, 0, General_Track).empty()
     || !MI.Get(Stream_General, 0, General_Album).empty()
     || !MI.Get(Stream_General, 0, General_Encoded_Library).empty()
     || !MI.Get(Stream_General, 0, General_Performer).empty())
    {
        ToReturn+=_T("\t<mpeg7:Description xsi:type=\"CreationDescriptionType\">\n");
        ToReturn+=_T("\t\t<mpeg7:CreationInformation>\n");
        ToReturn+=_T("\t\t\t<mpeg7:Creation>\n");
        if (!MI.Get(Stream_General, 0, General_Movie).empty())
            ToReturn+=_T("\t\t\t\t<mpeg7:Title type=\"songTitle\">")+MI.Get(Stream_General, 0, General_Movie)+_T("</mpeg7:Title>\n");
        if (!MI.Get(Stream_General, 0, General_Track).empty())
            ToReturn+=_T("\t\t\t\t<mpeg7:Title type=\"songTitle\">")+MI.Get(Stream_General, 0, General_Title)+_T("</mpeg7:Title>\n");
        if (!MI.Get(Stream_General, 0, General_Album).empty())
            ToReturn+=_T("\t\t\t\t<mpeg7:Title type=\"albumTitle\">")+MI.Get(Stream_General, 0, General_Album)+_T("</mpeg7:Title>\n");
        if (!MI.Get(Stream_General, 0, General_WrittenBy).empty())
        {
            ToReturn+=_T("\t\t\t\t<mpeg7:Creator>\n");
            ToReturn+=_T("\t\t\t\t\t<mpeg7:Role href=\"urn:mpeg:mpeg7:cs:RoleCS:2001:AUTHOR\"/>\n");
            ToReturn+=_T("\t\t\t\t\t<mpeg7:Agent xsi:type=\"PersonGroupType\">\n");
            ToReturn+=_T("\t\t\t\t\t\t<mpeg7:Name>")+MI.Get(Stream_General, 0, General_WrittenBy)+_T("</mpeg7:Name>\n");
            ToReturn+=_T("\t\t\t\t\t</mpeg7:Agent>\n");
            ToReturn+=_T("\t\t\t\t</mpeg7:Creator>\n");
        }
        if (!MI.Get(Stream_General, 0, General_Performer).empty())
        {
            ToReturn+=_T("\t\t\t\t<mpeg7:Creator>\n");
            ToReturn+=_T("\t\t\t\t\t<mpeg7:Role href=\"urn:mpeg:mpeg7:cs:RoleCS:2001:PERFORMER\"/>\n");
            ToReturn+=_T("\t\t\t\t\t<mpeg7:Agent xsi:type=\"PersonGroupType\">\n");
            ToReturn+=_T("\t\t\t\t\t\t<mpeg7:Name>")+MI.Get(Stream_General, 0, General_Performer)+_T("</mpeg7:Name>\n");
            ToReturn+=_T("\t\t\t\t\t</mpeg7:Agent>\n");
            ToReturn+=_T("\t\t\t\t</mpeg7:Creator>\n");
        }
        if (!MI.Get(Stream_General, 0, General_ExecutiveProducer).empty())
        {
            ToReturn+=_T("\t\t\t\t<mpeg7:Creator>\n");
            ToReturn+=_T("\t\t\t\t\t<mpeg7:Role href=\"urn:mpeg:mpeg7:cs:RoleCS:2001:EXECUTIVE-PRODUCER\"/>\n");
            ToReturn+=_T("\t\t\t\t\t<mpeg7:Agent xsi:type=\"PersonGroupType\">\n");
            ToReturn+=_T("\t\t\t\t\t\t<mpeg7:Name>")+MI.Get(Stream_General, 0, General_ExecutiveProducer)+_T("</mpeg7:Name>\n");
            ToReturn+=_T("\t\t\t\t\t</mpeg7:Agent>\n");
            ToReturn+=_T("\t\t\t\t</mpeg7:Creator>\n");
        }
        if (!MI.Get(Stream_General, 0, General_Producer).empty())
        {
            ToReturn+=_T("\t\t\t\t<mpeg7:Creator>\n");
            ToReturn+=_T("\t\t\t\t\t<mpeg7:Role href=\"urn:mpeg:mpeg7:cs:RoleCS:2001:PRODUCER\"/>\n");
            ToReturn+=_T("\t\t\t\t\t<mpeg7:Agent xsi:type=\"PersonGroupType\">\n");
            ToReturn+=_T("\t\t\t\t\t\t<mpeg7:Name>")+MI.Get(Stream_General, 0, General_Producer)+_T("</mpeg7:Name>\n");
            ToReturn+=_T("\t\t\t\t\t</mpeg7:Agent>\n");
            ToReturn+=_T("\t\t\t\t</mpeg7:Creator>\n");
        }
        if (!MI.Get(Stream_General, 0, General_Director).empty())
        {
            ToReturn+=_T("\t\t\t\t<mpeg7:Creator>\n");
            ToReturn+=_T("\t\t\t\t\t<mpeg7:Role href=\"urn:mpeg:mpeg7:cs:RoleCS:2001:PRODUCER\"/>\n");
            ToReturn+=_T("\t\t\t\t\t<mpeg7:Agent xsi:type=\"PersonGroupType\">\n");
            ToReturn+=_T("\t\t\t\t\t\t<mpeg7:Name>")+MI.Get(Stream_General, 0, General_Director)+_T("</mpeg7:Name>\n");
            ToReturn+=_T("\t\t\t\t\t</mpeg7:Agent>\n");
            ToReturn+=_T("\t\t\t\t</mpeg7:Creator>\n");
        }
        if (!MI.Get(Stream_General, 0, General_Composer).empty())
        {
            ToReturn+=_T("\t\t\t\t<mpeg7:Creator>\n");
            ToReturn+=_T("\t\t\t\t\t<mpeg7:Role href=\"urn:mpeg:mpeg7:cs:RoleCS:2001:COMPOSER\"/>\n");
            ToReturn+=_T("\t\t\t\t\t<mpeg7:Agent xsi:type=\"PersonGroupType\">\n");
            ToReturn+=_T("\t\t\t\t\t\t<mpeg7:Name>")+MI.Get(Stream_General, 0, General_Composer)+_T("</mpeg7:Name>\n");
            ToReturn+=_T("\t\t\t\t\t</mpeg7:Agent>\n");
            ToReturn+=_T("\t\t\t\t</mpeg7:Creator>\n");
        }
        if (!MI.Get(Stream_General, 0, General_CostumeDesigner).empty())
        {
            ToReturn+=_T("\t\t\t\t<mpeg7:Creator>\n");
            ToReturn+=_T("\t\t\t\t\t<mpeg7:Role href=\"urn:mpeg:mpeg7:cs:RoleCS:2001:COSTUME-SUPERVISOR\"/>\n");
            ToReturn+=_T("\t\t\t\t\t<mpeg7:Agent xsi:type=\"PersonGroupType\">\n");
            ToReturn+=_T("\t\t\t\t\t\t<mpeg7:Name>")+MI.Get(Stream_General, 0, General_CostumeDesigner)+_T("</mpeg7:Name>\n");
            ToReturn+=_T("\t\t\t\t\t</mpeg7:Agent>\n");
            ToReturn+=_T("\t\t\t\t</mpeg7:Creator>\n");
        }
        if (!MI.Get(Stream_General, 0, General_ProductionDesigner).empty())
        {
            ToReturn+=_T("\t\t\t\t<mpeg7:Creator>\n");
            ToReturn+=_T("\t\t\t\t\t<mpeg7:Role href=\"urn:mpeg:mpeg7:cs:RoleCS:2001:PRODUCTION-DESIGNER\"/>\n");
            ToReturn+=_T("\t\t\t\t\t<mpeg7:Agent xsi:type=\"PersonGroupType\">\n");
            ToReturn+=_T("\t\t\t\t\t\t<mpeg7:Name>")+MI.Get(Stream_General, 0, General_ProductionDesigner)+_T("</mpeg7:Name>\n");
            ToReturn+=_T("\t\t\t\t\t</mpeg7:Agent>\n");
            ToReturn+=_T("\t\t\t\t</mpeg7:Creator>\n");
        }
        if (!MI.Get(Stream_General, 0, General_Publisher).empty())
        {
            ToReturn+=_T("\t\t\t\t<mpeg7:Creator>\n");
            ToReturn+=_T("\t\t\t\t\t<mpeg7:Role href=\"urn:mpeg:mpeg7:cs:RoleCS:2001:PUBLISHER\"/>\n");
            ToReturn+=_T("\t\t\t\t\t<mpeg7:Agent xsi:type=\"PersonGroupType\">\n");
            ToReturn+=_T("\t\t\t\t\t\t<mpeg7:Name>")+MI.Get(Stream_General, 0, General_Publisher)+_T("</mpeg7:Name>\n");
            ToReturn+=_T("\t\t\t\t\t</mpeg7:Agent>\n");
            ToReturn+=_T("\t\t\t\t</mpeg7:Creator>\n");
        }
        if (!MI.Get(Stream_General, 0, General_DistributedBy).empty())
        {
            ToReturn+=_T("\t\t\t\t<mpeg7:Creator>\n");
            ToReturn+=_T("\t\t\t\t\t<mpeg7:Role href=\"urn:mpeg:mpeg7:cs:RoleCS:2001:DISTRIBUTOR\"/>\n");
            ToReturn+=_T("\t\t\t\t\t<mpeg7:Agent xsi:type=\"PersonGroupType\">\n");
            ToReturn+=_T("\t\t\t\t\t\t<mpeg7:Name>")+MI.Get(Stream_General, 0, General_DistributedBy)+_T("</mpeg7:Name>\n");
            ToReturn+=_T("\t\t\t\t\t</mpeg7:Agent>\n");
            ToReturn+=_T("\t\t\t\t</mpeg7:Creator>\n");
        }
        if (!MI.Get(Stream_General, 0, General_Encoded_Library).empty())
        {
            ToReturn+=_T("\t\t\t\t<mpeg7:CreationTool>\n");
            ToReturn+=_T("\t\t\t\t\t<mpeg7:Tool>\n");
            ToReturn+=_T("\t\t\t\t\t\t<mpeg7:Name>")+MI.Get(Stream_General, 0, General_Encoded_Library)+_T("</mpeg7:Name>\n");
            ToReturn+=_T("\t\t\t\t\t</mpeg7:Tool>\n");
            ToReturn+=_T("\t\t\t\t</mpeg7:CreationTool>\n");
        }
        ToReturn+=_T("\t\t\t</mpeg7:Creation>\n");
        ToReturn+=_T("\t\t</mpeg7:CreationInformation>\n");
        ToReturn+=_T("\t</mpeg7:Description>\n");
    }

    //Description - ContentEntity
    ToReturn+=_T("\t<mpeg7:Description xsi:type=\"ContentEntityType\">\n");

    //MultimediaContent
    ToReturn+=_T("\t\t<mpeg7:MultimediaContent xsi:type=\""); ToReturn+=Mpeg7_Type(MI); ToReturn+=_T("Type\">\n");

    //(Type)
    ToReturn+=_T("\t\t\t<mpeg7:"); ToReturn+=Mpeg7_Type(MI); ToReturn+=_T(">\n");

    //MediaFormat header
    ToReturn+=_T("\t\t\t\t<mpeg7:MediaInformation>\n");
    ToReturn+=_T("\t\t\t\t\t<mpeg7:MediaProfile>\n");
    ToReturn+=_T("\t\t\t\t\t\t<mpeg7:MediaFormat>\n");

    //Content
    ToReturn+=_T("\t\t\t\t\t\t\t<mpeg7:Content");
    int32u ContentCS_termID=Mpeg7_ContentCS_termID(MI);
    if (ContentCS_termID>=500000) //Out of spec
        ToReturn+=_T(" href=\"urn:x-mpeg7-mediainfo:cs:ContentCS:2009:");
    else
        ToReturn+=_T(" href=\"urn:mpeg:mpeg7:cs:ContentCS:2001:");
    ToReturn+=Ztring::ToZtring(ContentCS_termID/10000);
    if (ContentCS_termID%10000)
    {
        ToReturn+=_T(".");
        ToReturn+=Ztring::ToZtring((ContentCS_termID%10000)/100);
        if (ContentCS_termID%100)
        {
            ToReturn+=_T(".");
            ToReturn+=Ztring::ToZtring(ContentCS_termID%100);
        }
    }
    ToReturn+=_T("\"");
    ToReturn+=_T(">\n");
    ToReturn+=_T("\t\t\t\t\t\t\t\t<mpeg7:Name xml:lang=\"en\">"); ToReturn+=Mpeg7_ContentCS_Name(ContentCS_termID, MI); ToReturn+=_T("</mpeg7:Name>\n");
    ToReturn+=_T("\t\t\t\t\t\t\t</mpeg7:Content>\n");

    //FileFormat
    if (!MI.Get(Stream_General, 0, General_Format).empty())
    {
        ToReturn+=_T("\t\t\t\t\t\t\t<mpeg7:FileFormat");
        int32u FileFormatCS_termID=Mpeg7_FileFormatCS_termID(MI);
        if (FileFormatCS_termID)
        {
            if (FileFormatCS_termID>=500000) //Out of spec
                ToReturn+=_T(" href=\"urn:x-mpeg7-mediainfo:cs:FileFormatCS:2009:");
            else
                ToReturn+=_T(" href=\"urn:mpeg:mpeg7:cs:FileFormatCS:2001:");
            ToReturn+=Ztring::ToZtring(FileFormatCS_termID/10000);
            ToReturn+=_T("\"");
        }
        ToReturn+=_T(">\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t\t<mpeg7:Name xml:lang=\"en\">"); ToReturn+=Mpeg7_FileFormatCS_Name((FileFormatCS_termID/10000)*10000, MI); ToReturn+=_T("</mpeg7:Name>\n");
        if (FileFormatCS_termID%10000)
        {
            ToReturn+=_T("\t\t\t\t\t\t\t\t\t<mpeg7:Term termID=\"");
            ToReturn+=Ztring::ToZtring(FileFormatCS_termID/10000);
            ToReturn+=_T(".");
            ToReturn+=Ztring::ToZtring((FileFormatCS_termID%10000)/100);
            ToReturn+=_T("\">\n");
            ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t<mpeg7:Name xml:lang=\"en\">"); ToReturn+=Mpeg7_FileFormatCS_Name((FileFormatCS_termID/100)*100, MI); ToReturn+=_T("</mpeg7:Name>\n");
            if (FileFormatCS_termID%100)
            {
                ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t<mpeg7:Term termID=\"");
                ToReturn+=Ztring::ToZtring(FileFormatCS_termID/10000);
                ToReturn+=_T(".");
                ToReturn+=Ztring::ToZtring((FileFormatCS_termID%10000)/100);
                ToReturn+=_T(".");
                ToReturn+=Ztring::ToZtring(FileFormatCS_termID%100);
                ToReturn+=_T("\">\n");
                ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t\t<mpeg7:Name xml:lang=\"en\">"); ToReturn+=Mpeg7_FileFormatCS_Name(FileFormatCS_termID, MI); ToReturn+=_T("</mpeg7:Name>\n");
                ToReturn+=_T("\t\t\t\t\t\t\t\t\t\t</mpeg7:Term>\n");
            }
            ToReturn+=_T("\t\t\t\t\t\t\t\t\t</mpeg7:Term>\n");
        }
        ToReturn+=_T("\t\t\t\t\t\t\t</mpeg7:FileFormat>\n");
    }

    //FileSize
    ToReturn+=_T("\t\t\t\t\t\t\t<mpeg7:FileSize>"); ToReturn+=MI.Get(Stream_General, 0, General_FileSize); ToReturn+=_T("</mpeg7:FileSize>\n");

    //System
    if (!MI.Get(Stream_Video, 0, Video_Standard).empty())
    {
        ToReturn+=_T("\t\t\t\t\t\t\t<mpeg7:System");
        int32u SystemCS_termID=Mpeg7_SystemCS_termID(MI, 0); //2 video streams are not supported
        if (SystemCS_termID)
        {
            ToReturn+=_T(" href=\"urn:mpeg:mpeg7:cs:SystemCS:2001:");
            ToReturn+=Ztring::ToZtring(SystemCS_termID/10000);
            if (SystemCS_termID%10000)
            {
                ToReturn+=_T(".");
                ToReturn+=Ztring::ToZtring((SystemCS_termID%10000)/100);
                if (SystemCS_termID%100)
                {
                    ToReturn+=_T(".");
                    ToReturn+=Ztring::ToZtring(SystemCS_termID%100);
                }
            }
            ToReturn+=_T("\"");
        }
        ToReturn+=_T(">\n");
        ToReturn+=_T("\t\t\t\t\t\t\t\t<mpeg7:Name xml:lang=\"en\">"); ToReturn+=Mpeg7_SystemCS_Name(SystemCS_termID); ToReturn+=_T("</mpeg7:Name>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t</mpeg7:System>\n");
    }

    //BitRate
    if (!MI.Get(Stream_General, 0, General_OverallBitRate).empty())
    {
        ToReturn+=_T("\t\t\t\t\t\t\t<mpeg7:BitRate");
        bool IsCBR=true;
        bool IsVBR=true;
        for (size_t StreamKind=Stream_Video; StreamKind<=Stream_Audio; StreamKind++)
            for (size_t StreamPos=0; StreamPos<MI.Count_Get((stream_t)StreamKind); StreamPos++)
            {
                if (IsCBR && MI.Get((stream_t)StreamKind, StreamPos, _T("BitRate_Mode"))==_T("VBR"))
                    IsCBR=false;
                if (IsVBR && MI.Get((stream_t)StreamKind, StreamPos, _T("BitRate_Mode"))==_T("CBR"))
                    IsVBR=false;
            }
        if (IsCBR && IsVBR)
        {
            IsCBR=false;
            IsVBR=false;
        }
        if (IsCBR)
            ToReturn+=_T(" variable=\"false\"");
        if (IsVBR)
            ToReturn+=_T(" variable=\"true\"");
        ToReturn+=_T(">"); ToReturn+=MI.Get(Stream_General, 0, General_OverallBitRate); ToReturn+=_T("</mpeg7:BitRate>\n");
    }

    //xxxCoding
    for (size_t Pos=0; Pos<MI.Count_Get(Stream_Video); Pos++)
        Mpeg7_Transform_Visual(ToReturn, MI, Pos);
    for (size_t Pos=0; Pos<MI.Count_Get(Stream_Audio); Pos++)
        Mpeg7_Transform_Audio(ToReturn, MI, Pos);

    //MediaFormat header
    ToReturn+=_T("\t\t\t\t\t\t</mpeg7:MediaFormat>\n");

    //MediaTranscodingHints, intraFrameDistance and anchorFrameDistance
    if (!MI.Get(Stream_Video, 0, Video_Format_Settings_GOP).empty())
    {
        Ztring M=MI.Get(Stream_Video, 0, Video_Format_Settings_GOP).SubString(_T("M="), _T(","));
        Ztring N=MI.Get(Stream_Video, 0, Video_Format_Settings_GOP).SubString(_T("N="), _T(""));
        ToReturn+=_T("\t\t\t\t\t\t<mpeg7:MediaTranscodingHints>\n");
        ToReturn+=_T("\t\t\t\t\t\t\t<mpeg7:CodingHints");
        if (!N.empty())
            ToReturn+=_T(" intraFrameDistance=\"")+N;
        if (!M.empty())
            ToReturn+=_T("\" anchorFrameDistance=\"")+M;
        ToReturn+=_T("\"/>\n");
        ToReturn+=_T("\t\t\t\t\t\t</mpeg7:MediaTranscodingHints>\n");
    }

    ToReturn+=_T("\t\t\t\t\t</mpeg7:MediaProfile>\n");
    ToReturn+=_T("\t\t\t\t</mpeg7:MediaInformation>\n");

    if (MI.Count_Get(Stream_Video)==1 || MI.Count_Get(Stream_Audio)==1)
    {
        //MediaTimePoint
        ToReturn+=_T("\t\t\t\t<mpeg7:MediaTime>\n");
        ToReturn+=_T("\t\t\t\t\t<mpeg7:MediaTimePoint>")+Mpeg7_MediaTimePoint(MI)+_T("</mpeg7:MediaTimePoint>\n");

        //MediaDuration
        ToReturn+=_T("\t\t\t\t\t<mpeg7:MediaDuration>"); ToReturn+=Mpeg7_MediaDuration(MI); ToReturn+=_T("</mpeg7:MediaDuration>\n");
        ToReturn+=_T("\t\t\t\t</mpeg7:MediaTime>\n");
    }

    //Mpeg7 footer
    ToReturn+=_T("\t\t\t</mpeg7:"); ToReturn+=Mpeg7_Type(MI); ToReturn+=_T(">\n");
    ToReturn+=_T("\t\t</mpeg7:MultimediaContent>\n");
    ToReturn+=_T("\t</mpeg7:Description>\n");
    ToReturn+=_T("</mpeg7:Mpeg7>\n");

    //Carriage return
    ToReturn.FindAndReplace(_T("\n"), EOL, 0, Ztring_Recursive);

    //Find and replace
    ZtringListList ToReplace=MediaInfoLib::Config.Inform_Replace_Get_All();
    for (size_t Pos=0; Pos<ToReplace.size(); Pos++)
        ToReturn.FindAndReplace(ToReplace[Pos][0], ToReplace[Pos][1], 0, Ztring_Recursive);

    return ToReturn;
}

//***************************************************************************
//
//***************************************************************************

} //NameSpace
