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
#include "MediaInfo/File__Analyze.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#include "MediaInfo/MediaInfo_Config.h"
#if MEDIAINFO_IBI
    #if MEDIAINFO_SEEK
        #include "MediaInfo/Multiple/File_Ibi.h"
    #endif //MEDIAINFO_SEEK
    #include "MediaInfo/Multiple/File_Ibi_Creation.h"
#endif //MEDIAINFO_IBI
#include <cstring>
using namespace std;
using namespace tinyxml2;
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events_Internal.h"
#endif //MEDIAINFO_EVENTS
#ifdef MEDIAINFO_SSE2_YES
    #ifndef ZENLIB_MEMUTILS_SSE2
        #define ZENLIB_MEMUTILS_SSE2
    #endif //ZENLIB_MEMUTILS_SSE2
    #include "ZenLib/MemoryUtils.h"
#else //MEDIAINFO_SSE2_YES
    #define memcpy_Unaligned_Unaligned std::memcpy
    #define memcpy_Unaligned_Unaligned_Once1024 std::memcpy
#endif //MEDIAINFO_SSE2_YES
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
    //Info for speed optimization
    #if MEDIAINFO_TRACE
        Config_Trace_Level=MediaInfoLib::Config.Trace_Level_Get();
        Config_Trace_Layers=MediaInfoLib::Config.Trace_Layers_Get();
        Config_Trace_Format=MediaInfoLib::Config.Trace_Format_Get();
        Trace_DoNotSave=false;
        Trace_Layers.set();
        Trace_Layers_Update();
    #endif //MEDIAINFO_TRACE
    Config_Demux=MediaInfoLib::Config.Demux_Get();
    Config_LineSeparator=MediaInfoLib::Config.LineSeparator_Get();
    IsSub=false;
    IsRawStream=false;

    //In
    #if MEDIAINFO_EVENTS
        StreamIDs_Size=1;
        ParserIDs[0]=0x0;
        StreamIDs[0]=0;
        StreamIDs_Width[0]=0;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_Level=1; //Frame
        Demux_random_access=false;
        Demux_UnpacketizeContainer=false;
        Demux_IntermediateItemFound=true;
        Demux_Offset=0;
        Demux_TotalBytes=0;
        Demux_CurrentParser=NULL;
        Demux_EventWasSent_Accept_Specific=false;
    #endif //MEDIAINFO_DEMUX
    PTS_DTS_Needed=false;
    PTS_Begin=(int64u)-1;
    #if MEDIAINFO_ADVANCED2
        PTS_Begin_Segment=(int64u)-1;
    #endif //MEDIAINFO_ADVANCED2
    PTS_End=0;
    DTS_Begin=(int64u)-1;
    DTS_End=0;
    Offsets_Pos=(size_t)-1;
    OriginalBuffer=NULL;
    OriginalBuffer_Size=0;
    OriginalBuffer_Capacity=0;
    #if defined(MEDIAINFO_EIA608_YES) || defined(MEDIAINFO_EIA708_YES)
        ServiceDescriptors=NULL;
    #endif

    //Out
    Frame_Count=0;
    Frame_Count_Previous=0;
    Frame_Count_InThisBlock=0;
    Field_Count=0;
    Field_Count_Previous=0;
    Field_Count_InThisBlock=0;
    Frame_Count_NotParsedIncluded=(int64u)-1;
    FrameNumber_PresentationOrder=(int64u)-1;

    //Configuration
    DataMustAlwaysBeComplete=true;
    MustUseAlternativeParser=false;
    MustSynchronize=false;
    CA_system_ID_MustSkipSlices=false;

    //Buffer
    #if MEDIAINFO_SEEK
        Seek_Duration_Detected=false;
    #endif //MEDIAINFO_SEEK
    Buffer=NULL;
    Buffer_Temp=NULL;
    Buffer_Size=0;
    Buffer_Temp_Size=0;
    Buffer_Temp_Size_Max=0;
    Buffer_Offset=0;
    Buffer_Offset_Temp=0;
    Buffer_MinimumSize=0;
    Buffer_MaximumSize=16*1024*1024;
    Buffer_TotalBytes_FirstSynched=0;
    Buffer_TotalBytes_LastSynched=0;
    Buffer_TotalBytes=0;
    if (MediaInfoLib::Config.FormatDetection_MaximumOffset_Get())
        Buffer_TotalBytes_FirstSynched_Max=MediaInfoLib::Config.FormatDetection_MaximumOffset_Get();
    else
        Buffer_TotalBytes_FirstSynched_Max=1024*1024;
    if (Buffer_TotalBytes_FirstSynched_Max<(int64u)-1-64*1024*1024)
        Buffer_TotalBytes_Fill_Max=Buffer_TotalBytes_FirstSynched_Max+64*1024*1024;
    else
        Buffer_TotalBytes_Fill_Max=(int64u)-1;
    Buffer_PaddingBytes=0;
    Buffer_JunkBytes=0;
    Stream_BitRateFromContainer=0;

    //EOF
    EOF_AlreadyDetected=(MediaInfoLib::Config.ParseSpeed_Get()==1.0)?true:false;

    //Synchro
    MustParseTheHeaderFile=true;
    Synched=false;
    UnSynched_IsNotJunk=false;
    MustExtendParsingDuration=false;
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
    if (Config_Trace_Level!=0)
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
    BS=new BitStream_Fast;
    BT=new BitStream_LE;

    //Temp
    Status[IsAccepted]=false;
    Status[IsFilled]=false;
    Status[IsUpdated]=false;
    Status[IsFinished]=false;
    ShouldContinueParsing=false;

    //Events data
    PES_FirstByte_IsAvailable=false;

    //AES
    #if MEDIAINFO_AES
        AES=NULL;
        AES_IV=NULL;
        AES_Decrypted=NULL;
        AES_Decrypted_Size=0;
    #endif //MEDIAINFO_AES

    //MD5
    #if MEDIAINFO_MD5
        MD5=NULL;
        Md5_ParseUpTo=0;
    #endif //MEDIAINFO_MD5

    Unsynch_Frame_Count=(int64u)-1;
    #if MEDIAINFO_IBI
        Config_Ibi_Create=false;
        Ibi_SynchronizationOffset_Current=0;
        Ibi_SynchronizationOffset_BeginOfFrame=0;
        IbiStream=NULL;
    #endif //MEDIAINFO_IBI
}

//---------------------------------------------------------------------------
File__Analyze::~File__Analyze ()
{
    //Buffer
    delete[] Buffer_Temp; //Buffer_Temp=NULL;
    delete[] OriginalBuffer;

    //BitStream
    delete BS; //BS=NULL;
    delete BT; //BS=NULL;

    //AES
    #if MEDIAINFO_AES
        delete AES; //AES=NULL;
        delete AES_IV; //AES_IV=NULL;
        delete AES_Decrypted; //AES_Decrypted=NULL;
    #endif //MEDIAINFO_AES

    //MD5
    #if MEDIAINFO_MD5
        delete MD5; //MD5=NULL;
    #endif //MEDIAINFO_MD5

    #if MEDIAINFO_IBI
        if (!IsSub)
            delete IbiStream; //IbiStream=NULL;
    #endif //MEDIAINFO_IBI
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
        Open_Buffer_Unsynch();
        File_GoTo=(int64u)-1;
    }

    //Configuring
    if (MediaInfoLib::Config.FormatDetection_MaximumOffset_Get())
        Buffer_TotalBytes_FirstSynched_Max=MediaInfoLib::Config.FormatDetection_MaximumOffset_Get();
    Config->ParseSpeed=MediaInfoLib::Config.ParseSpeed_Get();
    if (Config->File_IsSub_Get())
        IsSub=true;
    #if MEDIAINFO_DEMUX
        if (Demux_Level==1 && !IsSub && Config->Demux_Unpacketize_Get()) //If Demux_Level is Frame
        {
            Demux_Level=2; //Container
            Demux_UnpacketizeContainer=true;
        }
    #endif //MEDIAINFO_DEMUX
    #if MEDIAINFO_EVENTS
        if (StreamIDs_Size && IsRawStream)
            StreamIDs[StreamIDs_Size-1]=(int64u)-1;
        if (!IsSub)
        {
            ZtringListList SubFile_IDs;
            SubFile_IDs.Separator_Set(0, EOL);
            SubFile_IDs.Separator_Set(1, __T(","));
            SubFile_IDs.Write(Config->SubFile_IDs_Get());
            if (!SubFile_IDs.empty())
            {
                StreamIDs_Size=1+SubFile_IDs.size();
                StreamIDs[SubFile_IDs.size()]=IsRawStream?(int64u)-1:StreamIDs[0];
                StreamIDs_Width[SubFile_IDs.size()]=StreamIDs_Width[0];
                ParserIDs[SubFile_IDs.size()]=ParserIDs[0];
                for (size_t Pos=0; Pos<SubFile_IDs.size(); Pos++)
                {
                    StreamIDs[Pos]=SubFile_IDs[Pos](0).To_int64u();
                    StreamIDs_Width[Pos]=SubFile_IDs[Pos](1).To_int8u();
                    ParserIDs[Pos]=SubFile_IDs[Pos](2).To_int8u();
                }
            }
        }
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_IBI
        Config_Ibi_Create=Config->Ibi_Create_Get() && Config->ParseSpeed==1.0;
        if (Config_Ibi_Create && !IsSub && IbiStream==NULL)
            IbiStream=new ibi::stream;
    #endif //MEDIAINFO_IBI
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
    Sub->IsSub=true;
    Sub->File_Name_WithoutDemux=IsSub?File_Name_WithoutDemux:File_Name;
    Sub->Open_Buffer_Init(File_Size_);
}

