/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef MediaInfo_File__AnalyzeMinimizeSizeH
#define MediaInfo_File__AnalyzeMinimizeSizeH
//---------------------------------------------------------------------------

//***************************************************************************
// Class File__Analyze
//***************************************************************************

class File__Analyze : public File__Base
{
public :
    //***************************************************************************
    // Constructor/Destructor
    //***************************************************************************

    File__Analyze();
    virtual ~File__Analyze();

    //***************************************************************************
    // Open
    //***************************************************************************

    void    Open_Buffer_Init        (                    int64u File_Size);
    void    Open_Buffer_Init        (File__Analyze* Sub);
    void    Open_Buffer_Init        (File__Analyze* Sub, int64u File_Size);
    void    Open_Buffer_Continue    (                    const int8u* Buffer, size_t Buffer_Size);
    void    Open_Buffer_Continue    (File__Analyze* Sub, const int8u* Buffer, size_t Buffer_Size, bool IsNewPacket=true, float64 Ratio=1.0);
    void    Open_Buffer_Continue    (File__Analyze* Sub, size_t Buffer_Size) {if (Element_Offset+Buffer_Size<=Element_Size) Open_Buffer_Continue(Sub, Buffer+Buffer_Offset+(size_t)Element_Offset, Buffer_Size); Element_Offset+=Buffer_Size;}
    void    Open_Buffer_Continue    (File__Analyze* Sub) {if (Element_Offset<=Element_Size) Open_Buffer_Continue(Sub, Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset)); Element_Offset=Element_Size;}
    void    Open_Buffer_Position_Set(int64u File_Offset);
    #if MEDIAINFO_SEEK
    size_t  Open_Buffer_Seek        (size_t Method, int64u Value, int64u ID);
    #endif //MEDIAINFO_SEEK
    void    Open_Buffer_Unsynch     ();
    void    Open_Buffer_Update      ();
    void    Open_Buffer_Update      (File__Analyze* Sub);
    void    Open_Buffer_Finalize    (bool NoBufferModification=false);
    void    Open_Buffer_Finalize    (File__Analyze* Sub);

    //***************************************************************************
    // In/Out (for parsers)
    //***************************************************************************

    //In
    Ztring ParserName;
    #if MEDIAINFO_EVENTS
        size_t  StreamIDs_Size;
        int64u  StreamIDs[16];
        int8u   StreamIDs_Width[16];
        int8u   ParserIDs[16];
        void    Event_Prepare (struct MediaInfo_Event_Generic* Event);
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        int8u   Demux_Level; //bit 0=frame, bit 1=container, bit 2=elementary (eg MPEG-TS), bit 3=ancillary (e.g. DTVCC), default with frame set
        bool    Demux_random_access;
        bool    Demux_UnpacketizeContainer;
        bool    Demux_IntermediateItemFound;
        size_t  Demux_Offset;
        int64u  Demux_TotalBytes;
        File__Analyze* Demux_CurrentParser;
    #endif //MEDIAINFO_DEMUX
    Ztring  File_Name_WithoutDemux;
    bool   PTS_DTS_Needed;
    struct frame_info
    {
        int64u Buffer_Offset_End;
        int64u PCR; //In nanoseconds
        int64u PTS; //In nanoseconds
        int64u DTS; //In nanoseconds
        int64u DUR; //In nanoseconds

        frame_info()
        {
            Buffer_Offset_End=(int64u)-1;
            PCR=(int64u)-1;
            PTS=(int64u)-1;
            DTS=(int64u)-1;
            DUR=(int64u)-1;
        }
    };
    frame_info FrameInfo;
    frame_info FrameInfo_Previous;
    frame_info FrameInfo_Next;
    std::vector<int64u> Offsets_Stream;
    std::vector<int64u> Offsets_Buffer;
    size_t              Offsets_Pos;
    int8u*              OriginalBuffer;
    size_t              OriginalBuffer_Size;
    size_t              OriginalBuffer_Capacity;

    //Out
    int64u PTS_Begin;                  //In nanoseconds
    int64u PTS_End;                    //In nanoseconds
    int64u DTS_Begin;                  //In nanoseconds
    int64u DTS_End;                    //In nanoseconds
    int64u Frame_Count;
    int64u Frame_Count_Previous;
    int64u Frame_Count_InThisBlock;
    int64u Field_Count;
    int64u Field_Count_Previous;
    int64u Field_Count_InThisBlock;
    int64u Frame_Count_NotParsedIncluded;
    int64u FrameNumber_PresentationOrder;
    bool   Synched;                    //Data is synched
    bool   UnSynched_IsNotJunk;        //Data is actually synched
    bool   MustExtendParsingDuration;  //Data has some substreams difficult to detect (e.g. captions), must wait a bit before final filling

protected :
    //***************************************************************************
    // Streams management
    //***************************************************************************

    virtual void Streams_Accept()                                               {};
    virtual void Streams_Fill()                                                 {};
    virtual void Streams_Update()                                               {};
    virtual void Streams_Finish()                                               {};

    //***************************************************************************
    // Synchro
    //***************************************************************************

    virtual bool Synchronize()    {Synched=true; return true;}; //Look for the synchro
    virtual bool Synched_Test()   {return true;}; //Test is synchro is OK
    virtual void Synched_Init()   {}; //When synched, we can Init data
    bool Synchro_Manage();
    bool Synchro_Manage_Test();

    //***************************************************************************
    // Buffer
    //***************************************************************************

    //Buffer
    virtual void Read_Buffer_Init ()          {}; //Temp, should be in File__Base caller
    virtual void Read_Buffer_Continue ()      {}; //Temp, should be in File__Base caller
    virtual void Read_Buffer_AfterParsing ()  {}; //Temp, should be in File__Base caller
    #if MEDIAINFO_SEEK
    virtual size_t Read_Buffer_Seek (size_t, int64u, int64u); //Temp, should be in File__Base caller
    size_t Read_Buffer_Seek_OneFramePerFile (size_t, int64u, int64u);
    #endif //MEDIAINFO_SEEK
    virtual void Read_Buffer_Unsynched ()     {}; //Temp, should be in File__Base caller
    void Read_Buffer_Unsynched_OneFramePerFile ();
    virtual void Read_Buffer_Finalize ()      {}; //Temp, should be in File__Base caller
    bool Buffer_Parse();

    //***************************************************************************
    // BitStream init
    //***************************************************************************

    void BS_Begin();
    void BS_Begin_LE(); //Little Endian version
    void BS_End();
    void BS_End_LE(); //Little Endian version

    //***************************************************************************
    // File Header
    //***************************************************************************

    //File Header - Management
    bool FileHeader_Manage ();

    //File Header - Begin
    virtual bool FileHeader_Begin ()                                            {return true;};

    //File Header - Parse
    virtual void FileHeader_Parse ()                                            {Element_DoNotShow();};

    //***************************************************************************
    // Header
    //***************************************************************************

    //Header - Management
    bool Header_Manage ();

    //Header - Begin
    virtual bool Header_Begin ()                                                {return true;};

    //Header - Parse
    virtual void Header_Parse ();

    //Header - Info
    void Header_Fill_Code (int64u Code);
    inline void Header_Fill_Code (int64u Code, const Ztring &) {Header_Fill_Code(Code);};
    #define Header_Fill_Code2(A,B) Header_Fill_Code(A)
    void Header_Fill_Size (int64u Size);

    //***************************************************************************
    // Data
    //***************************************************************************

    //Header - Management
    bool Data_Manage ();

    //Data - Parse
    virtual void Data_Parse ()                                                  {};

    //Data - Info
    inline void Data_Info (const Ztring &)                                      {};
    inline void Data_Info_From_Milliseconds (int64u)                            {}

    //Data - Get info
    size_t Data_Remain ()                                                       {return (size_t)(Element_Size-(Element_Offset+BS->Offset_Get()));};
    size_t Data_BS_Remain ()                                                    {return (size_t)BS->Remain();};

    //Data - Detect EOF
    virtual void Detect_EOF ()                                                  {};
    bool EOF_AlreadyDetected;

    //Data - Helpers
    void Data_Accept        (const char*)                                       {Accept();}
    void Data_Accept        ()                                                  {Accept();}
    void Data_Reject        (const char*)                                       {Reject();}
    void Data_Reject        ()                                                  {Reject();}
    void Data_Finish        (const char*)                                       {ForceFinish();}
    void Data_Finish        ()                                                  {ForceFinish();}
    void Data_GoTo          (int64u GoTo_, const char*)                         {GoTo(GoTo_);}
    void Data_GoTo          (int64u GoTo_)                                      {GoTo(GoTo_);}
    void Data_GoToFromEnd   (int64u GoToFromEnd_, const char*)                  {GoToFromEnd(GoToFromEnd_);}
    void Data_GoToFromEnd   (int64u GoToFromEnd_)                               {GoToFromEnd(GoToFromEnd_);}

    //***************************************************************************
    // Elements
    //***************************************************************************

    //Elements - Begin
    void Element_Begin ();
    #define Element_Begin0() Element_Begin()
    #define Element_Begin1(_NAME) Element_Begin()
    #define Element_Trace_Begin0()
    #define Element_Trace_Begin1(_NAME)

    //Elements - Name
    #define Element_Name(_A)

    //Elements - Info
    #define Element_Info1(_A)
    #define Element_Info2(_A,_B)
    #define Element_Info3(_A,_B,_C)
    #define Element_Info1C(_CONDITION,_A)
    #define Element_Info_From_Milliseconds(_A)

    //Elements - End
    inline void Element_End () {Element_End_Common_Flush();}
    #define Element_End0() Element_End()
    #define Element_End1(_NAME) Element_End()
    #define Element_Trace_End0()
    #define Element_Trace_End1(_NAME)

    //Elements - Preparation of element from external app
    void Element_Prepare (int64u Size);

