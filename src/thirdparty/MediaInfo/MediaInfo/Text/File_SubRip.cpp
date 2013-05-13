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
#if defined(MEDIAINFO_SUBRIP_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Text/File_SubRip.h"
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_SubRip::Read_Buffer_Continue()
{
    if (!IsSub && (Buffer_Size<File_Size && Buffer_Size<65536))
    {
        Element_WaitForMoreData();
        return;
    }

    ZtringListList Temp;
    Temp.Separator_Set(0, __T("\r\n\r\n"));
    Temp.Separator_Set(1, __T("\r\n"));

    Temp.Write(Ztring().From_UTF8((const char*)Buffer, Buffer_Size>65536?65536:Buffer_Size));
    if (Temp.empty())
        Temp.Write(Ztring().From_Local((const char*)Buffer, Buffer_Size>65536?65536:Buffer_Size)); // Trying from local code page

    size_t IsOk=0;
    size_t IsNok=0;
    for (size_t Pos=0; Pos<Temp.size(); Pos++)
    {
        if (Temp(Pos, 0).To_int64u()==Pos+1)
            IsOk++;
        else
            IsNok++;

        if (Temp(Pos, 1).size()>22 && Temp(Pos, 1)[2]==__T(':') && Temp(Pos, 1)[5]==__T(':') && Temp(Pos, 1).find(__T(" --> "))!=string::npos)
            IsOk++;
        else
            IsNok++;
    }

    if (!IsOk || IsNok>IsOk/2)
    {
        Reject();
        return;
    }

    if (!Status[IsAccepted])
    {
        Accept();
        Fill(Stream_General, 0, General_Format, "SubRip");
        Stream_Prepare(Stream_Text);
        Fill(Stream_Text, 0, "Format", "SubRip");
        Fill(Stream_Text, 0, "Codec", "SubRip");
    }

    Element_Offset=File_Size;
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_SUBRIP_YES
