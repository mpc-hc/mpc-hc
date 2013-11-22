/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

// Period
//  AdaptationSet --> One per stream
//   SegmentTemplate (optional)
//    SegmentTimeline
//     S --> duration per segment, count of segments
//   Representation --> file name from SegmentTemplate or BaseURL
//    SegmentBase
//    SegmentList
//  Representation --> file name from BaseURL

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
#if defined(MEDIAINFO_DASHMPD_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_DashMpd.h"
#include "MediaInfo/Multiple/File__ReferenceFilesHelper.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#include "ZenLib/FileName.h"
#include "ZenLib/File.h"
#include "tinyxml2.h"
using namespace ZenLib;
using namespace tinyxml2;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
void DashMpd_Transform (Ztring &Value, std::map<Ztring, Ztring> &Attributes)
{
    size_t Pos1=0;
    for (;;)
    {
        Pos1=Value.find(__T('$'), Pos1);
        if (Pos1==string::npos)
            break;
        size_t Pos2=Value.find(__T('$'), Pos1+1);
        if (Pos2==string::npos)
            break;
        Ztring Name=Value.substr(Pos1+1, Pos2-Pos1-1);
        if (Name.empty())
            Value.erase(Pos1, 1);
        else
        {
            if (Name==__T("RepresentationID"))
                Name=__T("id");
            if (Name==__T("Bandwidth"))
                Name=__T("bandwidth");
            std::map<Ztring, Ztring>::iterator Attribute_It=Attributes.find(Name);
            if (Attribute_It!=Attributes.end())
            {
                Value.erase(Pos1, Pos2-Pos1+1);
                Value.insert(Pos1, Attribute_It->second);
            }
            else
                Pos1+=2+Name.size();
        }
    }
}

//---------------------------------------------------------------------------
stream_t DashMpd_mimeType_StreamKind (const char* mimeType)
{
    Ztring StreamKind; StreamKind.From_UTF8(mimeType);
        if (StreamKind.find(__T("video"))==0)
        return Stream_Video;
    else if (StreamKind.find(__T("audio"))==0)
        return Stream_Audio;
    else if (StreamKind.find(__T("application/ttml+xml"))==0)
        return Stream_Text;
    else
        return Stream_Other;
}

//---------------------------------------------------------------------------
Ztring DashMpd_codecid_CodecID (const char* codecid)
{
    Ztring CodecID;

    Ztring Codecs; Codecs.From_UTF8(codecid);
    size_t DotPos=Codecs.find(__T('.'));
    if (DotPos==4 && Codecs.substr(0, DotPos).find(__T("mp4"))==0)
        DotPos=Codecs.find(__T('.'), 5);
    if (DotPos==string::npos)
    {
        CodecID=Codecs;
    }
    else
    {
        CodecID=Codecs.substr(0, DotPos);
        //TODO per format, rfc 6381 //ReferenceFile.Infos["Format_Profile"]=;
    }
    CodecID.FindAndReplace(__T("0x"), Ztring(), 0, Ztring_Recursive);

    return CodecID;
}

//---------------------------------------------------------------------------
struct template_generic
{
    File__ReferenceFilesHelper::reference ReferenceFile;
    Ztring SourceDir;
    Ztring BaseURL;
    Ztring initialization;
    Ztring media;
    int64u duration;
    int64u startNumber;
    int64u duration_Max;
    int64u startNumber_Max;
    struct segmenttimeline
    {
        int64u t; //start time
        int64u d; //duration per segment
        int64u r; //repeat count

        segmenttimeline()
        {
            t=1;
            d=1;
            r=0;
        }
    };
    std::vector<segmenttimeline> SegmentTimeLines;
    std::map<Ztring, Ztring> Attributes_ForMedia;

    template_generic(const Ztring &BaseURL=Ztring(), const Ztring &SourceDir=Ztring())
    {
        template_generic::BaseURL=BaseURL;
        template_generic::SourceDir=SourceDir;
        duration=1;
        startNumber=1;
        duration_Max=0;
        startNumber_Max=0;
    }

