#ifndef f_VD2_KASUMI_RESAMPLE_H
#define f_VD2_KASUMI_RESAMPLE_H

#include <vd2/system/vectors.h>

struct VDPixmap;

class IVDPixmapResampler {
public:
	enum FilterMode {
		kFilterPoint,
		kFilterLinear,
		kFilterCubic,
		kFilterLanczos3,
		kFilterCount
	};

	virtual ~IVDPixmapResampler() {}
	virtual void SetSplineFactor(double A) = 0;
	virtual void SetFilters(FilterMode h, FilterMode v, bool interpolationOnly) = 0;
	virtual bool Init(uint32 dw, uint32 dh, int dstformat, uint32 sw, uint32 sh, int srcformat) = 0;
	virtual bool Init(const vdrect32f& dstrect, uint32 dw, uint32 dh, int dstformat, const vdrect32f& srcrect, uint32 sw, uint32 sh, int srcformat) = 0;
	virtual void Shutdown() = 0;

	virtual void Process(const VDPixmap& dst, const VDPixmap& src) = 0;
};

IVDPixmapResampler *VDCreatePixmapResampler();
bool VDPixmapResample(const VDPixmap& dst, const VDPixmap& src, IVDPixmapResampler::FilterMode filter);

#endif
