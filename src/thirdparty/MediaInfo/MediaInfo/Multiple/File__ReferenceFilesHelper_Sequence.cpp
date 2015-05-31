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
#include "MediaInfo/Setup.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_REFERENCES_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File__ReferenceFilesHelper_Sequence.h"
#include "MediaInfo/Multiple/File__ReferenceFilesHelper_Common.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
sequence::sequence()
{
    //In
    StreamKind=Stream_Max;
    StreamPos=(size_t)-1;
    StreamID=(int64u)-1;
    MenuPos=(size_t)-1;
    Enabled=true;
    IsMain=false;
    //MenuPos=(size_t)-1;

    //Out
    State=0;
    IsCircular=false;
    #if MEDIAINFO_ADVANCED || MEDIAINFO_MD5
        List_Compute_Done=false;
    #endif //MEDIAINFO_ADVANCED || MEDIAINFO_MD5

    //Config
    Package=NULL;

    //Private
    Resources_Current=0;
    Common=new rfhs_common(&StreamKind, &StreamPos, &StreamID, NULL, &Enabled, &IsMain);




    FileNames.Separator_Set(0, __T(","));
    FrameRate=0;
    Delay=0;
    FileSize=(int64u)-1;
    IsMain=false;
    FileSize_IsPresent=false;
    #if MEDIAINFO_ADVANCED || MEDIAINFO_MD5
        List_Compute_Done=false;
    #endif //MEDIAINFO_ADVANCED || MEDIAINFO_MD5
    MI=NULL;
}

//---------------------------------------------------------------------------
sequence::~sequence()
{
        delete MI;
}

//***************************************************************************
// In
//***************************************************************************

//---------------------------------------------------------------------------
void sequence::AddFileName(const Ztring& FileName, size_t Pos)
{
    FileNames.push_back(FileName);
}

//---------------------------------------------------------------------------
void sequence::AddResource(resource* NewResource, size_t Pos)
{
    NewResource->Sequence=Common;

    #if MEDIAINFO_DEMUX
    if (Resources.empty())
    {
        NewResource->Demux_Offset_DTS=0;
    }
    #endif //MEDIAINFO_DEMUX

    if (Pos>=Resources.size())
        Resources.push_back(NewResource);
    else
        Resources.insert(Resources.begin()+Pos, NewResource);
}

//---------------------------------------------------------------------------
void sequence::UpdateFileName(const Ztring& OldFileName, const Ztring& NewFileName)
{
    size_t Resources_Size=Resources.size();
    for (size_t Resources_Pos=0; Resources_Pos<Resources_Size; ++Resources_Pos)
    {
        resource* Resource=Resources[Resources_Pos];

        Resource->UpdateFileName(OldFileName, NewFileName);

        for (size_t Pos=0; Pos<Resource->FileNames.size(); Pos++)
            if (Resource->FileNames[Pos]==NewFileName)
                Infos["UniqueID"]=OldFileName;
    }
}

//---------------------------------------------------------------------------
void sequence::FrameRate_Set(float64 NewFrameRate)
{
    FrameRate=NewFrameRate;
}

} //NameSpace

#endif //MEDIAINFO_REFERENCES_YES
