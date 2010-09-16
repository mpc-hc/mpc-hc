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
#include "MediaInfo/MediaInfo_Internal.h"
#include "MediaInfo/MediaInfo_Config.h"
#include "ZenLib/File.h"
#include "ZenLib/FileName.h"
#include "ZenLib/BitStream_LE.h"
#include <cmath>
#ifdef SS
   #undef SS //Solaris defines this somewhere
#endif
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
extern MediaInfo_Config Config;
//---------------------------------------------------------------------------

//***************************************************************************
// Preparation des streams
//***************************************************************************

//---------------------------------------------------------------------------
size_t File__Analyze::Stream_Prepare (stream_t KindOfStream)
{
    //Integrity
    if (!Status[IsAccepted] || KindOfStream>Stream_Max)
        return Error;

    //Clear
    if (KindOfStream==Stream_Max)
    {
        StreamKind_Last=Stream_Max;
        StreamPos_Last=(size_t)-1;
        return 0;
    }

    //Add a stream
    (*Stream)[KindOfStream].resize((*Stream)[KindOfStream].size()+1);
    (*Stream_More)[KindOfStream].resize((*Stream_More)[KindOfStream].size()+1);

    StreamKind_Last=KindOfStream;
    StreamPos_Last=(*Stream)[KindOfStream].size()-1;

    //Filling basic info
    Fill(StreamKind_Last, StreamPos_Last, (size_t)General_Count, Count_Get(StreamKind_Last, StreamPos_Last));
    Fill(StreamKind_Last, StreamPos_Last, General_StreamKind, MediaInfoLib::Config.Info_Get(StreamKind_Last).Read(General_StreamKind, Info_Text));
    Fill(StreamKind_Last, StreamPos_Last, General_StreamKind_String, MediaInfoLib::Config.Language_Get(MediaInfoLib::Config.Info_Get(StreamKind_Last).Read(General_StreamKind, Info_Text)), true);
    Fill(StreamKind_Last, StreamPos_Last, General_StreamKindID, StreamPos_Last);
    for (size_t Pos=0; Pos<=StreamPos_Last; Pos++)
    {
        Fill(StreamKind_Last, Pos, General_StreamCount, Count_Get(StreamKind_Last), 10, true);
        if (Count_Get(StreamKind_Last)>1)
            Fill(StreamKind_Last, Pos, General_StreamKindPos, Pos+1, 10, true);
    }

    //Filling Lists & Counts
    if (!IsSub && KindOfStream!=Stream_General)
    {
        const Ztring& StreamKind_Text=Get(KindOfStream, 0, General_StreamKind, Info_Text);
        if (Count_Get(KindOfStream)>1)
        {
            Ztring Temp;
            Temp=Retrieve(Stream_General, 0, Ztring(StreamKind_Text+_T("_Codec_List")).To_Local().c_str())+_T(" / ");
            Fill(Stream_General, 0, Ztring(StreamKind_Text+_T("_Codec_List")).To_Local().c_str(), Temp, true);
            Temp=Retrieve(Stream_General, 0, Ztring(StreamKind_Text+_T("_Language_List")).To_Local().c_str())+_T(" / ");
            Fill(Stream_General, 0, Ztring(StreamKind_Text+_T("_Language_List")).To_Local().c_str(), Temp, true);
            Temp=Retrieve(Stream_General, 0, Ztring(StreamKind_Text+_T("_Format_List")).To_Local().c_str())+_T(" / ");
            Fill(Stream_General, 0, Ztring(StreamKind_Text+_T("_Format_List")).To_Local().c_str(), Temp, true);
            Temp=Retrieve(Stream_General, 0, Ztring(StreamKind_Text+_T("_Format_WithHint_List")).To_Local().c_str())+_T(" / ");
            Fill(Stream_General, 0, Ztring(StreamKind_Text+_T("_Format_WithHint_List")).To_Local().c_str(), Temp, true);
        }

        Fill(Stream_General, 0, Ztring(StreamKind_Text+_T("Count")).To_Local().c_str(), StreamPos_Last+1, 10, true);
    }

    //File name and dates
    if (!IsSub && KindOfStream==Stream_General && File_Name.size()>0)
    {
        //File name
        if (File_Name.find(_T("://"))==string::npos)
        {
            Fill (Stream_General, 0, General_CompleteName, File_Name);
            Fill (Stream_General, 0, General_FolderName, FileName::Path_Get(File_Name));
            Fill (Stream_General, 0, General_FileName, FileName::Name_Get(File_Name));
            Fill (Stream_General, 0, General_FileExtension, FileName::Extension_Get(File_Name).MakeLowerCase());
        }
        else
        {
            Ztring FileName_Modified=File_Name;
            size_t Begin=FileName_Modified.find(_T(':'), 6);
            size_t End=FileName_Modified.find(_T('@'));
            if (Begin!=string::npos && End!=string::npos && Begin<End)
                FileName_Modified.erase(Begin, End-Begin);
            Fill (Stream_General, 0, General_CompleteName, FileName_Modified);
        }

        //File dates
        File F(File_Name);
        Fill (Stream_General, 0, General_File_Created_Date, F.Created_Get());
        Fill (Stream_General, 0, General_File_Created_Date_Local, F.Created_Local_Get());
        Fill (Stream_General, 0, General_File_Modified_Date, F.Modified_Get());
        Fill (Stream_General, 0, General_File_Modified_Date_Local, F.Modified_Local_Get());
    }

    //File size
    if ((!IsSub || !File_Name.empty()) && KindOfStream==Stream_General && File_Size!=(int64u)-1)
        Fill (Stream_General, 0, General_FileSize, File_Size);

    //Fill with already ready data
    for (size_t Pos=0; Pos<Fill_Temp.size(); Pos++)
        if (Fill_Temp(Pos, 0).IsNumber())
            Fill(StreamKind_Last, StreamPos_Last, Fill_Temp(Pos, 0).To_int32u(), Fill_Temp(Pos, 1));
        else
            Fill(StreamKind_Last, StreamPos_Last, Fill_Temp(Pos, 0).To_UTF8().c_str(), Fill_Temp(Pos, 1));
    Fill_Temp.clear();

    return (*Stream)[KindOfStream].size()-1; //The position in the stream count
}

