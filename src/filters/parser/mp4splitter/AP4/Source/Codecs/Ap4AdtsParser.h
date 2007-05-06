/*****************************************************************
|
|      File: Ap4AdtsParser.h
|
|      AP4 - ADTS Parser
|
|      (c) 2005 Gilles Boccon-Gibod
|      Author: Gilles Boccon-Gibod (bok@bok.net)
|
 ****************************************************************/

#ifndef _AP4_ADTS_PARSER_H_
#define _AP4_ADTS_PARSER_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4BitStream.h"

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
extern const unsigned long AP4_AdtsSamplingFrequencyTable[16];

/*----------------------------------------------------------------------
|       types
+---------------------------------------------------------------------*/
class AP4_AdtsHeader {
public:
    // constructor
    AP4_AdtsHeader(const AP4_UI08* bytes);
    
    // methods
    AP4_Result Check();

    // members

    // fixed part
    unsigned int m_Id;
    unsigned int m_ProtectionAbsent;
    unsigned int m_ProfileObjectType;
    unsigned int m_SamplingFrequencyIndex;
    unsigned int m_ChannelConfiguration;

    // variable part
    unsigned int m_FrameLength;
    unsigned int m_RawDataBlocks;

    // class methods
    static bool MatchFixed(unsigned char* a, unsigned char* b);
};

typedef enum {
    AP4_AAC_STANDARD_MPEG2,
    AP4_AAC_STANDARD_MPEG4
} AP4_AacStandard;

typedef enum {
    AP4_AAC_PROFILE_MAIN,
    AP4_AAC_PROFILE_LC,
    AP4_AAC_PROFILE_SSR,
    AP4_AAC_PROFILE_LTP
} AP4_AacProfile;

typedef struct {
    AP4_AacStandard m_Standard;
    AP4_AacProfile  m_Profile;
    unsigned int    m_SamplingFrequencyIndex;
    unsigned long   m_SamplingFrequency;
    unsigned int    m_ChannelConfiguration;
    unsigned int    m_FrameLength;     
} AP4_AacFrameInfo;

typedef struct {
    AP4_BitStream*   m_Source;
    AP4_AacFrameInfo m_Info;
} AP4_AacFrame;

class AP4_AdtsParser {
public:
    // constructor and destructor
    AP4_AdtsParser();
    virtual ~AP4_AdtsParser();

    // methods
    AP4_Result Reset();
    AP4_Result Feed(const AP4_UI08* buffer, 
                    AP4_Size*       buffer_size,
                    AP4_Flags       flags = 0);
    AP4_Result FindFrame(AP4_AacFrame& frame);
    AP4_Result Skip(AP4_Size size);
    AP4_Size   GetBytesFree();
    AP4_Size   GetBytesAvailable();

private:
    // methods
    AP4_Result FindHeader(AP4_UI08* header);

    // members
    AP4_BitStream m_Bits;
    AP4_Cardinal  m_FrameCount;
};

#endif // _AP4_ADTS_PARSER_H_
