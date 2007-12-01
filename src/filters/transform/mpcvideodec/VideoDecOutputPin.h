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
//	HRESULT			DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc);

	HRESULT			Deliver(IMediaSample* pMediaSample);
	HRESULT			Active();
	HRESULT			Inactive();

	HRESULT			DeliverEndOfStream();
	HRESULT			DeliverBeginFlush();
	HRESULT			DeliverEndFlush();
	HRESULT			DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

private :
	CMPCVideoDecFilter*		m_pVideoDecFilter;
	CVideoDecDXVAAllocator*	m_pDXVAAllocator;

	CAutoPtr<COutputQueue>	m_pOutputQueue;
};
