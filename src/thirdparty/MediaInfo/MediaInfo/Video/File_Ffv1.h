/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about FFV1 files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_Ffv1H
#define MediaInfo_Ffv1H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class RangeCoder
//***************************************************************************

const size_t states_size=32;
const size_t state_transitions_size=256;
typedef int8u states[states_size];
typedef int8u state_transitions[state_transitions_size];

class RangeCoder
{
public :
    RangeCoder(const int8u* Buffer, size_t Buffer_Size, const state_transitions default_state_transition);

    bool    get_rac(int8u States[]);
    int8u   get_symbol_u(states &States);
    int8u   get_symbol_s(states &States);

    int16u Current;
    int16u Mask;
    state_transitions zero_state;
public : //Temp
    state_transitions one_state;
    const int8u* Buffer_Cur;
    const int8u* Buffer_End;

};

//***************************************************************************
// Class File_Ffv1
//***************************************************************************

class File_Ffv1 : public File__Analyze
{
public :
    //In
    bool    IsOutOfBandData;

    //Constructor/Destructor
    File_Ffv1();

private :
    //Streams management
    void Streams_Accept();

    //Buffer - Global
    void Read_Buffer_Continue();

    //Elements
    void FrameHeader();

    //Range coder
    #if MEDIAINFO_TRACE
        void Get_RC (states &state, bool  &Info, const char* Name);
        void Get_RU (states &State, int8u &Info, const char* Name);
        void Get_RS (states &State, int8s &Info, const char* Name);
        void Skip_RC(states &state,              const char* Name);
        void Skip_RU(states &State,              const char* Name);
        void Skip_RS(states &State,              const char* Name);
        #define Info_RC(_STATE, _INFO, _NAME) bool  _INFO; Get_RC (_STATE, _INFO, _NAME)
        #define Info_RU(_STATE, _INFO, _NAME) int8u _INFO; Get_RU (_STATE, _INFO, _NAME)
        #define Info_RS(_STATE, _INFO, _NAME) int8s _INFO; Get_RS (_STATE, _INFO, _NAME)
    #else //MEDIAINFO_TRACE
        void Get_RC_ (states &state, bool  &Info);
        void Get_RU_ (states &State, int8u &Info);
        void Get_RS_ (states &State, int8s &Info);
        #define Get_RC(Bits, Info, Name) Get_RC_(Bits, Info)
        #define Get_RU(Bits, Info, Name) Get_RU_(Bits, Info)
        #define Get_RS(Bits, Info, Name) Get_RS_(Bits, Info)
        void Skip_RC_(states &state);
        void Skip_RU_(states &State);
        void Skip_RS_(states &State);
        #define Skip_RC(Bits, Name) Skip_RC_(Bits)
        #define Skip_RU(Bits, Name) Skip_RU_(Bits)
        #define Skip_RS(Bits, Name) Skip_RS_(Bits)
        #define Info_RC(_STATE, _INFO, _NAME) Skip_RC_(_STATE)
        #define Info_RU(_STATE, _INFO, _NAME) Skip_RU_(_STATE)
        #define Info_RS(_STATE, _INFO, _NAME) Skip_RS_(_STATE)
    #endif //MEDIAINFO_TRACE
    RangeCoder* RC;
};

} //NameSpace

#endif