//---------------------------------------------------------------------------
void File__Analyze::Open_Buffer_Continue (const int8u* ToAdd, size_t ToAdd_Size)
{
    //Deleyed events
    #if MEDIAINFO_DEMUX
        if (Config->Events_Delayed_CurrentSource)
        {
            File__Analyze* Temp=Config->Events_Delayed_CurrentSource;
            Config->Events_Delayed_CurrentSource=NULL;
            Config->Event_Accepted(Temp);
            if (Config->Events_Delayed_CurrentSource)
                return;
        }
    #endif //MEDIAINFO_DEMUX

    if (ToAdd_Size)
    {
        Frame_Count_InThisBlock=0;
        Field_Count_InThisBlock=0;
    }

    //MD5
    #if MEDIAINFO_MD5
        if (ToAdd_Size)
        {
            if (!IsSub && !Buffer_Temp_Size && File_Offset==Config->File_Current_Offset && Config->File_Md5_Get())
            {
                delete MD5; MD5=new struct MD5Context;
                MD5Init(MD5);
            }
            if (MD5)
                MD5Update(MD5, ToAdd, (unsigned int)ToAdd_Size);
        }
    #endif //MEDIAINFO_MD5

    //AES
    #if MEDIAINFO_AES
        if (ToAdd_Size)
        {
            if (!IsSub && !Buffer_Temp_Size && File_Offset==Config->File_Current_Offset
             && Config->Encryption_Format_Get()==Encryption_Format_Aes
             && Config->Encryption_Key_Get().size()==16
             && Config->Encryption_Method_Get()==Encryption_Method_Segment
             && Config->Encryption_Mode_Get()==Encryption_Mode_Cbc
             && Config->Encryption_Padding_Get()==Encryption_Padding_Pkcs7
             && Config->Encryption_InitializationVector_Get()=="Sequence number")
            {
                delete AES; AES=new AESdecrypt;
                AES->key128((const unsigned char*)Config->Encryption_Key_Get().c_str());
                AES_IV=new int8u[16];
                int128u2BigEndian(AES_IV, int128u((int64u)Config->File_Names_Pos-1));
            }
            if (AES)
            {
                if (AES_Decrypted_Size<ToAdd_Size)
                {
                    delete AES_Decrypted; AES_Decrypted=new int8u[ToAdd_Size*2];
                    AES_Decrypted_Size=ToAdd_Size*2;
                }
                AES->cbc_decrypt(ToAdd, AES_Decrypted, (int)ToAdd_Size, AES_IV);    //TODO: handle the case where ToAdd_Size is more than 2GB
                if (File_Offset+Buffer_Size+ToAdd_Size>=Config->File_Current_Size && ToAdd_Size)
                {
                    int8u LastByte=AES_Decrypted[ToAdd_Size-1];
                    ToAdd_Size-=LastByte;
                    if (Config->File_Names_Pos && Config->File_Names_Pos-1<Config->File_Sizes.size())
                        Config->File_Sizes[Config->File_Names_Pos-1]-=LastByte;
                    Config->File_Current_Size-=LastByte;
                }
                ToAdd=AES_Decrypted;
            }
        }
    #endif //MEDIAINFO_AES

    //Integrity
    if (Status[IsFinished])
        return;
    //{File F; F.Open(Ztring(__T("d:\\direct"))+Ztring::ToZtring((size_t)this, 16), File::Access_Write_Append); F.Write(ToAdd, ToAdd_Size);}

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
    }

    #if MEDIAINFO_MD5
        //MD5 parsing only
        if (Md5_ParseUpTo>File_Offset+Buffer_Size+ToAdd_Size)
        {
            File_Offset+=ToAdd_Size;
            return; //No need of this piece of data
        }
        if (Md5_ParseUpTo>File_Offset && Md5_ParseUpTo<=File_Offset+ToAdd_Size)
        {
            Buffer_Offset+=(size_t)(Md5_ParseUpTo-File_Offset);
            Md5_ParseUpTo=0;
        }
    #endif //MEDIAINFO_MD5

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
            memcpy_Unaligned_Unaligned(Buffer_Temp, Old, Buffer_Temp_Size);
            delete[] Old; //Old=NULL;
        }

        //Copying buffer
        if (ToAdd_Size>0)
        {
            memcpy_Unaligned_Unaligned(Buffer_Temp+Buffer_Size, ToAdd, ToAdd_Size);
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
    Trusted=(Buffer_Size>2*8*1024?Buffer_Size/8/1024:2)*Trusted_Multiplier; //Never less than 2 acceptable errors

    //Demand to go elsewhere
    if (File_GoTo!=(int64u)-1)
    {
        //The needed offset is in the new buffer
        Buffer_Offset+=(size_t)(File_GoTo-File_Offset);
        File_GoTo=(int64u)-1;
    }

    //Parsing
    if (!IsSub)
    {
        if (Config->File_Size && Config->File_Size!=(int64u)-1)
            Config->State_Set(((float)Buffer_TotalBytes)/Config->File_Size);
        else if (Config->File_Names.size()>1)
            Config->State_Set(((float)Config->File_Names_Pos)/Config->File_Names.size());
    }
    if (Buffer_Size>=Buffer_MinimumSize || File_Offset+Buffer_Size==File_Size) //Parsing only if we have enough buffer
        while (Open_Buffer_Continue_Loop());

    //MD5
    #if MEDIAINFO_MD5
        if (Md5_ParseUpTo>File_Size)
            Md5_ParseUpTo=File_Size;

        if (MD5 && File_Offset+Buffer_Size>=Config->File_Current_Size && Status[IsAccepted])
        {
            unsigned char Digest[16];
            MD5Final(Digest, MD5);
            Ztring Temp;
            Temp+=Ztring().From_CC2(BigEndian2int16u(Digest+ 0));
            Temp+=Ztring().From_CC2(BigEndian2int16u(Digest+ 2));
            Temp+=Ztring().From_CC2(BigEndian2int16u(Digest+ 4));
            Temp+=Ztring().From_CC2(BigEndian2int16u(Digest+ 6));
            Temp+=Ztring().From_CC2(BigEndian2int16u(Digest+ 8));
            Temp+=Ztring().From_CC2(BigEndian2int16u(Digest+10));
            Temp+=Ztring().From_CC2(BigEndian2int16u(Digest+12));
            Temp+=Ztring().From_CC2(BigEndian2int16u(Digest+14));
            Temp.MakeLowerCase();
            const char* MD5Pos=Config->File_Names.size()>1?"Source_List_MD5_Generated":"MD5_Generated";
            if (Config->File_Names_Pos<=1 && !Retrieve(Stream_General, 0, MD5Pos).empty() && Retrieve(Stream_General, 0, MD5Pos)==Temp)
                Clear(Stream_General, 0, MD5Pos);
            Fill(Stream_General, 0, MD5Pos, Temp);
            if (Config->File_Names_Pos<=1)
                (*Stream_More)[Stream_General][0](Ztring().From_Local(MD5Pos), Info_Options)=__T("N NT");

            delete MD5; MD5=NULL;
        }

        if (MD5 && File_GoTo!=(int64u)-1)
        {
            delete MD5; MD5=NULL; //MD5 not possible with a seek
        }

        if (MD5 && Buffer_Offset>Buffer_Size)
        {
            //We need the next data
            Md5_ParseUpTo=File_Offset+Buffer_Offset;
            Buffer_Offset=Buffer_Size;
        }
    #endif //MEDIAINFO_MD5

    //Should parse again?
    if (((File_GoTo==File_Size && File_Size!=(int64u)-1) || File_Offset+Buffer_Offset>=File_Size)
       #if MEDIAINFO_DEMUX
         && !Config->Demux_EventWasSent
        #endif //MEDIAINFO_DEMUX
         )
    {
        if (!BookMark_Code.empty())
            BookMark_Get();

        if (File_GoTo>=File_Size)
        {
            Element_Show(); //If Element_Level is >0, we must show what is in the details buffer
            while (Element_Level>0)
                Element_End0(); //This is Finish, must flush
            Buffer_Clear();
            File_Offset=File_Size;
            if (!IsSub && !Config->File_Names.empty())
            {
                if (Config->File_Sizes.size()>=Config->File_Names.size())
                    Config->File_Current_Size=Config->File_Sizes[Config->File_Names.size()-1];
                Config->File_Current_Offset=Config->File_Current_Size;
                Config->File_Names_Pos=Config->File_Names.size()-1;
            }
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
            Buffer_Clear();
        }
        else
            File_Offset+=Buffer_Offset;

        return;
    }
    #if MEDIAINFO_MD5
        if (Md5_ParseUpTo)
        {
            Buffer_Clear();
            return;
        }
    #endif //MEDIAINFO_MD5
    if (Buffer_Offset>=Buffer_Size
        #if MEDIAINFO_MD5
            && MD5==NULL
        #endif //MEDIAINFO_MD5
            )
    {
        if (Buffer_Offset>Buffer_Size)
            File_GoTo=File_Offset+Buffer_Offset;
        Buffer_Clear();
        return;
    }

    //Buffer handling
    if (Buffer_Size && Buffer_Offset<=Buffer_Size) //all is not used
    {
        if (File_Offset+Buffer_Size>=File_Size //No more data will come
        #if MEDIAINFO_DEMUX
         && !Config->Demux_EventWasSent
        #endif //MEDIAINFO_DEMUX
           )
        {
                ForceFinish();
                #if MEDIAINFO_DEMUX
                    if (Config->Demux_EventWasSent)
                        return;
                #endif //MEDIAINFO_DEMUX
        }

        if (Buffer_Temp_Size==0) //If there was no copy
        {
            #if MEDIAINFO_DEMUX
            if (!IsSub && Config->Demux_EventWasSent && Config->File_Buffer_Repeat_IsSupported && Buffer_Offset==0) //If there was no byte consumed
            {
                Config->File_Buffer_Repeat=true;
            }
            else
            #endif //MEDIAINFO_DEMUX
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
                memcpy_Unaligned_Unaligned(Buffer_Temp, ToAdd+Buffer_Offset, Buffer_Temp_Size);
            }
        }
        else if (Buffer_Offset) //Already a copy, just moving it
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
    if (Buffer_Offset)
    {
        if (Buffer_Offset>=FrameInfo.Buffer_Offset_End && FrameInfo_Next.DTS!=(int64u)-1)
        {
            FrameInfo=FrameInfo_Next;
            FrameInfo_Next=frame_info();
        }

        float64 Ratio=1;
        if (OriginalBuffer)
        {
            Ratio=((float64)OriginalBuffer_Size)/Buffer_Size;
            size_t Temp_Size=(size_t)float64_int64s(((float64)Buffer_Offset)*Ratio);

            OriginalBuffer_Size-=Temp_Size;
            memmove(OriginalBuffer, OriginalBuffer+Buffer_Offset, OriginalBuffer_Size);
        }

        Buffer_Size-=Buffer_Offset;
        File_Offset+=Buffer_Offset;
        if (Buffer_Offset_Temp>=Buffer_Offset)
            Buffer_Offset_Temp-=Buffer_Offset;
        if (FrameInfo.Buffer_Offset_End!=(int64u)-1 && FrameInfo.Buffer_Offset_End>=Buffer_Offset)
            FrameInfo.Buffer_Offset_End-=Buffer_Offset;
        if (FrameInfo_Next.Buffer_Offset_End!=(int64u)-1 && FrameInfo_Next.Buffer_Offset_End>=Buffer_Offset)
            FrameInfo_Next.Buffer_Offset_End-=Buffer_Offset;
        if (!Offsets_Buffer.empty())
        {
            if (Offsets_Buffer.size()>=2 && Offsets_Buffer.size()%2==0 && Offsets_Buffer[0]==Offsets_Buffer[1])
            {
                size_t Pos=Offsets_Buffer.size()-2;
                do
                {
                    if (Offsets_Buffer[Pos]>Buffer_Offset)
                    {
                        Offsets_Buffer[Pos]-=Buffer_Offset;
                        Offsets_Buffer[Pos+1]-=Buffer_Offset;
                    }
                    else
                    {
                        Offsets_Stream[Pos]+=float64_int64s(Buffer_Offset*Ratio/2)-Offsets_Buffer[Pos];
                        Offsets_Stream[Pos+1]+=float64_int64s(Buffer_Offset*Ratio/2)-Offsets_Buffer[Pos+1];
                        Offsets_Buffer[Pos]=0;
                        Offsets_Buffer[Pos+1]=0;
                        Offsets_Buffer.erase(Offsets_Buffer.begin(), Offsets_Buffer.begin()+Pos);
                        Offsets_Stream.erase(Offsets_Stream.begin(), Offsets_Stream.begin()+Pos);
                        if (Offsets_Pos!=(size_t)-1 && Pos)
                        {
                            if (Pos<Offsets_Pos)
                                Offsets_Pos-=Pos;
                            else
                                Offsets_Pos=0;
                        }
                        break;
                    }
                    if (Pos==0)
                        break;
                    Pos-=2;
                }
                while (Pos);
            }
            else
            {
                size_t Pos=Offsets_Buffer.size()-1;
                do
                {
                    if (Offsets_Buffer[Pos]>Buffer_Offset*Ratio)
                        Offsets_Buffer[Pos]-=float64_int64s(Buffer_Offset*Ratio);
                    else
                    {
                        Offsets_Stream[Pos]+=float64_int64s(Buffer_Offset*Ratio)-Offsets_Buffer[Pos];
                        Offsets_Buffer[Pos]=0;
                        Offsets_Buffer.erase(Offsets_Buffer.begin(), Offsets_Buffer.begin()+Pos);
                        Offsets_Stream.erase(Offsets_Stream.begin(), Offsets_Stream.begin()+Pos);
                        if (Offsets_Pos!=(size_t)-1 && Pos)
                        {
                            if (Pos<Offsets_Pos)
                                Offsets_Pos-=Pos;
                            else
                                Offsets_Pos=0;
                        }
                        break;
                    }
                    if (Pos==0)
                        break;
                    Pos--;
                }
                while (Pos);
            }
        }

        Buffer_Offset=0;
    }

    //Is it OK?
    if (Buffer_Size>Buffer_MaximumSize)
    {
        #if MEDIAINFO_MD5
            if (Config->File_Md5_Get() && MD5 && Status[IsAccepted])
            {
                Buffer_Clear();
                Md5_ParseUpTo=File_Size;
            }
            else
        #endif //MEDIAINFO_MD5
                ForceFinish();
        return;
    }
}

