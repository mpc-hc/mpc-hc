// File__Analyze - Base for analyze files
// Copyright (C) 2007-2010 MediaArea.net SARL, Info@MediaArea.net
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

//---------------------------------------------------------------------------
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#include "MediaInfo/MediaInfo_Config.h"
#include <cstring>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
extern MediaInfo_Config Config;
//---------------------------------------------------------------------------

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File__Analyze::File__Analyze ()
:File__Base()
{
    //Details
    #if MEDIAINFO_TRACE
        Config_DetailsLevel=MediaInfoLib::Config.DetailsLevel_Get();
        Details_DoNotSave=false;
    #endif //MEDIAINFO_TRACE
    IsSub=false;
    IsRawStream=false;

    //In
    #if MEDIAINFO_EVENTS
        StreamIDs_Size=1;
        StreamIDs_Width[0]=(int8u)-1;
    #endif //MEDIAINFO_EVENTS
    PTS_DTS_Needed=false;
    PTS_DTS_Offset_InThisBlock=0;
    PCR=(int64u)-1;
    PTS=(int64u)-1;
    DTS=(int64u)-1;

    //Out
    Frame_Count_InThisBlock=0;

    //Configuration
    DataMustAlwaysBeComplete=true;
    MustUseAlternativeParser=false;
    MustSynchronize=false;

    //Buffer
    Buffer=NULL;
    Buffer_Temp=NULL;
    Buffer_Size=0;
    Buffer_Temp_Size=0;
    Buffer_Temp_Size_Max=0;
    Buffer_Offset=0;
    Buffer_Offset_Temp=0;
    Buffer_MinimumSize=0;
    Buffer_MaximumSize=1024*1024;
    Buffer_TotalBytes_FirstSynched=0;
    Buffer_TotalBytes=0;
    if (MediaInfoLib::Config.FormatDetection_MaximumOffset_Get())
        Buffer_TotalBytes_FirstSynched_Max=MediaInfoLib::Config.FormatDetection_MaximumOffset_Get();
    else
        Buffer_TotalBytes_FirstSynched_Max=1024*1024;

    //EOF
    EOF_AlreadyDetected=false;

    //Synchro
    MustParseTheHeaderFile=true;
    Synched=false;
    Trusted=Error;
    Trusted_Multiplier=1;

    //Header
    Header_Size=0;

    //Element
    Element_Offset=0;
    Element_Size=0;

    //Elements
    Element.resize(64);
    Element[0].Code=0;
    Element[0].Next=File_Size;
    Element[0].WaitForMoreData=false;
    Element[0].UnTrusted=false;
    Element[0].IsComplete=false;
    #if MEDIAINFO_TRACE
    if (Config_DetailsLevel!=0)
    {
        //ToShow part
        Element[0].ToShow.Name.clear();
        Element[0].ToShow.Info.clear();
        Element[0].ToShow.Details.clear();
        Element[0].ToShow.NoShow=false;
    }
    #endif //MEDIAINFO_TRACE
    Element_Level_Base=0;
    Element_Level=0;

    //BitStream
    BS=new BitStream;

    //Temp
    Status[IsAccepted]=false;
    Status[IsFilled]=false;
    Status[IsUpdated]=false;
    Status[IsFinished]=false;
    ShouldContinueParsing=false;

    //Events data
    #if MEDIAINFO_EVENTS
        MpegPs_PES_FirstByte_IsAvailable=false;
    #endif //MEDIAINFO_EVENTS
}

//---------------------------------------------------------------------------
File__Analyze::~File__Analyze ()
{
    //Buffer
    delete[] Buffer_Temp; //Buffer_Temp=NULL;

    //BitStream
    delete BS; //BS=NULL;
}

//***************************************************************************
// Open
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Open_Buffer_Init (int64u File_Size_)
{
    //Preparing
    File_Size=File_Size_;
    Element[0].Next=File_Size;

    //Buffer - Global
    Read_Buffer_Init();

    //Integrity
    if (File_Offset>File_Size)
    {
        Reject();
        return; //There is a problem
    }

    //Jump handling
    if (File_GoTo!=(int64u)-1)
    {
        Synched=false;
        File_GoTo=(int64u)-1;
        Buffer_Clear();
    }

    //Configuring
    if (MediaInfoLib::Config.FormatDetection_MaximumOffset_Get())
        Buffer_TotalBytes_FirstSynched_Max=MediaInfoLib::Config.FormatDetection_MaximumOffset_Get();
    if (Config->File_IsSub_Get())
        IsSub=true;
}

void File__Analyze::Open_Buffer_Init (File__Analyze* Sub)
{
    Open_Buffer_Init(Sub, File_Size);
}

void File__Analyze::Open_Buffer_Init (File__Analyze* Sub, int64u File_Size_)
{
    //Integrity
    if (Sub==NULL)
        return;

    //Parsing
    #if MEDIAINFO_TRACE
        Sub->Init(Config, Details);
    #else //MEDIAINFO_TRACE
        Sub->Init(Config);
    #endif //MEDIAINFO_TRACE
    Sub->IsSub=true;
    Sub->Open_Buffer_Init(File_Size_);

    #if MEDIAINFO_EVENTS
        Sub->ParserIDs[StreamIDs_Size]=Sub->ParserIDs[0];
        Sub->StreamIDs_Width[StreamIDs_Size]=Sub->StreamIDs_Width[0];
        for (size_t Pos=0; Pos<StreamIDs_Size; Pos++)
        {
            Sub->ParserIDs[Pos]=ParserIDs[Pos];
            Sub->StreamIDs[Pos]=StreamIDs[Pos];
            Sub->StreamIDs_Width[Pos]=StreamIDs_Width[Pos];
        }
        Sub->StreamIDs[StreamIDs_Size-1]=Element_Code;
        Sub->StreamIDs_Size=StreamIDs_Size+1;
    #endif //MEDIAINFO_EVENTS
    Sub->File_Name_WithoutDemux=IsSub?File_Name_WithoutDemux:File_Name;
}

