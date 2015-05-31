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
#if defined(MEDIAINFO_SEQUENCEINFO_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_SequenceInfo.h"
#include "MediaInfo/MediaInfo.h"
#include "MediaInfo/MediaInfo_Internal.h"
#include "MediaInfo/Multiple/File__ReferenceFilesHelper.h"
#include "ZenLib/File.h"
#include "ZenLib/Dir.h"
#include "ZenLib/FileName.h"
#include "ZenLib/Format/Http/Http_Utils.h"
#include "tinyxml2.h"
using namespace tinyxml2;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_SequenceInfo::File_SequenceInfo()
:File__Analyze()
{
    #if MEDIAINFO_DEMUX
        Demux_EventWasSent_Accept_Specific=true;
    #endif //MEDIAINFO_DEMUX

    //Temp
    ReferenceFiles=NULL;
}

//---------------------------------------------------------------------------
File_SequenceInfo::~File_SequenceInfo()
{
    delete ReferenceFiles; //ReferenceFiles=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_SequenceInfo::Streams_Finish()
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
size_t File_SequenceInfo::Read_Buffer_Seek (size_t Method, int64u Value, int64u ID)
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
bool File_SequenceInfo::FileHeader_Begin()
{
    XMLDocument document;
    if (!FileHeader_Begin_XML(document))
       return false;

    {
        XMLElement* Root=document.FirstChildElement("SEQUENCEINFO");
        if (Root)
        {
            Accept("SequenceInfo");
            Fill(Stream_General, 0, General_Format, "SequenceInfo");

            ReferenceFiles=new File__ReferenceFilesHelper(this, Config);

            sequence* Sequence=new sequence;
            Sequence->StreamKind=Stream_Video;

            FileName FN(File_Name);
            Ztring Base=FN.Path_Get();
            size_t Pos=Base.rfind(PathSeparator);
            if (Pos!=string::npos)
            {
                Ztring ToAdd=Base.substr(Pos, string::npos);
                Ztring DirectoryBase=Base;
                DirectoryBase+=ToAdd;
                DirectoryBase+=__T('_');

                size_t DirNumberCount=1;
                Ztring Directory=DirectoryBase;
                for (; DirNumberCount<9; DirNumberCount++)
                {
                    Directory+=__T('0');
                    if (Dir::Exists(Directory))
                        break;
                }

                if (DirNumberCount<9)
                {
                    int32u DirNumber=0;
                    do
                    {
                        Ztring Number=Ztring::ToZtring(DirNumber);
                        if (Number.size()<DirNumberCount)
                            Number.insert(0, DirNumberCount-Number.size(), __T('0'));

                        Directory=DirectoryBase;
                        Directory+=Number;
                        if (!Dir::Exists(Directory))
                            break;

                        Ztring FileBase=Directory;
                        FileBase+=ToAdd;
                        FileBase+=__T('_');
                        FileBase+=__T('.');

                        size_t FileNumberCount=1;
                        Ztring FullFile=FileBase;
                        Ztring Extension;
                        for (; FileNumberCount<10; FileNumberCount++)
                        {
                            FullFile.insert(FullFile.begin()+FullFile.size()-Extension.size()-1, __T('0'));
                            if (Extension.empty())
                            {
                                ZtringList List=Dir::GetAllFileNames(FullFile+__T('*'));
                                if (List.size()>=2)
                                {
                                    FileNumberCount=(size_t)-1; //Problem, which one to choose?
                                    break;
                                }
                                else if (List.size()==1)
                                {
                                    FileName Temp(List[0]);
                                    Extension=Temp.Extension_Get();
                                    FileBase+=Extension;
                                    FullFile=FileBase;
                                    break;
                                }
                            }
                            else if (File::Exists(FullFile))
                                    break;
                        }
                        bool FromZero=true;
                        if (FileNumberCount>=9)
                        {
                            //Trying with consecutive file numbers betweens dirs
                            Number=Ztring::ToZtring(Sequence->FileNames.size());
                            FullFile=FileBase;
                            FullFile.insert(FullFile.size()-Extension.size()-1, Number);
                            FileNumberCount=Number.size();
                            if (!File::Exists(FullFile))
                            {
                                FileNumberCount++;
                                for (; FileNumberCount<10; FileNumberCount++)
                                {
                                    FullFile.insert(FullFile.begin()+FullFile.size()-Extension.size()-Number.size()-1, __T('0'));
                                    if (File::Exists(FullFile))
                                    {
                                        FromZero=false;
                                        break;
                                    }
                                }
                            }
                            else
                                FromZero=false;
                        }

                        if (FileNumberCount<9)
                        {
                            size_t FileNumber=FromZero?0:Sequence->FileNames.size();
                            do
                            {
                                Number=Ztring::ToZtring(FileNumber);
                                if (Number.size()<FileNumberCount)
                                    Number.insert(0, FileNumberCount-Number.size(), __T('0'));

                                FullFile=FileBase;
                                FullFile.insert(FullFile.size()-Extension.size()-1, Number);
                                if (!File::Exists(FullFile))
                                    break;

                                Sequence->AddFileName(FullFile);

                                FileNumber++;
                            }
                            while (FileNumber<1000000000);
                        }

                        DirNumber++;
                    }
                    while (DirNumber<1000000000);

                    ReferenceFiles->AddSequence(Sequence);
                }
            }
        }
        else
        {
            Reject("SequenceInfo");
            return false;
        }
    }

    Element_Offset=File_Size;

    //All should be OK...
    return true;
}

} //NameSpace

#endif //MEDIAINFO_SEQUENCEINFO_YES