void File__Analyze::Open_Buffer_Continue (File__Analyze* Sub, const int8u* ToAdd, size_t ToAdd_Size, bool IsNewPacket, float64 Ratio)
{
    if (Sub==NULL)
        return;

    //Sub
    if (Sub->File_GoTo!=(int64u)-1)
        Sub->File_GoTo=(int64u)-1;
    Sub->File_Offset=File_Offset+Buffer_Offset+Element_Offset;
    if (Sub->File_Size!=File_Size)
    {
        for (size_t Pos=0; Pos<=Sub->Element_Level; Pos++)
            if (Sub->Element[Pos].Next==Sub->File_Size)
                Sub->Element[Pos].Next=File_Size;
        Sub->File_Size=File_Size;
    }
    #if MEDIAINFO_TRACE
        Sub->Element_Level_Base=Element_Level_Base+Element_Level;
    #endif

    //{File F; F.Open(Ztring(__T("d:\\direct"))+Ztring::ToZtring((size_t)this, 16), File::Access_Write_Append); F.Write(ToAdd, ToAdd_Size);}

    //Adaptating File_Offset
    if (Sub!=this && Sub->Buffer_Size<=Sub->File_Offset)
        Sub->File_Offset-=Sub->Buffer_Size;

    //Parsing
    Sub->PES_FirstByte_IsAvailable=PES_FirstByte_IsAvailable;
    Sub->PES_FirstByte_Value=PES_FirstByte_Value;
    if (IsNewPacket && ToAdd_Size)
    {
        if (Offsets_Stream.empty())
        {
            Sub->Offsets_Stream.push_back(File_Offset+float64_int64s((Buffer_Offset+Element_Offset)*Ratio));
            Sub->Offsets_Buffer.push_back(Sub->Buffer_Size);
        }
        else
        {
            if (Offsets_Buffer[0]>=Buffer_Offset-Header_Size && (Sub->Offsets_Stream.empty() || Sub->Offsets_Stream[Sub->Offsets_Stream.size()-1]+Sub->Buffer_Size-Sub->Offsets_Buffer[Sub->Offsets_Stream.size()-1]!=Offsets_Stream[0]))
            {
                if ((Buffer_Offset-Header_Size)*Ratio<Offsets_Buffer[0])
                {
                    Sub->Offsets_Stream.push_back(Offsets_Stream[0]);
                    Sub->Offsets_Buffer.push_back((Sub->OriginalBuffer_Size?Sub->OriginalBuffer_Size:Sub->Buffer_Size)+Offsets_Buffer[0]-(Buffer_Offset+Element_Offset));
                }
                else
                {
                    Sub->Offsets_Stream.push_back(Offsets_Stream[0]+Buffer_Offset+Element_Offset-Offsets_Buffer[0]);
                    Sub->Offsets_Buffer.push_back(Sub->OriginalBuffer_Size?Sub->OriginalBuffer_Size:Sub->Buffer_Size);
                }
            }
            for (size_t Pos=1; Pos<Offsets_Stream.size(); Pos++)
                if (Offsets_Buffer[Pos]>=Buffer_Offset+Element_Offset && Offsets_Buffer[Pos]<Buffer_Offset+Element_Size)
                {
                    if ((Buffer_Offset-Header_Size)*Ratio<Offsets_Buffer[Pos])
                    {
                        Sub->Offsets_Stream.push_back(Offsets_Stream[Pos]);
                        Sub->Offsets_Buffer.push_back((Sub->OriginalBuffer_Size?Sub->OriginalBuffer_Size:Sub->Buffer_Size)+Offsets_Buffer[Pos]-(Buffer_Offset+Element_Offset));
                    }
                    else
                    {
                        Sub->Offsets_Stream.push_back(Offsets_Stream[Pos]+Buffer_Offset+Element_Offset-Offsets_Buffer[Pos]);
                        Sub->Offsets_Buffer.push_back(Sub->OriginalBuffer_Size?Sub->OriginalBuffer_Size:Sub->Buffer_Size);
                    }
                }
        }
    }

    if (Ratio!=1)
    {
        if (Sub->OriginalBuffer_Size+Element_Size-Element_Offset>Sub->OriginalBuffer_Capacity)
        {
            int8u* Temp=Sub->OriginalBuffer;
            Sub->OriginalBuffer_Capacity=(size_t)(Sub->OriginalBuffer_Size+Element_Size-Element_Offset);
            Sub->OriginalBuffer=new int8u[Sub->OriginalBuffer_Capacity];
            memcpy_Unaligned_Unaligned(Sub->OriginalBuffer, Temp, Sub->OriginalBuffer_Size);
            delete[] Temp;
        }
        memcpy_Unaligned_Unaligned(Sub->OriginalBuffer+Sub->OriginalBuffer_Size, Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset));
        Sub->OriginalBuffer_Size+=(size_t)(Element_Size-Element_Offset);
    }

    if (Sub->FrameInfo.DTS!=(int64u)-1)
        Sub->FrameInfo.Buffer_Offset_End=Sub->Buffer_Offset+Sub->Buffer_Size+ToAdd_Size;
    else if (Sub->FrameInfo_Previous.DTS!=(int64u)-1)
        Sub->FrameInfo_Previous.Buffer_Offset_End=Sub->Buffer_Offset+Sub->Buffer_Size+ToAdd_Size;
    if (Sub->FrameInfo_Previous.DTS!=(int64u)-1)
    {
        Sub->FrameInfo_Next=Sub->FrameInfo;
        Sub->FrameInfo=Sub->FrameInfo_Previous;
        Sub->FrameInfo_Previous=frame_info();

        Sub->Frame_Count_Previous=Sub->Frame_Count;
        Sub->Field_Count_Previous=Sub->Field_Count;
    }
     if (Frame_Count_NotParsedIncluded!=(int64u)-1)
         Sub->Frame_Count_NotParsedIncluded=Frame_Count_NotParsedIncluded;
    #if MEDIAINFO_DEMUX
        bool Demux_EventWasSent_Save=Config->Demux_EventWasSent;
        Config->Demux_EventWasSent=false;
    #endif //MEDIAINFO_DEMUX
    Sub->Open_Buffer_Continue(ToAdd, ToAdd_Size);
    #if MEDIAINFO_DEMUX
        if (Demux_EventWasSent_Save)
            Config->Demux_EventWasSent=true;
    #endif //MEDIAINFO_DEMUX
    if (Sub->Buffer_Size)
    {
        Sub->FrameInfo_Previous=Sub->FrameInfo;
        Sub->FrameInfo=Sub->FrameInfo_Next;
        Sub->FrameInfo_Next=frame_info();
    }

    #if MEDIAINFO_TRACE
        if (Trace_Activated)
        {
            //Details handling
            if (!Sub->Element[0].ToShow.Details.empty() && !Trace_DoNotSave)
            {
                //Line separator
                if (!Element[Element_Level].ToShow.Details.empty())
                    Element[Element_Level].ToShow.Details+=Config_LineSeparator;

                //From Sub
                while(Sub->Element_Level)
                    Sub->Element_End0();
                Element[Element_Level].ToShow.Details+=Sub->Element[0].ToShow.Details;
                Sub->Element[0].ToShow.Details.clear();
            }
            else
                Element[Element_Level].ToShow.NoShow=true; //We don't want to show this item because there is no info in it
        }
    #endif
}

//---------------------------------------------------------------------------
bool File__Analyze::Open_Buffer_Continue_Loop ()
{
    //Header
    if (MustParseTheHeaderFile)
    {
        if (!FileHeader_Manage())
            return false; //Wait for more data
        if (Status[IsFinished] || File_GoTo!=(int64u)-1)
            return false; //Finish
    }

    //Parsing specific
    Element_Offset=0;
    Element_Size=Buffer_Size;
    Element[Element_Level].WaitForMoreData=false;
    Read_Buffer_Continue();
    if (Element_IsWaitingForMoreData())
        return false; //Wait for more data
    if (sizeof(size_t)<sizeof(int64u) && Buffer_Offset+Element_Offset>=(int64u)(size_t)-1)
        GoTo(File_Offset+Buffer_Offset+Element_Offset);
    else
        Buffer_Offset+=(size_t)Element_Offset;
    if ((Status[IsFinished] && !ShouldContinueParsing) || Buffer_Offset>Buffer_Size || File_GoTo!=(int64u)-1)
        return false; //Finish
    #if MEDIAINFO_DEMUX
        if (Config->Demux_EventWasSent)
            return false;
    #endif //MEDIAINFO_DEMUX

    //Parsing;
    while (Buffer_Offset<Buffer_Size)
        if (!Buffer_Parse())
            break;
    Buffer_TotalBytes+=Buffer_Offset;

    //Handling of File_GoTo with already buffered data
    #if MEDIAINFO_MD5
        if (File_GoTo==(int64u)-1 && Md5_ParseUpTo && Md5_ParseUpTo>=File_Offset && Md5_ParseUpTo<File_Offset+Buffer_Size)
        {
            File_GoTo=Md5_ParseUpTo;
            Md5_ParseUpTo=0;
        }
    #endif //MEDIAINFO_MD5
    if (File_GoTo!=(int64u)-1 && File_GoTo>=File_Offset && File_GoTo<File_Offset+Buffer_Size)
    {
        if (Buffer_Temp_Size==0) //If there was no copy
        {
            Buffer_Temp_Size=(size_t)(File_Offset+Buffer_Size-File_GoTo);
            if (Buffer_Temp!=NULL && Buffer_Temp_Size_Max<Buffer_Temp_Size)
            {
                delete[] Buffer_Temp; Buffer_Temp=NULL; Buffer_Temp_Size=0; Buffer_Temp_Size_Max=0;
            }
            if (Buffer_Temp==NULL)
            {
                size_t Buffer_Temp_Size_Max_ToAdd=Buffer_Temp_Size>32768?Buffer_Temp_Size:32768;
                if (Buffer_Temp_Size_Max_ToAdd<Buffer_Temp_Size_Max) Buffer_Temp_Size_Max_ToAdd=Buffer_Temp_Size_Max;
                Buffer_Temp_Size_Max=Buffer_Temp_Size_Max_ToAdd;
                Buffer_Temp=new int8u[Buffer_Temp_Size_Max];
            }
            memcpy_Unaligned_Unaligned(Buffer_Temp, Buffer+Buffer_Size-Buffer_Temp_Size, Buffer_Temp_Size);
        }
        else //Already a copy, just moving it
        {
            Buffer_Temp_Size=(size_t)(File_Offset+Buffer_Size-File_GoTo);
            std::memmove(Buffer_Temp, Buffer+Buffer_Size-Buffer_Temp_Size, Buffer_Temp_Size);
        }
        File_Offset+=Buffer_Size-Buffer_Temp_Size;
        Buffer=Buffer_Temp;
        Buffer_Offset=0;
        Buffer_Size=Buffer_Temp_Size;
        File_GoTo=(int64u)-1;

        #if MEDIAINFO_DEMUX
            if (Config->Demux_EventWasSent)
                return false;
        #endif //MEDIAINFO_DEMUX

        return true;
    }

    #if MEDIAINFO_DEMUX
        if (Config->Demux_EventWasSent)
            return false;
    #endif //MEDIAINFO_DEMUX

    //Parsing specific
    Read_Buffer_AfterParsing();

    //Jumping to the end of the file if needed
    if (!IsSub && !EOF_AlreadyDetected && Config->ParseSpeed<1 && Count_Get(Stream_General))
    {
        Element[Element_Level].WaitForMoreData=false;
        Detect_EOF();
        if ((File_GoTo!=(int64u)-1 && File_GoTo>File_Offset+Buffer_Offset) || (Status[IsFinished] && !ShouldContinueParsing))
        {
            EOF_AlreadyDetected=true;
            return false;
        }
    }

    return false;
}

//---------------------------------------------------------------------------
#if MEDIAINFO_SEEK
size_t File__Analyze::Open_Buffer_Seek (size_t Method, int64u Value, int64u ID)
{
    #if MEDIAINFO_DEMUX
        Config->Demux_EventWasSent=false;
    #endif //MEDIAINFO_DEMUX

    size_t ToReturn=Read_Buffer_Seek(Method, Value, ID);

    if (File_GoTo!=(int64u)-1)
        Buffer_Clear();

    return ToReturn;
}
#endif //MEDIAINFO_SEEK

//---------------------------------------------------------------------------
void File__Analyze::Open_Buffer_Position_Set (int64u File_Offset_)
{
    if (File_Offset_==(int64u)-1)
        return;

    File_Offset=File_Offset_-Buffer_Temp_Size;
    File_GoTo=(int64u)-1;
}

//---------------------------------------------------------------------------
#if MEDIAINFO_ADVANCED2
void File__Analyze::Open_Buffer_SegmentChange ()
{
    Read_Buffer_SegmentChange();
}
#endif //MEDIAINFO_ADVANCED2

//---------------------------------------------------------------------------
void File__Analyze::Open_Buffer_Unsynch ()
{
    Status[IsFinished]=false;
    FrameInfo=frame_info();
    FrameInfo_Previous=frame_info();
    FrameInfo_Next=frame_info();
    Frame_Count_NotParsedIncluded=Unsynch_Frame_Count;
    Unsynch_Frame_Count=(int64u)-1;
    PTS_End=0;
    DTS_End=0;
    #if MEDIAINFO_DEMUX
        Demux_IntermediateItemFound=true;
        Demux_Offset=0;
        Demux_TotalBytes=Buffer_TotalBytes;
        Config->Demux_EventWasSent=false;
    #endif //MEDIAINFO_DEMUX

    //Clearing duration
    if (Synched)
    {
        for (size_t StreamKind=(size_t)Stream_General; StreamKind<(size_t)Stream_Menu; StreamKind++)
        {
            size_t StreamPos_Count=Count_Get((stream_t)StreamKind);
            for (size_t StreamPos=0; StreamPos<StreamPos_Count; StreamPos++)
                Clear((stream_t)StreamKind, StreamPos, Fill_Parameter((stream_t)StreamKind, Generic_Duration));
        }
    }

    //if (Synched)
    if (!MustSynchronize || (MustSynchronize && File_Offset_FirstSynched!=(int64u)-1)) //Synched at least once
    {
        Synched=false;
        UnSynched_IsNotJunk=true;
        Read_Buffer_Unsynched();
        Ibi_Read_Buffer_Unsynched();
    }
    Buffer_Clear();

    //Some default values
    if (IsRawStream && File_GoTo==0)
    {
        FrameInfo.DTS=0;
        Frame_Count_NotParsedIncluded=0;
    }
}

