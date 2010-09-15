// MediaInfo_Config - Configuration class
// Copyright (C) 2005-2010 MediaArea.net SARL, Info@MediaArea.net
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
//
// Global configuration of MediaInfo
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_ConfigH
#define MediaInfo_ConfigH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfo_Internal_Const.h"
#include "ZenLib/CriticalSection.h"
#include "ZenLib/ZtringListList.h"
#include "ZenLib/Translation.h"
#include "ZenLib/InfoMap.h"
#include <map>
#include <vector>
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class MediaInfo_Config
//***************************************************************************

class MediaInfo_Config
{
public :
    //Constructor/Destructor
    void Init(); //Must be called instead of constructor

    //General
    Ztring Option (const String &Option, const String &Value=Ztring());

    //Info
          void      Complete_Set (size_t NewValue);
          size_t    Complete_Get ();

          void      BlockMethod_Set (size_t NewValue);
          size_t    BlockMethod_Get ();

          void      Internet_Set (size_t NewValue);
          size_t    Internet_Get ();

          void      MultipleValues_Set (size_t NewValue);
          size_t    MultipleValues_Get ();

          void      ParseUnknownExtensions_Set (size_t NewValue);
          size_t    ParseUnknownExtensions_Get ();

          void      ShowFiles_Set (const ZtringListList &NewShowFiles);
          size_t    ShowFiles_Nothing_Get ();
          size_t    ShowFiles_VideoAudio_Get ();
          size_t    ShowFiles_VideoOnly_Get ();
          size_t    ShowFiles_AudioOnly_Get ();
          size_t    ShowFiles_TextOnly_Get ();

          void      ReadByHuman_Set (bool NewValue);
          bool      ReadByHuman_Get ();

          void      ParseSpeed_Set (float32 NewValue);
          float32   ParseSpeed_Get ();

          void      Verbosity_Set (float32 NewValue);
          float32   Verbosity_Get ();

          void      DetailsLevel_Set (float32 NewValue);
          float32   DetailsLevel_Get ();

          enum detailsFormat
          {
              DetailsFormat_Tree,
              DetailsFormat_CSV,
          };
          void      DetailsFormat_Set (detailsFormat NewValue);
          detailsFormat DetailsFormat_Get ();

          void      DetailsModificator_Set (const ZtringList &NewModifcator);
          Ztring    DetailsModificator_Get (const Ztring &Modificator);

          void      Demux_Set (int8u NewValue);
          int8u     Demux_Get ();

          void      LineSeparator_Set (const Ztring &NewValue);
          Ztring    LineSeparator_Get ();

          void      Version_Set (const Ztring &NewValue);
          Ztring    Version_Get ();

          void      ColumnSeparator_Set (const Ztring &NewValue);
          Ztring    ColumnSeparator_Get ();

          void      TagSeparator_Set (const Ztring &NewValue);
          Ztring    TagSeparator_Get ();

          void      Quote_Set (const Ztring &NewValue);
          Ztring    Quote_Get ();

          void      DecimalPoint_Set (const Ztring &NewValue);
          Ztring    DecimalPoint_Get ();

          void      ThousandsPoint_Set (const Ztring &NewValue);
          Ztring    ThousandsPoint_Get ();

          void      StreamMax_Set (const ZtringListList &NewValue);
          Ztring    StreamMax_Get ();

          void      Language_Set (const ZtringListList &NewLanguage);
          Ztring    Language_Get ();
          Ztring    Language_Get (const Ztring &Value);
          Ztring    Language_Get (const Ztring &Count, const Ztring &Value, bool ValueIsAlwaysSame=false);

          void      Inform_Set (const ZtringListList &NewInform);
          Ztring    Inform_Get ();
          Ztring    Inform_Get (const Ztring &Value);

          void      Inform_Replace_Set (const ZtringListList &NewInform_Replace);
          Ztring    Inform_Replace_Get ();
          ZtringListList Inform_Replace_Get_All ();

