#pragma once


class CMPCVideoDecFilter;
class CVideoDecDXVAAllocator;


class CDXVA2Sample : public CMediaSample, public IMFGetService
{
    friend class CVideoDecDXVAAllocator;

public:

    CDXVA2Sample(CVideoDecDXVAAllocator *pAlloc, HRESULT *phr);

    // Note: CMediaSample does not derive from CUnknown, so we cannot use the
    //       DECLARE_IUNKNOWN macro that is used by most of the filter classes.

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);

    STDMETHODIMP_(ULONG) AddRef();

    STDMETHODIMP_(ULONG) Release();

    // IMFGetService::GetService
    STDMETHODIMP GetService(REFGUID guidService, REFIID riid, LPVOID *ppv);

    // Override GetPointer because this class does not manage a system memory buffer.
    // The EVR uses the MR_BUFFER_SERVICE service to get the Direct3D surface.
    STDMETHODIMP GetPointer(BYTE ** ppBuffer);

private:

    // Sets the pointer to the Direct3D surface. 
    void SetSurface(DWORD surfaceId, IDirect3DSurface9 *pSurf);

    IDirect3DSurface9   *m_pSurface;
    DWORD               m_dwSurfaceId;
};

class CVideoDecDXVAAllocator : public CMemAllocator
{
public:
	CVideoDecDXVAAllocator(CMPCVideoDecFilter* pVideoDecFilter, HRESULT* phr);

protected:
	HRESULT		Alloc(void);
	void		Free(void);

private :
	CMPCVideoDecFilter*		m_pVideoDecFilter;


	CComPtr<IDirectXVideoAccelerationService>	m_pDXVA2Service;
	IDirect3DSurface9**							m_ppRTSurfaceArray;
	UINT										cSurfaceArray;

};