//---------------------------------------------------------------------------
void File__Analyze::Open_Buffer_Update ()
{
    if (Status[IsAccepted])
        Streams_Update();

    Status[File__Analyze::IsUpdated]=false;
    for (size_t Pos=File__Analyze::User_16; Pos<File__Analyze::User_16+16; Pos++)
        Status[Pos]=false;
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
        Element_End0();

    //Buffer - Global
    Fill();
    if (!NoBufferModification)
    {
        ForceFinish();
        #if MEDIAINFO_DEMUX
            if (Config->Demux_EventWasSent)
                return;
        #endif //MEDIAINFO_DEMUX
        Buffer_Clear();
    }

    #if MEDIAINFO_TRACE
    if (Details && Details->empty())
        Details->assign(Element[0].ToShow.Details);
    #endif //MEDIAINFO_TRACE

    #if MEDIAINFO_EVENTS
        if (Status[IsAccepted])
        {
            EVENT_BEGIN (General, End, 0)
                if (Event.StreamIDs_Size>=1)
                    Event.StreamIDs[Event.StreamIDs_Size-1]=(int64u)-1;
                Event.PCR=(int64u)-1;
                Event.DTS=(int64u)-1;
                Event.PTS=(int64u)-1;
                Event.DUR=(int64u)-1;
                Event.Stream_Bytes_Analyzed=Buffer_TotalBytes;
                Event.Stream_Size=File_Size;
                Event.Stream_Bytes_Padding=Buffer_PaddingBytes;
                Event.Stream_Bytes_Junk=Buffer_JunkBytes;
                if (!IsSub && MustSynchronize && !Synched && !UnSynched_IsNotJunk)
                    Event.Stream_Bytes_Junk+=Buffer_TotalBytes+Buffer_Offset-Buffer_TotalBytes_LastSynched;
            EVENT_END   ()
        }
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
#if MEDIAINFO_SEEK
size_t File__Analyze::Read_Buffer_Seek (size_t Method, int64u Value, int64u ID)
{
    #if MEDIAINFO_IBI
        if (!IsSub)
        {
            size_t ReturnValue=Ibi_Read_Buffer_Seek(Method, Value, ID);
            if (ReturnValue!=(size_t)-1) // If IBI file is supported
                return ReturnValue;
        }
    #endif //MEDIAINFO_IBI

    //Parsing
    switch (Method)
    {
        case 0  :   //Default stream seek (byte offset)
                    GoTo(Value);
                    Open_Buffer_Unsynch();
                    return 1;
        case 1  :   //Default stream seek (percentage)
                    GoTo(File_Size*Value/10000);
                    Open_Buffer_Unsynch();
                    return 1;
        default :
                    return (size_t)-1; //Not supported
    }
}
#endif //MEDIAINFO_SEEK

//---------------------------------------------------------------------------
#if MEDIAINFO_SEEK
size_t File__Analyze::Read_Buffer_Seek_OneFramePerFile (size_t Method, int64u Value, int64u ID)
{
    //Parsing
    switch (Method)
    {
        case 0  :
                    {
                    if (Value>=Config->File_Size)
                        return 2; //Invalid value
                    int64u Offset=0;
                    for (size_t Pos=0; Pos<Config->File_Sizes.size(); Pos++)
                    {
                        Offset+=Config->File_Sizes[Pos];
                        if (Offset>=Value)
                        {
                            Offset-=Config->File_Sizes[Pos];
                            break;
                        }
                    }
                    GoTo(Offset);
                    Open_Buffer_Unsynch();
                    return 1;
                    }
        case 1  :
                    {
                    if (Value>=10000)
                        return 2; //Invalid value
                    size_t FilePos=(size_t)((((float32)Value)/10000)*Config->File_Sizes.size());
                    int64u Offset=0;
                    for (size_t Pos=0; Pos<FilePos; Pos++)
                        Offset+=Config->File_Sizes[Pos];
                    GoTo(Offset);
                    Open_Buffer_Unsynch();
                    return 1;
                    }
        case 2  :   //Timestamp
                    #if MEDIAINFO_DEMUX
                        if (Config->Demux_Rate_Get()==0)
                            return (size_t)-1; //Not supported
                        Value=float64_int64s(((float64)Value)/1000000000*Config->Demux_Rate_Get());
                    #else //MEDIAINFO_DEMUX
                        return (size_t)-1; //Not supported
                    #endif //MEDIAINFO_DEMUX
        case 3  :   //FrameNumber
                    {
                    if (Value>=Config->File_Names.size())
                        return 2; //Invalid value
                    int64u Offset=0;
                    if (Config->File_Sizes.size()!=Config->File_Names.size())
                    {
                        Offset=Value; //File_GoTo is the frame offset in that case
                        Config->File_GoTo_IsFrameOffset=true;
                    }
                    else
                        for (size_t Pos=0; Pos<Value; Pos++)
                            Offset+=Config->File_Sizes[Pos];
                    GoTo(Offset);
                    Open_Buffer_Unsynch();
                    return 1;
                    }
        default :   return (size_t)-1; //Not supported
    }
}
#endif //MEDIAINFO_SEEK

//---------------------------------------------------------------------------
void File__Analyze::Read_Buffer_Unsynched_OneFramePerFile()
{
    #if MEDIAINFO_ADVANCED
        if (Config->File_Sizes.size()!=Config->File_Names.size())
        {
            Frame_Count_NotParsedIncluded=File_GoTo;
        }
        else
    #endif //MEDIAINFO_ADVANCED
    {
        int64u GoTo=File_GoTo;
        for (Frame_Count_NotParsedIncluded=0; Frame_Count_NotParsedIncluded<Config->File_Sizes.size(); Frame_Count_NotParsedIncluded++)
        {
            if (GoTo>=Config->File_Sizes[(size_t)Frame_Count_NotParsedIncluded])
                GoTo-=Config->File_Sizes[(size_t)Frame_Count_NotParsedIncluded];
            else
                break;
        }
    }

    #if MEDIAINFO_DEMUX
        if (Config->Demux_Rate_Get())
        {
            FrameInfo.DTS=float64_int64s(Frame_Count_NotParsedIncluded*((float64)1000000000)/Config->Demux_Rate_Get());
            FrameInfo.PTS=FrameInfo.DTS;
        }
    #endif //MEDIAINFO_DEMUX
}

//---------------------------------------------------------------------------
bool File__Analyze::Buffer_Parse()
{
    //End of this level?
    if (File_Offset+Buffer_Offset>=Element[Element_Level].Next)
    {
        //There is no loop handler, so we make the level down here
        while (Element_Level>0 && File_Offset+Buffer_Offset>=Element[Element_Level].Next)
            Element_End0(); //This is a buffer restart, must sync to Element level
        if (File_Offset+Buffer_Offset==File_Size)
            return false; //End of file
        MustUseAlternativeParser=false; //Reset it if we go out of an element
    }

    //Synchro
    if (MustSynchronize)
        do
        {
            if (!Synchro_Manage())
                return false; //Wait for more data
        }
        while (!Synched);
    #if MEDIAINFO_DEMUX
    else if (Demux_TotalBytes<=Buffer_TotalBytes+Buffer_Offset)
    {
        if (Demux_UnpacketizeContainer && !Demux_UnpacketizeContainer_Test())
        {
            Demux_Offset-=Buffer_Offset;
            return false; //Wait for more data
        }
        if (Config->Demux_EventWasSent)
            return false;
    }
    #endif //MEDIAINFO_DEMUX

    //Offsets
    if (Offsets_Pos==(size_t)-1 && !Offsets_Buffer.empty())
        Offsets_Pos=0;
    if (Offsets_Pos!=(size_t)-1)
    {
        while (Offsets_Pos<Offsets_Buffer.size() && Buffer_Offset>Offsets_Buffer[Offsets_Pos])
            Offsets_Pos++;
        if (Offsets_Pos>=Offsets_Buffer.size() || Buffer_Offset!=Offsets_Buffer[Offsets_Pos])
            Offsets_Pos--;
    }

    //Header
    if (!Header_Manage())
        return false; //Wait for more data

    //Data
    if (!Data_Manage())
        return false; //Wait for more data

    Buffer_TotalBytes_LastSynched=Buffer_TotalBytes+Buffer_Offset;

    return true;
}

//---------------------------------------------------------------------------
void File__Analyze::Buffer_Clear()
{
    //Buffer
    BS->Attach(NULL, 0);
    delete[] Buffer_Temp; Buffer_Temp=NULL;
    if (!Status[IsFinished])
        File_Offset+=Buffer_Size;
    else
    {
        File_Offset=File_Size;
        if (!IsSub && !Config->File_Names.empty())
        {
            if (Config->File_Sizes.size()>=Config->File_Names.size())
                Config->File_Current_Size=Config->File_Sizes[Config->File_Names.size()-1];
            Config->File_Current_Offset=Config->File_Current_Size;
            Config->File_Names_Pos=Config->File_Names.size()-1;
        }
    }
    Buffer_Size=0;
    Buffer_Temp_Size=0;
    Buffer_Offset=0;
    Buffer_Offset_Temp=0;
    Buffer_MinimumSize=0;

    OriginalBuffer_Size=0;
    Offsets_Stream.clear();
    Offsets_Buffer.clear();
    Offsets_Pos=(size_t)-1;

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
        if ((Buffer_Offset<Buffer_Size && Buffer[Buffer_Offset-1]==0x00) || Buffer_Offset>=Buffer_Size)
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
    if (Magic8==0x4B572D4449524143LL || Magic4==0x52494646 || Magic3==0x465753 || Magic3==0x464C56 || Magic4==0x7F454C46 || Magic4==0x44504730 || Magic4==0x3026B275 || Magic2==0x4D5A || Magic4==0x1A45DFA3)
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

//---------------------------------------------------------------------------
bool File__Analyze::FileHeader_Begin_XML(XMLDocument &Document)
{
    //Element_Size
    if (!IsSub && (File_Size<32 || File_Size>16*1024*1024))
    {
        Reject();
        return false; //XML files are not expected to be so big
    }

    //Element_Size
    if (!IsSub && Buffer_Size<File_Size)
    {
        Element_WaitForMoreData();
        return false; //Must wait for more data
    }

    //XML header
    Ztring Data;
         if ((Buffer[0]=='<'
           && Buffer[1]==0x00)
          || (Buffer[0]==0xFF
           && Buffer[1]==0xFE
           && Buffer[2]=='<'
           && Buffer[3]==0x00))
        Data.From_UTF16LE((const char*)Buffer, Buffer_Size);
    else if ((Buffer[0]==0x00
           && Buffer[1]=='<')
          || (Buffer[0]==0xFE
           && Buffer[1]==0xFF
           && Buffer[2]==0x00
           && Buffer[3]=='<'))
        Data.From_UTF16BE((const char*)Buffer, Buffer_Size);
    else if ((Buffer[0]=='<')
          || (Buffer[0]==0xEF
           && Buffer[1]==0xBB
           && Buffer[2]==0xBF
           && Buffer[3]=='<'))
        Data.From_UTF8((const char*)Buffer, Buffer_Size);
    else
    {
        Reject();
        return false;
    }

    string DataUTF8=Data.To_UTF8();
    if (Document.Parse(DataUTF8.c_str()))
    {
        Reject();
        return false;
    }

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
        if (!IsSub)
            Buffer_TotalBytes_LastSynched=Buffer_TotalBytes+Buffer_Offset;

        if (!Synchro_Manage_Test())
            return false;
    }

    //Trying to synchronize
    if (!Synched)
    {
        //Buffer_TotalBytes_Fill_Max
        if (!Status[IsFilled] && Buffer_TotalBytes>=Buffer_TotalBytes_Fill_Max)
        {
            Open_Buffer_Unsynch();
            GoToFromEnd(0);
            return false;
        }
        if (!Synchronize())
        {
            if (Status[IsFinished])
                Finish(); //Finish
            if (!IsSub && File_Offset_FirstSynched==(int64u)-1 && Buffer_TotalBytes+Buffer_Offset>=Buffer_TotalBytes_FirstSynched_Max)
            {
                Open_Buffer_Unsynch();
                GoToFromEnd(0);
            }
            return false; //Wait for more data
        }
        Synched=true;
        if (!IsSub)
        {
            if (!UnSynched_IsNotJunk)
                Buffer_JunkBytes+=Buffer_TotalBytes+Buffer_Offset-Buffer_TotalBytes_LastSynched;
            Buffer_TotalBytes_LastSynched=Buffer_TotalBytes+Buffer_Offset;
            UnSynched_IsNotJunk=false;
        }
        if (File_Offset_FirstSynched==(int64u)-1)
        {
            Synched_Init();
            Buffer_TotalBytes_FirstSynched+=Buffer_TotalBytes+Buffer_Offset;
            File_Offset_FirstSynched=File_Offset+Buffer_Offset;
        }
        #if MEDIAINFO_DEMUX
            if (Config->Demux_EventWasSent)
                return false;
        #endif //MEDIAINFO_DEMUX
        if (!Synchro_Manage_Test())
            return false;
    }

    return true;
}

//---------------------------------------------------------------------------
bool File__Analyze::Synchro_Manage_Test()
{
    //Testing if synchro is OK
    if (Synched)
    {
        if (!Synched_Test())
            return false;
        #if MEDIAINFO_DEMUX
            if (Synched && Demux_TotalBytes<=Buffer_TotalBytes+Buffer_Offset)
            {
                if (Demux_UnpacketizeContainer && !Demux_UnpacketizeContainer_Test())
                {
                    Demux_Offset-=Buffer_Offset;
                    return false; //Wait for more data
                }
                if (Config->Demux_EventWasSent)
                    return false;
            }
        #endif //MEDIAINFO_DEMUX
        if (Buffer_Offset>=FrameInfo.Buffer_Offset_End && FrameInfo_Next.DTS!=(int64u)-1)
        {
            FrameInfo=FrameInfo_Next;
            FrameInfo_Next=frame_info();
        }
        if (Synched)
        {
            if (!IsSub)
                Buffer_TotalBytes_LastSynched=Buffer_TotalBytes+Buffer_Offset;
        }
        else
        {
            Element[Element_Level].IsComplete=true; //Else the trusting algo will think it
            Trusted_IsNot("Synchronisation lost");
            while (Element_Level)
                Element_End();
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
        if (!IsSub)
        {
            if (!UnSynched_IsNotJunk)
                Buffer_JunkBytes+=Buffer_TotalBytes+Buffer_Offset-Buffer_TotalBytes_LastSynched;
            Buffer_TotalBytes_LastSynched=Buffer_TotalBytes+Buffer_Offset;
            UnSynched_IsNotJunk=false;
        }
        if (File_Offset_FirstSynched==(int64u)-1)
        {
            Synched_Init();
            Buffer_TotalBytes_FirstSynched+=Buffer_TotalBytes+Buffer_Offset;
            File_Offset_FirstSynched=File_Offset+Buffer_Offset;
        }
        if (!Synched_Test())
            return false;
        #if MEDIAINFO_DEMUX
            if (Synched && Demux_TotalBytes<=Buffer_TotalBytes+Buffer_Offset)
            {
                if (Demux_UnpacketizeContainer && !Demux_UnpacketizeContainer_Test())
                {
                    Demux_Offset-=Buffer_Offset;
                    return false; //Wait for more data
                }
                if (Config->Demux_EventWasSent)
                    return false;
            }
        #endif //MEDIAINFO_DEMUX
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
    if (!Status[IsAccepted] && !FileHeader_Begin())
    {
        if (Status[IsFinished]) //Newest parsers set this bool if there is an error
            Reject();
        if (File_Offset+Buffer_Size>=File_Size)
            Reject();
        return false; //Wait for more data
    }

    //Positionning
    if ((Buffer_Size && Buffer_Offset+Element_Offset>Buffer_Size) || (sizeof(size_t)<sizeof(int64u) && Buffer_Offset+Element_Offset>=(int64u)(size_t)-1))
    {
        GoTo(File_Offset+Buffer_Offset+Element_Offset);
        return false;
    }
    else
    {
        Buffer_Offset+=(size_t)Element_Offset;
        Element_Offset=0;
    }

    #if MEDIAINFO_DEMUX
        if (Config->Demux_EventWasSent)
            return false;
    #endif //MEDIAINFO_DEMUX

    //From the parser
    Element_Size=Buffer_Size-Buffer_Offset;
    Element_Begin1("File Header");
    FileHeader_Parse();
    if (Element_Offset==0)
        Element_DoNotShow();
    Element_End0();
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
    if ((Buffer_Size && Buffer_Offset+Element_Offset>Buffer_Size) || (sizeof(size_t)<sizeof(int64u) && Buffer_Offset+Element_Offset>=(int64u)(size_t)-1))
    {
        GoTo(File_Offset+Buffer_Offset+Element_Offset);
        return false;
    }
    else
    {
        Buffer_Offset+=(size_t)Element_Offset;
        Element_Offset=0;
    }

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
    Element_Size=Element[Element_Level].Next-(File_Offset+Buffer_Offset);
    Element_Offset=0;
    if (!Header_Begin())
    {
        //Jumping to the end of the file if needed
        if (!EOF_AlreadyDetected && Config->ParseSpeed<1 && File_GoTo==(int64u)-1)
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
    Element_Begin0(); //Element
    #if MEDIAINFO_TRACE
        Data_Level=Element_Level;
    #endif //MEDIAINFO_TRACE
    Element_Begin1("Header"); //Header

    //Header parsing
    Header_Parse();

    //Testing the parser result
    if (Element[Element_Level].UnTrusted) //Problem
    {
        Element[Element_Level].UnTrusted=false;
        Header_Fill_Code(0, "Problem");
        if (MustSynchronize)
        {
            //Unsynchronizing to the next byte
            Element_Offset=1;
            Header_Fill_Size(1);
            Synched=false;
        }
        else
        {
            //Can not synchronize anymore in this block
            Element_Offset=Element[Element_Level-2].Next-(File_Offset+Buffer_Offset);
            Header_Fill_Size(Element_Offset);
        }
    }

    if (Element_IsWaitingForMoreData() || ((DataMustAlwaysBeComplete && Element[Element_Level-1].Next>File_Offset+Buffer_Size) || File_GoTo!=(int64u)-1) //Wait or want to have a comple data chunk
        #if MEDIAINFO_DEMUX
            || (Config->Demux_EventWasSent)
        #endif //MEDIAINFO_DEMUX
    )
    {
        //The header is not complete, need more data
        Element[Element_Level].WaitForMoreData=true;
        Element_End0(); //Header
        Element_End0(); //Element
        return false;
    }

    //Filling
    Element[Element_Level].WaitForMoreData=false;
    Element[Element_Level].IsComplete=true;

    //ToShow
    #if MEDIAINFO_TRACE
    if (Trace_Activated)
    {
        if (Element[Element_Level-1].ToShow.Name.empty())
            Element[Element_Level-1].ToShow.Name=__T("Unknown");
        Element[Element_Level].ToShow.Size=Element_Offset;
        Element[Element_Level].ToShow.Header_Size=0;
        Element[Element_Level-1].ToShow.Header_Size=Header_Size;
        if (Element_Offset==0)
            Element_DoNotShow();
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

    Element_End0(); //Header
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
    if (Config_Trace_Level!=0)
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
        Trusted_IsNot("Block can't have a size of 0");
    if (DataMustAlwaysBeComplete && Size>Buffer_MaximumSize)
    {
        Element[Element_Level].IsComplete=true;
        Element[Element_Level-1].IsComplete=true;
        Trusted_IsNot("Block is too big");
    }

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
    if (Trace_Activated)
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

        if (Buffer_Offset+(Element_WantNextLevel?Element_Offset:Element_Size)>=FrameInfo.Buffer_Offset_End)
        {
            if (Frame_Count_Previous<Frame_Count)
                Frame_Count_Previous=Frame_Count;
            if (Field_Count_Previous<Field_Count)
                Field_Count_Previous=Field_Count;
        }

        if (Buffer_Offset+(Element_WantNextLevel?Element_Offset:Element_Size)>=FrameInfo.Buffer_Offset_End && FrameInfo_Next.DTS!=(int64u)-1)
        {
            FrameInfo=FrameInfo_Next;
            FrameInfo_Next=frame_info();
        }

        //Testing the parser result
        if (Element_IsWaitingForMoreData())
        {
            //The data is not complete, need more data
            Element_End0(); //Element
            Buffer_Offset-=(size_t)Header_Size;
            return false;
        }

        Element[Element_Level].IsComplete=true;

        if (!Element_WantNextLevel && DataMustAlwaysBeComplete && Element_Offset<Element_Size)
            Element_Offset=Element_Size; //In case the element is not fully parsed, an element with size from the header is assumed
    }

    //If no need of more
    if (File_GoTo!=(int64u)-1 || (Status[IsFinished] && !ShouldContinueParsing)
        #if MEDIAINFO_MD5
            || Md5_ParseUpTo
        #endif //MEDIAINFO_MD5
        )
    {
        if (!Element_WantNextLevel)
            Element_End0(); //Element
        if (!Element_WantNextLevel && Element_Offset<Element_Size)
            Buffer_Offset+=(size_t)Element_Size;
        else
        {
            if (sizeof(size_t)<sizeof(int64u) && Buffer_Offset+Element_Offset>=(int64u)(size_t)-1)
                GoTo(File_Offset+Buffer_Offset+Element_Offset);
            else
                Buffer_Offset+=(size_t)Element_Offset;
        }
        Header_Size=0;
        Element_Size=0;
        Element_Offset=0;
        return false;
    }

    //Next element
    if (!Element_WantNextLevel
        #if MEDIAINFO_MD5
            && MD5==NULL
        #endif //MEDIAINFO_MD5
            )
    {
        if (Element[Element_Level].Next<=File_Offset+Buffer_Size)
        {
            if (Element_Offset<(size_t)(Element[Element_Level].Next-File_Offset-Buffer_Offset))
                Element_Offset=(size_t)(Element[Element_Level].Next-File_Offset-Buffer_Offset);
        }
        else if (!Status[IsFinished])
        {
            GoTo(Element[Element_Level].Next);
            if (!Element_WantNextLevel)
                Element_End0(); //Element
            return false;
        }
    }

    if (!Element_WantNextLevel && Element_Offset<Element_Size)
        Buffer_Offset+=(size_t)Element_Size;
    else
    {
        if (sizeof(size_t)<sizeof(int64u) && Buffer_Offset+Element_Offset>=(int64u)(size_t)-1)
            GoTo(File_Offset+Buffer_Offset+Element_Offset);
        else
            Buffer_Offset+=(size_t)Element_Offset;
    }
    Header_Size=0;
    Element_Size=0;
    Element_Offset=0;

    #if MEDIAINFO_DEMUX
        if (Config->Demux_EventWasSent)
        {
            if (!Element_WantNextLevel)
                Element_End0();
            return false;
        }
    #endif //MEDIAINFO_DEMUX

    #if MEDIAINFO_TRACE
    if (Element_Level>0)
        Element[Element_Level-1].ToShow.NoShow=Element[Element_Level].ToShow.NoShow; //If data must not be shown, we hide the header too
    else
        Element[0].ToShow.NoShow=false; //This should never happen, but in case of
    #endif //MEDIAINFO_TRACE
    Element_End0(); //Element
    if (Element_WantNextLevel)
        Element_Level++;
    Element[Element_Level].UnTrusted=false;

    //Jumping to the end of the file if needed
    if (!EOF_AlreadyDetected && Config->ParseSpeed<1 && File_GoTo==(int64u)-1)
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
        Info(Ztring(ParserName)+__T(", accepted"));

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
        //    Info(Ztring(ParserName)+__T(", wants to finish, but should continue parsing"));
        return;
    }

    if (ParserName)
        Info(Ztring(ParserName)+__T(", finished"));

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
        Info(Ztring(ParserName)+__T(", rejected"));
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Data_GoTo (int64u GoTo_, const char* ParserName)
{
    Element_Show();

    if (ShouldContinueParsing)
    {
        if (ParserName)
            Info(Ztring(ParserName)+__T(", wants to go to somewhere, but should continue parsing"));
        return;
    }

    if (IsSub)
    {
        if (ParserName)
            Info(Ztring(ParserName)+__T(", wants to go to somewhere, but is sub, waiting data"));
        return;
    }

    Info(Ztring(ParserName)+__T(", jumping to offset ")+Ztring::ToZtring(GoTo_, 16));
    GoTo(GoTo_);
    Element_End0();
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Data_GoToFromEnd (int64u GoToFromEnd, const char* ParserName)
{
    if (IsSub && Config->ParseSpeed==1)
        return;

    if (GoToFromEnd>File_Size)
    {
        if (ParserName)
            Info(Ztring(ParserName)+__T(", wants to go to somewhere, but not valid"));
        return;
    }

    Data_GoTo(File_Size-GoToFromEnd, ParserName);
}
#endif //MEDIAINFO_TRACE


//***************************************************************************
// Element
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
Ztring Log_Offset (int64u OffsetToShow, MediaInfo_Config::trace_Format Config_Trace_Format)
{
    if (OffsetToShow==(int64u)-1)
        return __T("         ");
    int64u Offset=OffsetToShow%0x100000000ULL; //Only 32 bits
    Ztring Pos1; Pos1.From_Number(Offset, 16);
    Ztring Pos2;
    Pos2.resize(8-Pos1.size(), __T('0'));
    Pos2+=Pos1;
    Pos2.MakeUpperCase();
    switch (Config_Trace_Format)
    {
        case MediaInfo_Config::Trace_Format_Tree        : Pos2+=__T(' '); break;
        case MediaInfo_Config::Trace_Format_CSV         : Pos2+=__T(','); break;
        default                                         : ;
    }
    return Pos2;
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
void File__Analyze::Element_Begin()
{
    //Level
    Element_Level++;

    //Element
    Element[Element_Level].Code=0;
    Element[Element_Level].Next=Element[Element_Level-1].Next;
    Element[Element_Level].WaitForMoreData=Element[Element_Level-1].WaitForMoreData;
    Element[Element_Level].UnTrusted=Element[Element_Level-1].UnTrusted;
    Element[Element_Level].IsComplete=Element[Element_Level-1].IsComplete;

    //ToShow
    #if MEDIAINFO_TRACE
    Element[Element_Level].ToShow.Pos=File_Offset+Buffer_Offset+Element_Offset+BS->OffsetBeforeLastCall_Get(); //TODO: change this, used in Element_End0()
    if (Trace_Activated)
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
void File__Analyze::Element_Begin(const Ztring &Name)
{
    //Level
    Element_Level++;

    //Element
    Element[Element_Level].Code=0;
    Element[Element_Level].Next=Element[Element_Level-1].Next;
    Element[Element_Level].WaitForMoreData=false;
    Element[Element_Level].UnTrusted=Element[Element_Level-1].UnTrusted;
    Element[Element_Level].IsComplete=Element[Element_Level-1].IsComplete;

    //ToShow
    Element[Element_Level].ToShow.Pos=File_Offset+Buffer_Offset+Element_Offset+BS->OffsetBeforeLastCall_Get(); //TODO: change this, used in Element_End0()
    if (Trace_Activated)
    {
        Element[Element_Level].ToShow.Size=Element[Element_Level].Next-(File_Offset+Buffer_Offset+Element_Offset+BS->OffsetBeforeLastCall_Get());
        Element[Element_Level].ToShow.Header_Size=0;
        Element_Name(Name);
        Element[Element_Level].ToShow.Info.clear();
        Element[Element_Level].ToShow.Details.clear();
        Element[Element_Level].ToShow.NoShow=false;
    }
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Element_Name(const Ztring &Name)
{
    //ToShow
    if (Trace_Activated)
    {
        if (!Name.empty())
        {
            Ztring Name2=Name;
            Name2.FindAndReplace(__T("\r\n"), __T("__"), 0, Ztring_Recursive);
            Name2.FindAndReplace(__T("\r"), __T("_"), 0, Ztring_Recursive);
            Name2.FindAndReplace(__T("\n"), __T("_"), 0, Ztring_Recursive);
            if (Name2[0]==__T(' '))
                Name2[0]=__T('_');
            Element[Element_Level].ToShow.Name=Name2;
        }
        else
            Element[Element_Level].ToShow.Name=__T("(Empty)");
    }
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Element_Info(const Ztring &Parameter)
{
    //Coherancy
    if (Config_Trace_Level==0 || !(Trace_Layers.to_ulong()&Config_Trace_Layers.to_ulong()) || Element[Element_Level].ToShow.Details.size()>64*1024*1024)
        return;

    //Needed?
    if (Config_Trace_Level<=0.7)
        return;

    //ToShow
    Ztring Parameter2(Parameter);
    Parameter2.FindAndReplace(__T("\r\n"), __T(" / "));
    Parameter2.FindAndReplace(__T("\r"), __T(" / "));
    Parameter2.FindAndReplace(__T("\n"), __T(" / "));
    switch (Config_Trace_Format)
    {
        case MediaInfo_Config::Trace_Format_Tree        :
        case MediaInfo_Config::Trace_Format_CSV         : Element[Element_Level].ToShow.Info+=__T(" - "); break;
        default                                         : ;
    }
    Element[Element_Level].ToShow.Info+=Parameter2;
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Element_End(const Ztring &Name)
{
    //ToShow
    if (Trace_Activated)
    {
        Element[Element_Level].ToShow.Size=Element[Element_Level].Next-Element[Element_Level].ToShow.Pos;
        if (!Name.empty())
            Element[Element_Level].ToShow.Name=Name;
    }

    Element_End_Common_Flush();
}
#endif //MEDIAINFO_TRACE

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
    if (Trace_Activated)
    {
        if (!Element[Element_Level+1].WaitForMoreData && (Element[Element_Level+1].IsComplete || !Element[Element_Level+1].UnTrusted) && !Element[Element_Level+1].ToShow.NoShow)// && Config_Trace_Level!=0 && Element[Element_Level].ToShow.Details.size()<=64*1024*1024)
        {
            //Element
            if (!Element[Element_Level+1].ToShow.Name.empty())
            {
                if (!Element[Element_Level].ToShow.Details.empty())
                    Element[Element_Level].ToShow.Details+=Config_LineSeparator;
                Element[Element_Level].ToShow.Details+=Element_End_Common_Flush_Build();
                Element[Element_Level+1].ToShow.Name.clear();
            }

            //Info
            if (!Element[Element_Level+1].ToShow.Details.empty())
            {
                if (!Element[Element_Level].ToShow.Details.empty())
                    Element[Element_Level].ToShow.Details+=Config_LineSeparator;
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
    if (Config_Trace_Level>0.7)
    {
        ToReturn+=Log_Offset(Element[Element_Level+1].ToShow.Pos, Config_Trace_Format);
    }

    //Name
    switch (Config_Trace_Format)
    {
        case MediaInfo_Config::Trace_Format_Tree        : ToReturn.resize(ToReturn.size()+Element_Level_Base+Element_Level, __T(' ')); break;
        case MediaInfo_Config::Trace_Format_CSV         :
                    ToReturn+=__T("G,");
                    ToReturn+=Ztring::ToZtring(Element_Level_Base+Element_Level);
                    ToReturn+=__T(',');
                    break;
        default                                         : ;
    }
    ToReturn+=Element[Element_Level+1].ToShow.Name;

    //Info
    ToReturn+=Element[Element_Level+1].ToShow.Info;
    Element[Element_Level+1].ToShow.Info.clear();

    //Size
    if (Config_Trace_Level>0.3)
    {
        switch (Config_Trace_Format)
        {
            case MediaInfo_Config::Trace_Format_Tree        :
                    ToReturn+=__T(" (");
                    break;
            case MediaInfo_Config::Trace_Format_CSV         :
                    ToReturn+=__T(",(");
                    break;
            default                                         : ;
        }
        ToReturn+=Ztring::ToZtring(Element[Element_Level+1].ToShow.Size);
        if (Element[Element_Level+1].ToShow.Header_Size>0)
        {
            ToReturn+=__T("/");
            ToReturn+=Ztring::ToZtring(Element[Element_Level+1].ToShow.Size-Element[Element_Level+1].ToShow.Header_Size);
        }
        ToReturn+=__T(" bytes)");
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
    if (Config_Trace_Level==0 || !(Trace_Layers.to_ulong()&Config_Trace_Layers.to_ulong()))
        return;

    //Position
    int64u Pos=Element_Offset+BS->OffsetBeforeLastCall_Get();

    //Coherancy
    if (Element[Element_Level].UnTrusted)
        return;

    //Line separator
    if (!Element[Element_Level].ToShow.Details.empty())
        Element[Element_Level].ToShow.Details+=Config_LineSeparator;

    //Show Offset
    if (Config_Trace_Level>0.7)
    {
        Element[Element_Level].ToShow.Details+=Log_Offset(Pos==(int64u)-1?Pos:(File_Offset+Buffer_Offset+Pos), Config_Trace_Format);
    }

    //Show Parameter+Value
    switch (Config_Trace_Format)
    {
        case MediaInfo_Config::Trace_Format_Tree        :
                    {
                    const size_t Padding_Value=40;

                    //Show Parameter
                    Ztring Param; Param=Parameter;
                    if (Param.size()>Padding_Value) Param.resize(Padding_Value);
                    Element[Element_Level].ToShow.Details.resize(Element[Element_Level].ToShow.Details.size()+Element_Level_Base+Element_Level, __T(' '));
                    Element[Element_Level].ToShow.Details+=Param;

                    //Show Value
                    if (!Value.empty())
                    {
                        Element[Element_Level].ToShow.Details+=__T(": ");
                        Element[Element_Level].ToShow.Details.resize(Element[Element_Level].ToShow.Details.size()+Padding_Value-Param.size()-Element_Level+1, __T(' '));
                        Ztring Value2(Value);
                        Value2.FindAndReplace(__T("\r\n"), __T(" / "), 0, Ztring_Recursive);
                        Value2.FindAndReplace(__T("\r"), __T(" / "), 0, Ztring_Recursive);
                        Value2.FindAndReplace(__T("\n"), __T(" / "), 0, Ztring_Recursive);
                        Element[Element_Level].ToShow.Details+=Value2;
                    }
                    }
                    break;
        case MediaInfo_Config::Trace_Format_CSV         :
                    Element[Element_Level].ToShow.Details+=__T("T,");
                    Element[Element_Level].ToShow.Details+=Ztring::ToZtring(Element_Level_Base+Element_Level);
                    Element[Element_Level].ToShow.Details+=__T(',');
                    Element[Element_Level].ToShow.Details+=Parameter;
                    Element[Element_Level].ToShow.Details+=__T(',');
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
    if (Config_Trace_Format==MediaInfo_Config::Trace_Format_CSV)
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

    if (Config_Trace_Level==0 || !(Trace_Layers.to_ulong()&Config_Trace_Layers.to_ulong()))
        return;

    //Coherancy
    if (Element[Element_Level_Final].UnTrusted)
        return;

    //Line separator
    if (!Element[Element_Level_Final].ToShow.Details.empty())
        Element[Element_Level_Final].ToShow.Details+=Config_LineSeparator;

    //Preparing
    Ztring ToShow; ToShow.resize(Element_Level_Final, __T(' '));
    ToShow+=__T("---   ");
    ToShow+=Value;
    ToShow+=__T("   ---");
    Ztring Separator; Separator.resize(Element_Level_Final, __T(' '));
    Separator.resize(ToShow.size(), __T('-'));

    //Show Offset
    Ztring Offset;
    if (Config_Trace_Level>0.7)
        Offset=Log_Offset(File_Offset+Buffer_Offset+Element_Offset+BS->Offset_Get(), Config_Trace_Format);
    Offset.resize(Offset.size()+Element_Level_Base, __T(' '));

    //Show Value
    Element[Element_Level_Final].ToShow.Details+=Offset;
    Element[Element_Level_Final].ToShow.Details+=Separator;
    Element[Element_Level_Final].ToShow.Details+=Config_LineSeparator;
    Element[Element_Level_Final].ToShow.Details+=Offset;
    Element[Element_Level_Final].ToShow.Details+=ToShow;
    Element[Element_Level_Final].ToShow.Details+=Config_LineSeparator;
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
    if (Config_Trace_Level==0 || !(Trace_Layers.to_ulong()&Config_Trace_Layers.to_ulong()) || Element[Element_Level].ToShow.Details.size()>64*1024*1024)
        return;

    //Needed?
    if (Config_Trace_Level<=0.7)
        return;

    //Filling
    Element[Element_Level].ToShow.Details+=__T(" - ")+Text;
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
    Element_Offset=Element_Size;
    BS->Attach(NULL, 0);

    if (!Element[Element_Level].UnTrusted)
    {
        #if MEDIAINFO_TRACE
            Param(Reason, 0);
        #endif //MEDIAINFO_TRACE

        //Enough data?
        if (!Element[Element_Level].IsComplete)
        {
            Element_WaitForMoreData();
            return;
        }

        Element[Element_Level].UnTrusted=true;
        Synched=false;
        if (!Status[IsFilled] && Trusted>0)
            Trusted--;
    }

    if (Trusted==0 && !Status[IsAccepted])
        Reject();
}

//***************************************************************************
// Actions
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Accept (const char* ParserName_Char)
#else //MEDIAINFO_TRACE
void File__Analyze::Accept ()
#endif //MEDIAINFO_TRACE
{
    if (Status[IsAccepted] || Status[IsFinished])
        return;

    #if MEDIAINFO_TRACE
        if (ParserName.empty())
            ParserName.From_Local(ParserName_Char);

        if (!ParserName.empty())
        {
            bool MustElementBegin=Element_Level?true:false;
            if (Element_Level>0)
                Element_End0(); //Element
            Info(ParserName+__T(", accepted"));
            if (MustElementBegin)
                Element_Level++;
        }
    #endif //MEDIAINFO_TRACE

    Status[IsAccepted]=true;
    if (Count_Get(Stream_General)==0)
    {
        Stream_Prepare(Stream_General);
        Streams_Accept();
    }

    #if MEDIAINFO_EVENTS
        if (!IsSub)
        {
            EVENT_BEGIN (General, Parser_Selected, 0)
                std::memset(Event.Name, 0, 16);
                if (!ParserName.empty())
                    strncpy(Event.Name, ParserName.To_Local().c_str(), 15);
            EVENT_END   ()

            #if MEDIAINFO_DEMUX && MEDIAINFO_NEXTPACKET
                if (!Demux_EventWasSent_Accept_Specific && Config->NextPacket_Get() && Config->Event_CallBackFunction_IsSet())
                    Config->Demux_EventWasSent=true;
            #endif //MEDIAINFO_DEMUX && MEDIAINFO_NEXTPACKET
        }

        Config->Event_Accepted(this);
    #endif //MEDIAINFO_EVENTS
}

void File__Analyze::Accept (File__Analyze* Parser)
{
    if (Parser==NULL)
        return;

    Parser->Accept();
}

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Update (const char* ParserName_Char)
#else //MEDIAINFO_TRACE
void File__Analyze::Update ()
#endif //MEDIAINFO_TRACE
{
    if (!Status[IsAccepted])
        return;

    Open_Buffer_Update();
}

void File__Analyze::Update (File__Analyze* Parser)
{
    if (Parser==NULL)
        return;

    Parser->Update();
}

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Fill (const char* ParserName_Char)
#else //MEDIAINFO_TRACE
void File__Analyze::Fill ()
#endif //MEDIAINFO_TRACE
{
    if (!Status[IsAccepted] || Status[IsFilled] || Status[IsFinished])
        return;

    #if MEDIAINFO_TRACE
        if (ParserName.empty())
            ParserName.From_Local(ParserName_Char);

        if (!ParserName.empty())
        {
            bool MustElementBegin=Element_Level?true:false;
            if (Element_Level>0)
                Element_End0(); //Element
            Info(ParserName+__T(", filling"));
            if (MustElementBegin)
                Element_Level++;
        }
    #endif //MEDIAINFO_TRACE

    Streams_Fill();
    Status[IsFilled]=true;
    Status[IsUpdated]=true;

    //Instantaneous bitrate at the "fill" level
    if (File_Size==(int64u)-1 && FrameInfo.PTS!=(int64u)-1 && PTS_Begin!=(int64u)-1 && FrameInfo.PTS-PTS_Begin && StreamKind_Last!=Stream_General && StreamKind_Last!=Stream_Max)
    {
        Fill(StreamKind_Last, 0, "BitRate_Instantaneous", Buffer_TotalBytes*8*1000000000/(FrameInfo.PTS-PTS_Begin));
        (*Stream_More)[StreamKind_Last][0](Ztring().From_Local("BitRate_Instantaneous"), Info_Options)=__T("N NI");
    }
}

void File__Analyze::Fill (File__Analyze* Parser)
{
    if (Parser==NULL)
        return;

    Parser->Fill();
}

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Finish (const char* ParserName_Char)
#else //MEDIAINFO_TRACE
void File__Analyze::Finish ()
#endif //MEDIAINFO_TRACE
{
    if (Status[IsFinished])
        return;

    if (!ShouldContinueParsing && !Status[IsFilled])
        Fill();

    if (ShouldContinueParsing || Config->ParseSpeed==1)
    {
        #if MEDIAINFO_TRACE
            if (!ParserName.empty())
            {
                bool MustElementBegin=Element_Level?true:false;
                if (Element_Level>0)
                    Element_End0(); //Element
                //Info(Ztring(ParserName)+__T(", wants to finish, but should continue parsing"));
                if (MustElementBegin)
                    Element_Level++;
            }
        #endif //MEDIAINFO_TRACE

        return;
    }

    ForceFinish();
}

void File__Analyze::Finish (File__Analyze* Parser)
{
    ForceFinish(Parser); //The base parser wants, and is prepared to it, so nothing can be cancelled --> ForceFinish() instead of Finish()
}

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::ForceFinish (const char* ParserName_Char)
#else //MEDIAINFO_TRACE
void File__Analyze::ForceFinish ()
#endif //MEDIAINFO_TRACE
{
    if (Status[IsFinished])
        return;

    #if MEDIAINFO_TRACE
        if (ParserName.empty())
            ParserName.From_Local(ParserName_Char);

        if (!ParserName.empty())
        {
            bool MustElementBegin=Element_Level?true:false;
            if (Element_Level>0)
                Element_End0(); //Element
            Info(ParserName+__T(", finished"));
            if (MustElementBegin)
                Element_Level++;
        }
    #endif //MEDIAINFO_TRACE

    if (Status[IsAccepted])
    {
        //Total file size
        #if MEDIAINFO_ADVANCED
            if (!IsSub && !(!Config->File_IgnoreSequenceFileSize_Get() || Config->File_Names.size()<=1) && Config->ParseSpeed>=1.0 && Config->File_Names.size()>1 && Config->File_Names_Pos+1>=Config->File_Names.size())
            {
                Fill (Stream_General, 0, General_FileSize, Config->File_Current_Size, 10, true);
            }
        #endif //MEDIAINFO_ADVANCED

        Fill();
        #if MEDIAINFO_DEMUX
            if (Config->Demux_EventWasSent)
                return;
        #endif //MEDIAINFO_DEMUX
        Streams_Finish();
        #if MEDIAINFO_DEMUX
            if (Config->Demux_EventWasSent)
                return;
        #endif //MEDIAINFO_DEMUX
        if (Status[IsUpdated])
        {
            Open_Buffer_Update();
            if (IsSub)
                Status[IsUpdated]=true; //We want that container merges the result
            #if MEDIAINFO_DEMUX
                if (Config->Demux_EventWasSent)
                    return;
            #endif //MEDIAINFO_DEMUX
        }
        Streams_Finish_Global();
        #if MEDIAINFO_DEMUX
            if (Config->Demux_EventWasSent)
                return;
        #endif //MEDIAINFO_DEMUX
        Ibi_Stream_Finish();
    }

    Status[IsFinished]=true;

    //Real stream size
    if (Config->ParseSpeed==1 && IsRawStream && Buffer_TotalBytes)
    {
        //Exception with text streams embedded in video
        if (StreamKind_Last==Stream_Text)
            StreamKind_Last=Stream_Video;

        Fill(StreamKind_Last, 0, "StreamSize", Buffer_TotalBytes, 10, true);
    }

    //Frame count
    if (Config->ParseSpeed==1 && IsRawStream && Frame_Count && Frame_Count!=(int64u)-1 && Retrieve(StreamKind_Last, 0, Fill_Parameter(StreamKind_Last, Generic_FrameCount)).empty())
        Fill(StreamKind_Last, 0, Fill_Parameter(StreamKind_Last, Generic_FrameCount), Frame_Count);
}

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
#else //MEDIAINFO_TRACE
void File__Analyze::Reject ()
#endif //MEDIAINFO_TRACE
{
    Status[IsAccepted]=false;
    Status[IsFinished]=true;
    Clear();

    #if MEDIAINFO_TRACE
        if (ParserName)// && File_Offset+Buffer_Offset+Element_Size<File_Size)
        {
            bool MustElementBegin=Element_Level?true:false;
            if (Element_Level>0)
                Element_End0(); //Element
            Info(Ztring(ParserName)+__T(", rejected"));
            if (MustElementBegin)
                Element_Level++;
        }
    #endif //MEDIAINFO_TRACE
}

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

    if (IsSub && Config->ParseSpeed==1)
        return;

    if (GoTo==File_Size)
    {
        if (!BookMark_Code.empty())
            BookMark_Get();
        if (File_GoTo==(int64u)-1)
            ForceFinish();
        return;
    }

    if (ShouldContinueParsing)
    {
        if (ParserName)
        {
            bool MustElementBegin=Element_Level?true:false;
            if (Element_Level>0)
                Element_End0(); //Element
            Info(Ztring(ParserName)+__T(", wants to go to somewhere, but should continue parsing"));
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
                Element_End0(); //Element
            Info(Ztring(ParserName)+__T(", wants to go to somewhere, but is sub, waiting data"));
            if (MustElementBegin)
                Element_Level++;
        }
        return;
    }

    if (ParserName)
    {
        bool MustElementBegin=Element_Level?true:false;
        if (Element_Level>0)
            Element_End0(); //Element
        Info(Ztring(ParserName)+__T(", jumping to offset ")+Ztring::ToZtring(GoTo, 16));
        if (MustElementBegin)
            Element_Level++; //Element
    }

    File_GoTo=GoTo;

    #if MEDIAINFO_EVENTS
        EVENT_BEGIN (General, Move_Request, 0)
            Event.StreamOffset=File_GoTo;
        EVENT_END   ()
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

    if (IsSub && Config->ParseSpeed==1)
        return;

    if (GoTo==File_Size)
    {
        if (!BookMark_Code.empty())
            BookMark_Get();
        if (File_GoTo==(int64u)-1)
            ForceFinish();
        return;
    }

    if (ShouldContinueParsing)
    {
        return;
    }

    if (IsSub)
    {
        return;
    }

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
                Element_End0(); //Element
            Info(Ztring(ParserName)+__T(", wants to go to somewhere, but not valid"));
            if (MustElementBegin)
                Element_Level++;
        }
        return;
    }

    if (File_Size==(int64u)-1)
    {
        #if MEDIAINFO_SEEK
            if (Config->File_IgnoreSequenceFileSize_Get() && GoToFromEnd)
            {
                File_GoTo=Config->File_Names.size()-1;
                File_Offset=(int64u)-1;
                Config->File_Current_Offset=(int64u)-1;
                Config->File_GoTo_IsFrameOffset=true;
            }
            else
        #endif //MEDIAINFO_SEEK
                ForceFinish(); //We can not jump
        return;
    }

    GoTo(File_Size-GoToFromEnd, ParserName);
}
#else //MEDIAINFO_TRACE
void File__Analyze::GoToFromEnd (int64u GoToFromEnd)
{
    if (GoToFromEnd>File_Size)
        return;

    if (File_Size==(int64u)-1)
    {
        #if MEDIAINFO_SEEK
            if (Config->File_IgnoreSequenceFileSize_Get() && GoToFromEnd)
            {
                File_GoTo=Config->File_Names.size()-1;
                File_Offset=(int64u)-1;
                Config->File_Current_Offset=(int64u)-1;
                Config->File_GoTo_IsFrameOffset=true;
            }
            else
        #endif //MEDIAINFO_SEEK
                ForceFinish(); //We can not jump
        return;
    }

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
        Element[Element_Level].ToShow.Details+=Config_LineSeparator;

    //From Sub
    Element[Element_Level].ToShow.Details+=ToShow;
}
#endif //MEDIAINFO_TRACE

#if MEDIAINFO_TRACE
void File__Analyze::Trace_Layers_Update(size_t Layer)
{
    if (Layer!=(size_t)-1)
    {
        Trace_Layers.reset();
        Trace_Layers.set(Layer);
    }
    Trace_Activated=(Config_Trace_Level!=0 && (Trace_Layers&Config_Trace_Layers)!=0);
}
#endif //MEDIAINFO_TRACE

//***************************************************************************
// Status
//***************************************************************************

//---------------------------------------------------------------------------
bool File__Analyze::Element_IsOK ()
{
    #if !MEDIAINFO_TRACE
        if (BS && BS->BufferUnderRun)
            Trusted_IsNot();
    #endif //MEDIAINFO_TRACE

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
        Element_End0();
    while (Element_Level<BookMark_Element_Level)
    {
        Element_Begin1("Restarting parsing...");
        Element_WantNextLevel=true;
    }

    for (size_t Pos=0; Pos<=Element_Level; Pos++)
    {
        Element[Pos].Code=BookMark_Code[Pos];
        Element[Pos].Next=BookMark_Next[Pos];
    }
    BookMark_Code.clear();
    BookMark_Next.clear();
    if (File_GoTo==(int64u)-1)
    {
        #if MEDIAINFO_MD5
            delete MD5; MD5=NULL;
        #endif //MEDIAINFO_MD5
        File_GoTo=BookMark_GoTo;
    }
}

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File__Analyze::Details_Clear()
{
    Details->clear();
    Element[0].ToShow.Details.clear();
}
#endif //MEDIAINFO_TRACE

#if MEDIAINFO_EVENTS
void File__Analyze::Event_Prepare(struct MediaInfo_Event_Generic* Event)
{
    memset(Event, 0xFF, sizeof(struct MediaInfo_Event_Generic));
    Event->StreamIDs_Size=StreamIDs_Size;
    memcpy_Unaligned_Unaligned_Once1024(Event->StreamIDs, StreamIDs, 128);
    memcpy(Event->StreamIDs_Width, StreamIDs_Width, sizeof(StreamIDs_Width));
    memcpy(Event->ParserIDs, ParserIDs, sizeof(ParserIDs));
    Event->StreamOffset=File_Offset+Buffer_Offset+Element_Offset;
    Event->FrameNumber=Frame_Count_NotParsedIncluded;
    Event->PCR=FrameInfo.PCR;
    Event->DTS=(FrameInfo.DTS==(int64u)-1?FrameInfo.PTS:FrameInfo.DTS);
    Event->PTS=FrameInfo.PTS;
    Event->DUR=FrameInfo.DUR;
    //Event->FrameNumber_PresentationOrder=FrameNumber_PresentationOrder;
}
#endif //MEDIAINFO_EVENTS

//***************************************************************************
// Demux
//***************************************************************************
#if MEDIAINFO_DEMUX
void File__Analyze::Demux (const int8u* Buffer, size_t Buffer_Size, contenttype Content_Type, const int8u* xx, size_t xxx)
{
    if (!(Config_Demux&Demux_Level))
        return;

    if (!Buffer_Size)
        return;

    #if MEDIAINFO_DEMUX && MEDIAINFO_SEEK
        if (Config->Demux_IsSeeking)
            return;
    #endif //MEDIAINFO_SEEK

    #if MEDIAINFO_EVENTS
        //Demux
        if (StreamIDs_Size)
            StreamIDs[StreamIDs_Size-1]=Element_Code;

        EVENT_BEGIN(Global, Demux, 4)
            if (StreamIDs_Size)
                Event.EventCode|=((int32u)ParserIDs[StreamIDs_Size-1]<<24);
            Event.Content_Type=(int8u)Content_Type;
            Event.Content_Size=Buffer_Size;
            Event.Content=Buffer;
            Event.Flags=0;
            if (Demux_random_access)
                Event.Flags|=0x1; //Bit 0
            Event.Offsets_Size=Offsets_Buffer.size();
            std::vector<int64u> Offsets_Stream_Temp;
            std::vector<int64u> Offsets_Buffer_Temp;
            float64 Ratio=1;
            if (OriginalBuffer_Size)
                Ratio=((float64)File__Analyze::OriginalBuffer_Size)/File__Analyze::Buffer_Size;
            if (Offsets_Buffer.empty())
            {
                Event.Offsets_Stream=NULL;
                Event.Offsets_Content=NULL;
            }
            else if (Buffer_Offset+Element_Offset)
            {
                Offsets_Stream_Temp=Offsets_Stream;
                Offsets_Buffer_Temp=Offsets_Buffer;
                size_t Pos=0;
                if (Offsets_Buffer.size()>=2 && Offsets_Buffer.size()%2==0 && Offsets_Buffer[0]==Offsets_Buffer[1])
                {
                    while (Pos+2<Offsets_Buffer_Temp.size() && Offsets_Buffer_Temp[Pos+2]<Buffer_Offset+Element_Offset)
                        Pos+=2;
                    if (Pos)
                    {
                        Offsets_Buffer_Temp.erase(Offsets_Buffer_Temp.begin(), Offsets_Buffer_Temp.begin()+Pos);
                        Offsets_Stream_Temp.erase(Offsets_Stream_Temp.begin(), Offsets_Stream_Temp.begin()+Pos);
                        Event.Offsets_Size-=Pos;
                    }
                    Offsets_Stream_Temp[0]+=(Buffer_Offset+Element_Offset)/2-Offsets_Buffer_Temp[0];
                    Offsets_Stream_Temp[1]+=(Buffer_Offset+Element_Offset)/2-Offsets_Buffer_Temp[1];
                    Offsets_Buffer_Temp[0]=0;
                    Offsets_Buffer_Temp[1]=0;
                    for (size_t Pos=2; Pos<Offsets_Buffer_Temp.size(); Pos+=2)
                    {
                        Offsets_Buffer_Temp[Pos]-=(Buffer_Offset+Element_Offset)/2;
                        Offsets_Buffer_Temp[Pos+1]-=(Buffer_Offset+Element_Offset)/2;
                    }
                }
                else
                {
                    while (Pos+1<Offsets_Buffer_Temp.size() && Offsets_Buffer_Temp[Pos+1]<(Buffer_Offset+Element_Offset)*Ratio)
                        Pos++;
                    if (Pos)
                    {
                        Offsets_Buffer_Temp.erase(Offsets_Buffer_Temp.begin(), Offsets_Buffer_Temp.begin()+Pos);
                        Offsets_Stream_Temp.erase(Offsets_Stream_Temp.begin(), Offsets_Stream_Temp.begin()+Pos);
                        Event.Offsets_Size-=Pos;
                    }
                    Offsets_Stream_Temp[0]+=float64_int64s((Buffer_Offset+Element_Offset)*Ratio)-Offsets_Buffer_Temp[0];
                    Offsets_Buffer_Temp[0]=0;
                    for (size_t Pos=1; Pos<Offsets_Buffer_Temp.size(); Pos++)
                        Offsets_Buffer_Temp[Pos]-=float64_int64s((Buffer_Offset+Element_Offset)*Ratio);
                }
                Event.Offsets_Stream=&Offsets_Stream_Temp.front();
                Event.Offsets_Content=&Offsets_Buffer_Temp.front();
            }
            else
            {
                Event.Offsets_Stream=&Offsets_Stream.front();
                Event.Offsets_Content=&Offsets_Buffer.front();
            }
            Event.OriginalContent_Size=OriginalBuffer_Size?((size_t)float64_int64s(((float64)(Element_Size-Element_Offset))*Ratio)):0;
            Event.OriginalContent=OriginalBuffer_Size?(OriginalBuffer+(size_t)float64_int64s(((float64)(Buffer_Offset+Element_Offset))*Ratio)):NULL;
        EVENT_END()

        if (StreamIDs_Size)
            StreamIDs[StreamIDs_Size-1]=(int64u)-1;
        #if MEDIAINFO_DEMUX && MEDIAINFO_NEXTPACKET
            if (Status[IsAccepted] && Config->NextPacket_Get())
                Config->Demux_EventWasSent=true;
        #endif //MEDIAINFO_DEMUX && MEDIAINFO_NEXTPACKET
        if (StreamIDs_Size)
            StreamIDs[StreamIDs_Size-1]=(int64u)-1;
    #endif //MEDIAINFO_EVENTS
}
#endif //MEDIAINFO_DEMUX

#if MEDIAINFO_DEMUX
void File__Analyze::Demux_UnpacketizeContainer_Demux (bool random_access)
{
    Demux_random_access=random_access;

    if (StreamIDs_Size>=2)
        Element_Code=StreamIDs[StreamIDs_Size-2];
    StreamIDs_Size--;
    Demux(Buffer+Buffer_Offset, Demux_Offset-Buffer_Offset, ContentType_MainStream);
    StreamIDs_Size++;
    if (StreamIDs_Size>=2)
        StreamIDs[StreamIDs_Size-2]=Element_Code;
    Demux_UnpacketizeContainer_Demux_Clear();
}

bool File__Analyze::Demux_UnpacketizeContainer_Test_OneFramePerFile ()
{
    if (!IsSub && Buffer_Size<Config->File_Current_Size-Config->File_Current_Offset)
    {
        size_t* File_Buffer_Size_Hint_Pointer=Config->File_Buffer_Size_Hint_Pointer_Get();
        if (File_Buffer_Size_Hint_Pointer)
            (*File_Buffer_Size_Hint_Pointer) = (size_t)(Config->File_Current_Size - Config->File_Current_Offset - Buffer_Size);
        return false;
    }

    float64 Demux_Rate=Config->Demux_Rate_Get();
    if (!Demux_Rate)
        Demux_Rate=24;
    if (Frame_Count_NotParsedIncluded!=(int64u)-1)
        FrameInfo.DTS=float64_int64s(Frame_Count_NotParsedIncluded*1000000000/Demux_Rate);
    else
        FrameInfo.DTS=(int64u)-1;
    FrameInfo.PTS=FrameInfo.DTS;
    FrameInfo.DUR=float64_int64s(1000000000/Demux_Rate);
    Demux_Offset=Buffer_Size;
    Demux_UnpacketizeContainer_Demux();

    return true;
}

void File__Analyze::Demux_UnpacketizeContainer_Demux_Clear ()
{
    Demux_TotalBytes=Buffer_TotalBytes+Demux_Offset;
    Demux_Offset=0;
    //if (Frame_Count || Field_Count)
    //    Element_End0();
    //Element_Begin1("Frame or Field");
}
#endif //MEDIAINFO_DEMUX

//***************************************************************************
// IBI
//***************************************************************************
#if MEDIAINFO_IBI
void File__Analyze::Ibi_Read_Buffer_Unsynched ()
{
    Ibi_SynchronizationOffset_Current=(int64u)-1;

    if (IbiStream==NULL)
        return;

    IbiStream->Unsynch();
    for (size_t Pos=0; Pos<IbiStream->Infos.size(); Pos++)
    {
        if (File_GoTo==IbiStream->Infos[Pos].StreamOffset)
        {
            FrameInfo.DTS=(IbiStream->Infos[Pos].Dts!=(int64u)-1)?float64_int64s((((float64)IbiStream->Infos[Pos].Dts)*1000000000*IbiStream->DtsFrequencyDenominator/IbiStream->DtsFrequencyNumerator)):(int64u)-1;
            Frame_Count_NotParsedIncluded=IbiStream->Infos[Pos].FrameNumber;
            break;
        }
    }
}

size_t File__Analyze::Ibi_Read_Buffer_Seek (size_t Method, int64u Value, int64u ID)
{
    if (IbiStream==NULL)
        return (size_t)-1;

    //Init
    if (!Seek_Duration_Detected)
    {
        if (!IsSub)
        {
            //External IBI
            std::string IbiFile=Config->Ibi_Get();
            if (!IbiFile.empty())
            {
                IbiStream->Infos.clear(); //TODO: support IBI data from different inputs

                File_Ibi MI;
                Open_Buffer_Init(&MI, IbiFile.size());
                MI.Ibi=new ibi;
                MI.Open_Buffer_Continue((const int8u*)IbiFile.c_str(), IbiFile.size());
                (*IbiStream)=(*MI.Ibi->Streams.begin()->second);
            }
        }

        Seek_Duration_Detected=true;
    }

    //Parsing
    switch (Method)
    {
        case 0  :
                    #if MEDIAINFO_IBI
                    {
                    for (size_t Pos=0; Pos<IbiStream->Infos.size(); Pos++)
                    {
                        if (Value<=IbiStream->Infos[Pos].StreamOffset)
                        {
                            if (Value<IbiStream->Infos[Pos].StreamOffset && Pos)
                                Pos--;

                            //Checking continuity of Ibi
                            if (!IbiStream->Infos[Pos].IsContinuous && Pos+1<IbiStream->Infos.size())
                            {
                                Config->Demux_IsSeeking=true;
                                GoTo((IbiStream->Infos[Pos].StreamOffset+IbiStream->Infos[Pos+1].StreamOffset)/2);
                                Open_Buffer_Unsynch();

                                return 1;
                            }

                            Config->Demux_IsSeeking=false;

                            GoTo(IbiStream->Infos[Pos].StreamOffset);
                            Open_Buffer_Unsynch();

                            return 1;
                        }
                    }

                    if (IbiStream->Infos.empty())
                    {
                        GoTo(0);
                        Open_Buffer_Unsynch();
                    }
                    else if (!IbiStream->Infos[IbiStream->Infos.size()-1].IsContinuous)
                    {
                        GoTo(IbiStream->Infos[IbiStream->Infos.size()-1].StreamOffset);
                        Open_Buffer_Unsynch();
                    }
                    else
                        return 2; //Invalid value
                    return 1;
                    }
                    #else //MEDIAINFO_IBI
                    return (size_t)-2; //Not supported / IBI disabled
                    #endif //MEDIAINFO_IBI
        case 1  :
                    return Ibi_Read_Buffer_Seek(0, File_Size*Value/10000, ID);
        case 2  :   //Timestamp
                    #if MEDIAINFO_IBI
                    {
                    if (!(IbiStream->DtsFrequencyNumerator==1000000000 && IbiStream->DtsFrequencyDenominator==1))
                    {
                        float64 ValueF=(float64)Value;
                        ValueF/=1000000000; //Value is in ns
                        ValueF/=IbiStream->DtsFrequencyDenominator;
                        ValueF*=IbiStream->DtsFrequencyNumerator;
                        Value=float64_int64s(ValueF);
                    }

                    for (size_t Pos=0; Pos<IbiStream->Infos.size(); Pos++)
                    {
                        if (Value<=IbiStream->Infos[Pos].Dts)
                        {
                            if (Value<IbiStream->Infos[Pos].Dts && Pos)
                                Pos--;

                            //Checking continuity of Ibi
                            if (!IbiStream->Infos[Pos].IsContinuous && Pos+1<IbiStream->Infos.size())
                            {
                                Config->Demux_IsSeeking=true;
                                GoTo((IbiStream->Infos[Pos].StreamOffset+IbiStream->Infos[Pos+1].StreamOffset)/2);
                                Open_Buffer_Unsynch();

                                return 1;
                            }

                            Config->Demux_IsSeeking=false;

                            GoTo(IbiStream->Infos[Pos].StreamOffset);
                            Open_Buffer_Unsynch();

                            return 1;
                        }
                    }

                    if (IbiStream->Infos.empty())
                    {
                        GoTo(0);
                        Open_Buffer_Unsynch();
                    }
                    else if (!IbiStream->Infos[IbiStream->Infos.size()-1].IsContinuous)
                    {
                        GoTo(IbiStream->Infos[IbiStream->Infos.size()-1].StreamOffset);
                        Open_Buffer_Unsynch();
                    }
                    else
                        return 2; //Invalid value
                    return 1;
                    }
                    #else //MEDIAINFO_IBI
                    return (size_t)-2; //Not supported / IBI disabled
                    #endif //MEDIAINFO_IBI
        case 3  :   //FrameNumber
                    #if MEDIAINFO_IBI
                    {
                    for (size_t Pos=0; Pos<IbiStream->Infos.size(); Pos++)
                    {
                        if (Value<=IbiStream->Infos[Pos].FrameNumber)
                        {
                            if (Value<IbiStream->Infos[Pos].FrameNumber && Pos)
                                Pos--;

                            //Checking continuity of Ibi
                            if (!IbiStream->Infos[Pos].IsContinuous && Pos+1<IbiStream->Infos.size())
                            {
                                Config->Demux_IsSeeking=true;
                                GoTo((IbiStream->Infos[Pos].StreamOffset+IbiStream->Infos[Pos+1].StreamOffset)/2);
                                Open_Buffer_Unsynch();

                                return 1;
                            }

                            Config->Demux_IsSeeking=false;

                            GoTo(IbiStream->Infos[Pos].StreamOffset);
                            Open_Buffer_Unsynch();

                            return 1;
                        }
                    }

                    if (IbiStream->Infos.empty())
                    {
                        GoTo(0);
                        Open_Buffer_Unsynch();
                    }
                    else if (!IbiStream->Infos[IbiStream->Infos.size()-1].IsContinuous)
                    {
                        GoTo(IbiStream->Infos[IbiStream->Infos.size()-1].StreamOffset);
                        Open_Buffer_Unsynch();
                    }
                    else
                        return 2; //Invalid value
                    return 1;
                    }
                    #else //MEDIAINFO_IBI
                    return (size_t)-2; //Not supported / IBI disabled
                    #endif //MEDIAINFO_IBI
        default :   return (size_t)-1; //Not supported
    }
}

void File__Analyze::Ibi_Stream_Finish ()
{
    if (IsSub)
        return;

    if (!(IbiStream==NULL || IbiStream->Infos.empty()) && File_Offset+Buffer_Size==File_Size)
    {
        ibi::stream::info IbiInfo;
        IbiInfo.StreamOffset=File_Offset+Buffer_Size;
        IbiInfo.FrameNumber=Frame_Count_NotParsedIncluded;
        IbiInfo.Dts=(FrameInfo.DTS!=(int64u)-1)?float64_int64s(((float64)FrameInfo.DTS)/1000000000*IbiStream->DtsFrequencyNumerator/IbiStream->DtsFrequencyDenominator):(int64u)-1;
        IbiInfo.IsContinuous=true;
        IbiStream->Add(IbiInfo);
    }

    if (Config_Ibi_Create)
    {
        if (!(IbiStream==NULL || IbiStream->Infos.empty()))
            Ibi.Streams[(int64u)-1]=new ibi::stream(*IbiStream);

        //Inform_Data
        ZtringListList Content;
        for (size_t StreamKind=Stream_General; StreamKind<Stream_Max; ++StreamKind)
        {
            ZtringListList Source=MediaInfoLib::Config.Info_Get((stream_t)StreamKind);

            for (size_t StreamPos=0; StreamPos<Count_Get((stream_t)StreamKind); ++StreamPos)
            {
                ZtringList KindAndPos;
                KindAndPos.push_back(Get((stream_t)StreamKind, StreamPos, __T("StreamKind")));
                Content.push_back(KindAndPos);

                //Standard
                for (size_t Pos=0; Pos<Source.size(); ++Pos)
                {
                    Ztring &Options=Source[Pos](Info_Options);
                    if (InfoOption_ShowInSupported<Options.size() && Options[InfoOption_ShowInSupported]==__T('Y') && Pos<(*Stream)[StreamKind][StreamPos].size() && !(*Stream)[StreamKind][StreamPos][Pos].empty())
                    {
                        ZtringList Line;
                        Line.push_back(Source[Pos][Info_Name]);
                        Line.push_back((*Stream)[StreamKind][StreamPos][Pos]);
                        Content.push_back(Line);
                    }
                }

                //Additional
                for (size_t Pos=0; Pos<(*Stream_More)[StreamKind][StreamPos].size(); ++Pos)
                {
                    ZtringList Line=(*Stream_More)[StreamKind][StreamPos][Pos];
                    Line.resize(Info_Options+1);
                    Content.push_back(Line);
                }

                //Separator
                Content.push_back(Ztring());
            }
        }
        if (!Content.empty())
            Content.resize(Content.size()-1);
        Ibi.Inform_Data=Content.Read();

        //IBI Creation
        File_Ibi_Creation IbiCreation(Ibi);
        Ztring IbiText=IbiCreation.Finish();
        if (!IbiText.empty())
        {
            Fill(Stream_General, 0, "IBI", IbiText);
            (*Stream_More)[Stream_General][0]("IBI", Info_Options)=__T("N NT");
        }
    }
}

void File__Analyze::Ibi_Stream_Finish (int64u Numerator, int64u Denominator)
{
    if (IsSub || IbiStream==NULL)
        return;

    if (IbiStream->DtsFrequencyNumerator==1000000000 && IbiStream->DtsFrequencyDenominator==1 && !IbiStream->Infos.empty())
    {
        IbiStream->DtsFrequencyNumerator=Numerator;
        IbiStream->DtsFrequencyDenominator=Denominator;
        for (size_t Pos=0; Pos<IbiStream->Infos.size(); Pos++)
            if (IbiStream->Infos[Pos].Dts!=(int64u)-1)
                IbiStream->Infos[Pos].Dts=float64_int64s(((float64)IbiStream->Infos[Pos].Dts)/1000000000/Denominator*Numerator);
    }
}

void File__Analyze::Ibi_Add ()
{
    if (IbiStream==NULL)
        return;

    ibi::stream::info IbiInfo;
    IbiInfo.StreamOffset=IsSub?Ibi_SynchronizationOffset_Current:(File_Offset+Buffer_Offset);
    IbiInfo.FrameNumber=Frame_Count_NotParsedIncluded;
    IbiInfo.Dts=FrameInfo.DTS;
    IbiStream->Add(IbiInfo);

    if (Frame_Count_NotParsedIncluded==(int64u)-1)
        Frame_Count_NotParsedIncluded=IbiStream->Infos[IbiStream->Infos_Pos-1].FrameNumber;
}

#endif //MEDIAINFO_IBI

} //NameSpace