    const Ztring   &Format_Get (const Ztring &Value, infoformat_t KindOfFormatInfo=InfoFormat_Name);
          InfoMap  &Format_Get(); //Should not be, but too difficult to hide it

    const Ztring   &Codec_Get (const Ztring &Value, infocodec_t KindOfCodecInfo=InfoCodec_Name);
    const Ztring   &Codec_Get (const Ztring &Value, infocodec_t KindOfCodecInfo, stream_t KindOfStream);

    const Ztring   &CodecID_Get (stream_t KindOfStream, infocodecid_format_t Format, const Ztring &Value, infocodecid_t KindOfCodecIDInfo=InfoCodecID_Format);

    const Ztring   &Library_Get (infolibrary_format_t Format, const Ztring &Value, infolibrary_t KindOfLibraryInfo=InfoLibrary_Version);

    const Ztring   &Iso639_1_Get (const Ztring &Value);
    const Ztring   &Iso639_2_Get (const Ztring &Value);
    const Ztring    Iso639_Find (const Ztring &Value);

    const Ztring   &Info_Get (stream_t KindOfStream, const Ztring &Value, info_t KindOfInfo=Info_Text);
    const Ztring   &Info_Get (stream_t KindOfStream, size_t Pos, info_t KindOfInfo=Info_Text);
    const ZtringListList &Info_Get(stream_t KindOfStream); //Should not be, but too difficult to hide it

          Ztring    Info_Parameters_Get ();
          Ztring    Info_Tags_Get       () const;
          Ztring    Info_CodecsID_Get   ();
          Ztring    Info_Codecs_Get     ();
          Ztring    Info_Version_Get    () const;
          Ztring    Info_Url_Get        () const;

    const Ztring   &EmptyString_Get() const; //Use it when we can't return a reference to a true string
    const ZtringListList &EmptyStringListList_Get() const; //Use it when we can't return a reference to a true string list list

          void      FormatDetection_MaximumOffset_Set (int64u Value);
          int64u    FormatDetection_MaximumOffset_Get ();

          void      MpegTs_MaximumOffset_Set (int64u Value);
          int64u    MpegTs_MaximumOffset_Get ();

private :
    int64u          FormatDetection_MaximumOffset;
    int64u          MpegTs_MaximumOffset;
    size_t          Complete;
    size_t          BlockMethod;
    size_t          Internet;
    size_t          MultipleValues;
    size_t          ParseUnknownExtensions;
    size_t          ShowFiles_Nothing;
    size_t          ShowFiles_VideoAudio;
    size_t          ShowFiles_VideoOnly;
    size_t          ShowFiles_AudioOnly;
    size_t          ShowFiles_TextOnly;
    float32         ParseSpeed;
    float32         Verbosity;
    float32         DetailsLevel;
    bool            Language_Raw;
    bool            ReadByHuman;
    int8u           Demux;
    Ztring          Version;
    Ztring          ColumnSeparator;
    Ztring          LineSeparator;
    Ztring          TagSeparator;
    Ztring          Quote;
    Ztring          DecimalPoint;
    Ztring          ThousandsPoint;
    Translation     Language; //ex. : "KB;Ko"
    ZtringListList  Custom_View; //Definition of "General", "Video", "Audio", "Text", "Chapters", "Image"
    ZtringListList  Custom_View_Replace; //ToReplace;ReplaceBy
    detailsFormat   DetailsFormat;
    std::map<Ztring, bool> DetailsModificators; //If we want to add/remove some details

    InfoMap         Container;
    InfoMap         CodecID[InfoCodecID_Format_Max][Stream_Max];
    InfoMap         Format;
    InfoMap         Codec;
    InfoMap         Library[InfoLibrary_Format_Max];
    InfoMap         Iso639_1;
    InfoMap         Iso639_2;
    ZtringListList  Info[Stream_Max]; //General info

    ZenLib::CriticalSection CS;

    void      Language_Set (stream_t StreamKind);
};

extern MediaInfo_Config Config;

} //NameSpace

#endif