//---------------------------------------------------------------------------
void File__Analyze::Open_Buffer_Continue (const int8u* ToAdd, size_t ToAdd_Size)
{
    Frame_Count_InThisBlock=0; //Out

    //Integrity
    if (Status[IsFinished])
        return;
    //{File F; F.Open(Ztring(_T("d:\\direct"))+Ztring::ToZtring((size_t)this, 16), File::Access_Write_Append); F.Write(ToAdd, ToAdd_Size);}

    //Demand to go elsewhere
    if (File_GoTo!=(int64u)-1)
    {
        if (File_GoTo<File_Offset)
            return; //Seek must be done before
        if (File_GoTo>=File_Offset+ToAdd_Size)
        {
            File_Offset+=ToAdd_Size;
            return; //No need of this piece of data
        }

        //The needed offset is in the new buffer
        ToAdd+=(size_t)(File_GoTo-File_Offset);
        ToAdd_Size-=(size_t)(File_GoTo-File_Offset);
        File_Offset=File_GoTo;
        File_GoTo=(int64u)-1;
    }

    if (Buffer_Temp_Size) //There is buffered data from before
    {
        //Allocating new buffer if needed
        if (Buffer_Temp_Size+ToAdd_Size>Buffer_Temp_Size_Max)
        {
            int8u* Old=Buffer_Temp;
            size_t Buffer_Temp_Size_Max_ToAdd=ToAdd_Size>32768?ToAdd_Size:32768;
            if (Buffer_Temp_Size_Max_ToAdd<Buffer_Temp_Size_Max) Buffer_Temp_Size_Max_ToAdd=Buffer_Temp_Size_Max;
            Buffer_Temp_Size_Max+=Buffer_Temp_Size_Max_ToAdd;
            Buffer_Temp=new int8u[Buffer_Temp_Size_Max];
            std::memcpy(Buffer_Temp, Old, Buffer_Temp_Size);
            delete[] Old; //Old=NULL;
        }

        //Copying buffer
        if (ToAdd_Size>0)
        {
            std::memcpy(Buffer_Temp+Buffer_Size, ToAdd, ToAdd_Size);
            Buffer_Temp_Size+=ToAdd_Size;
        }

        //Buffer
        Buffer=Buffer_Temp;
        Buffer_Size=Buffer_Temp_Size;
    }
    else
    {
        Buffer=ToAdd;
        Buffer_Size=ToAdd_Size;
    }

    //Preparing
    Buffer_Offset=0;
    Trusted=(Buffer_Size>2*8*1024?Buffer_Size/8/1024:2)*Trusted_Multiplier; //Never less than 2 acceptable errors

    //Parsing
    if (Buffer_Size>=Buffer_MinimumSize || File_Offset+Buffer_Size==File_Size) //Parsing only if we have enough buffer
        Open_Buffer_Continue_Loop();
    Buffer_TotalBytes+=Buffer_Offset;

    //Should parse again?
    if ((File_GoTo==File_Size && File_Size!=(int64u)-1) || File_Offset+Buffer_Offset>=File_Size)
    {
        if (!BookMark_Code.empty())
            BookMark_Get();

        if (File_GoTo>=File_Size)
        {
            Element_Show(); //If Element_Level is >0, we must show what is in the details buffer
            while (Element_Level>0)
                Element_End(); //This is Finish, must flush
            ForceFinish();
            return;
        }
    }

    //Demand to go elsewhere
    if (File_GoTo!=(int64u)-1)
    {
        if (Config->File_IsSeekable_Get())
        {
            if (File_GoTo>=File_Size)
                File_GoTo=File_Size;
            File_Offset+=Buffer_Size;
            Buffer_Clear();
        }
        else
            File_Offset+=Buffer_Offset;
        
        return;
    }
    if (Buffer_Offset>=Buffer_Size)
    {
        if (Buffer_Offset>Buffer_Size)
            File_GoTo=File_Offset+Buffer_Offset;
        if (!Status[IsFinished])
            File_Offset+=Buffer_Size;
        Buffer_Clear();
        return;
    }

    //Buffer handling
    if (Buffer_Offset!=Buffer_Size) //all is not used
    {
        if (File_Offset+Buffer_Size>=File_Size) //No more data will come
        {
                ForceFinish();
                return;
        }
        if (Buffer_Temp_Size==0) //If there was no copy
        {
            if (Buffer_Temp!=NULL && Buffer_Temp_Size_Max<ToAdd_Size-Buffer_Offset)
            {
                delete[] Buffer_Temp; Buffer_Temp=NULL; Buffer_Temp_Size=0; Buffer_Temp_Size_Max=0;
            }
            if (Buffer_Temp==NULL)
            {
                size_t Buffer_Temp_Size_Max_ToAdd=ToAdd_Size-Buffer_Offset>32768?ToAdd_Size-Buffer_Offset:32768;
                if (Buffer_Temp_Size_Max_ToAdd<Buffer_Temp_Size_Max) Buffer_Temp_Size_Max_ToAdd=Buffer_Temp_Size_Max;
                Buffer_Temp_Size_Max=Buffer_Temp_Size_Max_ToAdd;
                Buffer_Temp=new int8u[Buffer_Temp_Size_Max];
            }
            Buffer_Temp_Size=ToAdd_Size-Buffer_Offset;
            std::memcpy(Buffer_Temp, ToAdd+Buffer_Offset, Buffer_Temp_Size);
        }
        else //Already a copy, just moving it
        {
            std::memmove(Buffer_Temp, Buffer_Temp+Buffer_Offset, Buffer_Size-Buffer_Offset);
            Buffer_Temp_Size=Buffer_Size-Buffer_Offset;
        }
    }
    else if (Buffer_Temp_Size)
        Buffer_Temp_Size=0;

    //Reserving unused data
    if ((int64u)-1-Buffer_Offset<File_Offset) //In case of unknown filesize, File_Offset may be (int64u)-1
        Buffer_Offset=(size_t)((int64u)-1-File_Offset);
    Buffer_Size-=Buffer_Offset;
    File_Offset+=Buffer_Offset;
    if (Buffer_Offset_Temp>=Buffer_Offset)
        Buffer_Offset_Temp-=Buffer_Offset;
    Buffer_Offset=0;

    //Is it OK?
    if (Buffer_Size>Buffer_MaximumSize)
    {
        ForceFinish();
        return;
    }
}

void File__Analyze::Open_Buffer_Continue (File__Analyze* Sub, const int8u* ToAdd, size_t ToAdd_Size)
{
    if (Sub==NULL)
        return;

    //Sub
    if (Sub->File_GoTo!=(int64u)-1)
        Sub->File_GoTo=(int64u)-1;
    Sub->File_Offset=File_Offset+Buffer_Offset+Element_Offset;
    #if MEDIAINFO_TRACE
        Sub->Element_Level_Base=Element_Level_Base+Element_Level;
    #endif

    //{File F; F.Open(Ztring(_T("d:\\direct"))+Ztring::ToZtring((size_t)this, 16), File::Access_Write_Append); F.Write(ToAdd, ToAdd_Size);}

    //Adaptating File_Offset
    if (Sub!=this && Sub->Buffer_Size<=Sub->File_Offset)
        Sub->File_Offset-=Sub->Buffer_Size;

    //Parsing
    Sub->Open_Buffer_Continue(ToAdd, ToAdd_Size);

    #if MEDIAINFO_TRACE
        //Details handling
        if (!Sub->Element[0].ToShow.Details.empty() && !Details_DoNotSave)
        {
            //Line separator
            if (!Element[Element_Level].ToShow.Details.empty())
                Element[Element_Level].ToShow.Details+=MediaInfoLib::Config.LineSeparator_Get();

            //From Sub
            while(Sub->Element_Level)
                Sub->Element_End();
            Element[Element_Level].ToShow.Details+=Sub->Element[0].ToShow.Details;
            Sub->Element[0].ToShow.Details.clear();
        }
        else
            Element[Element_Level].ToShow.NoShow=true; //We don't want to show this item because there is no info in it
    #endif
}

//---------------------------------------------------------------------------
void File__Analyze::Open_Buffer_Continue_Loop ()
{
    //Header
    if (MustParseTheHeaderFile)
    {
        if (!FileHeader_Manage())
            return; //Wait for more data
        if (Status[IsFinished] || File_GoTo!=(int64u)-1)
            return; //Finish
    }

    //Parsing specific
    Frame_Count_InThisBlock=0; //Out
    Element_Offset=0;
    Element_Size=Buffer_Size;
    Element[Element_Level].WaitForMoreData=false;
    Read_Buffer_Continue();
    if (Element_IsWaitingForMoreData())
        return; //Wait for more data
    Buffer_Offset+=(size_t)Element_Offset;
    if (Status[IsFinished] && !ShouldContinueParsing || Buffer_Offset>=Buffer_Size || File_GoTo!=(int64u)-1)
        return; //Finish

    //Parsing;
    while (Buffer_Parse());

    //Handling of File_GoTo with already buffered data
    if (File_GoTo!=(int64u)-1 && File_GoTo>=File_Offset && File_GoTo<=File_Offset+Buffer_Size)
    {
        Buffer_Offset=(size_t)(File_GoTo-File_Offset);
        File_GoTo=(int64u)-1;
        return Open_Buffer_Continue_Loop();
    }

    //Jumping to the end of the file if needed
    if (!IsSub && !EOF_AlreadyDetected && MediaInfoLib::Config.ParseSpeed_Get()<1 && Count_Get(Stream_General))
    {
        Element[Element_Level].WaitForMoreData=false;
        Detect_EOF();
        if ((File_GoTo!=(int64u)-1 && File_GoTo>File_Offset+Buffer_Offset) || (Status[IsFinished] && !ShouldContinueParsing))
        {
            EOF_AlreadyDetected=true;
            return;
        }
    }
}

//---------------------------------------------------------------------------
void File__Analyze::Open_Buffer_Position_Set (int64u File_Offset_)
{
    if (File_Offset_==(int64u)-1)
        return;

    File_Offset=File_Offset_;
    File_GoTo=(int64u)-1;
}

//---------------------------------------------------------------------------
void File__Analyze::Open_Buffer_Unsynch ()
{
    Buffer_Clear();
    Read_Buffer_Unsynched();

    //Clearing duration
    if (Synched)
    {
        for (size_t StreamKind=(size_t)Stream_General; StreamKind<(size_t)Stream_Menu; StreamKind++)
        {
            size_t StreamPos_Count=Count_Get((stream_t)StreamKind);
            for (size_t StreamPos=0; StreamPos<StreamPos_Count; StreamPos++)
                Clear((stream_t)StreamKind, StreamPos, Fill_Parameter((stream_t)StreamKind, Generic_Duration));
        }
        Synched=false;
    }

    File_GoTo=(int64u)-1;
}

//---------------------------------------------------------------------------
void File__Analyze::Open_Buffer_Finalize (bool NoBufferModification)
{
    //File with unknown size (stream...), finnishing
    if (!NoBufferModification && File_Size==(int64u)-1)
    {
        File_Size=File_Offset+Buffer_Size;
        Open_Buffer_Continue((const int8u*)NULL, 0);
    }

    //Element must be Finish
    while (Element_Level>0)
        Element_End();

    //Buffer - Global
    Fill();
    if (!NoBufferModification)
    {
        ForceFinish();
        Buffer_Clear();
    }

    #if MEDIAINFO_TRACE
    if (Details && Details->empty())
        Details->assign(Element[0].ToShow.Details);
    #endif //MEDIAINFO_TRACE

    #if MEDIAINFO_EVENTS
        struct MediaInfo_Event_General_End_0 Event;
        Event.EventCode=MediaInfo_EventCode_Create(MediaInfo_Parser_None, MediaInfo_Event_General_End, 0);
        Event.Stream_Bytes_Analyzed=Buffer_TotalBytes;
        Config->Event_Send((const int8u*)&Event, sizeof(MediaInfo_Event_General_End_0));
    #endif //MEDIAINFO_EVENTS
}

