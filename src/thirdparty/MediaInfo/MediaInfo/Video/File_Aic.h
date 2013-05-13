/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Apple Intermediate Codec video streams
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_AicH
#define MediaInfo_AicH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Aic
//***************************************************************************

class File_Aic : public File__Analyze
{
private :
    //Streams management
    void Streams_Fill();

    //Buffer - Per element
    void Header_Parse ();
    void Data_Parse ();

    //Temp
    int16u Width;
    int16u Height;
    int8u  FieldFrame;
};

} //NameSpace

#endif
