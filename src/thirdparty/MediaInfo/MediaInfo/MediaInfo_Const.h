// MediaInfo_Config - Configuration class
// Copyright (C) 2005-2011 Jerome Martinez, Zen@MediaArea.net
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
//
// Global configuration of MediaInfo
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_ConstH
#define MediaInfo_ConstH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include <string>
#include <ZenLib/Conf.h>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
/// \mainpage MediaInfoLib Index Page
///
/// \section Interfaces
///
/// There is 2 access methods
///
/// - MediaInfo class \n
/// To manage one file, this is the simplest interface. \n
///  - MediaInfo::Open to analyse file \n
///  - MediaInfo::Inform to have a summary \n
///  - MediaInfo::Get to retreive one piece of information \n
///
/// - MediaInfoList class \n
/// To manage a list of files \n
///  - MediaInfoList::Open to analyse file \n
///  - MediaInfoList::Inform to have a summary \n
///  - MediaInfoList::Get to retreive one piece of information \n
///  - MediaInfoList::Close to close one file \n
///
/// \section C C Interface (MediaInfo_*)
/// For compatibility and DLL interface \n
/// This is a C interface for the List class \n
/// Note : Don't forget to include the MediaInfoDLL.h file in your source file! \n
/// - Example of commands:
///  - MediaInfo_Open to analyse file \n
///  - MediaInfo_Inform to have a summary \n
///  - MediaInfo_Get to retreive one piece of information \n
///  - MediaInfo_Close to free memory \n
///
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
/// @brief Kinds of Stream
enum stream_t
{
    Stream_General,                 ///< StreamKind = General
    Stream_Video,                   ///< StreamKind = Video
    Stream_Audio,                   ///< StreamKind = Audio
    Stream_Text,                    ///< StreamKind = Text
    Stream_Chapters,                ///< StreamKind = Chapters
    Stream_Image,                   ///< StreamKind = Image
    Stream_Menu,                    ///< StreamKind = Menu
    Stream_Max
};

/// @brief Kind of information
enum info_t
{
    Info_Name,                      ///< InfoKind = Unique name of parameter
    Info_Text,                      ///< InfoKind = Value of parameter
    Info_Measure,                   ///< InfoKind = Unique name of measure unit of parameter
    Info_Options,                   ///< InfoKind = See infooptions_t
    Info_Name_Text,                 ///< InfoKind = Translated name of parameter
    Info_Measure_Text,              ///< InfoKind = Translated name of measure unit
    Info_Info,                      ///< InfoKind = More information about the parameter
    Info_HowTo,                     ///< InfoKind = How this parameter is supported, could be N (No), B (Beta), R (Read only), W (Read/Write)
    Info_Domain,                    ///< InfoKind = Domain of this piece of information
    Info_Max
};

/// Get(...)[infooptions_t] return a string like "YNYN..." \n
/// Use this enum to know at what correspond the Y (Yes) or N (No)
/// If Get(...)[0]==Y, then :
/// @brief Option if InfoKind = Info_Options
enum infooptions_t
{
    InfoOption_ShowInInform,        ///< Show this parameter in Inform()
    InfoOption_Reserved,            ///<
    InfoOption_ShowInSupported,     ///< Internal use only (info : Must be showed in Info_Capacities() )
    InfoOption_TypeOfValue,         ///< Value return by a standard Get() can be : T (Text), I (Integer, warning up to 64 bits), F (Float), D (Date), B (Binary datas coded Base64) (Numbers are in Base 10)
    InfoOption_Max
};

/// @brief File opening options
enum fileoptions_t
{
    FileOption_Nothing      =0x00,
    FileOption_NoRecursive  =0x01,  ///< Do not browse folders recursively
    FileOption_CloseAll     =0x02,  ///< Close all files before open
    FileOption_Max          =0x04
};

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//Char types
#undef  _T
#define _T(__x)     __T(__x)
#if defined(UNICODE) || defined (_UNICODE)
    typedef wchar_t Char;
    #undef  __T
    #define __T(__x) L ## __x
#else
    typedef char Char;
    #undef  __T
    #define __T(__x) __x
#endif
typedef std::basic_string<MediaInfoLib::Char>        String;
typedef std::basic_stringstream<MediaInfoLib::Char>  StringStream;
typedef std::basic_istringstream<MediaInfoLib::Char> tiStringStream;
typedef std::basic_ostringstream<MediaInfoLib::Char> toStringStream;
//---------------------------------------------------------------------------

} //NameSpace

#endif
