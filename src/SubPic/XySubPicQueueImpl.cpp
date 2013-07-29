/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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
#include "XySubPicQueueImpl.h"
#include "XySubPicProvider.h"

//
// CXySubPicQueueNoThread
//

CXySubPicQueueNoThread::CXySubPicQueueNoThread(ISubPicAllocator* pAllocator, HRESULT* phr)
    : CSubPicQueueNoThread(pAllocator, phr)
    , m_llSubId(0)
{
}

CXySubPicQueueNoThread::~CXySubPicQueueNoThread()
{
}

// ISubPicQueue

STDMETHODIMP CXySubPicQueueNoThread::Invalidate(REFERENCE_TIME rtInvalidate)
{
    m_llSubId = 0;
    return __super::Invalidate(rtInvalidate);
}

STDMETHODIMP_(bool) CXySubPicQueueNoThread::LookupSubPic(REFERENCE_TIME rtNow, CComPtr<ISubPic>& ppSubPic)
{
    CComPtr<ISubPic> pSubPic;

    {
        CAutoLock cAutoLock(&m_csLock);

        if (!m_pSubPic) {
            if (FAILED(m_pAllocator->AllocDynamic(&m_pSubPic))) {
                return false;
            }
        }

        pSubPic = m_pSubPic;
    }

    if (pSubPic->GetStart() <= rtNow && rtNow < pSubPic->GetStop()) {
        ppSubPic = pSubPic;
    } else {
        CComPtr<ISubPicProvider> pSubPicProvider;
        GetSubPicProvider(&pSubPicProvider);
        CComQIPtr<IXyCompatProvider> pXySubPicProvider = pSubPicProvider;

        if (pXySubPicProvider) {
            double fps = m_fps;
            REFERENCE_TIME rtTimePerFrame = (REFERENCE_TIME)(10000000.0 / fps);

            REFERENCE_TIME rtStart = rtNow;
            REFERENCE_TIME rtStop = rtNow + rtTimePerFrame;

            HRESULT hr = pXySubPicProvider->RequestFrame(rtStart, rtStop);
            if (SUCCEEDED(hr)) {
                ULONGLONG id;
                hr = pXySubPicProvider->GetID(&id, fps);
                if (SUCCEEDED(hr)) {
                    if (m_pSubPic && m_llSubId == id) { // same subtitle as last time
                        pSubPic->SetStop(rtStop);
                        ppSubPic = pSubPic;
                    } else if (m_pAllocator->IsDynamicWriteOnly()) {
                        CComPtr<ISubPic> pStatic;
                        hr = m_pAllocator->GetStatic(&pStatic);
                        if (SUCCEEDED(hr)) {
                            pStatic->SetInverseAlpha(true);
                            hr = RenderTo(pStatic, rtStart, rtStop, fps, true);
                        }
                        if (SUCCEEDED(hr)) {
                            hr = pStatic->CopyTo(pSubPic);
                        }
                        if (SUCCEEDED(hr)) {
                            ppSubPic = pSubPic;
                            m_llSubId = id;
                        }
                    } else {
                        pSubPic->SetInverseAlpha(true);
                        if (SUCCEEDED(RenderTo(pSubPic, rtStart, rtStop, fps, true))) {
                            ppSubPic = pSubPic;
                            m_llSubId = id;
                        }
                    }
                }
            }

            if (ppSubPic) {
                CAutoLock cAutoLock(&m_csLock);

                m_pSubPic = ppSubPic;
            }
        }
    }

    return !!ppSubPic;
}
