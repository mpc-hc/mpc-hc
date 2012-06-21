// File_Swf - Info for SWF Audio files
// Copyright (C) 2005-2011 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about SWF files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_SwfH
#define MediaInfo_File_SwfH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Swf
//***************************************************************************

class File_Swf : public File__Analyze
{
public :
    //In
    int64u Frame_Count_Valid;
    int32u FileLength;;
    int8u  Version;

public :
    File_Swf();
    
private :
    //Buffer
    bool FileHeader_Begin();
    void FileHeader_Parse();
    void Header_Parse();
    void Data_Parse();

    //Elements
    void Header();
    void Header_Continue();
    void End()                          {};
    void ShowFrame()                    {};
    void DefineShape()                  {};
    void PlaceObject()                  {};
    void RemoveObject()                 {};
    void DefineBits()                   {};
    void DefineButton()                 {};
    void JPEGTables()                   {};
    void SetBackgroundColor()           {};
    void DefineFont()                   {};
    void DefineText()                   {};
    void DoAction()                     {};
    void DefineFontInfo()               {};
    void DefineSound();
    void StartSound()                   {};
    void DefineButtonSound()            {};
    void SoundStreamHead();
    void SoundStreamBlock()             {};
    void DefineBitsLossless()           {};
    void DefineBitsJPEG2()              {};
    void DefineShape2()                 {};
    void DefineCxform()                 {};
    void Protect()                      {};
    void PlaceObject2()                 {};
    void RemoveObject2()                {};
    void DefineShape3()                 {};
    void DefineText2()                  {};
    void DefineButton2()                {};
    void DefineBitsJPEG3()              {};
    void DefineBitsLossless2()          {};
    void DefineEditText()               {};
    void DefineSprite();
    void FrameLabel()                   {};
    void DefineMorphShape()             {};
    void SoundStreamHead2()             {SoundStreamHead();};
    void DefineFont2()                  {};
    void ExportAssets()                 {};
    void ImportAssets()                 {};
    void EnableDebugger()               {};
    void DoInitAction()                 {};
    void DefineVideoStream();
    void DefineVideoFrame()             {};
    void DefineFontInfo2()              {};
    void EnableDebugger2()              {};
    void ScriptLimits()                 {};
    void SetTabIndex()                  {};
    void FileAttributes()               {};
    void PlaceObject3()                 {};
    void ImportAssets2()                {};
    void DefineFontAlignZones()         {};
    void CSMTextSettings()              {};
    void DefineFont3()                  {};
    void SymbolClass()                  {};
    void Metadata()                     {};
    void DefineScalingGrid()            {};
    void DoABC()                        {};
    void DefineShape4()                 {};
    void DefineMorphShape2()            {};
    void DefineSceneAndFrameLabelData() {};
    void DefineBinaryData()             {};
    void DefineFontName()               {};
    void StartSound2()                  {};

    //Helpers
    bool Decompress();
};

} //NameSpace

#endif
