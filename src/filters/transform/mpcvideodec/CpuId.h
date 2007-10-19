#pragma once

class CCpuId
{
public:

	typedef enum PROCESSOR_TYPE
	{
		PROCESSOR_AMD,
		PROCESSOR_INTEL,
		PROCESSOR_UNKNOWN
	};

	// Enum codes identicals to FFMpeg cpu features define !
	typedef enum PROCESSOR_FEATURES
	{
		MM_MMX      = 0x0001, /* standard MMX */
		MM_3DNOW    = 0x0004, /* AMD 3DNOW */
		MM_MMXEXT   = 0x0002, /* SSE integer functions or AMD MMX ext */
		MM_SSE      = 0x0008, /* SSE functions */
		MM_SSE2     = 0x0010, /* PIV SSE2 functions */
		MM_3DNOWEXT = 0x0020, /* AMD 3DNowExt */
		MM_SSE3     = 0x0040, /* AMD64 & PIV SSE3 functions*/
		MM_SSSE3    = 0x0080  /* PIV Core 2 SSSE3 functions*/
	};

	CCpuId();

	int					GetFeatures()	{ return m_nCPUFeatures; };
	PROCESSOR_TYPE		GetType()		{ return m_nType; };
	int					GetProcessorNumber();

private :
	int					m_nCPUFeatures;
	PROCESSOR_TYPE		m_nType;
};

