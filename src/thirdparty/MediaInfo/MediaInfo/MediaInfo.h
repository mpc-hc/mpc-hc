// MediaInfo - All information about media files
// Copyright (C) 2002-2011 MediaArea.net SARL, Info@MediaArea.net
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
// Give information about a lot of media files
// Dispatch the file to be tested by all containers
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfoH
#define MediaInfoH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfo_Const.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#undef MEDIAINFO_EXP
#if defined(_WIN32) && !defined(__MINGW32__) //MinGW32 does not support _declspec
    #ifdef MEDIAINFO_DLL_EXPORT
        #define MEDIAINFO_EXP
    #else
        #define MEDIAINFO_EXP
    #endif
#else //defined(_WIN32) && !defined(__MINGW32__)
    #if __GNUC__ >= 4
        #define MEDIAINFO_EXP __attribute__ ((visibility("default")))
    #else
        #define MEDIAINFO_EXP
    #endif
#endif //defined(_WIN32) && !defined(__MINGW32__)

#if !defined(__WINDOWS__)
    #define __stdcall //Supported only on windows
#endif //!defined(_WIN32)
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

class File__Analyze;
class Internet__Base;

//***************************************************************************
/// @brief MediaInfo
/// @version 0.7
//***************************************************************************

class MEDIAINFO_EXP MediaInfo
{
public :
    //Constructor/Destructor
    MediaInfo ();
    ~MediaInfo ();
    //File
        /// Open a file and collect information about it (technical information and tags)
        /// @brief Open a file
        /// @param File_Name Full name of file to open
        /// @retval 0 File not opened
        /// @retval 1 File opened
    size_t Open (const String &File_Name);
        /// Open a Buffer (Begin and end of the stream) and collect information about it (technical information and tags)
        /// @brief Open a buffer
        /// @param Begin First bytes of the buffer
        /// @param Begin_Size Size of Begin
        /// @param End Last bytes of the buffer
        /// @param End_Size Size of End
        /// @param File_Size Total size of the file
        /// @retval 0 File not opened
        /// @retval 1 File opened
    size_t Open (const ZenLib::int8u* Begin, size_t Begin_Size, const ZenLib::int8u* End=NULL, size_t End_Size=0, ZenLib::int64u File_Size=0);
        /// Open a stream and collect information about it (technical information and tags)
        /// @brief Open a stream (Init)
        /// @param File_Size Estimated file size
        /// @param File_Offset Offset of the file (if we don't have the beginning of the file)
    size_t Open_Buffer_Init (ZenLib::int64u File_Size=(ZenLib::int64u)-1, ZenLib::int64u File_Offset=0);
        /// Open a stream and collect information about it (technical information and tags)
        /// @brief Open a stream (Continue)
        /// @param Buffer pointer to the stream
        /// @param Buffer_Size Count of bytes to read
        /// @return a bitfield \n
        ///         bit 0: Is Accepted  (format is known)
        ///         bit 1: Is Filled    (main data is collected)
        ///         bit 2: Is Updated   (some data have beed updated, example: duration for a real time MPEG-TS stream)
        ///         bit 3: Is Finalized (No more data is needed, will not use further data)
        ///         bit 4-15: Reserved
        ///         bit 16-31: User defined
    size_t Open_Buffer_Continue (const ZenLib::int8u* Buffer, size_t Buffer_Size);
        /// Open a stream and collect information about it (technical information and tags)
        /// @brief Open a stream (Get the needed file Offset)
        /// @return the needed offset of the file \n
        ///         File size if no more bytes are needed
    ZenLib::int64u Open_Buffer_Continue_GoTo_Get ();
        /// Open a stream and collect information about it (technical information and tags)
        /// @brief Open a stream (Finalize)
    size_t Open_Buffer_Finalize ();
        /// If Open() is used in "PerPacket" mode, parse only one packet and return
        /// @brief Read one packet (if "PerPacket" mode is set)
        /// @return a bitfield \n
        ///         bit 0: A packet was read
    size_t Open_NextPacket ();
        /// (NOT IMPLEMENTED YET) Save the file opened before with Open() (modifications of tags)
        /// @brief (NOT IMPLEMENTED YET) Save the file
        /// @retval 0 failed
        /// @retval 1 suceed
    size_t Save ();
        /// Close a file opened before with Open() (without saving)
        /// @brief Close a file
        /// @warning without have saved before, modifications are lost
    void Close ();

    //General information
        /// Get all details about a file in one string
        /// @brief Get all details about a file
        /// @pre You can change default presentation with Inform_Set()
    String Inform (size_t Reserved=0);