//***************************************************************************
// Filling
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Fill (stream_t StreamKind, size_t StreamPos, size_t Parameter, const Ztring &Value, bool Replace)
{
	//Integrity
    if (!Status[IsAccepted] || StreamKind>Stream_Max || Parameter==(size_t)-1)
        return;

    //Handling values with \r\n inside
    if (Value.find(_T('\r'))!=string::npos || Value.find(_T('\n'))!=string::npos)
    {
        Ztring NewValue=Value;
        NewValue.FindAndReplace(_T("\r\n"), _T(" / "), 0, Ztring_Recursive);
        NewValue.FindAndReplace(_T("\r"), _T(" / "), 0, Ztring_Recursive);
        NewValue.FindAndReplace(_T("\n"), _T(" / "), 0, Ztring_Recursive);
        if (NewValue.size()>=3 && NewValue.rfind(_T(" / "))==NewValue.size()-3)
            NewValue.resize(NewValue.size()-3);
        Fill(StreamKind, StreamPos, Parameter, NewValue, Replace);
        return;
    }

    //Handle Value before StreamKind
    if (StreamKind==Stream_Max || StreamPos>=(*Stream)[StreamKind].size())
    {
        ZtringList NewList;
        NewList.push_back(Ztring().From_Number(Parameter));
        NewList.push_back(Value);
        Fill_Temp.push_back(NewList);
        return; //No streams
    }

    //Some defaults
    if (Parameter==Fill_Parameter(StreamKind, Generic_Format_Commercial))
        Replace=true;
    if (Parameter==Fill_Parameter(StreamKind, Generic_Format_Commercial_IfAny))
        Replace=true;

    Ztring &Target=(*Stream)[StreamKind][StreamPos](Parameter);
    if (Target.empty() || Replace)
        Target=Value; //First value
    else
    {
        Target+=MediaInfoLib::Config.TagSeparator_Get();
        Target+=Value;
    }
    Status[IsUpdated]=true;

    //Deprecated
    if (Parameter==Fill_Parameter(StreamKind, Generic_Resolution))
        Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_BitDepth), Value, Replace);
    if (StreamKind==Stream_Video && Parameter==Video_Colorimetry)
        Fill(Stream_Video, StreamPos, Video_ChromaSubsampling, Value, Replace);

    if (StreamKind==Stream_Video && Parameter==Video_DisplayAspectRatio && !Value.empty() && Retrieve(Stream_Video, StreamPos, Video_PixelAspectRatio).empty())
    {
        float DAR   =Retrieve(Stream_Video, StreamPos, Video_DisplayAspectRatio).To_float32();
        float Width =Retrieve(Stream_Video, StreamPos, Video_Width             ).To_float32();
        float Height=Retrieve(Stream_Video, StreamPos, Video_Height            ).To_float32();
        if (DAR && Height && Width)
        {
            if (Retrieve(Stream_Video, StreamPos, Video_DisplayAspectRatio)==_T("1.778"))
                DAR=((float)16)/9; //More exact value
            if (Retrieve(Stream_Video, StreamPos, Video_DisplayAspectRatio)==_T("1.333"))
                DAR=((float)4)/3; //More exact value
            Fill(Stream_Video, StreamPos, Video_PixelAspectRatio, DAR/(((float32)Width)/Height));
        }
    }

    if (StreamKind==Stream_Video && Parameter==Video_PixelAspectRatio && !Value.empty() && Retrieve(Stream_Video, StreamPos, Video_DisplayAspectRatio).empty())
    {
        float32 PAR   =Retrieve(Stream_Video, StreamPos, Video_PixelAspectRatio  ).To_float32();
        if (PAR>(float32)12/(float32)11*0.99 && PAR<(float32)12/(float32)11*1.01)
            PAR=(float32)12/(float32)11;
        if (PAR>(float32)10/(float32)11*0.99 && PAR<(float32)10/(float32)11*1.01)
            PAR=(float32)10/(float32)11;
        if (PAR>(float32)16/(float32)11*0.99 && PAR<(float32)16/(float32)11*1.01)
            PAR=(float32)16/(float32)11;
        if (PAR>(float32)40/(float32)33*0.99 && PAR<(float32)40/(float32)33*1.01)
            PAR=(float32)40/(float32)33;
        if (PAR>(float32)24/(float32)11*0.99 && PAR<(float32)24/(float32)11*1.01)
            PAR=(float32)24/(float32)11;
        if (PAR>(float32)20/(float32)11*0.99 && PAR<(float32)20/(float32)11*1.01)
            PAR=(float32)20/(float32)11;
        if (PAR>(float32)32/(float32)11*0.99 && PAR<(float32)32/(float32)11*1.01)
            PAR=(float32)32/(float32)11;
        if (PAR>(float32)80/(float32)33*0.99 && PAR<(float32)80/(float32)33*1.01)
            PAR=(float32)80/(float32)33;
        if (PAR>(float32)18/(float32)11*0.99 && PAR<(float32)18/(float32)11*1.01)
            PAR=(float32)18/(float32)11;
        if (PAR>(float32)15/(float32)11*0.99 && PAR<(float32)15/(float32)11*1.01)
            PAR=(float32)15/(float32)11;
        if (PAR>(float32)64/(float32)33*0.99 && PAR<(float32)64/(float32)33*1.01)
            PAR=(float32)64/(float32)33;
        if (PAR>(float32)160/(float32)99*0.99 && PAR<(float32)160/(float32)99*1.01)
            PAR=(float32)160/(float32)99;
        if (PAR>(float32)4/(float32)3*0.99 && PAR<(float32)4/(float32)3*1.01)
            PAR=(float32)4/(float32)3;
        if (PAR>(float32)3/(float32)2*0.99 && PAR<(float32)3/(float32)2*1.01)
            PAR=(float32)3/(float32)2;
        if (PAR>(float32)2/(float32)1*0.99 && PAR<(float32)2/(float32)1*1.01)
            PAR=(float32)2;
        if (PAR>(float32)59/(float32)54*0.99 && PAR<(float32)59/(float32)54*1.01)
            PAR=(float32)59/(float32)54;
        float32 Width =Retrieve(Stream_Video, StreamPos, Video_Width             ).To_float32();
        float32 Height=Retrieve(Stream_Video, StreamPos, Video_Height            ).To_float32();
        if (PAR && Height && Width)
            Fill(Stream_Video, StreamPos, Video_DisplayAspectRatio, ((float32)Width)/Height*PAR);
    }

    //Commercial name
    if (Parameter==Fill_Parameter(StreamKind, Generic_Format))
        Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format_Commercial), Value);
    if (Parameter==Fill_Parameter(StreamKind, Generic_Format_Commercial_IfAny))
        Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format_Commercial), Value, true);

    if (!IsSub)
    {
        //Human readable
        if (MediaInfoLib::Config.ReadByHuman_Get())
        {
            //Strings
            const Ztring &List_Measure_Value=MediaInfoLib::Config.Info_Get(StreamKind).Read(Parameter, Info_Measure);
                 if (List_Measure_Value==_T(" byte"))
                FileSize_FileSize123(StreamKind, StreamPos, Parameter);
            else if (List_Measure_Value==_T(" bps") || List_Measure_Value==_T(" Hz"))
                Kilo_Kilo123(StreamKind, StreamPos, Parameter);
            else if (List_Measure_Value==_T(" ms"))
                Duration_Duration123(StreamKind, StreamPos, Parameter);
            else if (List_Measure_Value==_T("Yes"))
                YesNo_YesNo(StreamKind, StreamPos, Parameter);
            else
                Value_Value123(StreamKind, StreamPos, Parameter);

            //Lists
            if (StreamKind!=Stream_General)
            {
                const Ztring& Parameter_Text=MediaInfoLib::Config.Info_Get(StreamKind).Read(Parameter, Info_Name);
                if (Parameter_Text==_T("Codec/String")
                 || Parameter_Text==_T("Language/String")
                 || Parameter_Text==_T("Format")
                 || Parameter_Text==_T("CodecID/Hint"))
                {
                    Ztring Temp1, Temp2;
                    for (size_t StreamPos_Local=0; StreamPos_Local<(*Stream)[StreamKind].size(); StreamPos_Local++)
                    {
                        if (Parameter_Text==_T("CodecID/Hint"))
                            Temp1+=Retrieve(StreamKind, StreamPos_Local, Fill_Parameter(StreamKind, Generic_Format))+_T(" / ");
                        else
                            Temp1+=Retrieve(StreamKind, StreamPos_Local, Parameter)+_T(" / ");
                        if (Parameter_Text==_T("Format")
                         || Parameter_Text==_T("CodecID/Hint"))
                        {
                            Temp2+=Retrieve(StreamKind, StreamPos_Local, Fill_Parameter(StreamKind, Generic_Format));
                            if (!Retrieve(StreamKind, StreamPos_Local, Fill_Parameter(StreamKind, Generic_CodecID_Hint)).empty())
                            {
                                Temp2+=_T(" (");
                                Temp2+=Retrieve(StreamKind, StreamPos_Local, Fill_Parameter(StreamKind, Generic_CodecID_Hint));
                                Temp2+=_T(")");
                            }
                            Temp2+=_T(" / ");
                        }
                    }
                    if (!Temp1.empty())
                        Temp1.resize(Temp1.size()-3); //Delete extra " / "
                    if (!Temp2.empty())
                        Temp2.resize(Temp2.size()-3); //Delete extra " / "
                    Ztring StreamKind_Text=Get(StreamKind, 0, General_StreamKind, Info_Text);
                    if (Parameter_Text==_T("Codec/String"))
                        Fill(Stream_General, 0, Ztring(StreamKind_Text+_T("_Codec_List")).To_Local().c_str(), Temp1, true);
                    if (Parameter_Text==_T("Language/String"))
                        Fill(Stream_General, 0, Ztring(StreamKind_Text+_T("_Language_List")).To_Local().c_str(), Temp1, true);
                    if (Parameter_Text==_T("Format")
                     || Parameter_Text==_T("CodecID/Hint"))
                    {
                        Fill(Stream_General, 0, Ztring(StreamKind_Text+_T("_Format_List")).To_Local().c_str(), Temp1, true);
                        Fill(Stream_General, 0, Ztring(StreamKind_Text+_T("_Format_WithHint_List")).To_Local().c_str(), Temp2, true);
                    }
                }
            }

            //BitRate_Mode / OverallBitRate_Mode
            if (Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==(StreamKind==Stream_General?_T("OverallBitRate_Mode"):_T("BitRate_Mode")) && MediaInfoLib::Config.ReadByHuman_Get())
            {
                Clear(StreamKind, StreamPos, StreamKind==Stream_General?"OverallBitRate_Mode/String":"BitRate_Mode/String");

                ZtringList List;
                List.Separator_Set(0, _T(" / "));
                List.Write(Retrieve(StreamKind, StreamPos, Parameter));

                //Per value
                for (size_t Pos=0; Pos<List.size(); Pos++)
                    List[Pos]=MediaInfoLib::Config.Language_Get(Ztring(_T("BitRate_Mode_"))+List[Pos]);

                Ztring Translated=List.Read();
                Fill(StreamKind, StreamPos, StreamKind==Stream_General?"OverallBitRate_Mode/String":"BitRate_Mode/String", Translated.find(_T("BitRate_Mode_"))?Translated:Value);
            }

            //Encoded_Library
            if (Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("Encoded_Library")
             || Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("Encoded_Library/Name")
             || Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("Encoded_Library/Version")
             || Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("Encoded_Library/Date"))
            {
                Ztring Name=Retrieve(StreamKind, StreamPos, "Encoded_Library/Name");
                Ztring Version=Retrieve(StreamKind, StreamPos, "Encoded_Library/Version");
                Ztring Date=Retrieve(StreamKind, StreamPos, "Encoded_Library/Date");
                if (!Name.empty())
                {
                    Ztring String=Name;
                    if (!Version.empty())
                    {
                        String+=_T(" ");
                        String+=Version;
                    }
                    if (!Date.empty())
                    {
                        String+=_T(" (");
                        String+=Date;
                        String+=_T(")");
                    }
                    Fill(StreamKind, StreamPos, "Encoded_Library/String", String, true);
                }
                else
                    Fill(StreamKind, StreamPos, "Encoded_Library/String", Retrieve(StreamKind, StreamPos, "Encoded_Library"), true);
            }

            //Format_Settings_Matrix
            if (StreamKind==Stream_Video && Parameter==Video_Format_Settings_Matrix)
            {
                Ztring Translated=MediaInfoLib::Config.Language_Get(Ztring(_T("Format_Settings_Matrix_"))+Value);
                Fill(Stream_Video, StreamPos, Video_Format_Settings_Matrix_String, Translated.find(_T("Format_Settings_Matrix_"))?Translated:Value, true);
            }

            //Scan type
            if (StreamKind==Stream_Video && Parameter==Video_ScanType)
            {
                Ztring Translated=MediaInfoLib::Config.Language_Get(Ztring(_T("Interlaced_"))+Value);
                Fill(Stream_Video, StreamPos, Video_ScanType_String, Translated.find(_T("Interlaced_"))?Translated:Value, true);
            }

            //Scan order
            if (StreamKind==Stream_Video && Parameter==Video_ScanOrder)
            {
                Ztring Translated=MediaInfoLib::Config.Language_Get(Ztring(_T("Interlaced_"))+Value);
                Fill(Stream_Video, StreamPos, Video_ScanOrder_String, Translated.find(_T("Interlaced_"))?Translated:Value, true);
            }

            //Interlacement
            if (StreamKind==Stream_Video && Parameter==Video_Interlacement)
            {
                const Ztring &Z1=Retrieve(Stream_Video, StreamPos, Video_Interlacement);
                if (Z1.size()==3)
                    Fill(Stream_Video, StreamPos, Video_Interlacement_String, MediaInfoLib::Config.Language_Get(Ztring(_T("Interlaced_"))+Z1), true);
                else
                    Fill(Stream_Video, StreamPos, Video_Interlacement_String, MediaInfoLib::Config.Language_Get(Z1), true);
                if (Retrieve(Stream_Video, StreamPos, Video_Interlacement_String).empty())
                    Fill(Stream_Video, StreamPos, Video_Interlacement_String, Z1, true);
            }

            //FrameRate_Mode
            if (StreamKind==Stream_Video && Parameter==Video_FrameRate_Mode)
            {
                Ztring Translated=MediaInfoLib::Config.Language_Get(Ztring(_T("FrameRate_Mode_"))+Value);
                Fill(Stream_Video, StreamPos, Video_FrameRate_Mode_String, Translated.find(_T("FrameRate_Mode_"))?Translated:Value, true);
            }
        }

        //Filling Lists & Counts
        if (StreamKind!=Stream_General && (Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("Codec")
                                        || Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("Language/String")
                                        || Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("Format")
                                        || Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("CodecID/Hint")
            ))
        {
            const Ztring& StreamKind_Text=Get(StreamKind, 0, General_StreamKind, Info_Text);
            Ztring Temp1, Temp2;
            for (size_t Pos=0; Pos<Count_Get(StreamKind); Pos++)
            {
                if (Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("Format")
                 || Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("CodecID/Hint"))
                {
                    Temp1+=Retrieve(StreamKind, Pos, Fill_Parameter(StreamKind, Generic_Format))+_T(" / ");
                    Temp2+=Retrieve(StreamKind, Pos, Fill_Parameter(StreamKind, Generic_Format));
                    if (!Retrieve(StreamKind, Pos, Fill_Parameter(StreamKind, Generic_CodecID_Hint)).empty())
                    {
                        Temp2+=_T(" (");
                        Temp2+=Retrieve(StreamKind, Pos, Fill_Parameter(StreamKind, Generic_CodecID_Hint));
                        Temp2+=_T(")");
                    }
                    Temp2+=_T(" / ");
                }
                else
                    Temp1+=Retrieve(StreamKind, Pos, Parameter)+_T(" / ");
            }
            Temp1.resize(Temp1.size()-3); //Removing last " / "
            if (!Temp2.empty())
                Temp2.resize(Temp2.size()-3); //Removing last " / "

            if (Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("Codec"))
                Fill(Stream_General, 0, Ztring(StreamKind_Text+_T("_Codec_List")).To_Local().c_str(), Temp1, true);
            if (Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("Language/String"))
                Fill(Stream_General, 0, Ztring(StreamKind_Text+_T("_Language_List")).To_Local().c_str(), Temp1, true);
            if (Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("Format")
             || Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("CodecID/Hint"))
            {
                Fill(Stream_General, 0, Ztring(StreamKind_Text+_T("_Format_List")).To_Local().c_str(), Temp1, true);
                Fill(Stream_General, 0, Ztring(StreamKind_Text+_T("_Format_WithHint_List")).To_Local().c_str(), Temp2, true);
            }
        }

        //General Format
        if (Parameter==Fill_Parameter(StreamKind, Generic_Format) && Retrieve(Stream_General, 0, General_Format).empty() && !Value.empty())
            Fill(Stream_General, 0, General_Format, Value); //If not already filled, we are filling with the stream format

        //ID
        if (Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("ID"))
            Fill(StreamKind, StreamPos, General_ID_String, Value, Replace);

        //Format
        if (Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("Format"))
        {
            Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format_Info), MediaInfoLib::Config.Format_Get(Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format)), InfoFormat_Info), true);
            Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format_Url) , MediaInfoLib::Config.Format_Get(Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format)), InfoFormat_Url ), true);
            if (StreamKind!=Stream_Menu)
                Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_InternetMediaType), MediaInfoLib::Config.Format_Get(Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format)), InfoFormat_InternetMediaType), true);
            if (StreamKind==Stream_General)
            {
                Fill(Stream_General, 0, General_Format_Extensions, MediaInfoLib::Config.Format_Get(Value, InfoFormat_Extensions), true);
                Fill(Stream_General, 0, General_Format_String, Value, true);
                Fill(Stream_General, 0, General_Codec, Value, true);
                Fill(Stream_General, 0, General_Codec_String, Value, true);
            }
        }
        if (StreamKind==Stream_General && Parameter==General_Format_Info)
            (*Stream)[Stream_General][0](General_Codec_Info)=Value;
        if (StreamKind==Stream_General && Parameter==General_Format_Url)
            (*Stream)[Stream_General][0](General_Codec_Url)=Value;
        if (StreamKind==Stream_General && Parameter==General_Format_Extensions)
            (*Stream)[Stream_General][0](General_Codec_Extensions)=Value;
        if (StreamKind==Stream_General && Parameter==General_Format_Settings)
            (*Stream)[Stream_General][0](General_Codec_Settings)=Value;

        //Codec
        if (Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("Codec"))
        {
            const Ztring &C1=MediaInfoLib::Config.Codec_Get(Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Codec)), InfoCodec_Name, (stream_t)StreamKind);
            if (C1.empty())
                Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Codec_String), Value, StreamKind!=Stream_Menu); //Specific for Menu: multiple codecs
            else
            {
                Ztring D=Retrieve(StreamKind, StreamPos, "Codec/Family");
                Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Codec_String), C1, StreamKind!=Stream_Menu); //Specific for Menu: multiple codecs
                Fill(StreamKind, StreamPos, "Codec/Family", MediaInfoLib::Config.Codec_Get(Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Codec)), InfoCodec_KindofCodec, StreamKind), true);
                Ztring B=Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Codec));
                Ztring C=MediaInfoLib::Config.Codec_Get(B, InfoCodec_KindofCodec, StreamKind);
                Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Codec_Info)  , MediaInfoLib::Config.Codec_Get(Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Codec)), InfoCodec_Description, StreamKind), true);
                Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Codec_Url)   , MediaInfoLib::Config.Codec_Get(Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Codec)), InfoCodec_Url,         StreamKind), true);
            }
        }

        //CodecID_Description
        if (Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("CodecID/Info") && Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_CodecID_Description))==Value)
            Clear(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_CodecID_Description));

        //BitRate from BitRate_Nominal
        if (Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("BitRate")
         || Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("BitRate_Nominal"))
        {
            float32 BitRate=Retrieve(StreamKind, StreamPos, "BitRate").To_float32();
            float32 BitRate_Nominal=Retrieve(StreamKind, StreamPos, "BitRate_Nominal").To_float32();
            if (BitRate_Nominal>BitRate*0.95 && BitRate_Nominal<BitRate*1.05)
            {
                Ztring Temp=Retrieve(StreamKind, StreamPos, "BitRate_Nominal");
                Clear(StreamKind, StreamPos, "BitRate_Nominal");
                Fill(StreamKind, StreamPos, "BitRate", Temp, true);
            }
        }

        //File size
        if (StreamKind==Stream_General && Parameter==General_FileSize)
        {
            int64u File_Size_Save=File_Size;
            File_Size=Value.To_int64u();
            for (size_t Kind=Stream_Video; Kind<Stream_Menu; Kind++)
                for (size_t Pos=0; Pos<Count_Get((stream_t)Kind); Pos++)
                    FileSize_FileSize123((stream_t)Kind, Pos, Fill_Parameter((stream_t)Kind, Generic_StreamSize));
            File_Size=File_Size_Save;
        }

        //Delay/Video
        if (StreamKind==Stream_Video && StreamPos==0 && Parameter==Video_Delay)
        {
            for (size_t Pos=0; Pos<Count_Get(Stream_Audio); Pos++)
                if (!Retrieve(Stream_Audio, Pos, Audio_Delay).empty())
                {
                    Fill(Stream_Audio, Pos, Audio_Video_Delay, Retrieve(Stream_Audio, Pos, Audio_Delay).To_int64s()-Value.To_int64s(), 10, true);
                    if (Retrieve(Stream_Audio, Pos, Audio_Video_Delay).To_int64u()==0)
                        for (size_t Param_Pos=Audio_Video_Delay+1; Param_Pos<=Audio_Video_Delay+4; Param_Pos++)
                            if (Param_Pos<(*Stream)[Stream_Audio][Pos].size())
                                (*Stream)[Stream_Audio][Pos][Param_Pos].clear();
                }
            for (size_t Pos=0; Pos<Count_Get(Stream_Text); Pos++)
                if (!Retrieve(Stream_Text, Pos, Text_Delay).empty())
                {
                    Fill(Stream_Text, Pos, Text_Video_Delay, Retrieve(Stream_Text, Pos, Text_Delay).To_int64s()-Value.To_int64s(), 10, true);
                    if (Retrieve(Stream_Text, Pos, Text_Video_Delay).To_int64u()==0)
                        for (size_t Param_Pos=Text_Video_Delay+1; Param_Pos<=Text_Video_Delay+4; Param_Pos++)
                            if (Param_Pos<(*Stream)[Stream_Text][Pos].size())
                                (*Stream)[Stream_Text][Pos][Param_Pos].clear();
                }
        }
        if (StreamKind==Stream_Audio && Parameter==Audio_Delay && Count_Get(Stream_Video) && !Retrieve(Stream_Audio, StreamPos, Audio_Delay).empty() && !Retrieve(Stream_Video, 0, Video_Delay).empty())
        {
            Fill(Stream_Audio, StreamPos, Audio_Video_Delay, Value.To_int64s()-Retrieve(Stream_Video, 0, Video_Delay).To_int64s(), 10, true);
            if (Retrieve(Stream_Audio, StreamPos, Audio_Video_Delay).To_int64u()==0)
                for (size_t Pos=Audio_Video_Delay+1; Pos<=Audio_Video_Delay+4; Pos++)
                    if (Pos<(*Stream)[Stream_Audio][StreamPos].size())
                        (*Stream)[Stream_Audio][StreamPos][Pos].clear();
        }
        if (StreamKind==Stream_Text && Parameter==Text_Delay && Count_Get(Stream_Video) && !Retrieve(Stream_Text, StreamPos, Text_Delay).empty() && !Retrieve(Stream_Video, 0, Video_Delay).empty())
        {
            Fill(Stream_Text, StreamPos, Text_Video_Delay, Value.To_int64s()-Retrieve(Stream_Video, 0, Video_Delay).To_int64s(), 10, true);
            if (Retrieve(Stream_Text, StreamPos, Text_Video_Delay).To_int64u()==0)
                for (size_t Pos=Text_Video_Delay+1; Pos<=Text_Video_Delay+4; Pos++)
                    if (Pos<(*Stream)[Stream_Text][StreamPos].size())
                        (*Stream)[Stream_Text][StreamPos][Pos].clear();
        }

        //Delay/Video0
        if (StreamKind==Stream_Video && StreamPos==0 && Parameter==Video_Delay)
        {
            for (size_t Pos=0; Pos<Count_Get(Stream_Audio); Pos++)
                if (!Retrieve(Stream_Audio, Pos, Audio_Delay).empty())
                {
                    Fill(Stream_Audio, Pos, Audio_Video0_Delay, Retrieve(Stream_Audio, Pos, Audio_Delay).To_int64s()-Value.To_int64s(), 10, true);
                    if (Retrieve(Stream_Audio, Pos, Audio_Video0_Delay).To_int64u()==0)
                        for (size_t Param_Pos=Audio_Video0_Delay+1; Param_Pos<=Audio_Video0_Delay+4; Param_Pos++)
                            if (Param_Pos<(*Stream)[Stream_Audio][Pos].size())
                                (*Stream)[Stream_Audio][Pos][Param_Pos].clear();
                }
            for (size_t Pos=0; Pos<Count_Get(Stream_Text); Pos++)
                if (!Retrieve(Stream_Text, Pos, Text_Delay).empty())
                {
                    Fill(Stream_Text, Pos, Text_Video0_Delay, Retrieve(Stream_Text, Pos, Text_Delay).To_int64s()-Value.To_int64s(), 10, true);
                    if (Retrieve(Stream_Text, Pos, Text_Video0_Delay).To_int64u()==0)
                        for (size_t Param_Pos=Text_Video0_Delay+1; Param_Pos<=Text_Video0_Delay+4; Param_Pos++)
                            if (Param_Pos<(*Stream)[Stream_Text][Pos].size())
                                (*Stream)[Stream_Text][Pos][Param_Pos].clear();
                }
        }
        if (StreamKind==Stream_Audio && Parameter==Audio_Delay && Count_Get(Stream_Video) && !Retrieve(Stream_Audio, StreamPos, Audio_Delay).empty() && !Retrieve(Stream_Video, 0, Video_Delay).empty())
        {
            Fill(Stream_Audio, StreamPos, Audio_Video0_Delay, Value.To_int64s()-Retrieve(Stream_Video, 0, Video_Delay).To_int64s(), 10, true);
            if (Retrieve(Stream_Audio, StreamPos, Audio_Video0_Delay).To_int64u()==0)
                for (size_t Pos=Audio_Video0_Delay+1; Pos<=Audio_Video0_Delay+4; Pos++)
                    if (Pos<(*Stream)[Stream_Audio][StreamPos].size())
                        (*Stream)[Stream_Audio][StreamPos][Pos].clear();
        }
        if (StreamKind==Stream_Text && Parameter==Text_Delay && Count_Get(Stream_Video) && !Retrieve(Stream_Text, StreamPos, Text_Delay).empty() && !Retrieve(Stream_Video, 0, Video_Delay).empty())
        {
            Fill(Stream_Text, StreamPos, Text_Video0_Delay, Value.To_int64s()-Retrieve(Stream_Video, 0, Video_Delay).To_int64s(), 10, true);
            if (Retrieve(Stream_Text, StreamPos, Text_Video0_Delay).To_int64u()==0)
                for (size_t Pos=Text_Video0_Delay+1; Pos<=Text_Video0_Delay+4; Pos++)
                    if (Pos<(*Stream)[Stream_Text][StreamPos].size())
                        (*Stream)[Stream_Text][StreamPos][Pos].clear();
        }

        //Language
        //-Find 2-digit language
        if (Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("Language"))
        {
            //Removing old strings
            Clear(StreamKind, StreamPos, Parameter+1); //String
            Clear(StreamKind, StreamPos, Parameter+2); //String1
            Clear(StreamKind, StreamPos, Parameter+3); //String2
            Clear(StreamKind, StreamPos, Parameter+4); //String3
            Clear(StreamKind, StreamPos, Parameter+5); //String4

            ZtringListList Languages;
            Languages.Separator_Set(0, _T(" / "));
            Languages.Separator_Set(1, _T("-"));
            Languages.Write((*Stream)[StreamKind][StreamPos][Parameter]);

            //Canonizing
            for (size_t Pos=0; Pos<Languages.size(); Pos++)
            {
                Ztring Language_Orig;

                //Removing undefined languages
                if (Languages[Pos].size()>=1)
                {
                    Language_Orig=Languages[Pos][0];
                    Languages[Pos][0].MakeLowerCase();
                    if (Languages[Pos][0].size()==3 && (Languages[Pos][0]==_T("mis") || Languages[Pos][0]==_T("und") || Languages[Pos][0]==_T("???") || Languages[Pos][0]==_T("   "))
                     || Languages[Pos][0].size()==2 && Languages[Pos][0]==_T("  "))
                        Languages[Pos].clear();
                }

                //Finding ISO-639-1 from ISO-639-2 or translated name
                if (Languages[Pos].size()>=1)
                {
                    if (Languages[Pos][0].size()==3 && !MediaInfoLib::Config.Iso639_1_Get(Languages[Pos][0]).empty())
                        Languages[Pos][0]=MediaInfoLib::Config.Iso639_1_Get(Languages[Pos][0]);
                    if (Languages[Pos][0].size()>3 && !MediaInfoLib::Config.Iso639_Find(Languages[Pos][0]).empty())
                        Languages[Pos][0]=MediaInfoLib::Config.Iso639_Find(Languages[Pos][0]);
                    if (Languages[Pos][0].size()>3 && !MediaInfoLib::Config.Language_Get(_T("Language_")+Languages[Pos][0].substr(0, 2)).empty())
                        Languages[Pos][0]=Languages[Pos][0].substr(0, 2);
                    if (Languages[Pos][0].size()>3)
                        Languages[Pos][0]=Language_Orig; //We failed to detect language, using the original version
                }

                //Country name
                if (Languages[Pos].size()>=2)
                    Languages[Pos][1].MakeLowerCase();
            }

            if (Languages.Read()!=Retrieve(StreamKind, StreamPos, Parameter))
                Fill(StreamKind, StreamPos, Parameter, Languages.Read(), true);
            else
            {
                ZtringList Language1; Language1.Separator_Set(0, _T(" / "));
                ZtringList Language2; Language2.Separator_Set(0, _T(" / "));
                ZtringList Language3; Language3.Separator_Set(0, _T(" / "));
                ZtringList Language4; Language4.Separator_Set(0, _T(" / "));

                for (size_t Pos=0; Pos<Languages.size(); Pos++)
                {
                    if (Languages[Pos].size()>=1)
                    {
                        Ztring Language_Translated=MediaInfoLib::Config.Language_Get(_T("Language_")+Languages[Pos][0]);
                        if (Language_Translated.find(_T("Language_"))==0)
                            Language_Translated=Languages[Pos][0]; //No translation found
                        if (Languages[Pos].size()>=2)
                        {
                            Language_Translated+=_T(" (");
                            Language_Translated+=Ztring(Languages[Pos][1]).MakeUpperCase();
                            Language_Translated+=_T(")");
                        }
                        Language1.push_back(Language_Translated);
                        if (Languages[Pos][0].size()==2)
                        {
                            Language2.push_back(Languages[Pos][0]);
                            Language4.push_back(Languages[Pos].Read());
                        }
                        else
                        {
                            Language2.push_back(Ztring());
                            Language4.push_back(Ztring());
                        }
                        if (Languages[Pos][0].size()==3)
                            Language3.push_back(Languages[Pos][0]);
                        else if (!MediaInfoLib::Config.Iso639_2_Get(Languages[Pos][0]).empty())
                            Language3.push_back(MediaInfoLib::Config.Iso639_2_Get(Languages[Pos][0]));
                        else
                            Language3.push_back(Ztring());
                    }
                    else
                    {
                        Language1.push_back(Ztring());
                        Language2.push_back(Ztring());
                        Language3.push_back(Ztring());
                        Language4.push_back(Ztring());
                    }
                }

                Fill(StreamKind, StreamPos, Parameter+2, Language1.Read()); //String1
                Fill(StreamKind, StreamPos, Parameter+3, Language2.Read()); //String2
                Fill(StreamKind, StreamPos, Parameter+4, Language3.Read()); //String3
                Fill(StreamKind, StreamPos, Parameter+5, Language4.Read()); //String4
                Fill(StreamKind, StreamPos, Parameter+1, Retrieve(StreamKind, StreamPos, Parameter+2)); //String
            }
        }

        //ServiceName / ServiceProvider
        if (Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("ServiceName")
         || Retrieve(StreamKind, StreamPos, Parameter, Info_Name)==_T("ServiceProvider"))
        {
            if (Retrieve(StreamKind, StreamPos, Parameter).find(_T(" - "))==string::npos && (Retrieve(StreamKind, StreamPos, Parameter).find(_T(":"))==2 || Retrieve(StreamKind, StreamPos, Parameter).find(_T(":"))==3))
            {
                Ztring Temp=Retrieve(StreamKind, StreamPos, Parameter);
                Temp.erase(0, Retrieve(StreamKind, StreamPos, Parameter).find(_T(":"))+1);
                (*Stream)[StreamKind][StreamPos](Parameter)=Temp;
            }
        }

        //FrameRate Nominal
        if (StreamKind==Stream_Video && (Parameter==Video_FrameRate || Parameter==Video_FrameRate_Nominal))
        {
            float32 FrameRate=Retrieve(Stream_Video, StreamPos, Video_FrameRate).To_float32();
            float32 FrameRate_Nominal=Retrieve(Stream_Video, StreamPos, Video_FrameRate_Nominal).To_float32();
            if (FrameRate_Nominal>FrameRate*0.995 && FrameRate_Nominal<FrameRate*1.005)
            {
                Ztring Temp=Retrieve(StreamKind, StreamPos, Video_FrameRate_Nominal);
                Clear(StreamKind, StreamPos, Video_FrameRate_Nominal);
                if (Parameter==Video_FrameRate)
                    Fill(StreamKind, StreamPos, Parameter, Temp, true);
            }
        }

        //Well known framerate values
        if (StreamKind==Stream_Video && (Parameter==Video_FrameRate || Parameter==Video_FrameRate_Nominal || Parameter==Video_FrameRate_Original))
        {
            Video_FrameRate_Rounding(StreamPos, (video)Parameter);
            if (Retrieve(Stream_Video, StreamPos, Video_FrameRate_Nominal)==Retrieve(Stream_Video, StreamPos, Video_FrameRate))
                Clear(Stream_Video, StreamPos, Video_FrameRate_Nominal);
            if (Parameter!=Video_FrameRate_Original && Retrieve(Stream_Video, StreamPos, Video_FrameRate_Original)==Retrieve(Stream_Video, StreamPos, Video_FrameRate))
                Clear(Stream_Video, StreamPos, Video_FrameRate_Original);
        }

        //Display Aspect Ratio and Pixel Aspect Ratio
        if (StreamKind==Stream_Video && Parameter==Video_DisplayAspectRatio && !Value.empty())
        {
            float F1=Retrieve(Stream_Video, StreamPos, Video_DisplayAspectRatio).To_float32();
            Ztring C1;
                 if (F1>1.23 && F1<1.27) C1=_T("5:4");
            else if (F1>1.30 && F1<1.37) C1=_T("4:3");
            else if (F1>1.45 && F1<1.55) C1=_T("3:2");
            else if (F1>1.70 && F1<1.85) C1=_T("16:9");
            else if (F1>2.10 && F1<2.22) C1=_T("2.2:1");
            else if (F1>2.23 && F1<2.30) C1=_T("2.25:1");
            else if (F1>2.30 && F1<2.40) C1=_T("2.35:1");
            else if (F1>2.37 && F1<2.45) C1=_T("2.40:1");
            else              C1.From_Number(F1);
            C1.FindAndReplace(_T("."), MediaInfoLib::Config.Language_Get(_T("  Config_Text_FloatSeparator")));
            if (MediaInfoLib::Config.Language_Get(_T("  Language_ISO639"))==_T("fr") && C1.find(_T(":1"))==string::npos)
                C1.FindAndReplace(_T(":"), _T("/"));
            Fill(Stream_Video, StreamPos, Video_DisplayAspectRatio_String, C1, true);
        }

        //Original Display Aspect Ratio and Original Pixel Aspect Ratio
        if (StreamKind==Stream_Video && Parameter==Video_DisplayAspectRatio_Original)
        {
            float F1=Retrieve(Stream_Video, StreamPos, Video_DisplayAspectRatio_Original).To_float32();
            Ztring C1;
                 if (F1>1.23 && F1<1.27) C1=_T("5:4");
            else if (F1>1.30 && F1<1.37) C1=_T("4:3");
            else if (F1>1.45 && F1<1.55) C1=_T("3:2");
            else if (F1>1.70 && F1<1.85) C1=_T("16:9");
            else if (F1>2.10 && F1<2.22) C1=_T("2.2:1");
            else if (F1>2.23 && F1<2.30) C1=_T("2.25:1");
            else if (F1>2.30 && F1<2.40) C1=_T("2.35:1");
            else if (F1>2.37 && F1<2.45) C1=_T("2.40:1");
            else              C1.From_Number(F1);
            C1.FindAndReplace(_T("."), MediaInfoLib::Config.Language_Get(_T("  Config_Text_FloatSeparator")));
            if (MediaInfoLib::Config.Language_Get(_T("  Language_ISO639"))==_T("fr") && C1.find(_T(":1"))==string::npos)
                C1.FindAndReplace(_T(":"), _T("/"));
            Fill(Stream_Video, StreamPos, Video_DisplayAspectRatio_Original_String, C1, true);
        }

        //Bits/(Pixel*Frame)
        if (StreamKind==Stream_Video && (Parameter==Video_BitRate || Parameter==Video_BitRate_Nominal || Parameter==Video_Width || Parameter==Video_Height || Parameter==Video_FrameRate))
        {
            float32 BitRate=Retrieve(Stream_Video, StreamPos, Video_BitRate).To_float32();
            if (BitRate==0)
                BitRate=Retrieve(Stream_Video, StreamPos, Video_BitRate_Nominal).To_float32();
            float F1=(float)Retrieve(Stream_Video, StreamPos, Video_Width).To_int32s()*(float)Retrieve(Stream_Video, StreamPos, Video_Height).To_int32s()*Retrieve(Stream_Video, StreamPos, Video_FrameRate).To_float32();
            if (BitRate && F1)
                Fill(Stream_Video, StreamPos, Video_Bits__Pixel_Frame_, BitRate/F1, 3, true);
        }

        //Special audio cases
        if (StreamKind==Stream_Audio && Parameter==Audio_CodecID
         && Retrieve(Stream_Audio, StreamPos, Audio_Channel_s_).empty()
         &&(Value==_T("samr")
         || Value==_T("sawb")
         || Value==_T("7A21")
         || Value==_T("7A22"))
            )
            Fill(Stream_Audio, StreamPos, Audio_Channel_s_, 1, 10, true); //AMR is always with 1 channel

        //Well known bitrate values
        if (StreamKind==Stream_Audio && (Parameter==Audio_BitRate || Parameter==Audio_BitRate_Nominal))
            Audio_BitRate_Rounding(StreamPos, (audio)Parameter);
    }
}

