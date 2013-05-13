/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
// For user: you can disable or enable it
//#define MEDIAINFO_DEBUG
//---------------------------------------------------------------------------

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
#if defined(__BORLANDC__) && defined (_DEBUG)
    //Why? in Debug mode with release Wx Libs, wxAssert is not defined?
    void wxAssert (int, const wchar_t*, int, const wchar_t*, const wchar_t*){return;}
    void wxAssert (int, const char*, int, const char*, const char*){return;}
#endif
#include "MediaInfoList.h"
#include "MediaInfoList_Internal.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//To clarify the code
namespace MediaInfoList_Debug
{
#ifdef MEDIAINFO_DEBUG
    #include <stdio.h>
    FILE* F;
    std::string Debug;

    #undef MEDIAINFO_DEBUG
    #define MEDIAINFO_DEBUG(_TOAPPEND) \
        F=fopen("MediaInfoList_Debug.txt", "a+t"); \
        Debug.clear(); \
        _TOAPPEND; \
        Debug+="\r\n"; \
        fwrite(Debug.c_str(), Debug.size(), 1, F); \
        fclose(F);
#else // MEDIAINFO_DEBUG
    #define MEDIAINFO_DEBUG(_TOAPPEND)
#endif // MEDIAINFO_DEBUG

#ifdef MEDIAINFO_DEBUG
#define EXECUTE_VOID(_METHOD,_DEBUGB) \
        ((MediaInfo_Internal*)Internal)->_METHOD;
#else //MEDIAINFO_DEBUG
#define EXECUTE_VOID(_METHOD,_DEBUGB) \
        ((MediaInfo_Internal*)Internal)->_METHOD; \
        MEDIAINFO_DEBUG(_DEBUGB)
#endif //MEDIAINFO_DEBUG

#ifdef MEDIAINFO_DEBUG
#define EXECUTE_INT(_METHOD,_DEBUGB) \
        return ((MediaInfo_Internal*)Internal)->_METHOD;
#else //MEDIAINFO_DEBUG
#define EXECUTE_INT(_METHOD, _DEBUGB) \
        int64u ToReturn=((MediaInfo_Internal*)Internal)->_METHOD; \
        MEDIAINFO_DEBUG(_DEBUGB) \
        return ToReturn;
#endif //MEDIAINFO_DEBUG

#ifdef MEDIAINFO_DEBUG
#define EXECUTE_STRING(_METHOD,_DEBUGB) \
        return ((MediaInfo_Internal*)Internal)->_METHOD;
#else //MEDIAINFO_DEBUG
#define EXECUTE_STRING(_METHOD,_DEBUGB) \
        Ztring ToReturn=((MediaInfo_Internal*)Internal)->_METHOD; \
        MEDIAINFO_DEBUG(_DEBUGB) \
        return ToReturn;
#endif //MEDIAINFO_DEBUG
}
using namespace MediaInfoList_Debug;