protected :
    //Element - Common
    void   Element_End_Common_Flush();
    Ztring Element_End_Common_Flush_Build();
public :

    //***************************************************************************
    // Param
    //***************************************************************************

    //Param - Main
    #define Param1(_A)
    #define Param2(_A,_B)
    #define Param3(_A,_B,_C)

    //Param - Info
    #define Param_Info1(_A)
    #define Param_Info2(_A,_B)
    #define Param_Info3(_A,_B,_C)
    #define Param_Info1C(_CONDITION,_A)
    #define Param_Info2C(_CONDITION,_A,_B)
    #define Param_Info3C(_CONDITION,_A,_B,_C)
    #define Param_Info_From_Milliseconds(A)

    //***************************************************************************
    // Information
    //***************************************************************************

    inline void Info (const Ztring&, size_t =0) {}

    //***************************************************************************
    // Big Endian (Integer, Float, Fixed-Point)
    //***************************************************************************

    void Get_B1_   (int8u   &Info);
    void Get_B2_   (int16u  &Info);
    void Get_B3_   (int32u  &Info);
    void Get_B4_   (int32u  &Info);
    void Get_B5_   (int64u  &Info);
    void Get_B6_   (int64u  &Info);
    void Get_B7_   (int64u  &Info);
    void Get_B8_   (int64u  &Info);
    void Get_B16_  (int128u &Info);
    void Get_BF4_  (float32 &Info);
    void Get_BF8_  (float64 &Info);
    void Get_BF10_ (float80 &Info);
    void Get_BFP4_ (int8u  Bits, float32 &Info);
    #define Get_B1(Info, Name) Get_B1_(Info)
    #define Get_B2(Info, Name) Get_B2_(Info)
    #define Get_B3(Info, Name) Get_B3_(Info)
    #define Get_B4(Info, Name) Get_B4_(Info)
    #define Get_B5(Info, Name) Get_B5_(Info)
    #define Get_B6(Info, Name) Get_B6_(Info)
    #define Get_B7(Info, Name) Get_B7_(Info)
    #define Get_B8(Info, Name) Get_B8_(Info)
    #define Get_B16(Info, Name) Get_B16_(Info)
    #define Get_BF4(Info, Name) Get_BF4_(Info)
    #define Get_BF8(Info, Name) Get_BF8_(Info)
    #define Get_BF10(Info, Name) Get_BF10_(Info)
    #define Get_BFP4(Bits, Info, Name) Get_BFP4_(Bits, Info)
    void Peek_B1  (int8u   &Info);
    void Peek_B2  (int16u  &Info);
    void Peek_B3  (int32u  &Info);
    void Peek_B4  (int32u  &Info);
    void Peek_B5  (int64u  &Info);
    void Peek_B6  (int64u  &Info);
    void Peek_B7  (int64u  &Info);
    void Peek_B8  (int64u  &Info);
    void Peek_B16 (int128u &Info);
    void Peek_BF4 (float32 &Info);
    void Peek_BF8 (float64 &Info);
    void Peek_BF10(float64 &Info);
    void Peek_BFP4(int8u  Bits, float64 &Info);
    #define Skip_B1(Name) Element_Offset++
    #define Skip_B2(Name) Element_Offset+=2
    #define Skip_B3(Name) Element_Offset+=3
    #define Skip_B4(Name) Element_Offset+=4
    #define Skip_B5(Name) Element_Offset+=5
    #define Skip_B6(Name) Element_Offset+=6
    #define Skip_B7(Name) Element_Offset+=7
    #define Skip_B8(Name) Element_Offset+=8
    #define Skip_BF4(Name) Element_Offset+=4
    #define Skip_BF8(Name) Element_Offset+=8
    #define Skip_B16(Name) Element_Offset+=16
    #define Skip_BFP4(Size, Name) Element_Offset+=4
    #define Info_B1(_INFO, _NAME)   Element_Offset++
    #define Info_B2(_INFO, _NAME)   Element_Offset+=2
    #define Info_B3(_INFO, _NAME)   Element_Offset+=3
    #define Info_B4(_INFO, _NAME)   Element_Offset+=4
    #define Info_B5(_INFO, _NAME)   Element_Offset+=5
    #define Info_B6(_INFO, _NAME)   Element_Offset+=6
    #define Info_B7(_INFO, _NAME)   Element_Offset+=7
    #define Info_B8(_INFO, _NAME)   Element_Offset+=8
    #define Info_B16(_INFO, _NAME)  Element_Offset+=16
    #define Info_BF4(_INFO, _NAME)  Element_Offset+=4
    #define Info_BF8(_INFO, _NAME)  Element_Offset+=8
    #define Info_BF10(_INFO, _NAME) Element_Offset+=10
    #define Info_BFP4(_BITS, _INFO, _NAME) Element_Offset+=4

    //***************************************************************************
    // Little Endian
    //***************************************************************************

    void Get_L1_   (int8u   &Info);
    void Get_L2_   (int16u  &Info);
    void Get_L3_   (int32u  &Info);
    void Get_L4_   (int32u  &Info);
    void Get_L5_   (int64u  &Info);
    void Get_L6_   (int64u  &Info);
    void Get_L7_   (int64u  &Info);
    void Get_L8_   (int64u  &Info);
    void Get_L16_  (int128u &Info);
    void Get_LF4_  (float32 &Info);
    void Get_LF8_  (float64 &Info);
    void Get_LF10_ (float80 &Info);
    void Get_LFP4_ (int8u  Bits, float32 &Info);
    #define Get_L1(Info, Name) Get_L1_(Info)
    #define Get_L2(Info, Name) Get_L2_(Info)
    #define Get_L3(Info, Name) Get_L3_(Info)
    #define Get_L4(Info, Name) Get_L4_(Info)
    #define Get_L5(Info, Name) Get_L5_(Info)
    #define Get_L6(Info, Name) Get_L6_(Info)
    #define Get_L7(Info, Name) Get_L7_(Info)
    #define Get_L8(Info, Name) Get_L8_(Info)
    #define Get_L16(Info, Name) Get_L16_(Info)
    #define Get_LF4(Info, Name) Get_LF4_(Info)
    #define Get_LF8(Info, Name) Get_LF8_(Info)
    #define Get_LF10(Info, Name) Get_LF10_(Info)
    #define Get_LFP4(Bits, Info, Name) Get_LFP4_(Bits, Info)
    void Peek_L1  (int8u   &Info);
    void Peek_L2  (int16u  &Info);
    void Peek_L3  (int32u  &Info);
    void Peek_L4  (int32u  &Info);
    void Peek_L5  (int64u  &Info);
    void Peek_L6  (int64u  &Info);
    void Peek_L7  (int64u  &Info);
    void Peek_L8  (int64u  &Info);
    void Peek_L16 (int128u &Info);
    void Peek_LF4 (float32 &Info);
    void Peek_LF8 (float64 &Info);
    void Peek_LF10(float64 &Info);
    void Peek_LFP4(int8u  Bits, float64 &Info);
    #define Skip_L1(Name) Element_Offset++
    #define Skip_L2(Name) Element_Offset+=2
    #define Skip_L3(Name) Element_Offset+=3
    #define Skip_L4(Name) Element_Offset+=4
    #define Skip_L5(Name) Element_Offset+=5
    #define Skip_L6(Name) Element_Offset+=6
    #define Skip_L7(Name) Element_Offset+=7
    #define Skip_L8(Name) Element_Offset+=8
    #define Skip_LF4(Name) Element_Offset+=4
    #define Skip_LF8(Name) Element_Offset+=8
    #define Skip_L16(Name) Element_Offset+=16
    #define Skip_LFP4(Size, Name) Element_Offset+=4
    #define Info_L1(_INFO, _NAME)   Element_Offset++
    #define Info_L2(_INFO, _NAME)   Element_Offset+=2
    #define Info_L3(_INFO, _NAME)   Element_Offset+=3
    #define Info_L4(_INFO, _NAME)   Element_Offset+=4
    #define Info_L5(_INFO, _NAME)   Element_Offset+=5
    #define Info_L6(_INFO, _NAME)   Element_Offset+=6
    #define Info_L7(_INFO, _NAME)   Element_Offset+=7
    #define Info_L8(_INFO, _NAME)   Element_Offset+=8
    #define Info_L16(_INFO, _NAME)  Element_Offset+=16
    #define Info_LF4(_INFO, _NAME)  Element_Offset+=4
    #define Info_LF8(_INFO, _NAME)  Element_Offset+=8
    #define Info_LF10(_INFO, _NAME) Element_Offset+=10
    #define Info_LFP4(_LITS, _INFO, _NAME) Element_Offset+=4

    //***************************************************************************
    // Little and Big Endian together
    //***************************************************************************

    void Get_D1_   (int8u   &Info);
    void Get_D2_   (int16u  &Info);
    void Get_D3_   (int32u  &Info);
    void Get_D4_   (int32u  &Info);
    void Get_D5_   (int64u  &Info);
    void Get_D6_   (int64u  &Info);
    void Get_D7_   (int64u  &Info);
    void Get_D8_   (int64u  &Info);
    void Get_D16_  (int128u &Info);
    void Get_DF4_  (float32 &Info);
    void Get_DF8_  (float64 &Info);
    void Get_DF10_ (float80 &Info);
    void Get_DFP4_ (int8u  Bits, float32 &Info);
    #define Get_D1(Info, Name) Get_D1_(Info)
    #define Get_D2(Info, Name) Get_D2_(Info)
    #define Get_D3(Info, Name) Get_D3_(Info)
    #define Get_D4(Info, Name) Get_D4_(Info)
    #define Get_D5(Info, Name) Get_D5_(Info)
    #define Get_D6(Info, Name) Get_D6_(Info)
    #define Get_D7(Info, Name) Get_D7_(Info)
    #define Get_D8(Info, Name) Get_D8_(Info)
    #define Get_D16(Info, Name) Get_D16_(Info)
    #define Get_DF4(Info, Name) Get_DF4_(Info)
    #define Get_DF8(Info, Name) Get_DF8_(Info)
    #define Get_DF10(Info, Name) Get_DF10_(Info)
    #define Get_DFP4(Bits, Info, Name) Get_DFP4_(Bits, Info)
    void Peek_D1  (int8u   &Info);
    void Peek_D2  (int16u  &Info);
    void Peek_D3  (int32u  &Info);
    void Peek_D4  (int32u  &Info);
    void Peek_D5  (int64u  &Info);
    void Peek_D6  (int64u  &Info);
    void Peek_D7  (int64u  &Info);
    void Peek_D8  (int64u  &Info);
    void Peek_D16 (int128u &Info);
    void Peek_DF4 (float32 &Info);
    void Peek_DF8 (float64 &Info);
    void Peek_DF10(float64 &Info);
    void Peek_DFP4(int8u  Bits, float64 &Info);
    #define Skip_D1(Name) Element_Offset++
    #define Skip_D2(Name) Element_Offset+=2
    #define Skip_D3(Name) Element_Offset+=3
    #define Skip_D4(Name) Element_Offset+=4
    #define Skip_D5(Name) Element_Offset+=5
    #define Skip_D6(Name) Element_Offset+=6
    #define Skip_D7(Name) Element_Offset+=7
    #define Skip_D8(Name) Element_Offset+=8
    #define Skip_DF4(Name) Element_Offset+=4
    #define Skip_DF8(Name) Element_Offset+=8
    #define Skip_D16(Name) Element_Offset+=16
    #define Skip_DFP4(Size, Name) Element_Offset+=4
    #define Info_D1(_INFO, _NAME)   Element_Offset++
    #define Info_D2(_INFO, _NAME)   Element_Offset+=2
    #define Info_D3(_INFO, _NAME)   Element_Offset+=3
    #define Info_D4(_INFO, _NAME)   Element_Offset+=4
    #define Info_D5(_INFO, _NAME)   Element_Offset+=5
    #define Info_D6(_INFO, _NAME)   Element_Offset+=6
    #define Info_D7(_INFO, _NAME)   Element_Offset+=7
    #define Info_D8(_INFO, _NAME)   Element_Offset+=8
    #define Info_D16(_INFO, _NAME)  Element_Offset+=16
    #define Info_DF4(_INFO, _NAME)  Element_Offset+=4
    #define Info_DF8(_INFO, _NAME)  Element_Offset+=8
    #define Info_DF10(_INFO, _NAME) Element_Offset+=10
    #define Info_DFP4(_DITS, _INFO, _NAME) Element_Offset+=4

    //***************************************************************************
    // GUID
    //***************************************************************************

    void Get_GUID (int128u &Info);
    inline void Get_GUID (int128u &Info, const char*) {Get_GUID(Info);}
    void Peek_GUID(int128u &Info);
    inline void Skip_GUID(               const char*) {if (Element_Offset+16>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=16;}
    #define Info_GUID(_INFO, _NAME) int128u _INFO; Get_GUID(_INFO, _NAME)

    //***************************************************************************
    // UUID
    //***************************************************************************

    inline void Get_UUID (int128u &Info) {Get_B16_(Info);}
    inline void Get_UUID (int128u &Info, const char*) {Get_B16_(Info);}
    inline void Peek_UUID(int128u &Info) {Peek_B16(Info);}
    inline void Skip_UUID(               const char*) {if (Element_Offset+16>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=16;}
    #define Info_UUID(_INFO, _NAME) int128u _INFO; Get_UUID(_INFO, _NAME)

    //***************************************************************************
    // EBML
    //***************************************************************************

    void Get_EB (int64u &Info);
    void Get_ES (int64s &Info);
    inline void Get_EB (int64u &Info, const char*) {Get_EB(Info);}
    inline void Get_ES (int64s &Info, const char*) {Get_ES(Info);}
    void Skip_EB();
    void Skip_ES();
    inline void Skip_EB(              const char*) {Skip_EB();}
    inline void Skip_ES(              const char*) {Skip_ES();}
    #define Info_EB(_INFO, _NAME) int64u _INFO; Get_EB(_INFO, _NAME)
    #define Info_ES(_INFO, _NAME) int64s _INFO; Get_ES(_INFO, _NAME)

    //***************************************************************************
    // Variable Size Value
    //***************************************************************************

    void Get_VS (int64u &Info);
    inline void Get_VS (int64u &Info, const char*) {Get_VS(Info);}
    void Skip_VS();
    inline void Skip_VS(              const char*) {Skip_VS();}
    #define Info_VS(_INFO, _NAME) int64u _INFO; Get_VS(_INFO, _NAME)

    //***************************************************************************
    // Exp-Golomb
    //***************************************************************************

    void Get_UE (int32u &Info);
    void Get_SE (int32s &Info);
    inline void Get_UE (int32u &Info, const char*) {Get_UE(Info);}
    inline void Get_SE (int32s &Info, const char*) {Get_SE(Info);}
    void Skip_UE();
    inline void Skip_SE() {Skip_UE();}
    inline void Skip_UE(              const char*) {Skip_UE();}
    inline void Skip_SE(              const char*) {Skip_SE();}
    #define Info_UE(_INFO, _NAME) int32u _INFO; Get_UE(_INFO, _NAME)
    #define Info_SE(_INFO, _NAME) int32s _INFO; Get_SE(_INFO, _NAME)

    //***************************************************************************
    // Interleaved Exp-Golomb
    //***************************************************************************

    void Get_UI (int32u &Info);
    void Get_SI (int32s &Info);
    inline void Get_UI (int32u &Info, const char*) {Get_UI(Info);}
    inline void Get_SI (int32s &Info, const char*) {Get_SI(Info);}
    void Skip_UI();
    void Skip_SI();
    inline void Skip_UI(              const char*) {Skip_UI();}
    inline void Skip_SI(              const char*) {Skip_SI();}
    #define Info_UI(_INFO, _NAME) int32u _INFO; Get_UI(_INFO, _NAME)
    #define Info_SI(_INFO, _NAME) int32s _INFO; Get_SI(_INFO, _NAME)

    //***************************************************************************
    // Variable Length Code
    //***************************************************************************

    struct vlc
    {
        int32u  value;
        int8u   bit_increment;
        int8s   mapped_to1;
        int8s   mapped_to2;
        int8s   mapped_to3;
    };
    struct vlc_fast
    {
        int8u*      Array;
        int8u*      BitsToSkip;
        const vlc*  Vlc;
        int8u       Size;
    };
    #define VLC_END \
        {(int32u)-1, (int8u)-1, 0, 0, 0}
    void Get_VL_Prepare(vlc_fast &Vlc);
    void Get_VL_ (const vlc Vlc[], size_t &Info);
    void Get_VL_ (const vlc_fast &Vlc, size_t &Info);
    #define Get_VL(Vlc, Info, Name) Get_VL_(Vlc, Info);
    void Skip_VL_(const vlc Vlc[]);
    void Skip_VL_(const vlc_fast &Vlc);
    #define Skip_VL(Vlc, Name) Skip_VL_(Vlc);
    #define Info_VL(Vlc, Info, Name) Skip_VL_(Vlc)

    //***************************************************************************
    // Characters
    //***************************************************************************

    void Get_C1 (int8u  &Info);
    void Get_C2 (int16u &Info);
    void Get_C3 (int32u &Info);
    void Get_C4 (int32u &Info);
    void Get_C5 (int64u &Info);
    void Get_C6 (int64u &Info);
    void Get_C7 (int64u &Info);
    void Get_C8 (int64u &Info);
    inline void Get_C1 (int8u  &Info, const char*) {Get_C1(Info);}
    inline void Get_C2 (int16u &Info, const char*) {Get_C2(Info);}
    inline void Get_C3 (int32u &Info, const char*) {Get_C3(Info);}
    inline void Get_C4 (int32u &Info, const char*) {Get_C4(Info);}
    inline void Get_C5 (int64u &Info, const char*) {Get_C5(Info);}
    inline void Get_C6 (int64u &Info, const char*) {Get_C6(Info);}
    inline void Get_C7 (int64u &Info, const char*) {Get_C7(Info);}
    inline void Get_C8 (int64u &Info, const char*) {Get_C8(Info);}
    inline void Skip_C1(              const char*) {if (Element_Offset+1>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=1;}
    inline void Skip_C2(              const char*) {if (Element_Offset+2>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=2;}
    inline void Skip_C3(              const char*) {if (Element_Offset+3>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=3;}
    inline void Skip_C4(              const char*) {if (Element_Offset+4>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=4;}
    inline void Skip_C5(              const char*) {if (Element_Offset+5>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=5;}
    inline void Skip_C6(              const char*) {if (Element_Offset+6>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=6;}
    inline void Skip_C7(              const char*) {if (Element_Offset+7>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=7;}
    inline void Skip_C8(              const char*) {if (Element_Offset+8>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=8;}
    #define Info_C1(_INFO, _NAME) int8u  _INFO; Get_C1(_INFO, _NAME)
    #define Info_C2(_INFO, _NAME) int16u _INFO; Get_C2(_INFO, _NAME)
    #define Info_C3(_INFO, _NAME) int32u _INFO; Get_C3(_INFO, _NAME)
    #define Info_C4(_INFO, _NAME) int32u _INFO; Get_C4(_INFO, _NAME)
    #define Info_C5(_INFO, _NAME) int64u _INFO; Get_C5(_INFO, _NAME)
    #define Info_C6(_INFO, _NAME) int64u _INFO; Get_C6(_INFO, _NAME)
    #define Info_C7(_INFO, _NAME) int64u _INFO; Get_C7(_INFO, _NAME)
    #define Info_C8(_INFO, _NAME) int64u _INFO; Get_C8(_INFO, _NAME)

    //***************************************************************************
    // Text
    //***************************************************************************

    void Get_Local  (int64u Bytes, Ztring      &Info);
    void Get_ISO_6937_2 (int64u Bytes, Ztring  &Info);
    void Get_ISO_8859_1 (int64u Bytes, Ztring  &Info);
    void Get_ISO_8859_2 (int64u Bytes, Ztring  &Info);
    void Get_ISO_8859_5 (int64u Bytes, Ztring  &Info);
    void Get_String (int64u Bytes, std::string &Info);
    void Get_UTF8   (int64u Bytes, Ztring      &Info);
    void Get_UTF16  (int64u Bytes, Ztring      &Info);
    void Get_UTF16B (int64u Bytes, Ztring      &Info);
    void Get_UTF16L (int64u Bytes, Ztring      &Info);
    inline void Get_Local  (int64u Bytes, Ztring      &Info, const char*) {Get_Local(Bytes, Info);}
    inline void Get_ISO_6937_2 (int64u Bytes, Ztring  &Info, const char*) {Get_ISO_8859_1(Bytes, Info);}
    inline void Get_ISO_8859_1 (int64u Bytes, Ztring  &Info, const char*) {Get_ISO_8859_1(Bytes, Info);}
    inline void Get_ISO_8859_2 (int64u Bytes, Ztring  &Info, const char*) {Get_ISO_8859_2(Bytes, Info);}
    inline void Get_ISO_8859_5 (int64u Bytes, Ztring  &Info, const char*) {Get_ISO_8859_5(Bytes, Info);}
    inline void Get_String (int64u Bytes, std::string &Info, const char*) {Get_String(Bytes, Info);}
    inline void Get_UTF8   (int64u Bytes, Ztring      &Info, const char*) {Get_UTF8(Bytes, Info);}
    inline void Get_UTF16  (int64u Bytes, Ztring      &Info, const char*) {Get_UTF16(Bytes, Info);}
    inline void Get_UTF16B (int64u Bytes, Ztring      &Info, const char*) {Get_UTF16B(Bytes, Info);}
    inline void Get_UTF16L (int64u Bytes, Ztring      &Info, const char*) {Get_UTF16L(Bytes, Info);}
    void Peek_Local (int64u Bytes, Ztring      &Info);
    void Peek_String(int64u Bytes, std::string &Info);
    void Skip_Local (int64u Bytes,                    const char*) {if (Element_Offset+Bytes>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=(size_t)Bytes;}
    void Skip_ISO_6937_2(int64u Bytes,                const char*) {if (Element_Offset+Bytes>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=(size_t)Bytes;}
    void Skip_String(int64u Bytes,                    const char*) {if (Element_Offset+Bytes>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=(size_t)Bytes;}
    void Skip_UTF8  (int64u Bytes,                    const char*) {if (Element_Offset+Bytes>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=(size_t)Bytes;}
    void Skip_UTF16B(int64u Bytes,                    const char*) {if (Element_Offset+Bytes>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=(size_t)Bytes;}
    void Skip_UTF16L(int64u Bytes,                    const char*) {if (Element_Offset+Bytes>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=(size_t)Bytes;}
    #define Info_Local(_BYTES, _INFO, _NAME)  Ztring _INFO; Get_Local (_BYTES, _INFO, _NAME)
    #define Info_ISO_6937_2(_BYTES, _INFO, _NAME)  Ztring _INFO; Info_ISO_6937_2(_BYTES, _INFO, _NAME)
    #define Info_UTF8(_BYTES, _INFO, _NAME)   Ztring _INFO; Get_UTF8  (_BYTES, _INFO, _NAME)
    #define Info_UTF16B(_BYTES, _INFO, _NAME) Ztring _INFO; Get_UTF16B(_BYTES, _INFO, _NAME)
    #define Info_UTF16L(_BYTES, _INFO, _NAME) Ztring _INFO; Get_UTF16L(_BYTES, _INFO, _NAME)

    //***************************************************************************
    // Pascal strings
    //***************************************************************************

    void Get_PA (std::string &Info);
    inline void Get_PA (std::string &Info, const char*) {Get_PA(Info);}
    void Peek_PA(std::string &Info);
    void Skip_PA();
    inline void Skip_PA(                   const char*) {Skip_PA();}
    #define Info_PA(_INFO, _NAME) Ztring _INFO; Get_PA (_INFO, _NAME)

    //***************************************************************************
    // Unknown
    //***************************************************************************

    void Skip_XX(int64u Bytes);
    inline void Skip_XX(int64u Bytes, const char*) {Skip_XX(Bytes);}

    //***************************************************************************
    // Flags
    //***************************************************************************

    void Get_Flags_ (int64u Flags, size_t Order, bool  &Info);
    void Get_FlagsM_ (int64u ValueToPut, int8u &Info); //Multiple
    #define Get_Flags(_FLAGS, _ORDER, _INFO, _NAME) Get_Flags_(_FLAGS, _ORDER, _INFO)
    #define Get_FlagsM(_VALUE, _INFO, _NAME) Get_FlagsM_(_VALUE, _INFO)
    #define Skip_Flags(_FLAGS, _ORDER, _NAME)
    #define Skip_FlagsM(_VALUE, _NAME)
    #define Info_Flags(_FLAGS, _ORDER, _INFO, _NAME)

    //***************************************************************************
    // BitStream
    //***************************************************************************

    void Get_BS_ (int8u  Bits, int32u  &Info);
    void Get_SB_ (             bool    &Info);
    bool Get_SB_ ()                                              {return BS->GetB();}
    void Get_S1_ (int8u  Bits, int8u   &Info);
    void Get_S2_ (int8u  Bits, int16u  &Info);
    void Get_S3_ (int8u  Bits, int32u  &Info);
    void Get_S4_ (int8u  Bits, int32u  &Info);
    void Get_S5_ (int8u  Bits, int64u  &Info);
    void Get_S6_ (int8u  Bits, int64u  &Info);
    void Get_S7_ (int8u  Bits, int64u  &Info);
    void Get_S8_ (int8u  Bits, int64u  &Info);
    #define Get_BS(Bits, Info, Name) Get_BS_(Bits, Info)
    #define Get_SB(      Info, Name) Get_SB_(      Info)
    #define Get_S1(Bits, Info, Name) Get_S1_(Bits, Info)
    #define Get_S2(Bits, Info, Name) Get_S2_(Bits, Info)
    #define Get_S3(Bits, Info, Name) Get_S3_(Bits, Info)
    #define Get_S4(Bits, Info, Name) Get_S4_(Bits, Info)
    #define Get_S5(Bits, Info, Name) Get_S5_(Bits, Info)
    #define Get_S6(Bits, Info, Name) Get_S6_(Bits, Info)
    #define Get_S7(Bits, Info, Name) Get_S7_(Bits, Info)
    #define Get_S8(Bits, Info, Name) Get_S8_(Bits, Info)
    void Peek_BS(int8u  Bits, int32u  &Info);
    void Peek_SB(              bool    &Info);
    bool Peek_SB()                                              {return BS->PeekB();}
    void Peek_S1(int8u  Bits, int8u   &Info);
    void Peek_S2(int8u  Bits, int16u  &Info);
    void Peek_S3(int8u  Bits, int32u  &Info);
    void Peek_S4(int8u  Bits, int32u  &Info);
    void Peek_S5(int8u  Bits, int64u  &Info);
    void Peek_S6(int8u  Bits, int64u  &Info);
    void Peek_S7(int8u  Bits, int64u  &Info);
    void Peek_S8(int8u  Bits, int64u  &Info);
    inline void Skip_BS_(size_t Bits) {BS->Skip(Bits);}
    inline void Skip_SB_(           ) {BS->Skip(1);}
    inline void Skip_S1_(int8u  Bits) {BS->Skip(Bits);}
    inline void Skip_S2_(int8u  Bits) {BS->Skip(Bits);}
    inline void Skip_S3_(int8u  Bits) {BS->Skip(Bits);}
    inline void Skip_S4_(int8u  Bits) {BS->Skip(Bits);}
    inline void Skip_S5_(int8u  Bits) {BS->Skip(Bits);}
    inline void Skip_S6_(int8u  Bits) {BS->Skip(Bits);}
    inline void Skip_S7_(int8u  Bits) {BS->Skip(Bits);}
    inline void Skip_S8_(int8u  Bits) {BS->Skip(Bits);}
    #define Skip_BS(Bits, Name) Skip_BS_(Bits)
    #define Skip_SB(      Name) Skip_SB_()
    #define Skip_S1(Bits, Name) Skip_S1_(Bits)
    #define Skip_S2(Bits, Name) Skip_S2_(Bits)
    #define Skip_S3(Bits, Name) Skip_S3_(Bits)
    #define Skip_S4(Bits, Name) Skip_S4_(Bits)
    #define Skip_S5(Bits, Name) Skip_S5_(Bits)
    #define Skip_S6(Bits, Name) Skip_S6_(Bits)
    #define Skip_S7(Bits, Name) Skip_S7_(Bits)
    #define Skip_S8(Bits, Name) Skip_S8_(Bits)
    void Mark_0 ();
    #define Mark_0_NoTrustError() Skip_SB_() //Use it for providing a warning instead of a non-trusting error
    void Mark_1 ();
    #define Mark_1_NoTrustError() Skip_SB_() //Use it for providing a warning instead of a non-trusting error
    #define Info_BS(_BITS, _INFO, _NAME) Skip_BS_(_BITS)
    #define Info_SB(_INFO, _NAME)        Skip_SB_(     )
    #define Info_S1(_BITS, _INFO, _NAME) Skip_S1_(_BITS)
    #define Info_S2(_BITS, _INFO, _NAME) Skip_S2_(_BITS)
    #define Info_S3(_BITS, _INFO, _NAME) Skip_S3_(_BITS)
    #define Info_S4(_BITS, _INFO, _NAME) Skip_S4_(_BITS)
    #define Info_S5(_BITS, _INFO, _NAME) Skip_S5_(_BITS)
    #define Info_S6(_BITS, _INFO, _NAME) Skip_S6_(_BITS)
    #define Info_S7(_BITS, _INFO, _NAME) Skip_S7_(_BITS)
    #define Info_S8(_BITS, _INFO, _NAME) Skip_S8_(_BITS)

    #define TEST_SB_GET(_CODE, _NAME) \
        { \
            Peek_SB(_CODE); \
            if (!_CODE) \
                Skip_SB_(); \
            else \
            { \
                Element_Begin0(); \
                Skip_SB_(); \

    #define TEST_SB_SKIP(_NAME) \
        { \
            if (!Peek_SB()) \
                Skip_SB_(); \
            else \
            { \
                Element_Begin0(); \
                Skip_SB_(); \

    #define TESTELSE_SB_GET(_CODE, _NAME) \
        { \
            Peek_SB(_CODE); \
            if (_CODE) \
            { \
                Element_Begin0(); \
                Skip_SB_(); \

    #define TESTELSE_SB_SKIP(_NAME) \
        { \
            if (Peek_SB()) \
            { \
                Element_Begin0(); \
                Skip_SB_(); \

    #define TESTELSE_SB_ELSE(_NAME) \
                Element_End0(); \
            } \
            else \
            { \
                Skip_SB_(); \

    #define TESTELSE_SB_END() \
            } \
        } \

    #define TEST_SB_END() \
                Element_End0(); \
            } \
        } \

    //***************************************************************************
    // BitStream (Little Endian)
    //***************************************************************************

    void Get_BT_ (int8u  Bits, int32u  &Info);
    void Get_TB_ (             bool    &Info);
    bool Get_TB_ ()                                              {bool Temp; Get_TB_(Temp); return Temp;}
    void Get_T1_ (int8u  Bits, int8u   &Info);
    void Get_T2_ (int8u  Bits, int16u  &Info);
    void Get_T4_ (int8u  Bits, int32u  &Info);
    void Get_T8_ (int8u  Bits, int64u  &Info);
    #define Get_BT(Bits, Info, Name) Get_BT_(Bits, Info)
    #define Get_TB(      Info, Name) Get_TB_(      Info)
    #define Get_T1(Bits, Info, Name) Get_T1_(Bits, Info)
    #define Get_T2(Bits, Info, Name) Get_T2_(Bits, Info)
    #define Get_T4(Bits, Info, Name) Get_T4_(Bits, Info)
    #define Get_T8(Bits, Info, Name) Get_T8_(Bits, Info)
    void Peek_BT(int8u  Bits, int32u  &Info);
    void Peek_TB(              bool    &Info);
    bool Peek_TB()                                              {bool Temp; Peek_TB(Temp); return Temp;}
    void Peek_T1(int8u  Bits, int8u   &Info);
    void Peek_T2(int8u  Bits, int16u  &Info);
    void Peek_T4(int8u  Bits, int32u  &Info);
    void Peek_T8(int8u  Bits, int64u  &Info);
    inline void Skip_BT_(size_t Bits) {BT->Skip(Bits);}
    inline void Skip_TB_(           ) {BT->SkipB();}
    inline void Skip_T1_(int8u  Bits) {BT->Skip1(Bits);}
    inline void Skip_T2_(int8u  Bits) {BT->Skip2(Bits);}
    inline void Skip_T4_(int8u  Bits) {BT->Skip4(Bits);}
    inline void Skip_T8_(int8u  Bits) {BT->Skip8(Bits);}
    #define Skip_BT(Bits, Name) Skip_BT_(Bits)
    #define Skip_TB(      Name) Skip_TB_()
    #define Skip_T1(Bits, Name) Skip_T1_(Bits)
    #define Skip_T2(Bits, Name) Skip_T2_(Bits)
    #define Skip_T4(Bits, Name) Skip_T4_(Bits)
    #define Skip_T8(Bits, Name) Skip_T8_(Bits)
    #define Info_BT(_BITS, _INFO, _NAME) Skip_BT_(_BITS)
    #define Info_TB(_INFO, _NAME)        Skip_TB_(     )
    #define Info_T1(_BITS, _INFO, _NAME) Skip_T1_(_BITS)
    #define Info_T2(_BITS, _INFO, _NAME) Skip_T2_(_BITS)
    #define Info_T4(_BITS, _INFO, _NAME) Skip_T4_(_BITS)
    #define Info_T8(_BITS, _INFO, _NAME) Skip_T8_(_BITS)

    #define TEST_TB_GET(_CODE, _NAME) \
        { \
            Peek_TB(_CODE); \
            if (!_CODE) \
                Skip_TB_(); \
            else \
            { \
                Element_Begin0(); \
                Skip_TB_(); \

    #define TEST_TB_TKIP(_NAME) \
        { \
            if (!Peek_TB()) \
                Skip_TB_(); \
            else \
            { \
                Element_Begin0(); \
                Skip_TB_(); \

    #define TESTELSE_TB_GET(_CODE, _NAME) \
        { \
            Peek_TB(_CODE); \
            if (_CODE) \
            { \
                Element_Begin0(); \
                Skip_TB_(); \

    #define TESTELSE_TB_TKIP(_NAME) \
        { \
            if (Peek_TB()) \
            { \
                Element_Begin0(); \
                Skip_TB_(); \

    #define TESTELSE_TB_ELSE(_NAME) \
                Element_End0(); \
            } \
            else \
            { \
                Skip_TB_(); \

    #define TESTELSE_TB_END() \
            } \
        } \

    #define TEST_TB_END() \
                Element_End0(); \
            } \
        } \

    //***************************************************************************
    // Next code planning
    //***************************************************************************

    void NextCode_Add(int64u Code);
    void NextCode_Clear();
    bool NextCode_Test();

    //***************************************************************************
    // Element trusting
    //***************************************************************************

    void Trusted_IsNot (const char*)                                            {Trusted_IsNot();}
    void Trusted_IsNot ();

    //***************************************************************************
    // Stream filling
    //***************************************************************************

    //Elements - Preparation of element from external app
    size_t Stream_Prepare   (stream_t KindOfStream, size_t StreamPos=(size_t)-1);
    size_t Stream_Erase     (stream_t KindOfStream, size_t StreamPos);

    //Fill with datas (with parameter as a size_t)
    void Fill (stream_t StreamKind, size_t StreamPos, size_t Parameter, const Ztring  &Value, bool Replace=false);
    inline void Fill (stream_t StreamKind, size_t StreamPos, size_t Parameter, const std::string &Value, bool Utf8=true, bool Replace=false) {if (Utf8) Fill(StreamKind, StreamPos, Parameter, Ztring().From_UTF8(Value.c_str(), Value.size()), Replace); else Fill(StreamKind, StreamPos, Parameter, Ztring().From_Local(Value.c_str(), Value.size()), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, size_t Parameter, const char*    Value, size_t Value_Size=Unlimited, bool Utf8=true, bool Replace=false) {if (Utf8) Fill(StreamKind, StreamPos, Parameter, Ztring().From_UTF8(Value, Value_Size), Replace); else Fill(StreamKind, StreamPos, Parameter, Ztring().From_Local(Value, Value_Size), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, size_t Parameter, const wchar_t* Value, size_t Value_Size=Unlimited, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring().From_Unicode(Value, Value_Size), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, size_t Parameter, int8u          Value, int8u Radix=10, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, Radix).MakeUpperCase(), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, size_t Parameter, int8s          Value, int8u Radix=10, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, Radix).MakeUpperCase(), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, size_t Parameter, int16u         Value, int8u Radix=10, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, Radix).MakeUpperCase(), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, size_t Parameter, int16s         Value, int8u Radix=10, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, Radix).MakeUpperCase(), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, size_t Parameter, int32u         Value, int8u Radix=10, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, Radix).MakeUpperCase(), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, size_t Parameter, int32s         Value, int8u Radix=10, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, Radix).MakeUpperCase(), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, size_t Parameter, int64u         Value, int8u Radix=10, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, Radix).MakeUpperCase(), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, size_t Parameter, int64s         Value, int8u Radix=10, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, Radix).MakeUpperCase(), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, size_t Parameter, float32        Value, int8u AfterComma=3, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, AfterComma), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, size_t Parameter, float64        Value, int8u AfterComma=3, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, AfterComma), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, size_t Parameter, float80        Value, int8u AfterComma=3, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, AfterComma), Replace);}
    #ifdef SIZE_T_IS_LONG
    inline void Fill (stream_t StreamKind, size_t StreamPos, size_t Parameter, size_t         Value, int8u Radix=10, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, Radix).MakeUpperCase(), Replace);}
    #endif //SIZE_T_IS_LONG
    //Fill with datas
    void Fill (stream_t StreamKind, size_t StreamPos, const char* Parameter, const Ztring  &Value, bool Replace=false);
    inline void Fill (stream_t StreamKind, size_t StreamPos, const char* Parameter, const std::string &Value, bool Utf8=true, bool Replace=false) {if (Utf8) Fill(StreamKind, StreamPos, Parameter, Ztring().From_UTF8(Value.c_str(), Value.size())); else Fill(StreamKind, StreamPos, Parameter, Ztring().From_Local(Value.c_str(), Value.size()), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, const char* Parameter, const char*    Value, size_t Value_Size=Unlimited, bool Utf8=true, bool Replace=false) {if (Utf8) Fill(StreamKind, StreamPos, Parameter, Ztring().From_UTF8(Value, Value_Size), Replace); else Fill(StreamKind, StreamPos, Parameter, Ztring().From_Local(Value, Value_Size), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, const char* Parameter, const wchar_t* Value, size_t Value_Size=Unlimited, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring().From_Unicode(Value, Value_Size), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, const char* Parameter, int8u          Value, int8u Radix=10, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, Radix).MakeUpperCase(), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, const char* Parameter, int8s          Value, int8u Radix=10, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, Radix).MakeUpperCase(), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, const char* Parameter, int16u         Value, int8u Radix=10, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, Radix).MakeUpperCase(), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, const char* Parameter, int16s         Value, int8u Radix=10, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, Radix).MakeUpperCase(), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, const char* Parameter, int32u         Value, int8u Radix=10, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, Radix).MakeUpperCase(), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, const char* Parameter, int32s         Value, int8u Radix=10, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, Radix).MakeUpperCase(), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, const char* Parameter, int64u         Value, int8u Radix=10, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, Radix).MakeUpperCase(), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, const char* Parameter, int64s         Value, int8u Radix=10, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, Radix).MakeUpperCase(), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, const char* Parameter, float32        Value, int8u AfterComma=3, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, AfterComma), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, const char* Parameter, float64        Value, int8u AfterComma=3, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, AfterComma), Replace);}
    inline void Fill (stream_t StreamKind, size_t StreamPos, const char* Parameter, float80        Value, int8u AfterComma=3, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, AfterComma), Replace);}
    #ifdef SIZE_T_IS_LONG
    inline void Fill (stream_t StreamKind, size_t StreamPos, const char* Parameter, size_t         Value, int8u Radix=10, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, Radix).MakeUpperCase(), Replace);}
    #endif //SIZE_T_IS_LONG
    ZtringListList Fill_Temp;
    void Fill_Flush ();
    size_t Fill_Parameter(stream_t StreamKind, generic StreamPos);

    const Ztring &Retrieve_Const (stream_t StreamKind, size_t StreamPos, size_t Parameter, info_t KindOfInfo=Info_Text);
    Ztring Retrieve (stream_t StreamKind, size_t StreamPos, size_t Parameter, info_t KindOfInfo=Info_Text);
    const Ztring &Retrieve_Const (stream_t StreamKind, size_t StreamPos, const char* Parameter, info_t KindOfInfo=Info_Text);
    Ztring Retrieve (stream_t StreamKind, size_t StreamPos, const char* Parameter, info_t KindOfInfo=Info_Text);

    void Clear (stream_t StreamKind, size_t StreamPos, size_t Parameter);
    void Clear (stream_t StreamKind, size_t StreamPos, const char* Parameter);
    void Clear (stream_t StreamKind, size_t StreamPos);
    void Clear (stream_t StreamKind);
    inline void Clear () {File__Base::Clear();}

    //***************************************************************************
    // Filling
    //***************************************************************************

    //Actions
    void Accept        (const char*)                                            {Accept();}
    void Accept        ();
    void Accept        (File__Analyze* Parser);
    void Reject        (const char*)                                            {Reject();}
    void Reject        ();
    void Reject        (File__Analyze* Parser);
    void Fill          (const char*)                                            {Fill();}
    void Fill          ();
    void Fill          (File__Analyze* Parser);
    void Update        (const char*)                                            {Fill();}
    void Update        ();
    void Update        (File__Analyze* Parser);
    void Finish        (const char*)                                            {Finish();}
    void Finish        ();
    void Finish        (File__Analyze* Parser);
    void ForceFinish   (const char*)                                            {ForceFinish();}
    void ForceFinish   ();
    void ForceFinish   (File__Analyze* Parser);
    void GoTo          (int64u GoTo_, const char*)                              {GoTo(GoTo_);}
    void GoTo          (int64u GoTo);
    void GoToFromEnd   (int64u GoToFromEnd_, const char*)                       {GoToFromEnd(GoToFromEnd_);}
    void GoToFromEnd   (int64u GoToFromEnd);
    int64u Element_Code_Get (size_t Level);
    int64u Element_TotalSize_Get (size_t LevelLess=0);
    bool Element_IsComplete_Get ();
    void Element_ThisIsAList ();
    void Element_WaitForMoreData ();
    void Element_DoNotTrust (const char*)                                       {Element_DoNotTrust();}
    void Element_DoNotTrust ();
    inline void Element_DoNotShow () {}
    inline void Element_Show () {}
    inline bool Element_Show_Get () {return false;}
    inline void Element_Show_Add (const Ztring &) {}

    //Status
    bool Element_IsOK ();
    bool Element_IsNotFinished ();
    bool Element_IsWaitingForMoreData ();

    //***************************************************************************
    // Merging
    //***************************************************************************

    //Utils