//---------------------------------------------------------------------------
void File__Analyze::Fill (stream_t StreamKind, size_t StreamPos, const char* Parameter, const Ztring &Value, bool Replace)
{
    //Integrity
    if (!Status[IsAccepted] || StreamKind>Stream_Max || Parameter==NULL || Parameter[0]=='\0')
        return;

    //Handling values with \r\n inside
    if (Value.find(_T('\r'))!=string::npos || Value.find(_T('\n'))!=string::npos)
    {
        Ztring NewValue=Value;
        NewValue.FindAndReplace(_T("\r\n"), _T(" / "), 0, Ztring_Recursive);
        NewValue.FindAndReplace(_T("\r"), _T(" / "), 0, Ztring_Recursive);
        NewValue.FindAndReplace(_T("\n"), _T(" / "), 0, Ztring_Recursive);
        if (NewValue.size()>=3 && NewValue.rfind(_T(" / "))==NewValue.size()-3)
            NewValue.resize(NewValue.size()-3);
        Fill(StreamKind, StreamPos, Parameter, NewValue, Replace);
        return;
    }

    //Handle Value before StreamKind
    if (StreamKind==Stream_Max || StreamPos>=(*Stream)[StreamKind].size())
    {
        ZtringList NewList;
        NewList.push_back(Ztring().From_UTF8(Parameter));
        NewList.push_back(Value);
        Fill_Temp.push_back(NewList);
        return; //No streams
    }

    //Handling of well known parameters
    size_t Pos=MediaInfoLib::Config.Info_Get(StreamKind).Find(Ztring().From_Local(Parameter));
    if (Pos!=Error)
    {
        Fill(StreamKind, StreamPos, Pos, Value, Replace);
        return;
    }

    //Handling of unknown parameters
    if (Value.empty())
    {
        if (Replace)
        {
            size_t Pos_ToReplace=(*Stream_More)[StreamKind][StreamPos].Find(Ztring().From_UTF8(Parameter), Info_Name);
            if (Pos_ToReplace!=(size_t)-1)
                (*Stream_More)[StreamKind][StreamPos].erase((*Stream_More)[StreamKind][StreamPos].begin()+Pos_ToReplace); //Empty value --> remove the line
        }
    }
    else
    {
        Ztring &Target=(*Stream_More)[StreamKind][StreamPos](Ztring().From_Local(Parameter), Info_Text);
        if (Target.empty() || Replace)
        {
            Target=Value; //First value
            (*Stream_More)[StreamKind][StreamPos](Ztring().From_Local(Parameter), Info_Name_Text)=MediaInfoLib::Config.Language_Get(Ztring().From_Local(Parameter));
            (*Stream_More)[StreamKind][StreamPos](Ztring().From_Local(Parameter), Info_Options)=_T("Y NT");
        }
        else
        {
            Target+=MediaInfoLib::Config.TagSeparator_Get();
            Target+=Value;
        }
    }
    Fill(StreamKind, StreamPos, (size_t)General_Count, Count_Get(StreamKind, StreamPos), 10, true);
}