void File__Analyze::Open_Buffer_Finalize (File__Analyze* Sub)
{
    if (Sub==NULL)
        return;

    //Finalize
    Open_Buffer_Init(Sub);
    Sub->Open_Buffer_Finalize();
}

//***************************************************************************
// Buffer
//***************************************************************************

//---------------------------------------------------------------------------
bool File__Analyze::Buffer_Parse()
{
    //End of this level?
    if (File_Offset+Buffer_Offset>=Element[Element_Level].Next)
    {
        //There is no loop handler, so we make the level down here
        while (Element_Level>0 && File_Offset+Buffer_Offset>=Element[Element_Level].Next)
            Element_End(); //This is a buffer restart, must sync to Element level
        if (File_Offset+Buffer_Offset==File_Size)
            return false; //End of file
        MustUseAlternativeParser=false; //Reset it if we go out of an element
    }

    //Synchro
    if (MustSynchronize && !Synchro_Manage())
        return false; //Wait for more data

    //Header
    if (!Header_Manage())
        return false; //Wait for more data

    //Data
    if (!Data_Manage())
        return false; //Wait for more data

    return Buffer_Offset!=Buffer_Size;
}

//---------------------------------------------------------------------------
void File__Analyze::Buffer_Clear()
{
    //Buffer
    BS->Attach(NULL, 0);
    delete[] Buffer_Temp; Buffer_Temp=NULL;
    Buffer_Size=0;
    Buffer_Temp_Size=0;
    Buffer_Offset=0;
    Buffer_Offset_Temp=0;
    Buffer_MinimumSize=0;

    //Details
    #if MEDIAINFO_TRACE
        Element[Element_Level].WaitForMoreData=false; //We must finalize the details
        Element[Element_Level].IsComplete=true; //We must finalize the details
    #endif //MEDIAINFO_TRACE

}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
bool File__Analyze::Synchronize_0x000001()
{
    //Synchronizing
    while(Buffer_Offset+3<=Buffer_Size && (Buffer[Buffer_Offset  ]!=0x00
                                        || Buffer[Buffer_Offset+1]!=0x00
                                        || Buffer[Buffer_Offset+2]!=0x01))
    {
        Buffer_Offset+=2;
        while(Buffer_Offset<Buffer_Size && Buffer[Buffer_Offset]!=0x00)
            Buffer_Offset+=2;
        if (Buffer_Offset<Buffer_Size && Buffer[Buffer_Offset-1]==0x00 || Buffer_Offset>=Buffer_Size)
            Buffer_Offset--;
    }

    //Parsing last bytes if needed
    if (Buffer_Offset+3==Buffer_Size && (Buffer[Buffer_Offset  ]!=0x00
                                      || Buffer[Buffer_Offset+1]!=0x00
                                      || Buffer[Buffer_Offset+2]!=0x01))
        Buffer_Offset++;
    if (Buffer_Offset+2==Buffer_Size && (Buffer[Buffer_Offset  ]!=0x00
                                      || Buffer[Buffer_Offset+1]!=0x00))
        Buffer_Offset++;
    if (Buffer_Offset+1==Buffer_Size &&  Buffer[Buffer_Offset  ]!=0x00)
        Buffer_Offset++;

    if (Buffer_Offset+3>Buffer_Size)
        return false;

    //Synched is OK
    Synched=true;
    return true;
}

//---------------------------------------------------------------------------
bool File__Analyze::FileHeader_Begin_0x000001()
{
    //Element_Size
    if (Buffer_Size<192*4)
        return true; //Not enough buffer for a test

    //Detecting OldDirac/WAV/SWF/FLV/ELF/DPG/WM files
    int64u Magic8=CC8(Buffer);
    int32u Magic4=Magic8>>32;
    int32u Magic3=Magic4>> 8;
    int16u Magic2=Magic4>>16;
    if (Magic8==0x4B572D4449524143LL || Magic4==0x52494646 || Magic3==0x465753 || Magic3==0x464C56 || Magic4==0x7F454C46 || Magic4==0x44504730 || Magic4==0x3026B275 || Magic2==0x4D5A)
    {
        Reject();
        return false;
    }

    //GXF
    if (CC5(Buffer)==0x0000000001 && CC2(Buffer+14)==0xE1E2)
    {
        Reject();
        return false;
    }

    //Detecting MPEG-4 files (ftyp/mdat/skip/free)
    Magic4=CC4(Buffer+4);
    switch (Magic4)
    {
        case 0x66747970 : //ftyp
        case 0x6D646174 : //mdat
        case 0x736B6970 : //skip
        case 0x66726565 : //free
                            Reject();
                            return false;
        default         :   break;
    }

    //Detect TS files, and the parser is not enough precise to detect them later
    size_t Buffer_Offset=0;
    while (Buffer_Offset<188 && Buffer[Buffer_Offset]!=0x47) //Look for first Sync word
        Buffer_Offset++;
    if (Buffer_Offset<188 && Buffer[Buffer_Offset+188]==0x47 && Buffer[Buffer_Offset+188*2]==0x47 && Buffer[Buffer_Offset+188*3]==0x47)
    {
        Status[IsFinished]=true;
        return false;
    }
    Buffer_Offset=0;

    //Detect BDAV files, and the parser is not enough precise to detect them later
    while (Buffer_Offset<192 && CC1(Buffer+Buffer_Offset+4)!=0x47) //Look for first Sync word
        Buffer_Offset++;
    if (Buffer_Offset<192 && CC1(Buffer+Buffer_Offset+192+4)==0x47 && CC1(Buffer+Buffer_Offset+192*2+4)==0x47 && CC1(Buffer+Buffer_Offset+192*3+4)==0x47)
    {
        Status[IsFinished]=true;
        return false;
    }
    Buffer_Offset=0;

    //All should be OK...
    return true;
}

//***************************************************************************
// Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File__Analyze::Synchro_Manage()
{
    //Testing if synchro is OK
    if (Synched)
    {
        if (!Synched_Test())
            return false;
        if (!Synched)
        {
            Element[Element_Level].IsComplete=true; //Else the trusting algo will think it
            Trusted_IsNot("Synchronisation lost");
        }
    }

    //Trying to synchronize
    if (!Synched)
    {
        if (!Synchronize())
        {
            if (Status[IsFinished])
                Finish(); //Finish
            if (!IsSub && File_Offset_FirstSynched==(int64u)-1 && Buffer_TotalBytes+Buffer_Offset>=Buffer_TotalBytes_FirstSynched_Max)
                Reject();
            return false; //Wait for more data
        }
        Synched=true;
        if (File_Offset_FirstSynched==(int64u)-1)
        {
            Synched_Init();
            Buffer_TotalBytes_FirstSynched+=Buffer_TotalBytes+Buffer_Offset;
            File_Offset_FirstSynched=File_Offset+Buffer_Offset;
        }
        return Synchro_Manage();
    }

    return true;
}

//***************************************************************************
// File Header
//***************************************************************************

//---------------------------------------------------------------------------
bool File__Analyze::FileHeader_Manage()
{
    //From the parser
    if (!FileHeader_Begin())
    {
        if (Status[IsFinished]) //Newest parsers set this bool if there is an error
            Reject();
        return false; //Wait for more data
    }

    //From the parser
    Element_Size=Buffer_Size;
    Element_Begin("File Header");
    FileHeader_Parse();
    Element_End();
    if (Status[IsFinished]) //Newest parsers set this bool if there is an error
    {
        Finish();
        return false;
    }

    //Testing the parser result
    if (Element_IsWaitingForMoreData() || Element[Element_Level].UnTrusted) //Wait or problem
    {
        //The header is not complete, need more data
        #if MEDIAINFO_TRACE
        Element[Element_Level].ToShow.Details.clear();
        #endif //MEDIAINFO_TRACE
        return false;
    }

    //Positionning
    Buffer_Offset+=(size_t)Element_Offset;

    MustParseTheHeaderFile=false;
    return true;
}

//***************************************************************************
// Header
//***************************************************************************