    //Get
        /// Get a piece of information about a file (parameter is an integer)
        /// @brief Get a piece of information about a file (parameter is an integer)
        /// @param StreamKind Kind of stream (general, video, audio...)
        /// @param StreamNumber Stream number in Kind of stream (first, second...)
        /// @param Parameter Parameter you are looking for in the stream (Codec, width, bitrate...), in integer format (first parameter, second parameter...)
        /// @param InfoKind Kind of information you want about the parameter (the text, the measure, the help...)
        /// @return a string about information you search \n
        ///         an empty string if there is a problem
    String Get (stream_t StreamKind, size_t StreamNumber, size_t Parameter, info_t InfoKind=Info_Text);
        /// Get a piece of information about a file (parameter is a string)
        /// @brief Get a piece of information about a file (parameter is a string)
        /// @param StreamKind Kind of stream (general, video, audio...)
        /// @param StreamNumber Stream number in Kind of stream (first, second...)
        /// @param Parameter Parameter you are looking for in the stream (Codec, width, bitrate...), in string format ("Codec", "Width"...) \n
        ///        See MediaInfo::Option("Info_Parameters") to have the full list
        /// @param InfoKind Kind of information you want about the parameter (the text, the measure, the help...)
        /// @param SearchKind Where to look for the parameter
        /// @return a string about information you search \n
        ///         an empty string if there is a problem
    String Get (stream_t StreamKind, size_t StreamNumber, const String &Parameter, info_t InfoKind=Info_Text, info_t SearchKind=Info_Name);

    //Set
        /// (NOT IMPLEMENTED YET) Set a piece of information about a file (parameter is an integer)
        /// @brief (NOT IMPLEMENTED YET) Set a piece of information about a file (parameter is an int)
        /// @warning Not yet implemented, do not use it
        /// @param ToSet Piece of information
        /// @param StreamKind Kind of stream (general, video, audio...)
        /// @param StreamNumber Stream number in Kind of stream (first, second...)
        /// @param Parameter Parameter you are looking for in the stream (Codec, width, bitrate...), in integer format (first parameter, second parameter...)
        /// @param OldValue The old value of the parameter \n if OldValue is empty and ToSet is filled: tag is added \n if OldValue is filled and ToSet is filled: tag is replaced \n if OldValue is filled and ToSet is empty: tag is deleted
        /// @retval >0 succeed
        /// @retval 0 failed
    size_t Set (const String &ToSet, stream_t StreamKind, size_t StreamNumber, size_t Parameter, const String &OldValue=String());
        /// (NOT IMPLEMENTED YET) Set a piece of information about a file (parameter is a string)
        /// @warning Not yet implemented, do not use it
        /// @brief (NOT IMPLEMENTED YET) Set information about a file (parameter is a string)
        /// @param ToSet Piece of information
        /// @param StreamKind Kind of stream (general, video, audio...)
        /// @param StreamNumber Stream number in Kind of stream (first, second...)
        /// @param Parameter Parameter you are looking for in the stream (Codec, width, bitrate...), in string format
        /// @param OldValue The old value of the parameter \n if OldValue is empty and ToSet is filled: tag is added \n if OldValue is filled and ToSet is filled: tag is replaced \n if OldValue is filled and ToSet is empty: tag is deleted
        /// @retval >0 succeed
        /// @retval 0 failed
    size_t Set (const String &ToSet, stream_t StreamKind, size_t StreamNumber, const String &Parameter, const String &OldValue=String());

    //Output_Buffered
        /// Output the written size when "File_Duplicate" option is used.
        /// @brief Output the written size when "File_Duplicate" option is used.
        /// @param Value The unique name of the duplicated stream (begin with "memory://")
        /// @return The size of the used buffer
    size_t Output_Buffer_Get (const String &Value);
        /// Output the written size when "File_Duplicate" option is used.
        /// @brief Output the written size when "File_Duplicate" option is used.
        /// @param Value The order of calling
        /// @return The size of the used buffer
    size_t Output_Buffer_Get (size_t Pos);