//---------------------------------------------------------------------------
const Ztring &File__Analyze::Retrieve_Const (stream_t StreamKind, size_t StreamPos, size_t Parameter, info_t KindOfInfo)
{
    //Integrity
    if (StreamKind>=Stream_Max
     || StreamPos>=(*Stream)[StreamKind].size()
     || Parameter>=(*Stream)[StreamKind][StreamPos].size())
        return MediaInfoLib::Config.EmptyString_Get();

    if (KindOfInfo!=Info_Text)
        return MediaInfoLib::Config.Info_Get(StreamKind, Parameter, KindOfInfo);
    return (*Stream)[StreamKind][StreamPos](Parameter);
}

//---------------------------------------------------------------------------
Ztring File__Analyze::Retrieve (stream_t StreamKind, size_t StreamPos, size_t Parameter, info_t KindOfInfo)
{
    //Integrity
    if (StreamKind>=Stream_Max
     || StreamPos>=(*Stream)[StreamKind].size()
     || Parameter>=(*Stream)[StreamKind][StreamPos].size())
        return MediaInfoLib::Config.EmptyString_Get();

    if (KindOfInfo!=Info_Text)
        return MediaInfoLib::Config.Info_Get(StreamKind, Parameter, KindOfInfo);
    return (*Stream)[StreamKind][StreamPos](Parameter);
}

//---------------------------------------------------------------------------
const Ztring &File__Analyze::Retrieve_Const (stream_t StreamKind, size_t StreamPos, const char* Parameter, info_t KindOfInfo)
{
    //Integrity
    if (StreamKind>=Stream_Max
     || StreamPos>=(*Stream)[StreamKind].size()
     || Parameter==NULL
     || Parameter[0]=='\0')
        return MediaInfoLib::Config.EmptyString_Get();

    if (KindOfInfo!=Info_Text)
        return MediaInfoLib::Config.Info_Get(StreamKind, Parameter, KindOfInfo);
    size_t Parameter_Pos=MediaInfoLib::Config.Info_Get(StreamKind).Find(Ztring().From_Local(Parameter));
    if (Parameter_Pos==Error)
    {
        Parameter_Pos=(*Stream_More)[StreamKind][StreamPos].Find(Ztring().From_Local(Parameter));
        if (Parameter_Pos==Error)
            return MediaInfoLib::Config.EmptyString_Get();
        return (*Stream_More)[StreamKind][StreamPos](Parameter_Pos, 1);
    }
    return (*Stream)[StreamKind][StreamPos](Parameter_Pos);
}

//---------------------------------------------------------------------------
Ztring File__Analyze::Retrieve (stream_t StreamKind, size_t StreamPos, const char* Parameter, info_t KindOfInfo)
{
    //Integrity
    if (StreamKind>=Stream_Max
     || StreamPos>=(*Stream)[StreamKind].size()
     || Parameter==NULL
     || Parameter[0]=='\0')
        return MediaInfoLib::Config.EmptyString_Get();

    if (KindOfInfo!=Info_Text)
        return MediaInfoLib::Config.Info_Get(StreamKind, Parameter, KindOfInfo);
    size_t Parameter_Pos=MediaInfoLib::Config.Info_Get(StreamKind).Find(Ztring().From_Local(Parameter));
    if (Parameter_Pos==Error)
    {
        Parameter_Pos=(*Stream_More)[StreamKind][StreamPos].Find(Ztring().From_Local(Parameter));
        if (Parameter_Pos==Error)
            return MediaInfoLib::Config.EmptyString_Get();
        return (*Stream_More)[StreamKind][StreamPos](Parameter_Pos, 1);
    }
    return (*Stream)[StreamKind][StreamPos](Parameter_Pos);
}

//---------------------------------------------------------------------------
void File__Analyze::Clear (stream_t StreamKind, size_t StreamPos, const char* Parameter)
{
    //Integrity
    if (StreamKind>=Stream_Max
     || StreamPos>=(*Stream)[StreamKind].size()
     || Parameter==NULL
     || Parameter[0]=='\0')
        return;

    size_t Parameter_Pos=MediaInfoLib::Config.Info_Get(StreamKind).Find(Ztring().From_Local(Parameter));
    if (Parameter_Pos==Error)
    {
        Parameter_Pos=(*Stream_More)[StreamKind][StreamPos].Find(Ztring().From_Local(Parameter));
        if (Parameter_Pos==Error)
            return;
        (*Stream_More)[StreamKind][StreamPos](Parameter_Pos, 1).clear();
        return;
    }

    Clear(StreamKind, StreamPos, Parameter_Pos);
}

//---------------------------------------------------------------------------
void File__Analyze::Clear (stream_t StreamKind, size_t StreamPos, size_t Parameter)
{
    //Integrity
    if (StreamKind>=Stream_Max
     || StreamPos>=(*Stream)[StreamKind].size())
        return;

    //Normal
    if (Parameter<MediaInfoLib::Config.Info_Get(StreamKind).size())
    {
        //Is something available?
        if (Parameter>=(*Stream)[StreamKind][StreamPos].size())
            return; //Was never filled, no nead to clear it

        //Clearing
        (*Stream)[StreamKind][StreamPos][Parameter].clear();

        //Human readable
        if (MediaInfoLib::Config.ReadByHuman_Get())
        {
            //Strings
            const Ztring &List_Measure_Value=MediaInfoLib::Config.Info_Get(StreamKind).Read(Parameter, Info_Measure);
                 if (List_Measure_Value==_T(" byte"))
            {
                for (size_t Pos=Parameter+1; Pos<=Parameter+5; Pos++)
                    if (Pos<(*Stream)[StreamKind][StreamPos].size())
                        (*Stream)[StreamKind][StreamPos][Pos].clear();
            }
            else if (List_Measure_Value==_T(" bps") || List_Measure_Value==_T(" Hz"))
            {
                if (Parameter+1<(*Stream)[StreamKind][StreamPos].size())
                    (*Stream)[StreamKind][StreamPos][Parameter+1].clear();
            }
            else if (List_Measure_Value==_T(" ms"))
            {
                for (size_t Pos=Parameter+1; Pos<=Parameter+4; Pos++)
                    if (Pos<(*Stream)[StreamKind][StreamPos].size())
                        (*Stream)[StreamKind][StreamPos][Pos].clear();
            }
            else if (List_Measure_Value==_T("Yes"))
            {
                if (Parameter+1<(*Stream)[StreamKind][StreamPos].size())
                    (*Stream)[StreamKind][StreamPos][Parameter+1].clear();
            }
            else if (!List_Measure_Value.empty())
            {
                if (Parameter+1<(*Stream)[StreamKind][StreamPos].size())
                    (*Stream)[StreamKind][StreamPos][Parameter+1].clear();
            }
        }

        return;
    }

    //More
    Parameter-=(*Stream)[StreamKind][StreamPos].size(); //For having Stream_More position
    if (Parameter<(*Stream_More)[StreamKind][StreamPos].size())
    {
        (*Stream_More)[StreamKind][StreamPos].erase((*Stream_More)[StreamKind][StreamPos].begin()+Parameter);
        return;
    }
}

//---------------------------------------------------------------------------
void File__Analyze::Clear (stream_t StreamKind, size_t StreamPos)
{
    //Integrity
    if (StreamKind>=Stream_Max
     || StreamPos>=(*Stream)[StreamKind].size())
        return;

    (*Stream)[StreamKind].erase((*Stream)[StreamKind].begin()+StreamPos);
}

//---------------------------------------------------------------------------
void File__Analyze::Clear (stream_t StreamKind)
{
    //Integrity
    if (StreamKind>=Stream_Max)
        return;

    (*Stream)[StreamKind].clear();
}

//---------------------------------------------------------------------------
void File__Analyze::Fill_Flush()
{
    Stream_Prepare(Stream_Max); //clear filling
    Fill_Temp.clear();
}

//---------------------------------------------------------------------------
size_t File__Analyze::Merge(MediaInfo_Internal &ToAdd, bool)
{
    size_t Count=0;
    for (size_t StreamKind=(size_t)Stream_General; StreamKind<(size_t)Stream_Max; StreamKind++)
    {
        size_t StreamPos_Count=ToAdd.Count_Get((stream_t)StreamKind);
        for (size_t StreamPos=0; StreamPos<StreamPos_Count; StreamPos++)
        {
            //Prepare a new stream
            if (StreamPos>=Count_Get((stream_t)StreamKind))
                Stream_Prepare((stream_t)StreamKind);

            //Merge
            size_t Pos_Count=ToAdd.Count_Get((stream_t)StreamKind, StreamPos);
            for (size_t Pos=0; Pos<Pos_Count; Pos++)
            {
                if (StreamKind!=Stream_General
                 || !(Pos==General_CompleteName
                   || Pos==General_FolderName
                   || Pos==General_FileName
                   || Pos==General_FileExtension
                   || Pos==General_File_Created_Date
                   || Pos==General_Format
                   || Pos==General_Format_String
                   || Pos==General_Format_Extensions
                   || Pos==General_Format_Info
                   || Pos==General_Codec
                   || Pos==General_Codec_String
                   || Pos==General_Codec_Extensions
                   || Pos==General_FileSize
                   || Pos==General_FileSize_String
                   || Pos==General_FileSize_String1
                   || Pos==General_FileSize_String2
                   || Pos==General_FileSize_String3
                   || Pos==General_FileSize_String4
                   || Pos==General_File_Created_Date_Local
                   || Pos==General_File_Modified_Date
                   || Pos==General_File_Modified_Date_Local))
                    Fill((stream_t)StreamKind, StreamPos, Ztring(ToAdd.Get((stream_t)StreamKind, StreamPos, Pos, Info_Name)).To_UTF8().c_str(), ToAdd.Get((stream_t)StreamKind, StreamPos, Pos), true);
            }

            Count++;
        }
    }

    return Count;
}

//---------------------------------------------------------------------------
size_t File__Analyze::Merge(MediaInfo_Internal &ToAdd, stream_t StreamKind, size_t StreamPos_From, size_t StreamPos_To, bool)
{
    size_t Pos_Count=ToAdd.Count_Get(StreamKind, StreamPos_From);
    for (size_t Pos=0; Pos<Pos_Count; Pos++)
        if (!ToAdd.Get(StreamKind, StreamPos_From, Pos).empty())
            Fill(StreamKind, StreamPos_To, Ztring(ToAdd.Get((stream_t)StreamKind, StreamPos_From, Pos, Info_Name)).To_UTF8().c_str(), ToAdd.Get(StreamKind, StreamPos_From, Pos), true);

    return 1;
}

//---------------------------------------------------------------------------
size_t File__Analyze::Merge(File__Analyze &ToAdd, bool Erase)
{
    size_t Count=0;
    for (size_t StreamKind=(size_t)Stream_General+1; StreamKind<(size_t)Stream_Max; StreamKind++)
        for (size_t StreamPos=0; StreamPos<(*ToAdd.Stream)[StreamKind].size(); StreamPos++)
        {
            //Prepare a new stream
            Stream_Prepare((stream_t)StreamKind);

            //Merge
            Merge(ToAdd, (stream_t)StreamKind, StreamPos, StreamPos_Last, Erase);

            Count++;
        }
    return Count;
}

