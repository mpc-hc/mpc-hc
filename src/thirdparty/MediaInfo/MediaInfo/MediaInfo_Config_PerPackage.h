/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Configuration of MediaInfo (per Package block)
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_Config_PerPackageH
#define MediaInfo_Config_PerPackageH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Setup.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfo_Internal_Const.h"
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Config.h"
    #include "MediaInfo/MediaInfo_Events.h"
    #include "ZenLib/File.h"
#endif //MEDIAINFO_EVENTS
#include "ZenLib/CriticalSection.h"
#include "ZenLib/Translation.h"
#include "ZenLib/InfoMap.h"
#include <deque>
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

#if MEDIAINFO_EVENTS
    class File__Analyze;
#endif //MEDIAINFO_EVENTS

//***************************************************************************
// Class MediaInfo_Config_PerPackage
//***************************************************************************

class MediaInfo_Config_PerPackage
{
public :
    //Constructor/Destructor
    MediaInfo_Config_PerPackage();
    ~MediaInfo_Config_PerPackage();

    //General
    Ztring Option (const String &Option, const String &Value=Ztring());

    #if MEDIAINFO_EVENTS
    void          IsClosedGOP (File__Analyze* Source);
    #endif //MEDIAINFO_EVENTS

    #if MEDIAINFO_EVENTS
    bool          Event_CallBackFunction_IsSet ();
    Ztring        Event_CallBackFunction_Set (const Ztring &Value);
    Ztring        Event_CallBackFunction_Get ();
    void          Event_Send(File__Analyze* Source, const int8u* Data_Content, size_t Data_Size, const Ztring &File_Name=Ztring());
    void          Event_SubFile_Start(const Ztring &FileName_Absolute);
    #endif //MEDIAINFO_EVENTS

    //Internal
    size_t        CountOfPackages;

private :
    //Event
    #if MEDIAINFO_EVENTS
    MediaInfo_Event_CallBackFunction* Event_CallBackFunction; //void Event_Handler(unsigned char* Data_Content, size_t Data_Size, void* UserHandler)
    void*                   Event_UserHandler;
    #endif //MEDIAINFO_EVENTS

    ZenLib::CriticalSection CS;

    //Constructor
    MediaInfo_Config_PerPackage (const MediaInfo_Config_PerPackage&);             // Prevent copy-construction
    MediaInfo_Config_PerPackage& operator=(const MediaInfo_Config_PerPackage&);   // Prevent assignment
};

} //NameSpace

#endif
