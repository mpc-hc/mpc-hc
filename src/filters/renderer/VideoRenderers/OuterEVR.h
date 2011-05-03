/*
 * $Id$
 *
 * (C) 2006-2011 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

/// === Outer EVR
namespace DSObjects
{
	class COuterEVR
		: public CUnknown
		, public IVMRffdshow9
		, public IVMRMixerBitmap9
		, public IBaseFilter
	{
		CComPtr<IUnknown>	m_pEVR;
		VMR9AlphaBitmap*	m_pVMR9AlphaBitmap;
		CEVRAllocatorPresenter *m_pAllocatorPresenter;

	public:
		COuterEVR(const TCHAR* pName, LPUNKNOWN pUnk, HRESULT& hr, VMR9AlphaBitmap* pVMR9AlphaBitmap, CEVRAllocatorPresenter *pAllocatorPresenter) : CUnknown(pName, pUnk) {
			hr = m_pEVR.CoCreateInstance(CLSID_EnhancedVideoRenderer, GetOwner());
			m_pVMR9AlphaBitmap = pVMR9AlphaBitmap;
			m_pAllocatorPresenter = pAllocatorPresenter;
		}

		~COuterEVR() {}

		DECLARE_IUNKNOWN;
		STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv) {
			HRESULT hr;

			if(riid == __uuidof(IVMRMixerBitmap9)) {
				return GetInterface((IVMRMixerBitmap9*)this, ppv);
			}
			if (riid == __uuidof(IMediaFilter)) {
				return GetInterface((IMediaFilter*)this, ppv);
			}
			if (riid == __uuidof(IPersist)) {
				return GetInterface((IPersist*)this, ppv);
			}
			if (riid == __uuidof(IBaseFilter)) {
				return GetInterface((IBaseFilter*)this, ppv);
			}

			hr = m_pEVR ? m_pEVR->QueryInterface(riid, ppv) : E_NOINTERFACE;
			if(m_pEVR && FAILED(hr)) {
				if(riid == __uuidof(IVMRffdshow9)) { // Support ffdshow queueing. We show ffdshow that this is patched Media Player Classic.
					return GetInterface((IVMRffdshow9*)this, ppv);
				}
			}

			return SUCCEEDED(hr) ? hr : __super::NonDelegatingQueryInterface(riid, ppv);
		}

		// IBaseFilter
		STDMETHODIMP EnumPins(__out  IEnumPins **ppEnum);
		STDMETHODIMP FindPin(LPCWSTR Id, __out  IPin **ppPin);
		STDMETHODIMP QueryFilterInfo(__out  FILTER_INFO *pInfo);
		STDMETHODIMP JoinFilterGraph(__in_opt  IFilterGraph *pGraph, __in_opt  LPCWSTR pName);
		STDMETHODIMP QueryVendorInfo(__out  LPWSTR *pVendorInfo);
		STDMETHODIMP Stop();
		STDMETHODIMP Pause();
		STDMETHODIMP Run(REFERENCE_TIME tStart);
		STDMETHODIMP GetState( DWORD dwMilliSecsTimeout, __out  FILTER_STATE *State);
		STDMETHODIMP SetSyncSource(__in_opt  IReferenceClock *pClock);
		STDMETHODIMP GetSyncSource(__deref_out_opt  IReferenceClock **pClock);
		STDMETHODIMP GetClassID(__RPC__out CLSID *pClassID);

		// IVMRffdshow9
		STDMETHODIMP support_ffdshow() {
			queue_ffdshow_support = true;
			return S_OK;
		}

		// IVMRMixerBitmap9
		STDMETHODIMP GetAlphaBitmapParameters(VMR9AlphaBitmap* pBmpParms);
		STDMETHODIMP SetAlphaBitmap(const VMR9AlphaBitmap*  pBmpParms);
		STDMETHODIMP UpdateAlphaBitmapParameters(const VMR9AlphaBitmap* pBmpParms);
	};
}
