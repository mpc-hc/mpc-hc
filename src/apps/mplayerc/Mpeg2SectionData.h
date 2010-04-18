#pragma once

#include "DVBChannel.h"


#pragma pack(1)
typedef struct
{
    UINT8		TableID;
    WORD		SectionSyntaxIndicator  : 1;
    WORD		Reserved1				: 3;
    WORD		SectionLength			: 12;
    WORD		BouquetID;
    UINT8		Reserved2				: 1;
    UINT8		VersionNumber			: 5;
    UINT8		CurrentNextIndicator	: 1;
    UINT8		SectionNumber;
    UINT8		LastSectionNumber;
} SI_HEADER;



class CMpeg2DataParser
{
public :

    CMpeg2DataParser(IBaseFilter* pFilter);

    HRESULT		ParseSDT(ULONG ulFreq);
    HRESULT		ParsePAT();
    HRESULT		ParseNIT();

    static CString ConvertString (BYTE* pBuffer, int nLength);

    CAtlMap<int,CDVBChannel>	Channels;

private :

    CComQIPtr<IMpeg2Data>		m_pData;
    MPEG2_FILTER				m_Filter;


    DVB_STREAM_TYPE	ConvertToDVBType(PES_STREAM_TYPE nType);
    HRESULT			ParseSIHeader(CGolombBuffer& gb, DVB_SI SIType, WORD& wSectionLength, WORD& wTSID);
    HRESULT			ParsePMT(CDVBChannel& Channel);
};
