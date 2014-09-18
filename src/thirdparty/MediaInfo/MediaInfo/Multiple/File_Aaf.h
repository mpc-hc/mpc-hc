/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about AAF files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_AafH
#define MediaInfo_File_AafH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include <vector>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

class File__ReferenceFilesHelper;

//***************************************************************************
// Class File_Aaf
//***************************************************************************

class File_Aaf : public File__Analyze
{
public :
    //Constructor/Destructor
    File_Aaf();
    ~File_Aaf();

private :
    //Streams management
    void Streams_Finish ();

    //Buffer - Global
    #if MEDIAINFO_SEEK
    size_t Read_Buffer_Seek (size_t Method, int64u Value, int64u ID);
    #endif //MEDIAINFO_SEEK
    void Read_Buffer_Continue ();

    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Elements
    void FatSect();
    void Fat();
    void MiniFat();
    void Directory();
    void Directory_Entry();
    void StreamElement();
    void StreamElement_Parse();

    void MetaDictionary();                  //0x0001
    void Header();                          //0x0002
    void ClassDefinitions();                //0x0003
    void TypeDefinitions();                 //0x0004
    void Identification();                  //0x0005
    void Name();                            //0x0006
    void MetaDefinition();                  //0x0007
    void ParentClass();                     //0x0008
    void Properties();                      //0x0009
    void IsConcrete();                      //0x000A
    void Type();                            //0x000B
    void IsOptional();                      //0x000C
    void LocalIdentification();             //0x000D
    void IsUniqueIdentifier();              //0x000E
    void Size();                            //0x000F
    void Locked();                          //0x3D02
    void NetworkLocator();                  //0x4001

    //Temp
    int16u xxxSize;
    int16u SectorShift;
    int16u MiniSectorShift;
    int32u sectMiniFatStart;
    int32u sectDirStart;
    int32u MiniSectorCutoff;
    typedef enum step
    {
        Step_None,
        Step_Fat,
        Step_MiniFat,
        Step_Directory,
        Step_Stream,
    } step;
    step Step;
    vector<int32u> sectsFat;
    vector<int32u> sectsMiniStream;
    vector<int32u> Pointers;
    vector<int32u> MiniPointers;
    typedef struct stream
    {
        Ztring Name;
        size_t Directory_Pos;
        int32u ChildSID;
        int64u Size;
        int8u* Buffer;
        vector<int32u> StreamOffsets;

        stream(const Ztring Name_, size_t Directory_Pos_, int64u Size_)
            :
            Name(Name_),
            Directory_Pos(Directory_Pos_),
            ChildSID((int32u)-1),
            Size(Size_),
            Buffer(NULL)
        {
        }

    private:
        stream &operator=(const stream &v);
        stream();
    } stream;
    vector<stream*> Streams;
    size_t sectsFat_Pos;
    size_t Streams_Pos;
    size_t Streams_Pos2;
    size_t Directory_Pos;
    File__ReferenceFilesHelper*     ReferenceFiles;

    //Descriptor
    /*
    struct descriptor
    {
        stream_t StreamKind;
        int32u   ChildSID;

        descriptor()
        {
            StreamKind=Stream_Max;
            ChildSID=(int32u)-1;
        }
    };
    typedef std::map<int32u, descriptor> descriptors; //Key is Directory_Pos of Descriptor
    descriptors Descriptors;
    */

    //Locator
    /*
    struct locator
    {
        Ztring      EssenceLocator;
    };
    typedef std::map<int32u, locator> locators; //Key is Directory_Pos of the locator
    locators Locators;
    */
};

} //NameSpace

#endif