public :
    size_t Merge(MediaInfo_Internal &ToAdd, bool Erase=true); //Merge 2 File_Base
    size_t Merge(MediaInfo_Internal &ToAdd, stream_t StreamKind, size_t StreamPos_From, size_t StreamPos_To, bool Erase=true); //Merge 2 streams
    size_t Merge(File__Analyze &ToAdd, bool Erase=true); //Merge 2 File_Base
    size_t Merge(File__Analyze &ToAdd, stream_t StreamKind, size_t StreamPos_From, size_t StreamPos_To, bool Erase=true); //Merge 2 streams

    void CodecID_Fill           (const Ztring &Value, stream_t StreamKind, size_t StreamPos, infocodecid_format_t Format, stream_t StreamKind_CodecID=Stream_Max);

    //***************************************************************************
    // Finalize
    //***************************************************************************

    //End
    void Streams_Finish_Global();

protected :
    void Streams_Finish_StreamOnly();
    void Streams_Finish_StreamOnly(stream_t StreamKid, size_t StreamPos);
    void Streams_Finish_StreamOnly_General(size_t StreamPos);
    void Streams_Finish_StreamOnly_Video(size_t StreamPos);
    void Streams_Finish_StreamOnly_Audio(size_t StreamPos);
    void Streams_Finish_StreamOnly_Text(size_t StreamPos);
    void Streams_Finish_StreamOnly_Other(size_t StreamPos);
    void Streams_Finish_StreamOnly_Image(size_t StreamPos);
    void Streams_Finish_StreamOnly_Menu(size_t StreamPos);
    void Streams_Finish_InterStreams();
    void Streams_Finish_Cosmetic();
    void Streams_Finish_Cosmetic(stream_t StreamKid, size_t StreamPos);
    void Streams_Finish_Cosmetic_General(size_t StreamPos);
    void Streams_Finish_Cosmetic_Video(size_t StreamPos);
    void Streams_Finish_Cosmetic_Audio(size_t StreamPos);
    void Streams_Finish_Cosmetic_Text(size_t StreamPos);
    void Streams_Finish_Cosmetic_Chapters(size_t StreamPos);
    void Streams_Finish_Cosmetic_Image(size_t StreamPos);
    void Streams_Finish_Cosmetic_Menu(size_t StreamPos);

    void Tags ();
    void Video_FrameRate_Rounding (size_t Pos, video Parameter);
    void Video_BitRate_Rounding (size_t Pos, video Parameter);
    void Audio_BitRate_Rounding (size_t Pos, audio Parameter);

    //Utils - Finalize
    void Duration_Duration123   (stream_t StreamKind, size_t StreamPos, size_t Parameter);
    void FileSize_FileSize123   (stream_t StreamKind, size_t StreamPos, size_t Parameter);
    void Kilo_Kilo123           (stream_t StreamKind, size_t StreamPos, size_t Parameter);
    void Value_Value123         (stream_t StreamKind, size_t StreamPos, size_t Parameter);
    void YesNo_YesNo            (stream_t StreamKind, size_t StreamPos, size_t Parameter);

    //***************************************************************************
    //
    //***************************************************************************

