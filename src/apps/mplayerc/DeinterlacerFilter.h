#pragma once

[uuid("96F3E0BE-1BA4-4E79-973D-191FE425C86B")]
class CDeinterlacerFilter : public CTransformFilter
{
protected:
    HRESULT CDeinterlacerFilter::CheckConnect(PIN_DIRECTION dir, IPin* pPin);
	HRESULT CheckInputType(const CMediaType* mtIn);
    HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);
	HRESULT Transform(IMediaSample* pIn, IMediaSample* pOut);
	HRESULT DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties);
	HRESULT GetMediaType(int iPosition, CMediaType* pmt);

public:
	CDeinterlacerFilter(LPUNKNOWN punk, HRESULT* phr);
};