//---------------------------------------------------------------------------
size_t File__Analyze::Merge(File__Analyze &ToAdd, stream_t StreamKind, size_t StreamPos_From, size_t StreamPos_To, bool Erase)
{
    //Integrity
    if (&ToAdd==NULL || StreamKind>=Stream_Max || !ToAdd.Stream || StreamPos_From>=(*ToAdd.Stream)[StreamKind].size())
        return 0;

    //Destination
    while (StreamPos_To>=(*Stream)[StreamKind].size())
        Stream_Prepare(StreamKind);

    //Specific stuff
    Ztring Width_Temp, Height_Temp, PixelAspectRatio_Temp, DisplayAspectRatio_Temp, FrameRate_Temp, FrameRate_Mode_Temp, Delay_Temp, Delay_Source_Temp, Delay_Settings_Temp;
    if (StreamKind==Stream_Video)
    {
        Width_Temp=Retrieve(Stream_Video, StreamPos_Last, Video_Width);
        Height_Temp=Retrieve(Stream_Video, StreamPos_Last, Video_Height);
        PixelAspectRatio_Temp=Retrieve(Stream_Video, StreamPos_Last, Video_PixelAspectRatio); //We want to keep the PixelAspectRatio_Temp of the video stream
        DisplayAspectRatio_Temp=Retrieve(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio); //We want to keep the DisplayAspectRatio_Temp of the video stream
        FrameRate_Temp=Retrieve(Stream_Video, StreamPos_Last, Video_FrameRate); //We want to keep the FrameRate of AVI 120 fps
        FrameRate_Mode_Temp=Retrieve(Stream_Video, StreamPos_Last, Video_FrameRate_Mode); //We want to keep the FrameRate_Mode of AVI 120 fps
    }
    if (ToAdd.Retrieve(StreamKind, StreamPos_From, Fill_Parameter(StreamKind, Generic_Delay_Source))==_T("Container"))
    {
        Fill(StreamKind_Last, StreamPos_Last, "Delay_Original", Retrieve(StreamKind_Last, StreamPos_Last, "Delay"), true);
        Clear(StreamKind_Last, StreamPos_Last, "Delay");
        Fill(StreamKind_Last, StreamPos_Last, "Delay_Original_Source", Retrieve(StreamKind_Last, StreamPos_Last, "Delay_Source"), true);
        Clear(StreamKind_Last, StreamPos_Last, "Delay_Source");
        if (!ToAdd.Retrieve(StreamKind_Last, StreamPos_Last, "Format").empty()) //Exception: MPEG-4 TimeCode, settings are in the MPEG-4 header
        {
            Fill(StreamKind_Last, StreamPos_Last, "Delay_Original_Settings", Retrieve(StreamKind_Last, StreamPos_Last, "Delay_Settings"), true);
            Clear(StreamKind_Last, StreamPos_Last, "Delay_Settings");
        }
    }
    else
    {
        Delay_Temp=Retrieve(StreamKind, StreamPos_Last, "Delay"); //We want to keep the Delay from the stream
        Delay_Settings_Temp=Retrieve(StreamKind, StreamPos_Last, "Delay_Settings"); //We want to keep the Delay_Settings from the stream
        Delay_Source_Temp=Retrieve(StreamKind, StreamPos_Last, "Delay_Source"); //We want to keep the Delay_Source from the stream
    }

    //Merging
    size_t Count=0;
    size_t Size=ToAdd.Count_Get(StreamKind, StreamPos_From);
    for (size_t Pos=General_Inform; Pos<Size; Pos++)
    {
        const Ztring &ToFill_Value=ToAdd.Get(StreamKind, StreamPos_From, Pos);
        if (!ToFill_Value.empty() && (Erase || Get(StreamKind, StreamPos_From, Pos).empty()))
        {
            if (Pos<MediaInfoLib::Config.Info_Get(StreamKind).size())
                Fill(StreamKind, StreamPos_To, Pos, ToFill_Value, true);
            else
            {
                Fill(StreamKind, StreamPos_To, ToAdd.Get(StreamKind, StreamPos_From, Pos, Info_Name).To_UTF8().c_str(), ToFill_Value, true);
                (*Stream_More)[StreamKind][StreamPos_To](ToAdd.Get(StreamKind, StreamPos_From, Pos, Info_Name), Info_Options)=ToAdd.Get(StreamKind, StreamPos_From, Pos, Info_Options);
            }
            Count++;
        }
    }

    //Clearing
    for (size_t Pos=Size-1; Pos>=General_Inform; Pos--) //Descendant because Clear() remove the unknown fields
    {
        const Ztring &ToFill_Value=ToAdd.Get(StreamKind, StreamPos_From, Pos);
        if (!ToFill_Value.empty())
            ToAdd.Clear(StreamKind, StreamPos_From, Pos);
    }

    //Specific stuff
    if (StreamKind==Stream_Video)
    {
        Ztring PixelAspectRatio_Original=Retrieve(Stream_Video, StreamPos_Last, Video_PixelAspectRatio);
        Ztring DisplayAspectRatio_Original=Retrieve(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio);

        if (!Width_Temp.empty() && Width_Temp!=Retrieve(Stream_Video, StreamPos_Last, Video_Width))
        {
            Fill(Stream_Video, StreamPos_Last, Video_Width_Original, (*Stream)[Stream_Video][StreamPos_Last][Video_Width], true);
            Fill(Stream_Video, StreamPos_Last, Video_Width, Width_Temp, true);
        }
        if (!Height_Temp.empty() && Height_Temp!=Retrieve(Stream_Video, StreamPos_Last, Video_Height))
        {
            Fill(Stream_Video, StreamPos_Last, Video_Height_Original, (*Stream)[Stream_Video][StreamPos_Last][Video_Height], true);
            Fill(Stream_Video, StreamPos_Last, Video_Height, Height_Temp, true);
        }
        if (!PixelAspectRatio_Temp.empty() && PixelAspectRatio_Temp!=Retrieve(Stream_Video, StreamPos_Last, Video_PixelAspectRatio))
        {
            Fill(Stream_Video, StreamPos_Last, Video_PixelAspectRatio_Original, PixelAspectRatio_Original, true);
            Fill(Stream_Video, StreamPos_Last, Video_PixelAspectRatio, PixelAspectRatio_Temp, true);
        }
        if (!DisplayAspectRatio_Temp.empty() && DisplayAspectRatio_Temp!=DisplayAspectRatio_Original)
        {
            Fill(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio_Original, DisplayAspectRatio_Original, true);
            Fill(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio, DisplayAspectRatio_Temp, true);
        }
        if (!FrameRate_Temp.empty() && FrameRate_Temp!=Retrieve(Stream_Video, StreamPos_Last, Video_FrameRate))
        {
            Fill(Stream_Video, StreamPos_Last, Video_FrameRate_Original, (*Stream)[Stream_Video][StreamPos_Last][Video_FrameRate], true);
            Fill(Stream_Video, StreamPos_Last, Video_FrameRate, FrameRate_Temp, true);
        }
        if (!FrameRate_Mode_Temp.empty() && FrameRate_Mode_Temp!=Retrieve(Stream_Video, StreamPos_Last, Video_FrameRate_Mode))
        {
            Fill(Stream_Video, StreamPos_Last, Video_FrameRate_Mode_Original, (*Stream)[Stream_Video][StreamPos_Last][Video_FrameRate_Mode], true);
            Fill(Stream_Video, StreamPos_Last, Video_FrameRate_Mode, FrameRate_Mode_Temp, true);
        }
    }
    if (!Delay_Source_Temp.empty() && Delay_Source_Temp!=Retrieve(StreamKind_Last, StreamPos_Last, "Delay_Source"))
    {
        Fill(StreamKind_Last, StreamPos_Last, "Delay_Original", Retrieve(StreamKind_Last, StreamPos_Last, "Delay"), true);
        Fill(StreamKind_Last, StreamPos_Last, "Delay", Delay_Temp, true);
        Fill(StreamKind_Last, StreamPos_Last, "Delay_Original_Settings", Retrieve(StreamKind_Last, StreamPos_Last, "Delay_Settings"), true);
        Fill(StreamKind_Last, StreamPos_Last, "Delay_Settings", Delay_Settings_Temp, true);
        Fill(StreamKind_Last, StreamPos_Last, "Delay_Original_Source", Retrieve(StreamKind_Last, StreamPos_Last, "Delay_Source"), true);
        Fill(StreamKind_Last, StreamPos_Last, "Delay_Source", Delay_Source_Temp, true);
    }

    Fill(StreamKind, StreamPos_To, (size_t)General_Count, Count_Get(StreamKind, StreamPos_To), 10, true);
    return 1;
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Video_FrameRate_Rounding(size_t Pos, video Parameter)
{
    float64 FrameRate=Retrieve(Stream_Video, Pos, Parameter).To_float64();
    float64 FrameRate_Sav=FrameRate;

         if (FrameRate> 9.990 && FrameRate<=10.010) FrameRate=10.000;
    else if (FrameRate>14.990 && FrameRate<=15.010) FrameRate=15.000;
    else if (FrameRate>23.964 && FrameRate<=23.988) FrameRate=23.976;
    else if (FrameRate>23.988 && FrameRate<=24.012) FrameRate=24.000;
    else if (FrameRate>24.988 && FrameRate<=25.012) FrameRate=25.000;
    else if (FrameRate>29.955 && FrameRate<=29.985) FrameRate=29.970;
    else if (FrameRate>29.985 && FrameRate<=30.015) FrameRate=30.000;
    else if (FrameRate>23.964*2 && FrameRate<=23.988*2) FrameRate=23.976*2;
    else if (FrameRate>23.988*2 && FrameRate<=24.012*2) FrameRate=24.000*2;
    else if (FrameRate>24.988*2 && FrameRate<=25.012*2) FrameRate=25.000*2;
    else if (FrameRate>29.955*2 && FrameRate<=29.985*2) FrameRate=29.970*2;
    else if (FrameRate>30.985*2 && FrameRate<=30.015*2) FrameRate=30.000*2;

    if (FrameRate!=FrameRate_Sav)
        Fill(Stream_Video, Pos, Parameter, FrameRate, 3, true);
}

//---------------------------------------------------------------------------
void File__Analyze::Audio_BitRate_Rounding(size_t Pos, audio Parameter)
{
    const Ztring& Format=Retrieve(Stream_Audio, Pos, Audio_Format);
    const Ztring& Codec=Retrieve(Stream_Audio, Pos, Audio_Codec);
    int32u BitRate=Retrieve(Stream_Audio, Pos, Parameter).To_int32u();
    int32u BitRate_Sav=BitRate;
    if (MediaInfoLib::Config.Codec_Get(Codec, InfoCodec_KindofCodec, Stream_Audio).find(_T("MPEG-"))==0
     || Retrieve(Stream_Audio, Pos, Audio_Codec_String).find(_T("MPEG-"))==0)
    {
        if (BitRate>=   7500 && BitRate<=   8500) BitRate=   8000;
        if (BitRate>=  15000 && BitRate<=  17000) BitRate=  16000;
        if (BitRate>=  23000 && BitRate<=  25000) BitRate=  24000;
        if (BitRate>=  31000 && BitRate<=  33000) BitRate=  32000;
        if (BitRate>=  38000 && BitRate<=  42000) BitRate=  40000;
        if (BitRate>=  46000 && BitRate<=  50000) BitRate=  48000;
        if (BitRate>=  54000 && BitRate<=  58000) BitRate=  56000;
        if (BitRate>=  62720 && BitRate<=  65280) BitRate=  64000;
        if (BitRate>=  78400 && BitRate<=  81600) BitRate=  80000;
        if (BitRate>=  94080 && BitRate<=  97920) BitRate=  96000;
        if (BitRate>= 109760 && BitRate<= 114240) BitRate= 112000;
        if (BitRate>= 125440 && BitRate<= 130560) BitRate= 128000;
        if (BitRate>= 156800 && BitRate<= 163200) BitRate= 160000;
        if (BitRate>= 156800 && BitRate<= 163200) BitRate= 160000;
        if (BitRate>= 188160 && BitRate<= 195840) BitRate= 192000;
        if (BitRate>= 219520 && BitRate<= 228480) BitRate= 224000;
        if (BitRate>= 219520 && BitRate<= 228480) BitRate= 224000;
        if (BitRate>= 250880 && BitRate<= 261120) BitRate= 256000;
        if (BitRate>= 282240 && BitRate<= 293760) BitRate= 288000;
        if (BitRate>= 313600 && BitRate<= 326400) BitRate= 320000;
        if (BitRate>= 344960 && BitRate<= 359040) BitRate= 352000;
        if (BitRate>= 376320 && BitRate<= 391680) BitRate= 384000;
        if (BitRate>= 407680 && BitRate<= 424320) BitRate= 416000;
        if (BitRate>= 439040 && BitRate<= 456960) BitRate= 448000;
        if (Retrieve(Stream_Audio, Pos, "BitRate_Mode")==_T("VBR"))
            BitRate=BitRate_Sav; //If VBR, we want the exact value
    }

    else if (MediaInfoLib::Config.Codec_Get(Codec, InfoCodec_Name, Stream_Audio).find(_T("AC3"))==0)
    {
        if (BitRate>=  31000 && BitRate<=  33000) BitRate=  32000;
        if (BitRate>=  39000 && BitRate<=  41000) BitRate=  40000;
        if (BitRate>=  46000 && BitRate<=  50000) BitRate=  48000;
        if (BitRate>=  54000 && BitRate<=  58000) BitRate=  56000;
        if (BitRate>=  62720 && BitRate<=  65280) BitRate=  64000;
        if (BitRate>=  78400 && BitRate<=  81600) BitRate=  80000;
        if (BitRate>=  94080 && BitRate<=  97920) BitRate=  96000;
        if (BitRate>= 109760 && BitRate<= 114240) BitRate= 112000;
        if (BitRate>= 125440 && BitRate<= 130560) BitRate= 128000;
        if (BitRate>= 156800 && BitRate<= 163200) BitRate= 160000;
        if (BitRate>= 188160 && BitRate<= 195840) BitRate= 192000;
        if (BitRate>= 219520 && BitRate<= 228480) BitRate= 224000;
        if (BitRate>= 250880 && BitRate<= 261120) BitRate= 256000;
        if (BitRate>= 313600 && BitRate<= 326400) BitRate= 320000;
        if (BitRate>= 376320 && BitRate<= 391680) BitRate= 384000;
        if (BitRate>= 439040 && BitRate<= 456960) BitRate= 448000;
        if (BitRate>= 501760 && BitRate<= 522240) BitRate= 512000;
        if (BitRate>= 564480 && BitRate<= 587520) BitRate= 576000;
        if (BitRate>= 627200 && BitRate<= 652800) BitRate= 640000;
  }

    else if (MediaInfoLib::Config.Codec_Get(Codec, InfoCodec_Name, Stream_Audio).find(_T("DTS"))==0)
    {
        if (BitRate>=  31000 && BitRate<=  33000) BitRate=  32000;
        if (BitRate>=  54000 && BitRate<=  58000) BitRate=  56000;
        if (BitRate>=  62720 && BitRate<=  65280) BitRate=  64000;
        if (BitRate>=  94080 && BitRate<=  97920) BitRate=  96000;
        if (BitRate>= 109760 && BitRate<= 114240) BitRate= 112000;
        if (BitRate>= 125440 && BitRate<= 130560) BitRate= 128000;
        if (BitRate>= 188160 && BitRate<= 195840) BitRate= 192000;
        if (BitRate>= 219520 && BitRate<= 228480) BitRate= 224000;
        if (BitRate>= 250880 && BitRate<= 261120) BitRate= 256000;
        if (BitRate>= 313600 && BitRate<= 326400) BitRate= 320000;
        if (BitRate>= 376320 && BitRate<= 391680) BitRate= 384000;
        if (BitRate>= 439040 && BitRate<= 456960) BitRate= 448000;
        if (BitRate>= 501760 && BitRate<= 522240) BitRate= 512000;
        if (BitRate>= 564480 && BitRate<= 587520) BitRate= 576000;
        if (BitRate>= 627200 && BitRate<= 652800) BitRate= 640000;
        if (BitRate>= 752640 && BitRate<= 783360) BitRate= 768000;
        if (BitRate>= 940800 && BitRate<= 979200) BitRate= 960000;
        if (BitRate>=1003520 && BitRate<=1044480) BitRate=1024000;
        if (BitRate>=1128960 && BitRate<=1175040) BitRate=1152000;
        if (BitRate>=1254400 && BitRate<=1305600) BitRate=1280000;
        if (BitRate>=1317120 && BitRate<=1370880) BitRate=1344000;
        if (BitRate>=1379840 && BitRate<=1436160) BitRate=1408000;
        if (BitRate>=1382976 && BitRate<=1439424) BitRate=1411200;
        if (BitRate>=1442560 && BitRate<=1501440) BitRate=1472000;
        if (BitRate>=1505280 && BitRate<=1566720) BitRate=1536000;
        if (BitRate>=1881600 && BitRate<=1958400) BitRate=1920000;
        if (BitRate>=2007040 && BitRate<=2088960) BitRate=2048000;
        if (BitRate>=3010560 && BitRate<=3133440) BitRate=3072000;
        if (BitRate>=3763200 && BitRate<=3916800) BitRate=3840000;
    }

    else if (MediaInfoLib::Config.Codec_Get(Codec, InfoCodec_Name, Stream_Audio).find(_T("AAC"))==0)
    {
        if (BitRate>=  46000 && BitRate<=  50000) BitRate=  48000;
        if (BitRate>=  64827 && BitRate<=  67473) BitRate=  66150;
        if (BitRate>=  70560 && BitRate<=  73440) BitRate=  72000;
        if (BitRate>=  94080 && BitRate<=  97920) BitRate=  96000;
        if (BitRate>=  94080 && BitRate<=  97920) BitRate=  96000;
        if (BitRate>= 129654 && BitRate<= 134946) BitRate= 132300;
        if (BitRate>= 141120 && BitRate<= 146880) BitRate= 144000;
        if (BitRate>= 188160 && BitRate<= 195840) BitRate= 192000;
        if (BitRate>= 259308 && BitRate<= 269892) BitRate= 264600;
        if (BitRate>= 282240 && BitRate<= 293760) BitRate= 288000;
        if (BitRate>= 345744 && BitRate<= 359856) BitRate= 352800;
        if (BitRate>= 376320 && BitRate<= 391680) BitRate= 384000;
        if (BitRate>= 518616 && BitRate<= 539784) BitRate= 529200;
        if (BitRate>= 564480 && BitRate<= 587520) BitRate= 576000;
        if (BitRate>= 648270 && BitRate<= 674730) BitRate= 661500;
    }

    else if (Codec==_T("PCM") || Codec==_T("QDM2") || MediaInfoLib::Config.Codec_Get(Codec, InfoCodec_Name, Stream_Audio).find(_T("PCM"))==0)
    {
        if (BitRate>=  62720 && BitRate<=  65280) BitRate=  64000;
        if (BitRate>=  86436 && BitRate<=  89964) BitRate=  88200;
        if (BitRate>= 125440 && BitRate<= 130560) BitRate= 128000;
        if (BitRate>= 172872 && BitRate<= 179928) BitRate= 176400;
        if (BitRate>= 188160 && BitRate<= 195840) BitRate= 192000;
        if (BitRate>= 250880 && BitRate<= 261120) BitRate= 256000;
        if (BitRate>= 345744 && BitRate<= 359856) BitRate= 352800;
        if (BitRate>= 376320 && BitRate<= 391680) BitRate= 384000;
        if (BitRate>= 501760 && BitRate<= 522240) BitRate= 512000;
        if (BitRate>= 691488 && BitRate<= 719712) BitRate= 705600;
        if (BitRate>= 752640 && BitRate<= 783360) BitRate= 768000;
        if (BitRate>=1003520 && BitRate<=1044480) BitRate=1024000;
        if (BitRate>=1128960 && BitRate<=1175040) BitRate=1152000;
        if (BitRate>=1382976 && BitRate<=1439424) BitRate=1411200;
        if (BitRate>=1505280 && BitRate<=1566720) BitRate=1536000;
        if (BitRate>=4515840 && BitRate<=4700160) BitRate=4608000;
        if (BitRate>=6021120 && BitRate<=6266880) BitRate=6144000;
    }

    else if (MediaInfoLib::Config.Codec_Get(Codec, InfoCodec_Name, Stream_Audio).find(_T("ADPCM"))==0
          || MediaInfoLib::Config.Codec_Get(Codec, InfoCodec_Name, Stream_Audio).find(_T("U-Law"))==0
          || MediaInfoLib::Config.Codec_Get(Codec, InfoCodec_KindofCodec, Stream_Audio)==_T("ADPCM")
          || MediaInfoLib::Config.Codec_Get(Codec, InfoCodec_KindofCodec, Stream_Audio)==_T("U-Law")
          || Format==_T("ADPCM"))
    {
        if (BitRate>=  42000 && BitRate<=  46000) BitRate=  44100;
        if (BitRate>=  62720 && BitRate<=  65280) BitRate=  64000;
        if (BitRate>=  86436 && BitRate<=  89964) BitRate=  88200;
        if (BitRate>= 125440 && BitRate<= 130560) BitRate= 128000;
        if (BitRate>= 172872 && BitRate<= 179928) BitRate= 176400;
        if (BitRate>= 188160 && BitRate<= 195840) BitRate= 192000;
        if (BitRate>= 250880 && BitRate<= 261120) BitRate= 256000;
        if (BitRate>= 345744 && BitRate<= 359856) BitRate= 352800;
        if (BitRate>= 376320 && BitRate<= 391680) BitRate= 384000;
    }

    if (BitRate!=BitRate_Sav)
        Fill(Stream_Audio, Pos, Parameter, BitRate, 0, true);
}

//---------------------------------------------------------------------------
void File__Analyze::Tags()
{
    //Integrity
    if (!Count_Get(Stream_General))
        return;

    //-Movie/Album
    if (!Retrieve(Stream_General, 0, General_Title).empty() && Retrieve(Stream_General, 0, General_Movie).empty() && Retrieve(Stream_General, 0, General_Track).empty())
    {
        if (Count_Get(Stream_Video))
            Fill(Stream_General, 0, "Movie", Retrieve(Stream_General, 0, General_Title));
        else
            Fill(Stream_General, 0, "Track", Retrieve(Stream_General, 0, General_Title));
    }
    if (!Retrieve(Stream_General, 0, General_Title_More).empty() && Retrieve(Stream_General, 0, General_Movie_More).empty() && Retrieve(Stream_General, 0, General_Track_More).empty())
    {
        if (Count_Get(Stream_Video))
            Fill(Stream_General, 0, "Movie/More", Retrieve(Stream_General, 0, General_Title_More));
        else
            Fill(Stream_General, 0, "Track/More", Retrieve(Stream_General, 0, General_Title_More));
    }
    if (!Retrieve(Stream_General, 0, General_Title_Url).empty() && Retrieve(Stream_General, 0, General_Movie_Url).empty() && Retrieve(Stream_General, 0, General_Track_Url).empty())
    {
        if (Count_Get(Stream_Video))
            Fill(Stream_General, 0, "Movie/Url", Retrieve(Stream_General, 0, General_Title_Url));
        else
            Fill(Stream_General, 0, "Track/Url", Retrieve(Stream_General, 0, General_Title_Url));
    }
    //-Title
    if (Retrieve(Stream_General, 0, General_Title).empty() && !Retrieve(Stream_General, 0, General_Movie).empty())
        Fill(Stream_General, 0, "Title", Retrieve(Stream_General, 0, General_Movie));
    if (Retrieve(Stream_General, 0, General_Title).empty() && !Retrieve(Stream_General, 0, General_Track).empty())
        Fill(Stream_General, 0, "Title", Retrieve(Stream_General, 0, General_Track));
    if (Retrieve(Stream_General, 0, General_Title_More).empty() && !Retrieve(Stream_General, 0, General_Movie_More).empty())
        Fill(Stream_General, 0, "Title/More", Retrieve(Stream_General, 0, General_Movie_More));
    if (Retrieve(Stream_General, 0, General_Title_More).empty() && !Retrieve(Stream_General, 0, General_Track_More).empty())
        Fill(Stream_General, 0, "Title/More", Retrieve(Stream_General, 0, General_Track_More));
    if (Retrieve(Stream_General, 0, General_Title_Url).empty() && !Retrieve(Stream_General, 0, General_Movie_Url).empty())
        Fill(Stream_General, 0, "Title/Url", Retrieve(Stream_General, 0, General_Movie_Url));
    if (Retrieve(Stream_General, 0, General_Title_Url).empty() && !Retrieve(Stream_General, 0, General_Track_Url).empty())
        Fill(Stream_General, 0, "Title/Url", Retrieve(Stream_General, 0, General_Track_Url));

    //-Genre
    if (!Retrieve(Stream_General, 0, General_Genre).empty() && Retrieve(Stream_General, 0, General_Genre).size()<4 && Retrieve(Stream_General, 0, General_Genre)[0]>=_T('0') && Retrieve(Stream_General, 0, General_Genre)[0]<=_T('9'))
    {
        Ztring Genre;
        if (Retrieve(Stream_General, 0, General_Genre).size()==1) Genre=Ztring(_T("Genre_00"))+Retrieve(Stream_General, 0, General_Genre);
        if (Retrieve(Stream_General, 0, General_Genre).size()==2) Genre=Ztring(_T("Genre_0" ))+Retrieve(Stream_General, 0, General_Genre);
        if (Retrieve(Stream_General, 0, General_Genre).size()==3) Genre=Ztring(_T("Genre_"  ))+Retrieve(Stream_General, 0, General_Genre);
        Fill(Stream_General, 0, "Genre", MediaInfoLib::Config.Language_Get(Genre), true);
    }
}

//***************************************************************************
// Internal Functions
//***************************************************************************

//---------------------------------------------------------------------------
//Duration
void File__Analyze::Duration_Duration123(stream_t StreamKind, size_t StreamPos, size_t Parameter)
{
    if (Retrieve(StreamKind, StreamPos, Parameter).empty())
        return;

    int32s HH, MM, SS, MS;
    Ztring DurationString1, DurationString2, DurationString3;
    bool Negative=false;
    MS=Retrieve(StreamKind, StreamPos, Parameter).To_int32s(); //en ms

    if (MS<0)
    {
        Negative=true;
        MS=-MS;
    }

    //Hours
    HH=MS/1000/60/60; //h
    if (HH>0)
    {
        DurationString1+=Ztring::ToZtring(HH)+MediaInfoLib::Config.Language_Get(_T("h"));
        DurationString2+=Ztring::ToZtring(HH)+MediaInfoLib::Config.Language_Get(_T("h"));
        if (HH<10)
            DurationString3+=Ztring(_T("0"))+Ztring::ToZtring(HH)+_T(":");
        else
            DurationString3+=Ztring::ToZtring(HH)+_T(":");
        MS-=HH*60*60*1000;
    }
    else
    {
        DurationString3+=_T("00:");
    }

    //Minutes
    MM=MS/1000/60; //mn
    if (MM>0 || HH>0)
    {
        if (DurationString1.size()>0)
            DurationString1+=_T(" ");
        DurationString1+=Ztring::ToZtring(MM)+MediaInfoLib::Config.Language_Get(_T("mn"));
        if (DurationString2.size()<5)
        {
            if (DurationString2.size()>0)
                DurationString2+=_T(" ");
            DurationString2+=Ztring::ToZtring(MM)+MediaInfoLib::Config.Language_Get(_T("mn"));
        }
        if (MM<10)
            DurationString3+=Ztring(_T("0"))+Ztring::ToZtring(MM)+_T(":");
        else
            DurationString3+=Ztring::ToZtring(MM)+_T(":");
        MS-=MM*60*1000;
    }
    else
    {
        DurationString3+=_T("00:");
    }

    //Seconds
    SS=MS/1000; //s
    if (SS>0 || MM>0 || HH>0)
    {
        if (DurationString1.size()>0)
            DurationString1+=_T(" ");
        DurationString1+=Ztring::ToZtring(SS)+MediaInfoLib::Config.Language_Get(_T("s"));
        if (DurationString2.size()<5)
        {
            if (DurationString2.size()>0)
                DurationString2+=_T(" ");
            DurationString2+=Ztring::ToZtring(SS)+MediaInfoLib::Config.Language_Get(_T("s"));
        }
        else if (DurationString2.size()==0)
            DurationString2+=Ztring::ToZtring(SS)+MediaInfoLib::Config.Language_Get(_T("s"));
        if (SS<10)
            DurationString3+=Ztring(_T("0"))+Ztring::ToZtring(SS)+_T(".");
        else
            DurationString3+=Ztring::ToZtring(SS)+_T(".");
        MS-=SS*1000;
    }
    else
    {
        DurationString3+=_T("00.");
    }

    //Milliseconds
    if (MS>0 || SS>0 || MM>0 || HH>0)
    {
        if (DurationString1.size()>0)
            DurationString1+=_T(" ");
        DurationString1+=Ztring::ToZtring(MS)+MediaInfoLib::Config.Language_Get(_T("ms"));
        if (DurationString2.size()<5)
        {
            if (DurationString2.size()>0)
                DurationString2+=_T(" ");
            DurationString2+=Ztring::ToZtring(MS)+MediaInfoLib::Config.Language_Get(_T("ms"));
        }
        if (MS<10)
            DurationString3+=Ztring(_T("00"))+Ztring::ToZtring(MS);
        else if (MS<100)
            DurationString3+=Ztring(_T("0"))+Ztring::ToZtring(MS);
        else
            DurationString3+=Ztring::ToZtring(MS);
    }
    else
    {
        DurationString3+=_T("000");
    }

    if (Negative)
    {
        DurationString1=Ztring(_T("-"))+DurationString1;
        DurationString2=Ztring(_T("-"))+DurationString2;
        DurationString3=Ztring(_T("-"))+DurationString3;
    }

    Fill(StreamKind, StreamPos, Parameter+1,  DurationString2, true); // /String
    Fill(StreamKind, StreamPos, Parameter+2, DurationString1, true); // /String1
    Fill(StreamKind, StreamPos, Parameter+3, DurationString2, true); // /String2
    Fill(StreamKind, StreamPos, Parameter+4, DurationString3, true); // /String3
}

//---------------------------------------------------------------------------
//FileSize
void File__Analyze::FileSize_FileSize123(stream_t StreamKind, size_t StreamPos, size_t Parameter)
{
    if (Retrieve(StreamKind, StreamPos, Parameter).empty())
        return;

    float F1=(float)Retrieve(StreamKind, StreamPos, Parameter).To_int64s(); //Video C++ 6 patch, should be int64u

    //--Bytes, KiB, MiB or GiB...
    int32u Pow3=0;
    while(F1>=1024)
    {
        F1/=1024;
        Pow3++;
    }
    //--Count of digits
    int8u I2, I3, I4;
         if (F1>=100)
    {
        I2=0;
        I3=0;
        I4=1;
    }
    else if (F1>=10)
    {
        I2=0;
        I3=1;
        I4=2;
    }
    else //if (F1>=1)
    {
        I2=1;
        I3=2;
        I4=3;
    }
    Ztring Measure; bool MeasureIsAlwaysSame;
    switch (Pow3)
    {
        case  0 : Measure=_T(" Byte"); MeasureIsAlwaysSame=false; break;
        case  1 : Measure=_T(" KiB");  MeasureIsAlwaysSame=true;  break;
        case  2 : Measure=_T(" MiB");  MeasureIsAlwaysSame=true;  break;
        case  3 : Measure=_T(" GiB");  MeasureIsAlwaysSame=true;  break;
        default : Measure=_T(" ?iB");  MeasureIsAlwaysSame=true;
    }
    Fill(StreamKind, StreamPos, Parameter+2, MediaInfoLib::Config.Language_Get(Ztring::ToZtring(F1,  0), Measure, MeasureIsAlwaysSame), true); // /String1
    Fill(StreamKind, StreamPos, Parameter+3, MediaInfoLib::Config.Language_Get(Ztring::ToZtring(F1, I2), Measure, MeasureIsAlwaysSame), true); // /String2
    Fill(StreamKind, StreamPos, Parameter+4, MediaInfoLib::Config.Language_Get(Ztring::ToZtring(F1, I3), Measure, MeasureIsAlwaysSame), true); // /String3
    Fill(StreamKind, StreamPos, Parameter+5, MediaInfoLib::Config.Language_Get(Ztring::ToZtring(F1, I4), Measure, MeasureIsAlwaysSame), true); // /String4
    float F2=(float)Retrieve(StreamKind, StreamPos, Parameter).To_int64s(); //Video C++ 6 patch, should be int64u
    if (File_Size>0 && File_Size<(int64u)-1 && Parameter==Fill_Parameter(StreamKind, Generic_StreamSize) && F2*100/File_Size<=100)
    {
        Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_StreamSize_Proportion), F2/File_Size, 5, true);
        Fill(StreamKind, StreamPos, Parameter+6, MediaInfoLib::Config.Language_Get(Ztring::ToZtring(F1, I3), Measure, MeasureIsAlwaysSame)+_T(" (")+Ztring::ToZtring(F2*100/File_Size, 0)+_T("%)"), true); // /String5
        Fill(StreamKind, StreamPos, Parameter+1,  MediaInfoLib::Config.Language_Get(Ztring::ToZtring(F1, I3), Measure, MeasureIsAlwaysSame)+_T(" (")+Ztring::ToZtring(F2*100/File_Size, 0)+_T("%)"), true);
    }
    else
        Fill(StreamKind, StreamPos, Parameter+1,  MediaInfoLib::Config.Language_Get(Ztring::ToZtring(F1, I3), Measure, MeasureIsAlwaysSame), true);
}