    void AdaptationSet_Attributes_Parse     (XMLElement* Item);
    void SegmentTemplate_Attributes_Parse   (XMLElement* Item);
    void SegmentTimeline_Attributes_Parse   (XMLElement* Item);
    void Representation_Attributes_Parse    (XMLElement* Item);

    void Decode ();
};

void template_generic::AdaptationSet_Attributes_Parse (XMLElement* Item)
{
    const char* Attribute;

    //Attributes - mineType
    Attribute=Item->Attribute("mimeType");
    if (Attribute)
        ReferenceFile.StreamKind=DashMpd_mimeType_StreamKind(Attribute);

    //Attributes - codecs
    Attribute=Item->Attribute("codecs");
    if (Attribute)
        ReferenceFile.Infos["CodecID"]=DashMpd_codecid_CodecID(Attribute);

    //Attributes - lang
    Attribute=Item->Attribute("lang");
    if (Attribute)
        ReferenceFile.Infos["Language"].From_UTF8(Attribute);
}

void template_generic::SegmentTemplate_Attributes_Parse (XMLElement* Item)
{
    const char* Attribute;

    //Attributes - initialization
    Attribute=Item->Attribute("initialization");
    if (Attribute)
    {
        initialization.From_UTF8(Attribute);
    }

    //Attributes - media
    Attribute=Item->Attribute("media");
    if (Attribute)
    {
        media.From_UTF8(Attribute);
    }

    //Attributes - duration
    Attribute=Item->Attribute("duration");
    if (Attribute)
    {
        duration=Ztring().From_UTF8(Attribute).To_int64u();
    }

    //Attributes - startNumber
    Attribute=Item->Attribute("startNumber");
    if (Attribute)
    {
        startNumber=Ztring().From_UTF8(Attribute).To_int64u();
    }
}

void template_generic::SegmentTimeline_Attributes_Parse (XMLElement* Item)
{
    const char* Attribute;
    segmenttimeline SegmentTimeLine;

    //Attributes - t (start time)
    Attribute=Item->Attribute("t");
    if (Attribute)
    {
        SegmentTimeLine.t=Ztring().From_UTF8(Attribute).To_int64u();
    }
    else
        SegmentTimeLine.t=startNumber;

    //Attributes - d (duration per segment)
    Attribute=Item->Attribute("d");
    if (Attribute)
    {
        SegmentTimeLine.d=Ztring().From_UTF8(Attribute).To_int64u();
    }
    else
        SegmentTimeLine.d=duration;

    //Attributes - r (repeat count)
    Attribute=Item->Attribute("r");
    if (Attribute)
    {
        SegmentTimeLine.r=Ztring().From_UTF8(Attribute).To_int64u();
    }

    SegmentTimeLines.push_back(SegmentTimeLine);
    duration_Max+=SegmentTimeLine.d*(SegmentTimeLine.r+1);
    startNumber_Max+=SegmentTimeLine.r+1;
}

