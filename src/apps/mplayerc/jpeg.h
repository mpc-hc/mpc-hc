#pragma once

class CJpegEncoder
{
	static const int ColorComponents = 3;

	int m_w, m_h;
	BYTE* m_p;

	unsigned int m_bbuff, m_bwidth;
	bool PutBit(int b, int n);
	void Flush();
	int GetBitWidth(short q);

	void WriteSOI();
	void WriteDQT();
	void WriteSOF0();
	void WriteDHT();
	void WriteSOS();
	void WriteEOI();

protected:
	virtual bool PutByte(BYTE b) = 0;
	virtual bool PutBytes(const void* pData, int len) = 0;
	virtual bool Encode(const BYTE* dib);

public:
	CJpegEncoder();
};

class CJpegEncoderFile : public CJpegEncoder
{
	CString m_fn;
	FILE* m_file;

protected:
	bool PutByte(BYTE b);
	bool PutBytes(const void* pData, int len);

public:
	CJpegEncoderFile(LPCTSTR fn);

	bool Encode(const BYTE* dib);
};

class CJpegEncoderMem : public CJpegEncoder
{
	CAtlArray<BYTE>* m_pdata;

protected:
	bool PutByte(BYTE b);
	bool PutBytes(const void* pData, int len);

public:
	CJpegEncoderMem();

	bool Encode(const BYTE* dib, CAtlArray<BYTE>& data);
};

