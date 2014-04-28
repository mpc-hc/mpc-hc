/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Give information about a lot of media files
// Dispatch the file to be tested by all containers
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef Reader_FileH
#define Reader_FileH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Reader/Reader__Base.h"
#include "ZenLib/File.h"
#include "ZenLib/Thread.h"
#include "ZenLib/CriticalSection.h"
#if MEDIAINFO_READTHREAD
    #ifdef WINDOWS
        #undef __TEXT
        #include "Windows.h"
    #endif //WINDOWS
#endif //MEDIAINFO_READTHREAD
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
/// @brief Reader_File
//***************************************************************************

#if MEDIAINFO_READTHREAD
class Reader_File;
class Reader_File_Thread : public Thread
{
public:
    Reader_File* Base;
    void Entry();
};
#endif //MEDIAINFO_READTHREAD

class Reader_File : public Reader__Base
{
public :
    //Constructor/Destructor
    virtual ~Reader_File();

    //Format testing
    size_t Format_Test(MediaInfo_Internal* MI, String File_Name);
    size_t Format_Test_PerParser(MediaInfo_Internal* MI, const String &File_Name);
    size_t Format_Test_PerParser_Continue (MediaInfo_Internal* MI);
    size_t Format_Test_PerParser_Seek (MediaInfo_Internal* MI, size_t Method, int64u Value, int64u ID);

    ZenLib::File F;
    std::bitset<32> Status;
    int64u          Partial_Begin;
    int64u          Partial_End;

    //Thread
    #if MEDIAINFO_READTHREAD
        Reader_File_Thread* ThreadInstance;
        int8u* Buffer;
        size_t Buffer_Max;
        size_t Buffer_Begin;
        size_t Buffer_End;
        size_t Buffer_End2; //Is also used for counting bytes before activating the thread
        bool   IsLooping;
        #ifdef WINDOWS
            HANDLE Condition_WaitingForMorePlace;
            HANDLE Condition_WaitingForMoreData;
        #endif //WINDOWS
    #endif //MEDIAINFO_READTHREAD
    CriticalSection CS;
    MediaInfo_Internal* MI_Internal;
};

} //NameSpace
#endif
