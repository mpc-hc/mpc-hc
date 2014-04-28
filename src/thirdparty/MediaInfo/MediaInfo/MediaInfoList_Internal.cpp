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
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfoList_Internal.h"
#include "MediaInfo/MediaInfo_Config.h"
#include "ZenLib/File.h"
#include "ZenLib/Dir.h"
#include "MediaInfo/Reader/Reader_Directory.h"
#include "MediaInfo/File__Analyse_Automatic.h"
using namespace ZenLib;
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
extern MediaInfo_Config Config;
//---------------------------------------------------------------------------

//***************************************************************************
// Gestion de la classe
//***************************************************************************

//---------------------------------------------------------------------------
//Constructeurs
MediaInfoList_Internal::MediaInfoList_Internal(size_t Count_Init)
: Thread()
{
    CriticalSectionLocker CSL(CS);

    //Initialisation
    Info.reserve(Count_Init);
    for (size_t Pos=0; Pos<Info.size(); Pos++)
        Info[Pos]=NULL;
    ToParse_AlreadyDone=0;
    ToParse_Total=0;
    CountValid=0;

    //Threading
    BlockMethod=0;
    State=0;
    IsInThread=false;
}

//---------------------------------------------------------------------------
//Destructeur
MediaInfoList_Internal::~MediaInfoList_Internal()
{
    Close();
}

//***************************************************************************
// Fichiers
//***************************************************************************

//---------------------------------------------------------------------------
size_t MediaInfoList_Internal::Open(const String &File_Name, const fileoptions_t Options)
{
    //Option FileOption_Close
    if (Options & FileOption_CloseAll)
        Close(All);

    //Option Recursive
    //TODO

    //Get all filenames
    ZtringList List;
    size_t Pos=File_Name.find(__T(':'));
    if (Pos!=string::npos && Pos!=1)
        List.push_back(File_Name);
    else if (File::Exists(File_Name))
        List.push_back(File_Name);
    else
        List=Dir::GetAllFileNames(File_Name, (Options&FileOption_NoRecursive)?Dir::Include_Files:((Dir::dirlist_t)(Dir::Include_Files|Dir::Parse_SubDirs)));

    #if defined(MEDIAINFO_DIRECTORY_YES)
        Reader_Directory().Directory_Cleanup(List);
    #endif //defined(MEDIAINFO_DIRECTORY_YES)

    //Registering files
    CS.Enter();
    if (ToParse.empty())
        CountValid=0;
    for (ZtringList::iterator L=List.begin(); L!=List.end(); ++L)
        ToParse.push(*L);
    ToParse_Total+=List.size();
    if (ToParse_Total)
        State=ToParse_AlreadyDone*10000/ToParse_Total;
    else
        State=10000;
    CS.Leave();

    //Parsing
    if (BlockMethod==1)
    {
        CS.Enter();
        if (!IsRunning()) //If already created, the routine will read the new files
        {
            RunAgain();
            IsInThread=true;
        }
        CS.Leave();
        return 0;
    }
    else
    {
        Entry(); //Normal parsing
        return Count_Get();
    }
}

void MediaInfoList_Internal::Entry()
{
    if (ToParse_Total==0)
        return;

    for (;;)
    {
        CS.Enter();
        if (!ToParse.empty())
        {
            MediaInfo_Internal* MI=new MediaInfo_Internal();
            for (std::map<String, String>::iterator Config_MediaInfo_Item=Config_MediaInfo_Items.begin(); Config_MediaInfo_Item!=Config_MediaInfo_Items.end(); ++Config_MediaInfo_Item)
                MI->Option(Config_MediaInfo_Item->first, Config_MediaInfo_Item->second);
            if (BlockMethod==1)
                MI->Option(__T("Thread"), __T("1"));
            MI->Open(ToParse.front());

            if (BlockMethod==1)
            {
                CS.Leave();
                while (MI->State_Get()<10000)
                {
                    size_t A=MI->State_Get();
                    CS.Enter();
                    State=(ToParse_AlreadyDone*10000+A)/ToParse_Total;
                    CS.Leave();
                    if (IsTerminating())
                    {
                        break;
                    }
                    Yield();
                }
                CS.Enter();
            }
            Info.push_back(MI);
            ToParse.pop();
            ToParse_AlreadyDone++;

            //Removing sequences of files from the list
            if (!MI->Get(Stream_General, 0, General_CompleteName_Last).empty())
            {
                Ztring CompleteName_Begin=MI->Get(Stream_General, 0, General_CompleteName);
                Ztring CompleteName_Last=MI->Get(Stream_General, 0, General_CompleteName_Last);
                size_t Pos=0;
                for (; Pos<CompleteName_Begin.size(); Pos++)
                {
                    if (Pos>=CompleteName_Last.size())
                        break;
                    if (CompleteName_Begin[Pos]!=CompleteName_Last[Pos])
                        break;
                }
                if (Pos<CompleteName_Begin.size())
                {
                    CompleteName_Begin.resize(Pos);
                    while (!ToParse.empty() && ToParse.front().find(CompleteName_Begin)==0)
                    {
                        ToParse.pop();
                        ToParse_Total--;
                    }
                }
            }

            State=ToParse_AlreadyDone*10000/ToParse_Total;
            //if ((ToParse_AlreadyDone%10)==0)
            //    printf("%f done (%i/%i %s)\n", ((float)State)/100, (int)ToParse_AlreadyDone, (int)ToParse_Total, Ztring(ToParse.front()).To_UTF8().c_str());
        }
        if (IsTerminating() || State==10000)
        {
            CS.Leave();
            break;
        }
        CS.Leave();
        Yield();
    }
}

