/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Ibi files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_Ibi_CreationH
#define MediaInfo_File_Ibi_CreationH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Ztring.h"
#include <vector>
#include <map>
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Ibi structure
//***************************************************************************

struct ibi
{
    struct stream
    {
        int64u DtsFrequencyNumerator;
        int64u DtsFrequencyDenominator;
        struct info
        {
            int64u StreamOffset;
            int64u FrameNumber;
            int64u Dts;
            bool   IsContinuous;

            info()
            {
                StreamOffset=(int64u)-1;
                FrameNumber=(int64u)-1;
                Dts=(int64u)-1;
                IsContinuous=false;
            }
        };
        std::vector<info>   Infos;
        size_t              Infos_Pos;
        bool                IsContinuous;
        bool                IsModified;
        bool                IsSynchronized;

        stream()
        {
            DtsFrequencyNumerator=1000000000; //nanosecond
            DtsFrequencyDenominator=1;
            Infos_Pos=0;
            IsContinuous=false;
            IsModified=false;
            IsSynchronized=false;
        }

        void Add (const info &Info);
        void Unsynch();
    };
    typedef std::map<int64u, stream*>   streams;
    streams                             Streams;

    Ztring                              Inform_Data;
    Ztring                              FileName;

    //Constructor/Destructor
    ibi();
    ~ibi();
};

//***************************************************************************
/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */
//***************************************************************************

class File_Ibi_Creation
{
public :
    File_Ibi_Creation();
    File_Ibi_Creation(const ibi &Ibi) {Set(Ibi);}
    ~File_Ibi_Creation();

    void Set(const ibi &Ibi);
    void Add(int64u ID, const ibi::stream &Stream);
    Ztring Finish();

private :
    struct buffer
    {
        int8u* Content;
        size_t Size;

        buffer()
        {
            Content=NULL;
            Size=0;
        }

        ~buffer()
        {
            delete[] Content; //Content=NULL;
        }
    };
    std::vector<buffer*> Buffers;
};

} //NameSpace

#endif

