#include "stdafx.h"
#include "VideoDecDXVAAllocator.h"
#include "MPCVideoDecFilter.h"


CDXVA2Sample::CDXVA2Sample(CVideoDecDXVAAllocator *pAlloc, HRESULT *phr)
			: CMediaSample(NAME("CDXVA2Sample"), (CBaseAllocator*)pAlloc, phr, NULL, 0)
			, m_pSurface(NULL)
			, m_dwSurfaceId(0)
{ 
}

// Note: CMediaSample does not derive from CUnknown, so we cannot use the
//       DECLARE_IUNKNOWN macro that is used by most of the filter classes.

STDMETHODIMP CDXVA2Sample::QueryInterface(REFIID riid, void **ppv)
{
	CheckPointer(ppv, E_POINTER);

	if (riid == __uuidof(IMFGetService))
	{
		*ppv = static_cast<IMFGetService*>(this);
		return S_OK;
	}
	else
	{
		return CMediaSample::QueryInterface(riid, ppv);
	}
}


STDMETHODIMP_(ULONG) CDXVA2Sample::AddRef()
{
	return CMediaSample::AddRef();
}

STDMETHODIMP_(ULONG) CDXVA2Sample::Release()
{
	// Return a temporary variable for thread safety.
	ULONG cRef = CMediaSample::Release();
	return cRef;
}

// IMFGetService::GetService
STDMETHODIMP CDXVA2Sample::GetService(REFGUID guidService, REFIID riid, LPVOID *ppv)
{
	if (guidService != MR_BUFFER_SERVICE)
	{
		return MF_E_UNSUPPORTED_SERVICE;
	}
	else if (m_pSurface == NULL)
	{
		return E_NOINTERFACE;
	}
	else
	{
		return m_pSurface->QueryInterface(riid, ppv);
	}
}

// Override GetPointer because this class does not manage a system memory buffer.
// The EVR uses the MR_BUFFER_SERVICE service to get the Direct3D surface.
STDMETHODIMP CDXVA2Sample::GetPointer(BYTE ** ppBuffer)
{
	return E_NOTIMPL;
}



// Sets the pointer to the Direct3D surface. 
void CDXVA2Sample::SetSurface(DWORD surfaceId, IDirect3DSurface9 *pSurf)
{
	if (m_pSurface != NULL) m_pSurface->Release();

	m_pSurface = pSurf;
	if (m_pSurface)
	{
		m_pSurface->AddRef();
	}

	m_dwSurfaceId = surfaceId;
}



CVideoDecDXVAAllocator::CVideoDecDXVAAllocator(CMPCVideoDecFilter* pVideoDecFilter,  HRESULT* phr)
					  : CMemAllocator(NAME("CVideoDecDXVAAllocator"), NULL, phr)
{
	m_pVideoDecFilter	= pVideoDecFilter;
	m_pDXVA2Service		= pVideoDecFilter->m_pAccelerationService;
}



HRESULT CVideoDecDXVAAllocator::Alloc()
{
	CheckPointer (m_pDXVA2Service, E_UNEXPECTED);
	CAutoLock lock(this);

    HRESULT hr = S_OK;

    hr = __super::Alloc();

    // If the requirements have not changed, do not reallocate.
    if (hr == S_FALSE)
    {
        return S_OK;
    }

    if (SUCCEEDED(hr))
    {
        // Free the old resources.
        Free();

		cSurfaceArray = m_lCount;

        // Allocate a new array of pointers.
        m_ppRTSurfaceArray = new IDirect3DSurface9*[m_lCount];
        if (m_ppRTSurfaceArray == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            ZeroMemory(m_ppRTSurfaceArray, sizeof(IDirect3DSurface9*) * m_lCount);
        }
    }

    // Allocate the surfaces.
	D3DFORMAT m_dwFormat = m_pVideoDecFilter->m_VideoDesc.Format;
    if (SUCCEEDED(hr))
    {
        hr = m_pDXVA2Service->CreateSurface(
            m_pVideoDecFilter->PictWidth(),
            m_pVideoDecFilter->PictHeight(),
            m_lCount - 1,
            (D3DFORMAT)m_dwFormat,
            D3DPOOL_DEFAULT,
            0,
            DXVA2_VideoDecoderRenderTarget,
            m_ppRTSurfaceArray,
            NULL
            );
    }

    if (SUCCEEDED(hr))
    {
        for (m_lAllocated = 0; m_lAllocated < m_lCount; m_lAllocated++)
        {
            CDXVA2Sample *pSample = new CDXVA2Sample(this, &hr);
            if (pSample == NULL)
            {
                hr = E_OUTOFMEMORY;
                break;
            }
            if (FAILED(hr))
            {
                break;
            }
            // Assign the Direct3D surface pointer and the index.
            pSample->SetSurface(m_lAllocated, m_ppRTSurfaceArray[m_lAllocated]);

            // Add to the sample list.
            m_lFree.Add(pSample);
        }
    }

    if (SUCCEEDED(hr))
    {
        m_bChanged = FALSE;
    }
    return hr;
}

void CVideoDecDXVAAllocator::Free()
{
    CMediaSample *pSample = NULL;

    do
    {
        pSample = m_lFree.RemoveHead();
        if (pSample)
        {
            delete pSample;
        }
    } while (pSample);

    if (m_ppRTSurfaceArray)
    {
        for (long i = 0; i < m_lAllocated; i++)
        {
            if (m_ppRTSurfaceArray[i] != NULL)
				m_ppRTSurfaceArray[i]->Release();
        }

        delete [] m_ppRTSurfaceArray;
    }
    m_lAllocated = 0;
}
