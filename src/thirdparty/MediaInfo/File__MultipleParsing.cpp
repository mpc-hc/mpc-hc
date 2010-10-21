// File__MultipleParsing - Info for MultipleParsing files
// Copyright (C) 2007-2010 MediaArea.net SARL, Info@MediaArea.net
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
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__MultipleParsing.h"
//---------------------------------------------------------------------------
// Multiple
#if defined(MEDIAINFO_CDXA_YES)
    #include "MediaInfo/Multiple/File_Cdxa.h"
#endif
#if defined(MEDIAINFO_DPG_YES)
    #include "MediaInfo/Multiple/File_Dpg.h"
#endif
#if defined(MEDIAINFO_DVDIF_YES)
    #include "MediaInfo/Multiple/File_DvDif.h"
#endif
#if defined(MEDIAINFO_DVDV_YES)
    #include "MediaInfo/Multiple/File_Dvdv.h"
#endif
#if defined(MEDIAINFO_FLV_YES)
    #include "MediaInfo/Multiple/File_Flv.h"
#endif
#if defined(MEDIAINFO_GXF_YES)
    #include "MediaInfo/Multiple/File_Gxf.h"
#endif
#if defined(MEDIAINFO_IVF_YES)
    #include "MediaInfo/Multiple/File_Ivf.h"
#endif
#if defined(MEDIAINFO_LXF_YES)
    #include "MediaInfo/Multiple/File_Lxf.h"
#endif
#if defined(MEDIAINFO_MK_YES)
    #include "MediaInfo/Multiple/File_Mk.h"
#endif
#if defined(MEDIAINFO_MPEG4_YES)
    #include "MediaInfo/Multiple/File_Mpeg4.h"
#endif
#if defined(MEDIAINFO_MPEGPS_YES)
    #include "MediaInfo/Multiple/File_MpegPs.h"
#endif
#if defined(MEDIAINFO_MPEGTS_YES) || defined(MEDIAINFO_BDAV_YES) || defined(MEDIAINFO_TSP_YES)
    #include "MediaInfo/Multiple/File_MpegTs.h"
#endif
#if defined(MEDIAINFO_MXF_YES)
    #include "MediaInfo/Multiple/File_Mxf.h"
#endif
#if defined(MEDIAINFO_NUT_YES)
    #include "MediaInfo/Multiple/File_Nut.h"
#endif
#if defined(MEDIAINFO_OGG_YES)
    #include "MediaInfo/Multiple/File_Ogg.h"
#endif
#if defined(MEDIAINFO_P2_YES)
    #include "MediaInfo/Multiple/File_P2_Clip.h"
#endif
#if defined(MEDIAINFO_RIFF_YES)
    #include "MediaInfo/Multiple/File_Riff.h"
#endif
#if defined(MEDIAINFO_RM_YES)
    #include "MediaInfo/Multiple/File_Rm.h"
#endif
#if defined(MEDIAINFO_SKM_YES)
    #include "MediaInfo/Multiple/File_Skm.h"
#endif
#if defined(MEDIAINFO_SWF_YES)
    #include "MediaInfo/Multiple/File_Swf.h"
#endif
#if defined(MEDIAINFO_WM_YES)
    #include "MediaInfo/Multiple/File_Wm.h"
#endif
#if defined(MEDIAINFO_XDCAM_YES)
    #include "MediaInfo/Multiple/File_Xdcam_Clip.h"
#endif

//---------------------------------------------------------------------------
// Video
#if defined(MEDIAINFO_AVC_YES)
    #include "MediaInfo/Video/File_Avc.h"
#endif
#if defined(MEDIAINFO_AVSV_YES)
    #include "MediaInfo/Video/File_AvsV.h"
#endif
#if defined(MEDIAINFO_DIRAC_YES)
    #include "MediaInfo/Video/File_Dirac.h"
#endif
#if defined(MEDIAINFO_FLIC_YES)
    #include "MediaInfo/Video/File_Flic.h"
#endif
#if defined(MEDIAINFO_MPEG4V_YES)
    #include "MediaInfo/Video/File_Mpeg4v.h"
#endif
#if defined(MEDIAINFO_MPEGV_YES)
    #include "MediaInfo/Video/File_Mpegv.h"
#endif
#if defined(MEDIAINFO_VC1_YES)
    #include "MediaInfo/Video/File_Vc1.h"
#endif

//---------------------------------------------------------------------------
// Audio
#if defined(MEDIAINFO_AC3_YES)
    #include "MediaInfo/Audio/File_Ac3.h"
#endif
#if defined(MEDIAINFO_ADIF_YES)
    #include "MediaInfo/Audio/File_Adif.h"
#endif
#if defined(MEDIAINFO_ADTS_YES)
    #include "MediaInfo/Audio/File_Adts.h"
