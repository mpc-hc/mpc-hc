/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

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
#include "MediaInfo/Export/Export_EbuCore.h"
#include "MediaInfo/File__Analyse_Automatic.h"
#include <ctime>
using namespace std;
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
int32u EbuCore_VideoCompressionCodeCS_termID(MediaInfo_Internal &MI, size_t StreamPos)
{
    const Ztring &Format=MI.Get(Stream_Video, StreamPos, Video_Format);
    const Ztring &Version=MI.Get(Stream_Video, StreamPos, Video_Format_Version);
    const Ztring &Profile=MI.Get(Stream_Video, StreamPos, Video_Format_Profile);

    if (Format==__T("MPEG Video"))
    {
        if (Version.find(__T("1"))!=string::npos)
            return 10000;
        if (Version.find(__T("2"))!=string::npos)
        {
            if (Profile.find(__T("Simple@"))!=string::npos)
            {
                if (Profile.find(__T("Main"))!=string::npos)
                    return 20101;
                return 20100;
            }
            if (Profile.find(__T("Main@"))!=string::npos)
            {
                if (Profile.find(__T("Low"))!=string::npos)
                    return 20201;
                if (Profile.find(__T("@Main"))!=string::npos)
                    return 20202;
                if (Profile.find(__T("High 1440"))!=string::npos)
                    return 20203;
                if (Profile.find(__T("High"))!=string::npos)
                    return 20204;
                return 20200;
            }
            if (Profile.find(__T("SNR Scalable@"))!=string::npos)
            {
                if (Profile.find(__T("Low"))!=string::npos)
                    return 20301;
                if (Profile.find(__T("Main"))!=string::npos)
                    return 20302;
                return 20300;
            }
            if (Profile.find(__T("Spatial Sclable@"))!=string::npos)
            {
                if (Profile.find(__T("Main"))!=string::npos)
                    return 20401;
                if (Profile.find(__T("High 1440"))!=string::npos)
                    return 20402;
                if (Profile.find(__T("High"))!=string::npos)
                    return 20403;
                return 20400;
            }
            if (Profile.find(__T("High@"))!=string::npos)
            {
                if (Profile.find(__T("Main"))!=string::npos)
                    return 20501;
                if (Profile.find(__T("High 1440"))!=string::npos)
                    return 20502;
                if (Profile.find(__T("High"))!=string::npos)
                    return 20503;
                return 20500;
            }
            if (Profile.find(__T("Multi-view@"))!=string::npos)
            {
                if (Profile.find(__T("Main"))!=string::npos)
                    return 20601;
                return 20600;
            }
            if (Profile.find(__T("4:2:2@"))!=string::npos)
            {
                if (Profile.find(__T("Main"))!=string::npos)
                    return 20701;
                return 20700;
            }
            return 20000;
        }
    }
    if (Format==__T("MPEG-4 Visual"))
    {
        if (Profile.find(__T("Simple@"))==0)
        {
            if (Profile.find(__T("L0"))!=string::npos)
                return 30101;
            if (Profile.find(__T("L1"))!=string::npos)
                return 30102;
            if (Profile.find(__T("L2"))!=string::npos)
                return 30103;
            if (Profile.find(__T("L3"))!=string::npos)
                return 30104;
            if (Profile.find(__T("L4"))!=string::npos)
                return 30105;
            if (Profile.find(__T("L5"))!=string::npos)
                return 30106;
            return 30100;
        }
        if (Profile.find(__T("Simple Scalable@"))==0)
        {
            if (Profile.find(__T("L1"))!=string::npos)
                return 30201;
            if (Profile.find(__T("L2"))!=string::npos)
                return 30202;
            if (Profile.find(__T("L3"))!=string::npos)
                return 30203;
            return 30200;
        }
        if (Profile.find(__T("Advanced Simple@"))==0)
        {
            if (Profile.find(__T("L0"))!=string::npos)
                return 30301;
            if (Profile.find(__T("L1"))!=string::npos)
                return 30302;
            if (Profile.find(__T("L2"))!=string::npos)
                return 30303;
            if (Profile.find(__T("L3"))!=string::npos)
                return 30304;
            if (Profile.find(__T("L4"))!=string::npos)
                return 30305;
            if (Profile.find(__T("L5"))!=string::npos)
                return 30306;
            return 30100;
        }
        if (Profile.find(__T("Core@"))==0)
        {
            if (Profile.find(__T("L1"))!=string::npos)
                return 30401;
            if (Profile.find(__T("L2"))!=string::npos)
                return 30402;
            return 30400;
        }
        if (Profile.find(__T("Core Scalable@"))==0)
        {
            if (Profile.find(__T("L1"))!=string::npos)
                return 30501;
            if (Profile.find(__T("L2"))!=string::npos)
                return 30502;
            if (Profile.find(__T("L3"))!=string::npos)
                return 30503;
            return 30500;
        }
        if (Profile.find(__T("Advanced Core@"))==0)
        {
            if (Profile.find(__T("L1"))!=string::npos)
                return 30601;
            if (Profile.find(__T("L2"))!=string::npos)
                return 30602;
            return 30600;
        }
        if (Profile.find(__T("Main@"))==0)
        {
            if (Profile.find(__T("L2"))!=string::npos)
                return 30701;
            if (Profile.find(__T("L3"))!=string::npos)
                return 30702;
            if (Profile.find(__T("L4"))!=string::npos)
                return 30703;
            return 30700;
        }
        if (Profile.find(__T("N-bit@"))==0)
        {
            if (Profile.find(__T("L2"))!=string::npos)
                return 30801;
            return 30800;
        }
        if (Profile.find(__T("Advanced Real Time Simple@"))==0)
        {
            if (Profile.find(__T("L1"))!=string::npos)
                return 30901;
            if (Profile.find(__T("L2"))!=string::npos)
                return 30902;
            if (Profile.find(__T("L3"))!=string::npos)
                return 30903;
            if (Profile.find(__T("L4"))!=string::npos)
                return 30904;
            return 30900;
        }
        if (Profile.find(__T("Advanced Coding Efficiency@"))==0)
        {
            if (Profile.find(__T("L1"))!=string::npos)
                return 31001;
            if (Profile.find(__T("L2"))!=string::npos)
                return 31002;
            if (Profile.find(__T("L3"))!=string::npos)
                return 31003;
            if (Profile.find(__T("L4"))!=string::npos)
                return 31004;
            return 31000;
        }
        if (Profile.find(__T("Simple Studio@"))==0)
        {
            if (Profile.find(__T("L1"))!=string::npos)
                return 31101;
            if (Profile.find(__T("L2"))!=string::npos)
                return 31102;
            if (Profile.find(__T("L3"))!=string::npos)
                return 31103;
            if (Profile.find(__T("L4"))!=string::npos)
                return 31104;
            return 31100;
        }
        if (Profile.find(__T("Core Studio@"))==0)
        {
            if (Profile.find(__T("L1"))!=string::npos)
                return 31201;
            if (Profile.find(__T("L2"))!=string::npos)
                return 31202;
            if (Profile.find(__T("L3"))!=string::npos)
                return 31203;
            if (Profile.find(__T("L4"))!=string::npos)
                return 31204;
            return 31200;
        }
        if (Profile.find(__T("Fine Granularity Scalable@"))==0)
        {
            if (Profile.find(__T("L0"))!=string::npos)
                return 31301;
            if (Profile.find(__T("L1"))!=string::npos)
                return 31302;
            if (Profile.find(__T("L2"))!=string::npos)
                return 31303;
            if (Profile.find(__T("L3"))!=string::npos)
                return 31304;
            if (Profile.find(__T("L4"))!=string::npos)
                return 31305;
            if (Profile.find(__T("L5"))!=string::npos)
                return 31306;
            return 31300;
        }
        if (Profile.find(__T("Simple Face Animation@"))==0)
        {
            if (Profile.find(__T("L1"))!=string::npos)
                return 31401;
            if (Profile.find(__T("L2"))!=string::npos)
                return 31402;
            return 31400;
        }
        if (Profile.find(__T("Simple FBA@"))==0)
        {
            if (Profile.find(__T("L1"))!=string::npos)
                return 31501;
            if (Profile.find(__T("L2"))!=string::npos)
                return 31502;
            return 31500;
        }
        if (Profile.find(__T("Basic Animated Texture@"))==0)
        {
            if (Profile.find(__T("L1"))!=string::npos)
                return 31601;
            if (Profile.find(__T("L2"))!=string::npos)
                return 31602;
            return 31600;
        }
        if (Profile.find(__T("Scalable Texture@"))==0)
        {
            if (Profile.find(__T("L1"))!=string::npos)
                return 31701;
            return 31700;
        }
        if (Profile.find(__T("Advanced Scalable Texture@"))==0)
        {
            if (Profile.find(__T("L1"))!=string::npos)
                return 31801;
            if (Profile.find(__T("L2"))!=string::npos)
                return 31802;
            if (Profile.find(__T("L3"))!=string::npos)
                return 31803;
            return 31800;
        }
        if (Profile.find(__T("Hybrid@"))==0)
        {
            if (Profile.find(__T("L1"))!=string::npos)
                return 31901;
            if (Profile.find(__T("L2"))!=string::npos)
                return 31902;
            return 31900;
        }
        return 30000;
    }
    if (Format==__T("JPEG"))
        return 50000;
    if (Format==__T("JPEG 2000"))
    {
        const Ztring &CodecID=MI.Get(Stream_Video, StreamPos, Video_CodecID);
        if (CodecID==__T("mjp2"))
            return 60100;
        if (CodecID==__T("mjs2"))
            return 60200;
        return 60000;
    }
    if (Format==__T("H.261"))
        return 70000;
    if (Format==__T("H.263"))
        return 80000;

    return 0;
}