namespace MediaInfoLib
{

//***************************************************************************
// Gestion de la classe
//***************************************************************************

//---------------------------------------------------------------------------
//Constructeurs
MediaInfoList::MediaInfoList(size_t Count_Init)
{
    MEDIAINFO_DEBUG(Debug+="Construction";)
    Internal=new MediaInfoList_Internal(Count_Init);
}

//---------------------------------------------------------------------------
//Destructeur
MediaInfoList::~MediaInfoList()
{
    MEDIAINFO_DEBUG(Debug+="Destruction";)
    delete Internal; //Internal=NULL;
}

//***************************************************************************
// Files
//***************************************************************************

//---------------------------------------------------------------------------
size_t MediaInfoList::Open(const String &File, const fileoptions_t Options)
{
    MEDIAINFO_DEBUG(Debug+="Open, File=";Debug+=Ztring(File).To_Local().c_str();)
    return Internal->Open(File, Options);
}

//---------------------------------------------------------------------------
size_t MediaInfoList::Open_Buffer_Init (int64u File_Size_, int64u File_Offset_)
{
    return Internal->Open_Buffer_Init(File_Size_, File_Offset_);
}

//---------------------------------------------------------------------------
size_t MediaInfoList::Open_Buffer_Continue (size_t FilePos, const int8u* ToAdd, size_t ToAdd_Size)
{
    return Internal->Open_Buffer_Continue(FilePos, ToAdd, ToAdd_Size);
}

//---------------------------------------------------------------------------
int64u MediaInfoList::Open_Buffer_Continue_GoTo_Get (size_t FilePos)
{
    return Internal->Open_Buffer_Continue_GoTo_Get(FilePos);
}

//---------------------------------------------------------------------------
size_t MediaInfoList::Open_Buffer_Finalize (size_t FilePos)
{
    return Internal->Open_Buffer_Finalize(FilePos);
}

//---------------------------------------------------------------------------
size_t MediaInfoList::Save(size_t)
{
    return 0; //Not yet implemented
}

//---------------------------------------------------------------------------
void MediaInfoList::Close(size_t FilePos)
{
    Internal->Close(FilePos);
}

//***************************************************************************
// Get File info
//***************************************************************************

//---------------------------------------------------------------------------
String MediaInfoList::Inform(size_t FilePos, size_t)
{
    return Internal->Inform(FilePos);
}

//---------------------------------------------------------------------------
String MediaInfoList::Get(size_t FilePos, stream_t KindOfStream, size_t StreamNumber, size_t Parameter, info_t KindOfInfo)
{
    return Internal->Get(FilePos, KindOfStream, StreamNumber, Parameter, KindOfInfo);
}

//---------------------------------------------------------------------------
String MediaInfoList::Get(size_t FilePos, stream_t KindOfStream, size_t StreamNumber, const String &Parameter, info_t KindOfInfo, info_t KindOfSearch)
{
    //TRACE(Trace+=__T("Get(L), CompleteName=");Trace+=Info[FilePos].Get(Stream_General, 0, __T("CompleteName")).c_str();)
    //TRACE(Trace+=__T("Get(L), StreamKind=");Trace+=ZenLib::Ztring::ToZtring((int8u)KindOfStream);Trace+=__T(", StreamNumber=");Trace+=ZenLib::Ztring::ToZtring((int8u)StreamNumber);Trace+=__T(", Parameter=");Trace+=ZenLib::Ztring(Parameter);Trace+=__T(", KindOfInfo=");Trace+=ZenLib::Ztring::ToZtring((int8u)KindOfInfo);Trace+=__T(", KindOfSearch=");Trace+=ZenLib::Ztring::ToZtring((int8u)KindOfSearch);)
    //TRACE(Trace+=__T("Get(L), will return ");Trace+=Info[FilePos].Get(KindOfStream, StreamNumber, Parameter, KindOfInfo, KindOfSearch).c_str();)

    return Internal->Get(FilePos, KindOfStream, StreamNumber, Parameter, KindOfInfo, KindOfSearch);
}

//***************************************************************************
// Set File info
//***************************************************************************

//---------------------------------------------------------------------------
size_t MediaInfoList::Set(const String &ToSet, size_t FilePos, stream_t StreamKind, size_t StreamNumber, size_t Parameter, const String &OldValue)
{
    return Internal->Set(ToSet, FilePos, StreamKind, StreamNumber, Parameter, OldValue);
}

//---------------------------------------------------------------------------
size_t MediaInfoList::Set(const String &ToSet, size_t FilePos, stream_t StreamKind, size_t StreamNumber, const String &Parameter, const String &OldValue)
{
    return Internal->Set(ToSet, FilePos, StreamKind, StreamNumber, Parameter, OldValue);
}

//***************************************************************************
// Output buffer
//***************************************************************************

/*
//---------------------------------------------------------------------------
char* MediaInfoList::Output_Buffer_Get (size_t FilePos, size_t &Output_Buffer_Size)
{
    return Internal->Output_Buffer_Get(FilePos, Output_Buffer_Size);
}
*/

//***************************************************************************
// Information
//***************************************************************************

//---------------------------------------------------------------------------
String MediaInfoList::Option (const String &Option, const String &Value)
{
    return Internal->Option(Option, Value);

}

//---------------------------------------------------------------------------
String MediaInfoList::Option_Static (const String &Option, const String &Value)
{
    return MediaInfo::Option_Static(Option, Value);
}

//---------------------------------------------------------------------------
size_t MediaInfoList::State_Get()
{
    return Internal->State_Get();
}

//---------------------------------------------------------------------------
size_t MediaInfoList::Count_Get (size_t FilePos, stream_t StreamKind, size_t StreamNumber)
{
    return Internal->Count_Get(FilePos, StreamKind, StreamNumber);
}

//---------------------------------------------------------------------------
size_t MediaInfoList::Count_Get()
{
    return Internal->Count_Get();
}

} //NameSpace
