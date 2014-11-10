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
#if defined(MEDIAINFO_PROPERTYLIST_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Tag/File_PropertyList.h"
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
const char* PropertyList_key(const string &key)
{
    if (key=="director" || key=="directors")
        return "Director";
    if (key=="codirector" || key=="codirectors")
        return "CoDirector";
    if (key=="producer" || key=="producers")
        return "Producer";
    if (key=="coproducer" || key=="coproducers")
        return "CoProducer";
    if (key=="screenwriter" || key=="screenwriters")
        return "ScreenplayBy";
    if (key=="studio" || key=="studios")
        return "ProductionStudio";
    if (key=="cast")
        return "Actor";
    return key.c_str();
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_PropertyList::FileHeader_Begin()
{
    XMLDocument document;
    if (!FileHeader_Begin_XML(document))
       return false;

    XMLElement* plist=document.FirstChildElement("plist");
    if (!plist)
    {
        Reject("XMP");
        return false;
    }

    XMLElement* dict=plist->FirstChildElement("dict");
    if (!dict)
    {
        Reject("XMP");
        return false;
    }

    Accept("PropertyList");

    string key;
    for (XMLElement* dict_Item=dict->FirstChildElement(); dict_Item; dict_Item=dict_Item->NextSiblingElement())
    {
        //key
        if (!strcmp(dict_Item->Value(), "key"))
        {
            const char* Text=dict_Item->GetText();
            if (Text)
                key=Text;
        }

        //string
        if (!strcmp(dict_Item->Value(), "string"))
        {
            const char* Text=dict_Item->GetText();
            if (Text)
                Fill(Stream_General, 0, PropertyList_key(key), Text);

            key.clear();
        }

        //string
        if (!strcmp(dict_Item->Value(), "array"))
        {
            for (XMLElement* array_Item=dict_Item->FirstChildElement(); array_Item; array_Item=array_Item->NextSiblingElement())
            {
                //dict
                if (!strcmp(array_Item->Value(), "dict"))
                {
                    string key2;
                    for (XMLElement* dict2_Item=array_Item->FirstChildElement(); dict2_Item; dict2_Item=dict2_Item->NextSiblingElement())
                    {
                        //key
                        if (!strcmp(dict2_Item->Value(), "key"))
                        {
                            const char* Text=dict2_Item->GetText();
                            if (Text)
                                key2=Text;
                        }

                        //string
                        if (!strcmp(dict2_Item->Value(), "string"))
                        {
                            const char* Text2=dict2_Item->GetText();
                            if (Text2)
                                Fill(Stream_General, 0, key2=="name"?PropertyList_key(key):((string(PropertyList_key(key))+", "+key2).c_str()), Text2);

                            key2.clear();
                        }
                    }
                }
            }

            key.clear();
        }
    }

    Finish();
    return true;
}

} //NameSpace

#endif //MEDIAINFO_PROPERTYLIST_YES