Ztring EbuCore_VideoCompressionCodeCS_Name(int32u termID, MediaInfo_Internal &MI, size_t StreamPos) //xxyyzz: xx=main number, yy=sub-number, zz=sub-sub-number
{
    switch (termID/10000)
    {
        case 1 : return __T("MPEG-1 Video");
        case 2 :    switch ((termID%10000)/100)
                    {
                        case 1 :    switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-2 Video Simple Profile @ Main Level");
                                        default: return __T("MPEG-2 Video Simple Profile");
                                    }
                        case 2 :    switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-2 Video Main Profile @ Low Level");
                                        case 2 : return __T("MPEG-2 Video Main Profile @ Main Level");
                                        case 3 : return __T("MPEG-2 Video Main Profile @ High 1440 Level");
                                        case 4 : return __T("MPEG-2 Video Main Profile @ High Level");
                                        default: return __T("MPEG-2 Video Main Profile");
                                    }
                        case 3 :    switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-2 Video SNR Scalable Profile @ Low Level");
                                        case 2 : return __T("MPEG-2 Video SNR Scalable Profile @ Main Level");
                                        default: return __T("MPEG-2 Video SNR Scalable Profile");
                                    }
                        case 4 :    switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-2 Video Spatial Scalable Profile @ Main Level");
                                        case 2 : return __T("MPEG-2 Video Spatial Scalable Profile @ High 1440 Level");
                                        case 3 : return __T("MPEG-2 Video Spatial Scalable Profile @ High Level");
                                        default: return __T("MPEG-2 Video Spatial Scalable Profile");
                                    }
                        case 5 :    switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-2 Video High Profile @ Main Level");
                                        case 2 : return __T("MPEG-2 Video High Profile @ High 1440 Level");
                                        case 3 : return __T("MPEG-2 Video High Profile @ High Level");
                                        default: return __T("MPEG-2 Video High Profile");
                                    }
                        case 6 :    switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-2 Video Multiview Profile @ Main Level");
                                        default: return __T("MPEG-2 Video Multiview Profile");
                                    }
                        case 7 :    switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-2 Video 4:2:2 Profile @ Main Level");
                                        default: return __T("MPEG-2 Video 4:2:2 Profile");
                                    }
                        default: return __T("MPEG-2 Video");
                    }
        case 3 :    switch ((termID%10000)/100)
                    {
                        case  1 :   switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-4 Visual Simple Profile @ Level 0");
                                        case 2 : return __T("MPEG-4 Visual Simple Profile @ Level 1");
                                        case 3 : return __T("MPEG-4 Visual Simple Profile @ Level 2");
                                        case 4 : return __T("MPEG-4 Visual Simple Profile @ Level 3");
                                        default: return __T("MPEG-4 Visual Simple Profile");
                                    }
                        case  2 :   switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-4 Visual Simple Scalable Profile @ Level 1");
                                        case 2 : return __T("MPEG-4 Visual Simple Scalable Profile @ Level 2");
                                        default: return __T("MPEG-4 Visual Simple Scalable Profile");
                                    }
                        case  3 :   switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-4 Advanced Visual Simple Profile @ Level 0");
                                        case 2 : return __T("MPEG-4 Advanced Visual Simple Profile @ Level 1");
                                        case 3 : return __T("MPEG-4 Advanced Visual Simple Profile @ Level 2");
                                        case 4 : return __T("MPEG-4 Advanced Visual Simple Profile @ Level 3");
                                        case 5 : return __T("MPEG-4 Advanced Visual Simple Profile @ Level 4");
                                        case 6 : return __T("MPEG-4 Advanced Visual Simple Profile @ Level 5");
                                        default: return __T("MPEG-4 Advanced Visual Simple Profile");
                                    }
                        case  4 :   switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-4 Visual Core Profile @ Level 1");
                                        case 2 : return __T("MPEG-4 Visual Core Profile @ Level 2");
                                        default: return __T("MPEG-4 Visual Core Profile");
                                    }
                        case  5 :   switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-4 Visual Core-Scalable Profile @ Level 1");
                                        case 2 : return __T("MPEG-4 Visual Core-Scalable Profile @ Level 2");
                                        case 3 : return __T("MPEG-4 Visual Core-Scalable Profile @ Level 3");
                                        default: return __T("MPEG-4 Visual Core-Scalable Profile");
                                    }
                        case  6 :   switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-4 Visual AdvancedCore Profile @ Level 1");
                                        case 2 : return __T("MPEG-4 Visual AdvancedCore Profile @ Level 2");
                                        default: return __T("MPEG-4 Visual AdvancedCore Profile");
                                    }
                        case  7 :   switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-4 Visual Main Profile @ Level 2");
                                        case 2 : return __T("MPEG-4 Visual Main Profile @ Level 3");
                                        case 3 : return __T("MPEG-4 Visual Main Profile @ Level 4");
                                        default: return __T("MPEG-4 Visual Main Profile");
                                    }
                        case  8 :   switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-4 Visual N-bit Profile @ Level 2");
                                        default: return __T("MPEG-4 Visual Main Profile");
                                    }
                        case  9 :   switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-4 Visual Advanced Real Time Simple Profile @ Level 1");
                                        case 2 : return __T("MPEG-4 Visual Advanced Real Time Simple Profile @ Level 2");
                                        case 3 : return __T("MPEG-4 Visual Advanced Real Time Simple Profile @ Level 3");
                                        case 4 : return __T("MPEG-4 Visual Advanced Real Time Simple Profile @ Level 4");
                                        default: return __T("MPEG-4 Visual Advanced Real Time Simple Profile");
                                    }
                        case 10 :   switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-4 Visual Advanced Coding Efficiency Profile @ Level 1");
                                        case 2 : return __T("MPEG-4 Visual Advanced Coding Efficiency Profile @ Level 2");
                                        case 3 : return __T("MPEG-4 Visual Advanced Coding Efficiency Profile @ Level 3");
                                        case 4 : return __T("MPEG-4 Visual Advanced Coding Efficiency Profile @ Level 4");
                                        default: return __T("MPEG-4 Visual Advanced Coding Efficiency Profile");
                                    }
                        case 11 :   switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-4 Visual Simple Studio Profile @ Level 1");
                                        case 2 : return __T("MPEG-4 Visual Simple Studio Profile @ Level 2");
                                        case 3 : return __T("MPEG-4 Visual Simple Studio Profile @ Level 3");
                                        case 4 : return __T("MPEG-4 Visual Simple Studio Profile @ Level 4");
                                        default: return __T("MPEG-4 Visual Simple Studio Profile");
                                    }
                        case 12 :   switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-4 Visual Core Studio Profile @ Level 1");
                                        case 2 : return __T("MPEG-4 Visual Core Studio Profile @ Level 2");
                                        case 3 : return __T("MPEG-4 Visual Core Studio Profile @ Level 3");
                                        case 4 : return __T("MPEG-4 Visual Core Studio Profile @ Level 4");
                                        default: return __T("MPEG-4 Visual Core Studio Profile");
                                    }
                        case 13 :   switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-4 Visual Fine Granularity Scalable Profile @ Level 0");
                                        case 2 : return __T("MPEG-4 Visual Fine Granularity Scalable Profile @ Level 1");
                                        case 3 : return __T("MPEG-4 Visual Fine Granularity Scalable Profile @ Level 2");
                                        case 4 : return __T("MPEG-4 Visual Fine Granularity Scalable Profile @ Level 3");
                                        case 5 : return __T("MPEG-4 Visual Fine Granularity Scalable Profile @ Level 4");
                                        case 6 : return __T("MPEG-4 Visual Fine Granularity Scalable Profile @ Level 5");
                                        default: return __T("MPEG-4 Visual Fine Granularity Scalable Profile");
                                    }
                        case 14 :   switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-4 Simple Face Animation Profile @ Level 1");
                                        case 2 : return __T("MPEG-4 Simple Face Animation Profile @ Level 2");
                                        default: return __T("MPEG-4 Simple Face Animation Profile");
                                    }
                        case 15 :   switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-4 Simple FBA Profile @ Level 1");
                                        case 2 : return __T("MPEG-4 Simple FBA Profile @ Level 2");
                                        default: return __T("MPEG-4 Simple FBA Profile");
                                    }
                        case 16 :   switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-4 Basic Animated Texture Profile @ Level 1");
                                        case 2 : return __T("MPEG-4 Basic Animated Texture Profile @ Level 2");
                                        default: return __T("MPEG-4 Basic Animated Texture Profile");
                                    }
                        case 17 :   switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-4 Advanced Scalable Texture Profile @ Level 1");
                                        default: return __T("MPEG-4 Advanced Scalable Texture Profile");
                                    }
                        case 18 :   switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-4 Visual Advanced Scalable Texture Profile @ Level 1");
                                        case 2 : return __T("MPEG-4 Visual Advanced Scalable Texture Profile @ Level 2");
                                        case 3 : return __T("MPEG-4 Visual Advanced Scalable Texture Profile @ Level 3");
                                        default: return __T("MPEG-4 Visual Advanced Scalable Texture Profile");
                                    }
                        case 19 :   switch (termID%100)
                                    {
                                        case 1 : return __T("MPEG-4 Visual Hybrid Profile @ Level 1");
                                        case 2 : return __T("MPEG-4 Visual Hybrid Profile @ Level 2");
                                        default: return __T("MPEG-4 Visual Hybrid Profile");
                                    }
                        default: return __T("MPEG-4 Visual");
                    }
        case 4 :    return __T("JPEG");
        case 5 :    return __T("MJPEG");
        case 6 :    return __T("JPEG2000");
        case 7 :    return __T("H261");
        case 8 :    return __T("H263");
        default: return MI.Get(Stream_Video, StreamPos, Video_Format);
    }
}

