/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about DCP/IMF Package List files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_DcpPklH
#define MediaInfo_File_DcpPklH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include <vector>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

class File__ReferenceFilesHelper;

//***************************************************************************
// Class File_DcpPkl
//***************************************************************************

class File_DcpPkl : public File__Analyze
{
public :
    //Constructor/Destructor
    File_DcpPkl();
    ~File_DcpPkl();

    //Streams
    struct stream
    {
        stream_t    StreamKind; // With special cases: Stream_Max+1 means CPL, Stream_Max+2 means PKL
        string      Id;
        string      OriginalFileName;
        string      Type;
        string      AnnotationText;
        struct chunk
        {
            string Path;
        };
        typedef std::vector<chunk> chunks;
        chunks      ChunkList;

        stream()
        {
            StreamKind=Stream_Max;
        }
    };
    typedef std::vector<stream> streams;
    streams Streams;

private :
    //Streams management
    void Streams_Finish ();

    //Buffer - Global
    #if MEDIAINFO_SEEK
    size_t Read_Buffer_Seek (size_t Method, int64u Value, int64u ID);
    #endif //MEDIAINFO_SEEK

    //Buffer - File header
    bool FileHeader_Begin();

    //AM
    void MergeFromAm (File_DcpPkl::streams &StreamsToMerge);

    //Temp
    File__ReferenceFilesHelper*     ReferenceFiles;
};

} //NameSpace

#endif

