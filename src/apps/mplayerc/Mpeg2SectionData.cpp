
#include "stdafx.h"
#include <streams.h>
#include <mpeg2data.h>

#include "../../DSUtil/GolombBuffer.h"
#include "Mpeg2SectionData.h"

#define BeginEnumDescriptors(gb, nType, nLength)								\
	{																			\
		BYTE	DescBuffer[256];												\
		int		nLimit = gb.BitRead(12)+gb.GetPos();							\
		while (gb.GetPos() < nLimit)											\
		{																		\
			MPEG2_DESCRIPTOR 	nType	= (MPEG2_DESCRIPTOR)gb.BitRead(8);		\
			WORD			 	nLength	= gb.BitRead(8);

#define SkipDescriptor(gb, nType, nLength)										\
			gb.ReadBuffer(DescBuffer, nLength);									\
			TRACE ("Skipped descriptor : 0x%02x\n", nType);

#define EndEnumDescriptors	}}


CMpeg2DataParser::CMpeg2DataParser(IBaseFilter* pFilter)
{
    m_pData = pFilter;

    memset(&m_Filter, 0, sizeof(m_Filter));
    m_Filter.bVersionNumber			= 1;
    m_Filter.wFilterSize			= MPEG2_FILTER_VERSION_1_SIZE;
    m_Filter.fSpecifySectionNumber	= TRUE;
}

CString CMpeg2DataParser::ConvertString (BYTE* pBuffer, int nLength)
{
    static const UINT16 codepages[0x20] =
    {
        28591,	// 00 - ISO 8859-1 Latin I
        28595,	// 01 - ISO 8859-5 Cyrillic
        28596,	// 02 - ISO 8859-6 Arabic
        28597,	// 03 - ISO 8859-7 Greek
        28598,	// 04 - ISO 8859-8 Hebrew
        28599,	// 05 - ISO 8859-9 Latin 5
        28591,	// 06 - ??? - ISO/IEC 8859-10 - Latin alphabet No. 6
        28591,	// 07 - ??? - ISO/IEC 8859-11 - Latin/Thai (draft only)
        28591,	// 08 - ??? - ISO/IEC 8859-12 - possibly reserved for Indian
        28591,	// 09 - ??? - ISO/IEC 8859-13 - Latin alphabet No. 7
        28591,	// 0a - ??? - ISO/IEC 8859-14 - Latin alphabet No. 8 (Celtic)
        28605,	// 0b - ISO 8859-15 Latin 9
        28591,	// 0c - réservé
        28591,	// 0d - réservé
        28591,	// 0e - réservé
        28591,	// 0f - réservé

        // TODO !
        28591, 28591, 28591, 28591, 28591, 28591, 28591, 28591,
        28591, 28591, 28591, 28591, 28591, 28591, 28591, 28591
    };

    UINT		cp = CP_ACP;
    int			nDestSize;
    CString		strResult;

    if (nLength>0 && pBuffer[0]<0x20)
    {
        cp = codepages[pBuffer[0]];
        pBuffer++;
        nLength--;
    }

    nDestSize = MultiByteToWideChar (cp, MB_PRECOMPOSED, (LPCSTR)pBuffer, nLength, NULL, 0);
    if(nDestSize < 0) return strResult;

    MultiByteToWideChar (cp, MB_PRECOMPOSED, (LPCSTR)pBuffer, nLength, strResult.GetBuffer(nLength), nDestSize);
    strResult.ReleaseBuffer();

    return strResult;
}

DVB_STREAM_TYPE	CMpeg2DataParser::ConvertToDVBType(PES_STREAM_TYPE nType)
{
    switch (nType)
    {
    case VIDEO_STREAM_MPEG1 :
    case VIDEO_STREAM_MPEG2 :
        return DVB_MPV;
    case AUDIO_STREAM_MPEG1 :
    case AUDIO_STREAM_MPEG2 :
        return DVB_MPA;
    case VIDEO_STREAM_H264  :
        return DVB_H264;
    case AUDIO_STREAM_AC3 :
        return DVB_AC3;
    case AUDIO_STREAM_AC3_PLUS :
        return DVB_EAC3;
    case SUBTITLE_STREAM :
        return DVB_SUBTITLE;
    }

    return DVB_UNKNOWN;
}

HRESULT CMpeg2DataParser::ParseSIHeader(CGolombBuffer& gb, DVB_SI SIType, WORD& wSectionLength, WORD& wTSID)
{
    if (gb.BitRead(8) != SIType) return ERROR_INVALID_DATA;				// table_id
    gb.BitRead(1);														// section_syntax_indicator
    gb.BitRead(1);														// reserved_future_use
    gb.BitRead(2);														// reserved
    wSectionLength = gb.BitRead(12);									// section_length
    wTSID = gb.BitRead(16);												// transport_stream_id
    gb.BitRead(2);														// reserved
    gb.BitRead(5);														// version_number
    gb.BitRead(1);														// current_next_indicator
    gb.BitRead(8);														// section_number
    gb.BitRead(8);														// last_section_number

    return S_OK;
}


HRESULT CMpeg2DataParser::ParseSDT(ULONG ulFreq)
{
    HRESULT					hr;
    CComPtr<ISectionList>	pSectionList;
    DWORD					dwLength;
    PSECTION				data;
    WORD					wTSID;
    WORD					wONID;
    WORD					wSectionLength;

    CheckNoLog (m_pData->GetSection (PID_SDT, SI_SDT, &m_Filter, 5000, &pSectionList));
    CheckNoLog (pSectionList->GetSectionData (0, &dwLength, &data));

    CGolombBuffer	gb ((BYTE*)data, dwLength);

    // service_description_section()
    CheckNoLog (ParseSIHeader (gb, SI_SDT, wSectionLength, wTSID));

    wONID = gb.BitRead(16);												// original_network_id
    gb.BitRead(8);														// reserved_future_use

    while (gb.GetSize() - gb.GetPos() > 4)
    {
        CDVBChannel			Channel;
        Channel.SetFrequency (ulFreq);
        Channel.SetTSID (wTSID);
        Channel.SetONID (wONID);
        Channel.SetSID  (gb.BitRead(16));								// service_id   uimsbf
        gb.BitRead(6);													// reserved_future_use   bslbf
        gb.BitRead(1);													// EIT_schedule_flag   bslbf
        gb.BitRead(1);													// EIT_present_following_flag   bslbf
        gb.BitRead(3);													// running_status   uimsbf
        gb.BitRead(1);													// free_CA_mode   bslbf

        BeginEnumDescriptors(gb, nType, nLength)
        {
            switch (nType)
            {
            case DT_SERVICE :
                gb.BitRead(8);											// service_type
                nLength = gb.BitRead(8);								// service_provider_name_length
                gb.ReadBuffer (DescBuffer, nLength);					// service_provider_name

                nLength = gb.BitRead(8);								// service_name_length
                gb.ReadBuffer (DescBuffer, nLength);					// service_name
                DescBuffer[nLength] = 0;
                Channel.SetName (ConvertString (DescBuffer, nLength));
                TRACE ("%15S %d\n", Channel.GetName(), Channel.GetSID());
                break;
            default :
                SkipDescriptor (gb, nType, nLength);					// descriptor()
                break;
            }
        }
        EndEnumDescriptors


        if (!Channels.Lookup(Channel.GetSID()))
            Channels [Channel.GetSID()] = Channel;
    }

    return S_OK;
}


HRESULT CMpeg2DataParser::ParsePAT()
{
    HRESULT					hr;
    CComPtr<ISectionList>	pSectionList;
    DWORD					dwLength;
    PSECTION				data;
    WORD					wTSID;
    WORD					wSectionLength;

    CheckNoLog (m_pData->GetSection (PID_PAT, SI_PAT, &m_Filter, 5000, &pSectionList));
    CheckNoLog (pSectionList->GetSectionData (0, &dwLength, &data));

    CGolombBuffer	gb ((BYTE*)data, dwLength);

    // program_association_section()
    CheckNoLog (ParseSIHeader (gb, SI_PAT, wSectionLength, wTSID));
    while (gb.GetSize() - gb.GetPos() > 4)
    {
        WORD		program_number;
        WORD		program_map_PID	= 0;

        program_number	= gb.BitRead(16);				// program_number
        gb.BitRead(3);									// reserved
        if (program_number==0)
            gb.BitRead(13);								// network_PID
        else
        {
            program_map_PID = gb.BitRead(13);			// program_map_PID
            if (Channels.Lookup(program_number))
            {
                Channels [program_number].SetPMT (program_map_PID);
                ParsePMT (Channels [program_number]);
            }
        }
    }

    return S_OK;
}


HRESULT CMpeg2DataParser::ParsePMT(CDVBChannel& Channel)
{
    HRESULT					hr;
    CComPtr<ISectionList>	pSectionList;
    DWORD					dwLength;
    PSECTION				data;
    WORD					wTSID;
    WORD					wSectionLength;
    int						nLimit;

    CheckNoLog (m_pData->GetSection (Channel.GetPMT(), SI_PMT, &m_Filter, 5000, &pSectionList));
    CheckNoLog (pSectionList->GetSectionData (0, &dwLength, &data));

    CGolombBuffer	gb ((BYTE*)data, dwLength);

    // TS_program_map_section()
    CheckNoLog (ParseSIHeader (gb, SI_PMT, wSectionLength, wTSID));

    gb.BitRead(3);												// reserved
    Channel.SetPCR (gb.BitRead(13));							// PCR_PID
    gb.BitRead(4);												// reserved
    BeginEnumDescriptors(gb, nType, nLength)					// for (i=0;i<N;i++) {
    {
        SkipDescriptor (gb, nType, nLength);					//		descriptor()
    }
    EndEnumDescriptors


    while (gb.GetSize() - gb.GetPos() > 4)
    {
        PES_STREAM_TYPE		pes_stream_type;
        DVB_STREAM_TYPE		dvb_stream_type;
        WORD				wPID;
        CString				strLanguage;

        pes_stream_type	= (PES_STREAM_TYPE)gb.BitRead(8);		// stream_type
        gb.BitRead(3);											// reserved
        wPID		= gb.BitRead(13);							// elementary_PID
        gb.BitRead(4);											// reserved

        BeginEnumDescriptors(gb, nType, nLength)				// ES_info_length
        {
            switch (nType)
            {
            case DT_ISO_639_LANGUAGE :
                gb.ReadBuffer(DescBuffer, nLength);
                strLanguage = ConvertString (DescBuffer, 3);
                break;
            case DT_AC3_AUDIO :
                pes_stream_type = AUDIO_STREAM_AC3;
                SkipDescriptor (gb, nType, nLength);
                break;
            case DT_EXTENDED_AC3_AUDIO :
                pes_stream_type = AUDIO_STREAM_AC3_PLUS;
                SkipDescriptor (gb, nType, nLength);
                break;
            case DT_SUBTITLING :
            {
                gb.ReadBuffer(DescBuffer, nLength);
                strLanguage = ConvertString (DescBuffer, 3);
                pes_stream_type = SUBTITLE_STREAM;
            }
            break;
            default :
                SkipDescriptor (gb, nType, nLength);
                break;
            }
        }
        EndEnumDescriptors
        if ((dvb_stream_type = ConvertToDVBType(pes_stream_type)) != DVB_UNKNOWN)
            Channel.AddStreamInfo (wPID, dvb_stream_type, pes_stream_type, strLanguage);
    }

    return S_OK;
}


HRESULT CMpeg2DataParser::ParseNIT()
{
    HRESULT					hr;
    CComPtr<ISectionList>	pSectionList;
    DWORD					dwLength;
    PSECTION				data;
    WORD					wTSID;
    WORD					wSectionLength;
    WORD					transport_stream_loop_length;

    CheckNoLog (m_pData->GetSection (PID_NIT, SI_NIT, &m_Filter, 5000, &pSectionList));
    CheckNoLog (pSectionList->GetSectionData (0, &dwLength, &data));

    CGolombBuffer	gb ((BYTE*)data, dwLength);

    // network_information_section()
    CheckNoLog (ParseSIHeader (gb, SI_NIT, wSectionLength, wTSID));

    gb.BitRead(4);												// reserved_future_use
    BeginEnumDescriptors(gb, nType, nLength)					// for (i=0;i<N;i++) {
    {
        SkipDescriptor (gb, nType, nLength);					//		descriptor()
    }
    EndEnumDescriptors

    gb.BitRead(4);												// reserved_future_use
    transport_stream_loop_length = gb.BitRead(12);				// network_descriptors_length
    while (gb.GetSize() - gb.GetPos() > 4)
    {
        WORD	transport_stream_id = gb.BitRead(16);			// transport_stream_id
        WORD	original_network_id = gb.BitRead(16);			// original_network_id
        gb.BitRead(4);											// reserved_future_use
        BeginEnumDescriptors (gb, nType, nLength)
        {
            switch (nType)
            {
            case DT_LOGICAL_CHANNEL :
                for (int i=0; i<nLength/4; i++)
                {
                    WORD	service_id	= gb.BitRead (16);
                    gb.BitRead(6);
                    WORD	logical_channel_number	= gb.BitRead(10);
                    if (Channels.Lookup(service_id))
                    {
                        Channels[service_id].SetOriginNumber (logical_channel_number);
                        TRACE ("NIT association : %d -> %S\n", logical_channel_number, Channels[service_id].ToString());
                    }
                }
                break;
            default :
                SkipDescriptor (gb, nType, nLength);
                break;
            }
        }
        EndEnumDescriptors
    }

    return S_OK;
}