protected :
    //Save for speed improvement
    int8u           Config_Demux;
    Ztring          Config_LineSeparator;
    bool            IsSub;
    bool            IsRawStream;

    //Configuration
    bool DataMustAlwaysBeComplete;  //Data must always be complete, else wait for more data
    bool MustUseAlternativeParser;  //Must use the second parser (example: for Data part)

    //Synchro
    bool MustParseTheHeaderFile;    //There is an header part, must parse it
    size_t Trusted;
    size_t Trusted_Multiplier;

    //Elements
    size_t Element_Level;           //Current level
    bool   Element_WantNextLevel;   //Want to go to the next leavel instead of the same level

    //Element
    int64u Element_Code;            //Code filled in the file, copy of Element[Element_Level].Code
    int64u Element_Offset;          //Position in the Element (without header)
    int64u Element_Size;            //Size of the Element (without header)

private :
    //***************************************************************************
    // Buffer
    //***************************************************************************

    void Buffer_Clear(); //Clear the buffer
protected :
    //Buffer
    bool Open_Buffer_Continue_Loop();
    const int8u* Buffer;
public : //TO CHANGE
    size_t Buffer_Size;
    int64u Buffer_TotalBytes;
    int64u Buffer_TotalBytes_FirstSynched;
    int64u Buffer_TotalBytes_LastSynched;
    int64u Buffer_PaddingBytes;
    int64u Buffer_JunkBytes;
    float64 Stream_BitRateFromContainer;
