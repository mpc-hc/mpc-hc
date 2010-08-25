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
    void    Open_Buffer_Continue    (File__Analyze* Sub, const int8u* Buffer, size_t Buffer_Size);
    void    Open_Buffer_Continue    (File__Analyze* Sub, size_t Buffer_Size) {Open_Buffer_Continue(Sub, Buffer+Buffer_Offset+(size_t)Element_Offset, Buffer_Size); Element_Offset+=Buffer_Size;}
    void    Open_Buffer_Continue    (File__Analyze* Sub) {Open_Buffer_Continue(Sub, Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset)); Element_Offset=Element_Size;}
    void    Open_Buffer_Position_Set(int64u File_Offset);
    void    Open_Buffer_Unsynch     ();
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
    #endif //MEDIAINFO_EVENTS
    Ztring  File_Name_WithoutDemux;
    bool   PTS_DTS_Needed;
    int64u PCR; //In nanoseconds
    int64u PTS; //In nanoseconds
    int64u DTS; //In nanoseconds

    //Out
    int64u PTS_DTS_Offset_InThisBlock; //In nanoseconds
    size_t Frame_Count_InThisBlock;

protected :
    //***************************************************************************
    // Streams management
    //***************************************************************************

    virtual void Streams_Fill()                                                 {};
    virtual void Streams_Finish()                                               {};

    //***************************************************************************
    // Synchro
    //***************************************************************************

    virtual bool Synchronize()    {Synched=true; return true;}; //Look for the synchro
    virtual bool Synched_Test()   {return true;}; //Test is synchro is OK
    virtual void Synched_Init()   {}; //When synched, we can Init data
    bool Synchro_Manage();

    //***************************************************************************
    // Buffer
    //***************************************************************************

    //Buffer
    virtual void Read_Buffer_Init ()          {}; //Temp, should be in File__Base caller
    virtual void Read_Buffer_Continue ()      {}; //Temp, should be in File__Base caller
    virtual void Read_Buffer_Continue_Once () {}; //Temp, should be in File__Base caller
    virtual void Read_Buffer_Unsynched ()     {}; //Temp, should be in File__Base caller
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
    inline void Element_Begin (const Ztring &, int64u Size=(int64u)-1) {Element_Begin(Size);}
    void Element_Begin (int64u Size);
    inline void Element_Begin (const char *, int64u Size=(int64u)-1) {Element_Begin(Size);}

    //Elements - Name
    #define Element_Name(_A)

    //Elements - Info
    inline void Element_Info (const Ztring &) {}
    inline void Element_Info (const char*) {}
    inline void Element_Info (const char*, const char* ) {}
    inline void Element_Info (int8s      , const char* =NULL) {}
    inline void Element_Info (int8u      , const char* =NULL) {}
    inline void Element_Info (int16s     , const char* =NULL) {}
    inline void Element_Info (int16u     , const char* =NULL) {}
    inline void Element_Info (int32s     , const char* =NULL) {}
    inline void Element_Info (int32u     , const char* =NULL) {}
    inline void Element_Info (int64s     , const char* =NULL) {}
    inline void Element_Info (int64u     , const char* =NULL) {}
    inline void Element_Info (int128u    , const char* =NULL) {}
    #ifdef NEED_SIZET
    inline void Element_Info (size_t     , const char* =NULL) {}
    #endif //NEED_SIZET
    inline void Element_Info (float32    , int8u =3, const char* =NULL) {}
    inline void Element_Info (float64    , int8u =3, const char* =NULL) {}
    #define Element_Info_From_Milliseconds(_A)

    //Elements - End
    void Element_End ();
    inline void Element_End (const Ztring &, int64u Size=(int64u)-1) {Element_End(Size);}
    void Element_End (int64u Size);
    inline void Element_End (const char*   , int64u Size=(int64u)-1) {Element_End(Size);}

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

    //TODO: put this in Ztring()
    Ztring ToZtring(const char* Value, size_t Value_Size=Unlimited, bool Utf8=true)
    {
        if (Utf8)
            return Ztring().From_UTF8(Value, Value_Size);
        else
            return Ztring().From_Local(Value, Value_Size);
    }
    #define VALUE(Value) \
        Ztring::ToZtring(Value, 16).MakeUpperCase()+_T(" (")+Ztring::ToZtring(Value, 10).MakeUpperCase()+_T(")")

    //Param - Main
    inline void Param      (const Ztring &, const Ztring&) {}
    inline void Param      (const char*, const Ztring&) {}
    inline void Param      (const char*, const std::string&) {}
    inline void Param      (const char*, const char*, size_t =Unlimited, bool =true) {}
    inline void Param      (const char*, const int8u*, size_t =Unlimited, bool =true) {}
    inline void Param      (const char*, bool) {}
    inline void Param      (const char*, int8u) {}
    inline void Param      (const char*, int8s) {}
    inline void Param      (const char*, int16u) {}
    inline void Param      (const char*, int16s) {}
    inline void Param      (const char*, int32u) {}
    inline void Param      (const char*, int32s) {}
    inline void Param      (const char*, int64u) {}
    inline void Param      (const char*, int64s) {}
    inline void Param      (const char*, int128u) {}
    inline void Param_GUID (const char*, int128u) {}
    inline void Param_UUID (const char*, int128u) {}
    #ifdef NEED_SIZET
    inline void Param      (const char*, size_t, intu =16) {}
    #endif //NEED_SIZET
    inline void Param      (const char*, float32, intu =3) {}
    inline void Param      (const char*, float64, intu =3) {}
    inline void Param      (const char*, float80, intu =3)  {}
    inline void Param      (const int32u, const Ztring&) {}
    inline void Param      (const int16u, const Ztring&) {}

    //Param - Info
    inline void Param_Info (const Ztring &) {}
    inline void Param_Info (const char*) {}
    inline void Param_Info (const char*, const char*) {}
    inline void Param_Info (int64u     , const char* =NULL) {}
    inline void Param_Info (int64s     , const char* =NULL) {}
    inline void Param_Info (int32u     , const char* =NULL) {}
    inline void Param_Info (int32s     , const char* =NULL) {}
    inline void Param_Info (int16u     , const char* =NULL) {}
    inline void Param_Info (int16s     , const char* =NULL) {}
    inline void Param_Info (int8u      , const char* =NULL) {}
    inline void Param_Info (int8s      , const char* =NULL) {}
    inline void Param_Info (float32    , int8u =3, const char* =NULL) {}
    inline void Param_Info (float64    , int8u =3, const char* =NULL) {}
    inline void Param_Info (float80    , int8u =3, const char* =NULL) {}
    #ifdef NEED_SIZET
    inline void Param_Info (size_t     , const char* =NULL) {}
    #endif //NEED_SIZET
    inline void Param_Info_From_Milliseconds (int64u) {}

    //***************************************************************************
    // Information
    //***************************************************************************

    inline void Info (const Ztring&, size_t =0) {}

    //***************************************************************************
    // Big Endian (Integer, Float, Fixed-Point)
    //***************************************************************************

    void Get_B1   (int8u   &Info);
    void Get_B2   (int16u  &Info);
    void Get_B3   (int32u  &Info);
    void Get_B4   (int32u  &Info);
    void Get_B5   (int64u  &Info);
    void Get_B6   (int64u  &Info);
    void Get_B7   (int64u  &Info);
    void Get_B8   (int64u  &Info);
    void Get_B16  (int128u &Info);
    void Get_BF4  (float32 &Info);
    void Get_BF8  (float64 &Info);
    void Get_BF10 (float80 &Info);
    void Get_BFP4 (size_t Bits, float32 &Info);
    inline void Get_B1   (int8u   &Info, const char*) {Get_B1(Info);}
    inline void Get_B2   (int16u  &Info, const char*) {Get_B2(Info);}
    inline void Get_B3   (int32u  &Info, const char*) {Get_B3(Info);}
    inline void Get_B4   (int32u  &Info, const char*) {Get_B4(Info);}
    inline void Get_B5   (int64u  &Info, const char*) {Get_B5(Info);}
    inline void Get_B6   (int64u  &Info, const char*) {Get_B6(Info);}
    inline void Get_B7   (int64u  &Info, const char*) {Get_B7(Info);}
    inline void Get_B8   (int64u  &Info, const char*) {Get_B8(Info);}
    inline void Get_B16  (int128u &Info, const char*) {Get_B16(Info);}
    inline void Get_BF4  (float32 &Info, const char*) {Get_BF4(Info);}
    inline void Get_BF8  (float64 &Info, const char*) {Get_BF8(Info);}
    inline void Get_BF10 (float80 &Info, const char*) {Get_BF10(Info);}
    inline void Get_BFP4 (size_t Bits, float32 &Info, const char*) {Get_BFP4(Bits, Info);}
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
    void Peek_BFP4(size_t Bits, float64 &Info);
    inline void Skip_B1  (               const char*) {if (Element_Offset+1>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=1;}
    inline void Skip_B2  (               const char*) {if (Element_Offset+2>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=2;}
    inline void Skip_B3  (               const char*) {if (Element_Offset+3>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=3;}
    inline void Skip_B4  (               const char*) {if (Element_Offset+4>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=4;}
    inline void Skip_B5  (               const char*) {if (Element_Offset+5>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=5;}
    inline void Skip_B6  (               const char*) {if (Element_Offset+6>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=6;}
    inline void Skip_B7  (               const char*) {if (Element_Offset+7>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=7;}
    inline void Skip_B8  (               const char*) {if (Element_Offset+8>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=8;}
    inline void Skip_BF4 (               const char*) {if (Element_Offset+4>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=4;}
    inline void Skip_BF8 (               const char*) {if (Element_Offset+8>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=8;}
    inline void Skip_B16 (               const char*) {if (Element_Offset+16>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=16;}
    inline void Skip_BFP4(size_t,                     const char*) {if (Element_Offset+4>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=4;};
    #define Info_B1(_INFO, _NAME)   int8u   _INFO; Get_B1 (_INFO, _NAME)
    #define Info_B2(_INFO, _NAME)   int16u  _INFO; Get_B2 (_INFO, _NAME)
    #define Info_B3(_INFO, _NAME)   int32u  _INFO; Get_B3 (_INFO, _NAME)
    #define Info_B4(_INFO, _NAME)   int32u  _INFO; Get_B4 (_INFO, _NAME)
    #define Info_B5(_INFO, _NAME)   int64u  _INFO; Get_B5 (_INFO, _NAME)
    #define Info_B6(_INFO, _NAME)   int64u  _INFO; Get_B6 (_INFO, _NAME)
    #define Info_B7(_INFO, _NAME)   int64u  _INFO; Get_B7 (_INFO, _NAME)
    #define Info_B8(_INFO, _NAME)   int64u  _INFO; Get_B8 (_INFO, _NAME)
    #define Info_B16(_INFO, _NAME)  int128u _INFO; Get_B16(_INFO, _NAME)
    #define Info_BF4(_INFO, _NAME)  float32 _INFO; Get_BF4(_INFO, _NAME)
    #define Info_BF8(_INFO, _NAME)  float64 _INFO; Get_BF8(_INFO, _NAME)
    #define Info_BF10(_INFO, _NAME) float80 _INFO; Get_BF8(_INFO, _NAME)
    #define Info_BFP4(_BITS, _INFO, _NAME) float32 _INFO; Get_BFP4(_BITS, _INFO, _NAME)

    //***************************************************************************
    // Little Endian
    //***************************************************************************

    void Get_L1  (int8u   &Info);
    void Get_L2  (int16u  &Info);
    void Get_L3  (int32u  &Info);
    void Get_L4  (int32u  &Info);
    void Get_L5  (int64u  &Info);
    void Get_L6  (int64u  &Info);
    void Get_L7  (int64u  &Info);
    void Get_L8  (int64u  &Info);
    void Get_L16 (int128u &Info);
    void Get_LF4 (float32 &Info);
    void Get_LF8 (float64 &Info);
    inline void Get_L1  (int8u   &Info, const char*) {Get_L1(Info);}
    inline void Get_L2  (int16u  &Info, const char*) {Get_L2(Info);}
    inline void Get_L3  (int32u  &Info, const char*) {Get_L3(Info);}
    inline void Get_L4  (int32u  &Info, const char*) {Get_L4(Info);}
    inline void Get_L5  (int64u  &Info, const char*) {Get_L5(Info);}
    inline void Get_L6  (int64u  &Info, const char*) {Get_L6(Info);}
    inline void Get_L7  (int64u  &Info, const char*) {Get_L7(Info);}
    inline void Get_L8  (int64u  &Info, const char*) {Get_L8(Info);}
    inline void Get_L16 (int128u &Info, const char*) {Get_L16(Info);}
    inline void Get_LF4 (float32 &Info, const char*) {Get_LF4(Info);}
    inline void Get_LF8 (float64 &Info, const char*) {Get_LF8(Info);}
    void Peek_L1 (int8u   &Info);
    void Peek_L2 (int16u  &Info);
    void Peek_L3 (int32u  &Info);
    void Peek_L4 (int32u  &Info);
    void Peek_L5 (int64u  &Info);
    void Peek_L6 (int64u  &Info);
    void Peek_L7 (int64u  &Info);
    void Peek_L8 (int64u  &Info);
    void Peek_L16(int128u &Info);
    void Peek_LF4(float32 &Info);
    void Peek_LF8(float64 &Info);
    inline void Skip_L1  (               const char*) {if (Element_Offset+1>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=1;}
    inline void Skip_L2  (               const char*) {if (Element_Offset+2>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=2;}
    inline void Skip_L3  (               const char*) {if (Element_Offset+3>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=3;}
    inline void Skip_L4  (               const char*) {if (Element_Offset+4>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=4;}
    inline void Skip_L5  (               const char*) {if (Element_Offset+5>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=5;}
    inline void Skip_L6  (               const char*) {if (Element_Offset+6>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=6;}
    inline void Skip_L7  (               const char*) {if (Element_Offset+7>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=7;}
    inline void Skip_L8  (               const char*) {if (Element_Offset+8>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=8;}
    inline void Skip_LF4 (               const char*) {if (Element_Offset+4>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=4;}
    inline void Skip_LF8 (               const char*) {if (Element_Offset+8>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=8;}
    inline void Skip_L16 (               const char*) {if (Element_Offset+16>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=16;}
    #define Info_L1(_INFO, _NAME)  int8u   _INFO; Get_L1 (_INFO, _NAME)
    #define Info_L2(_INFO, _NAME)  int16u  _INFO; Get_L2 (_INFO, _NAME)
    #define Info_L3(_INFO, _NAME)  int32u  _INFO; Get_L3 (_INFO, _NAME)
    #define Info_L4(_INFO, _NAME)  int32u  _INFO; Get_L4 (_INFO, _NAME)
    #define Info_L5(_INFO, _NAME)  int64u  _INFO; Get_L5 (_INFO, _NAME)
    #define Info_L6(_INFO, _NAME)  int64u  _INFO; Get_L6 (_INFO, _NAME)
    #define Info_L7(_INFO, _NAME)  int64u  _INFO; Get_L7 (_INFO, _NAME)
    #define Info_L8(_INFO, _NAME)  int64u  _INFO; Get_L8 (_INFO, _NAME)
    #define Info_L16(_INFO, _NAME) int128u _INFO; Get_L16(_INFO, _NAME)
    #define Info_LF4(_INFO, _NAME) float32 _INFO; Get_LF4(_INFO, _NAME)
    #define Info_LF8(_INFO, _NAME) float64 _INFO; Get_LF8(_INFO, _NAME)

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

    void Get_UUID (int128u &Info);
    inline void Get_UUID (int128u &Info, const char*) {Get_UUID(Info);}
    void Peek_UUID(int128u &Info);
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
    void Skip_SE();
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

    void Get_VL (int32u Call(int8u Size, int32u ToCall), int32u &Info);
    inline void Get_VL (int32u Call(int8u Size, int32u ToCall), int32u &Info, const char*) {Get_VL(Call, Info);};
    void Skip_VL(int32u Call(int8u Size, int32u ToCall));
    inline void Skip_VL(int32u Call(int8u Size, int32u ToCall),               const char*) {Skip_VL(Call);};
    #define Info_VL(_CALL, _INFO, _NAME) int32u _INFO; Get_VL(_CALL, _INFO, _NAME)

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
    void Get_ISO_8859_1 (int64u Bytes, Ztring  &Info);
    void Get_String (int64u Bytes, std::string &Info);
    void Get_UTF8   (int64u Bytes, Ztring      &Info);
    void Get_UTF16  (int64u Bytes, Ztring      &Info);
    void Get_UTF16B (int64u Bytes, Ztring      &Info);
    void Get_UTF16L (int64u Bytes, Ztring      &Info);
    inline void Get_Local  (int64u Bytes, Ztring      &Info, const char*) {Get_Local(Bytes, Info);}
    inline void Get_ISO_8859_1 (int64u Bytes, Ztring  &Info, const char*) {Get_ISO_8859_1(Bytes, Info);}
    inline void Get_String (int64u Bytes, std::string &Info, const char*) {Get_String(Bytes, Info);}
    inline void Get_UTF8   (int64u Bytes, Ztring      &Info, const char*) {Get_UTF8(Bytes, Info);}
    inline void Get_UTF16  (int64u Bytes, Ztring      &Info, const char*) {Get_UTF16(Bytes, Info);}
    inline void Get_UTF16B (int64u Bytes, Ztring      &Info, const char*) {Get_UTF16B(Bytes, Info);}
    inline void Get_UTF16L (int64u Bytes, Ztring      &Info, const char*) {Get_UTF16L(Bytes, Info);}
    void Peek_Local (int64u Bytes, Ztring      &Info);
    void Peek_String(int64u Bytes, std::string &Info);
    void Skip_Local (int64u Bytes,                    const char*) {if (Element_Offset+Bytes>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=(size_t)Bytes;}
    void Skip_String(int64u Bytes,                    const char*) {if (Element_Offset+Bytes>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=(size_t)Bytes;}
    void Skip_UTF8  (int64u Bytes,                    const char*) {if (Element_Offset+Bytes>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=(size_t)Bytes;}
    void Skip_UTF16B(int64u Bytes,                    const char*) {if (Element_Offset+Bytes>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=(size_t)Bytes;}
    void Skip_UTF16L(int64u Bytes,                    const char*) {if (Element_Offset+Bytes>Element_Size) {Trusted_IsNot(); return;} Element_Offset+=(size_t)Bytes;}
    #define Info_Local(_BYTES, _INFO, _NAME)  Ztring _INFO; Get_Local (_BYTES, _INFO, _NAME)
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

    inline void Skip_XX(int64u Bytes, const char*) {Element_Offset+=Bytes;}

    //***************************************************************************
    // Flags
    //***************************************************************************

    void Get_Flags (int64u Flags, size_t Order, bool  &Info);
    void Get_Flags (int64u ValueToPut,          int8u &Info);
    inline void Get_Flags (int64u Flags, size_t Order, bool  &Info, const char*) {Get_Flags(Flags, Order, Info);}
    inline void Get_Flags (int64u ValueToPut,          int8u &Info, const char*) {Get_Flags(ValueToPut, Info);}
    inline void Skip_Flags(int64u, size_t,                          const char*) {}
    inline void Skip_Flags(int64u,                                  const char*) {}
    #define Info_Flags(_FLAGS, _ORDER, _INFO, _NAME) bool _INFO; Get_Flags (_FLAGS, _ORDER, _INFO, _NAME)

    //***************************************************************************
    // BitStream
    //***************************************************************************

    void Get_BS (size_t Bits, int32u  &Info);
    void Get_SB (             bool    &Info);
    bool Get_SB ()                                              {bool Temp; Get_SB(Temp); return Temp;}
    void Get_S1 (size_t Bits, int8u   &Info);
    void Get_S2 (size_t Bits, int16u  &Info);
    void Get_S3 (size_t Bits, int32u  &Info);
    void Get_S4 (size_t Bits, int32u  &Info);
    void Get_S5 (size_t Bits, int64u  &Info);
    void Get_S6 (size_t Bits, int64u  &Info);
    void Get_S7 (size_t Bits, int64u  &Info);
    void Get_S8 (size_t Bits, int64u  &Info);
    inline void Get_BS (size_t Bits, int32u  &Info, const char*) {Get_BS(Bits, Info);}
    inline void Get_SB (             bool    &Info, const char*) {Get_SB(      Info);}
    inline void Get_S1 (size_t Bits, int8u   &Info, const char*) {Get_S1(Bits, Info);}
    inline void Get_S2 (size_t Bits, int16u  &Info, const char*) {Get_S2(Bits, Info);}
    inline void Get_S3 (size_t Bits, int32u  &Info, const char*) {Get_S3(Bits, Info);}
    inline void Get_S4 (size_t Bits, int32u  &Info, const char*) {Get_S4(Bits, Info);}
    inline void Get_S5 (size_t Bits, int64u  &Info, const char*) {Get_S5(Bits, Info);}
    inline void Get_S6 (size_t Bits, int64u  &Info, const char*) {Get_S6(Bits, Info);}
    inline void Get_S7 (size_t Bits, int64u  &Info, const char*) {Get_S7(Bits, Info);}
    inline void Get_S8 (size_t Bits, int64u  &Info, const char*) {Get_S8(Bits, Info);}
    void Peek_BS(size_t Bits, int32u  &Info);
    void Peek_SB(              bool    &Info);
    bool Peek_SB()                                              {bool Temp; Peek_SB(Temp); return Temp;}
    void Peek_S1(size_t Bits, int8u   &Info);
    void Peek_S2(size_t Bits, int16u  &Info);
    void Peek_S3(size_t Bits, int32u  &Info);
    void Peek_S4(size_t Bits, int32u  &Info);
    void Peek_S5(size_t Bits, int64u  &Info);
    void Peek_S6(size_t Bits, int64u  &Info);
    void Peek_S7(size_t Bits, int64u  &Info);
    void Peek_S8(size_t Bits, int64u  &Info);
    void Skip_BS(size_t Bits);
    void Skip_SB(           );
    void Skip_S1(size_t Bits);
    void Skip_S2(size_t Bits);
    void Skip_S3(size_t Bits);
    void Skip_S4(size_t Bits);
    void Skip_S5(size_t Bits);
    void Skip_S6(size_t Bits);
    void Skip_S7(size_t Bits);
    void Skip_S8(size_t Bits);
    inline void Skip_BS(size_t Bits,                const char*) {BS->Skip8(Bits);}
    inline void Skip_SB(                            const char*) {BS->Skip1(1);}
    inline void Skip_S1(size_t Bits,                const char*) {BS->Skip1(Bits);}
    inline void Skip_S2(size_t Bits,                const char*) {BS->Skip2(Bits);}
    inline void Skip_S3(size_t Bits,                const char*) {BS->Skip4(Bits);}
    inline void Skip_S4(size_t Bits,                const char*) {BS->Skip4(Bits);}
    inline void Skip_S5(size_t Bits,                const char*) {BS->Skip8(Bits);}
    inline void Skip_S6(size_t Bits,                const char*) {BS->Skip8(Bits);}
    inline void Skip_S7(size_t Bits,                const char*) {BS->Skip8(Bits);}
    inline void Skip_S8(size_t Bits,                const char*) {BS->Skip8(Bits);}
    void Mark_0 ();
    void Mark_0_NoTrustError (); //Use it for providing a warning instead of a non-trusting error
    void Mark_1 ();
    void Mark_1_NoTrustError (); //Use it for providing a warning instead of a non-trusting error
    #define Info_BS(_BITS, _INFO, _NAME) int32u  _INFO; Get_BS(_BITS, _INFO, _NAME)
    #define Info_SB(_INFO, _NAME)        bool    _INFO; Get_SB(       _INFO, _NAME)
    #define Info_S1(_BITS, _INFO, _NAME) int8u   _INFO; Get_S1(_BITS, _INFO, _NAME)
    #define Info_S2(_BITS, _INFO, _NAME) int16u  _INFO; Get_S2(_BITS, _INFO, _NAME)
    #define Info_S3(_BITS, _INFO, _NAME) int32u  _INFO; Get_S4(_BITS, _INFO, _NAME)
    #define Info_S4(_BITS, _INFO, _NAME) int32u  _INFO; Get_S4(_BITS, _INFO, _NAME)
    #define Info_S5(_BITS, _INFO, _NAME) int64u  _INFO; Get_S5(_BITS, _INFO, _NAME)
    #define Info_S6(_BITS, _INFO, _NAME) int64u  _INFO; Get_S6(_BITS, _INFO, _NAME)
    #define Info_S7(_BITS, _INFO, _NAME) int64u  _INFO; Get_S7(_BITS, _INFO, _NAME)
    #define Info_S8(_BITS, _INFO, _NAME) int64u  _INFO; Get_S8(_BITS, _INFO, _NAME)

    #define TEST_SB_GET(_CODE, _NAME) \
        { \
            Peek_SB(_CODE); \
            if (!_CODE) \
                Skip_SB(                                        _NAME); \
            else \
            { \
                Element_Begin(_NAME); \
                Skip_SB(                                        _NAME); \

    #define TEST_SB_SKIP(_NAME) \
        { \
            if (!Peek_SB()) \
                Skip_SB(                                        _NAME); \
            else \
            { \
                Element_Begin(_NAME); \
                Skip_SB(                                        _NAME); \

    #define TESTELSE_SB_GET(_CODE, _NAME) \
        { \
            Peek_SB(_CODE); \
            if (_CODE) \
            { \
                Element_Begin(_NAME); \
                Skip_SB(                                        _NAME); \

    #define TESTELSE_SB_SKIP(_NAME) \
        { \
            if (Peek_SB()) \
            { \
                Element_Begin(_NAME); \
                Skip_SB(                                        _NAME); \

    #define TESTELSE_SB_ELSE(_NAME) \
                Element_End(); \
            } \
            else \
            { \
                Skip_SB(                                        _NAME); \

    #define TESTELSE_SB_END() \
            } \
        } \

    #define TEST_SB_END() \
                Element_End(); \
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

    //Before filling the stream, the stream must be prepared
    size_t Stream_Prepare   (stream_t KindOfStream);

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
    #ifdef NEED_SIZET
    inline void Fill (stream_t StreamKind, size_t StreamPos, size_t Parameter, size_t         Value, int8u Radix=10, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, Radix).MakeUpperCase(), Replace);}
    #endif //NEED_SIZET
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
    #ifdef NEED_SIZET
    inline void Fill (stream_t StreamKind, size_t StreamPos, const char* Parameter, size_t         Value, int8u Radix=10, bool Replace=false) {Fill(StreamKind, StreamPos, Parameter, Ztring::ToZtring(Value, Radix).MakeUpperCase(), Replace);}
    #endif //NEED_SIZET
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

    //Begin
    #define FILLING_BEGIN() \
        if (Element_IsOK()) \
        {

    #define FILLING_BEGIN_PRECISE() \
        if (Element_Offset!=Element_Size) \
            Trusted_IsNot("Size error"); \
        else if (Element_IsOK()) \
        {

    //Else
    #define FILLING_ELSE() \
        } \
        else \
        { \

    //End
    #define FILLING_END() \
        }

    //***************************************************************************
    // Merging
    //***************************************************************************

    //Utils
public :
    size_t Merge(MediaInfo_Internal &ToAdd, bool Erase=true); //Merge 2 File_Base
    size_t Merge(MediaInfo_Internal &ToAdd, stream_t StreamKind, size_t StreamPos_From, size_t StreamPos_To, bool Erase=true); //Merge 2 streams
    size_t Merge(File__Analyze &ToAdd, bool Erase=true); //Merge 2 File_Base
    size_t Merge(File__Analyze &ToAdd, stream_t StreamKind, size_t StreamPos_From, size_t StreamPos_To, bool Erase=true); //Merge 2 streams

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
    void Streams_Finish_StreamOnly_Chapters(size_t StreamPos);
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
    void Audio_BitRate_Rounding (size_t Pos, audio Parameter);

    //Utils - Finalize
    void Duration_Duration123   (stream_t StreamKind, size_t StreamPos, size_t Parameter);
    void FileSize_FileSize123   (stream_t StreamKind, size_t StreamPos, size_t Parameter);
    void Kilo_Kilo123           (stream_t StreamKind, size_t StreamPos, size_t Parameter);
    void Value_Value123         (stream_t StreamKind, size_t StreamPos, size_t Parameter);
    void YesNo_YesNo            (stream_t StreamKind, size_t StreamPos, size_t Parameter);
    void CodecID_Fill           (const Ztring &Value, stream_t StreamKind, size_t StreamPos, infocodecid_format_t Format);

    //***************************************************************************
    //
    //***************************************************************************

protected :
    #if MEDIAINFO_TRACE
        //Save for speed improvement
        float Config_Details;
    #endif //MEDIAINFO_TRACE
    bool IsSub;
    bool IsRawStream;

    //Configuration
    bool DataMustAlwaysBeComplete;  //Data must always be complete, else wait for more data
    bool MustUseAlternativeParser;  //Must use the second parser (example: for Data part)

    //Synchro
    bool MustParseTheHeaderFile;    //There is an header part, must parse it
    bool Synched;                   //Data is synched
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
    void Open_Buffer_Continue_Loop();
protected :
    //Buffer
    const int8u* Buffer;
public : //TO CHANGE
    size_t Buffer_Size;
    int64u Buffer_TotalBytes_FirstSynched;
protected :
    int8u* Buffer_Temp;
    size_t Buffer_Temp_Size;
    size_t Buffer_Temp_Size_Max;
    size_t Buffer_Offset; //Temporary usage in this parser
    size_t Buffer_Offset_Temp; //Temporary usage in this parser
    size_t Buffer_MinimumSize;
    size_t Buffer_MaximumSize;
    int64u Buffer_TotalBytes;
    int64u Buffer_TotalBytes_FirstSynched_Max;
    friend class File__Tags_Helper;

    //***************************************************************************
    // Helpers
    //***************************************************************************

    bool FileHeader_Begin_0x000001();
    bool Synchronize_0x000001();

private :

    //***************************************************************************
    // Elements
    //***************************************************************************

    //Element
    BitStream* BS;                  //For conversion from bytes to bitstream
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
    };
    bool ShouldContinueParsing;

    //Configuration
    bool MustSynchronize;

    //Demux
    enum contenttype
    {
        ContentType_MainStream,
        ContentType_SubStream,
        ContentType_Header,
        ContentType_Synchro
    };
    #if MEDIAINFO_DEMUX
        void Demux (const int8u* Buffer, size_t Buffer_Size, contenttype ContentType);
    #else //MEDIAINFO_DEMUX
        #define Demux(_A, _B, _C)
    #endif //MEDIAINFO_DEMUX

    //Events data
    #if MEDIAINFO_EVENTS
        bool    MpegPs_PES_FirstByte_IsAvailable;
        bool    MpegPs_PES_FirstByte_Value;
    #endif //MEDIAINFO_EVENTS
};

//Helpers
#define DETAILS_INFO(_DATA)