//---------------------------------------------------------------------------
bool File__Analyze::Header_Manage()
{
    //Test
    if (Buffer_Offset>=Buffer_Size)
        return false;

    //Header begin
    if (!Header_Begin())
    {
        //Jumping to the end of the file if needed
        if (!EOF_AlreadyDetected && MediaInfoLib::Config.ParseSpeed_Get()<1 && File_GoTo==(int64u)-1)
        {
            Element[Element_Level].WaitForMoreData=false;
            Detect_EOF();
            if ((File_GoTo!=(int64u)-1 && File_GoTo>File_Offset+Buffer_Offset) || (Status[IsFinished] && !ShouldContinueParsing))
                EOF_AlreadyDetected=true;
        }
        return false; //Wait for more data
    }

    //Going in a lower level
    Element_Size=Element[Element_Level].Next-(File_Offset+Buffer_Offset+Element_Offset);
    Element[Element_Level].UnTrusted=false;
    if (Buffer_Offset+Element_Size>Buffer_Size)
    {
        Element_Size=Buffer_Size-Buffer_Offset;
        Element[Element_Level].IsComplete=false;
    }
    else
        Element[Element_Level].IsComplete=true;
    if (Element_Size==0)
        return false;
    Element_Offset=0;
    Element_Begin(); //Element
    #if MEDIAINFO_TRACE
        Data_Level=Element_Level;
    #endif //MEDIAINFO_TRACE
    Element_Begin("Header"); //Header

    //Header parsing
    Header_Parse();

    //Testing the parser result
    if (Element[Element_Level].UnTrusted) //Problem
    {
        Header_Fill_Code(0, "Problem");
        Header_Fill_Size(Element_Size);
    }

    if (Element_IsWaitingForMoreData() || (DataMustAlwaysBeComplete && Element[Element_Level-1].Next>File_Offset+Buffer_Size)) //Wait or want to have a comple data chunk
    {
        //The header is not complete, need more data
        Element[Element_Level].WaitForMoreData=true;
        Element_End(); //Header
        Element_End(); //Element
        return false;
    }

    //Filling
    Element[Element_Level].WaitForMoreData=false;
    Element[Element_Level].IsComplete=true;

    //ToShow
    #if MEDIAINFO_TRACE
    if (Config_DetailsLevel!=0)
    {
        if (Element[Element_Level-1].ToShow.Name.empty())
            Element[Element_Level-1].ToShow.Name=_T("Unknown");
        Element[Element_Level].ToShow.Size=Element_Offset;
        Element[Element_Level].ToShow.Header_Size=0;
        Element[Element_Level-1].ToShow.Header_Size=Header_Size;
    }
    #endif //MEDIAINFO_TRACE

    //Integrity
    if (Element[Element_Level-1].Next<(File_Offset+Buffer_Offset+Element_Offset))
        Element[Element_Level-1].Next=File_Offset+Buffer_Offset+Element_Offset; //Size is not good

    //Positionning
    Element_Size=Element[Element_Level-1].Next-(File_Offset+Buffer_Offset+Element_Offset);
    Header_Size=Element_Offset;
    Buffer_Offset+=(size_t)Header_Size;
    Element_Offset=0;
    if (Buffer_Offset+Element_Size>Buffer_Size)
    {
        if (Buffer_Size>Buffer_Offset)
            Element_Size=Buffer_Size-Buffer_Offset;
        else
            Element_Size=0; //There is an error in the parsing
        Element[Element_Level-1].IsComplete=false;
    }

    Element_End(); //Header
    return true;
}

//---------------------------------------------------------------------------
void File__Analyze::Header_Parse()
{
    //Filling
    Header_Fill_Code(0);
    Header_Fill_Size(Element_Size);
}

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Header_Fill_Code(int64u Code, const Ztring &Name)
{
    //Filling
    Element[Element_Level-1].Code=Code;

    //ToShow
    if (Config_DetailsLevel!=0)
    {
        Element_Level--;
        Element_Name(Name);
        Element_Level++;
    }
}
#endif //MEDIAINFO_TRACE

void File__Analyze::Header_Fill_Code(int64u Code)
{
    //Filling
    Element[Element_Level-1].Code=Code;
}

//---------------------------------------------------------------------------
void File__Analyze::Header_Fill_Size(int64u Size)
{
    if (Size==0)
        Trusted_IsNot("Header can't be 0");

    if (Element[Element_Level].UnTrusted)
        return;

    //Integrity
    if (Size<Element_Offset)
        Size=Element_Offset; //At least what we read before!!!

    //Filling
    if (Element_Level==1)
        Element[0].Next=File_Offset+Buffer_Offset+Size;
    else if (File_Offset+Buffer_Offset+Size>Element[Element_Level-2].Next)
        Element[Element_Level-1].Next=Element[Element_Level-2].Next;
    else
        Element[Element_Level-1].Next=File_Offset+Buffer_Offset+Size;
    Element[Element_Level-1].IsComplete=true;

    //ToShow
    #if MEDIAINFO_TRACE
    if (Config_DetailsLevel!=0)
    {
        Element[Element_Level-1].ToShow.Pos=File_Offset+Buffer_Offset;
        Element[Element_Level-1].ToShow.Size=Element[Element_Level-1].Next-(File_Offset+Buffer_Offset);
    }
    #endif //MEDIAINFO_TRACE
}

//***************************************************************************
// Data
//***************************************************************************