protected :
    int8u* Buffer_Temp;
    size_t Buffer_Temp_Size;
    size_t Buffer_Temp_Size_Max;
    size_t Buffer_Offset; //Temporary usage in this parser
    size_t Buffer_Offset_Temp; //Temporary usage in this parser
    size_t Buffer_MinimumSize;
    size_t Buffer_MaximumSize;
    int64u Buffer_TotalBytes_FirstSynched_Max;
    int64u Buffer_TotalBytes_Fill_Max;
    friend class File__Tags_Helper;

    //***************************************************************************
    // Helpers
    //***************************************************************************

    bool FileHeader_Begin_0x000001();
    bool FileHeader_Begin_XML(tinyxml2::XMLDocument &Document);
    bool Synchronize_0x000001();
public:
    void TestContinuousFileNames(size_t CountOfFiles=24, Ztring FileExtension=Ztring());

private :

    //***************************************************************************
    // Elements
    //***************************************************************************

    //Element
    BitStream_Fast* BS;             //For conversion from bytes to bitstream
    BitStream*      BT;             //For conversion from bytes to bitstream (Little Endian)
public : //TO CHANGE
    int64u Header_Size;             //Size of the header of the current element
private :

    //Elements
    size_t Element_Level_Base;      //From other parsers

public : //For very quick access, to not use except if you know what you do
    struct element_details
    {
        int64u  Code;               //Code filled in the file
        int64u  Next;               //
        bool    WaitForMoreData;    //This element is not complete, we need more data
        bool    UnTrusted;          //This element has a problem
        bool    IsComplete;         //This element is fully buffered, no need of more
    };
    std::vector<element_details> Element;

