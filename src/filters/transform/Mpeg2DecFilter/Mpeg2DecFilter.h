/* 
 *  Copyright (C) 2003-2006 Gabest
 *  http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

#include <atlcoll.h>
#include <Videoacc.h>
#include "../../../DeCSS/DeCSSInputPin.h"
#include "../BaseVideoFilter/BaseVideoFilter.h"
#include "IMpeg2DecFilter.h"
#include "Mpeg2DecSettingsWnd.h"

class CSubpicInputPin;
class CClosedCaptionOutputPin;
class CMpeg2Dec;

class __declspec(uuid("39F498AF-1A09-4275-B193-673B0BA3D478"))
CMpeg2DecFilter 
	: public CBaseVideoFilter
	, public IMpeg2DecFilter
	, public ISpecifyPropertyPages2
{
	CSubpicInputPin* m_pSubpicInput;
	CClosedCaptionOutputPin* m_pClosedCaptionOutput;

	CAutoPtr<CMpeg2Dec> m_dec;

	REFERENCE_TIME m_AvgTimePerFrame;
	bool m_fWaitForKeyFrame;
	bool m_fInitializedBuffer;
	CSize m_par;

	struct framebuf 
	{
		int w, h, pitch;
		BYTE* buf_base;
		BYTE* buf[6];
		REFERENCE_TIME rtStart, rtStop;
		DWORD flags;
		ditype di;
        framebuf()
		{
			w = h = pitch = 0;
			buf_base = NULL;
			memset(&buf, 0, sizeof(buf));
			rtStart = rtStop = 0;
			flags = 0;
		}
        ~framebuf() {Free();}
		void Alloc(int w, int h, int pitch)
		{
			this->w = w; this->h = h; this->pitch = pitch;
			int size = pitch*h;
			buf_base = (BYTE*)_aligned_malloc(size*3+6*32, 32);
			BYTE* p = buf_base;
			buf[0] = p; p += (size + 31) & ~31;
			buf[3] = p; p += (size + 31) & ~31;
			buf[1] = p; p += (size/4 + 31) & ~31;
			buf[4] = p; p += (size/4 + 31) & ~31;
			buf[2] = p; p += (size/4 + 31) & ~31;
			buf[5] = p; p += (size/4 + 31) & ~31;
		}
		void Free()
		{
			if(buf_base) _aligned_free(buf_base); 
			buf_base = NULL;
		}
	} m_fb;

	bool m_fFilm;
	void SetDeinterlaceMethod();
	void SetTypeSpecificFlags(IMediaSample* pMS);

	AM_SimpleRateChange m_rate;

protected:
	void InputTypeChanged();
	HRESULT Transform(IMediaSample* pIn);
	bool IsVideoInterlaced();
	void UpdateAspectRatio();

public:
	CMpeg2DecFilter(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CMpeg2DecFilter();

	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	HRESULT DeliverFast();
	HRESULT DeliverNormal();
	HRESULT Deliver(bool fRepeatLast);

	int GetPinCount();
	CBasePin* GetPin(int n);

    HRESULT EndOfStream();
	HRESULT BeginFlush();
	HRESULT EndFlush();
    HRESULT NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

	HRESULT CheckConnect(PIN_DIRECTION dir, IPin* pPin);
    HRESULT CheckInputType(const CMediaType* mtIn);
	HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);

	HRESULT StartStreaming();
	HRESULT StopStreaming();

	bool m_fDropFrames;
	HRESULT AlterQuality(Quality q);

protected:
	CCritSec m_csProps;
	ditype m_ditype;
	float m_bright, m_cont, m_hue, m_sat;
	BYTE m_YTbl[256], m_UTbl[256*256], m_VTbl[256*256];
	bool m_fForcedSubs;
	bool m_fPlanarYUV;
	bool m_fInterlaced;
	bool m_bReadARFromStream;

	static void CalcBrCont(BYTE* YTbl, float bright, float cont);
	static void CalcHueSat(BYTE* UTbl, BYTE* VTbl, float hue, float sat);
	void ApplyBrContHueSat(BYTE* srcy, BYTE* srcu, BYTE* srcv, int w, int h, int pitch);
	
public:

	// ISpecifyPropertyPages2

	STDMETHODIMP GetPages(CAUUID* pPages);
	STDMETHODIMP CreatePage(const GUID& guid, IPropertyPage** ppPage);

	// IMpeg2DecFilter

	STDMETHODIMP SetDeinterlaceMethod(ditype di);
	STDMETHODIMP_(ditype) GetDeinterlaceMethod();

	STDMETHODIMP SetBrightness(float bright);
	STDMETHODIMP SetContrast(float cont);
	STDMETHODIMP SetHue(float hue);
	STDMETHODIMP SetSaturation(float sat);
	STDMETHODIMP_(float) GetBrightness();
	STDMETHODIMP_(float) GetContrast();
	STDMETHODIMP_(float) GetHue();
	STDMETHODIMP_(float) GetSaturation();

	STDMETHODIMP EnableForcedSubtitles(bool fEnable);
	STDMETHODIMP_(bool) IsForcedSubtitlesEnabled();

	STDMETHODIMP EnablePlanarYUV(bool fEnable);
	STDMETHODIMP_(bool) IsPlanarYUVEnabled();

	// IMpeg2DecFilter2

	STDMETHODIMP EnableInterlaced(bool fEnable);
	STDMETHODIMP_(bool) IsInterlacedEnabled();

	STDMETHODIMP EnableReadARFromStream(bool fEnable);
	STDMETHODIMP_(bool) IsReadARFromStreamEnabled();
};

class CMpeg2DecInputPin : public CDeCSSInputPin
{
	LONG m_CorrectTS;

public:
    CMpeg2DecInputPin(CTransformFilter* pFilter, HRESULT* phr, LPWSTR pName);

	CCritSec m_csRateLock;
	AM_SimpleRateChange m_ratechange;

	// IKsPropertySet
    STDMETHODIMP Set(REFGUID PropSet, ULONG Id, LPVOID InstanceData, ULONG InstanceLength, LPVOID PropertyData, ULONG DataLength);
    STDMETHODIMP Get(REFGUID PropSet, ULONG Id, LPVOID InstanceData, ULONG InstanceLength, LPVOID PropertyData, ULONG DataLength, ULONG* pBytesReturned);
    STDMETHODIMP QuerySupported(REFGUID PropSet, ULONG Id, ULONG* pTypeSupport);
};

class CMpeg2DecOutputPin : public CBaseVideoOutputPin
{
public:
	CMpeg2DecOutputPin(CBaseVideoFilter* pFilter, HRESULT* phr, LPWSTR pName);

	CAutoPtr<COutputQueue> m_pOutputQueue;

	HRESULT Active();
    HRESULT Inactive();

	HRESULT Deliver(IMediaSample* pMediaSample);
    HRESULT DeliverEndOfStream();
    HRESULT DeliverBeginFlush();
	HRESULT DeliverEndFlush();
    HRESULT DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);
};

class CSubpicInputPin : public CMpeg2DecInputPin
{
	CCritSec m_csReceive;

	AM_PROPERTY_COMPOSIT_ON m_spon;
	AM_DVD_YUV m_sppal[16];
	bool m_fsppal;
	CAutoPtr<AM_PROPERTY_SPHLI> m_sphli; // temp

	class spu : public CAtlArray<BYTE>
	{
	public:
		bool m_fForced;
		REFERENCE_TIME m_rtStart, m_rtStop; 
		DWORD m_offset[2];
		AM_PROPERTY_SPHLI m_sphli; // parsed
		CAutoPtr<AM_PROPERTY_SPHLI> m_psphli; // for the menu (optional)
		spu() {memset(&m_sphli, 0, sizeof(m_sphli)); m_fForced = false; m_rtStart = m_rtStop = 0;}
		virtual ~spu() {}
		virtual bool Parse() = 0;
		virtual void Render(REFERENCE_TIME rt, BYTE** p, int w, int h, AM_DVD_YUV* sppal, bool fsppal) = 0;
	};

	class dvdspu : public spu
	{
	public:
		struct offset_t {REFERENCE_TIME rt; AM_PROPERTY_SPHLI sphli;};
		CAtlList<offset_t> m_offsets;
		bool Parse();
		void Render(REFERENCE_TIME rt, BYTE** p, int w, int h, AM_DVD_YUV* sppal, bool fsppal);
	};

	class cvdspu : public spu
	{
	public:
		AM_DVD_YUV m_sppal[2][4];
		cvdspu() {memset(m_sppal, 0, sizeof(m_sppal));}
		bool Parse();
		void Render(REFERENCE_TIME rt, BYTE** p, int w, int h, AM_DVD_YUV* sppal, bool fsppal);
	};

	class svcdspu : public spu
	{
	public:
		AM_DVD_YUV m_sppal[4];
		svcdspu() {memset(m_sppal, 0, sizeof(m_sppal));}
		bool Parse();
		void Render(REFERENCE_TIME rt, BYTE** p, int w, int h, AM_DVD_YUV* sppal, bool fsppal);
	};

	CAutoPtrList<spu> m_sps;

protected:
	HRESULT Transform(IMediaSample* pSample);

public:
	CSubpicInputPin(CTransformFilter* pFilter, HRESULT* phr);

	bool HasAnythingToRender(REFERENCE_TIME rt);
	void RenderSubpics(REFERENCE_TIME rt, BYTE** p, int w, int h);

    HRESULT CheckMediaType(const CMediaType* mtIn);
	HRESULT SetMediaType(const CMediaType* mtIn);

	// we shouldn't pass these to the filter from this pin
	STDMETHODIMP EndOfStream() {return S_OK;}
    STDMETHODIMP BeginFlush() {return S_OK;}
    STDMETHODIMP EndFlush();
    STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate) {return S_OK;}

	// IKsPropertySet
    STDMETHODIMP Set(REFGUID PropSet, ULONG Id, LPVOID InstanceData, ULONG InstanceLength, LPVOID PropertyData, ULONG DataLength);
    STDMETHODIMP QuerySupported(REFGUID PropSet, ULONG Id, ULONG* pTypeSupport);
};

class CClosedCaptionOutputPin : public CBaseOutputPin
{
public:
	CClosedCaptionOutputPin(CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr);

    HRESULT CheckMediaType(const CMediaType* mtOut);
	HRESULT GetMediaType(int iPosition, CMediaType* pmt);
    HRESULT DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties);

	CMediaType& CurrentMediaType() {return m_mt;}

	HRESULT Deliver(const void* ptr, int len);
};
