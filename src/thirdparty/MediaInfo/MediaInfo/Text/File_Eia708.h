/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about PGS files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_Eia708H
#define MediaInfo_File_Eia708H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include <vector>
#include <string>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Eia708
//***************************************************************************

class File_Eia708 : public File__Analyze
{
public :
    //In
    int8u cc_type;
    float64 AspectRatio;

    //Constructor/Destructor
    File_Eia708();
    ~File_Eia708();

private :
    //Streams management
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();

    //Buffer - Global
    void Read_Buffer_Init();
    void Read_Buffer_Continue();
    void Read_Buffer_Unsynched();

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Temp
    struct character
    {
        wchar_t Value;
        int8u   Attribute;

        character()
            :
            Value(L' '),
            Attribute(0x00)
        {
        }
    };
    struct window
    {
        enum scroll_direction
        {
            Scroll_Direction_Unknown,
            Scroll_Direction_LeftToRight,
            Scroll_Direction_RightToLeft,
            Scroll_Direction_TopToBottom,
        };
        bool  visible;
        scroll_direction Scroll_Direction;
        int8u row_count;
        int8u column_count;
        bool  relative_positioning;
        int8u anchor_vertical;
        int8u anchor_horizontal;
        struct data
        {
            vector<vector<character> > CC;
            int8u Window_x; //x of the Windows, relative to the global area
            int8u Window_y;
            int8u x;
            int8u y;

            data()
            {
                Window_x=0;
                Window_y=0;
                x=0;
                y=0;
            }
        };
        data Minimal; //In characters
        int8u PenSize;

        window()
        {
            visible=false;
            Scroll_Direction=Scroll_Direction_Unknown;
            row_count=(int8u)-1;
            column_count=(int8u)-1;
            relative_positioning=false;
            anchor_vertical=(int8u)-1;
            anchor_horizontal=(int8u)-1;
            PenSize=1; //Standard
        }
    };
    struct stream
    {
        std::vector<window*> Windows;
        struct data
        {
            vector<vector<character> > CC;
        };
        data Minimal;
        int8u WindowID;

        stream()
        {
            WindowID=(int8u)-1;
        }

        ~stream()
        {
            for (size_t Pos=0; Pos<Windows.size(); Pos++)
                delete Windows[Pos]; //Windows[Pos]=NULL;
        }
    };
    std::vector<stream*> Streams;
    int8u service_number;
    int8u block_size;
    bool   HasContent;
    int64u DataDetected; //1 service per bit

    //Elements
    void NUL();                 //NUL
    void ETX();                 //End Of Text
    void BS();                  //Backspace
    void FF();                  //Form Feed
    void CR();                  //Carriage Return
    void HCR();                 //Horizontal Carriage Return
    void CWx(int8u WindowID);   //SetCurrentWindow
    void CLW();                 //ClearWindows
    void DSW();                 //DisplayWindows
    void HDW();                 //HideWindows
    void TGW();                 //ToggleWindows
    void DLW();                 //DeleteWindows
    void DLY();                 //Delay
    void DLC();                 //Delay Cancel
    void RST();                 //Reset
    void SPA();                 //SetPenAttributes
    void SPC();                 //SetPenColor
    void SPL();                 //SetPenLocation
    void SWA();                 //SetWindowAttributes
    void DFx(int8u WindowID);   //DefineWindow

    //Temp
    bool StandAloneCommand; //If this is a command simulated from another command

    //Helpers
    void Service();
    void Character_Fill(wchar_t Character);
    void HasChanged();
    void Window_HasChanged();
    void Illegal(int8u Size, int8u cc_data_1, int8u cc_data_2=(int8u)-1, int8u cc_data_3=(int8u)-1, int8u cc_data_4=(int8u)-1, int8u cc_data_5=(int8u)-1, int8u cc_data_6=(int8u)-1);
};

} //NameSpace

#endif