//---------------------------------------------------------------------------
size_t MediaInfoList_Internal::Open_Buffer_Init (int64u File_Size_, int64u File_Offset_)
{
    MediaInfo_Internal* MI=new MediaInfo_Internal();
    MI->Open_Buffer_Init(File_Size_, File_Offset_);

    CriticalSectionLocker CSL(CS);
    size_t Pos=Info.size();
    Info.push_back(MI);
    return Pos;
}

//---------------------------------------------------------------------------
size_t MediaInfoList_Internal::Open_Buffer_Continue (size_t FilePos, const int8u* ToAdd, size_t ToAdd_Size)
{
    CriticalSectionLocker CSL(CS);
    if (FilePos>=Info.size() || Info[FilePos]==NULL)
        return 0;

    return Info[FilePos]->Open_Buffer_Continue(ToAdd, ToAdd_Size).to_ulong();
}

//---------------------------------------------------------------------------
int64u MediaInfoList_Internal::Open_Buffer_Continue_GoTo_Get (size_t FilePos)
{
    CriticalSectionLocker CSL(CS);
    if (FilePos>=Info.size() || Info[FilePos]==NULL)
        return (int64u)-1;

    return Info[FilePos]->Open_Buffer_Continue_GoTo_Get();
}

//---------------------------------------------------------------------------
size_t MediaInfoList_Internal::Open_Buffer_Finalize (size_t FilePos)
{
    CriticalSectionLocker CSL(CS);
    if (FilePos>=Info.size() || Info[FilePos]==NULL)
        return 0;

    return Info[FilePos]->Open_Buffer_Finalize();
}

//---------------------------------------------------------------------------
size_t MediaInfoList_Internal::Save(size_t)
{
    CriticalSectionLocker CSL(CS);
    return 0; //Not yet implemented
}

//---------------------------------------------------------------------------
void MediaInfoList_Internal::Close(size_t FilePos)
{
    if (IsRunning())
    {
        RequestTerminate();
        while(IsExited())
            Yield();
    }

    CriticalSectionLocker CSL(CS);
    if (FilePos==Unlimited)
    {
        for (size_t Pos=0; Pos<Info.size(); Pos++)
        {
            delete Info[Pos]; Info[Pos]=NULL;
        }
        Info.clear();
    }
    else if (FilePos<Info.size())
    {
        delete Info[FilePos]; Info[FilePos]=NULL;
        Info.erase(Info.begin()+FilePos);
    }

    ToParse_AlreadyDone=0;
    ToParse_Total=0;
}

//***************************************************************************
// Get File info
//***************************************************************************

//---------------------------------------------------------------------------
String MediaInfoList_Internal::Inform(size_t FilePos, size_t)
{
    if (FilePos==Error)
    {
        Ztring Retour;
        FilePos=0;
        ZtringListList MediaInfo_Custom_View; MediaInfo_Custom_View.Write(Option(__T("Inform_Get")));
        bool XML=false;
        if (MediaInfoLib::Config.Inform_Get()==__T("XML"))
            XML=true;
        if (XML) Retour+=__T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>")+MediaInfoLib::Config.LineSeparator_Get()+__T("<Mediainfo version=\"")+MediaInfoLib::Config.Info_Version_Get().SubString(__T(" v"), Ztring())+__T("\">")+MediaInfoLib::Config.LineSeparator_Get();
        else
        Retour+=MediaInfo_Custom_View("Page_Begin");
        while (FilePos<Info.size())
        {
            Retour+=Inform(FilePos);
            if (FilePos<Info.size()-1)
            {
                Retour+=MediaInfo_Custom_View("Page_Middle");
            }
            FilePos++;
        }
        if (XML) Retour+=__T("</Mediainfo>")+MediaInfoLib::Config.LineSeparator_Get();
        else Retour+=MediaInfo_Custom_View("Page_End");//
        return Retour.c_str();
    }

    CriticalSectionLocker CSL(CS);

    if (FilePos>=Info.size() || Info[FilePos]==NULL || Info[FilePos]->Count_Get(Stream_General)==0)
        return MediaInfoLib::Config.EmptyString_Get();

    Info[FilePos]->IsFirst=FilePos==0;
    Info[FilePos]->IsLast=(FilePos+1)==Info.size();
    return Info[FilePos]->Inform();
}