void template_generic::Representation_Attributes_Parse (XMLElement* Item)
{
    const char* Attribute;

    //Attributes - id
    Attribute=Item->Attribute("id");
    if (Attribute)
    {
        ReferenceFile.StreamID=Ztring().From_UTF8(Attribute).To_int64u(16);
    }

    //Attributes - bandwidth
    Attribute=Item->Attribute("bandwidth");
    if (Attribute)
    {
        ReferenceFile.Infos["BitRate"].From_UTF8(Attribute);
    }

    //Attributes - frame size
    Attribute=Item->Attribute("width");
    if (Attribute)
    {
        ReferenceFile.Infos["Width"].From_UTF8(Attribute);
    }
    Attribute=Item->Attribute("height");
    if (Attribute)
    {
        ReferenceFile.Infos["Height"].From_UTF8(Attribute);
    }

    //Attributes - mineType
    Attribute=Item->Attribute("mimeType");
    if (Attribute)
        ReferenceFile.StreamKind=DashMpd_mimeType_StreamKind(Attribute);

    //Attributes - codecs
    Attribute=Item->Attribute("codecs");
    if (Attribute)
        ReferenceFile.Infos["CodecID"]=DashMpd_codecid_CodecID(Attribute);

    //Attributes - lang
    Attribute=Item->Attribute("lang");
    if (Attribute)
        ReferenceFile.Infos["Language"].From_UTF8(Attribute);

    //Attributes - Saving all attributes
    for (const XMLAttribute* Attribute_Item=Item->FirstAttribute(); Attribute_Item; Attribute_Item=Attribute_Item->Next())
    {
        Ztring Name; Name.From_UTF8(Attribute_Item->Name());
        Ztring Value; Value.From_UTF8(Attribute_Item->Value());
        Attributes_ForMedia[Name]=Value;
    }
}