#endif
#if defined(MEDIAINFO_ALS_YES)
    #include "MediaInfo/Audio/File_Als.h"
#endif
#if defined(MEDIAINFO_AMR_YES)
    #include "MediaInfo/Audio/File_Amr.h"
#endif
#if defined(MEDIAINFO_AMV_YES)
    #include "MediaInfo/Audio/File_Amv.h"
#endif
#if defined(MEDIAINFO_APE_YES)
    #include "MediaInfo/Audio/File_Ape.h"
#endif
#if defined(MEDIAINFO_AU_YES)
    #include "MediaInfo/Audio/File_Au.h"
#endif
#if defined(MEDIAINFO_DTS_YES)
    #include "MediaInfo/Audio/File_Dts.h"
#endif
#if defined(MEDIAINFO_XM_YES)
    #include "MediaInfo/Audio/File_ExtendedModule.h"
#endif
#if defined(MEDIAINFO_FLAC_YES)
    #include "MediaInfo/Audio/File_Flac.h"
#endif
#if defined(MEDIAINFO_IT_YES)
    #include "MediaInfo/Audio/File_ImpulseTracker.h"
#endif
#if defined(MEDIAINFO_LA_YES)
    #include "MediaInfo/Audio/File_La.h"
#endif
#if defined(MEDIAINFO_MIDI_YES)
    #include "MediaInfo/Audio/File_Midi.h"
#endif
#if defined(MEDIAINFO_MOD_YES)
    #include "MediaInfo/Audio/File_Module.h"
#endif
#if defined(MEDIAINFO_MPC_YES)
    #include "MediaInfo/Audio/File_Mpc.h"
#endif
#if defined(MEDIAINFO_MPCSV8_YES)
    #include "MediaInfo/Audio/File_MpcSv8.h"
#endif
#if defined(MEDIAINFO_MPEGA_YES)
    #include "MediaInfo/Audio/File_Mpega.h"
#endif
#if defined(MEDIAINFO_PCM_YES)
    #include "MediaInfo/Audio/File_Pcm.h"
#endif
#if defined(MEDIAINFO_S3M_YES)
    #include "MediaInfo/Audio/File_ScreamTracker3.h"
#endif
#if defined(MEDIAINFO_TAK_YES)
    #include "MediaInfo/Audio/File_Tak.h"
#endif
#if defined(MEDIAINFO_TTA_YES)
    #include "MediaInfo/Audio/File_Tta.h"
#endif
#if defined(MEDIAINFO_TWINVQ_YES)
    #include "MediaInfo/Audio/File_TwinVQ.h"
#endif
#if defined(MEDIAINFO_WVPK_YES)
    #include "MediaInfo/Audio/File_Wvpk.h"
#endif

//---------------------------------------------------------------------------
// Text
#if defined(MEDIAINFO_N19_YES)
    #include "MediaInfo/Text/File_N19.h"
#endif
#if defined(MEDIAINFO_OTHERTEXT_YES)
    #include "MediaInfo/Text/File_OtherText.h"
#endif

//---------------------------------------------------------------------------
// Image
#if defined(MEDIAINFO_BMP_YES)
    #include "MediaInfo/Image/File_Bmp.h"
#endif
#if defined(MEDIAINFO_GIF_YES)
    #include "MediaInfo/Image/File_Gif.h"
#endif
#if defined(MEDIAINFO_ICO_YES)
    #include "MediaInfo/Image/File_Ico.h"
#endif
#if defined(MEDIAINFO_JPEG_YES)
    #include "MediaInfo/Image/File_Jpeg.h"
#endif
#if defined(MEDIAINFO_PNG_YES)
    #include "MediaInfo/Image/File_Png.h"
#endif
#if defined(MEDIAINFO_TIFF_YES)
    #include "MediaInfo/Image/File_Tiff.h"
#endif

//---------------------------------------------------------------------------
// Archive
#if defined(MEDIAINFO_7Z_YES)
    #include "MediaInfo/Archive/File_7z.h"
#endif
#if defined(MEDIAINFO_ACE_YES)
    #include "MediaInfo/Archive/File_Ace.h"
#endif
#if defined(MEDIAINFO_BZIP2_YES)
    #include "MediaInfo/Archive/File_Bzip2.h"
#endif
#if defined(MEDIAINFO_ELF_YES)
    #include "MediaInfo/Archive/File_Elf.h"
#endif
#if defined(MEDIAINFO_GZIP_YES)
    #include "MediaInfo/Archive/File_Gzip.h"
#endif
#if defined(MEDIAINFO_MZ_YES)
    #include "MediaInfo/Archive/File_Mz.h"
#endif
#if defined(MEDIAINFO_RAR_YES)
    #include "MediaInfo/Archive/File_Rar.h"
#endif
#if defined(MEDIAINFO_TAR_YES)
    #include "MediaInfo/Archive/File_Tar.h"
