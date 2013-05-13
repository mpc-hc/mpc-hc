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
#if defined(MEDIAINFO_TTML_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Text/File_Ttml.h"
#include "tinyxml2.h"
using namespace tinyxml2;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Ttml::FileHeader_Begin()
{
    XMLDocument document;
    if (!FileHeader_Begin_XML(document))
       return false;

    {
        XMLElement* Root=document.FirstChildElement("tt");
        if (Root)
        {
            Accept();
            Fill(Stream_General, 0, General_Format, "TTML");
            Stream_Prepare(Stream_Text);
            Fill(Stream_Text, 0, "Format", "TTML");
        }
        else
        {
            Reject();
            return false;
        }
    }

    //All should be OK...
    return true;
}

} //NameSpace

#endif //MEDIAINFO_TTML_YES