//---------------------------------------------------------------------------
void template_generic::Decode()
{
    //initialization - URL decoding, template adaptation and add it
    if (!initialization.empty())
    {
        DashMpd_Transform(initialization, Attributes_ForMedia);
        ReferenceFile.FileNames.push_back(BaseURL+initialization);
    }

    //media - URL decoding, template adaptation and add it
    if (!media.empty())
    {
        DashMpd_Transform(media, Attributes_ForMedia);
        size_t Index_Pos=media.find(__T("$Index"));
        size_t Index_StringSize=5;
        if (Index_Pos==string::npos)
        {
            Index_Pos=media.find(__T("$Number"));
            Index_StringSize++;
        }
        int8u Index_Size=1;
        if (Index_Pos!=string::npos)
        {
            size_t Index_Pos_End=media.find(__T('$'), Index_Pos+1+Index_StringSize);
            if (Index_Pos_End!=string::npos && Index_Pos+1+Index_StringSize+2<Index_Pos_End && media[Index_Pos+1+Index_StringSize]=='%' && media[Index_Pos+1+Index_StringSize+1]=='0')
            {
                Index_Size=Ztring(media.substr(Index_Pos+1+Index_StringSize+2, Index_Pos_End-(Index_Pos+1+Index_StringSize+2))).To_int8u();
            }
            else if (Index_Pos_End==string::npos || Index_Pos+1+Index_StringSize!=Index_Pos_End)
                Index_Pos=string::npos;
        }
        size_t Time_Pos=media.find(__T("$Time$"));
        if (Index_Pos!=string::npos || Time_Pos!=string::npos)
        {
            Ztring Media_Name(media);
            if (Index_Pos!=string::npos)
            {
                Media_Name.erase(Index_Pos, 1+Index_StringSize+1);
                if (Time_Pos!=string::npos && Time_Pos>Index_Pos)
                    Time_Pos-=1+Index_StringSize+1;
            }
            if (Time_Pos!=string::npos)
            {
                Media_Name.erase(Time_Pos, 6);
                if (Index_Pos!=string::npos && Index_Pos>Time_Pos)
                    Index_Pos-=6;
            }
            if (SegmentTimeLines.empty())
            {
                int64u Index_Pos_Temp=startNumber;
                for (;;)
                {
                    Ztring Media_Name_Temp(Media_Name);
                    Ztring Index; Index.From_Number(Index_Pos_Temp);
                    if (Index.size()<Index_Size)
                        Index.insert(0, Index_Size-Index.size(), __T('0'));
                    Media_Name_Temp.insert(Index_Pos, Index);

                    Ztring File_Name;
                    if (!SourceDir.empty())
                        File_Name+=SourceDir+PathSeparator;
                    File_Name+=BaseURL+Media_Name_Temp;
                    if (!File::Exists(File_Name))
                        break;
                    ReferenceFile.FileNames.push_back(File_Name);
                    Index_Pos_Temp++;
                }
            }
            else
            {
                int64u SegmentTimeLines_duration=0;
                int64u SegmentTimeLines_startNumber=startNumber;
                for (size_t SegmentTimeLines_Pos=0; SegmentTimeLines_Pos<SegmentTimeLines.size(); SegmentTimeLines_Pos++)
                {
                    for (int64u Pos=0; Pos<=SegmentTimeLines[SegmentTimeLines_Pos].r; Pos++)
                    {
                        Ztring Media_Name_Temp(Media_Name);
                        size_t Time_Pos_Temp=Time_Pos;
                        if (Index_Pos!=string::npos)
                        {
                            Ztring Index; Index.From_Number(SegmentTimeLines_startNumber);
                            if (Index.size()<Index_Size)
                                Index.insert(0, Index_Size-Index.size(), __T('0'));
                            Media_Name_Temp.insert(Index_Pos, Index);
                            if (Time_Pos!=string::npos && Time_Pos>Index_Pos)
                                Time_Pos_Temp+=Index.size();
                        }
                        if (Time_Pos_Temp!=string::npos)
                        {
                            Ztring Time; Time.From_Number(SegmentTimeLines_duration);
                            Media_Name_Temp.insert(Time_Pos_Temp, Time);
                        }

                        ReferenceFile.FileNames.push_back(BaseURL+Media_Name_Temp);
                        SegmentTimeLines_duration+=SegmentTimeLines[SegmentTimeLines_Pos].d;
                        SegmentTimeLines_startNumber++;
                    }
                }
            }
        }
        else
            ReferenceFile.FileNames.push_back(BaseURL+media);
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_DashMpd::File_DashMpd()
:File__Analyze()
{
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_DashMpd;
        StreamIDs_Width[0]=16;
    #endif //MEDIAINFO_EVENTS

    //Temp
    ReferenceFiles=NULL;
}

//---------------------------------------------------------------------------
File_DashMpd::~File_DashMpd()
{
    delete ReferenceFiles; //ReferenceFiles=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_DashMpd::Streams_Finish()
{
    if (ReferenceFiles==NULL)
        return;

    ReferenceFiles->ParseReferences();
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_SEEK
size_t File_DashMpd::Read_Buffer_Seek (size_t Method, int64u Value, int64u ID)
{
    if (ReferenceFiles==NULL)
        return 0;

    return ReferenceFiles->Read_Buffer_Seek(Method, Value, ID);
}
#endif //MEDIAINFO_SEEK

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_DashMpd::FileHeader_Begin()
{
    XMLDocument document;
    if (!FileHeader_Begin_XML(document))
       return false;

    {
        XMLElement* Root=document.FirstChildElement("MPD");
        if (Root)
        {
            const char* Attribute=Root->Attribute("xmlns");
            if (Attribute==NULL
             || (Ztring().From_UTF8(Attribute)!=__T("urn:mpeg:DASH:schema:MPD:2011")
              && Ztring().From_UTF8(Attribute)!=__T("urn:mpeg:dash:schema:mpd:2011") //Some muxers use lower case version
              && Ztring().From_UTF8(Attribute)!=__T("urn:3GPP:ns:PSS:AdaptiveHTTPStreamingMPD:2009")))
            {
                Reject("DashMpd");
                return false;
            }

            Accept("DashMpd");
            Fill(Stream_General, 0, General_Format, "DASH MPD");
            Config->File_ID_OnlyRoot_Set(false);

            ReferenceFiles=new File__ReferenceFilesHelper(this, Config);

            //Parsing main elements
            Ztring BaseURL;

            for (XMLElement* Root_Item=Root->FirstChildElement(); Root_Item; Root_Item=Root_Item->NextSiblingElement())
            {
                //Common information
                if (string(Root_Item->Value())=="BaseURL")
                {
                    if (BaseURL.empty()) //Using the first one
                        BaseURL=Root_Item->GetText();
                }

                //Period
                if (string(Root_Item->Value())=="Period")
                {
                    for (XMLElement* Period_Item=Root_Item->FirstChildElement(); Period_Item; Period_Item=Period_Item->NextSiblingElement())
                    {
                        //AdaptationSet (=a stream)
                        if (string(Period_Item->Value())=="AdaptationSet")
                        {
                            template_generic Template_Generic(BaseURL, FileName(File_Name).Path_Get());

                            Template_Generic.AdaptationSet_Attributes_Parse(Period_Item);

                            //Sub
                            for (XMLElement* AdaptationSet_Item=Period_Item->FirstChildElement(); AdaptationSet_Item; AdaptationSet_Item=AdaptationSet_Item->NextSiblingElement())
                            {
                                //SegmentTemplate
                                if (string(AdaptationSet_Item->Value())=="SegmentTemplate")
                                {
                                    Template_Generic.SegmentTemplate_Attributes_Parse(AdaptationSet_Item);

                                    //Sub
                                    for (XMLElement* SegmentTemplate_Item=AdaptationSet_Item->FirstChildElement(); SegmentTemplate_Item; SegmentTemplate_Item=SegmentTemplate_Item->NextSiblingElement())
                                    {
                                        //SegmentTimeline
                                        if (string(SegmentTemplate_Item->Value())=="SegmentTimeline")
                                        {
                                            //Sub
                                            for (XMLElement* SegmentTimeline_Item=SegmentTemplate_Item->FirstChildElement(); SegmentTimeline_Item; SegmentTimeline_Item=SegmentTimeline_Item->NextSiblingElement())
                                            {
                                                //SegmentTimeline
                                                if (string(SegmentTimeline_Item->Value())=="S")
                                                {
                                                    Template_Generic.SegmentTimeline_Attributes_Parse(SegmentTimeline_Item);
                                                }
                                            }
                                        }
                                    }
                                }

                                //Representation
                                if (string(AdaptationSet_Item->Value())=="Representation")
                                {
                                    template_generic Template_Generic_PerRepresentation(Template_Generic);

                                    Template_Generic_PerRepresentation.Representation_Attributes_Parse(AdaptationSet_Item);

                                    //Sub
                                    for (XMLElement* Representation_Item=AdaptationSet_Item->FirstChildElement(); Representation_Item; Representation_Item=Representation_Item->NextSiblingElement())
                                    {
                                        //BaseURL
                                        if (string(Representation_Item->Value())=="BaseURL")
                                        {
                                            Template_Generic_PerRepresentation.ReferenceFile.FileNames.push_back(BaseURL+Ztring().From_UTF8(Representation_Item->GetText()));
                                        }

                                        //SegmentTemplate
                                        if (string(Representation_Item->Value())=="SegmentTemplate")
                                        {
                                            Template_Generic_PerRepresentation.SegmentTemplate_Attributes_Parse(Representation_Item);
                                        }

                                        //SegmentBase
                                        if (string(Representation_Item->Value())=="SegmentBase")
                                        {
                                            //Sub
                                            for (XMLElement* SegmentBase_Item=Representation_Item->FirstChildElement(); SegmentBase_Item; SegmentBase_Item=SegmentBase_Item->NextSiblingElement())
                                            {
                                                //Initialization
                                                if (string(SegmentBase_Item->Value())=="Initialization")
                                                {
                                                    Attribute=SegmentBase_Item->Attribute("sourceURL");
                                                    if (Attribute)
                                                        Template_Generic_PerRepresentation.ReferenceFile.FileNames.insert(Template_Generic_PerRepresentation.ReferenceFile.FileNames.begin(), BaseURL+Ztring().From_UTF8(Attribute));
                                                }
                                            }
                                        }

                                        //SegmentList
                                        if (string(Representation_Item->Value())=="SegmentList")
                                        {
                                            //Sub
                                            for (XMLElement* SegmentBase_Item=Representation_Item->FirstChildElement(); SegmentBase_Item; SegmentBase_Item=SegmentBase_Item->NextSiblingElement())
                                            {
                                                //Initialization
                                                if (string(SegmentBase_Item->Value())=="SegmentURL")
                                                {
                                                    bool IsSupported=true;
                                                    Attribute=SegmentBase_Item->Attribute("mediaRange");
                                                    if (Attribute)
                                                    {
                                                        size_t Length=strlen(Attribute);
                                                        if (Length<2
                                                         || Attribute[0]!='0'
                                                         || Attribute[1]!='-')
                                                            IsSupported=false; //Currently, we do not support ranges
                                                    }

                                                    Attribute=SegmentBase_Item->Attribute("media");
                                                    if (Attribute && IsSupported)
                                                        Template_Generic_PerRepresentation.ReferenceFile.FileNames.push_back(BaseURL+Ztring().From_UTF8(Attribute));
                                                }
                                            }
                                        }
                                    }

                                    Template_Generic_PerRepresentation.Decode();
                                    ReferenceFiles->References.push_back(Template_Generic_PerRepresentation.ReferenceFile);
                                }
                            }
                        }

                        //Representation (=a stream)
                        if (string(Period_Item->Value())=="Representation")
                        {
                            File__ReferenceFilesHelper::reference ReferenceFile;
                            int64u duration=1;

                            //Attributes - mineType
                            Attribute=Period_Item->Attribute("mimeType");
                            if (Attribute)
                                ReferenceFile.StreamKind=DashMpd_mimeType_StreamKind(Attribute);

                            //Attributes - codecs
                            Attribute=Period_Item->Attribute("codecs");
                            if (Attribute)
                                ReferenceFile.Infos["CodecID"]=DashMpd_codecid_CodecID(Attribute);

                            //Attributes - lang
                            Attribute=Period_Item->Attribute("lang");
                            if (Attribute)
                                ReferenceFile.Infos["Language"].From_UTF8(Attribute);

                            //Sub
                            for (XMLElement* AdaptationSet_Item=Period_Item->FirstChildElement(); AdaptationSet_Item; AdaptationSet_Item=AdaptationSet_Item->NextSiblingElement())
                            {
                                //SegmentInfo
                                if (string(AdaptationSet_Item->Value())=="SegmentInfo")
                                {
                                    //Attributes - duration
                                    Attribute=AdaptationSet_Item->Attribute("duration");
                                    if (Attribute)
                                    {
                                        duration=Ztring().From_UTF8(Attribute).To_int64u();
                                    }

                                    //Sub
                                    for (XMLElement* SegmentInfo_Item=AdaptationSet_Item->FirstChildElement(); SegmentInfo_Item; SegmentInfo_Item=SegmentInfo_Item->NextSiblingElement())
                                    {
                                        //InitialisationSegmentURL
                                        if (string(SegmentInfo_Item->Value())=="InitialisationSegmentURL")
                                        {
                                            Attribute=SegmentInfo_Item->Attribute("sourceURL");
                                            if (Attribute)
                                                ReferenceFile.FileNames.insert(ReferenceFile.FileNames.begin(), BaseURL+Ztring().From_UTF8(Attribute));
                                        }

                                        //Url
                                        if (string(SegmentInfo_Item->Value())=="Url")
                                        {
                                            Attribute=SegmentInfo_Item->Attribute("sourceURL");
                                            if (Attribute)
                                                ReferenceFile.FileNames.push_back(BaseURL+Ztring().From_UTF8(Attribute));
                                        }
                                    }

                                    ReferenceFiles->References.push_back(ReferenceFile);
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            Reject("DashMpd");
            return false;
        }
    }

    Element_Offset=File_Size;

    //All should be OK...
    return true;
}

} //NameSpace

#endif //MEDIAINFO_DASHMPD_YES

