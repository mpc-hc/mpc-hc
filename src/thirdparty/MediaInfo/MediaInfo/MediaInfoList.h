/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Give information about a lot of media files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfoListH
#define MediaInfoListH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfo.h"
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

class MediaInfoList_Internal;

//***************************************************************************
/// @brief MediaInfoList
/// @version 0.7
//***************************************************************************

class MEDIAINFO_EXP MediaInfoList
{
public :
    //Class
        /// @brief Constructor
        /// @param Count_Init optimization information: How many files do you plan to handle?
    MediaInfoList (size_t Count_Init=64);
    ~MediaInfoList ();

    //Files
        /// Open one or more files and collect information about them (technical information and tags)
        /// @brief Open files
        /// @param File Full name of file(s) to open \n
        ///             or Full name of folder(s) to open \n
        ///             (if multiple names, names must be separated by "|")
        /// @param Options: FileOption_Recursive = Recursive mode for folders \n
        ///                  FileOption_Close = Close all already opened files before
        /// @return Number of files successfuly added
    size_t Open (const String &File, const fileoptions_t Options=FileOption_Nothing);
        /// Open a stream and collect information about it (technical information and tags)
        /// @brief Open a stream (Init)
        /// @param File_Size Estimated file size
        /// @param File_Offset Offset of the file (if we don't have the beginning of the file)
        /// @retval 0 failed
        /// @retval 1 succeed
    size_t Open_Buffer_Init (ZenLib::int64u File_Size=(ZenLib::int64u)-1, ZenLib::int64u File_Offset=0);
        /// Open a stream and collect information about it (technical information and tags)
        /// @brief Open a stream (Continue)
        /// @param FilePos File position
        /// @param Buffer pointer to the stream
        /// @param Buffer_Size Count of bytes to read
        /// @retval 0 failed
        /// @retval 1 succeed
    size_t Open_Buffer_Continue (size_t FilePos, const ZenLib::int8u* Buffer, size_t Buffer_Size);
        /// Open a stream and collect information about it (technical information and tags)
        /// @brief Open a stream (Get the needed file Offset)
        /// @param FilePos File position
        /// @return the needed offset of the file \n
        ///         File size if no more bytes are needed
    ZenLib::int64u Open_Buffer_Continue_GoTo_Get (size_t FilePos);
        /// Open a stream and collect information about it (technical information and tags)
        /// @brief Open a stream (Finalize)
        /// @param FilePos File position
        /// @retval 0 failed
        /// @retval 1 succeed
    size_t Open_Buffer_Finalize (size_t FilePos);
        /// Save the file opened before with Open() (modifications of tags)
        /// @brief Save the file
        /// @param FilePos File position \n
        ///        (you can know the position in searching the filename with MediaInfoList::Get(FilePos, 0, 0, "CompleteName") )
        /// @retval 0 failed
        /// @retval 1 succeed
    size_t Save (size_t FilePos);
        /// (NOT IMPLEMENTED YET) Save all files opened before with Open() (modifications of tags)
        /// @brief (NOT IMPLEMENTED YET) Save all files
        /// @param FilePos File position \n
        ///        (you can know the position in searching the filename with MediaInfoList::Get(FilePos, 0, 0, "CompleteName") )
        /// @retval Count of files saved
    void Close (size_t FilePos=(size_t)-1);

        /// Get all details about a file in one string
        /// @brief Get all details about a file
        /// @param FilePos File position \n
        ///        (you can know the position in searching the filename with MediaInfoList::Get(FilePos, 0, 0, "CompleteName") )
        /// @param Reserved Deprecated, do not use it anymore
        /// @pre You can change default presentation with Inform_Set()
        /// @return Text with information about the file
    String Inform (size_t FilePos=(size_t)-1, size_t Reserved=0);

    //Get
        /// Get a piece of information about a file (parameter is an integer)
        /// @brief Get a piece of information about a file (parameter is an integer)
        /// @param FilePos File position \n
        ///        (you can know the position in searching the filename with MediaInfoList::Get(FilePos, 0, 0, "CompleteName") )
        /// @param StreamKind Kind of stream (general, video, audio...)
        /// @param StreamNumber Stream number in Kind of stream (first, second...)
        /// @param Parameter Parameter you are looking for in the stream (Codec, width, bitrate...), in integer format (first parameter, second parameter...)
        /// @param KindOfInfo Kind of information you want about the parameter (the text, the measure, the help...)
        /// @return a string about information you search \n
        ///         an empty string if there is a problem
    String Get (size_t FilePos, stream_t StreamKind, size_t StreamNumber, size_t Parameter, info_t KindOfInfo=Info_Text); //Get info, FilePos=File position, StreamKind=General video audio text chapter, StreamNumber=stream number, PosInStream=parameter you want, KindOfInfo=name, text, measure, options, name (language), measure (language), info, how to
        /// Get a piece of information about a file (parameter is a string)
        /// @brief Get a piece of information about a file (parameter is a string)
        /// @param FilePos File position \n
        ///        (you can know the position in searching the filename with MediaInfoList::Get(FilePos, 0, 0, "CompleteName") )
        /// @param StreamKind Kind of stream (general, video, audio...)
        /// @param StreamNumber Stream number in Kind of stream (first, second...)
        /// @param Parameter Parameter you are looking for in the stream (Codec, width, bitrate...), in string format ("Codec", "Width"...) \n
        ///        See MediaInfo::Option("Info_Parameters") to have the full list
        /// @param KindOfInfo Kind of information you want about the parameter (the text, the measure, the help...)
        /// @param KindOfSearch Where to look for the parameter
        /// @return a string about information you search \n
        ///         an empty string if there is a problem
    String Get (size_t FilePos, stream_t StreamKind, size_t StreamNumber, const String &Parameter, info_t KindOfInfo=Info_Text, info_t KindOfSearch=Info_Name); //Get info, FilePos=File position, StreamKind=General video audio text chapter, StreamNumber=stream number, PosInStream=parameter you want, KindOfInfo=name text measure options name(language) measure(language) information how to, KindOfSearch=which Kind Of information Parameter must be searched?