private :

    //NextCode
    std::map<int64u, bool> NextCode;

    //BookMarks
    size_t              BookMark_Element_Level;
    int64u              BookMark_GoTo;
    std::vector<int64u> BookMark_Code;
    std::vector<int64u> BookMark_Next;

public :
    void BookMark_Set(size_t Element_Level_ToGet=(size_t)-1);
    void BookMark_Get();
    virtual bool BookMark_Needed()                                              {return false;};

    //Temp
    std::bitset<32> Status;
    enum status
    {
        IsAccepted,
        IsFilled,
        IsUpdated,
        IsFinished,
        Reserved_04,
        Reserved_05,
        Reserved_06,
        Reserved_07,
        Reserved_08,
        Reserved_09,
        Reserved_10,
        Reserved_11,
        Reserved_12,
        Reserved_13,
        Reserved_14,
        Reserved_15,
        User_16,
        User_17,
        User_18,
        User_19,
        User_20,
        User_21,
        User_22,
        User_23,
        User_24,
        User_25,
        User_26,
        User_27,
        User_28,
        User_29,
        User_30,
        User_31,
    };
    bool ShouldContinueParsing;

    //Configuration
    bool MustSynchronize;
    bool CA_system_ID_MustSkipSlices;

    //Demux
    enum contenttype
    {
        ContentType_MainStream,
        ContentType_SubStream,
        ContentType_Header,
        ContentType_Synchro
    };
    #if MEDIAINFO_DEMUX
        void Demux (const int8u* Buffer, size_t Buffer_Size, contenttype ContentType, const int8u* OriginalBuffer=NULL, size_t OriginalBuffer_Size=0);
        virtual bool Demux_UnpacketizeContainer_Test() {return true;}
        bool Demux_UnpacketizeContainer_Test_OneFramePerFile();
        void Demux_UnpacketizeContainer_Demux(bool random_access=true);
        void Demux_UnpacketizeContainer_Demux_Clear();
        bool Demux_EventWasSent_Accept_Specific;
    #else //MEDIAINFO_DEMUX
        #define Demux(_A, _B, _C)
    #endif //MEDIAINFO_DEMUX

    //Events data
    bool    PES_FirstByte_IsAvailable;
    bool    PES_FirstByte_Value;

    int64u  Unsynch_Frame_Count;

    //MD5
    #if MEDIAINFO_MD5
        struct MD5Context*  MD5;
        int64u              Md5_ParseUpTo;
    #endif //MEDIAINFO_MD5

    #if MEDIAINFO_SEEK
    private:
        bool Seek_Duration_Detected;
    #endif //MEDIAINFO_SEEK

    #if MEDIAINFO_IBI
    public:
        bool    Config_Ibi_Create;
        int64u  Ibi_SynchronizationOffset_Current;
        int64u  Ibi_SynchronizationOffset_BeginOfFrame;
        ibi     Ibi; //If Main only
        ibi::stream* IbiStream; //If sub only
        size_t  Ibi_Read_Buffer_Seek        (size_t Method, int64u Value, int64u ID);
        void    Ibi_Read_Buffer_Unsynched   ();
        void    Ibi_Stream_Finish           ();
        void    Ibi_Stream_Finish           (int64u Numerator, int64u Denominator); //Partial
        void    Ibi_Add                     ();
    #else //MEDIAINFO_IBI
        size_t  Ibi_Read_Buffer_Seek        (size_t, int64u, int64u)            {return (size_t)-1;}
        void    Ibi_Read_Buffer_Unsynched   ()                                  {}
        void    Ibi_Stream_Finish           ()                                  {}
        void    Ibi_Stream_Finish           (int64u, int64u)                    {}
        void    Ibi_Add                     ()                                  {}
    #endif //MEDIAINFO_IBI
};

//Helpers
#define DETAILS_INFO(_DATA)

#endif