//---------------------------------------------------------------------------
//FileSize
void File__Analyze::Kilo_Kilo123(stream_t StreamKind, size_t StreamPos, size_t Parameter)
{
    if (Retrieve(StreamKind, StreamPos, Parameter).empty())
        return;

    //Clearing old data
    Clear(StreamKind, StreamPos, Parameter+1);

    //Retrieving multiple values
    ZtringList List;
    List.Separator_Set(0, _T(" / "));
    List.Write(Retrieve(StreamKind, StreamPos, Parameter));

    //Per value
    for (size_t Pos=0; Pos<List.size(); Pos++)
    {
        int32u BitRate=List[Pos].To_int32u();

        //Text
        if (BitRate==      0)
        {
            Fill(StreamKind, StreamPos, Parameter+1, MediaInfoLib::Config.Language_Get(List[Pos]));
        }
        else
        {
            //Well known values
            Ztring BitRateS;
            if (BitRate==  11024) BitRateS=  "11.024";
            if (BitRate==  11025) BitRateS=  "11.025";
            if (BitRate==  22050) BitRateS=  "22.05";
            if (BitRate==  44100) BitRateS=  "44.1";
            if (BitRate==  66150) BitRateS=  "66.15";
            if (BitRate==  88200) BitRateS=  "88.2";
            if (BitRate== 132300) BitRateS= "132.3";
            if (BitRate== 176400) BitRateS= "176.4";
            if (BitRate== 264600) BitRateS= "264.6";
            if (BitRate== 352800) BitRateS= "352.8";
            if (BitRate== 529200) BitRateS= "529.2";
            if (BitRate== 705600) BitRateS= "705.6";
            if (BitRate==1411200) BitRateS="1411.2";
            if (!BitRateS.empty())
            {
                Ztring Measure=MediaInfoLib::Config.Info_Get(StreamKind).Read(Parameter, Info_Measure);
                Measure.insert(1, _T("K"));
                Fill(StreamKind, StreamPos, Parameter+1, MediaInfoLib::Config.Language_Get(BitRateS, Measure, true));
            }
            else
            {
                //Standard
                if (BitRate>10000000)
                {
                    Ztring Measure=MediaInfoLib::Config.Info_Get(StreamKind).Read(Parameter, Info_Measure);
                    Measure.insert(1, _T("M"));
                    Fill(StreamKind, StreamPos, Parameter+1, MediaInfoLib::Config.Language_Get(Ztring::ToZtring(((float)BitRate)/1000000, BitRate>100000000?0:1), Measure, true));
                }
                else if (BitRate>10000)
                {
                    Ztring Measure=MediaInfoLib::Config.Info_Get(StreamKind).Read(Parameter, Info_Measure);
                    Measure.insert(1, _T("K"));
                    Fill(StreamKind, StreamPos, Parameter+1, MediaInfoLib::Config.Language_Get(Ztring::ToZtring(((float)BitRate)/1000, BitRate>100000?0:1), Measure, true));
                }
                else if (BitRate>0)
                    Fill(StreamKind, StreamPos, Parameter+1, MediaInfoLib::Config.Language_Get(Ztring::ToZtring(BitRate), MediaInfoLib::Config.Info_Get(StreamKind).Read(Parameter, Info_Measure), true));
            }
        }
    }
}

