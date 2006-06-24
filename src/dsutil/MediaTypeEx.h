#pragma once

class CMediaTypeEx : public CMediaType
{
public:
	CMediaTypeEx();
	CMediaTypeEx(const CMediaType& mt) {CMediaType::operator = (mt);}

	CString ToString(IPin* pPin = NULL);

	static CString GetVideoCodecName(const GUID& subtype, DWORD biCompression);
	static CString GetAudioCodecName(const GUID& subtype, WORD wFormatTag);
	static CString GetSubtitleCodecName(const GUID& subtype);

	void Dump(CAtlList<CString>& sl);
};
