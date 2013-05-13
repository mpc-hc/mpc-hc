/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about SKM (Korean mobilphoner) files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_SkmH
#define MediaInfo_File_SkmH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Skm
//***************************************************************************

class File_Skm : public File__Analyze
{
public :
    //Constructor/Destructor
    File_Skm();

private :
    //Streams management
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin();
    void FileHeader_Parse();

    //Buffer - Synchro
    bool Synchronize() {return Synchronize_0x000001();}
    bool Synched_Test();

    //Buffer - Per element
    void Header_Parse();
    bool Header_Parse_Fill_Size();
    void Data_Parse();

    //Streams
    struct stream
    {
        File__Analyze*          Parser;

        stream()
        {
            Parser=NULL;
        }

        ~stream()
        {
            delete Parser; //Parser=NULL;
        }
    };
    stream Stream;
};

} //NameSpace

#endif