#endif
#if defined(MEDIAINFO_ZIP_YES)
    #include "MediaInfo/Archive/File_Zip.h"
#endif

//---------------------------------------------------------------------------
// Other
#if !defined(MEDIAINFO_OTHER_NO)
    #include "MediaInfo/File_Other.h"
#endif
#include "MediaInfo/File_Unknown.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Out
//***************************************************************************

//---------------------------------------------------------------------------
File__Analyze* File__MultipleParsing::Parser_Get()
{
    if (Parser.size()!=1)
        return NULL;

    File__Analyze* ToReturn=Parser[0]; //The first parser
    Parser.clear();
    return ToReturn;
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File__MultipleParsing::File__MultipleParsing()
:File__Analyze()
{
    #if MEDIAINFO_TRACE
        Trace_DoNotSave=true;
    #endif //MEDIAINFO_TRACE

    File__Analyze* Temp;
    // Multiple
    #if defined(MEDIAINFO_BDAV_YES)
        Temp=new File_MpegTs(); ((File_MpegTs*)Temp)->BDAV_Size=4; Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_CDXA_YES)
        Temp=new File_Cdxa(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_DPG_YES)
        Temp=new File_Dpg(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_DVDIF_YES)
        Temp=new File_DvDif(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_DVDV_YES)
        Temp=new File_Dvdv(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_FLV_YES)
        Temp=new File_Flv(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_GXF_YES)
        Temp=new File_Gxf(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_IVF_YES)
        Temp=new File_Ivf(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_LXF_YES)
        Temp=new File_Lxf(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_MK_YES)
        Temp=new File_Mk(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_MPEG4_YES)
        Temp=new File_Mpeg4(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_MPEGPS_YES)
        Temp=new File_MpegPs(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_MPEGTS_YES)
        Temp=new File_MpegTs(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_TSP_YES)
        Temp=new File_MpegTs(); ((File_MpegTs*)Temp)->TSP_Size=16; Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_MXF_YES)
        Temp=new File_Mxf(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_NUT_YES)
        Temp=new File_Nut(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_OGG_YES)
        Temp=new File_Ogg(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_P2_YES)
        Temp=new File_P2_Clip(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_RIFF_YES)
        Temp=new File_Riff(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_RM_YES)
        Temp=new File_Rm(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_SKM_YES)
        Temp=new File_Skm(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_SWF_YES)
        Temp=new File_Swf(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_WM_YES)
        Temp=new File_Wm(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_XDCAM_YES)
        Temp=new File_Xdcam_Clip(); Parser.push_back(Temp);
    #endif

    // Video
    #if defined(MEDIAINFO_AVC_YES)
        Temp=new File_Avc(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_AVSV_YES)
        Temp=new File_AvsV(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_DIRAC_YES)
        Temp=new File_Dirac(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_FLIC_YES)
        Temp=new File_Flic(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_MPEG4V_YES)
        Temp=new File_Mpeg4v(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_MPEGV_YES)
        Temp=new File_Mpegv(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_VC1_YES)
        Temp=new File_Vc1(); Parser.push_back(Temp);
    #endif

    // Audio
    #if defined(MEDIAINFO_AC3_YES)
        Temp=new File_Ac3(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_ADIF_YES)
        Temp=new File_Adif(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_ADTS_YES)
        Temp=new File_Adts(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_ALS_YES)
        Temp=new File_Als(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_AMR_YES)
        Temp=new File_Amr(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_AMV_YES)
        Temp=new File_Amv(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_APE_YES)
        Temp=new File_Ape(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_AU_YES)
        Temp=new File_Au(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_DTS_YES)
        Temp=new File_Dts(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_FLAC_YES)
        Temp=new File_Flac(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_IT_YES)
        Temp=new File_ImpulseTracker(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_LA_YES)
        Temp=new File_La(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_MIDI_YES)
        Temp=new File_Midi(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_MOD_YES)
        Temp=new File_Module(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_MPC_YES)
        Temp=new File_Mpc(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_MPCSV8_YES)
        Temp=new File_MpcSv8(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_MPEGA_YES)
        Temp=new File_Mpega(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_S3M_YES)
        Temp=new File_ScreamTracker3(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_TAK_YES)
        Temp=new File_Tak(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_TTA_YES)
        Temp=new File_Tta(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_TWINVQ_YES)
        Temp=new File_TwinVQ(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_WVPK_YES)
        Temp=new File_Wvpk(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_XM_YES)
        Temp=new File_ExtendedModule(); Parser.push_back(Temp);
    #endif

    // Text
    #if defined(MEDIAINFO_N19_YES)
        Temp=new File_N19(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_OTHERTEXT_YES)
        Temp=new File_OtherText(); Parser.push_back(Temp);
    #endif

    // Image
    #if defined(MEDIAINFO_BMP_YES)
        Temp=new File_Bmp(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_GIF_YES)
        Temp=new File_Gif(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_ICO_YES)
        Temp=new File_Ico(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_JPEG_YES)
        Temp=new File_Jpeg(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_PNG_YES)
        Temp=new File_Png(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_TIFF_YES)
        Temp=new File_Tiff(); Parser.push_back(Temp);
    #endif

    // Archive
    #if defined(MEDIAINFO_7Z_YES)
        Temp=new File_7z(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_ACE_YES)
        Temp=new File_Ace(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_BZIP2_YES)
        Temp=new File_Bzip2(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_ELF_YES)
        Temp=new File_Elf(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_GZIP_YES)
        Temp=new File_Gzip(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_MZ_YES)
        Temp=new File_Mz(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_RAR_YES)
        Temp=new File_Rar(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_TAR_YES)
        Temp=new File_Tar(); Parser.push_back(Temp);
    #endif
    #if defined(MEDIAINFO_ZIP_YES)
        Temp=new File_Zip(); Parser.push_back(Temp);
    #endif

    #if defined(MEDIAINFO_OTHER_YES)
        Temp=new File_Other(); Parser.push_back(Temp);
    #endif
}