//---------------------------------------------------------------------------
bool File__Analyze::Data_Manage()
{
    Element_WantNextLevel=false;
    if (!Element[Element_Level].UnTrusted)
    {
        Element_Code=Element[Element_Level].Code;
        //size_t Element_Level_Save=Element_Level;
        Data_Parse();
        BS->Attach(NULL, 0); //Clear it
        //Element_Level=Element_Level_Save;

        //Testing the parser result
        if (Element_IsWaitingForMoreData())
        {
            //The data is not complete, need more data
            Element_End(); //Element
            Buffer_Offset-=(size_t)Header_Size;
            return false;
        }

        Element[Element_Level].IsComplete=true;
    }

    if (File_GoTo!=(int64u)-1)
        return false;

    //Next element
    if (!Element_WantNextLevel)
    {
        if (Element[Element_Level].Next<=File_Offset+Buffer_Size)
            Element_Offset=(size_t)(Element[Element_Level].Next-File_Offset-Buffer_Offset);
        else if (!Status[IsFinished])
        {
            GoTo(Element[Element_Level].Next);
            return false;
        }
    }
    if (File_GoTo==(int64u)-1 && Buffer_Offset+Element_Offset>Buffer_Size && File_Offset!=File_Size)
        GoTo(File_Offset+Buffer_Offset+Element_Offset); //Preparing to go far

    //If no need of more
    if (File_GoTo!=(int64u)-1 || (Status[IsFinished] && !ShouldContinueParsing))
    {
        if (!Element_WantNextLevel)
            Element_End(); //Element
        Element_Offset=0;
        return false;
    }

    Buffer_Offset+=(size_t)Element_Offset;
    Header_Size=0;
    Element_Size=0;
    Element_Offset=0;

    #if MEDIAINFO_TRACE
    if (Element_Level>0)
        Element[Element_Level-1].ToShow.NoShow=Element[Element_Level].ToShow.NoShow; //If data must not be shown, we hide the header too
    else
        Element[0].ToShow.NoShow=false; //This should never happen, but in case of
    #endif //MEDIAINFO_TRACE
    Element_End(); //Element
    if (Element_WantNextLevel)
        Element_Level++;
    Element[Element_Level].UnTrusted=false;

    //Jumping to the end of the file if needed
    if (!EOF_AlreadyDetected && MediaInfoLib::Config.ParseSpeed_Get()<1 && File_GoTo==(int64u)-1)
    {
        Element[Element_Level].WaitForMoreData=false;
        Detect_EOF();
        if ((File_GoTo!=(int64u)-1 && File_GoTo>File_Offset+Buffer_Offset) || (Status[IsFinished] && !ShouldContinueParsing))
        {
            EOF_AlreadyDetected=true;
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Data_Info (const Ztring &Parameter)
{
    size_t Element_Level_Temp=Element_Level;
    Element_Level=Data_Level;
    Element_Info(Parameter);
    Element_Level=Element_Level_Temp;
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Data_Accept (const char* ParserName)
{
    if (Status[IsAccepted] || Status[IsFinished])
        return;

    if (ParserName)
        Info(Ztring(ParserName)+_T(", accepted"));

    Accept(ParserName);
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Data_Finish (const char* ParserName)
{
    if (ShouldContinueParsing)
    {
        //if (ParserName)
        //    Info(Ztring(ParserName)+_T(", wants to finish, but should continue parsing"));
        return;
    }

    if (ParserName)
        Info(Ztring(ParserName)+_T(", finished"));

    Finish();
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Data_Reject (const char* ParserName)
{
    Status[IsAccepted]=false;
    Status[IsFinished]=true;
    Clear();

    if (ParserName)// && File_Offset+Buffer_Offset+Element_Size<File_Size)
        Info(Ztring(ParserName)+_T(", rejected"));
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Data_GoTo (int64u GoTo, const char* ParserName)
{
    if (IsSub && MediaInfoLib::Config.ParseSpeed_Get()==1)
        return;

    Element_Show();

    if (GoTo==File_Size)
    {
        ForceFinish();
        return;
    }
    if (File_GoTo!=(int64u)-1)
        return; //Already done

    if (ShouldContinueParsing)
    {
        if (ParserName)
            Info(Ztring(ParserName)+_T(", wants to go to somewhere, but should continue parsing"));
        return;
    }

    if (IsSub)
    {
        if (ParserName)
            Info(Ztring(ParserName)+_T(", wants to go to somewhere, but is sub, waiting data"));
        return;
    }

    Info(Ztring(ParserName)+_T(", jumping to offset ")+Ztring::ToZtring(GoTo, 16));
    if (!IsSub)
        File_GoTo=GoTo;
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Data_GoToFromEnd (int64u GoToFromEnd, const char* ParserName)
{
    if (IsSub && MediaInfoLib::Config.ParseSpeed_Get()==1)
        return;

    if (GoToFromEnd>File_Size)
    {
        if (ParserName)
            Info(Ztring(ParserName)+_T(", wants to go to somewhere, but not valid"));
        return;
    }

    Data_GoTo(File_Size-GoToFromEnd, ParserName);
}
#endif //MEDIAINFO_TRACE


//***************************************************************************
// Element
//***************************************************************************

//---------------------------------------------------------------------------
Ztring Log_Offset (int64u OffsetToShow)
{
    if (OffsetToShow==(int64u)-1)
        return _T("         ");
    int64u Offset=OffsetToShow%0x100000000ULL; //Only 32 bits
    Ztring Pos1; Pos1.From_Number(Offset, 16);
    Ztring Pos2;
    Pos2.resize(8-Pos1.size(), _T('0'));
    Pos2+=Pos1;
    Pos2.MakeUpperCase();
    switch (MediaInfoLib::Config.DetailsFormat_Get())
    {
        case MediaInfo_Config::DetailsFormat_Tree       : Pos2+=_T(' '); break;
        case MediaInfo_Config::DetailsFormat_CSV        : Pos2+=_T(','); break;
        default                                         : ;
    }
    return Pos2;
}

//---------------------------------------------------------------------------
void File__Analyze::Element_Begin()
{
    //Level
    Element_Level++;
    Header_Size=0;

    //Element
    Element[Element_Level].Code=0;
    Element[Element_Level].Next=Element[Element_Level-1].Next;
    Element[Element_Level].WaitForMoreData=Element[Element_Level-1].WaitForMoreData;
    Element[Element_Level].UnTrusted=Element[Element_Level-1].UnTrusted;
    Element[Element_Level].IsComplete=Element[Element_Level-1].IsComplete;

    //ToShow
    #if MEDIAINFO_TRACE
    Element[Element_Level].ToShow.Pos=File_Offset+Buffer_Offset+Element_Offset+BS->OffsetBeforeLastCall_Get(); //TODO: change this, used in Element_End()
    if (Config_DetailsLevel!=0)
    {
        Element[Element_Level].ToShow.Size=Element[Element_Level].Next-(File_Offset+Buffer_Offset+Element_Offset+BS->OffsetBeforeLastCall_Get());
        Element[Element_Level].ToShow.Header_Size=0;
        Element[Element_Level].ToShow.Name.clear();
        Element[Element_Level].ToShow.Info.clear();
        Element[Element_Level].ToShow.Details.clear();
        Element[Element_Level].ToShow.NoShow=false;
    }
    #endif //MEDIAINFO_TRACE
}

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Element_Begin(const Ztring &Name, int64u Size)
{
    //Level
    Element_Level++;

    //Element
    Element[Element_Level].Code=0;
    if (Size==(int64u)-1)
        Element[Element_Level].Next=Element[Element_Level-1].Next;
    else
    {
        Element[Element_Level].Next=File_Offset+Buffer_Offset+Element_Offset+Size;
        if (Element[Element_Level].Next>Element[Element_Level-1].Next)
            Element[Element_Level].Next=Element[Element_Level-1].Next;
    }
    Element[Element_Level].WaitForMoreData=false;
    Element[Element_Level].UnTrusted=Element[Element_Level-1].UnTrusted;
    Element[Element_Level].IsComplete=Element[Element_Level-1].IsComplete;

    //ToShow
    Element[Element_Level].ToShow.Pos=File_Offset+Buffer_Offset+Element_Offset+BS->OffsetBeforeLastCall_Get(); //TODO: change this, used in Element_End()
    if (Config_DetailsLevel!=0)
    {
        Element[Element_Level].ToShow.Size=Element[Element_Level].Next-(File_Offset+Buffer_Offset+Element_Offset+BS->OffsetBeforeLastCall_Get());
        Element[Element_Level].ToShow.Header_Size=0;
        Element_Name(Name);
        Element[Element_Level].ToShow.Info.clear();
        Element[Element_Level].ToShow.Details.clear();
        Element[Element_Level].ToShow.NoShow=false;
    }
}
#else //MEDIAINFO_TRACE
void File__Analyze::Element_Begin(int64u Size)
{
    //Level
    Element_Level++;

    //Element
    Element[Element_Level].Code=0;
    if (Size==(int64u)-1)
        Element[Element_Level].Next=Element[Element_Level-1].Next;
    else
    {
        Element[Element_Level].Next=File_Offset+Buffer_Offset+Element_Offset+Size;
        if (Element[Element_Level].Next>Element[Element_Level-1].Next)
            Element[Element_Level].Next=Element[Element_Level-1].Next;
    }
    Element[Element_Level].WaitForMoreData=false;
    Element[Element_Level].UnTrusted=Element[Element_Level-1].UnTrusted;
    Element[Element_Level].IsComplete=Element[Element_Level-1].IsComplete;
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Element_Name(const Ztring &Name)
{
    //ToShow
    if (Config_DetailsLevel!=0)
    {
        if (!Name.empty())
        {
            Ztring Name2=Name;
            Name2.FindAndReplace(_T("\r\n"), _T("__"), 0, Ztring_Recursive);
            Name2.FindAndReplace(_T("\r"), _T("_"), 0, Ztring_Recursive);
            Name2.FindAndReplace(_T("\n"), _T("_"), 0, Ztring_Recursive);
            if (Name2[0]==_T(' '))
                Name2[0]=_T('_');
            Element[Element_Level].ToShow.Name=Name2;
        }
        else
            Element[Element_Level].ToShow.Name=_T("(Empty)");
    }
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Element_Info(const Ztring &Parameter)
{
    //Coherancy
    if (Config_DetailsLevel==0 || Element[Element_Level].ToShow.Details.size()>64*1024*1024)
        return;

    //Needed?
    if (Config_DetailsLevel<=0.7)
        return;

    //ToShow
    Ztring Parameter2(Parameter);
    Parameter2.FindAndReplace(_T("\r\n"), _T(" / "));
    Parameter2.FindAndReplace(_T("\r"), _T(" / "));
    Parameter2.FindAndReplace(_T("\n"), _T(" / "));
    switch (MediaInfoLib::Config.DetailsFormat_Get())
    {
        case MediaInfo_Config::DetailsFormat_Tree       : Element[Element_Level].ToShow.Info+=_T(" - "); break;
        case MediaInfo_Config::DetailsFormat_CSV        : Element[Element_Level].ToShow.Info+=_T(','); break;
        default                                         : ;
    }
    Element[Element_Level].ToShow.Info+=Parameter2;
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Element_End(const Ztring &Name, int64u Size)
{
    //Element
    if (Size!=(int64u)-1)
    {
        Element[Element_Level].Next=Element[Element_Level].ToShow.Pos+Size; //TODO: change this
        if (Element[Element_Level].Next>Element[Element_Level-1].Next)
            Element[Element_Level].Next=Element[Element_Level-1].Next;
    }

    //ToShow
    if (Config_DetailsLevel!=0)
    {
        Element[Element_Level].ToShow.Size=Element[Element_Level].Next-Element[Element_Level].ToShow.Pos;
        if (!Name.empty())
            Element[Element_Level].ToShow.Name=Name;
    }

    Element_End_Common_Flush();
}
#else //MEDIAINFO_TRACE
void File__Analyze::Element_End(int64u Size)
{
    //Element
    if (Size!=(int64u)-1)
    {
        if (Element[Element_Level].Next>Element[Element_Level-1].Next)
            Element[Element_Level].Next=Element[Element_Level-1].Next;
    }

    Element_End_Common_Flush();
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
void File__Analyze::Element_End()
{
    Element_End_Common_Flush();
}


//***************************************************************************
// Element - Common
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Element_End_Common_Flush()
{
    #if MEDIAINFO_TRACE
    //Size if not filled
    if (File_Offset+Buffer_Offset+Element_Offset+BS->Offset_Get()<Element[Element_Level].Next)
        Element[Element_Level].ToShow.Size=File_Offset+Buffer_Offset+Element_Offset+BS->Offset_Get()-Element[Element_Level].ToShow.Pos;
    #endif //MEDIAINFO_TRACE

    //Level
    if (Element_Level==0)
        return;

    //Element level
    Element_Level--;

    //Element
    Element[Element_Level].UnTrusted=Element[Element_Level+1].UnTrusted;
    Element[Element_Level].WaitForMoreData=Element[Element_Level+1].WaitForMoreData;

    #if MEDIAINFO_TRACE
        Element_End_Common_Flush_Details();
    #endif //MEDIAINFO_TRACE
}

#if MEDIAINFO_TRACE
//---------------------------------------------------------------------------
void File__Analyze::Element_End_Common_Flush_Details()
{
    Element[Element_Level].ToShow.NoShow=Element[Element_Level+1].ToShow.NoShow;
    if (Config_DetailsLevel!=0)
    {
        if (!Element[Element_Level+1].WaitForMoreData && (Element[Element_Level+1].IsComplete || !Element[Element_Level+1].UnTrusted) && !Element[Element_Level+1].ToShow.NoShow)// && Config_DetailsLevel!=0 && Element[Element_Level].ToShow.Details.size()<=64*1024*1024)
        {
            //Element
            if (!Element[Element_Level+1].ToShow.Name.empty())
            {
                if (!Element[Element_Level].ToShow.Details.empty())
                    Element[Element_Level].ToShow.Details+=MediaInfoLib::Config.LineSeparator_Get();
                Element[Element_Level].ToShow.Details+=Element_End_Common_Flush_Build();
                Element[Element_Level+1].ToShow.Name.clear();
            }

            //Info
            if (!Element[Element_Level+1].ToShow.Details.empty())
            {
                if (!Element[Element_Level].ToShow.Details.empty())
                    Element[Element_Level].ToShow.Details+=MediaInfoLib::Config.LineSeparator_Get();
                Element[Element_Level].ToShow.Details+=Element[Element_Level+1].ToShow.Details;
                Element[Element_Level+1].ToShow.Details.clear();
            }
        }
    }
}
#endif //MEDIAINFO_TRACE

#if MEDIAINFO_TRACE
//---------------------------------------------------------------------------
Ztring File__Analyze::Element_End_Common_Flush_Build()
{
    Ztring ToReturn;

    //Show Offset
    if (Config_DetailsLevel>0.7)
    {
        ToReturn+=Log_Offset(Element[Element_Level+1].ToShow.Pos);
    }

    //Name
    switch (MediaInfoLib::Config.DetailsFormat_Get())
    {
        case MediaInfo_Config::DetailsFormat_Tree       : ToReturn.resize(ToReturn.size()+Element_Level_Base+Element_Level, _T(' ')); break;
        case MediaInfo_Config::DetailsFormat_CSV        :
                    ToReturn+=_T("G,");
                    ToReturn+=Ztring::ToZtring(Element_Level_Base+Element_Level);
                    ToReturn+=_T(',');
                    break;
        default                                         : ;
    }
    ToReturn+=Element[Element_Level+1].ToShow.Name;

    //Info
    ToReturn+=Element[Element_Level+1].ToShow.Info;
    Element[Element_Level+1].ToShow.Info.clear();

    //Size
    if (Config_DetailsLevel>0.3)
    {
        switch (MediaInfoLib::Config.DetailsFormat_Get())
        {
            case MediaInfo_Config::DetailsFormat_Tree       :
                    ToReturn+=_T(" (");
                    ToReturn+=Ztring::ToZtring(Element[Element_Level+1].ToShow.Size);
                    if (Element[Element_Level+1].ToShow.Header_Size>0)
                    {
                        ToReturn+=_T("/");
                        ToReturn+=Ztring::ToZtring(Element[Element_Level+1].ToShow.Size-Element[Element_Level+1].ToShow.Header_Size);
                    }
                    ToReturn+=_T(" bytes)");
                    break;
            default                                         : ;
        }
    }

    return ToReturn;
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
void File__Analyze::Element_Prepare (int64u Size)
{
    Element_Offset=0;
    Element_Size=Size;
    #if MEDIAINFO_TRACE
    Element[Element_Level].ToShow.Size=Size;
    #endif //MEDIAINFO_TRACE
}
//***************************************************************************
// Param
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Param(const Ztring& Parameter, const Ztring& Value)
{
    if (Config_DetailsLevel==0)
        return;

    //Position
    int64u Pos=Element_Offset+BS->OffsetBeforeLastCall_Get();

    //Coherancy
    if (Element[Element_Level].UnTrusted)
        return;

    const size_t Padding_Value=40;

    //Line separator
    if (!Element[Element_Level].ToShow.Details.empty())
        Element[Element_Level].ToShow.Details+=MediaInfoLib::Config.LineSeparator_Get();

    //Show Offset
    if (Config_DetailsLevel>0.7)
    {
        Element[Element_Level].ToShow.Details+=Log_Offset(Pos==(int64u)-1?Pos:(File_Offset+Buffer_Offset+Pos));
    }

    //Show Parameter+Value
    switch (MediaInfoLib::Config.DetailsFormat_Get())
    {
        case MediaInfo_Config::DetailsFormat_Tree       :
                    {
                    //Show Parameter
                    Ztring Param; Param=Parameter;
                    if (Param.size()>Padding_Value) Param.resize(Padding_Value);
                    Element[Element_Level].ToShow.Details.resize(Element[Element_Level].ToShow.Details.size()+Element_Level_Base+Element_Level, _T(' '));
                    Element[Element_Level].ToShow.Details+=Param;

                    //Show Value
                    if (!Value.empty())
                    {
                        Element[Element_Level].ToShow.Details+=_T(": ");
                        Element[Element_Level].ToShow.Details.resize(Element[Element_Level].ToShow.Details.size()+Padding_Value-Param.size()-Element_Level+1, _T(' '));
                        Ztring Value2(Value);
                        Value2.FindAndReplace(_T("\r\n"), _T(" / "), 0, Ztring_Recursive);
                        Value2.FindAndReplace(_T("\r"), _T(" / "), 0, Ztring_Recursive);
                        Value2.FindAndReplace(_T("\n"), _T(" / "), 0, Ztring_Recursive);
                        Element[Element_Level].ToShow.Details+=Value2;
                    }
                    }
                    break;
        case MediaInfo_Config::DetailsFormat_CSV        :
                    Element[Element_Level].ToShow.Details+=_T("T,");
                    Element[Element_Level].ToShow.Details+=Ztring::ToZtring(Element_Level_Base+Element_Level);
                    Element[Element_Level].ToShow.Details+=_T(',');
                    Element[Element_Level].ToShow.Details+=Parameter;
                    Element[Element_Level].ToShow.Details+=_T(',');
                    Element[Element_Level].ToShow.Details+=Value;
                    break;
        default                                         : ;
    }
}
#endif //MEDIAINFO_TRACE

//***************************************************************************
// Information
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Info(const Ztring& Value, size_t Element_Level_Minus)
{
    if (MediaInfoLib::Config.DetailsFormat_Get()==MediaInfo_Config::DetailsFormat_CSV)
        return; //Do not display info

    //Handling a different level (only Element_Level_Minus to 1 is currently well supported)
    size_t Element_Level_Final=Element_Level;
    if (Element_Level_Minus<=Element_Level)
    {
        if (Element_Level_Minus==1)
        {
            Element_Level--;
            Element_End_Common_Flush_Details();
            Element_Level++;
        }
        Element_Level_Final-=Element_Level_Minus;
    }

    if (Config_DetailsLevel==0)
        return;

    //Coherancy
    if (Element[Element_Level_Final].UnTrusted)
        return;

    //Line separator
    if (!Element[Element_Level_Final].ToShow.Details.empty())
        Element[Element_Level_Final].ToShow.Details+=MediaInfoLib::Config.LineSeparator_Get();

    //Preparing
    Ztring ToShow; ToShow.resize(Element_Level_Final, _T(' '));
    ToShow+=_T("---   ");
    ToShow+=Value;
    ToShow+=_T("   ---");
    Ztring Separator; Separator.resize(Element_Level_Final, _T(' '));
    Separator.resize(ToShow.size(), _T('-'));

    //Show Offset
    Ztring Offset;
    if (Config_DetailsLevel>0.7)
        Offset=Log_Offset(File_Offset+Buffer_Offset+Element_Offset+BS->Offset_Get());
    Offset.resize(Offset.size()+Element_Level_Base, _T(' '));

    //Show Value
    Element[Element_Level_Final].ToShow.Details+=Offset;
    Element[Element_Level_Final].ToShow.Details+=Separator;
    Element[Element_Level_Final].ToShow.Details+=MediaInfoLib::Config.LineSeparator_Get();
    Element[Element_Level_Final].ToShow.Details+=Offset;
    Element[Element_Level_Final].ToShow.Details+=ToShow;
    Element[Element_Level_Final].ToShow.Details+=MediaInfoLib::Config.LineSeparator_Get();
    Element[Element_Level_Final].ToShow.Details+=Offset;
    Element[Element_Level_Final].ToShow.Details+=Separator;
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Param_Info (const Ztring &Text)
{
    //Coherancy
    if (Element[Element_Level].UnTrusted)
        return;
    if (Config_DetailsLevel==0 || Element[Element_Level].ToShow.Details.size()>64*1024*1024)
        return;

    //Needed?
    if (Config_DetailsLevel<=0.7)
        return;

    //Filling
    Element[Element_Level].ToShow.Details+=_T(" - ")+Text;
}
#endif //MEDIAINFO_TRACE

//***************************************************************************
// Next code planning
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::NextCode_Add (int64u Code)
{
    NextCode[Code]=true;
}

//---------------------------------------------------------------------------
void File__Analyze::NextCode_Clear ()
{
    NextCode.clear();
}

//---------------------------------------------------------------------------
bool File__Analyze::NextCode_Test ()
{
    if (NextCode.find(Element_Code)==NextCode.end())
    {
        Trusted_IsNot("Frames are not in the right order");
        return false;
    }

    return true;
}

//***************************************************************************
// Element trusting
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Trusted_IsNot (const char* Reason)
#else //MEDIAINFO_TRACE
void File__Analyze::Trusted_IsNot ()
#endif //MEDIAINFO_TRACE
{
    if (!Element[Element_Level].UnTrusted)
    {
        #if MEDIAINFO_TRACE
            Param(Reason, 0);
        #endif //MEDIAINFO_TRACE
        Element_Offset=Element_Size;
        BS->Attach(NULL, 0);

        //Enough data?
        if (!Element[Element_Level].IsComplete)
        {
            Element[Element_Level].WaitForMoreData=true;
            return;
        }

        Element[Element_Level].UnTrusted=true;
        Synched=false;
        if (Trusted>0)
            Trusted--;
    }

    if (Trusted==0)
        Reject();
}

//***************************************************************************
// Actions
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Accept (const char* ParserName_Char)
{
    if (Status[IsAccepted] || Status[IsFinished])
        return;

    if (ParserName.empty())
        ParserName.From_Local(ParserName_Char);

    if (!ParserName.empty())
    {
        bool MustElementBegin=Element_Level?true:false;
        if (Element_Level>0)
            Element_End(); //Element
        Info(Ztring(ParserName)+_T(", accepted"));
        if (MustElementBegin)
            Element_Level++;
    }

    Status[IsAccepted]=true;
    //if (!IsSub)
        Stream_Prepare(Stream_General);

    #if MEDIAINFO_EVENTS
        if (!IsSub)
        {
            struct MediaInfo_Event_General_Parser_Selected_0 Event;
            Event.EventCode=MediaInfo_EventCode_Create(MediaInfo_Parser_None, MediaInfo_Event_General_Parser_Selected, 0);
            std::memset(Event.Name, 0, 16);
            if (!ParserName.empty())
                strncpy(Event.Name, ParserName.To_Local().c_str(), 15);
            Config->Event_Send((const int8u*)&Event, sizeof(MediaInfo_Event_General_Parser_Selected_0));
        }
    #endif //MEDIAINFO_EVENTS
}
#else //MEDIAINFO_TRACE
void File__Analyze::Accept ()
{
    if (Status[IsAccepted] || Status[IsFinished])
        return;

    Status[IsAccepted]=true;
    //if (!IsSub)
        Stream_Prepare(Stream_General);

    #if MEDIAINFO_EVENTS
        if (!IsSub)
        {
            struct MediaInfo_Event_General_Parser_Selected_0 Event;
            Event.EventCode=MediaInfo_EventCode_Create(MediaInfo_Parser_None, MediaInfo_Event_General_Parser_Selected, 0);
            memset(Event.Name, 0, 16);
            Config->Event_Send((const int8u*)&Event, sizeof(MediaInfo_Event_General_Parser_Selected_0));
        }
    #endif //MEDIAINFO_EVENTS
}
#endif //MEDIAINFO_TRACE

void File__Analyze::Accept (File__Analyze* Parser)
{
    if (Parser==NULL)
        return;

    Parser->Accept();
}

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Fill (const char* ParserName)
{
    if (Status[IsFilled])
        return;

    if (ParserName)
    {
        bool MustElementBegin=Element_Level?true:false;
        if (Element_Level>0)
            Element_End(); //Element
        Info(Ztring(ParserName)+_T(", filling"));
        if (MustElementBegin)
            Element_Level++;
    }

    if (Status[IsAccepted])
    {
        Streams_Fill();
        Status[IsFilled]=true;
        Status[IsUpdated]=true;
    }
}
#else //MEDIAINFO_TRACE
void File__Analyze::Fill ()
{
    if (Status[IsFilled])
        return;

    if (Status[IsAccepted])
    {
        Streams_Fill();
        Status[IsFilled]=true;
        Status[IsUpdated]=true;
    }
}
#endif //MEDIAINFO_TRACE

void File__Analyze::Fill (File__Analyze* Parser)
{
    if (Parser==NULL)
        return;

    Parser->Fill();
}

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Finish (const char* ParserName)
{
    if (MediaInfoLib::Config.ParseSpeed_Get()==1)
        return;

    ForceFinish(ParserName);
}
#else //MEDIAINFO_TRACE
void File__Analyze::Finish ()
{
    if (MediaInfoLib::Config.ParseSpeed_Get()==1)
        return;

    ForceFinish();
}
#endif //MEDIAINFO_TRACE

void File__Analyze::Finish (File__Analyze* Parser)
{
    if (Parser==NULL)
        return;

    if (File_Offset+Buffer_Offset+Element_Size>=File_Size)
    {
        Element_Size=0;
        Parser->Buffer_Offset=(size_t)(Parser->File_Size-Parser->File_Offset);
    }

    Parser->ForceFinish(); //The base parser wants, and is prepared to it, so nothing can be cancelled --> ForceFinish() instead of Finish()
}

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::ForceFinish (const char* ParserName_Char)
{
    if (ParserName.empty())
        ParserName.From_Local(ParserName_Char);

    if (ShouldContinueParsing)
    {
        if (!ParserName.empty())
        {
            bool MustElementBegin=Element_Level?true:false;
            if (Element_Level>0)
                Element_End(); //Element
            //Info(Ztring(ParserName)+_T(", wants to finish, but should continue parsing"));
            if (MustElementBegin)
                Element_Level++;
        }
        return;
    }

    if (Status[IsFinished])
        return;

    if (!ParserName.empty())
    {
        bool MustElementBegin=Element_Level?true:false;
        if (Element_Level>0)
            Element_End(); //Element
        Info(ParserName+_T(", finished"));
        if (MustElementBegin)
            Element_Level++;
    }

    if (Status[IsAccepted])
        Fill();

    if (Status[IsAccepted])
    {
        Streams_Finish();
        Streams_Finish_Global();
    }

    Status[IsFinished]=true;

    //Real stream size
    if (MediaInfoLib::Config.ParseSpeed_Get()==1 && IsRawStream)
    {
        Fill(StreamKind_Last, StreamPos_Last, "StreamSize", Buffer_TotalBytes, 10, true);
    }
}
#else //MEDIAINFO_TRACE
void File__Analyze::ForceFinish ()
{
    if (ShouldContinueParsing || Status[IsFinished])
        return;

    if (Status[IsAccepted])
        Fill();

    if (Status[IsAccepted])
    {
        Streams_Finish();
        Streams_Finish_Global();
    }

    Status[IsFinished]=true;

    //Real stream size
    if (MediaInfoLib::Config.ParseSpeed_Get()==1 && IsRawStream)
    {
        Fill(StreamKind_Last, StreamPos_Last, "StreamSize", Buffer_TotalBytes, 10, true);
    }
}
#endif //MEDIAINFO_TRACE

void File__Analyze::ForceFinish (File__Analyze* Parser)
{
    if (Parser==NULL)
        return;

    if (File_Offset+Buffer_Offset+Element_Size>=File_Size)
    {
        Element_Size=0;
        Parser->Buffer_Offset=(size_t)(Parser->File_Size-Parser->File_Offset);
    }

    Parser->ForceFinish();
}

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Reject (const char* ParserName)
{
    Status[IsAccepted]=false;
    Status[IsFinished]=true;
    Clear();

    if (ParserName)// && File_Offset+Buffer_Offset+Element_Size<File_Size)
    {
        bool MustElementBegin=Element_Level?true:false;
        if (Element_Level>0)
            Element_End(); //Element
        Info(Ztring(ParserName)+_T(", rejected"));
        if (MustElementBegin)
            Element_Level++;
    }
}
#else //MEDIAINFO_TRACE
void File__Analyze::Reject ()
{
    Status[IsAccepted]=false;
    Status[IsFinished]=true;
    Clear();
}
#endif //MEDIAINFO_TRACE

void File__Analyze::Reject (File__Analyze* Parser)
{
    if (Parser==NULL)
        return;

    Parser->Reject();
}

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::GoTo (int64u GoTo, const char* ParserName)
{
    if (!Status[IsAccepted])
    {
        Reject();
        return;
    }

    Element_Show();

    if (IsSub && MediaInfoLib::Config.ParseSpeed_Get()==1)
        return;

    if (GoTo==File_Size)
    {
        if (!BookMark_Code.empty())
            BookMark_Get();
        if (File_GoTo==(int64u)-1)
            ForceFinish();
        return;
    }
    if (File_GoTo!=(int64u)-1)
        return; //Already done

    if (ShouldContinueParsing)
    {
        if (ParserName)
        {
            bool MustElementBegin=Element_Level?true:false;
            if (Element_Level>0)
                Element_End(); //Element
            Info(Ztring(ParserName)+_T(", wants to go to somewhere, but should continue parsing"));
            if (MustElementBegin)
                Element_Level++;
        }
        return;
    }

    if (IsSub)
    {
        if (ParserName)
        {
            bool MustElementBegin=Element_Level?true:false;
            if (Element_Level>0)
                Element_End(); //Element
            Info(Ztring(ParserName)+_T(", wants to go to somewhere, but is sub, waiting data"));
            if (MustElementBegin)
                Element_Level++;
        }
        return;
    }

    if (ParserName)
    {
        bool MustElementBegin=Element_Level?true:false;
        if (Element_Level>0)
            Element_End(); //Element
        Info(Ztring(ParserName)+_T(", jumping to offset ")+Ztring::ToZtring(GoTo, 16));
        if (MustElementBegin)
            Element_Level++;
    }

    Open_Buffer_Unsynch();
    if (!IsSub)
        File_GoTo=GoTo;

    #if MEDIAINFO_EVENTS
        struct MediaInfo_Event_General_Move_Request_0 Event;
        Event.EventCode=MediaInfo_EventCode_Create(MediaInfo_Parser_None, MediaInfo_Event_General_Move_Request, 0);
        Event.Stream_Offset=File_GoTo;
        Config->Event_Send((const int8u*)&Event, sizeof(MediaInfo_Event_General_Move_Request_0));
    #endif //MEDIAINFO_EVENTS
}
#else //MEDIAINFO_TRACE
void File__Analyze::GoTo (int64u GoTo)
{
    if (!Status[IsAccepted])
    {
        Reject();
        return;
    }

    if (IsSub && MediaInfoLib::Config.ParseSpeed_Get()==1)
        return;

    if (GoTo==File_Size)
    {
        if (!BookMark_Code.empty())
            BookMark_Get();
        if (File_GoTo==(int64u)-1)
            Finish();
        return;
    }
    if (File_GoTo!=(int64u)-1)
        return; //Already done

    if (ShouldContinueParsing)
    {
        return;
    }

    if (IsSub)
    {
        return;
    }

    Open_Buffer_Unsynch();
    if (!IsSub)
        File_GoTo=GoTo;
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::GoToFromEnd (int64u GoToFromEnd, const char* ParserName)
{
    if (GoToFromEnd>File_Size)
    {
        if (ParserName)
        {
            bool MustElementBegin=Element_Level?true:false;
            if (Element_Level>0)
                Element_End(); //Element
            Info(Ztring(ParserName)+_T(", wants to go to somewhere, but not valid"));
            if (MustElementBegin)
                Element_Level++;
        }
        return;
    }

    GoTo(File_Size-GoToFromEnd, ParserName);
}
#else //MEDIAINFO_TRACE
void File__Analyze::GoToFromEnd (int64u GoToFromEnd)
{
    if (GoToFromEnd>File_Size)
        return;

    GoTo(File_Size-GoToFromEnd);
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
int64u File__Analyze::Element_Code_Get (size_t Level)
{
    return Element[Level].Code;
}

//---------------------------------------------------------------------------
int64u File__Analyze::Element_TotalSize_Get (size_t LevelLess)
{
    return Element[Element_Level-LevelLess].Next-(File_Offset+Buffer_Offset);
}

//---------------------------------------------------------------------------
bool File__Analyze::Element_IsComplete_Get ()
{
    return Element[Element_Level].IsComplete;
}

//---------------------------------------------------------------------------
void File__Analyze::Element_ThisIsAList ()
{
    Element_WantNextLevel=true;
}

//---------------------------------------------------------------------------
void File__Analyze::Element_WaitForMoreData ()
{
    //if (File_Offset+Buffer_Size<File_Size)
        Element[Element_Level].WaitForMoreData=true;
}

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Element_DoNotTrust (const char* Reason)
#else //MEDIAINFO_TRACE
void File__Analyze::Element_DoNotTrust ()
#endif //MEDIAINFO_TRACE
{
    Element[Element_Level].WaitForMoreData=false;
    Element[Element_Level].IsComplete=true;
    #if MEDIAINFO_TRACE
        Trusted_IsNot(Reason);
    #else //MEDIAINFO_TRACE
        Trusted_IsNot();
    #endif //MEDIAINFO_TRACE
}

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Element_DoNotShow ()
{
    Element[Element_Level].ToShow.NoShow=true;
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Element_Show ()
{
    Element[Element_Level].ToShow.NoShow=false;
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
bool File__Analyze::Element_Show_Get ()
{
    return !Element[Element_Level].ToShow.NoShow;
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Element_Show_Add (const Ztring &ToShow)
{
    if (ToShow.empty())
        return;

    //Line separator
    if (!Element[Element_Level].ToShow.Details.empty())
        Element[Element_Level].ToShow.Details+=MediaInfoLib::Config.LineSeparator_Get();

    //From Sub
    Element[Element_Level].ToShow.Details+=ToShow;
}
#endif //MEDIAINFO_TRACE

//***************************************************************************
// Status
//***************************************************************************

//---------------------------------------------------------------------------
bool File__Analyze::Element_IsOK ()
{
    return !Element[Element_Level].WaitForMoreData && !Element[Element_Level].UnTrusted;
}

//---------------------------------------------------------------------------
bool File__Analyze::Element_IsNotFinished ()
{
    if (BS->Remain()>0 || Element_Offset+BS->Offset_Get()<Element_Size)
        return true;
    else
        return false;
}

//---------------------------------------------------------------------------
bool File__Analyze::Element_IsWaitingForMoreData ()
{
    return Element[Element_Level].WaitForMoreData;
}

//***************************************************************************
// BookMarks
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::BookMark_Set (size_t Element_Level_ToSet)
{
    Element_Level_ToSet=Element_Level;
    BookMark_Element_Level=Element_Level_ToSet;
    BookMark_Code.resize(BookMark_Element_Level+1);
    BookMark_Next.resize(BookMark_Element_Level+1);
    for (size_t Pos=0; Pos<=BookMark_Element_Level; Pos++)
    {
        BookMark_Code[Pos]=Element[Pos].Code;
        BookMark_Next[Pos]=Element[Pos].Next;
    }
    BookMark_GoTo=File_Offset+Buffer_Offset+Element_Offset;
}

//---------------------------------------------------------------------------
void File__Analyze::BookMark_Get ()
{
    if (!BookMark_Needed())
        return;

    Element_Show();
    while (Element_Level>0)
        Element_End();
    while (Element_Level<BookMark_Element_Level)
        Element_Begin("Restarting parsing...", File_Size);

    for (size_t Pos=0; Pos<=Element_Level; Pos++)
    {
        Element[Pos].Code=BookMark_Code[Pos];
        Element[Pos].Next=BookMark_Next[Pos];
    }
    BookMark_Code.clear();
    BookMark_Next.clear();
    if (File_GoTo==(int64u)-1)
        File_GoTo=BookMark_GoTo;
}

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Details_Clear()
{
    Details->clear();
    Element[0].ToShow.Details.clear();
}
#endif //MEDIAINFO_TRACE

//***************************************************************************
// Demux
//***************************************************************************

#if MEDIAINFO_DEMUX
void File__Analyze::Demux (const int8u* Buffer, size_t Buffer_Size, contenttype Content_Type)
{
    if (!MediaInfoLib::Config.Demux_Get())
        return;

    #if MEDIAINFO_EVENTS
        //Demux
        StreamIDs[StreamIDs_Size-1]=Element_Code;
        struct MediaInfo_Event_Global_Demux_0 Event;
        if (StreamIDs_Size && StreamIDs_Size<17)
             Event.EventCode=MediaInfo_EventCode_Create(ParserIDs[StreamIDs_Size-1], MediaInfo_Event_Global_Demux, 0);
        else
             Event.EventCode=MediaInfo_EventCode_Create(0x00, MediaInfo_Event_Global_Demux, 0);
        Event.Stream_Offset=File_Offset+Buffer_Offset;
        Event.StreamIDs_Size=StreamIDs_Size;
        Event.StreamIDs=(MediaInfo_int64u*)StreamIDs;
        Event.StreamIDs_Width=(MediaInfo_int8u*)StreamIDs_Width;
        Event.ParserIDs=(MediaInfo_int8u* )ParserIDs;
        Event.Content_Type=(int8u)Content_Type;
        Event.Content_Size=Buffer_Size;
        Event.Content=Buffer;
        Config->Event_Send((const int8u*)&Event, sizeof(MediaInfo_Event_Global_Demux_0), IsSub?File_Name_WithoutDemux:File_Name);
    #endif //MEDIAINFO_EVENTS
}
#endif //MEDIAINFO_DEMUX

} //NameSpace

