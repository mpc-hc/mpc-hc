#include "stdafx.h"
#include "VideoDecOutputPin.h"
#include "VideoDecDXVAAllocator.h"
#include "MPCVideoDecFilter.h"

CVideoDecOutputPin::CVideoDecOutputPin(TCHAR* pObjectName, CBaseVideoFilter* pFilter, HRESULT* phr, LPCWSTR pName)
				  : CBaseVideoOutputPin(pObjectName, pFilter, phr, pName)
{
	m_pVideoDecFilter	= (CMPCVideoDecFilter*) pFilter;
	m_pDXVAAllocator	= NULL;
}

CVideoDecOutputPin::~CVideoDecOutputPin(void)
{
}


HRESULT CVideoDecOutputPin::InitAllocator(IMemAllocator **ppAlloc)
{
	TRACE("CVideoDecOutputPin::InitAllocator");
	if (m_pVideoDecFilter->UseDXVA2())
	{
		HRESULT hr = S_FALSE;
		m_pDXVAAllocator = new CVideoDecDXVAAllocator(m_pVideoDecFilter, &hr);
		if (!m_pDXVAAllocator)
		{
			return E_OUTOFMEMORY;
		}
		if (FAILED(hr))
		{
			delete m_pDXVAAllocator;
			return hr;
		}
		// Return the IMemAllocator interface.
		return m_pDXVAAllocator->QueryInterface(__uuidof(IMemAllocator), (void **)ppAlloc);
	}
	else
		return __super::InitAllocator(ppAlloc);
}


HRESULT CVideoDecOutputPin::DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc)
{
	if (m_pVideoDecFilter->UseDXVA2())
	{
		HRESULT hr = NOERROR;
		*ppAlloc = NULL;

		// get downstream prop request
		// the derived class may modify this in DecideBufferSize, but
		// we assume that he will consistently modify it the same way,
		// so we only get it once
		ALLOCATOR_PROPERTIES prop;
		ZeroMemory(&prop, sizeof(prop));

		// whatever he returns, we assume prop is either all zeros
		// or he has filled it out.
		pPin->GetAllocatorRequirements(&prop);

		// if he doesn't care about alignment, then set it to 1
		if (prop.cbAlign == 0) {
			prop.cbAlign = 1;
		}

		/* If the GetAllocator failed we may not have an interface */

		if (*ppAlloc) {
			(*ppAlloc)->Release();
			*ppAlloc = NULL;
		}

		/* Try the output pin's allocator by the same method */

		hr = InitAllocator(ppAlloc);
		if (SUCCEEDED(hr)) {

			// note - the properties passed here are in the same
			// structure as above and may have been modified by
			// the previous call to DecideBufferSize
			hr = DecideBufferSize(*ppAlloc, &prop);
			if (SUCCEEDED(hr)) {
				hr = pPin->NotifyAllocator(*ppAlloc, FALSE);
				if (SUCCEEDED(hr)) {
					return NOERROR;
				}
			}
		}

		/* Likewise we may not have an interface to release */

		if (*ppAlloc) {
			(*ppAlloc)->Release();
			*ppAlloc = NULL;
		}
		return hr;
	}
	else
		return __super::DecideAllocator(pPin, ppAlloc);
}
