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
#if defined(MEDIAINFO_HDSF4M_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_HdsF4m.h"
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

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_HdsF4m::File_HdsF4m()
:File__Analyze()
{
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_HdsF4m;
        StreamIDs_Width[0]=16;
    #endif //MEDIAINFO_EVENTS

    //Temp
    ReferenceFiles=NULL;
}

//---------------------------------------------------------------------------
File_HdsF4m::~File_HdsF4m()
{
    delete ReferenceFiles; //ReferenceFiles=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_HdsF4m::Streams_Finish()
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
size_t File_HdsF4m::Read_Buffer_Seek (size_t Method, int64u Value, int64u ID)
{
    if (ReferenceFiles==NULL)
        return 0;

    return ReferenceFiles->Seek(Method, Value, ID);
}
#endif //MEDIAINFO_SEEK

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_HdsF4m::FileHeader_Begin()
{
    XMLDocument document;
    if (!FileHeader_Begin_XML(document))
       return false;

    {
        XMLElement* Root=document.FirstChildElement("manifest");
        if (Root)
        {
            const char* Attribute=Root->Attribute("xmlns");
            if (Attribute==NULL || Ztring().From_UTF8(Attribute)!=__T("http://ns.adobe.com/f4m/1.0"))
            {
                Reject("HdsF4m");
                return false;
            }

            Accept("HdsF4m");
            Fill(Stream_General, 0, General_Format, "HDS F4M");
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
                if (string(Root_Item->Value())=="media")
                {
                    sequence* Sequence=new sequence;
                    const char* Attribute;

                    //Attributes - mineType
                    Attribute=Root_Item->Attribute("url");
                    if (Attribute)
                        Sequence->AddFileName(Ztring().From_UTF8(Attribute)+__T("Seg1.f4f"));

                    Sequence->StreamID=ReferenceFiles->Sequences_Size()+1;
                    ReferenceFiles->AddSequence(Sequence);
                }
            }
        }
        else
        {
            Reject("HdsF4m");
            return false;
        }
    }

    Element_Offset=File_Size;

    //All should be OK...
    return true;
}

} //NameSpace

#endif //MEDIAINFO_HDSF4M_YES