    //Set
        /// (NOT IMPLEMENTED YET) Set a piece of information about a file (parameter is an int)
        /// @brief (NOT IMPLEMENTED YET) Set a piece of information about a file (parameter is an int)
        /// @warning Not yet implemented, do not use it
        /// @param ToSet Piece of information
        /// @param FilePos File position \n
        ///        (you can know the position in searching the filename with MediaInfoList::Get(FilePos, 0, 0, "CompleteName") )
        /// @param StreamKind Kind of stream (general, video, audio...)
        /// @param StreamNumber Stream number in Kind of stream (first, second...)
        /// @param Parameter Parameter you are looking for in the stream (Codec, width, bitrate...), in integer format (first parameter, second parameter...)
        /// @param OldValue The old value of the parameter \n if OldValue is empty and ToSet is filled: tag is added \n if OldValue is filled and ToSet is filled: tag is replaced \n if OldValue is filled and ToSet is empty: tag is deleted
        /// @retval >0 succeed
        /// @retval 0 failed
    size_t Set (const String &ToSet, size_t FilePos, stream_t StreamKind, size_t StreamNumber, size_t Parameter, const String &OldValue=String()); //Get info, FilePos=File position, StreamKind=General video audio text chapter, StreamNumber=stream number, PosInStream=parameter you want, KindOfInfo=name, text, measure, options name(language) measure(language) information how to
        /// @brief (NOT IMPLEMENTED YET) Get information about a file (parameter is a string)
        /// @warning Not yet implemented, do not use it
        /// @param ToSet Piece of information
        /// @param FilePos File position \n
        ///        (you can know the position in searching the filename with MediaInfoList::Get(FilePos, 0, 0, "CompleteName") )
        /// @param StreamKind Kind of stream (general, video, audio...)
        /// @param StreamNumber Stream number in Kind of stream (first, second...)
        /// @param Parameter Parameter you are looking for in the stream (Codec, width, bitrate...), in string format ("Codec", "Width"...) \n
        ///        See Option("Info_Parameters") to have the full list
        /// @param OldValue The old value of the parameter \n if OldValue is empty and ToSet is filled: tag is added \n if OldValue is filled and ToSet is filled: tag is replaced \n if OldValue is filled and ToSet is empty: tag is deleted
        /// @retval >0 succeed
        /// @retval 0 failed
    size_t Set (const String &ToSet, size_t FilePos, stream_t StreamKind, size_t StreamNumber, const String &Parameter, const String &OldValue=String()); //Get info, FilePos=File position, StreamKind=General video audio text chapter, StreamNumber=stream number, PosInStream=parameter you want, KindOfInfo=name text measure options name (language) measure (language) information how to, KindOfSearch=which Kind Of information Parameter must be searched?

    //Output_Buffered
        /// Output buffer retrieving, used for File_Duplicate option.
        /// @brief Output buffer retrieving
        /// @param FilePos File position
        /// @param Output_Buffer_Size A pointer to the variable that receives the size of the buffer \n
        ///        Note: you must use all the size of the buffer before the next call to this procedure
        /// @return A pointer on the output buffer, NULL if there is nothing in the buffer
    char* Output_Buffer_Get (size_t FilePos, size_t &Output_Buffer_Size);

    //Info
        /// Configure or get information about MediaInfoLib
        /// @param Option The name of option
        /// @param Value The value of option
        /// @return Depend of the option: by default "" (nothing) means No, other means Yes
        /// @post Known options are: See MediaInfo::Option()
    String        Option (const String &Option, const String &Value=String());
        /// Configure or get information about MediaInfoLib (static version)
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
        /// @brief Count of streams, or count of piece of information in this stream
        /// @param FilePos File position \n
        ///        (you can know the position in searching the filename with MediaInfoList::Get(FilePos, 0, 0, "CompleteName") )
        /// @param StreamKind Kind of stream (general, video, audio...)
        /// @param StreamNumber Stream number in this kind of stream (first, second...)
        /// @return The count of fields for this stream kind / stream number if stream number is provided, else the count of streams for this stream kind
    size_t                  Count_Get (size_t FilePos, stream_t StreamKind, size_t StreamNumber=(size_t)-1);
        /// @brief Get the count of opened files
        /// @return Count of files opened
    size_t                  Count_Get ();

private :
    MediaInfoList_Internal* Internal;

    //Constructor
    MediaInfoList (const MediaInfoList&);                   // Prevent copy-construction
    MediaInfoList& operator=(const MediaInfoList&);         // Prevent assignment
};

} //NameSpace
#endif
