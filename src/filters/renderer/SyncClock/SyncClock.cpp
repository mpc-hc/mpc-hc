#include "stdafx.h"
#include <initguid.h>
#include "SyncClock.h"
#include "../../../DSUtil/DSUtil.h"
#include <moreuuids.h>

#pragma warning(disable:4355)

#ifdef REGISTER_FILTER

const AMOVIESETUP_FILTER sudFilter[] =
{
	{&__uuidof(CSyncClockFilter), L"SyncClock", MERIT_NORMAL, 0, NULL, CLSID_LegacyAmFilterCategory}
};

CFactoryTemplate g_Templates[] =
{
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CSyncClockFilter>, NULL, &sudFilter[0]}
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;

#endif

CSyncClockFilter::CSyncClockFilter(LPUNKNOWN pUnk, HRESULT *phr)
                : CBaseFilter(NAME("SyncClock"), NULL, &m_Lock, CLSID_NULL)
                , m_Clock(static_cast<IBaseFilter*>(this), phr)
{
}

CSyncClockFilter::~CSyncClockFilter()
{
}

STDMETHODIMP CSyncClockFilter::AdjustClock(DOUBLE adjustment)
{
	m_Clock.adjustment = adjustment;
	return S_OK;
}

STDMETHODIMP CSyncClockFilter::SetBias(DOUBLE bias)
{
	m_Clock.bias = bias;
	return S_OK;
}

STDMETHODIMP CSyncClockFilter::GetBias(DOUBLE *bias)
{
	*bias = m_Clock.bias;
	return S_OK;
}

STDMETHODIMP CSyncClockFilter::GetStartTime(REFERENCE_TIME *startTime)
{
	*startTime = m_tStart;
	return S_OK;
}


STDMETHODIMP CSyncClockFilter::NonDelegatingQueryInterface( REFIID riid, void ** ppv )
{
    CheckPointer(ppv, E_POINTER);

    if(riid == IID_IReferenceClock)
    {
        return GetInterface(static_cast<IReferenceClock*>(&m_Clock), ppv);
    }
    else
    if(riid == IID_ISyncClock)
    {
        return GetInterface(static_cast<ISyncClock*>(this), ppv);
    }
	else
	{
        return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}

int CSyncClockFilter::GetPinCount()
{
    return 0;
}

CBasePin *CSyncClockFilter::GetPin(int i)
{
    UNREFERENCED_PARAMETER(i);
    return NULL;
}

// CSyncClock methods
CSyncClock::CSyncClock(LPUNKNOWN pUnk, HRESULT *phr)
            : CBaseReferenceClock(NAME("SyncClock"), pUnk, phr)
            , m_pCurrentRefClock(0), m_pPrevRefClock(0)
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&m_llPerfFrequency);
    m_rtPrivateTime = GetTicks100ns();
    m_rtPrevTime = m_rtPrivateTime;
	adjustment = 1.0;
	bias = 1.0;
}

REFERENCE_TIME CSyncClock::GetPrivateTime()
{
    CAutoLock cObjectLock(this);

	REFERENCE_TIME rtTime = GetTicks100ns();

	REFERENCE_TIME delta = rtTime - m_rtPrevTime;
	// We ignore that rtTime may wrap around. Not gonna happen too often
	m_rtPrevTime = rtTime;

	delta = (REFERENCE_TIME)((DOUBLE)delta * adjustment * bias);
	m_rtPrivateTime = m_rtPrivateTime + delta;
    return m_rtPrivateTime;
}

REFERENCE_TIME CSyncClock::GetTicks100ns()
{
	LONGLONG i64Ticks100ns;
	if (m_llPerfFrequency != 0)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&i64Ticks100ns);
		i64Ticks100ns = LONGLONG((double(i64Ticks100ns) * 10000000) / double(m_llPerfFrequency) + 0.5);
		return (REFERENCE_TIME)i64Ticks100ns;
	}
	return 0;
}
