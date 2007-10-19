#pragma once

#include "..\BaseVideoFilter\BaseVideoFilter.h"


class CMPCVideoDecFilter;
class CVideoDecDXVAAllocator;


class CVideoDecOutputPin : public CBaseVideoOutputPin
{
public:
	CVideoDecOutputPin(TCHAR* pObjectName, CBaseVideoFilter* pFilter, HRESULT* phr, LPCWSTR pName);

	~CVideoDecOutputPin();

	HRESULT			InitAllocator(IMemAllocator **ppAlloc);
	HRESULT			DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc);

private :
	CMPCVideoDecFilter*		m_pVideoDecFilter;
	CVideoDecDXVAAllocator*	m_pDXVAAllocator;
};