//---------------------------------------------------------------------------
String MediaInfoList_Internal::Get(size_t FilePos, stream_t KindOfStream, size_t StreamNumber, size_t Parameter, info_t KindOfInfo)
{
    CriticalSectionLocker CSL(CS);
    if (FilePos==Error || FilePos>=Info.size() || Info[FilePos]==NULL || Info[FilePos]->Count_Get(Stream_General)==0)
        return MediaInfoLib::Config.EmptyString_Get();

    return Info[FilePos]->Get(KindOfStream, StreamNumber, Parameter, KindOfInfo);
}

//---------------------------------------------------------------------------
String MediaInfoList_Internal::Get(size_t FilePos, stream_t KindOfStream, size_t StreamNumber, const String &Parameter, info_t KindOfInfo, info_t KindOfSearch)
{
    CriticalSectionLocker CSL(CS);
    if (FilePos==Error || FilePos>=Info.size() || Info[FilePos]==NULL || Info[FilePos]->Count_Get(Stream_General)==0)
        return MediaInfoLib::Config.EmptyString_Get();

    return Info[FilePos]->Get(KindOfStream, StreamNumber, Parameter, KindOfInfo, KindOfSearch);
}

//***************************************************************************
// Set File info
//***************************************************************************

//---------------------------------------------------------------------------
size_t MediaInfoList_Internal::Set(const String &ToSet, size_t FilePos, stream_t StreamKind, size_t StreamNumber, size_t Parameter, const String &OldValue)
{
    CriticalSectionLocker CSL(CS);
    if (FilePos==(size_t)-1)
        FilePos=0; //TODO : average

    if (FilePos>=Info.size() || Info[FilePos]==NULL || Info[FilePos]->Count_Get(Stream_General)==0)
        return 0;

    return Info[FilePos]->Set(ToSet, StreamKind, StreamNumber, Parameter, OldValue);
}

//---------------------------------------------------------------------------
size_t MediaInfoList_Internal::Set(const String &ToSet, size_t FilePos, stream_t StreamKind, size_t StreamNumber, const String &Parameter, const String &OldValue)
{
    CriticalSectionLocker CSL(CS);
    if (FilePos==(size_t)-1)
        FilePos=0; //TODO : average

    if (FilePos>=Info.size() || Info[FilePos]==NULL || Info[FilePos]->Count_Get(Stream_General)==0)
        return 0;

    return Info[FilePos]->Set(ToSet, StreamKind, StreamNumber, Parameter, OldValue);
}

//***************************************************************************
// Output buffer
//***************************************************************************

/*
//---------------------------------------------------------------------------
char* MediaInfoList_Internal::Output_Buffer_Get (size_t FilePos, size_t &Output_Buffer_Size)
{
    if (FilePos==(size_t)-1)
        FilePos=0; //TODO : average

    if (FilePos>=Info.size() || Info[FilePos]==NULL || Info[FilePos]->Count_Get(Stream_General)==0)
        return 0;

    return Info[FilePos]->Output_Buffer_Get(Output_Buffer_Size);
}
*/

//***************************************************************************
// Information
//***************************************************************************

//---------------------------------------------------------------------------
String MediaInfoList_Internal::Option (const String &Option, const String &Value)
{
    CriticalSectionLocker CSL(CS);
    Ztring OptionLower=Option; OptionLower.MakeLowerCase();
         if (Option==__T(""))
        return __T("");
    else if (OptionLower==__T("manguage_update"))
    {
        //Special case : Language_Update must update all MediaInfo classes
        for (unsigned int Pos=0; Pos<Info.size(); Pos++)
            if (Info[Pos])
                Info[Pos]->Option(__T("language_update"), Value);

        return __T("");
    }
    else if (OptionLower==__T("create_dummy"))
    {
        Info.resize(Info.size()+1);
        Info[Info.size()-1]=new MediaInfo_Internal();
        Info[Info.size()-1]->Option(Option, Value);
        return __T("");
    }
    else if (OptionLower==__T("thread"))
    {
        BlockMethod=1;
        return __T("");
    }
    else if (OptionLower.find(__T("file_"))==0)
    {
        Config_MediaInfo_Items[Option]=Value;
        return __T("");
    }
    else
        return MediaInfo::Option_Static(Option, Value);
}

//---------------------------------------------------------------------------
String MediaInfoList_Internal::Option_Static (const String &Option, const String &Value)
{
    return MediaInfo::Option_Static(Option, Value);
}

//---------------------------------------------------------------------------
size_t MediaInfoList_Internal::State_Get()
{
    CriticalSectionLocker CSL(CS);
    if (State==10000)
    {
        //Pause();
        IsInThread=false;
    }
    return State;
}

//---------------------------------------------------------------------------
size_t MediaInfoList_Internal::Count_Get (size_t FilePos, stream_t StreamKind, size_t StreamNumber)
{
    CriticalSectionLocker CSL(CS);
    if (FilePos>=Info.size() || Info[FilePos]==NULL)
        return 0;

    return Info[FilePos]->Count_Get(StreamKind, StreamNumber);
}

//---------------------------------------------------------------------------
size_t MediaInfoList_Internal::Count_Get()
{
    CriticalSectionLocker CSL(CS);
    return Info.size();
}

} //NameSpace