    //Info
        /// Configure or get information about MediaInfoLib
        /// @param Option The name of option
        /// @param Value The value of option
        /// @return Depend of the option: by default "" (nothing) means No, other means Yes
        /// @post Known options are: \n
        ///       * (NOT IMPLEMENTED YET) "BlockMethod": Configure when Open Method must return (default or not command not understood: "1") \n
        ///                 "0": Immediatly \n
        ///                 "1": After geting local information \n
        ///                 "2": When user interaction is needed, or whan Internet information is get
        ///       * "Complete": For debug, configure if MediaInfoLib::Inform() show all information (doesn't care of InfoOption_NoShow tag): shows all information if true, shows only useful for user information if false (No by default)\n
        ///       * "Complete_Get": return the state of "Complete" \n
        ///       * "Language": Configure language (default language, and this object); Value is Description of language (format: "Column1;Colum2\n...) \n
        ///                 Column 1: Unique name ("Bytes", "Title") \n
        ///                 Column 2: translation ("Octets", "Titre") \n
        ///       * "Language_Get": Get the language file in memory
        ///       * "Language_Update": Configure language of this object only (for optimisation); Value is Description of language (format: "Column1;Colum2\n...) \n
        ///                 Column 1: Unique name ("Bytes", "Title") \n
        ///                 Column 2: translation ("Octets", "Titre") \n
        ///       * "Inform": Configure custom text, See MediaInfoLib::Inform() function; Description of views (format: "Column1;Colum2...) \n
        ///                 Column 1: code (11 lines: "General", "Video", "Audio", "Text", "Chapters", "Begin", "End", "Page_Begin", "Page_Middle", "Page_End") \n
        ///                 Column 2: The text to show (exemple: "Audio: %FileName% is at %BitRate/String%") \n
        ///       * "ParseUnknownExtensions": Configure if MediaInfo parse files with unknown extension\n
        ///       * "ParseUnknownExtensions_Get": Get if MediaInfo parse files with unknown extension\n
        ///       * "ShowFiles": Configure if MediaInfo keep in memory files with specific kind of streams (or no streams); Value is Description of components (format: "Column1;Colum2\n...) \n\n
        ///                 Column 1: code (available: "Nothing" for unknown format, "VideoAudio" for at least 1 video and 1 audio, "VideoOnly" for video streams only, "AudioOnly", "TextOnly") \n
        ///                 Column 2: "" (nothing) not keeping, other for keeping
        ///       * (NOT IMPLEMENTED YET) "TagSeparator": Configure the separator if there are multiple same tags (" | " by default)\n
        ///       * (NOT IMPLEMENTED YET) "TagSeparator_Get": return the state of "TagSeparator" \n
        ///       * (NOT IMPLEMENTED YET) "Internet": Authorize Internet connection (Yes by default)
        ///       * (NOT IMPLEMENTED YET) "Internet_Title_Get": When State=5000, give all possible titles for this file (one per line) \n
        ///                 Form: Author TagSeparator Title TagSeparator Year\n...
        ///       * (NOT IMPLEMENTED YET) "Internet_Title_Set": Set the Good title (same as given by Internet_Title_Get) \n
        ///                 Form: Author TagSeparator Title TagSeparator Year
        ///       * "Info_Parameters": Information about what are known unique names for parameters \n
        ///       * "Info_Parameters_CSV": Information about what are known unique names for parameters, in CSV format \n
        ///       * "Info_Codecs": Information about which codec is known \n
        ///       * "Info_Version": Information about the version of MediaInfoLib
        ///       * "Info_Url": Information about where to find the last version
    String        Option (const String &Option, const String &Value=String());
        /// Configure or get information about MediaInfoLib
        /// @param Option The name of option
        /// @param Value The value of option
        /// @return Depend of the option: by default "" (nothing) means No, other means Yes
        /// @post Known options are: See MediaInfo::Option()
    static String Option_Static (const String &Option, const String &Value=String());
        /// @brief (NOT IMPLEMENTED YET) Get the state of the library
        /// @retval <1000 No information is available for the file yet
        /// @retval >=1000_<5000 Only local (into the file) information is available, getting Internet information (titles only) is no finished yet
        /// @retval 5000 (only if Internet connection is accepted) User interaction is needed (use Option() with "Internet_Title_Get") \n
        ///              Warning: even there is only one possible, user interaction (or the software) is needed
        /// @retval >5000<=10000 Only local (into the file) information is available, getting Internet information (all) is no finished yet
        /// @retval <10000 Done
    size_t                  State_Get ();
        /// @brief Count of streams of a stream kind (StreamNumber not filled), or count of piece of information in this stream
        /// @param StreamKind Kind of stream (general, video, audio...)
        /// @param StreamNumber Stream number in this kind of stream (first, second...)
    size_t                  Count_Get (stream_t StreamKind, size_t StreamNumber=(size_t)-1);

private :
    void* Internal;
};

} //NameSpace
#endif
