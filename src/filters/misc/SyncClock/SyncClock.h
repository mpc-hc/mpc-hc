#pragma once

#include <atlbase.h>
#include <atlutil.h>
#include "Interfaces.h"

class CSyncClockFilter;

class CSyncClock: public CBaseReferenceClock
{
    friend class CSyncClockFilter;
public:
    CSyncClock(LPUNKNOWN pUnk, HRESULT *phr);

	REFERENCE_TIME GetPrivateTime();
	IUnknown * pUnk() 
    {
		return static_cast<IUnknown*>(static_cast<IReferenceClock*>(this));
	}
	DOUBLE adjustment; // For adjusting speed temporarily
	DOUBLE bias; // For changing speed permanently

private:
    REFERENCE_TIME m_rtPrivateTime;
	LONGLONG m_llPerfFrequency;
	REFERENCE_TIME m_rtPrevTime;
    CCritSec m_csClock;
    IReferenceClock * m_pCurrentRefClock;
    IReferenceClock * m_pPrevRefClock;
	REFERENCE_TIME GetTicks100ns();
};

[uuid("57797fe5-ee9b-4408-98a9-20b134e7e8f0")]
class CSyncClockFilter: public ISyncClock, public CBaseFilter
{
public:
    CSyncClockFilter(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CSyncClockFilter();

	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface( REFIID riid, void ** ppv);

	// ISyncClock
	STDMETHODIMP AdjustClock(DOUBLE adjustment);
	STDMETHODIMP SetBias(DOUBLE bias);
	STDMETHODIMP GetBias(DOUBLE *bias);
	STDMETHODIMP GetStartTime(REFERENCE_TIME *startTime);

    //  CBaseFilter methods
    int CSyncClockFilter::GetPinCount();
    CBasePin *CSyncClockFilter::GetPin(int iPin);

private:
    CSyncClock m_Clock;
    CCritSec m_Lock;
};

