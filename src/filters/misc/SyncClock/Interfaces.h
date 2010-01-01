#pragma once

const IID IID_ISyncClock = {0xa62888fb, 0x8e37, 0x44d2, {0x88, 0x50, 0xb3, 0xe3, 0xf2, 0xc1, 0x16, 0x9f}};

MIDL_INTERFACE("A62888FB-8E37-44d2-8850-B3E3F2C1169F")
ISyncClock: public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE AdjustClock(DOUBLE adjustment) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetBias(DOUBLE bias) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetBias(DOUBLE *bias) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetStartTime(REFERENCE_TIME *startTime);
};
