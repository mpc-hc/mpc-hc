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
#include "MediaInfo/MediaInfo_Internal.h"
#include "MediaInfo/MediaInfo_Config.h"
#include "ZenLib/File.h"
#include "ZenLib/FileName.h"
#include "ZenLib/BitStream_LE.h"
#include <cmath>
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
size_t File__Analyze::Stream_Prepare (stream_t KindOfStream, size_t StreamPos)
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

    if (StreamPos>=Count_Get(KindOfStream))
    {
        //Add a stream
        (*Stream)[KindOfStream].resize((*Stream)[KindOfStream].size()+1);
        (*Stream_More)[KindOfStream].resize((*Stream_More)[KindOfStream].size()+1);
        StreamKind_Last=KindOfStream;
        StreamPos_Last=(*Stream)[KindOfStream].size()-1;
    }
    else
    {
        //Insert a stream
        (*Stream)[KindOfStream].insert((*Stream)[KindOfStream].begin()+StreamPos, ZtringList());
        (*Stream_More)[KindOfStream].insert((*Stream_More)[KindOfStream].begin()+StreamPos, ZtringListList());
        StreamKind_Last=KindOfStream;
        StreamPos_Last=StreamPos;
    }

    //Filling basic info
    Fill(StreamKind_Last, StreamPos_Last, (size_t)General_Count, Count_Get(StreamKind_Last, StreamPos_Last));
    Fill(StreamKind_Last, StreamPos_Last, General_StreamKind, MediaInfoLib::Config.Info_Get(StreamKind_Last).Read(General_StreamKind, Info_Text));
    Fill(StreamKind_Last, StreamPos_Last, General_StreamKind_String, MediaInfoLib::Config.Language_Get(MediaInfoLib::Config.Info_Get(StreamKind_Last).Read(General_StreamKind, Info_Text)), true);
    for (size_t Pos=0; Pos<Count_Get(KindOfStream); Pos++)
    {
        Fill(StreamKind_Last, Pos, General_StreamCount, Count_Get(StreamKind_Last), 10, true);
        Fill(StreamKind_Last, Pos, General_StreamKindID, Pos, 10, true);
        if (Count_Get(StreamKind_Last)>1)
            Fill(StreamKind_Last, Pos, General_StreamKindPos, Pos+1, 10, true);
    }

    //Filling Lists & Counts
    if (!IsSub && KindOfStream!=Stream_General)
    {
        const Ztring& StreamKind_Text=Get(KindOfStream, 0, General_StreamKind, Info_Text);
        if (Count_Get(KindOfStream)>1)
        {
            ZtringList Temp; Temp.Separator_Set(0, __T(" / "));
            Temp.Write(Retrieve(Stream_General, 0, Ztring(StreamKind_Text+__T("_Codec_List")).To_Local().c_str()));
            if (StreamPos<Temp.size())
                Temp.insert(Temp.begin()+StreamPos, Ztring());
            else
                Temp.push_back(Ztring());
            Fill(Stream_General, 0, Ztring(StreamKind_Text+__T("_Codec_List")).To_Local().c_str(), Temp.Read(), true);
            Temp.Write(Retrieve(Stream_General, 0, Ztring(StreamKind_Text+__T("_Language_List")).To_Local().c_str()));
            if (StreamPos<Temp.size())
                Temp.insert(Temp.begin()+StreamPos, Ztring());
            else
                Temp.push_back(Ztring());
            Fill(Stream_General, 0, Ztring(StreamKind_Text+__T("_Language_List")).To_Local().c_str(), Temp.Read(), true);
            Temp.Write(Retrieve(Stream_General, 0, Ztring(StreamKind_Text+__T("_Format_List")).To_Local().c_str()));
            if (StreamPos<Temp.size())
                Temp.insert(Temp.begin()+StreamPos, Ztring());
            else
                Temp.push_back(Ztring());
            Fill(Stream_General, 0, Ztring(StreamKind_Text+__T("_Format_List")).To_Local().c_str(), Temp.Read(), true);
            Temp.Write(Retrieve(Stream_General, 0, Ztring(StreamKind_Text+__T("_Format_WithHint_List")).To_Local().c_str()));
            if (StreamPos<Temp.size())
                Temp.insert(Temp.begin()+StreamPos, Ztring());
            else
                Temp.push_back(Ztring());
            Fill(Stream_General, 0, Ztring(StreamKind_Text+__T("_Format_WithHint_List")).To_Local().c_str(), Temp.Read(), true);
        }

        Fill(Stream_General, 0, Ztring(StreamKind_Text+__T("Count")).To_Local().c_str(), Count_Get(KindOfStream), 10, true);
    }

    //File name and dates
    if (!IsSub && KindOfStream==Stream_General && File_Name.size()>0)
    {
        //File name
        if (File_Name.find(__T("://"))==string::npos)
        {
            Fill (Stream_General, 0, General_CompleteName, File_Name);
            Fill (Stream_General, 0, General_FolderName, FileName::Path_Get(File_Name));
            Fill (Stream_General, 0, General_FileName, FileName::Name_Get(File_Name));
            Fill (Stream_General, 0, General_FileExtension, FileName::Extension_Get(File_Name));
        }
        else
        {
            Ztring FileName_Modified=File_Name;
            size_t Begin=FileName_Modified.find(__T(':'), 6);
            size_t End=FileName_Modified.find(__T('@'));
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
    if (((!IsSub || !File_Name.empty()) && KindOfStream==Stream_General && File_Size!=(int64u)-1)
        #if MEDIAINFO_ADVANCED
            && !Config->File_IgnoreSequenceFileSize_Get()
        #endif //MEDIAINFO_ADVANCED
                )
        Fill (Stream_General, 0, General_FileSize, File_Size);

    //Fill with already ready data
    for (size_t Pos=0; Pos<Fill_Temp.size(); Pos++)
        if (Fill_Temp(Pos, 0).IsNumber())
            Fill(StreamKind_Last, StreamPos_Last, Fill_Temp(Pos, 0).To_int32u(), Fill_Temp(Pos, 1));
        else
            Fill(StreamKind_Last, StreamPos_Last, Fill_Temp(Pos, 0).To_UTF8().c_str(), Fill_Temp(Pos, 1));
    Fill_Temp.clear();

    return StreamPos_Last; //The position in the stream count
}

size_t File__Analyze::Stream_Erase (stream_t KindOfStream, size_t StreamPos)
{
    //Integrity
    if (!Status[IsAccepted] || KindOfStream>Stream_Max || StreamPos>=Count_Get(KindOfStream))
        return Error;

    //Insert a stream
    (*Stream)[KindOfStream].erase((*Stream)[KindOfStream].begin()+StreamPos);
    (*Stream_More)[KindOfStream].erase((*Stream_More)[KindOfStream].begin()+StreamPos);

    //Filling basic info
    for (size_t Pos=0; Pos<Count_Get(KindOfStream); Pos++)
    {
        Fill(KindOfStream, Pos, General_StreamCount, Count_Get(StreamKind_Last), 10, true);
        Fill(KindOfStream, Pos, General_StreamKindID, Pos, 10, true);
        if (Count_Get(KindOfStream)>1)
            Fill(KindOfStream, Pos, General_StreamKindPos, Pos+1, 10, true);
    }

    StreamKind_Last=Stream_Max;
    StreamPos_Last=(size_t)-1;

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
    if (Value.find(__T('\r'))!=string::npos || Value.find(__T('\n'))!=string::npos)
    {
        Ztring NewValue=Value;
        NewValue.FindAndReplace(__T("\r\n"), __T(" / "), 0, Ztring_Recursive);
        NewValue.FindAndReplace(__T("\r"), __T(" / "), 0, Ztring_Recursive);
        NewValue.FindAndReplace(__T("\n"), __T(" / "), 0, Ztring_Recursive);
        if (NewValue.size()>=3 && NewValue.rfind(__T(" / "))==NewValue.size()-3)
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

    if (!Replace && Value.empty())
        return;
    if (Replace && Value.empty())
    {
        Clear(StreamKind, StreamPos, Parameter);
        return;
    }

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
    if (Parameter==Fill_Parameter(StreamKind, Generic_BitDepth))
        Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Resolution), Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_BitDepth)), true);
    if (StreamKind==Stream_Video && Parameter==Video_Colorimetry)
        Fill(Stream_Video, StreamPos, Video_ChromaSubsampling, Value, Replace);

    if (StreamKind==Stream_Video && Parameter==Video_DisplayAspectRatio && !Value.empty() && Retrieve(Stream_Video, StreamPos, Video_PixelAspectRatio).empty())
    {
        float DAR   =Retrieve(Stream_Video, StreamPos, Video_DisplayAspectRatio).To_float32();
        float Width =Retrieve(Stream_Video, StreamPos, Video_Width             ).To_float32();
        float Height=Retrieve(Stream_Video, StreamPos, Video_Height            ).To_float32();
        if (DAR && Height && Width)
        {
            if (Retrieve(Stream_Video, StreamPos, Video_DisplayAspectRatio)==__T("1.778"))
                DAR=((float)16)/9; //More exact value
            if (Retrieve(Stream_Video, StreamPos, Video_DisplayAspectRatio)==__T("1.333"))
                DAR=((float)4)/3; //More exact value
            Fill(Stream_Video, StreamPos, Video_PixelAspectRatio, DAR/(((float32)Width)/Height));
        }
    }

    if (StreamKind==Stream_Video && Parameter==Video_PixelAspectRatio && !Value.empty() && Retrieve(Stream_Video, StreamPos, Video_DisplayAspectRatio).empty())
    {
        float32 PAR   =Retrieve(Stream_Video, StreamPos, Video_PixelAspectRatio  ).To_float32();
        if (PAR>(float32)12/(float32)11*0.999 && PAR<(float32)12/(float32)11*1.001)
            PAR=(float32)12/(float32)11;
        if (PAR>(float32)10/(float32)11*0.999 && PAR<(float32)10/(float32)11*1.001)
            PAR=(float32)10/(float32)11;
        if (PAR>(float32)16/(float32)11*0.999 && PAR<(float32)16/(float32)11*1.001)
            PAR=(float32)16/(float32)11;
        if (PAR>(float32)40/(float32)33*0.999 && PAR<(float32)40/(float32)33*1.001)
            PAR=(float32)40/(float32)33;
        if (PAR>(float32)24/(float32)11*0.999 && PAR<(float32)24/(float32)11*1.001)
            PAR=(float32)24/(float32)11;
        if (PAR>(float32)20/(float32)11*0.999 && PAR<(float32)20/(float32)11*1.001)
            PAR=(float32)20/(float32)11;
        if (PAR>(float32)32/(float32)11*0.999 && PAR<(float32)32/(float32)11*1.001)
            PAR=(float32)32/(float32)11;
        if (PAR>(float32)80/(float32)33*0.999 && PAR<(float32)80/(float32)33*1.001)
            PAR=(float32)80/(float32)33;
        if (PAR>(float32)18/(float32)11*0.999 && PAR<(float32)18/(float32)11*1.001)
            PAR=(float32)18/(float32)11;
        if (PAR>(float32)15/(float32)11*0.999 && PAR<(float32)15/(float32)11*1.001)
            PAR=(float32)15/(float32)11;
        if (PAR>(float32)64/(float32)33*0.999 && PAR<(float32)64/(float32)33*1.001)
            PAR=(float32)64/(float32)33;
        if (PAR>(float32)160/(float32)99*0.999 && PAR<(float32)160/(float32)99*1.001)
            PAR=(float32)160/(float32)99;
        if (PAR>(float32)4/(float32)3*0.999 && PAR<(float32)4/(float32)3*1.01)
            PAR=(float32)4/(float32)3;
        if (PAR>(float32)3/(float32)2*0.999 && PAR<(float32)3/(float32)2*1.001)
            PAR=(float32)3/(float32)2;
        if (PAR>(float32)2/(float32)1*0.999 && PAR<(float32)2/(float32)1*1.001)
            PAR=(float32)2;
        if (PAR>(float32)59/(float32)54*0.999 && PAR<(float32)59/(float32)54*1.001)
            PAR=(float32)59/(float32)54;
        float32 Width =Retrieve(Stream_Video, StreamPos, Video_Width             ).To_float32();
        float32 Height=Retrieve(Stream_Video, StreamPos, Video_Height            ).To_float32();
        if (PAR && Height && Width)
            Fill(Stream_Video, StreamPos, Video_DisplayAspectRatio, ((float32)Width)/Height*PAR);
    }

    if (StreamKind==Stream_Image && Parameter==Image_DisplayAspectRatio && !Value.empty() && Retrieve(Stream_Image, StreamPos, Image_PixelAspectRatio).empty())
    {
        float DAR   =Retrieve(Stream_Image, StreamPos, Image_DisplayAspectRatio).To_float32();
        float Width =Retrieve(Stream_Image, StreamPos, Image_Width             ).To_float32();
        float Height=Retrieve(Stream_Image, StreamPos, Image_Height            ).To_float32();
        if (DAR && Height && Width)
        {
            if (Retrieve(Stream_Image, StreamPos, Image_DisplayAspectRatio)==__T("1.778"))
                DAR=((float)16)/9; //More exact value
            if (Retrieve(Stream_Image, StreamPos, Image_DisplayAspectRatio)==__T("1.333"))
                DAR=((float)4)/3; //More exact value
            Fill(Stream_Image, StreamPos, Image_PixelAspectRatio, DAR/(((float32)Width)/Height));
        }
    }

    if (StreamKind==Stream_Image && Parameter==Image_PixelAspectRatio && !Value.empty() && Retrieve(Stream_Image, StreamPos, Image_DisplayAspectRatio).empty())
    {
        float32 PAR   =Retrieve(Stream_Image, StreamPos, Image_PixelAspectRatio  ).To_float32();
        if (PAR>(float32)12/(float32)11*0.999 && PAR<(float32)12/(float32)11*1.001)
            PAR=(float32)12/(float32)11;
        if (PAR>(float32)10/(float32)11*0.999 && PAR<(float32)10/(float32)11*1.001)
            PAR=(float32)10/(float32)11;
        if (PAR>(float32)16/(float32)11*0.999 && PAR<(float32)16/(float32)11*1.001)
            PAR=(float32)16/(float32)11;
        if (PAR>(float32)40/(float32)33*0.999 && PAR<(float32)40/(float32)33*1.001)
            PAR=(float32)40/(float32)33;
        if (PAR>(float32)24/(float32)11*0.999 && PAR<(float32)24/(float32)11*1.001)
            PAR=(float32)24/(float32)11;
        if (PAR>(float32)20/(float32)11*0.999 && PAR<(float32)20/(float32)11*1.001)
            PAR=(float32)20/(float32)11;
        if (PAR>(float32)32/(float32)11*0.999 && PAR<(float32)32/(float32)11*1.001)
            PAR=(float32)32/(float32)11;
        if (PAR>(float32)80/(float32)33*0.999 && PAR<(float32)80/(float32)33*1.001)
            PAR=(float32)80/(float32)33;
        if (PAR>(float32)18/(float32)11*0.999 && PAR<(float32)18/(float32)11*1.001)
            PAR=(float32)18/(float32)11;
        if (PAR>(float32)15/(float32)11*0.999 && PAR<(float32)15/(float32)11*1.001)
            PAR=(float32)15/(float32)11;
        if (PAR>(float32)64/(float32)33*0.999 && PAR<(float32)64/(float32)33*1.001)
            PAR=(float32)64/(float32)33;
        if (PAR>(float32)160/(float32)99*0.999 && PAR<(float32)160/(float32)99*1.001)
            PAR=(float32)160/(float32)99;
        if (PAR>(float32)4/(float32)3*0.999 && PAR<(float32)4/(float32)3*1.01)
            PAR=(float32)4/(float32)3;
        if (PAR>(float32)3/(float32)2*0.999 && PAR<(float32)3/(float32)2*1.001)
            PAR=(float32)3/(float32)2;
        if (PAR>(float32)2/(float32)1*0.999 && PAR<(float32)2/(float32)1*1.001)
            PAR=(float32)2;
        if (PAR>(float32)59/(float32)54*0.999 && PAR<(float32)59/(float32)54*1.001)
            PAR=(float32)59/(float32)54;
        float32 Width =Retrieve(Stream_Image, StreamPos, Image_Width             ).To_float32();
        float32 Height=Retrieve(Stream_Image, StreamPos, Image_Height            ).To_float32();
        if (PAR && Height && Width)
            Fill(Stream_Image, StreamPos, Image_DisplayAspectRatio, ((float32)Width)/Height*PAR);
    }

    //Commercial name
    if (Parameter==Fill_Parameter(StreamKind, Generic_Format))
        Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format_Commercial), Value);
    if (Parameter==Fill_Parameter(StreamKind, Generic_Format_Commercial_IfAny))
        Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format_Commercial), Value, true);

    if (!IsSub)
    {
        Ztring ParameterName=Retrieve(StreamKind, StreamPos, Parameter, Info_Name);

        //Lists
        if (StreamKind!=Stream_General &&  (ParameterName==__T("Codec/String")
                                         || ParameterName==__T("Language/String")
                                         || ParameterName==__T("Format")
                                         || ParameterName==__T("CodecID/Hint")))
        {
            Ztring Temp1, Temp2;
            for (size_t StreamPos_Local=0; StreamPos_Local<(*Stream)[StreamKind].size(); StreamPos_Local++)
            {
                if (ParameterName==__T("CodecID/Hint"))
                    Temp1+=Retrieve(StreamKind, StreamPos_Local, Fill_Parameter(StreamKind, Generic_Format))+__T(" / ");
                else
                    Temp1+=Retrieve(StreamKind, StreamPos_Local, Parameter)+__T(" / ");
                if (ParameterName==__T("Format")
                 || ParameterName==__T("CodecID/Hint"))
                {
                    Temp2+=Retrieve(StreamKind, StreamPos_Local, Fill_Parameter(StreamKind, Generic_Format));
                    if (!Retrieve(StreamKind, StreamPos_Local, Fill_Parameter(StreamKind, Generic_CodecID_Hint)).empty())
                    {
                        Temp2+=__T(" (");
                        Temp2+=Retrieve(StreamKind, StreamPos_Local, Fill_Parameter(StreamKind, Generic_CodecID_Hint));
                        Temp2+=__T(")");
                    }
                    Temp2+=__T(" / ");
                }
            }
            if (!Temp1.empty())
                Temp1.resize(Temp1.size()-3); //Delete extra " / "
            if (!Temp2.empty())
                Temp2.resize(Temp2.size()-3); //Delete extra " / "
            Ztring StreamKind_Text=Get(StreamKind, 0, General_StreamKind, Info_Text);
            if (ParameterName==__T("Codec/String"))
                Fill(Stream_General, 0, Ztring(StreamKind_Text+__T("_Codec_List")).To_Local().c_str(), Temp1, true);
            if (ParameterName==__T("Language/String"))
                Fill(Stream_General, 0, Ztring(StreamKind_Text+__T("_Language_List")).To_Local().c_str(), Temp1, true);
            if (ParameterName==__T("Format")
             || ParameterName==__T("CodecID/Hint"))
            {
                Fill(Stream_General, 0, Ztring(StreamKind_Text+__T("_Format_List")).To_Local().c_str(), Temp1, true);
                Fill(Stream_General, 0, Ztring(StreamKind_Text+__T("_Format_WithHint_List")).To_Local().c_str(), Temp2, true);
            }
        }

        //Human readable
        if (MediaInfoLib::Config.ReadByHuman_Get())
        {
            //Strings
            const Ztring &List_Measure_Value=MediaInfoLib::Config.Info_Get(StreamKind).Read(Parameter, Info_Measure);
                 if (List_Measure_Value==__T(" byte"))
                FileSize_FileSize123(StreamKind, StreamPos, Parameter);
            else if (List_Measure_Value==__T(" bps") || List_Measure_Value==__T(" Hz"))
                Kilo_Kilo123(StreamKind, StreamPos, Parameter);
            else if (List_Measure_Value==__T(" ms"))
                Duration_Duration123(StreamKind, StreamPos, Parameter);
            else if (List_Measure_Value==__T("Yes"))
                YesNo_YesNo(StreamKind, StreamPos, Parameter);
            else
                Value_Value123(StreamKind, StreamPos, Parameter);

            //BitRate_Mode / OverallBitRate_Mode
            if (ParameterName==(StreamKind==Stream_General?__T("OverallBitRate_Mode"):__T("BitRate_Mode")) && MediaInfoLib::Config.ReadByHuman_Get())
            {
                Clear(StreamKind, StreamPos, StreamKind==Stream_General?"OverallBitRate_Mode/String":"BitRate_Mode/String");

                ZtringList List;
                List.Separator_Set(0, __T(" / "));
                List.Write(Retrieve(StreamKind, StreamPos, Parameter));

                //Per value
                for (size_t Pos=0; Pos<List.size(); Pos++)
                    List[Pos]=MediaInfoLib::Config.Language_Get(Ztring(__T("BitRate_Mode_"))+List[Pos]);

                Ztring Translated=List.Read();
                Fill(StreamKind, StreamPos, StreamKind==Stream_General?"OverallBitRate_Mode/String":"BitRate_Mode/String", Translated.find(__T("BitRate_Mode_"))?Translated:Value);
            }

            //Encoded_Library
            if (ParameterName==__T("Encoded_Library")
             || ParameterName==__T("Encoded_Library/Name")
             || ParameterName==__T("Encoded_Library/Version")
             || ParameterName==__T("Encoded_Library/Date"))
            {
                Ztring Name=Retrieve(StreamKind, StreamPos, "Encoded_Library/Name");
                Ztring Version=Retrieve(StreamKind, StreamPos, "Encoded_Library/Version");
                Ztring Date=Retrieve(StreamKind, StreamPos, "Encoded_Library/Date");
                if (!Name.empty())
                {
                    Ztring String=Name;
                    if (!Version.empty())
                    {
                        String+=__T(" ");
                        String+=Version;
                    }
                    if (!Date.empty())
                    {
                        String+=__T(" (");
                        String+=Date;
                        String+=__T(")");
                    }
                    Fill(StreamKind, StreamPos, "Encoded_Library/String", String, true);
                }
                else
                    Fill(StreamKind, StreamPos, "Encoded_Library/String", Retrieve(StreamKind, StreamPos, "Encoded_Library"), true);
            }

            //Format_Settings_Matrix
            if (StreamKind==Stream_Video && Parameter==Video_Format_Settings_Matrix)
            {
                Ztring Translated=MediaInfoLib::Config.Language_Get(Ztring(__T("Format_Settings_Matrix_"))+Value);
                Fill(Stream_Video, StreamPos, Video_Format_Settings_Matrix_String, Translated.find(__T("Format_Settings_Matrix_"))?Translated:Value, Replace);
            }

            //Scan type
            if (StreamKind==Stream_Video && Parameter==Video_ScanType)
            {
                Ztring Translated=MediaInfoLib::Config.Language_Get(Ztring(__T("Interlaced_"))+Value);
                Fill(Stream_Video, StreamPos, Video_ScanType_String, Translated.find(__T("Interlaced_"))?Translated:Value, Replace);
            }
            if (StreamKind==Stream_Video && Parameter==Video_ScanType_Original)
            {
                Ztring Translated=MediaInfoLib::Config.Language_Get(Ztring(__T("Interlaced_"))+Value);
                Fill(Stream_Video, StreamPos, Video_ScanType_Original_String, Translated.find(__T("Interlaced_"))?Translated:Value, Replace);
            }

            //Scan order
            if (StreamKind==Stream_Video && Parameter==Video_ScanOrder)
            {
                Ztring Translated=MediaInfoLib::Config.Language_Get(Ztring(__T("Interlaced_"))+Value);
                Fill(Stream_Video, StreamPos, Video_ScanOrder_String, Translated.find(__T("Interlaced_"))?Translated:Value, Replace);
            }
            if (StreamKind==Stream_Video && Parameter==Video_ScanOrder_Original)
            {
                Ztring Translated=MediaInfoLib::Config.Language_Get(Ztring(__T("Interlaced_"))+Value);
                Fill(Stream_Video, StreamPos, Video_ScanOrder_Original_String, Translated.find(__T("Interlaced_"))?Translated:Value, Replace);
            }

            //Interlacement
            if (StreamKind==Stream_Video && Parameter==Video_Interlacement)
            {
                const Ztring &Z1=Retrieve(Stream_Video, StreamPos, Video_Interlacement);
                if (Z1.size()==3)
                    Fill(Stream_Video, StreamPos, Video_Interlacement_String, MediaInfoLib::Config.Language_Get(Ztring(__T("Interlaced_"))+Z1), Replace);
                else
                    Fill(Stream_Video, StreamPos, Video_Interlacement_String, MediaInfoLib::Config.Language_Get(Z1), Replace);
                if (Retrieve(Stream_Video, StreamPos, Video_Interlacement_String).empty())
                    Fill(Stream_Video, StreamPos, Video_Interlacement_String, Z1, true);
            }

            //FrameRate_Mode
            if (StreamKind==Stream_Video && Parameter==Video_FrameRate_Mode)
            {
                Ztring Translated=MediaInfoLib::Config.Language_Get(Ztring(__T("FrameRate_Mode_"))+Value);
                Fill(Stream_Video, StreamPos, Video_FrameRate_Mode_String, Translated.find(__T("FrameRate_Mode_"))?Translated:Value, Replace);
            }

            //Compression_Mode
            if (Parameter==Fill_Parameter(StreamKind, Generic_Compression_Mode))
            {
                Ztring Translated=MediaInfoLib::Config.Language_Get(Ztring(__T("Compression_Mode_"))+Value);
                Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Compression_Mode_String), Translated.find(__T("Compression_Mode_"))?Translated:Value, Replace);
            }

            //Delay_Source
            if (Parameter==Fill_Parameter(StreamKind, Generic_Delay_Source))
            {
                Ztring Translated=MediaInfoLib::Config.Language_Get(Ztring(__T("Delay_Source_"))+Value);
                Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Delay_Source_String), Translated.find(__T("Delay_Source_"))?Translated:Value, Replace);
            }
        }

        //General Format
        if (Parameter==Fill_Parameter(StreamKind, Generic_Format) && Retrieve(Stream_General, 0, General_Format).empty() && !Value.empty())
            Fill(Stream_General, 0, General_Format, Value); //If not already filled, we are filling with the stream format

        //ID
        if (Parameter==General_ID)
            Fill(StreamKind, StreamPos, General_ID_String, Value, Replace);

        //Format
        if (Parameter==Fill_Parameter(StreamKind, Generic_Format))
        {
            if ((Replace && !MediaInfoLib::Config.Format_Get(Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format)), InfoFormat_Info).empty()) || Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format_Info)).empty())
                Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format_Info), MediaInfoLib::Config.Format_Get(Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format)), InfoFormat_Info), true);
            if ((Replace && !MediaInfoLib::Config.Format_Get(Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format)), InfoFormat_Url).empty()) || Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format_Url)).empty())
                Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format_Url) , MediaInfoLib::Config.Format_Get(Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format)), InfoFormat_Url), true);
            if (StreamKind!=Stream_Menu)
            {
                if ((Replace && !MediaInfoLib::Config.Format_Get(Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format)), InfoFormat_InternetMediaType).empty()) || Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_InternetMediaType)).empty())
                    Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_InternetMediaType), MediaInfoLib::Config.Format_Get(Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format)), InfoFormat_InternetMediaType), true);
                if ((Replace && !MediaInfoLib::Config.Format_Get(Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format)), InfoFormat_Compression_Mode).empty()) || Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Compression_Mode)).empty())
                    Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Compression_Mode), MediaInfoLib::Config.Format_Get(Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Format)), InfoFormat_Compression_Mode), true);
            }
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
        if (Parameter==Fill_Parameter(StreamKind, Generic_Codec))
        {
            const Ztring &C1=MediaInfoLib::Config.Codec_Get(Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Codec)), InfoCodec_Name, (stream_t)StreamKind);
            if (C1.empty())
                Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Codec_String), Value, true);
            else
            {
                Ztring D=Retrieve(StreamKind, StreamPos, "Codec/Family");
                Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Codec_String), C1, true);
                Fill(StreamKind, StreamPos, "Codec/Family", MediaInfoLib::Config.Codec_Get(Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Codec)), InfoCodec_KindofCodec, StreamKind), true);
                Ztring B=Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Codec));
                Ztring C=MediaInfoLib::Config.Codec_Get(B, InfoCodec_KindofCodec, StreamKind);
                Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Codec_Info)  , MediaInfoLib::Config.Codec_Get(Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Codec)), InfoCodec_Description, StreamKind), true);
                Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Codec_Url)   , MediaInfoLib::Config.Codec_Get(Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Codec)), InfoCodec_Url,         StreamKind), true);
            }
        }

        //CodecID_Description
        if (Parameter==Fill_Parameter(StreamKind, Generic_CodecID_Info) && Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_CodecID_Description))==Value)
            Clear(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_CodecID_Description));

        //BitRate from BitRate_Nominal
        if (Parameter==Fill_Parameter(StreamKind, Generic_BitRate)
         || Parameter==Fill_Parameter(StreamKind, Generic_BitRate_Nominal))
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

        //BitRate from BitRate_Maximum
        if (Parameter==Fill_Parameter(StreamKind, Generic_BitRate)
         || Parameter==Fill_Parameter(StreamKind, Generic_BitRate_Maximum))
        {
            float32 BitRate=Retrieve(StreamKind, StreamPos, "BitRate").To_float32();
            float32 BitRate_Maximum=Retrieve(StreamKind, StreamPos, "BitRate_Maximum").To_float32();
            if (BitRate>BitRate_Maximum*0.99 && BitRate<BitRate_Maximum*1.01)
            {
                Ztring Temp=Retrieve(StreamKind, StreamPos, "BitRate_Maximum");
                Clear(StreamKind, StreamPos, "BitRate_Maximum");
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
            if (Replace)
                Clear(Stream_Audio, StreamPos, Audio_Video_Delay);
            ZtringList AudioDelay; AudioDelay.Separator_Set(0, __T(" / ")); AudioDelay.Write(Retrieve(Stream_Audio, StreamPos, Audio_Delay));
            ZtringList VideoDelay; VideoDelay.Separator_Set(0, __T(" / ")); VideoDelay.Write(Retrieve(Stream_Video, 0, Video_Delay));
            if (AudioDelay.size()<=VideoDelay.size())
            {
                Fill(Stream_Audio, StreamPos, Audio_Video_Delay, AudioDelay(AudioDelay.size()-1).To_int64s()-VideoDelay(AudioDelay.size()-1).To_int64s(), 10);
                if (VideoDelay.size()==1 && Retrieve(Stream_Audio, StreamPos, Audio_Video_Delay).To_int64u()==0)
                    for (size_t Pos=Audio_Video_Delay+1; Pos<=Audio_Video_Delay+4; Pos++)
                        if (Pos<(*Stream)[Stream_Audio][StreamPos].size())
                            (*Stream)[Stream_Audio][StreamPos][Pos].clear();
            }
        }
        if (StreamKind==Stream_Text && Parameter==Text_Delay && Count_Get(Stream_Video) && !Retrieve(Stream_Text, StreamPos, Text_Delay).empty() && !Retrieve(Stream_Video, 0, Video_Delay).empty())
        {
            Ztring MuxingMode_MoreInfo=Get(Stream_Text, StreamPos, "MuxingMode_MoreInfo");
            Ztring StreamID=MuxingMode_MoreInfo.SubString(__T("Muxed in Video #"), Ztring());
            size_t StreamID_Int=(size_t)StreamID.To_int64u();
            if (StreamID_Int)
                StreamID_Int--;
            Fill(Stream_Text, StreamPos, Text_Video_Delay, Value.To_int64s()-Retrieve(Stream_Video, StreamID_Int, Video_Delay).To_int64s(), 10, true);
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
            if (Replace)
                Clear(Stream_Audio, StreamPos, Audio_Video0_Delay);
            ZtringList AudioDelay; AudioDelay.Separator_Set(0, __T(" / ")); AudioDelay.Write(Retrieve(Stream_Audio, StreamPos, Audio_Delay));
            ZtringList VideoDelay; VideoDelay.Separator_Set(0, __T(" / ")); VideoDelay.Write(Retrieve(Stream_Video, 0, Video_Delay));
            if (AudioDelay.size()<=VideoDelay.size())
            {
                Fill(Stream_Audio, StreamPos, Audio_Video0_Delay, AudioDelay(AudioDelay.size()-1).To_int64s()-VideoDelay(AudioDelay.size()-1).To_int64s(), 10);
                if (VideoDelay.size()==1 && Retrieve(Stream_Audio, StreamPos, Audio_Video0_Delay).To_int64u()==0)
                    for (size_t Pos=Audio_Video0_Delay+1; Pos<=Audio_Video0_Delay+4; Pos++)
                        if (Pos<(*Stream)[Stream_Audio][StreamPos].size())
                            (*Stream)[Stream_Audio][StreamPos][Pos].clear();
            }
        }
        if (StreamKind==Stream_Text && Parameter==Text_Delay && Count_Get(Stream_Video) && !Retrieve(Stream_Text, StreamPos, Text_Delay).empty() && !Retrieve(Stream_Video, 0, Video_Delay).empty())
        {
            Ztring MuxingMode_MoreInfo=Get(Stream_Text, StreamPos, "MuxingMode_MoreInfo");
            Ztring StreamID=MuxingMode_MoreInfo.SubString(__T("Muxed in Video #"), Ztring());
            size_t StreamID_Int=(size_t)StreamID.To_int64u();
            if (StreamID_Int)
                StreamID_Int--;
            Fill(Stream_Text, StreamPos, Text_Video0_Delay, Value.To_int64s()-Retrieve(Stream_Video, StreamID_Int, Video_Delay).To_int64s(), 10, true);
            if (Retrieve(Stream_Text, StreamPos, Text_Video0_Delay).To_int64u()==0)
                for (size_t Pos=Text_Video0_Delay+1; Pos<=Text_Video0_Delay+4; Pos++)
                    if (Pos<(*Stream)[Stream_Text][StreamPos].size())
                        (*Stream)[Stream_Text][StreamPos][Pos].clear();
        }

        //Language
        //-Find 2-digit language
        if (Parameter==Fill_Parameter(StreamKind, Generic_Language))
        {
            //Removing old strings
            Clear(StreamKind, StreamPos, Parameter+1); //String
            Clear(StreamKind, StreamPos, Parameter+2); //String1
            Clear(StreamKind, StreamPos, Parameter+3); //String2
            Clear(StreamKind, StreamPos, Parameter+4); //String3
            Clear(StreamKind, StreamPos, Parameter+5); //String4

            ZtringListList Languages;
            Languages.Separator_Set(0, __T(" / "));
            Languages.Separator_Set(1, __T("-"));
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
                    if ((Languages[Pos][0].size()==3 && (Languages[Pos][0]==__T("mis")
                                                      || Languages[Pos][0]==__T("und")
                                                      || Languages[Pos][0]==__T("???")
                                                      || Languages[Pos][0]==__T("   ")))
                     || (Languages[Pos][0].size()==2 && Languages[Pos][0]==__T("  ")))
                        Languages[Pos].clear();
                }

                //Finding ISO-639-1 from ISO-639-2 or translated name
                if (Languages[Pos].size()>=1)
                {
                    if (Languages[Pos][0].size()==3 && !MediaInfoLib::Config.Iso639_1_Get(Languages[Pos][0]).empty())
                        Languages[Pos][0]=MediaInfoLib::Config.Iso639_1_Get(Languages[Pos][0]);
                    if (Languages[Pos][0].size()>3 && !MediaInfoLib::Config.Iso639_Find(Languages[Pos][0]).empty())
                        Languages[Pos][0]=MediaInfoLib::Config.Iso639_Find(Languages[Pos][0]);
                    if (Languages[Pos][0].size()>3)
                        Languages[Pos][0]=Language_Orig; //We failed to detect language, using the original version
                }
            }

            if (Languages.Read()!=Retrieve(StreamKind, StreamPos, Parameter))
                Fill(StreamKind, StreamPos, Parameter, Languages.Read(), true);
            else
            {
                ZtringList Language1; Language1.Separator_Set(0, __T(" / "));
                ZtringList Language2; Language2.Separator_Set(0, __T(" / "));
                ZtringList Language3; Language3.Separator_Set(0, __T(" / "));
                ZtringList Language4; Language4.Separator_Set(0, __T(" / "));

                for (size_t Pos=0; Pos<Languages.size(); Pos++)
                {
                    if (Languages[Pos].size()>=1)
                    {
                        Ztring Language_Translated=MediaInfoLib::Config.Language_Get(__T("Language_")+Languages[Pos][0]);
                        if (Language_Translated.find(__T("Language_"))==0)
                            Language_Translated=Languages[Pos][0]; //No translation found
                        if (Languages[Pos].size()>=2)
                        {
                            if (Languages[Pos].size()==2 && Languages[Pos][0].size()>=2 && Languages[Pos][0].size()<=3 && (Languages[Pos][1][0]&0xDF)>=__T('A') && (Languages[Pos][1][0]&0xDF)<=__T('Z') && (Languages[Pos][1][1]&0xDF)>=__T('A') && (Languages[Pos][1][1]&0xDF)<=__T('Z'))
                            {
                                Language_Translated+=__T(" (");
                                Language_Translated+=Ztring(Languages[Pos][1]).MakeUpperCase();
                                Language_Translated+=__T(")");
                            }
                            else
                                for (size_t Pos2=1; Pos2<Languages[Pos].size(); Pos2++)
                                {
                                    Language_Translated+=__T('-'); //As the original string
                                    Language_Translated+=Languages[Pos][Pos2];
                                }
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
        if (Parameter==Fill_Parameter(StreamKind, Generic_ServiceName)
         || Parameter==Fill_Parameter(StreamKind, Generic_ServiceProvider))
        {
            if (Retrieve(StreamKind, StreamPos, Parameter).find(__T(" - "))==string::npos && (Retrieve(StreamKind, StreamPos, Parameter).find(__T(":"))==2 || Retrieve(StreamKind, StreamPos, Parameter).find(__T(":"))==3))
            {
                Ztring Temp=Retrieve(StreamKind, StreamPos, Parameter);
                Temp.erase(0, Retrieve(StreamKind, StreamPos, Parameter).find(__T(":"))+1);
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
                 if (F1>=(float)1.23 && F1<(float)1.27) C1=__T("5:4");
            else if (F1>=(float)1.30 && F1<(float)1.37) C1=__T("4:3");
            else if (F1>=(float)1.45 && F1<(float)1.55) C1=__T("3:2");
            else if (F1>=(float)1.55 && F1<(float)1.65) C1=__T("16:10");
            else if (F1>=(float)1.74 && F1<(float)1.82) C1=__T("16:9");
            else if (F1>=(float)1.82 && F1<(float)1.88) C1=__T("1.85:1");
            else if (F1>=(float)2.15 && F1<(float)2.22) C1=__T("2.2:1");
            else if (F1>=(float)2.23 && F1<(float)2.30) C1=__T("2.25:1");
            else if (F1>=(float)2.30 && F1<(float)2.37) C1=__T("2.35:1");
            else if (F1>=(float)2.37 && F1<(float)2.45) C1=__T("2.40:1");
            else              C1.From_Number(F1);
            C1.FindAndReplace(__T("."), MediaInfoLib::Config.Language_Get(__T("  Config_Text_FloatSeparator")));
            if (MediaInfoLib::Config.Language_Get(__T("  Language_ISO639"))==__T("fr") && C1.find(__T(":1"))==string::npos)
                C1.FindAndReplace(__T(":"), __T("/"));
            Fill(Stream_Video, StreamPos, Video_DisplayAspectRatio_String, C1, true);
        }

        //Original Display Aspect Ratio and Original Pixel Aspect Ratio
        if (StreamKind==Stream_Video && Parameter==Video_DisplayAspectRatio_Original)
        {
            float F1=Retrieve(Stream_Video, StreamPos, Video_DisplayAspectRatio_Original).To_float32();
            Ztring C1;
                 if (F1>=(float)1.23 && F1<(float)1.27) C1=__T("5:4");
            else if (F1>=(float)1.30 && F1<(float)1.37) C1=__T("4:3");
            else if (F1>=(float)1.45 && F1<(float)1.55) C1=__T("3:2");
            else if (F1>=(float)1.55 && F1<(float)1.65) C1=__T("16:10");
            else if (F1>=(float)1.74 && F1<(float)1.82) C1=__T("16:9");
            else if (F1>=(float)1.82 && F1<(float)1.88) C1=__T("1.85:1");
            else if (F1>=(float)2.15 && F1<(float)2.22) C1=__T("2.2:1");
            else if (F1>=(float)2.23 && F1<(float)2.30) C1=__T("2.25:1");
            else if (F1>=(float)2.30 && F1<(float)2.37) C1=__T("2.35:1");
            else if (F1>=(float)2.37 && F1<(float)2.45) C1=__T("2.40:1");
            else              C1.From_Number(F1);
            C1.FindAndReplace(__T("."), MediaInfoLib::Config.Language_Get(__T("  Config_Text_FloatSeparator")));
            if (MediaInfoLib::Config.Language_Get(__T("  Language_ISO639"))==__T("fr") && C1.find(__T(":1"))==string::npos)
                C1.FindAndReplace(__T(":"), __T("/"));
            Fill(Stream_Video, StreamPos, Video_DisplayAspectRatio_Original_String, C1, true);
        }

        //Display Aspect Ratio and Pixel Aspect Ratio
        if (StreamKind==Stream_Image && Parameter==Image_DisplayAspectRatio && !Value.empty())
        {
            float F1=Retrieve(Stream_Image, StreamPos, Image_DisplayAspectRatio).To_float32();
            Ztring C1;
                 if (F1>=(float)1.23 && F1<(float)1.27) C1=__T("5:4");
            else if (F1>=(float)1.30 && F1<(float)1.37) C1=__T("4:3");
            else if (F1>=(float)1.45 && F1<(float)1.55) C1=__T("3:2");
            else if (F1>=(float)1.55 && F1<(float)1.65) C1=__T("16:10");
            else if (F1>=(float)1.74 && F1<(float)1.82) C1=__T("16:9");
            else if (F1>=(float)1.82 && F1<(float)1.88) C1=__T("1.85:1");
            else if (F1>=(float)2.15 && F1<(float)2.22) C1=__T("2.2:1");
            else if (F1>=(float)2.23 && F1<(float)2.30) C1=__T("2.25:1");
            else if (F1>=(float)2.30 && F1<(float)2.37) C1=__T("2.35:1");
            else if (F1>=(float)2.37 && F1<(float)2.45) C1=__T("2.40:1");
            else              C1.From_Number(F1);
            C1.FindAndReplace(__T("."), MediaInfoLib::Config.Language_Get(__T("  Config_Text_FloatSeparator")));
            if (MediaInfoLib::Config.Language_Get(__T("  Language_ISO639"))==__T("fr") && C1.find(__T(":1"))==string::npos)
                C1.FindAndReplace(__T(":"), __T("/"));
            Fill(Stream_Image, StreamPos, Image_DisplayAspectRatio_String, C1, true);
        }

        //Original Display Aspect Ratio and Original Pixel Aspect Ratio
        if (StreamKind==Stream_Image && Parameter==Image_DisplayAspectRatio_Original)
        {
            float F1=Retrieve(Stream_Image, StreamPos, Image_DisplayAspectRatio_Original).To_float32();
            Ztring C1;
                 if (F1>=(float)1.23 && F1<(float)1.27) C1=__T("5:4");
            else if (F1>=(float)1.30 && F1<(float)1.37) C1=__T("4:3");
            else if (F1>=(float)1.45 && F1<(float)1.55) C1=__T("3:2");
            else if (F1>=(float)1.55 && F1<(float)1.65) C1=__T("16:10");
            else if (F1>=(float)1.74 && F1<(float)1.82) C1=__T("16:9");
            else if (F1>=(float)1.82 && F1<(float)1.88) C1=__T("1.85:1");
            else if (F1>=(float)2.15 && F1<(float)2.22) C1=__T("2.2:1");
            else if (F1>=(float)2.23 && F1<(float)2.30) C1=__T("2.25:1");
            else if (F1>=(float)2.30 && F1<(float)2.37) C1=__T("2.35:1");
            else if (F1>=(float)2.37 && F1<(float)2.45) C1=__T("2.40:1");
            else              C1.From_Number(F1);
            C1.FindAndReplace(__T("."), MediaInfoLib::Config.Language_Get(__T("  Config_Text_FloatSeparator")));
            if (MediaInfoLib::Config.Language_Get(__T("  Language_ISO639"))==__T("fr") && C1.find(__T(":1"))==string::npos)
                C1.FindAndReplace(__T(":"), __T("/"));
            Fill(Stream_Image, StreamPos, Image_DisplayAspectRatio_Original_String, C1, true);
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
         &&(Value==__T("samr")
         || Value==__T("sawb")
         || Value==__T("7A21")
         || Value==__T("7A22"))
            )
            Fill(Stream_Audio, StreamPos, Audio_Channel_s_, 1, 10, true); //AMR is always with 1 channel

        //Well known bitrate values
        if (StreamKind==Stream_Video && (Parameter==Video_BitRate || Parameter==Video_BitRate_Nominal))
            Video_BitRate_Rounding(StreamPos, (video)Parameter);
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
    if (Value.find(__T('\r'))!=string::npos || Value.find(__T('\n'))!=string::npos)
    {
        Ztring NewValue=Value;
        NewValue.FindAndReplace(__T("\r\n"), __T(" / "), 0, Ztring_Recursive);
        NewValue.FindAndReplace(__T("\r"), __T(" / "), 0, Ztring_Recursive);
        NewValue.FindAndReplace(__T("\n"), __T(" / "), 0, Ztring_Recursive);
        if (NewValue.size()>=3 && NewValue.rfind(__T(" / "))==NewValue.size()-3)
            NewValue.resize(NewValue.size()-3);
        Fill(StreamKind, StreamPos, Parameter, NewValue, Replace);
        return;
    }

    //Handle Value before StreamKind
    if (StreamKind==Stream_Max || StreamPos>=(*Stream)[StreamKind].size())
    {
        Ztring ParameterZ=Ztring().From_UTF8(Parameter);
        if (Replace)
            for (size_t Pos=0; Pos<Fill_Temp.size(); Pos++)
                if (Fill_Temp[Pos](0)==ParameterZ)
                {
                    Fill_Temp.erase(Fill_Temp.begin()+Pos);
                    Pos--;
                }
        ZtringList NewList;
        NewList.push_back(ParameterZ);
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
            size_t Pos_ToReplace=(*Stream_More)[StreamKind][StreamPos].Find(Ztring().From_ISO_8859_1(Parameter), Info_Name);
            if (Pos_ToReplace!=(size_t)-1)
                (*Stream_More)[StreamKind][StreamPos].erase((*Stream_More)[StreamKind][StreamPos].begin()+Pos_ToReplace); //Empty value --> remove the line
        }
    }
    else
    {
        Ztring &Target=(*Stream_More)[StreamKind][StreamPos](Ztring().From_ISO_8859_1(Parameter), Info_Text);
        if (Target.empty() || Replace)
        {
            Target=Value; //First value
            (*Stream_More)[StreamKind][StreamPos](Ztring().From_ISO_8859_1(Parameter), Info_Name_Text)=MediaInfoLib::Config.Language_Get(Ztring().From_Local(Parameter));
            (*Stream_More)[StreamKind][StreamPos](Ztring().From_ISO_8859_1(Parameter), Info_Options)=__T("Y NT");
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
                 if (List_Measure_Value==__T(" byte"))
            {
                const Ztring &Temp=MediaInfoLib::Config.Info_Get(StreamKind).Read(Parameter, Info_Name);
                size_t List_Size=Temp.find(__T("StreamSize"))==string::npos?5:7; //for /String5, with percentage, and proportion
                for (size_t Pos=Parameter+1; Pos<=Parameter+List_Size; Pos++)
                    if (Pos<(*Stream)[StreamKind][StreamPos].size())
                        (*Stream)[StreamKind][StreamPos][Pos].clear();
            }
            else if (List_Measure_Value==__T(" bps") || List_Measure_Value==__T(" Hz"))
            {
                if (Parameter+1<(*Stream)[StreamKind][StreamPos].size())
                    (*Stream)[StreamKind][StreamPos][Parameter+1].clear();
            }
            else if (List_Measure_Value==__T(" ms"))
            {
                for (size_t Pos=Parameter+1; Pos<=Parameter+4; Pos++)
                    if (Pos<(*Stream)[StreamKind][StreamPos].size())
                        (*Stream)[StreamKind][StreamPos][Pos].clear();
            }
            else if (List_Measure_Value==__T("Yes"))
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
    for (size_t Pos=General_Inform; Pos<Pos_Count; Pos++)
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
    if (!Status[IsAccepted] || &ToAdd==NULL || StreamKind>=Stream_Max || !ToAdd.Stream || StreamPos_From>=(*ToAdd.Stream)[StreamKind].size())
        return 0;

    //Destination
    while (StreamPos_To>=(*Stream)[StreamKind].size())
        Stream_Prepare(StreamKind);

    //Specific stuff
    Ztring Width_Temp, Height_Temp, PixelAspectRatio_Temp, DisplayAspectRatio_Temp, FrameRate_Temp, FrameRate_Mode_Temp, ScanType_Temp, ScanOrder_Temp, Channels_Temp, Delay_Temp, Delay_DropFrame_Temp, Delay_Source_Temp, Delay_Settings_Temp, Source_Temp, Source_Kind_Temp, Source_Info_Temp;
    if (StreamKind==Stream_Video)
    {
        Width_Temp=Retrieve(Stream_Video, StreamPos_To, Video_Width);
        Height_Temp=Retrieve(Stream_Video, StreamPos_To, Video_Height);
        PixelAspectRatio_Temp=Retrieve(Stream_Video, StreamPos_To, Video_PixelAspectRatio); //We want to keep the PixelAspectRatio_Temp of the video stream
        DisplayAspectRatio_Temp=Retrieve(Stream_Video, StreamPos_To, Video_DisplayAspectRatio); //We want to keep the DisplayAspectRatio_Temp of the video stream
        FrameRate_Temp=Retrieve(Stream_Video, StreamPos_To, Video_FrameRate); //We want to keep the FrameRate of AVI 120 fps
        FrameRate_Mode_Temp=Retrieve(Stream_Video, StreamPos_To, Video_FrameRate_Mode); //We want to keep the FrameRate_Mode of AVI 120 fps
        ScanType_Temp=Retrieve(Stream_Video, StreamPos_To, Video_ScanType);
        ScanOrder_Temp=Retrieve(Stream_Video, StreamPos_To, Video_ScanOrder);
    }
    if (StreamKind==Stream_Audio)
    {
        Channels_Temp=Retrieve(Stream_Audio, StreamPos_To, Audio_Channel_s_);
    }
    if (ToAdd.Retrieve(StreamKind, StreamPos_From, Fill_Parameter(StreamKind, Generic_Delay_Source))==__T("Container"))
    {
        Fill(StreamKind, StreamPos_To, "Delay_Original", Retrieve(StreamKind, StreamPos_To, "Delay"), true);
        Clear(StreamKind, StreamPos_To, "Delay");
        Fill(StreamKind, StreamPos_To, "Delay_Original_DropFrame", Retrieve(StreamKind, StreamPos_To, "Delay_DropFrame"), true);
        Clear(StreamKind, StreamPos_To, "Delay_DropFrame");
        Fill(StreamKind, StreamPos_To, "Delay_Original_Source", Retrieve(StreamKind, StreamPos_To, "Delay_Source"), true);
        Clear(StreamKind, StreamPos_To, "Delay_Source");
        if (!ToAdd.Retrieve(StreamKind, StreamPos_To, "Format").empty()) //Exception: MPEG-4 TimeCode, settings are in the MPEG-4 header
        {
            Fill(StreamKind, StreamPos_To, "Delay_Original_Settings", Retrieve(StreamKind, StreamPos_To, "Delay_Settings"), true);
            Clear(StreamKind, StreamPos_To, "Delay_Settings");
        }
    }
    else
    {
        Delay_Temp=Retrieve(StreamKind, StreamPos_To, "Delay"); //We want to keep the Delay from the stream
        Delay_Settings_Temp=Retrieve(StreamKind, StreamPos_To, "Delay_Settings"); //We want to keep the Delay_Settings from the stream
        Delay_DropFrame_Temp=Retrieve(StreamKind, StreamPos_To, "Delay_DropFrame"); //We want to keep the Delay_Source from the stream
        Delay_Source_Temp=Retrieve(StreamKind, StreamPos_To, "Delay_Source"); //We want to keep the Delay_Source from the stream
    }
    Source_Temp=Retrieve(StreamKind, StreamPos_To, "Source");
    Source_Kind_Temp=Retrieve(StreamKind, StreamPos_To, "Source_Kind");
    Source_Info_Temp=Retrieve(StreamKind, StreamPos_To, "Source_Info");

    //Merging
    size_t Count=0;
    size_t Size=ToAdd.Count_Get(StreamKind, StreamPos_From);
    for (size_t Pos=General_Inform; Pos<Size; Pos++)
    {
        const Ztring &ToFill_Value=ToAdd.Get(StreamKind, StreamPos_From, Pos);
        if (!ToFill_Value.empty() && (Erase || Get(StreamKind, StreamPos_To, Pos).empty()))
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

    //Specific stuff
    if (StreamKind==Stream_Video)
    {
        Ztring PixelAspectRatio_Original=Retrieve(Stream_Video, StreamPos_To, Video_PixelAspectRatio);
        Ztring DisplayAspectRatio_Original=Retrieve(Stream_Video, StreamPos_To, Video_DisplayAspectRatio);

        if (!Width_Temp.empty() && Width_Temp!=Retrieve(Stream_Video, StreamPos_To, Video_Width))
        {
            Fill(Stream_Video, StreamPos_To, Video_Width_Original, (*Stream)[Stream_Video][StreamPos_To][Video_Width], true);
            Fill(Stream_Video, StreamPos_To, Video_Width, Width_Temp, true);
        }
        if (!Height_Temp.empty() && Height_Temp!=Retrieve(Stream_Video, StreamPos_To, Video_Height))
        {
            Fill(Stream_Video, StreamPos_To, Video_Height_Original, (*Stream)[Stream_Video][StreamPos_To][Video_Height], true);
            Fill(Stream_Video, StreamPos_To, Video_Height, Height_Temp, true);
        }
        if (!PixelAspectRatio_Temp.empty() && PixelAspectRatio_Temp!=Retrieve(Stream_Video, StreamPos_To, Video_PixelAspectRatio))
        {
            Fill(Stream_Video, StreamPos_To, Video_PixelAspectRatio_Original, PixelAspectRatio_Original, true);
            Fill(Stream_Video, StreamPos_To, Video_PixelAspectRatio, PixelAspectRatio_Temp, true);
        }
        if (!DisplayAspectRatio_Temp.empty() && DisplayAspectRatio_Temp!=DisplayAspectRatio_Original)
        {
            Fill(Stream_Video, StreamPos_To, Video_DisplayAspectRatio_Original, DisplayAspectRatio_Original, true);
            Fill(Stream_Video, StreamPos_To, Video_DisplayAspectRatio, DisplayAspectRatio_Temp, true);
        }
        if (!FrameRate_Temp.empty() && FrameRate_Temp!=Retrieve(Stream_Video, StreamPos_To, Video_FrameRate))
        {
            Fill(Stream_Video, StreamPos_To, Video_FrameRate_Original, (*Stream)[Stream_Video][StreamPos_To][Video_FrameRate], true);
            Fill(Stream_Video, StreamPos_To, Video_FrameRate, FrameRate_Temp, true);
        }
        if (!FrameRate_Mode_Temp.empty() && FrameRate_Mode_Temp!=Retrieve(Stream_Video, StreamPos_To, Video_FrameRate_Mode))
        {
            Fill(Stream_Video, StreamPos_To, Video_FrameRate_Mode_Original, (*Stream)[Stream_Video][StreamPos_To][Video_FrameRate_Mode], true);
            Fill(Stream_Video, StreamPos_To, Video_FrameRate_Mode, FrameRate_Mode_Temp, true);
        }
        if (!ScanType_Temp.empty() && (ScanType_Temp!=Retrieve(Stream_Video, StreamPos_To, Video_ScanType) && !(ScanType_Temp==__T("Interlaced") && Retrieve(Stream_Video, StreamPos_To, Video_ScanType)==__T("MBAFF"))))
        {
            Fill(Stream_Video, StreamPos_To, Video_ScanType_Original, (*Stream)[Stream_Video][StreamPos_To][Video_ScanType], true);
            Fill(Stream_Video, StreamPos_To, Video_ScanType, ScanType_Temp, true);
        }
        if ((!ScanOrder_Temp.empty() && ScanOrder_Temp!=Retrieve(Stream_Video, StreamPos_To, Video_ScanOrder)) || !Retrieve(Stream_Video, StreamPos_To, Video_ScanType_Original).empty())
        {
            Fill(Stream_Video, StreamPos_To, Video_ScanOrder_Original, (*Stream)[Stream_Video][StreamPos_To][Video_ScanOrder], true);
            if (ScanOrder_Temp.empty())
            {
                Clear(Stream_Video, StreamPos_To, Video_ScanOrder);
                Clear(Stream_Video, StreamPos_To, Video_ScanOrder_String);
            }
            else
                Fill(Stream_Video, StreamPos_To, Video_ScanOrder, ScanOrder_Temp, true);
        }
    }
    if (StreamKind==Stream_Audio)
    {
        if (!Channels_Temp.empty())
        {
            //Test with legacy streams information
            bool IsOk=(Channels_Temp==Retrieve(Stream_Audio, StreamPos_To, Audio_Channel_s_));
            if (!IsOk)
            {
                ZtringList Temp; Temp.Separator_Set(0, __T(" / "));
                Temp.Write(Retrieve(Stream_Audio, StreamPos_To, Audio_Channel_s_));
                for (size_t Pos=0; Pos<Temp.size(); Pos++)
                    if (Channels_Temp==Temp[Pos])
                        IsOk=true;
            }

            if (!IsOk)
            {
                Fill(Stream_Audio, StreamPos_To, Audio_Channel_s__Original, (*Stream)[Stream_Audio][StreamPos_To][Audio_Channel_s_], true);
                Fill(Stream_Audio, StreamPos_To, Audio_Channel_s_, Channels_Temp, true);
            }
        }
    }
    if (!Delay_Source_Temp.empty() && Delay_Source_Temp!=Retrieve(StreamKind, StreamPos_To, "Delay_Source"))
    {
        Fill(StreamKind, StreamPos_To, "Delay_Original", Retrieve(StreamKind, StreamPos_To, "Delay"), true);
        Fill(StreamKind, StreamPos_To, "Delay", Delay_Temp, true);
        Fill(StreamKind, StreamPos_To, "Delay_Original_Settings", Retrieve(StreamKind, StreamPos_To, "Delay_Settings"), true);
        Fill(StreamKind, StreamPos_To, "Delay_Settings", Delay_Settings_Temp, true);
        Fill(StreamKind, StreamPos_To, "Delay_Original_DropFrame", Retrieve(StreamKind, StreamPos_To, "Delay_DropFrame"), true);
        Fill(StreamKind, StreamPos_To, "Delay_DropFrame", Delay_DropFrame_Temp, true);
        Fill(StreamKind, StreamPos_To, "Delay_Original_Source", Retrieve(StreamKind, StreamPos_To, "Delay_Source"), true);
        Fill(StreamKind, StreamPos_To, "Delay_Source", Delay_Source_Temp, true);
    }
    if (!Source_Temp.empty() && Source_Temp!=Retrieve(StreamKind, StreamPos_To, "Source"))
    {
        Fill(StreamKind, StreamPos_To, "Source_Original", Retrieve(StreamKind, StreamPos_To, "Source"), true);
        Fill(StreamKind, StreamPos_To, "Source", Source_Temp, true);
        Fill(StreamKind, StreamPos_To, "Source_Original_Kind", Retrieve(StreamKind, StreamPos_To, "Source_Kind"), true);
        Fill(StreamKind, StreamPos_To, "Source_Kind", Source_Info_Temp, true);
        Fill(StreamKind, StreamPos_To, "Source_Original_Info", Retrieve(StreamKind, StreamPos_To, "Source_Info"), true);
        Fill(StreamKind, StreamPos_To, "Source_Info", Source_Info_Temp, true);
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
    else if (FrameRate>23.952 && FrameRate<=23.988) FrameRate=23.976;
    else if (FrameRate>23.988 && FrameRate<=24.024) FrameRate=24.000;
    else if (FrameRate>24.975 && FrameRate<=25.025) FrameRate=25.000;
    else if (FrameRate>29.940 && FrameRate<=29.985) FrameRate=29.970;
    else if (FrameRate>29.970 && FrameRate<=30.030) FrameRate=30.000;
    else if (FrameRate>23.952*2 && FrameRate<=23.988*2) FrameRate=23.976*2;
    else if (FrameRate>23.988*2 && FrameRate<=24.024*2) FrameRate=24.000*2;
    else if (FrameRate>24.975*2 && FrameRate<=25.025*2) FrameRate=25.000*2;
    else if (FrameRate>29.940*2 && FrameRate<=29.985*2) FrameRate=29.970*2;
    else if (FrameRate>29.970*2 && FrameRate<=30.030*2) FrameRate=30.000*2;

    if (FrameRate!=FrameRate_Sav)
        Fill(Stream_Video, Pos, Parameter, FrameRate, 3, true);
}

//---------------------------------------------------------------------------
void File__Analyze::Video_BitRate_Rounding(size_t Pos, video Parameter)
{
    const Ztring& Format=Retrieve(Stream_Video, Pos, Video_Format);
    int32u BitRate=Retrieve(Stream_Video, Pos, Parameter).To_int32u();
    int32u BitRate_Sav=BitRate;
    if (Format==__T("AVC"))
    {
        if (BitRate>= 54942720 && BitRate<= 57185280) BitRate= 56064000; //AVC-INTRA50
        if (BitRate>=111390720 && BitRate<=115937280) BitRate=113664000; //AVC-INTRA100
    }

    if (BitRate!=BitRate_Sav)
        Fill(Stream_Video, Pos, Parameter, BitRate, 0, true);
}

//---------------------------------------------------------------------------
void File__Analyze::Audio_BitRate_Rounding(size_t Pos, audio Parameter)
{
    const Ztring& Format=Retrieve(Stream_Audio, Pos, Audio_Format);
    const Ztring& Codec=Retrieve(Stream_Audio, Pos, Audio_Codec);
    int32u BitRate=Retrieve(Stream_Audio, Pos, Parameter).To_int32u();
    int32u BitRate_Sav=BitRate;
    if (MediaInfoLib::Config.Codec_Get(Codec, InfoCodec_KindofCodec, Stream_Audio).find(__T("MPEG-"))==0
     || Retrieve(Stream_Audio, Pos, Audio_Codec_String).find(__T("MPEG-"))==0)
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
        if (Retrieve(Stream_Audio, Pos, "BitRate_Mode")==__T("VBR"))
            BitRate=BitRate_Sav; //If VBR, we want the exact value
    }

    else if (MediaInfoLib::Config.Codec_Get(Codec, InfoCodec_Name, Stream_Audio).find(__T("AC3"))==0)
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

    else if (MediaInfoLib::Config.Codec_Get(Codec, InfoCodec_Name, Stream_Audio).find(__T("DTS"))==0)
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

    else if (Codec.find(__T("AAC"))==0 || MediaInfoLib::Config.Codec_Get(Codec, InfoCodec_Name, Stream_Audio).find(__T("AAC"))==0)
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

    else if (Codec==__T("PCM") || Codec==__T("QDM2") || MediaInfoLib::Config.Codec_Get(Codec, InfoCodec_Name, Stream_Audio).find(__T("PCM"))==0)
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

    else if (MediaInfoLib::Config.Codec_Get(Codec, InfoCodec_Name, Stream_Audio).find(__T("ADPCM"))==0
          || MediaInfoLib::Config.Codec_Get(Codec, InfoCodec_Name, Stream_Audio).find(__T("U-Law"))==0
          || MediaInfoLib::Config.Codec_Get(Codec, InfoCodec_KindofCodec, Stream_Audio)==__T("ADPCM")
          || MediaInfoLib::Config.Codec_Get(Codec, InfoCodec_KindofCodec, Stream_Audio)==__T("U-Law")
          || Format==__T("ADPCM"))
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
    if (!Retrieve(Stream_General, 0, General_Genre).empty() && Retrieve(Stream_General, 0, General_Genre).size()<4 && Retrieve(Stream_General, 0, General_Genre)[0]>=__T('0') && Retrieve(Stream_General, 0, General_Genre)[0]<=__T('9'))
    {
        Ztring Genre;
        if (Retrieve(Stream_General, 0, General_Genre).size()==1) Genre=Ztring(__T("Genre_00"))+Retrieve(Stream_General, 0, General_Genre);
        if (Retrieve(Stream_General, 0, General_Genre).size()==2) Genre=Ztring(__T("Genre_0" ))+Retrieve(Stream_General, 0, General_Genre);
        if (Retrieve(Stream_General, 0, General_Genre).size()==3) Genre=Ztring(__T("Genre_"  ))+Retrieve(Stream_General, 0, General_Genre);
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

    //Clearing old data
    Clear(StreamKind, StreamPos, Parameter+1);
    Clear(StreamKind, StreamPos, Parameter+2);
    Clear(StreamKind, StreamPos, Parameter+3);
    Clear(StreamKind, StreamPos, Parameter+4);

    //Retrieving multiple values
    ZtringList List;
    List.Separator_Set(0, __T(" / "));
    List.Write(Retrieve(StreamKind, StreamPos, Parameter));

    //Per value
    for (size_t Pos=0; Pos<List.size(); Pos++)
    {
        int32s HH, MM, Sec, MS;
        Ztring DurationString1, DurationString2, DurationString3;
        bool Negative=false;
        MS=List[Pos].To_int32s(); //in ms

        if (MS<0)
        {
            Negative=true;
            MS=-MS;
        }

        //Hours
        HH=MS/1000/60/60; //h
        if (HH>0)
        {
            DurationString1+=Ztring::ToZtring(HH)+MediaInfoLib::Config.Language_Get(__T("h"));
            DurationString2+=Ztring::ToZtring(HH)+MediaInfoLib::Config.Language_Get(__T("h"));
            if (HH<10)
                DurationString3+=Ztring(__T("0"))+Ztring::ToZtring(HH)+__T(":");
            else
                DurationString3+=Ztring::ToZtring(HH)+__T(":");
            MS-=HH*60*60*1000;
        }
        else
        {
            DurationString3+=__T("00:");
        }

        //Minutes
        MM=MS/1000/60; //mn
        if (MM>0 || HH>0)
        {
            if (DurationString1.size()>0)
                DurationString1+=__T(" ");
            DurationString1+=Ztring::ToZtring(MM)+MediaInfoLib::Config.Language_Get(__T("mn"));
            if (DurationString2.size()<5)
            {
                if (DurationString2.size()>0)
                    DurationString2+=__T(" ");
                DurationString2+=Ztring::ToZtring(MM)+MediaInfoLib::Config.Language_Get(__T("mn"));
            }
            if (MM<10)
                DurationString3+=Ztring(__T("0"))+Ztring::ToZtring(MM)+__T(":");
            else
                DurationString3+=Ztring::ToZtring(MM)+__T(":");
            MS-=MM*60*1000;
        }
        else
        {
            DurationString3+=__T("00:");
        }

        //Seconds
        Sec=MS/1000; //s
        if (Sec>0 || MM>0 || HH>0)
        {
            if (DurationString1.size()>0)
                DurationString1+=__T(" ");
            DurationString1+=Ztring::ToZtring(Sec)+MediaInfoLib::Config.Language_Get(__T("s"));
            if (DurationString2.size()<5)
            {
                if (DurationString2.size()>0)
                    DurationString2+=__T(" ");
                DurationString2+=Ztring::ToZtring(Sec)+MediaInfoLib::Config.Language_Get(__T("s"));
            }
            else if (DurationString2.size()==0)
                DurationString2+=Ztring::ToZtring(Sec)+MediaInfoLib::Config.Language_Get(__T("s"));
            if (Sec<10)
                DurationString3+=Ztring(__T("0"))+Ztring::ToZtring(Sec)+__T(".");
            else
                DurationString3+=Ztring::ToZtring(Sec)+__T(".");
            MS-=Sec*1000;
        }
        else
        {
            DurationString3+=__T("00.");
        }

        //Milliseconds
        if (MS>0 || Sec>0 || MM>0 || HH>0)
        {
            if (DurationString1.size()>0)
                DurationString1+=__T(" ");
            DurationString1+=Ztring::ToZtring(MS)+MediaInfoLib::Config.Language_Get(__T("ms"));
            if (DurationString2.size()<5)
            {
                if (DurationString2.size()>0)
                    DurationString2+=__T(" ");
                DurationString2+=Ztring::ToZtring(MS)+MediaInfoLib::Config.Language_Get(__T("ms"));
            }
            if (MS<10)
                DurationString3+=Ztring(__T("00"))+Ztring::ToZtring(MS);
            else if (MS<100)
                DurationString3+=Ztring(__T("0"))+Ztring::ToZtring(MS);
            else
                DurationString3+=Ztring::ToZtring(MS);
        }
        else
        {
            DurationString3+=__T("000");
        }

        if (Negative)
        {
            DurationString1=Ztring(__T("-"))+DurationString1;
            DurationString2=Ztring(__T("-"))+DurationString2;
            DurationString3=Ztring(__T("-"))+DurationString3;
        }

        Fill(StreamKind, StreamPos, Parameter+1, DurationString2); // /String
        Fill(StreamKind, StreamPos, Parameter+2, DurationString1); // /String1
        Fill(StreamKind, StreamPos, Parameter+3, DurationString2); // /String2
        Fill(StreamKind, StreamPos, Parameter+4, DurationString3); // /String3
    }
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
        case  0 : Measure=__T(" Byte"); MeasureIsAlwaysSame=false; break;
        case  1 : Measure=__T(" KiB");  MeasureIsAlwaysSame=true;  break;
        case  2 : Measure=__T(" MiB");  MeasureIsAlwaysSame=true;  break;
        case  3 : Measure=__T(" GiB");  MeasureIsAlwaysSame=true;  break;
        default : Measure=__T(" ?iB");  MeasureIsAlwaysSame=true;
    }
    Fill(StreamKind, StreamPos, Parameter+2, MediaInfoLib::Config.Language_Get(Ztring::ToZtring(F1,  0), Measure, MeasureIsAlwaysSame), true); // /String1
    Fill(StreamKind, StreamPos, Parameter+3, MediaInfoLib::Config.Language_Get(Ztring::ToZtring(F1, I2), Measure, MeasureIsAlwaysSame), true); // /String2
    Fill(StreamKind, StreamPos, Parameter+4, MediaInfoLib::Config.Language_Get(Ztring::ToZtring(F1, I3), Measure, MeasureIsAlwaysSame), true); // /String3
    Fill(StreamKind, StreamPos, Parameter+5, MediaInfoLib::Config.Language_Get(Ztring::ToZtring(F1, I4), Measure, MeasureIsAlwaysSame), true); // /String4
    float64 F2=(float)Retrieve(StreamKind, StreamPos, Parameter).To_float64();
    float64 File_Size_WithReferencedFiles=(float)Retrieve(Stream_General, 0, General_FileSize).To_float64();
    if (File_Size_WithReferencedFiles>0 && Parameter==Fill_Parameter(StreamKind, Generic_StreamSize) && F2*100/File_Size_WithReferencedFiles<=100)
    {
        Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_StreamSize_Proportion), F2/File_Size_WithReferencedFiles, 5, true);
        Fill(StreamKind, StreamPos, Parameter+6, MediaInfoLib::Config.Language_Get(Ztring::ToZtring(F1, I3), Measure, MeasureIsAlwaysSame)+__T(" (")+Ztring::ToZtring(F2*100/File_Size_WithReferencedFiles, 0)+__T("%)"), true); // /String5
        Fill(StreamKind, StreamPos, Parameter+1,  MediaInfoLib::Config.Language_Get(Ztring::ToZtring(F1, I3), Measure, MeasureIsAlwaysSame)+__T(" (")+Ztring::ToZtring(F2*100/File_Size_WithReferencedFiles, 0)+__T("%)"), true);
    }
    else if (File_Size_WithReferencedFiles>0 && Parameter==Fill_Parameter(StreamKind, Generic_StreamSize_Encoded) && F2*100/File_Size_WithReferencedFiles<=100)
    {
        Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_StreamSize_Encoded_Proportion), F2/File_Size_WithReferencedFiles, 5, true);
        Fill(StreamKind, StreamPos, Parameter+6, MediaInfoLib::Config.Language_Get(Ztring::ToZtring(F1, I3), Measure, MeasureIsAlwaysSame)+__T(" (")+Ztring::ToZtring(F2*100/File_Size_WithReferencedFiles, 0)+__T("%)"), true); // /String5
        Fill(StreamKind, StreamPos, Parameter+1,  MediaInfoLib::Config.Language_Get(Ztring::ToZtring(F1, I3), Measure, MeasureIsAlwaysSame)+__T(" (")+Ztring::ToZtring(F2*100/File_Size_WithReferencedFiles, 0)+__T("%)"), true);
    }
    else if (File_Size_WithReferencedFiles>0 && Parameter==Fill_Parameter(StreamKind, Generic_Source_StreamSize) && F2*100/File_Size_WithReferencedFiles<=100)
    {
        Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Source_StreamSize_Proportion), F2/File_Size_WithReferencedFiles, 5, true);
        Fill(StreamKind, StreamPos, Parameter+6, MediaInfoLib::Config.Language_Get(Ztring::ToZtring(F1, I3), Measure, MeasureIsAlwaysSame)+__T(" (")+Ztring::ToZtring(F2*100/File_Size_WithReferencedFiles, 0)+__T("%)"), true); // /String5
        Fill(StreamKind, StreamPos, Parameter+1,  MediaInfoLib::Config.Language_Get(Ztring::ToZtring(F1, I3), Measure, MeasureIsAlwaysSame)+__T(" (")+Ztring::ToZtring(F2*100/File_Size_WithReferencedFiles, 0)+__T("%)"), true);
    }
    else if (File_Size_WithReferencedFiles>0 && Parameter==Fill_Parameter(StreamKind, Generic_Source_StreamSize_Encoded) && F2*100/File_Size_WithReferencedFiles<=100)
    {
        Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Source_StreamSize_Encoded_Proportion), F2/File_Size_WithReferencedFiles, 5, true);
        Fill(StreamKind, StreamPos, Parameter+6, MediaInfoLib::Config.Language_Get(Ztring::ToZtring(F1, I3), Measure, MeasureIsAlwaysSame)+__T(" (")+Ztring::ToZtring(F2*100/File_Size_WithReferencedFiles, 0)+__T("%)"), true); // /String5
        Fill(StreamKind, StreamPos, Parameter+1,  MediaInfoLib::Config.Language_Get(Ztring::ToZtring(F1, I3), Measure, MeasureIsAlwaysSame)+__T(" (")+Ztring::ToZtring(F2*100/File_Size_WithReferencedFiles, 0)+__T("%)"), true);
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
    List.Separator_Set(0, __T(" / "));
    List.Write(Retrieve(StreamKind, StreamPos, Parameter));

    //Per value
    for (size_t Pos=0; Pos<List.size(); Pos++)
    {
        int64u BitRate=List[Pos].To_int64u();

        //Text
        if (BitRate==0 && (List[Pos].empty() || List[Pos][0]>__T('9')))
        {
            Fill(StreamKind, StreamPos, Parameter+1, MediaInfoLib::Config.Language_Get(List[Pos]));
        }
        else
        {
            //Well known values
            Ztring BitRateS;
            if (StreamKind==Stream_Audio)
            {
                if (Parameter==Audio_BitRate
                 && (Retrieve(Stream_Audio, StreamPos, Audio_Format)==__T("PCM")
                  || Retrieve(Stream_Audio, StreamPos, Audio_Format)==__T("ADPCM")
                  || Retrieve(Stream_Audio, StreamPos, Audio_Format)==__T("U-Law")
                  || Retrieve(Stream_Audio, StreamPos, Audio_Format)==__T("Qdesign 1")
                  || Retrieve(Stream_Audio, StreamPos, Audio_Format)==__T("Qdesign 2")
                  || Retrieve(Stream_Audio, StreamPos, Audio_Format)==__T("DTS")))
                {
                    if (BitRate==  66150) BitRateS=  "66.15";
                    if (BitRate== 132300) BitRateS= "132.3";
                    if (BitRate== 176400) BitRateS= "176.4";
                    if (BitRate== 264600) BitRateS= "264.6";
                    if (BitRate== 352800) BitRateS= "352.8";
                    if (BitRate== 529200) BitRateS= "529.2";
                    if (BitRate== 705600) BitRateS= "705.6";
                    if (BitRate==1411200) BitRateS="1411.2";
                }
                if (Parameter==Audio_SamplingRate)
                {
                    if (BitRate==  11024) BitRateS=  "11.024";
                    if (BitRate==  11025) BitRateS=  "11.025";
                    if (BitRate==  22050) BitRateS=  "22.05";
                    if (BitRate==  44100) BitRateS=  "44.1";
                    if (BitRate==  88200) BitRateS=  "88.2";
                    if (BitRate== 176400) BitRateS= "176.4";
                    if (BitRate== 352800) BitRateS= "352.8";
                }
            }
            if (!BitRateS.empty())
            {
                Ztring Measure=MediaInfoLib::Config.Info_Get(StreamKind).Read(Parameter, Info_Measure);
                Measure.insert(1, __T("K"));
                Fill(StreamKind, StreamPos, Parameter+1, MediaInfoLib::Config.Language_Get(BitRateS, Measure, true));
            }
            else
            {
                //Standard
                if (BitRate>10000000000LL)
                {
                    Ztring Measure=MediaInfoLib::Config.Info_Get(StreamKind).Read(Parameter, Info_Measure);
                    Measure.insert(1, __T("G"));
                    Fill(StreamKind, StreamPos, Parameter+1, MediaInfoLib::Config.Language_Get(Ztring::ToZtring(((float)BitRate)/1000000000, BitRate>100000000000LL?0:1), Measure, true));
                }
                else if (BitRate>10000000)
                {
                    Ztring Measure=MediaInfoLib::Config.Info_Get(StreamKind).Read(Parameter, Info_Measure);
                    Measure.insert(1, __T("M"));
                    Fill(StreamKind, StreamPos, Parameter+1, MediaInfoLib::Config.Language_Get(Ztring::ToZtring(((float)BitRate)/1000000, BitRate>100000000?0:1), Measure, true));
                }
                else if (BitRate>10000)
                {
                    Ztring Measure=MediaInfoLib::Config.Info_Get(StreamKind).Read(Parameter, Info_Measure);
                    Measure.insert(1, __T("K"));
                    Fill(StreamKind, StreamPos, Parameter+1, MediaInfoLib::Config.Language_Get(Ztring::ToZtring(((float)BitRate)/1000, BitRate>100000?0:1), Measure, true));
                }
                else
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
    List.Separator_Set(0, __T(" / "));
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
    Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_CodecID), Value);
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
    if (Retrieve(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Compression_Mode)).empty())
        Fill(StreamKind, StreamPos, Fill_Parameter(StreamKind, Generic_Compression_Mode), MediaInfoLib::Config.CodecID_Get(StreamKind, Format, Value, InfoCodecID_Compression_Mode), true);

    //Specific cases
    if (Value==__T("v210") || Value==__T("V210"))
        Fill(Stream_Video, StreamPos, Video_BitDepth, 10);
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
                                    case Generic_ServiceName : return General_ServiceName;
                                    case Generic_ServiceProvider : return General_ServiceProvider;
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
                                    case Generic_Source_Duration : return Video_Source_Duration;
                                    case Generic_Source_Duration_String : return Video_Source_Duration_String;
                                    case Generic_Source_Duration_String1 : return Video_Source_Duration_String1;
                                    case Generic_Source_Duration_String2 : return Video_Source_Duration_String2;
                                    case Generic_Source_Duration_String3 : return Video_Source_Duration_String3;
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
                                    case Generic_BitRate_Encoded : return Video_BitRate_Encoded;
                                    case Generic_BitRate_Encoded_String : return Video_BitRate_Encoded_String;
                                    case Generic_FrameCount : return Video_FrameCount;
                                    case Generic_Source_FrameCount : return Video_Source_FrameCount;
                                    case Generic_ColorSpace : return Video_ColorSpace;
                                    case Generic_ChromaSubsampling : return Video_ChromaSubsampling;
                                    case Generic_Resolution : return Video_Resolution;
                                    case Generic_Resolution_String : return Video_Resolution_String;
                                    case Generic_BitDepth : return Video_BitDepth;
                                    case Generic_BitDepth_String : return Video_BitDepth_String;
                                    case Generic_Compression_Mode : return Video_Compression_Mode;
                                    case Generic_Compression_Mode_String : return Video_Compression_Mode_String;
                                    case Generic_Compression_Ratio : return Video_Compression_Ratio;
                                    case Generic_Delay : return Video_Delay;
                                    case Generic_Delay_String : return Video_Delay_String;
                                    case Generic_Delay_String1 : return Video_Delay_String1;
                                    case Generic_Delay_String2 : return Video_Delay_String2;
                                    case Generic_Delay_String3 : return Video_Delay_String3;
                                    case Generic_Delay_String4 : return Video_Delay_String4;
                                    case Generic_Delay_Settings : return Video_Delay_Settings;
                                    case Generic_Delay_DropFrame : return Video_Delay_DropFrame;
                                    case Generic_Delay_Source : return Video_Delay_Source;
                                    case Generic_Delay_Source_String : return Video_Delay_Source_String;
                                    case Generic_Delay_Original : return Video_Delay_Original;
                                    case Generic_Delay_Original_String : return Video_Delay_Original_String;
                                    case Generic_Delay_Original_String1 : return Video_Delay_Original_String1;
                                    case Generic_Delay_Original_String2 : return Video_Delay_Original_String2;
                                    case Generic_Delay_Original_String3 : return Video_Delay_Original_String3;
                                    case Generic_Delay_Original_String4 : return Video_Delay_Original_String4;
                                    case Generic_Delay_Original_Settings : return Video_Delay_Original_Settings;
                                    case Generic_Delay_Original_DropFrame : return Video_Delay_Original_DropFrame;
                                    case Generic_Delay_Original_Source : return Video_Delay_Original_Source;
                                    case Generic_StreamSize : return Video_StreamSize;
                                    case Generic_StreamSize_String : return Video_StreamSize_String;
                                    case Generic_StreamSize_String1 : return Video_StreamSize_String1;
                                    case Generic_StreamSize_String2 : return Video_StreamSize_String2;
                                    case Generic_StreamSize_String3 : return Video_StreamSize_String3;
                                    case Generic_StreamSize_String4 : return Video_StreamSize_String4;
                                    case Generic_StreamSize_String5 : return Video_StreamSize_String5;
                                    case Generic_StreamSize_Proportion : return Video_StreamSize_Proportion;
                                    case Generic_StreamSize_Encoded : return Video_StreamSize_Encoded;
                                    case Generic_StreamSize_Encoded_String : return Video_StreamSize_Encoded_String;
                                    case Generic_StreamSize_Encoded_String1 : return Video_StreamSize_Encoded_String1;
                                    case Generic_StreamSize_Encoded_String2 : return Video_StreamSize_Encoded_String2;
                                    case Generic_StreamSize_Encoded_String3 : return Video_StreamSize_Encoded_String3;
                                    case Generic_StreamSize_Encoded_String4 : return Video_StreamSize_Encoded_String4;
                                    case Generic_StreamSize_Encoded_String5 : return Video_StreamSize_Encoded_String5;
                                    case Generic_StreamSize_Encoded_Proportion : return Video_StreamSize_Encoded_Proportion;
                                    case Generic_Source_StreamSize : return Video_Source_StreamSize;
                                    case Generic_Source_StreamSize_String : return Video_Source_StreamSize_String;
                                    case Generic_Source_StreamSize_String1 : return Video_Source_StreamSize_String1;
                                    case Generic_Source_StreamSize_String2 : return Video_Source_StreamSize_String2;
                                    case Generic_Source_StreamSize_String3 : return Video_Source_StreamSize_String3;
                                    case Generic_Source_StreamSize_String4 : return Video_Source_StreamSize_String4;
                                    case Generic_Source_StreamSize_String5 : return Video_Source_StreamSize_String5;
                                    case Generic_Source_StreamSize_Proportion : return Video_Source_StreamSize_Proportion;
                                    case Generic_Source_StreamSize_Encoded : return Video_Source_StreamSize_Encoded;
                                    case Generic_Source_StreamSize_Encoded_String : return Video_Source_StreamSize_Encoded_String;
                                    case Generic_Source_StreamSize_Encoded_String1 : return Video_Source_StreamSize_Encoded_String1;
                                    case Generic_Source_StreamSize_Encoded_String2 : return Video_Source_StreamSize_Encoded_String2;
                                    case Generic_Source_StreamSize_Encoded_String3 : return Video_Source_StreamSize_Encoded_String3;
                                    case Generic_Source_StreamSize_Encoded_String4 : return Video_Source_StreamSize_Encoded_String4;
                                    case Generic_Source_StreamSize_Encoded_String5 : return Video_Source_StreamSize_Encoded_String5;
                                    case Generic_Source_StreamSize_Encoded_Proportion : return Video_Source_StreamSize_Encoded_Proportion;
                                    case Generic_Language : return Video_Language;
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
                                    case Generic_Source_Duration : return Audio_Source_Duration;
                                    case Generic_Source_Duration_String : return Audio_Source_Duration_String;
                                    case Generic_Source_Duration_String1 : return Audio_Source_Duration_String1;
                                    case Generic_Source_Duration_String2 : return Audio_Source_Duration_String2;
                                    case Generic_Source_Duration_String3 : return Audio_Source_Duration_String3;
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
                                    case Generic_BitRate_Encoded : return Audio_BitRate_Encoded;
                                    case Generic_BitRate_Encoded_String : return Audio_BitRate_Encoded_String;
                                    case Generic_FrameCount : return Audio_FrameCount;
                                    case Generic_Source_FrameCount : return Audio_Source_FrameCount;
                                    case Generic_Resolution : return Audio_Resolution;
                                    case Generic_Resolution_String : return Audio_Resolution_String;
                                    case Generic_BitDepth : return Audio_BitDepth;
                                    case Generic_BitDepth_String : return Audio_BitDepth_String;
                                    case Generic_Compression_Mode : return Audio_Compression_Mode;
                                    case Generic_Compression_Mode_String : return Audio_Compression_Mode_String;
                                    case Generic_Compression_Ratio : return Audio_Compression_Ratio;
                                    case Generic_Delay : return Audio_Delay;
                                    case Generic_Delay_String : return Audio_Delay_String;
                                    case Generic_Delay_String1 : return Audio_Delay_String1;
                                    case Generic_Delay_String2 : return Audio_Delay_String2;
                                    case Generic_Delay_String3 : return Audio_Delay_String3;
                                    case Generic_Delay_String4 : return Audio_Delay_String4;
                                    case Generic_Delay_Settings : return Audio_Delay_Settings;
                                    case Generic_Delay_DropFrame : return Audio_Delay_DropFrame;
                                    case Generic_Delay_Source : return Audio_Delay_Source;
                                    case Generic_Delay_Source_String : return Audio_Delay_Source_String;
                                    case Generic_Delay_Original : return Audio_Delay_Original;
                                    case Generic_Delay_Original_String : return Audio_Delay_Original_String;
                                    case Generic_Delay_Original_String1 : return Audio_Delay_Original_String1;
                                    case Generic_Delay_Original_String2 : return Audio_Delay_Original_String2;
                                    case Generic_Delay_Original_String3 : return Audio_Delay_Original_String3;
                                    case Generic_Delay_Original_String4 : return Audio_Delay_Original_String4;
                                    case Generic_Delay_Original_Settings : return Audio_Delay_Original_Settings;
                                    case Generic_Delay_Original_DropFrame : return Audio_Delay_Original_DropFrame;
                                    case Generic_Delay_Original_Source : return Audio_Delay_Original_Source;
                                    case Generic_Video_Delay : return Audio_Video_Delay;
                                    case Generic_Video_Delay_String : return Audio_Video_Delay_String;
                                    case Generic_Video_Delay_String1 : return Audio_Video_Delay_String1;
                                    case Generic_Video_Delay_String2 : return Audio_Video_Delay_String2;
                                    case Generic_Video_Delay_String3 : return Audio_Video_Delay_String3;
                                    case Generic_Video_Delay_String4 : return Audio_Video_Delay_String4;
                                    case Generic_StreamSize : return Audio_StreamSize;
                                    case Generic_StreamSize_String : return Audio_StreamSize_String;
                                    case Generic_StreamSize_String1 : return Audio_StreamSize_String1;
                                    case Generic_StreamSize_String2 : return Audio_StreamSize_String2;
                                    case Generic_StreamSize_String3 : return Audio_StreamSize_String3;
                                    case Generic_StreamSize_String4 : return Audio_StreamSize_String4;
                                    case Generic_StreamSize_String5 : return Audio_StreamSize_String5;
                                    case Generic_StreamSize_Proportion : return Audio_StreamSize_Proportion;
                                    case Generic_StreamSize_Encoded : return Audio_StreamSize_Encoded;
                                    case Generic_StreamSize_Encoded_String : return Audio_StreamSize_Encoded_String;
                                    case Generic_StreamSize_Encoded_String1 : return Audio_StreamSize_Encoded_String1;
                                    case Generic_StreamSize_Encoded_String2 : return Audio_StreamSize_Encoded_String2;
                                    case Generic_StreamSize_Encoded_String3 : return Audio_StreamSize_Encoded_String3;
                                    case Generic_StreamSize_Encoded_String4 : return Audio_StreamSize_Encoded_String4;
                                    case Generic_StreamSize_Encoded_String5 : return Audio_StreamSize_Encoded_String5;
                                    case Generic_StreamSize_Encoded_Proportion : return Audio_StreamSize_Encoded_Proportion;
                                    case Generic_Source_StreamSize : return Audio_Source_StreamSize;
                                    case Generic_Source_StreamSize_String : return Audio_Source_StreamSize_String;
                                    case Generic_Source_StreamSize_String1 : return Audio_Source_StreamSize_String1;
                                    case Generic_Source_StreamSize_String2 : return Audio_Source_StreamSize_String2;
                                    case Generic_Source_StreamSize_String3 : return Audio_Source_StreamSize_String3;
                                    case Generic_Source_StreamSize_String4 : return Audio_Source_StreamSize_String4;
                                    case Generic_Source_StreamSize_String5 : return Audio_Source_StreamSize_String5;
                                    case Generic_Source_StreamSize_Proportion : return Audio_Source_StreamSize_Proportion;
                                    case Generic_Source_StreamSize_Encoded : return Audio_Source_StreamSize_Encoded;
                                    case Generic_Source_StreamSize_Encoded_String : return Audio_Source_StreamSize_Encoded_String;
                                    case Generic_Source_StreamSize_Encoded_String1 : return Audio_Source_StreamSize_Encoded_String1;
                                    case Generic_Source_StreamSize_Encoded_String2 : return Audio_Source_StreamSize_Encoded_String2;
                                    case Generic_Source_StreamSize_Encoded_String3 : return Audio_Source_StreamSize_Encoded_String3;
                                    case Generic_Source_StreamSize_Encoded_String4 : return Audio_Source_StreamSize_Encoded_String4;
                                    case Generic_Source_StreamSize_Encoded_String5 : return Audio_Source_StreamSize_Encoded_String5;
                                    case Generic_Source_StreamSize_Encoded_Proportion : return Audio_Source_StreamSize_Encoded_Proportion;
                                    case Generic_Language : return Audio_Language;
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
                                    case Generic_Source_Duration : return Text_Source_Duration;
                                    case Generic_Source_Duration_String : return Text_Source_Duration_String;
                                    case Generic_Source_Duration_String1 : return Text_Source_Duration_String1;
                                    case Generic_Source_Duration_String2 : return Text_Source_Duration_String2;
                                    case Generic_Source_Duration_String3 : return Text_Source_Duration_String3;
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
                                    case Generic_BitRate_Encoded : return Text_BitRate_Encoded;
                                    case Generic_BitRate_Encoded_String : return Text_BitRate_Encoded_String;
                                    case Generic_FrameCount : return Text_FrameCount;
                                    case Generic_Source_FrameCount : return Text_Source_FrameCount;
                                    case Generic_ColorSpace : return Text_ColorSpace;
                                    case Generic_ChromaSubsampling : return Text_ChromaSubsampling;
                                    case Generic_Resolution : return Text_Resolution;
                                    case Generic_Resolution_String : return Text_Resolution_String;
                                    case Generic_BitDepth : return Text_BitDepth;
                                    case Generic_BitDepth_String : return Text_BitDepth_String;
                                    case Generic_Compression_Mode : return Text_Compression_Mode;
                                    case Generic_Compression_Mode_String : return Text_Compression_Mode_String;
                                    case Generic_Compression_Ratio : return Text_Compression_Ratio;
                                    case Generic_Delay : return Text_Delay;
                                    case Generic_Delay_String : return Text_Delay_String;
                                    case Generic_Delay_String1 : return Text_Delay_String1;
                                    case Generic_Delay_String2 : return Text_Delay_String2;
                                    case Generic_Delay_String3 : return Text_Delay_String3;
                                    case Generic_Delay_String4 : return Text_Delay_String4;
                                    case Generic_Delay_Settings : return Text_Delay_Settings;
                                    case Generic_Delay_DropFrame : return Text_Delay_DropFrame;
                                    case Generic_Delay_Source : return Text_Delay_Source;
                                    case Generic_Delay_Source_String : return Text_Delay_Source_String;
                                    case Generic_Delay_Original : return Text_Delay_Original;
                                    case Generic_Delay_Original_String : return Text_Delay_Original_String;
                                    case Generic_Delay_Original_String1 : return Text_Delay_Original_String1;
                                    case Generic_Delay_Original_String2 : return Text_Delay_Original_String2;
                                    case Generic_Delay_Original_String3 : return Text_Delay_Original_String3;
                                    case Generic_Delay_Original_String4 : return Text_Delay_Original_String4;
                                    case Generic_Delay_Original_Settings : return Text_Delay_Original_Settings;
                                    case Generic_Delay_Original_DropFrame : return Text_Delay_Original_DropFrame;
                                    case Generic_Delay_Original_Source : return Text_Delay_Original_Source;
                                    case Generic_Video_Delay : return Text_Video_Delay;
                                    case Generic_Video_Delay_String : return Text_Video_Delay_String;
                                    case Generic_Video_Delay_String1 : return Text_Video_Delay_String1;
                                    case Generic_Video_Delay_String2 : return Text_Video_Delay_String2;
                                    case Generic_Video_Delay_String3 : return Text_Video_Delay_String3;
                                    case Generic_Video_Delay_String4 : return Text_Video_Delay_String4;
                                    case Generic_StreamSize : return Text_StreamSize;
                                    case Generic_StreamSize_String : return Text_StreamSize_String;
                                    case Generic_StreamSize_String1 : return Text_StreamSize_String1;
                                    case Generic_StreamSize_String2 : return Text_StreamSize_String2;
                                    case Generic_StreamSize_String3 : return Text_StreamSize_String3;
                                    case Generic_StreamSize_String4 : return Text_StreamSize_String4;
                                    case Generic_StreamSize_String5 : return Text_StreamSize_String5;
                                    case Generic_StreamSize_Proportion : return Text_StreamSize_Proportion;
                                    case Generic_StreamSize_Encoded : return Text_StreamSize_Encoded;
                                    case Generic_StreamSize_Encoded_String : return Text_StreamSize_Encoded_String;
                                    case Generic_StreamSize_Encoded_String1 : return Text_StreamSize_Encoded_String1;
                                    case Generic_StreamSize_Encoded_String2 : return Text_StreamSize_Encoded_String2;
                                    case Generic_StreamSize_Encoded_String3 : return Text_StreamSize_Encoded_String3;
                                    case Generic_StreamSize_Encoded_String4 : return Text_StreamSize_Encoded_String4;
                                    case Generic_StreamSize_Encoded_String5 : return Text_StreamSize_Encoded_String5;
                                    case Generic_StreamSize_Encoded_Proportion : return Text_StreamSize_Encoded_Proportion;
                                    case Generic_Source_StreamSize : return Text_Source_StreamSize;
                                    case Generic_Source_StreamSize_String : return Text_Source_StreamSize_String;
                                    case Generic_Source_StreamSize_String1 : return Text_Source_StreamSize_String1;
                                    case Generic_Source_StreamSize_String2 : return Text_Source_StreamSize_String2;
                                    case Generic_Source_StreamSize_String3 : return Text_Source_StreamSize_String3;
                                    case Generic_Source_StreamSize_String4 : return Text_Source_StreamSize_String4;
                                    case Generic_Source_StreamSize_String5 : return Text_Source_StreamSize_String5;
                                    case Generic_Source_StreamSize_Proportion : return Text_Source_StreamSize_Proportion;
                                    case Generic_Source_StreamSize_Encoded : return Text_Source_StreamSize_Encoded;
                                    case Generic_Source_StreamSize_Encoded_String : return Text_Source_StreamSize_Encoded_String;
                                    case Generic_Source_StreamSize_Encoded_String1 : return Text_Source_StreamSize_Encoded_String1;
                                    case Generic_Source_StreamSize_Encoded_String2 : return Text_Source_StreamSize_Encoded_String2;
                                    case Generic_Source_StreamSize_Encoded_String3 : return Text_Source_StreamSize_Encoded_String3;
                                    case Generic_Source_StreamSize_Encoded_String4 : return Text_Source_StreamSize_Encoded_String4;
                                    case Generic_Source_StreamSize_Encoded_String5 : return Text_Source_StreamSize_Encoded_String5;
                                    case Generic_Source_StreamSize_Encoded_Proportion : return Text_Source_StreamSize_Encoded_Proportion;
                                    case Generic_Language : return Text_Language;
                                    default: return (size_t)-1;
                                }
        case Stream_Other :
                                switch (StreamPos)
                                {
                                    case Generic_Format : return Other_Format;
                                    case Generic_Format_Info : return Other_Format_Info;
                                    case Generic_Format_Url : return Other_Format_Url;
                                    case Generic_Format_Commercial : return Other_Format_Commercial;
                                    case Generic_Format_Commercial_IfAny : return Other_Format_Commercial_IfAny;
                                    case Generic_Format_Version : return Other_Format_Version;
                                    case Generic_Format_Profile : return Other_Format_Profile;
                                    case Generic_Format_Settings : return Other_Format_Settings;
                                    case Generic_CodecID : return Other_CodecID;
                                    case Generic_CodecID_Info : return Other_CodecID_Info;
                                    case Generic_CodecID_Hint : return Other_CodecID_Hint;
                                    case Generic_CodecID_Url : return Other_CodecID_Url;
                                    case Generic_CodecID_Description : return Other_CodecID_Description;
                                    case Generic_Duration : return Other_Duration;
                                    case Generic_Duration_String : return Other_Duration_String;
                                    case Generic_Duration_String1 : return Other_Duration_String1;
                                    case Generic_Duration_String2 : return Other_Duration_String2;
                                    case Generic_Duration_String3 : return Other_Duration_String3;
                                    case Generic_FrameCount : return Other_FrameCount;
                                    case Generic_Language : return Other_Language;
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
                                    case Generic_Compression_Mode : return Image_Compression_Mode;
                                    case Generic_Compression_Mode_String : return Image_Compression_Mode_String;
                                    case Generic_Compression_Ratio : return Image_Compression_Ratio;
                                    case Generic_StreamSize : return Image_StreamSize;
                                    case Generic_StreamSize_String : return Image_StreamSize_String;
                                    case Generic_StreamSize_String1 : return Image_StreamSize_String1;
                                    case Generic_StreamSize_String2 : return Image_StreamSize_String2;
                                    case Generic_StreamSize_String3 : return Image_StreamSize_String3;
                                    case Generic_StreamSize_String4 : return Image_StreamSize_String4;
                                    case Generic_StreamSize_String5 : return Image_StreamSize_String5;
                                    case Generic_StreamSize_Proportion : return Image_StreamSize_Proportion;
                                    case Generic_Language : return Image_Language;
                                    default: return (size_t)-1;
                                }
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
                                    case Generic_Language : return Menu_Language;
                                    case Generic_ServiceName : return Menu_ServiceName;
                                    case Generic_ServiceProvider : return Menu_ServiceProvider;
                                    default: return (size_t)-1;
                                }
        default: return (size_t)-1;
    }
}

} //NameSpace