//---------------------------------------------------------------------------
//Value --> Value with measure
void File__Analyze::Value_Value123(stream_t StreamKind, size_t StreamPos, size_t Parameter)
{
    if (Retrieve(StreamKind, StreamPos, Parameter, Info_Measure).empty())
        return;

    //Clearing old data
    Clear(StreamKind, StreamPos, Parameter+1);

    //Retrieving multiple values
    ZtringList List;
    List.Separator_Set(0, _T(" / "));
    List.Write(Retrieve(StreamKind, StreamPos, Parameter));

    //Per value
    for (size_t Pos=0; Pos<List.size(); Pos++)
    {
        //Filling
        Fill(StreamKind, StreamPos, Parameter+1, MediaInfoLib::Config.Language_Get(List[Pos], MediaInfoLib::Config.Info_Get(StreamKind).Read(Parameter, Info_Measure)));
    }

}

//---------------------------------------------------------------------------
//Value --> Yes or No
void File__Analyze::YesNo_YesNo(stream_t StreamKind, size_t StreamPos, size_t Parameter)
{
    //Filling
    Fill(StreamKind, StreamPos, Parameter+1, MediaInfoLib::Config.Language_Get(Retrieve(StreamKind, StreamPos, Parameter)), true);
}

//---------------------------------------------------------------------------
void File__Analyze::CodecID_Fill(const Ztring &Value, stream_t StreamKind, size_t StreamPos, infocodecid_format_t Format)
{
    Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_CodecID), Value, true);
    const Ztring &C1=MediaInfoLib::Config.CodecID_Get(StreamKind, Format, Value, InfoCodecID_Format);
    Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format), C1.empty()?Value:C1, true);
    Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_CodecID_Info), MediaInfoLib::Config.CodecID_Get(StreamKind, Format, Value, InfoCodecID_Description), true);
    Fill(StreamKind, StreamPos, "CodecID/Hint", MediaInfoLib::Config.CodecID_Get(StreamKind, Format, Value, InfoCodecID_Hint), true);
    Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_CodecID_Url), MediaInfoLib::Config.CodecID_Get(StreamKind, Format, Value, InfoCodecID_Url), true);
    Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format_Version), MediaInfoLib::Config.CodecID_Get(StreamKind, Format, Value, InfoCodecID_Version), true);
    Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format_Profile), MediaInfoLib::Config.CodecID_Get(StreamKind, Format, Value, InfoCodecID_Profile), true);
    Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_ColorSpace), MediaInfoLib::Config.CodecID_Get(StreamKind, Format, Value, InfoCodecID_ColorSpace), true);
    Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_ChromaSubsampling), MediaInfoLib::Config.CodecID_Get(StreamKind, Format, Value, InfoCodecID_ChromaSubsampling), true);
    if (Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_BitDepth)).empty())
        Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_BitDepth), MediaInfoLib::Config.CodecID_Get(StreamKind, Format, Value, InfoCodecID_BitDepth), true);

    //Specific cases
    if (Value==_T("v210") || Value==_T("V210"))
        Fill(Stream_Video, StreamPos, Video_Resolution, 10);
}