//---------------------------------------------------------------------------
File__MultipleParsing::~File__MultipleParsing()
{
    for (size_t Pos=0; Pos<Parser.size(); Pos++)
        delete Parser[Pos]; //Parser[Pos]=NULL
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File__MultipleParsing::Streams_Finish()
{
    if (Parser.size()!=1)
        return;

    Parser[0]->Open_Buffer_Finalize();
    #if MEDIAINFO_TRACE
        Details=Parser[0]->Details;
    #endif //MEDIAINFO_TRACE
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File__MultipleParsing::Read_Buffer_Init()
{
    //Parsing
    for (size_t Pos=0; Pos<Parser.size(); Pos++)
    {
        //Parsing
        #if MEDIAINFO_TRACE
            Parser[Pos]->Init(Config, Details, Stream, Stream_More);
        #else //MEDIAINFO_TRACE
            Parser[Pos]->Init(Config, Stream, Stream_More);
        #endif //MEDIAINFO_TRACE
        Parser[Pos]->File_Name=File_Name;
        Parser[Pos]->Open_Buffer_Init(File_Size);
    }
}

//---------------------------------------------------------------------------
void File__MultipleParsing::Read_Buffer_Unsynched()
{
    //Parsing
    for (size_t Pos=0; Pos<Parser.size(); Pos++)
        Parser[Pos]->Open_Buffer_Unsynch();
}

//---------------------------------------------------------------------------
void File__MultipleParsing::Read_Buffer_Continue()
{
    //Parsing
    for (size_t Pos=0; Pos<Parser.size(); Pos++)
    {
        //Parsing
        Open_Buffer_Continue(Parser[Pos], Buffer+Buffer_Offset, (size_t)Element_Size);
        if (File_Offset+Buffer_Size==File_Size)
            Open_Buffer_Finalize(Parser[Pos]);

        //Testing if the parser failed
        if (Parser[Pos]->Status[IsFinished] && !Parser[Pos]->Status[IsAccepted])
        {
            delete Parser[Pos];
            Parser.erase(Parser.begin()+Pos);
            Pos--; //for the next position

            if (Parser.empty())
            {
                File__Analyze* Temp=new File_Unknown(); Parser.push_back(Temp);
                Read_Buffer_Init();
            }
        }
        else
        {
            //If Parser is found, erasing all the other parsers
            if (Parser.size()>1 && Parser[Pos]->Status[IsAccepted])
            {
                File__Analyze* Temp=Parser[Pos];
                for (size_t To_Delete_Pos=0; To_Delete_Pos<Parser.size(); To_Delete_Pos++)
                    if (To_Delete_Pos!=Pos)
                        delete Parser[To_Delete_Pos]; //Parser[Pos]=NULL
                Parser.clear();
                Parser.push_back(Temp);
                Pos=0;
            }

            if (Parser.size()==1)
            {
                //Status
                if (!Status[IsAccepted] && Parser[Pos]->Status[IsAccepted])
                    Status[IsAccepted]=true;
                if (!Status[IsFilled] && Parser[Pos]->Status[IsFilled])
                    Status[IsFilled]=true;
                if (!Status[IsUpdated] && Parser[Pos]->Status[IsUpdated])
                    Status[IsUpdated]=true;
                if (!Status[IsFinished] && Parser[Pos]->Status[IsFinished])
                    Status[IsFinished]=true;

                //Positionning if requested
                if (Parser[0]->File_GoTo!=(int64u)-1)
                   File_GoTo=Parser[0]->File_GoTo;
            }
        }
    }
}

} //NameSpace