//---------------------------------------------------------------------------
int32u EbuCore_AudioCompressionCodeCS_termID(MediaInfo_Internal &MI, size_t StreamPos)
{
    const Ztring &Format=MI.Get(Stream_Audio, StreamPos, Audio_Format);
    const Ztring &Version=MI.Get(Stream_Audio, StreamPos, Audio_Format_Version);
    const Ztring &Profile=MI.Get(Stream_Audio, StreamPos, Audio_Format_Profile);

    if (Format==__T("AC-3"))
        return 40200;
    if (Format==__T("E-AC-3"))
        return 40300;
    if (Format==__T("Dolby E"))
        return 40600;
    if (Format==__T("DTS"))
        return 50000;
    if (Format==__T("MPEG Audio"))
    {
        if (Version.find(__T("1"))!=string::npos)
        {
            if (Profile.find(__T("1"))!=string::npos)
                return 70100;
            if (Profile.find(__T("2"))!=string::npos)
                return 70200;
            if (Profile.find(__T("3"))!=string::npos)
                return 70300;
            return 70000;
        }
        if (Version.find(__T("2"))!=string::npos)
        {
            if (Profile.find(__T("1"))!=string::npos)
                return 90100;
            if (Profile.find(__T("2"))!=string::npos)
                return 90200;
            if (Profile.find(__T("3"))!=string::npos)
                return 90300;
            return 90000;
        }
        return 0;
    }
    if (Format==__T("PCM"))
        return 110000;

    return 0;
}