//---------------------------------------------------------------------------
size_t File__Analyze::Fill_Parameter(stream_t StreamKind, generic StreamPos)
{
    switch (StreamKind)
    {
        case Stream_General :
                                switch (StreamPos)
                                {
                                    case Generic_Format : return General_Format;
                                    case Generic_Format_Info : return General_Format_Info;
                                    case Generic_Format_Url : return General_Format_Url;
                                    case Generic_Format_Version : return General_Format_Version;
                                    case Generic_Format_Commercial : return General_Format_Commercial;
                                    case Generic_Format_Commercial_IfAny : return General_Format_Commercial_IfAny;
                                    case Generic_Format_Profile : return General_Format_Profile;
                                    case Generic_Format_Settings : return General_Format_Settings;
                                    case Generic_InternetMediaType : return General_InternetMediaType;
                                    case Generic_CodecID : return General_CodecID;
                                    case Generic_CodecID_Info : return General_CodecID_Info;
                                    case Generic_CodecID_Hint : return General_CodecID_Hint;
                                    case Generic_CodecID_Url : return General_CodecID_Url;
                                    case Generic_CodecID_Description : return General_CodecID_Description;
                                    case Generic_Codec : return General_Codec;
                                    case Generic_Codec_String : return General_Codec_String;
                                    case Generic_Codec_Info : return General_Codec_Info;
                                    case Generic_Codec_Url : return General_Codec_Url;
                                    case Generic_Duration : return General_Duration;
                                    case Generic_Duration_String : return General_Duration_String;
                                    case Generic_Duration_String1 : return General_Duration_String1;
                                    case Generic_Duration_String2 : return General_Duration_String2;
                                    case Generic_Duration_String3 : return General_Duration_String3;
                                    case Generic_StreamSize : return General_StreamSize;
                                    case Generic_StreamSize_String : return General_StreamSize_String;
                                    case Generic_StreamSize_String1 : return General_StreamSize_String1;
                                    case Generic_StreamSize_String2 : return General_StreamSize_String2;
                                    case Generic_StreamSize_String3 : return General_StreamSize_String3;
                                    case Generic_StreamSize_String4 : return General_StreamSize_String4;
                                    case Generic_StreamSize_String5 : return General_StreamSize_String5;
                                    case Generic_StreamSize_Proportion : return General_StreamSize_Proportion;
                                    default: return (size_t)-1;
                                }
        case Stream_Video :
                                switch (StreamPos)
                                {
                                    case Generic_Format : return Video_Format;
                                    case Generic_Format_Info : return Video_Format_Info;
                                    case Generic_Format_Url : return Video_Format_Url;
                                    case Generic_Format_Commercial : return Video_Format_Commercial;
                                    case Generic_Format_Commercial_IfAny : return Video_Format_Commercial_IfAny;
                                    case Generic_Format_Version : return Video_Format_Version;
                                    case Generic_Format_Profile : return Video_Format_Profile;
                                    case Generic_Format_Settings : return Video_Format_Settings;
                                    case Generic_InternetMediaType : return Video_InternetMediaType;
                                    case Generic_CodecID : return Video_CodecID;
                                    case Generic_CodecID_Info : return Video_CodecID_Info;
                                    case Generic_CodecID_Hint : return Video_CodecID_Hint;
                                    case Generic_CodecID_Url : return Video_CodecID_Url;
                                    case Generic_CodecID_Description : return Video_CodecID_Description;
                                    case Generic_Codec : return Video_Codec;
                                    case Generic_Codec_String : return Video_Codec_String;
                                    case Generic_Codec_Info : return Video_Codec_Info;
                                    case Generic_Codec_Url : return Video_Codec_Url;
                                    case Generic_Codec_CC : return Video_Codec_CC;
                                    case Generic_Duration : return Video_Duration;
                                    case Generic_Duration_String : return Video_Duration_String;
                                    case Generic_Duration_String1 : return Video_Duration_String1;
                                    case Generic_Duration_String2 : return Video_Duration_String2;
                                    case Generic_Duration_String3 : return Video_Duration_String3;
                                    case Generic_BitRate_Mode : return Video_BitRate_Mode;
                                    case Generic_BitRate_Mode_String : return Video_BitRate_Mode_String;
                                    case Generic_BitRate : return Video_BitRate;
                                    case Generic_BitRate_String : return Video_BitRate_String;
                                    case Generic_BitRate_Minimum : return Video_BitRate_Minimum;
                                    case Generic_BitRate_Minimum_String : return Video_BitRate_Minimum_String;
                                    case Generic_BitRate_Nominal : return Video_BitRate_Nominal;
                                    case Generic_BitRate_Nominal_String : return Video_BitRate_Nominal_String;
                                    case Generic_BitRate_Maximum : return Video_BitRate_Maximum;
                                    case Generic_BitRate_Maximum_String : return Video_BitRate_Maximum_String;
                                    case Generic_ColorSpace : return Video_ColorSpace;
                                    case Generic_ChromaSubsampling : return Video_ChromaSubsampling;
                                    case Generic_Resolution : return Video_Resolution;
                                    case Generic_Resolution_String : return Video_Resolution_String;
                                    case Generic_BitDepth : return Video_BitDepth;
                                    case Generic_BitDepth_String : return Video_BitDepth_String;
                                    case Generic_Delay : return Video_Delay;
                                    case Generic_Delay_String : return Video_Delay_String;
                                    case Generic_Delay_String1 : return Video_Delay_String1;
                                    case Generic_Delay_String2 : return Video_Delay_String2;
                                    case Generic_Delay_String3 : return Video_Delay_String3;
                                    case Generic_Delay_String4 : return Video_Delay_String4;
                                    case Generic_Delay_Settings : return Video_Delay_Settings;
                                    case Generic_Delay_Source : return Video_Delay_Source;
                                    case Generic_Delay_Original : return Video_Delay_Original;
                                    case Generic_Delay_Original_String : return Video_Delay_Original_String;
                                    case Generic_Delay_Original_String1 : return Video_Delay_Original_String1;
                                    case Generic_Delay_Original_String2 : return Video_Delay_Original_String2;
                                    case Generic_Delay_Original_String3 : return Video_Delay_Original_String3;
                                    case Generic_Delay_Original_String4 : return Video_Delay_Original_String4;
                                    case Generic_Delay_Original_Settings : return Video_Delay_Original_Settings;
                                    case Generic_Delay_Original_Source : return Video_Delay_Original_Source;
                                    case Generic_StreamSize : return Video_StreamSize;
                                    case Generic_StreamSize_String : return Video_StreamSize_String;
                                    case Generic_StreamSize_String1 : return Video_StreamSize_String1;
                                    case Generic_StreamSize_String2 : return Video_StreamSize_String2;
                                    case Generic_StreamSize_String3 : return Video_StreamSize_String3;
                                    case Generic_StreamSize_String4 : return Video_StreamSize_String4;
                                    case Generic_StreamSize_String5 : return Video_StreamSize_String5;
                                    case Generic_StreamSize_Proportion : return Video_StreamSize_Proportion;
                                    default: return (size_t)-1;
                                }
        case Stream_Audio :
                                switch (StreamPos)
                                {
                                    case Generic_Format : return Audio_Format;
                                    case Generic_Format_Info : return Audio_Format_Info;
                                    case Generic_Format_Url : return Audio_Format_Url;
                                    case Generic_Format_Commercial : return Audio_Format_Commercial;
                                    case Generic_Format_Commercial_IfAny : return Audio_Format_Commercial_IfAny;
                                    case Generic_Format_Version : return Audio_Format_Version;
                                    case Generic_Format_Profile : return Audio_Format_Profile;
                                    case Generic_Format_Settings : return Audio_Format_Settings;
                                    case Generic_InternetMediaType : return Audio_InternetMediaType;
                                    case Generic_CodecID : return Audio_CodecID;
                                    case Generic_CodecID_Info : return Audio_CodecID_Info;
                                    case Generic_CodecID_Hint : return Audio_CodecID_Hint;
                                    case Generic_CodecID_Url : return Audio_CodecID_Url;
                                    case Generic_CodecID_Description : return Audio_CodecID_Description;
                                    case Generic_Codec : return Audio_Codec;
                                    case Generic_Codec_String : return Audio_Codec_String;
                                    case Generic_Codec_Info : return Audio_Codec_Info;
                                    case Generic_Codec_Url : return Audio_Codec_Url;
                                    case Generic_Codec_CC : return Audio_Codec_CC;
                                    case Generic_Duration : return Audio_Duration;
                                    case Generic_Duration_String : return Audio_Duration_String;
                                    case Generic_Duration_String1 : return Audio_Duration_String1;
                                    case Generic_Duration_String2 : return Audio_Duration_String2;
                                    case Generic_Duration_String3 : return Audio_Duration_String3;
                                    case Generic_BitRate_Mode : return Audio_BitRate_Mode;
                                    case Generic_BitRate_Mode_String : return Audio_BitRate_Mode_String;
                                    case Generic_BitRate : return Audio_BitRate;
                                    case Generic_BitRate_String : return Audio_BitRate_String;
                                    case Generic_BitRate_Minimum : return Audio_BitRate_Minimum;
                                    case Generic_BitRate_Minimum_String : return Audio_BitRate_Minimum_String;
                                    case Generic_BitRate_Nominal : return Audio_BitRate_Nominal;
                                    case Generic_BitRate_Nominal_String : return Audio_BitRate_Nominal_String;
                                    case Generic_BitRate_Maximum : return Audio_BitRate_Maximum;
                                    case Generic_BitRate_Maximum_String : return Audio_BitRate_Maximum_String;
                                    case Generic_Resolution : return Audio_Resolution;
                                    case Generic_Resolution_String : return Audio_Resolution_String;
                                    case Generic_BitDepth : return Audio_BitDepth;
                                    case Generic_BitDepth_String : return Audio_BitDepth_String;
                                    case Generic_Delay : return Audio_Delay;
                                    case Generic_Delay_String : return Audio_Delay_String;
                                    case Generic_Delay_String1 : return Audio_Delay_String1;
                                    case Generic_Delay_String2 : return Audio_Delay_String2;
                                    case Generic_Delay_String3 : return Audio_Delay_String3;
                                    case Generic_Delay_String4 : return Audio_Delay_String4;
                                    case Generic_Delay_Settings : return Audio_Delay_Settings;
                                    case Generic_Delay_Source : return Audio_Delay_Source;
                                    case Generic_Delay_Original : return Audio_Delay_Original;
                                    case Generic_Delay_Original_String : return Audio_Delay_Original_String;
                                    case Generic_Delay_Original_String1 : return Audio_Delay_Original_String1;
                                    case Generic_Delay_Original_String2 : return Audio_Delay_Original_String2;
                                    case Generic_Delay_Original_String3 : return Audio_Delay_Original_String3;
                                    case Generic_Delay_Original_String4 : return Audio_Delay_Original_String4;
                                    case Generic_Delay_Original_Settings : return Audio_Delay_Original_Settings;
                                    case Generic_Delay_Original_Source : return Audio_Delay_Original_Source;
                                    case Generic_Video_Delay : return Audio_Video_Delay;
                                    case Generic_Video_Delay_String : return Audio_Video_Delay_String;
                                    case Generic_Video_Delay_String1 : return Audio_Video_Delay_String1;
                                    case Generic_Video_Delay_String2 : return Audio_Video_Delay_String2;
                                    case Generic_Video_Delay_String3 : return Audio_Video_Delay_String3;
                                    case Generic_Video_Delay_String4 : return Audio_Video_Delay_String4;                                    case Generic_StreamSize : return Audio_StreamSize;
                                    case Generic_StreamSize_String : return Audio_StreamSize_String;
                                    case Generic_StreamSize_String1 : return Audio_StreamSize_String1;
                                    case Generic_StreamSize_String2 : return Audio_StreamSize_String2;
                                    case Generic_StreamSize_String3 : return Audio_StreamSize_String3;
                                    case Generic_StreamSize_String4 : return Audio_StreamSize_String4;
                                    case Generic_StreamSize_String5 : return Audio_StreamSize_String5;
                                    case Generic_StreamSize_Proportion : return Audio_StreamSize_Proportion;
                                    default: return (size_t)-1;
                                }
        case Stream_Text :
                                switch (StreamPos)
                                {
                                    case Generic_Format : return Text_Format;
                                    case Generic_Format_Info : return Text_Format_Info;
                                    case Generic_Format_Url : return Text_Format_Url;
                                    case Generic_Format_Commercial : return Text_Format_Commercial;
                                    case Generic_Format_Commercial_IfAny : return Text_Format_Commercial_IfAny;
                                    case Generic_Format_Version : return Text_Format_Version;
                                    case Generic_Format_Profile : return Text_Format_Profile;
                                    case Generic_Format_Settings : return Text_Format_Settings;
                                    case Generic_InternetMediaType : return Text_InternetMediaType;
                                    case Generic_CodecID : return Text_CodecID;
                                    case Generic_CodecID_Info : return Text_CodecID_Info;
                                    case Generic_CodecID_Hint : return Text_CodecID_Hint;
                                    case Generic_CodecID_Url : return Text_CodecID_Url;
                                    case Generic_CodecID_Description : return Text_CodecID_Description;
                                    case Generic_Codec : return Text_Codec;
                                    case Generic_Codec_String : return Text_Codec_String;
                                    case Generic_Codec_Info : return Text_Codec_Info;
                                    case Generic_Codec_Url : return Text_Codec_Url;
                                    case Generic_Codec_CC : return Text_Codec_CC;
                                    case Generic_Duration : return Text_Duration;
                                    case Generic_Duration_String : return Text_Duration_String;
                                    case Generic_Duration_String1 : return Text_Duration_String1;
                                    case Generic_Duration_String2 : return Text_Duration_String2;
                                    case Generic_Duration_String3 : return Text_Duration_String3;
                                    case Generic_BitRate_Mode : return Text_BitRate_Mode;
                                    case Generic_BitRate_Mode_String : return Text_BitRate_Mode_String;
                                    case Generic_BitRate : return Text_BitRate;
                                    case Generic_BitRate_String : return Text_BitRate_String;
                                    case Generic_BitRate_Minimum : return Text_BitRate_Minimum;
                                    case Generic_BitRate_Minimum_String : return Text_BitRate_Minimum_String;
                                    case Generic_BitRate_Nominal : return Text_BitRate_Nominal;
                                    case Generic_BitRate_Nominal_String : return Text_BitRate_Nominal_String;
                                    case Generic_BitRate_Maximum : return Text_BitRate_Maximum;
                                    case Generic_BitRate_Maximum_String : return Text_BitRate_Maximum_String;
                                    case Generic_ColorSpace : return Text_ColorSpace;
                                    case Generic_ChromaSubsampling : return Text_ChromaSubsampling;
                                    case Generic_Resolution : return Text_Resolution;
                                    case Generic_Resolution_String : return Text_Resolution_String;
                                    case Generic_BitDepth : return Text_BitDepth;
                                    case Generic_BitDepth_String : return Text_BitDepth_String;
                                    case Generic_Delay : return Text_Delay;
                                    case Generic_Delay_String : return Text_Delay_String;
                                    case Generic_Delay_String1 : return Text_Delay_String1;
                                    case Generic_Delay_String2 : return Text_Delay_String2;
                                    case Generic_Delay_String3 : return Text_Delay_String3;
                                    case Generic_Delay_String4 : return Text_Delay_String4;
                                    case Generic_Delay_Settings : return Text_Delay_Settings;
                                    case Generic_Delay_Source : return Text_Delay_Source;
                                    case Generic_Delay_Original : return Text_Delay_Original;
                                    case Generic_Delay_Original_String : return Text_Delay_Original_String;
                                    case Generic_Delay_Original_String1 : return Text_Delay_Original_String1;
                                    case Generic_Delay_Original_String2 : return Text_Delay_Original_String2;
                                    case Generic_Delay_Original_String3 : return Text_Delay_Original_String3;
                                    case Generic_Delay_Original_String4 : return Text_Delay_Original_String4;
                                    case Generic_Delay_Original_Settings : return Text_Delay_Original_Settings;
                                    case Generic_Delay_Original_Source : return Text_Delay_Original_Source;
                                    case Generic_Video_Delay : return Text_Video_Delay;
                                    case Generic_Video_Delay_String : return Text_Video_Delay_String;
                                    case Generic_Video_Delay_String1 : return Text_Video_Delay_String1;
                                    case Generic_Video_Delay_String2 : return Text_Video_Delay_String2;
                                    case Generic_Video_Delay_String3 : return Text_Video_Delay_String3;
                                    case Generic_Video_Delay_String4 : return Text_Video_Delay_String4;                                     case Generic_StreamSize : return Text_StreamSize;
                                    case Generic_StreamSize_String : return Text_StreamSize_String;
                                    case Generic_StreamSize_String1 : return Text_StreamSize_String1;
                                    case Generic_StreamSize_String2 : return Text_StreamSize_String2;
                                    case Generic_StreamSize_String3 : return Text_StreamSize_String3;
                                    case Generic_StreamSize_String4 : return Text_StreamSize_String4;
                                    case Generic_StreamSize_String5 : return Text_StreamSize_String5;
                                    case Generic_StreamSize_Proportion : return Text_StreamSize_Proportion;
                                    default: return (size_t)-1;
                                }
        case Stream_Image :
                                switch (StreamPos)
                                {
                                    case Generic_Format : return Image_Format;
                                    case Generic_Format_Info : return Image_Format_Info;
                                    case Generic_Format_Url : return Image_Format_Url;
                                    case Generic_Format_Commercial : return Image_Format_Commercial;
                                    case Generic_Format_Commercial_IfAny : return Image_Format_Commercial_IfAny;
                                    case Generic_Format_Version : return Image_Format_Version;
                                    case Generic_Format_Profile : return Image_Format_Profile;
                                    case Generic_InternetMediaType : return Image_InternetMediaType;
                                    case Generic_CodecID : return Image_CodecID;
                                    case Generic_CodecID_Info : return Image_CodecID_Info;
                                    case Generic_CodecID_Hint : return Image_CodecID_Hint;
                                    case Generic_CodecID_Url : return Image_CodecID_Url;
                                    case Generic_CodecID_Description : return Image_CodecID_Description;
                                    case Generic_Codec : return Image_Codec;
                                    case Generic_Codec_String : return Image_Codec_String;
                                    case Generic_Codec_Info : return Image_Codec_Info;
                                    case Generic_Codec_Url : return Image_Codec_Url;
                                    case Generic_ColorSpace : return Image_ColorSpace;
                                    case Generic_ChromaSubsampling : return Image_ChromaSubsampling;
                                    case Generic_Resolution : return Image_Resolution;
                                    case Generic_Resolution_String : return Image_Resolution_String;
                                    case Generic_BitDepth : return Image_BitDepth;
                                    case Generic_BitDepth_String : return Image_BitDepth_String;
                                    case Generic_StreamSize : return Image_StreamSize;
                                    case Generic_StreamSize_String : return Image_StreamSize_String;
                                    case Generic_StreamSize_String1 : return Image_StreamSize_String1;
                                    case Generic_StreamSize_String2 : return Image_StreamSize_String2;
                                    case Generic_StreamSize_String3 : return Image_StreamSize_String3;
                                    case Generic_StreamSize_String4 : return Image_StreamSize_String4;
                                    case Generic_StreamSize_String5 : return Image_StreamSize_String5;
                                    case Generic_StreamSize_Proportion : return Image_StreamSize_Proportion;
                                    default: return (size_t)-1;
                                }
        case Stream_Chapters : return (size_t)-1;
        case Stream_Menu :
                                switch (StreamPos)
                                {
                                    case Generic_Format : return Menu_Format;
                                    case Generic_Format_Info : return Menu_Format_Info;
                                    case Generic_Format_Url : return Menu_Format_Url;
                                    case Generic_Format_Commercial : return Menu_Format_Commercial;
                                    case Generic_Format_Commercial_IfAny : return Menu_Format_Commercial_IfAny;
                                    case Generic_Format_Version : return Menu_Format_Version;
                                    case Generic_Format_Profile : return Menu_Format_Profile;
                                    case Generic_Format_Settings : return Menu_Format_Settings;
                                    case Generic_CodecID : return Menu_CodecID;
                                    case Generic_CodecID_Info : return Menu_CodecID_Info;
                                    case Generic_CodecID_Hint : return Menu_CodecID_Hint;
                                    case Generic_CodecID_Url : return Menu_CodecID_Url;
                                    case Generic_CodecID_Description : return Menu_CodecID_Description;
                                    case Generic_Codec : return Menu_Codec;
                                    case Generic_Codec_String : return Menu_Codec_String;
                                    case Generic_Codec_Info : return Menu_Codec_Info;
                                    case Generic_Codec_Url : return Menu_Codec_Url;
                                    case Generic_Duration : return Menu_Duration;
                                    case Generic_Duration_String : return Menu_Duration_String;
                                    case Generic_Duration_String1 : return Menu_Duration_String1;
                                    case Generic_Duration_String2 : return Menu_Duration_String2;
                                    case Generic_Duration_String3 : return Menu_Duration_String3;
                                    default: return (size_t)-1;
                                }
        default: return (size_t)-1;
    }
}

} //NameSpace

