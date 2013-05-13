/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Global configuration of MediaInfo
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_Internal_ConstH
#define MediaInfo_Internal_ConstH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfo_Const.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
/// @brief Used with Format info
enum infoformat_t
{
    InfoFormat_Name,                ///<
    InfoFormat_LongName,            ///<
    InfoFormat_Family,              ///<
    InfoFormat_KindofFormat,        ///<
    InfoFormat_Parser,              ///<
    InfoFormat_Info,                ///<
    InfoFormat_Extensions,          ///<
    InfoFormat_InternetMediaType,   ///<
    InfoFormat_Url,                 ///<
    InfoFormat_Compression_Mode,    ///<
    InfoFormat_Max
};

/// @brief Used with Codec ID info
enum infocodecid_t
{
    InfoCodecID_Codec,              ///<
    InfoCodecID_Format,             ///<
    InfoCodecID_Hint,               ///<
    InfoCodecID_Description,        ///<
    InfoCodecID_Url,                ///<
    InfoCodecID_Profile,            ///<
    InfoCodecID_Version,            ///<
    InfoCodecID_ColorSpace,         ///<
    InfoCodecID_ChromaSubsampling,  ///<
    InfoCodecID_BitDepth,           ///<
    InfoCodecID_Compression_Mode,   ///<
    InfoCodecID_Max
};

/// @brief Used with Codec ID info (Format type part)
enum infocodecid_format_t
{
    InfoCodecID_Format_Matroska,    ///<
    InfoCodecID_Format_Mpeg4,       ///<
    InfoCodecID_Format_Real,        ///<
    InfoCodecID_Format_Riff,        ///<
    InfoCodecID_Format_Max
};

/// @brief Used with Codec info
enum infocodec_t
{
    InfoCodec_Codec,                ///<
    InfoCodec_Name,                 ///<
    InfoCodec_KindOfCode,           ///<
    InfoCodec_KindOfStream,         ///<
    InfoCodec_KindofCodec,          ///<
    InfoCodec_BitRate_Mode,         ///<
    InfoCodec_Description,          ///<
    InfoCodec_Url,                  ///<
    InfoCodec_Max
};

/// @brief Used with Encoder info
enum infoencoder_t
{
    InfoEncoder_Name,               ///<
    InfoEncoder_LongName,           ///<
    InfoEncoder_Date,               ///<
    InfoEncoder_Max
};

/// @brief Used with Library info
enum infolibrary_t
{
    InfoLibrary_Numlber,            ///<
    InfoLibrary_Version,            ///<
    InfoLibrary_Date,               ///<
    InfoLibrary_Max
};

/// @brief Used with Library info (Format type part)
enum infolibrary_format_t
{
    InfoLibrary_Format_DivX,        ///<
    InfoLibrary_Format_XviD,        ///<
    InfoLibrary_Format_MainConcept_Avc, ///<
    InfoLibrary_Format_VorbisCom,   ///<
    InfoLibrary_Format_Max
};

/// @brief Used by BlockMethod
enum blockmethod_t
{
    BlockMethod_Now,                ///< Return now, without parsing (init only)
    BlockMethod_Often,              ///< Return as often as possible
    BlockMethod_Local,              ///< Return after local parsing (no Internet connection)
    BlockMethod_Needed,             ///< Return when a user interaction is needed
    BlockMethod_Max
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//Char types
#if defined(UNICODE) || defined (_UNICODE)
    typedef wchar_t Char;
    #undef  __T
    #define __T(__x) L ## __x
#else
    typedef char Char;
    #undef  __T
    #define __T(__x) __x
#endif
//---------------------------------------------------------------------------

} //NameSpace

#endif