Ztring EbuCore_AudioCompressionCodeCS_Name(int32u termID, MediaInfo_Internal &MI, size_t StreamPos) //xxyyzz: xx=main number, yy=sub-number, zz=sub-sub-number
{
    switch (termID/10000)
    {
        case 4 :    switch ((termID%10000)/100)
                    {
                        case 2 : return __T("AC3");
                        case 3 : return __T("E-AC3");
                        case 6 : return __T("Dolby E");
                        default: return __T("Dolby");
                    }
        case 5 : return __T("DTS");
        case 7 :    switch ((termID%10000)/100)
                    {
                        case 1 : return __T("MPEG-1 Audio Layer I");
                        case 2 : return __T("MPEG-1 Audio Layer II");
                        case 3 : return __T("MPEG-1 Audio Layer III");
                        default: return __T("MPEG-1 Audio");
                    }
        case 9 :    switch ((termID%10000)/100)
                    {
                        case 1 : return __T("MPEG-2 Audio Layer I");
                        case 2 : return __T("MPEG-2 Audio Layer II");
                        case 3 : return __T("MPEG-2 Audio Layer III");
                        default: return __T("MPEG-2 Audio");
                    }
        default: return MI.Get(Stream_Audio, StreamPos, Video_Format);
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
Export_EbuCore::Export_EbuCore ()
{
}

//---------------------------------------------------------------------------
Export_EbuCore::~Export_EbuCore ()
{
}

//***************************************************************************
// Input
//***************************************************************************

//---------------------------------------------------------------------------
Ztring EbuCore_Transform_Video(Ztring &ToReturn, MediaInfo_Internal &MI, size_t StreamPos)
{
    ToReturn+=__T("\t\t\t<ebucore:videoFormat");
    //if (!MI.Get(Stream_Video, StreamPos, Video_ID).empty())
    //    ToReturn+=__T(" videoFormatId=\"")+MI.Get(Stream_Video, StreamPos, Video_ID)+__T("\"");
    if (!MI.Get(Stream_Video, StreamPos, Video_Format).empty())
        ToReturn+=__T(" videoFormatName=\"")+MI.Get(Stream_Video, StreamPos, Video_Format)+__T("\"");
    if (!MI.Get(Stream_Video, StreamPos, Video_Format_Version).empty())
        ToReturn+=__T(" videoFormatVersionId=\"")+MI.Get(Stream_Video, StreamPos, Video_Format_Version)+__T("\"");
    ToReturn+=__T(">\n");

    //width
    if (!MI.Get(Stream_Video, StreamPos, Video_Width).empty())
    {
        Ztring Width;
        if (!MI.Get(Stream_Video, StreamPos, Video_Width_Original).empty())
            Width=MI.Get(Stream_Video, StreamPos, Video_Width_Original);
        else
            Width=MI.Get(Stream_Video, StreamPos, Video_Width);
        ToReturn+=__T("\t\t\t\t<ebucore:width unit=\"pixel\">")+MI.Get(Stream_Video, StreamPos, Video_Width)+__T("</ebucore:width>\n");
    }

    //height
    if (!MI.Get(Stream_Video, StreamPos, Video_Height).empty())
    {
        Ztring Height;
        if (!MI.Get(Stream_Video, StreamPos, Video_Height_Original).empty())
            Height=MI.Get(Stream_Video, StreamPos, Video_Height_Original);
        else
            Height=MI.Get(Stream_Video, StreamPos, Video_Height);
        ToReturn+=__T("\t\t\t\t<ebucore:height unit=\"pixel\">")+Height+__T("</ebucore:height>\n");
    }

    //lines
    if (!MI.Get(Stream_Video, StreamPos, Video_Height_Original).empty())
        ToReturn+=__T("\t\t\t\t<ebucore:lines>")+MI.Get(Stream_Video, StreamPos, Video_Height_Original)+__T("</ebucore:lines>\n");

    //frameRate
    if (!MI.Get(Stream_Video, StreamPos, Video_FrameRate).empty())
    {
        Ztring FrameRateString=MI.Get(Stream_Video, StreamPos, Video_FrameRate);
        Ztring factorNumerator, factorDenominator;
        if (FrameRateString==__T("23.976"))
        {
            factorNumerator=__T("24000");
            factorDenominator=__T("1001");
        }
        if (FrameRateString==__T("29.970"))
        {
            factorNumerator=__T("30000");
            factorDenominator=__T("1001");
        }
        if (FrameRateString==__T("59.940"))
        {
            factorNumerator=__T("60000");
            factorDenominator=__T("1001");
        }
        if (factorNumerator.empty())
        {
            factorNumerator=Ztring::ToZtring(FrameRateString.To_float64()*1000, 0);
            factorDenominator=__T("1000");
        }
        ToReturn+=__T("\t\t\t\t<ebucore:frameRate");
        ToReturn+=__T(" factorNumerator=\"")+factorNumerator+__T("\"");
        ToReturn+=__T(" factorDenominator=\"")+factorDenominator+__T("\"");
        ToReturn+=__T(">")+Ztring::ToZtring(FrameRateString.To_float64(), 0);
        ToReturn+=__T("</ebucore:frameRate>\n");
    }

    //aspectRatio
    if (!MI.Get(Stream_Video, StreamPos, Video_DisplayAspectRatio).empty())
    {
        Ztring AspectRatioString=MI.Get(Stream_Video, StreamPos, Video_DisplayAspectRatio_String);
        size_t AspectRatioString_Pos=AspectRatioString.find(__T(':'));
        Ztring factorNumerator, factorDenominator;
        if (AspectRatioString_Pos!=(size_t)-1)
        {
            factorNumerator=AspectRatioString.substr(0, AspectRatioString_Pos);
            factorDenominator=AspectRatioString.substr(AspectRatioString_Pos+1);
        }
        if (factorNumerator.empty())
            ToReturn+=__T("\t\t\t\t<ebucore:aspectRatio typeLabel=\"display\">")+MI.Get(Stream_Video, StreamPos, Video_DisplayAspectRatio)+__T("</ebucore:aspectRatio>\n");
        else
        {
            ToReturn+=__T("\t\t\t\t<ebucore:aspectRatio typeLabel=\"display\">\n");
            ToReturn+=__T("\t\t\t\t\t<ebucore:factorNumerator>")+factorNumerator+__T("</ebucore:factorNumerator>\n");
            ToReturn+=__T("\t\t\t\t\t<ebucore:factorDenominator>")+factorDenominator+__T("</ebucore:factorDenominator>\n");
            ToReturn+=__T("\t\t\t\t</ebucore:aspectRatio>\n");
        }
    }

    //videoEncoding
    //if (!MI.Get(Stream_Video, StreamPos, Video_Format_Profile).empty())
    {
        int32u TermID=EbuCore_VideoCompressionCodeCS_termID(MI, StreamPos);
        Ztring typeLabel;
        Ztring TermID_String;
        if (TermID)
        {
            typeLabel=EbuCore_VideoCompressionCodeCS_Name(TermID, MI, StreamPos);
            TermID_String=Ztring::ToZtring(TermID/10000);
            if (TermID%10000)
            {
                TermID_String+=__T('.');
                TermID_String+=Ztring::ToZtring((TermID%10000)/100);
                if (TermID%100)
                {
                    TermID_String+=__T('.');
                    TermID_String+=Ztring::ToZtring(TermID%100);
                }
            }
        }
        else
            typeLabel=MI.Get(Stream_Video, StreamPos, Video_Format_Profile);
        ToReturn+=__T("\t\t\t\t<ebucore:videoEncoding typeLabel=\"")+typeLabel+__T("\"");
        if (!TermID_String.empty())
            ToReturn+=__T(" typeLink=\"http://www.ebu.ch/metadata/cs/ebu_VideoCompressionCodeCS.xml#")+TermID_String+__T("\"");
        ToReturn+=__T("/>\n");
    }

    //codec
    if (!MI.Get(Stream_Video, StreamPos, Video_CodecID).empty() || !MI.Get(Stream_Video, StreamPos, Video_Format_Commercial_IfAny).empty())
    {
        ToReturn+=__T("\t\t\t\t<ebucore:codec>\n");
        if (!MI.Get(Stream_Video, StreamPos, Video_CodecID).empty())
        {
            ToReturn+=__T("\t\t\t\t\t<ebucore:codecIdentifier>\n");
            ToReturn+=__T("\t\t\t\t\t\t<dc:identifier>")+MI.Get(Stream_Video, StreamPos, Video_CodecID)+__T("</dc:identifier>\n");
            ToReturn+=__T("\t\t\t\t\t</ebucore:codecIdentifier>\n");
        }
        if (!MI.Get(Stream_Video, StreamPos, Video_Format_Commercial_IfAny).empty())
            ToReturn+=__T("\t\t\t\t\t<ebucore:name>")+MI.Get(Stream_Video, StreamPos, Video_Format_Commercial_IfAny)+__T("</ebucore:name>\n");
        ToReturn+=__T("\t\t\t\t</ebucore:codec>\n");
    }

    //bitRate
    if (!MI.Get(Stream_Video, StreamPos, Video_BitRate).empty())
        ToReturn+=__T("\t\t\t\t<ebucore:bitRate>")+MI.Get(Stream_Video, StreamPos, Video_BitRate)+__T("</ebucore:bitRate>\n");

    //bitRateMax
    if (!MI.Get(Stream_Video, StreamPos, Video_BitRate_Maximum).empty())
        ToReturn+=__T("\t\t\t\t<ebucore:bitRateMax>")+MI.Get(Stream_Video, StreamPos, Video_BitRate_Maximum)+__T("</ebucore:bitRateMax>\n");

    //bitRateMode
    if (!MI.Get(Stream_Video, StreamPos, Video_BitRate_Mode).empty())
    {
        Ztring bitRateMode=MI.Get(Stream_Video, StreamPos, Video_BitRate_Mode);
        if (bitRateMode==__T("CBR"))
            bitRateMode=__T("constant");
        if (bitRateMode==__T("VBR"))
            bitRateMode=__T("variable");
        ToReturn+=__T("\t\t\t\t<ebucore:bitRateMode>")+bitRateMode+__T("</ebucore:bitRateMode>\n");
    }

    //scanningFormat
    if (!MI.Get(Stream_Video, StreamPos, Video_ScanType).empty())
    {
        Ztring ScanType=MI.Get(Stream_Video, 0, Video_ScanType);
        if (ScanType==__T("MBAFF"))
            ScanType=__T("Interlaced");
        ScanType.MakeLowerCase();
        ToReturn+=__T("\t\t\t\t<ebucore:scanningFormat>")+ScanType+__T("</ebucore:scanningFormat>\n");
     }

    //scanningOrder
    if (!MI.Get(Stream_Video, StreamPos, Video_ScanOrder).empty())
    {
        Ztring ScanOrder=MI.Get(Stream_Video, 0, Video_ScanOrder);
        if (ScanOrder==__T("TFF"))
            ScanOrder=__T("top");
        if (ScanOrder==__T("BFF"))
            ScanOrder=__T("bottom");
        if (ScanOrder.find(__T("Pulldown"))!=string::npos)
            ScanOrder=__T("pulldown");
        ToReturn+=__T("\t\t\t\t<ebucore:scanningOrder>")+ScanOrder+__T("</ebucore:scanningOrder>\n");
     }

    //videoTrack
    if (!MI.Get(Stream_Video, StreamPos, Video_ID).empty() || !MI.Get(Stream_Video, StreamPos, Video_Title).empty())
    {
        ToReturn+=__T("\t\t\t\t<ebucore:videoTrack");
        if (!MI.Get(Stream_Video, StreamPos, Video_ID).empty())
            ToReturn+=__T(" trackId=\"")+MI.Get(Stream_Video, StreamPos, Video_ID)+__T("\"");
        if (!MI.Get(Stream_Video, StreamPos, Video_Title).empty())
            ToReturn+=__T(" trackName=\"")+MI.Get(Stream_Video, StreamPos, Video_Title)+__T("\"");
        ToReturn+=__T("/>\n");
    }

    //flag_3D
    if (!MI.Get(Stream_Video, StreamPos, Video_MultiView_Count).empty())
        ToReturn+=__T("\t\t\t\t<ebucore:flag_3D>true</ebucore:flag_3D>\n");

     ToReturn+=__T("\t\t\t</ebucore:videoFormat>\n");

    return ToReturn;
}

//---------------------------------------------------------------------------
Ztring EbuCore_Transform_Audio(Ztring &ToReturn, MediaInfo_Internal &MI, size_t StreamPos)
{
    ToReturn+=__T("\t\t\t<ebucore:audioFormat");
    //if (!MI.Get(Stream_Audio, StreamPos, Audio_ID).empty())
    //    ToReturn+=__T(" audioFormatId=\"")+MI.Get(Stream_Audio, StreamPos, Audio_ID)+__T("\"");
    if (!MI.Get(Stream_Audio, StreamPos, Audio_Format).empty())
        ToReturn+=__T(" audioFormatName=\"")+MI.Get(Stream_Audio, StreamPos, Audio_Format)+__T("\"");
    if (!MI.Get(Stream_Audio, StreamPos, Audio_Format_Version).empty())
        ToReturn+=__T(" audioFormatVersionId=\"")+MI.Get(Stream_Audio, StreamPos, Audio_Format_Version)+__T("\"");
    ToReturn+=__T(">\n");

    //audioEncoding
    //if (!MI.Get(Stream_Audio, StreamPos, Audio_Format_Profile).empty())
    {
        int32u TermID=EbuCore_AudioCompressionCodeCS_termID(MI, StreamPos);
        Ztring typeLabel;
        Ztring TermID_String;
        if (TermID)
        {
            typeLabel=EbuCore_AudioCompressionCodeCS_Name(TermID, MI, StreamPos);
            TermID_String=Ztring::ToZtring(TermID/10000);
            if (TermID%10000)
            {
                TermID_String+=__T('.');
                TermID_String+=Ztring::ToZtring((TermID%10000)/100);
                if (TermID%100)
                {
                    TermID_String+=__T('.');
                    TermID_String+=Ztring::ToZtring(TermID%100);
                }
            }
        }
        else
            typeLabel=MI.Get(Stream_Audio, StreamPos, Audio_Format_Profile);
        ToReturn+=__T("\t\t\t\t<ebucore:audioEncoding typeLabel=\"")+typeLabel+__T("\"");
        if (!TermID_String.empty())
            ToReturn+=__T(" typeLink=\"http://www.ebu.ch/metadata/cs/ebu_AudioCompressionCodeCS.xml#")+TermID_String+__T("\"");
        ToReturn+=__T("/>\n");
    }

    //codec
    if (!MI.Get(Stream_Audio, StreamPos, Audio_CodecID).empty() || !MI.Get(Stream_Audio, StreamPos, Audio_Format_Commercial_IfAny).empty())
    {
        ToReturn+=__T("\t\t\t\t<ebucore:codec>\n");
        if (!MI.Get(Stream_Audio, StreamPos, Audio_CodecID).empty())
        {
            ToReturn+=__T("\t\t\t\t\t<ebucore:codecIdentifier>\n");
            ToReturn+=__T("\t\t\t\t\t\t<dc:identifier>")+MI.Get(Stream_Audio, StreamPos, Audio_CodecID)+__T("</dc:identifier>\n");
            ToReturn+=__T("\t\t\t\t\t</ebucore:codecIdentifier>\n");
        }
        if (!MI.Get(Stream_Audio, StreamPos, Audio_Format_Commercial_IfAny).empty())
            ToReturn+=__T("\t\t\t\t\t<ebucore:name>")+MI.Get(Stream_Audio, StreamPos, Audio_Format_Commercial_IfAny)+__T("</ebucore:name>\n");
        ToReturn+=__T("\t\t\t\t</ebucore:codec>\n");
    }

    //audioTrackConfiguration //TODO
    if (!MI.Get(Stream_Audio, StreamPos, Audio_ChannelPositions).empty())
    {
        Ztring ChannelPositions=MI.Get(Stream_Audio, StreamPos, Audio_ChannelPositions);
        ToReturn+=__T("\t\t\t\t<ebucore:audioTrackConfiguration typeLabel=\"")+ChannelPositions+__T("\"/>\n");
    }

    //samplingRate
    if (!MI.Get(Stream_Audio, StreamPos, Audio_SamplingRate).empty())
        ToReturn+=__T("\t\t\t\t<ebucore:samplingRate>")+MI.Get(Stream_Audio, StreamPos, Audio_SamplingRate)+__T("</ebucore:samplingRate>\n");

    //sampleSize
    if (!MI.Get(Stream_Audio, StreamPos, Audio_BitDepth).empty())
        ToReturn+=__T("\t\t\t\t<ebucore:sampleSize>")+MI.Get(Stream_Audio, StreamPos, Audio_BitDepth)+__T("</ebucore:sampleSize>\n");

    //bitRate
    if (!MI.Get(Stream_Audio, StreamPos, Audio_BitRate).empty())
        ToReturn+=__T("\t\t\t\t<ebucore:bitRate>")+MI.Get(Stream_Audio, StreamPos, Audio_BitRate)+__T("</ebucore:bitRate>\n");

    //bitRateMax
    if (!MI.Get(Stream_Audio, StreamPos, Audio_BitRate_Maximum).empty())
        ToReturn+=__T("\t\t\t\t<ebucore:bitRateMax>")+MI.Get(Stream_Audio, StreamPos, Audio_BitRate_Maximum)+__T("</ebucore:bitRateMax>\n");

    //bitRateMode
    if (!MI.Get(Stream_Audio, StreamPos, Audio_BitRate_Mode).empty())
    {
        Ztring bitRateMode=MI.Get(Stream_Audio, StreamPos, Audio_BitRate_Mode);
        if (bitRateMode==__T("CBR"))
            bitRateMode=__T("constant");
        if (bitRateMode==__T("VBR"))
            bitRateMode=__T("variable");
        ToReturn+=__T("\t\t\t\t<ebucore:bitRateMode>")+bitRateMode+__T("</ebucore:bitRateMode>\n");
    }

    //audioTrack
    if (!MI.Get(Stream_Audio, StreamPos, Audio_ID).empty() || !MI.Get(Stream_Audio, StreamPos, Audio_Title).empty() || !MI.Get(Stream_Audio, StreamPos, Audio_Language).empty())
    {
        ToReturn+=__T("\t\t\t\t<ebucore:audioTrack");
        if (!MI.Get(Stream_Audio, StreamPos, Audio_ID).empty())
        {
            Ztring ID=MI.Get(Stream_Audio, StreamPos, Audio_ID);
            ID.FindAndReplace(__T(" / "), __T("_"));
            ToReturn+=__T(" trackId=\"")+ID+__T("\"");
        }
        if (!MI.Get(Stream_Audio, StreamPos, Audio_Title).empty())
            ToReturn+=__T(" trackName=\"")+MI.Get(Stream_Audio, StreamPos, Audio_Title)+__T("\"");
        if (!MI.Get(Stream_Audio, StreamPos, Audio_Language).empty())
            ToReturn+=__T(" trackLanguage=\"")+MI.Get(Stream_Audio, StreamPos, Audio_Language)+__T("\"");
        ToReturn+=__T("/>\n");
   }

    //channels
    if (!MI.Get(Stream_Audio, StreamPos, Audio_Channel_s_).empty())
        ToReturn+=__T("\t\t\t\t<ebucore:channels>")+MI.Get(Stream_Audio, StreamPos, Audio_Channel_s_)+__T("</ebucore:channels>\n");

    ToReturn+=__T("\t\t\t</ebucore:audioFormat>\n");

    return ToReturn;
}

//---------------------------------------------------------------------------
Ztring EbuCore_Transform_Text(Ztring &ToReturn, MediaInfo_Internal &MI, size_t StreamPos)
{
    ToReturn+=__T("\t\t\t<ebucore:dataFormat");
    //if (!MI.Get(Stream_Text, StreamPos, Text_ID).empty())
    //    ToReturn+=__T(" dataFormatId=\"")+MI.Get(Stream_Text, StreamPos, Text_ID)+__T("\"");
    if (!MI.Get(Stream_Text, StreamPos, Text_Format_Version).empty())
        ToReturn+=__T(" dataFormatVersionId=\"")+MI.Get(Stream_Text, StreamPos, Text_Format_Version)+__T("\"");
    if (!MI.Get(Stream_Text, StreamPos, Text_Format).empty())
        ToReturn+=__T(" dataFormatName=\"")+MI.Get(Stream_Text, StreamPos, Text_Format)+__T("\"");
    if (!MI.Get(Stream_Text, StreamPos, Text_ID).empty())
        ToReturn+=__T(" dataTrackId=\"")+MI.Get(Stream_Text, StreamPos, Text_ID)+__T("\"");
    ToReturn+=__T(">\n");

    //subtitlingTrack
    {
        ToReturn+=__T("\t\t\t\t<ebucore:captioningFormat");
        if (!MI.Get(Stream_Text, StreamPos, Text_Format).empty())
            ToReturn+=__T(" captioningFormatName=\"")+MI.Get(Stream_Text, StreamPos, Text_Format)+__T("\"");
        if (!MI.Get(Stream_Text, StreamPos, Text_ID).empty())
            ToReturn+=__T(" trackId=\"")+MI.Get(Stream_Text, StreamPos, Text_ID)+__T("\"");
        if (!MI.Get(Stream_Text, StreamPos, Text_Title).empty())
            ToReturn+=__T(" typeLabel=\"")+MI.Get(Stream_Text, StreamPos, Text_Title)+__T("\"");
        if (!MI.Get(Stream_Text, StreamPos, Text_Language).empty())
            ToReturn+=__T(" language=\"")+MI.Get(Stream_Text, StreamPos, Text_Language)+__T("\"");
        ToReturn+=__T("/>\n");
        if (!MI.Get(Stream_Text, StreamPos, Text_CodecID).empty() || !MI.Get(Stream_Text, StreamPos, Text_Format_Commercial_IfAny).empty())
        {
            ToReturn+=__T("\t\t\t\t<ebucore:codec>\n");
            if (!MI.Get(Stream_Text, StreamPos, Text_CodecID).empty())
            {
                ToReturn+=__T("\t\t\t\t\t<ebucore:codecIdentifier>\n");
                ToReturn+=__T("\t\t\t\t\t\t<dc:identifier>")+MI.Get(Stream_Text, StreamPos, Text_CodecID)+__T("</dc:identifier>\n");
                ToReturn+=__T("\t\t\t\t\t</ebucore:codecIdentifier>\n");
            }
            if (!MI.Get(Stream_Text, StreamPos, Text_Format_Commercial_IfAny).empty())
                ToReturn+=__T("\t\t\t\t\t<ebucore:name>")+MI.Get(Stream_Text, StreamPos, Text_Format_Commercial_IfAny)+__T("</ebucore:name>\n");
            ToReturn+=__T("\t\t\t\t</ebucore:codec>\n");
        }
    }

    ToReturn+=__T("\t\t\t</ebucore:dataFormat>\n");

    return ToReturn;
}

//---------------------------------------------------------------------------
Ztring Export_EbuCore::Transform(MediaInfo_Internal &MI)
{
    //Current date/time is ISO format
    time_t Seconds=time(NULL);
    Ztring DateTime; DateTime.Date_From_Seconds_1970((int32u)Seconds);
    DateTime.FindAndReplace(__T("UTC "), __T(""));
    DateTime.FindAndReplace(__T(" "), __T("T"));
    Ztring Date=DateTime.substr(0, 10);
    Ztring Time=DateTime.substr(11, 8);

    Ztring ToReturn;

    //ebuCoreMain
    ToReturn+=__T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    ToReturn+=__T("<!-- Generated by ")+MediaInfoLib::Config.Info_Version_Get()+__T(" -->\n");
    ToReturn+=__T("<ebucore:ebuCoreMain xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:ebucore=\"urn:ebu:metadata-schema:ebuCore_2014\" xmlns:xalan=\"http://xml.apache.org/xalan\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"urn:ebu:metadata-schema:ebuCore_2014 http://www.ebu.ch/metadata/schemas/EBUCore/20140318/EBU_CORE_20140318.xsd\" version=\"1.5\" dateLastModified=\"")+Date+__T("\" timeLastModified=\"")+Time+__T("Z\">\n");

    //coreMetadata
    ToReturn+=__T("\t<ebucore:coreMetadata>\n");

    //format
    ToReturn+=__T("\t\t<ebucore:format>\n");

    //format - videoFormat
    for (size_t Pos=0; Pos<MI.Count_Get(Stream_Video); Pos++)
        EbuCore_Transform_Video(ToReturn, MI, Pos);

    //format - audioFormat
    for (size_t Pos=0; Pos<MI.Count_Get(Stream_Audio); Pos++)
        EbuCore_Transform_Audio(ToReturn, MI, Pos);

    //format - containerFormat
    ToReturn+=__T("\t\t\t<ebucore:containerFormat");
    if (!MI.Get(Stream_General, 0, General_Format).empty())
        ToReturn+=__T(" formatLabel=\"")+MI.Get(Stream_General, 0, General_Format)+__T("\"");
    if (!MI.Get(Stream_General, 0, General_ID).empty())
        ToReturn+=__T(" containerFormatId=\"")+MI.Get(Stream_General, 0, General_ID)+__T("\"");
    if (!MI.Get(Stream_General, 0, General_CodecID).empty() || !MI.Get(Stream_General, 0, General_Format_Commercial_IfAny).empty())
    {
        ToReturn+=__T(">\n");
        if (!MI.Get(Stream_General, 0, General_CodecID).empty() || (!MI.Get(Stream_General, 0, General_Format_Commercial_IfAny).empty()))
        {
            ToReturn+=__T("\t\t\t\t<ebucore:codec>\n");
            if (!MI.Get(Stream_General, 0, General_CodecID).empty())
            {
                ToReturn+=__T("\t\t\t\t\t<ebucore:codecIdentifier>\n");
                ToReturn+=__T("\t\t\t\t\t\t<dc:identifier>")+MI.Get(Stream_General, 0, General_CodecID)+__T("</dc:identifier>\n");
                ToReturn+=__T("\t\t\t\t\t</ebucore:codecIdentifier>\n");
            }
            if (!MI.Get(Stream_General, 0, General_Format_Commercial_IfAny).empty())
                ToReturn+=__T("\t\t\t\t\t<ebucore:name>")+MI.Get(Stream_General, 0, General_Format_Commercial_IfAny)+__T("</ebucore:name>\n");
            ToReturn+=__T("\t\t\t\t</ebucore:codec>\n");
        }
        ToReturn+=__T("\t\t\t</ebucore:containerFormat>\n");
    }
    else
        ToReturn+=__T("/>\n");

    //format - dataFormat
    for (size_t Pos=0; Pos<MI.Count_Get(Stream_Text); Pos++)
        EbuCore_Transform_Text(ToReturn, MI, Pos);

    //format - start
    bool startCount=0;
    for (size_t StreamPos=0; StreamPos<MI.Count_Get(Stream_Other); ++StreamPos)
    {
        if (MI.Get(Stream_Other, StreamPos, Other_Type)==__T("Time code"))
        {
            if (startCount)
                ToReturn+=__T("\t\t\t<!-- (Not in XSD)\n");
            if (!(!MI.Get(Stream_Video, StreamPos, Video_ID).empty() || !MI.Get(Stream_Video, StreamPos, Video_Title).empty())) //No extra out of spec fields
                ToReturn+=__T("\t\t\t<ebucore:start>\n");
            else if (startCount) //No out of spec in all cases
            {
                ToReturn+=__T("\t\t\t<ebucore:start");
                if (!MI.Get(Stream_Video, StreamPos, Video_ID).empty() || !MI.Get(Stream_Video, StreamPos, Video_Title).empty())
                {
                    if (!MI.Get(Stream_Video, StreamPos, Video_ID).empty())
                        ToReturn+=__T(" trackId=\"")+MI.Get(Stream_Video, StreamPos, Video_ID)+__T("\"");
                    if (!MI.Get(Stream_Video, StreamPos, Video_Title).empty())
                        ToReturn+=__T(" trackName=\"")+MI.Get(Stream_Video, StreamPos, Video_Title)+__T("\"");
                }
                ToReturn+=__T(">\n");
            }
            else //Extra out of spec fields
            {
                ToReturn+=__T("\t\t\t<ebucore:start><!-- (Not in XSD)");
                if (!MI.Get(Stream_Video, StreamPos, Video_ID).empty())
                    ToReturn+=__T(" trackId=\"")+MI.Get(Stream_Video, StreamPos, Video_ID)+__T("\"");
                if (!MI.Get(Stream_Video, StreamPos, Video_Title).empty())
                    ToReturn+=__T(" trackName=\"")+MI.Get(Stream_Video, StreamPos, Video_Title)+__T("\"");
                ToReturn+=__T("-->\n");
            }
            ToReturn+=__T("\t\t\t\t<ebucore:timecode>")+MI.Get(Stream_Other, StreamPos, Other_TimeCode_FirstFrame)+__T("</ebucore:timecode>\n");
            if (!MI.Get(Stream_Other, 0, Other_MuxingMode).empty())
            {
                if (!startCount)
                    ToReturn+=__T("\t\t\t\t<!-- (Not in XSD)\n");
                ToReturn+=__T("\t\t\t\t<ebucore:source>")+MI.Get(Stream_Other, StreamPos, Other_MuxingMode)+__T("</ebucore:source>\n");
                if (!startCount)
                    ToReturn+=__T("\t\t\t\t-->\n");
            }
            ToReturn+=__T("\t\t\t</ebucore:start>\n");
            if (startCount)
                ToReturn+=__T("\t\t\t-->\n");
            startCount++;
        }
    }
    
    //format - duration
    if (!MI.Get(Stream_General, 0, General_Duration).empty())
    {
        float64 DurationS=MI.Get(Stream_General, 0, General_Duration).To_float64()/1000;
        int64u DurationH=(int64u)(DurationS/60/60);
        DurationS-=DurationH*60*60;
        int64u DurationM=(int64u)(DurationS/60);
        DurationS-=DurationM*60;
        Ztring Duration;
        if (DurationH)
            Duration+=Ztring::ToZtring(DurationH)+__T('H');
        if (DurationM)
            Duration+=Ztring::ToZtring(DurationM)+__T('M');
        Duration+=Ztring::ToZtring(DurationS, 3)+__T('S');
        ToReturn+=__T("\t\t\t<ebucore:duration>\n");
        ToReturn+=__T("\t\t\t\t<ebucore:normalPlayTime>PT")+Duration+__T("</ebucore:normalPlayTime>\n");
        ToReturn+=__T("\t\t\t</ebucore:duration>\n");
    }

    //format - fileSize
    if (!MI.Get(Stream_General, 0, General_FileSize).empty())
        ToReturn+=__T("\t\t\t<ebucore:fileSize>")+MI.Get(Stream_General, 0, General_FileSize)+__T("</ebucore:fileSize>\n");

    //format - fileName
    if (!MI.Get(Stream_General, 0, General_FileName).empty())
    {
        Ztring Name=MI.Get(Stream_General, 0, General_FileName);
        if (!MI.Get(Stream_General, 0, General_FileExtension).empty())
        {
            Name+=__T('.');
            Name+=MI.Get(Stream_General, 0, General_FileExtension);
        }
        Name.FindAndReplace(__T("&"), __T("&amp;"), 0, Ztring_Recursive);
        Name.FindAndReplace(__T("<"), __T("&lt;"), 0, Ztring_Recursive);
        Name.FindAndReplace(__T(">"), __T("&gt;"), 0, Ztring_Recursive);
        Name.FindAndReplace(__T("\""), __T("&quot;"), 0, Ztring_Recursive);
        Name.FindAndReplace(__T("'"), __T("&apos;"), 0, Ztring_Recursive);
        ToReturn+=__T("\t\t\t<ebucore:fileName>")+Name+__T("</ebucore:fileName>\n");
    }

    //format - locator
    if (!MI.Get(Stream_General, 0, General_CompleteName).empty())
    {
        Ztring Name=MI.Get(Stream_General, 0, General_CompleteName);
        Name.FindAndReplace(__T("&"), __T("&amp;"), 0, Ztring_Recursive);
        Name.FindAndReplace(__T("<"), __T("&lt;"), 0, Ztring_Recursive);
        Name.FindAndReplace(__T(">"), __T("&gt;"), 0, Ztring_Recursive);
        Name.FindAndReplace(__T("\""), __T("&quot;"), 0, Ztring_Recursive);
        Name.FindAndReplace(__T("'"), __T("&apos;"), 0, Ztring_Recursive);
        ToReturn+=__T("\t\t\t<ebucore:locator>")+Name+__T("</ebucore:locator>\n");
    }

    //format - technicalAttributeString - overallBitRate
    if (!MI.Get(Stream_General, 0, General_OverallBitRate).empty())
    {
        ToReturn+=__T("\t\t\t<ebucore:technicalAttributeString typeLabel=\"overallBitRate\">\">")+MI.Get(Stream_General, 0, General_OverallBitRate)+__T("</ebucore:technicalAttributeString>\n");
    }

    //format - dateCreated
    if (!MI.Get(Stream_General, 0, General_Encoded_Date).empty())
    {
        Ztring DateTime=MI.Get(Stream_General, 0, General_Encoded_Date);
        DateTime.FindAndReplace(__T("UTC "), __T(""));
        DateTime.FindAndReplace(__T(" "), __T("T"));
        Ztring Date=DateTime.substr(0, 10);
        Ztring Time=DateTime.substr(11, 8);

        ToReturn+=__T("\t\t\t<ebucore:dateCreated startDate=\"")+Date+__T("\" startTime=\"")+Time+__T("Z\"/>\n");
    }

    //format - dateModified
    if (!MI.Get(Stream_General, 0, General_Tagged_Date).empty())
    {
        Ztring DateTime=MI.Get(Stream_General, 0, General_Tagged_Date);
        DateTime.FindAndReplace(__T("UTC "), __T(""));
        DateTime.FindAndReplace(__T(" "), __T("T"));
        Ztring Date=DateTime.substr(0, 10);
        Ztring Time=DateTime.substr(11, 8);

        ToReturn+=__T("\t\t\t<ebucore:dateModified startDate=\"")+Date+__T("\" startTime=\"")+Time+__T("Z\"/>\n");
    }

    //format
    ToReturn+=__T("\t\t</ebucore:format>\n");

    //coreMetadata
    ToReturn+=__T("\t</ebucore:coreMetadata>\n");

    //ebuCoreMain
    ToReturn+=__T("</ebucore:ebuCoreMain>\n");

    //Carriage return
    ToReturn.FindAndReplace(__T("\n"), EOL, 0, Ztring_Recursive);

    return ToReturn;
}

//***************************************************************************
//
//***************************************************************************

} //NameSpace
