/*
 * $Id$
 *
 * (C) 2006-2012 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include <BaseClasses/streams.h>
#include <mpeg2data.h>

#include "../DSUtil/GolombBuffer.h"
#include "Mpeg2SectionData.h"


#define BeginEnumDescriptors(gb, nType, nLength)                       \
    {                                                                  \
        BYTE DescBuffer[256];                                          \
        int  nLimit = ((int)gb.BitRead(12)) + gb.GetPos();             \
        while (gb.GetPos() < nLimit)                                   \
        {                                                              \
            MPEG2_DESCRIPTOR nType = (MPEG2_DESCRIPTOR)gb.BitRead(8);  \
            WORD             nLength = (WORD)gb.BitRead(8);

#define SkipDescriptor(gb, nType, nLength)                             \
    gb.ReadBuffer(DescBuffer, nLength);                                \
    TRACE("Skipped descriptor : 0x%02x\n", nType);                     \
    UNREFERENCED_PARAMETER(nType);

#define EndEnumDescriptors }}


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
	static const UINT16 codepages[0x20] = {
		28591,	// 00 - ISO 8859-1 Latin I
		28595,	// 01 - ISO 8859-5 Cyrillic
		28596,	// 02 - ISO 8859-6 Arabic
		28597,	// 03 - ISO 8859-7 Greek
		28598,	// 04 - ISO 8859-8 Hebrew
		28599,	// 05 - ISO 8859-9 Latin 5
		28591,	// 06 - ??? - ISO/IEC 8859-10 - Latin alphabet No. 6
		28591,	// 07 - ??? - ISO/IEC 8859-11 - Latin/Thai (draft only)
		28591,	// 08 - reserved
		28603,	// 09 - ISO 8859-13 - Estonian
		28591,	// 0a - ??? - ISO/IEC 8859-14 - Latin alphabet No. 8 (Celtic)
		28605,	// 0b - ISO 8859-15 Latin 9
		28591,	// 0c - reserved
		28591,	// 0d - reserved
		28591,	// 0e - reserved
		28591,	// 0f - reserved
		0,		// 10 - See codepages10 array
		28591,  // 11 - ??? - ISO/IEC 10646 - Basic Multilingual Plane (BMP)
		28591,  // 12 - ??? - KSX1001-2004 - Korean Character Set
		20936,  // 13 - Chinese Simplified (GB2312-80)
		950,    // 14 - Chinese Traditional (Big5)
		28591,  // 15 - ??? - UTF-8 encoding of ISO/IEC 10646 - Basic Multilingual Plane (BMP)
		28591,  // 16 - reserved
		28591,  // 17 - reserved
		28591,  // 18 - reserved
		28591,  // 19 - reserved
		28591,  // 1a - reserved
		28591,  // 1b - reserved
		28591,  // 1c - reserved
		28591,  // 1d - reserved
		28591,  // 1e - reserved
		28591   // 1f - TODO!
	};

	static const UINT16 codepages10[0x10] = {
		28591,	// 00 - reserved
		28591,	// 01 - ISO 8859-1 Western European
		28592,	// 02 - ISO 8859-2 Central European
		28593,	// 03 - ISO 8859-3 Latin 3
		28594,	// 04 - ISO 8859-4 Baltic
		28595,	// 05 - ISO 8859-5 Cyrillic
		28596,	// 06 - ISO 8859-6 Arabic
		28597,	// 07 - ISO 8859-7 Greek
		28598,	// 08 - ISO 8859-8 Hebrew
		28599,	// 09 - ISO 8859-9 Turkish
		28591,	// 0a - ??? - ISO/IEC 8859-10
		28591,	// 0b - ??? - ISO/IEC 8859-11
		28591,	// 0c - ??? - ISO/IEC 8859-12
		28603,	// 0d - ISO 8859-13 Estonian
		28591,	// 0e - ??? - ISO/IEC 8859-14
		28605,	// 0f - ISO 8859-15 Latin 9

		// 0x10 to 0xFF - reserved for future use
	};

	UINT		cp = CP_ACP;
	int			nDestSize;
	CString		strResult;

	if (nLength > 0) {
		if (pBuffer[0] == 0x10) {
			pBuffer++;
			nLength--;
			if (pBuffer[0] == 0x00) {
				cp = codepages10[pBuffer[1]];
			} else { // if (pBuffer[0] > 0x00)
				// reserved for future use, use default codepage
				cp = codepages[0];
			}
			pBuffer += 2;
			nLength -= 2;
		} else if (pBuffer[0] < 0x20) {
			cp = codepages[pBuffer[0]];
			pBuffer++;
			nLength--;
		} else { // No code page indication, use the default
			cp = codepages[0];
		}
	}

	nDestSize = MultiByteToWideChar (cp, MB_PRECOMPOSED, (LPCSTR)pBuffer, nLength, NULL, 0);
	if (nDestSize < 0) {
		return strResult;
	}

	MultiByteToWideChar (cp, MB_PRECOMPOSED, (LPCSTR)pBuffer, nLength, strResult.GetBuffer(nLength), nDestSize);
	strResult.ReleaseBuffer();

	return strResult;
}

DVB_STREAM_TYPE	CMpeg2DataParser::ConvertToDVBType(PES_STREAM_TYPE nType)
{
	switch (nType) {
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
	if (gb.BitRead(8) != SIType) {
		return ERROR_INVALID_DATA;    // table_id
	}
	gb.BitRead(1);														// section_syntax_indicator
	gb.BitRead(1);														// reserved_future_use
	gb.BitRead(2);														// reserved
	wSectionLength = (WORD)gb.BitRead(12);								// section_length
	wTSID = (WORD)gb.BitRead(16);										// transport_stream_id
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

	wONID = (WORD)gb.BitRead(16);										// original_network_id
	gb.BitRead(8);														// reserved_future_use

	while (gb.GetSize() - gb.GetPos() > 4) {
		CDVBChannel			Channel;
		Channel.SetFrequency (ulFreq);
		Channel.SetTSID (wTSID);
		Channel.SetONID (wONID);
		Channel.SetSID  ((ULONG)gb.BitRead(16));						// service_id   uimsbf
		gb.BitRead(6);													// reserved_future_use   bslbf
		gb.BitRead(1);													// EIT_schedule_flag   bslbf
		Channel.SetNowNextFlag(!!gb.BitRead(1));						// EIT_present_following_flag   bslbf
		gb.BitRead(3);													// running_status   uimsbf
		Channel.SetEncrypted (!!gb.BitRead(1));							// free_CA_mode   bslbf

		// Descriptors:
		BeginEnumDescriptors(gb, nType, nLength) {
			switch (nType) {
				case DT_SERVICE :
					gb.BitRead(8);											// service_type
					nLength = (WORD)gb.BitRead(8);							// service_provider_name_length
					gb.ReadBuffer (DescBuffer, nLength);					// service_provider_name

					nLength = (WORD)gb.BitRead(8);							// service_name_length
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


		if (!Channels.Lookup(Channel.GetSID())) {
			Channels [Channel.GetSID()] = Channel;
		}
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
	while (gb.GetSize() - gb.GetPos() > 4) {
		WORD program_number = (WORD)gb.BitRead(16);			// program_number
		gb.BitRead(3);										// reserved
		if (program_number == 0) {
			gb.BitRead(13);    // network_PID
		} else {
			WORD program_map_PID = (WORD)gb.BitRead(13);	// program_map_PID
			if (Channels.Lookup(program_number)) {
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

	CheckNoLog (m_pData->GetSection (Channel.GetPMT(), SI_PMT, &m_Filter, 5000, &pSectionList));
	CheckNoLog (pSectionList->GetSectionData (0, &dwLength, &data));

	CGolombBuffer	gb ((BYTE*)data, dwLength);

	// TS_program_map_section()
	CheckNoLog (ParseSIHeader (gb, SI_PMT, wSectionLength, wTSID));

	gb.BitRead(3);												// reserved
	Channel.SetPCR((ULONG)gb.BitRead(13));						// PCR_PID
	gb.BitRead(4);												// reserved
	BeginEnumDescriptors(gb, nType, nLength) {				// for (i=0;i<N;i++) {
		SkipDescriptor (gb, nType, nLength);					//		descriptor()
	}
	EndEnumDescriptors


	while (gb.GetSize() - gb.GetPos() > 4) {
		PES_STREAM_TYPE		pes_stream_type;
		DVB_STREAM_TYPE		dvb_stream_type;
		WORD				wPID;
		CString				strLanguage;

		pes_stream_type	= (PES_STREAM_TYPE)gb.BitRead(8);		// stream_type
		gb.BitRead(3);											// reserved
		wPID = (WORD)gb.BitRead(13);							// elementary_PID
		gb.BitRead(4);											// reserved

		BeginEnumDescriptors(gb, nType, nLength) {				// ES_info_length
			switch (nType) {
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
				case DT_SUBTITLING : {
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
		if ((dvb_stream_type = ConvertToDVBType(pes_stream_type)) != DVB_UNKNOWN) {
			Channel.AddStreamInfo (wPID, dvb_stream_type, pes_stream_type, strLanguage);
		}
	}

	return S_OK;
}

HRESULT CMpeg2DataParser::SetTime(CGolombBuffer& gb, PresentFollowing &NowNext)
{
	char	descBuffer[10];
	time_t	tTime1 ,tTime2;
	tm		tmTime1, tmTime2;
	long	nDuration;
	long	timezone;
	int		daylight;

	//	init tm structures
	time( &tTime1 );
	localtime_s(&tmTime1, &tTime1);
	_tzset();
	_get_timezone(&timezone); // The difference in seconds between UTC and local time.
	if (!_get_daylight(&daylight)) {
		timezone -= daylight * 3600;
	}

	// Start time:
	tmTime1.tm_hour = (int)(gb.BitRead(4)*10);
	tmTime1.tm_hour += (int)gb.BitRead(4);
	tmTime1.tm_min  = (int)(gb.BitRead(4)*10);
	tmTime1.tm_min  += (int)gb.BitRead(4);
	tmTime1.tm_sec  = (int)(gb.BitRead(4)*10);
	tmTime1.tm_sec  += (int)gb.BitRead(4);
	tTime1 = mktime(&tmTime1) - timezone;
	localtime_s(&tmTime2, &tTime1);
	tTime1 = mktime(&tmTime2);
	strftime(descBuffer, 6, "%H:%M", &tmTime2);
	descBuffer[6] = '\0';
	NowNext.StartTime = descBuffer;

	// Duration:
	nDuration = (long)(36000*gb.BitRead(4));
	nDuration += (long)(3600*gb.BitRead(4));
	nDuration += (long)(600*gb.BitRead(4));
	nDuration += (long)(60*gb.BitRead(4));
	nDuration += (long)(10*gb.BitRead(4));
	nDuration += (long)gb.BitRead(4);

	tTime2 = tTime1 + nDuration;
	localtime_s(&tmTime2, &tTime2);
	strftime(descBuffer, 6, "%H:%M", &tmTime2);
	descBuffer[6] = '\0';
	NowNext.Duration = descBuffer;

	return S_OK;
}

HRESULT CMpeg2DataParser::ParseEIT(ULONG ulSID, PresentFollowing &NowNext)
{
	HRESULT					hr;
	CComPtr<ISectionList>	pSectionList;
	DWORD					dwLength;
	PSECTION				data;
	ULONG					ulGetSID;
	EventInformationSection InfoEvent;
	int nLen;
	int descriptorNumber;
	int itemsLength;
	CString itemDesc, itemText;
	CString text;

	do {
		CheckNoLog (m_pData->GetSection (PID_EIT, SI_EIT_act, NULL, 5000, &pSectionList));

		CheckNoLog (pSectionList->GetSectionData (0, &dwLength, &data));
		CGolombBuffer	gb ((BYTE*)data, dwLength);

		InfoEvent.TableID = (UINT8)gb.BitRead(8);
		InfoEvent.SectionSyntaxIndicator = (WORD)gb.BitRead(1);
		gb.BitRead(3);
		InfoEvent.SectionLength = (WORD)gb.BitRead(12);
		ulGetSID  = (ULONG)gb.BitRead(8);
		ulGetSID += 0x100 * (ULONG)gb.BitRead(8);
		InfoEvent.ServiceId = ulGetSID; // This is really strange, ServiceID should be uimsbf ???
		gb.BitRead(2);
		InfoEvent.VersionNumber = (UINT8)gb.BitRead(5);
		InfoEvent.CurrentNextIndicator = (UINT8)gb.BitRead(1);

		InfoEvent.SectionNumber = (UINT8)gb.BitRead(8);
		InfoEvent.LastSectionNumber = (UINT8)gb.BitRead(8);
		InfoEvent.TransportStreamID = (WORD)gb.BitRead(16);
		InfoEvent.OriginalNetworkID = (WORD)gb.BitRead(16);
		InfoEvent.SegmentLastSectionNumber = (UINT8)gb.BitRead(8);
		InfoEvent.LastTableID = (UINT8)gb.BitRead(8);

		// Info event
		InfoEvent.EventID   = (WORD)gb.BitRead(16);
		InfoEvent.StartDate = (WORD)gb.BitRead(16);
		SetTime(gb, NowNext);

		InfoEvent.RunninStatus = (WORD)gb.BitRead(3);
		InfoEvent.FreeCAMode   = (WORD)gb.BitRead(1);

		NowNext.ExtendedDescriptorsItems.RemoveAll();
		NowNext.ExtendedDescriptorsTexts.RemoveAll();

		if ((InfoEvent.ServiceId == ulSID) && (InfoEvent.CurrentNextIndicator == 1) && (InfoEvent.RunninStatus == 4)) {
			//	Descriptors:
			BeginEnumDescriptors(gb, nType, nLength) {
				switch (nType) {
					case DT_SHORT_EVENT:
						gb.BitRead(24); // ISO_639_language_code

						nLen = (UINT8)gb.BitRead(8); // event_name_length
						gb.ReadBuffer(DescBuffer, nLen);
						NowNext.cPresent = ConvertString(DescBuffer, nLen);

						nLen = (UINT8)gb.BitRead(8); // text_length
						gb.ReadBuffer(DescBuffer, nLen);
						NowNext.SummaryDesc = ConvertString(DescBuffer, nLen);
						break;
					case DT_EXTENDED_EVENT:
						descriptorNumber = (UINT8)gb.BitRead(4); // descriptor_number
						gb.BitRead(4); // last_descriptor_number
						gb.BitRead(24); // ISO_639_language_code

						itemsLength = (UINT8)gb.BitRead(8); // length_of_items
						while (itemsLength > 0) {
							nLen = (UINT8)gb.BitRead(8); // item_description_length
							gb.ReadBuffer(DescBuffer, nLen);
							itemDesc = ConvertString(DescBuffer, nLen);
							itemsLength -= nLen + 1;

							nLen = (UINT8)gb.BitRead(8); // item_length
							gb.ReadBuffer(DescBuffer, nLen);
							itemText = ConvertString(DescBuffer, nLen);
							itemsLength -= nLen + 1;

							NowNext.ExtendedDescriptorsItems.SetAt(itemDesc, itemText);
						}

						nLen = (UINT8)gb.BitRead(8); // text_length
						gb.ReadBuffer(DescBuffer, nLen);
						text = ConvertString(DescBuffer, nLen);
						if (descriptorNumber == 0) { // new descriptor set
							NowNext.ExtendedDescriptorsTexts.AddTail(text);
						} else {
							NowNext.ExtendedDescriptorsTexts.GetTail().Append(text);
						}
						break;
					default:
						SkipDescriptor (gb, nType, nLength);
						break;
				}
			}
			EndEnumDescriptors
		}
		m_Filter.SectionNumber++;
		pSectionList.Release();
	} while (((InfoEvent.ServiceId != ulSID) || (InfoEvent.CurrentNextIndicator != 1) || (InfoEvent.RunninStatus != 4)) &&
			 (m_Filter.SectionNumber <= 22));

	if (InfoEvent.ServiceId != ulSID) {
		NowNext.StartTime = _T("");
		NowNext.Duration = _T("");
		NowNext.cPresent = _T(" Info not available.");
		NowNext.SummaryDesc = _T("");
		NowNext.cFollowing = _T("");
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
	BeginEnumDescriptors(gb, nType, nLength) {				// for (i=0;i<N;i++) {
		SkipDescriptor (gb, nType, nLength);					//		descriptor()
	}
	EndEnumDescriptors

	gb.BitRead(4);												// reserved_future_use
	transport_stream_loop_length = (WORD)gb.BitRead(12);		// network_descriptors_length
	while (gb.GetSize() - gb.GetPos() > 4) {
		WORD	transport_stream_id = (WORD)gb.BitRead(16);		// transport_stream_id
		UNREFERENCED_PARAMETER(transport_stream_id);
		WORD	original_network_id = (WORD)gb.BitRead(16);		// original_network_id
		UNREFERENCED_PARAMETER(original_network_id);
		gb.BitRead(4);											// reserved_future_use
		BeginEnumDescriptors (gb, nType, nLength) {
			switch (nType) {
				case DT_LOGICAL_CHANNEL :
					for (int i=0; i<nLength/4; i++) {
						WORD	service_id	= (WORD)gb.BitRead(16);
						gb.BitRead(6);
						WORD	logical_channel_number	= (WORD)gb.BitRead(10);
						if (Channels.Lookup(service_id)) {
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
