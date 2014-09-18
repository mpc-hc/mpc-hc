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
#if defined(MEDIAINFO_PDF_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Tag/File_Xmp.h"
#include <cstring>
#include "tinyxml2.h"
using namespace tinyxml2;
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Xmp::FileHeader_Begin()
{
    XMLDocument document;
    if (!FileHeader_Begin_XML(document))
       return false;

    std::string NameSpace;
    XMLElement* XmpMeta=document.FirstChildElement("xmpmeta");
    if (XmpMeta==NULL)
    {
        NameSpace="x:";
        XmpMeta=document.FirstChildElement((NameSpace+"xmpmeta").c_str());
    }
    if (!XmpMeta)
    {
        Reject("XMP");
        return false;
    }

    XMLElement* Rdf=XmpMeta->FirstChildElement("RDF");
    if (Rdf==NULL)
    {
        NameSpace="rdf:";
        Rdf=XmpMeta->FirstChildElement((NameSpace+"RDF").c_str());
    }
    if (!Rdf)
    {
        Reject("XMP");
        return false;
    }

    Accept("XMP");

    for (XMLElement* Rdf_Item=Rdf->FirstChildElement(); Rdf_Item; Rdf_Item=Rdf_Item->NextSiblingElement())
    {
        //RDF item
        if (!strcmp(Rdf_Item->Value(), (NameSpace+"Description").c_str()))
        {
            const char* Attribute;

            Attribute=Rdf_Item->Attribute("xmlns:pdfaid");
            if (Attribute)
            {
                string Profile;

                if (strcmp(Attribute, "http://www.aiim.org/pdfa/ns/id/"))
                    Profile=Attribute;
                else
                {
                    Profile+="A";

                    Attribute=Rdf_Item->Attribute("pdfaid:part");
                    if (Attribute)
                    {
                        Profile+='-';
                        Profile+=Attribute;

                        Attribute=Rdf_Item->Attribute("pdfaid:conformance");
                        if (Attribute)
                        {
                            string Conformance(Attribute);
                            if (Conformance.size()==1 && Conformance[0]>='A' && Conformance[0]<='Z')
                                Conformance[0]+=0x20; // From "A" to "a"
                            Profile+=Conformance;
                        }
                    }
                }

                Fill(Stream_General, 0, General_Format_Profile, Profile);
            }
        }
    }

    Finish();
    return true;
}

} //NameSpace

#endif //MEDIAINFO_PDF_YES
