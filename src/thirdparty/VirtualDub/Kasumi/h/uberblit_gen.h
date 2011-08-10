#ifndef f_VD2_KASUMI_UBERBLIT_GEN_H
#define f_VD2_KASUMI_UBERBLIT_GEN_H

#include <vd2/system/vectors.h>
#include "uberblit.h"

class IVDPixmapGenSrc;
struct VDPixmapGenYCbCrBasis;

class VDPixmapUberBlitterDirectCopy : public IVDPixmapBlitter {
public:
	VDPixmapUberBlitterDirectCopy();
	~VDPixmapUberBlitterDirectCopy();

	void Blit(const VDPixmap& dst, const VDPixmap& src);
	void Blit(const VDPixmap& dst, const vdrect32 *rDst, const VDPixmap& src);
};

class VDPixmapUberBlitter : public IVDPixmapBlitter {
public:
	VDPixmapUberBlitter();
	~VDPixmapUberBlitter();

	void Blit(const VDPixmap& dst, const VDPixmap& src);
	void Blit(const VDPixmap& dst, const vdrect32 *rDst, const VDPixmap& src);

protected:
	void Blit(const VDPixmap& dst, const vdrect32 *rDst);
	void Blit3(const VDPixmap& dst, const vdrect32 *rDst);
	void Blit3Split(const VDPixmap& dst, const vdrect32 *rDst);
	void Blit3Separated(const VDPixmap& px, const vdrect32 *rDst);
	void Blit2(const VDPixmap& dst, const vdrect32 *rDst);
	void Blit2Separated(const VDPixmap& px, const vdrect32 *rDst);

	friend class VDPixmapUberBlitterGenerator;

	struct OutputEntry {
		IVDPixmapGen *mpSrc;
		int mSrcIndex;
	} mOutputs[3];

	struct SourceEntry {
		IVDPixmapGenSrc *mpSrc;
		int mSrcIndex;
		int mSrcPlane;
		int mSrcX;
		int mSrcY;
	};

	typedef vdfastvector<IVDPixmapGen *> Generators;
	Generators mGenerators;

	typedef vdfastvector<SourceEntry> Sources;
	Sources mSources;

	bool mbIndependentChromaPlanes;
	bool mbIndependentPlanes;
};

class VDPixmapUberBlitterGenerator {
public:
	VDPixmapUberBlitterGenerator();
	~VDPixmapUberBlitterGenerator();

	void swap(int index);
	void dup();
	void pop();

	void ldsrc(int srcIndex, int srcPlane, int x, int y, uint32 w, uint32 h, uint32 type, uint32 bpr);

	void ldconst(uint8 fill, uint32 bpr, uint32 w, uint32 h, uint32 type);

	void extract_8in16(int offset, uint32 w, uint32 h);
	void extract_8in32(int offset, uint32 w, uint32 h);
	void swap_8in16(uint32 w, uint32 h, uint32 bpr);

	void conv_Pal1_to_8888(int srcIndex);
	void conv_Pal2_to_8888(int srcIndex);
	void conv_Pal4_to_8888(int srcIndex);
	void conv_Pal8_to_8888(int srcIndex);

	void conv_555_to_8888();
	void conv_565_to_8888();
	void conv_888_to_8888();
	void conv_555_to_565();
	void conv_565_to_555();
	void conv_8888_to_X32F();
	void conv_8_to_32F();
	void conv_16F_to_32F();
	void conv_V210_to_32F();

	void conv_8888_to_555();
	void conv_8888_to_565();
	void conv_8888_to_888();
	void conv_32F_to_8();
	void conv_X32F_to_8888();
	void conv_32F_to_16F();
	void conv_32F_to_V210();

	void convd_8888_to_555();
	void convd_8888_to_565();
	void convd_32F_to_8();
	void convd_X32F_to_8888();

	void interleave_B8G8_R8G8();
	void interleave_G8B8_G8R8();
	void interleave_X8R8G8B8();
	void interleave_B8R8();

	void merge_fields(uint32 w, uint32 h, uint32 bpr);
	void split_fields(uint32 bpr);

	void ycbcr601_to_rgb32();
	void ycbcr709_to_rgb32();
	void rgb32_to_ycbcr601();
	void rgb32_to_ycbcr709();

	void ycbcr601_to_rgb32_32f();
	void ycbcr709_to_rgb32_32f();
	void rgb32_to_ycbcr601_32f();
	void rgb32_to_ycbcr709_32f();

	void ycbcr601_to_ycbcr709();
	void ycbcr709_to_ycbcr601();

	void ycbcr_to_rgb32_generic(const VDPixmapGenYCbCrBasis& basis, bool studioRGB);
	void ycbcr_to_rgb32f_generic(const VDPixmapGenYCbCrBasis& basis);
	void rgb32_to_ycbcr_generic(const VDPixmapGenYCbCrBasis& basis, bool studioRGB, uint32 colorSpace);
	void rgb32f_to_ycbcr_generic(const VDPixmapGenYCbCrBasis& basis, uint32 colorSpace);
	void ycbcr_to_ycbcr_generic(const VDPixmapGenYCbCrBasis& basisDst, bool dstLimitedRange, const VDPixmapGenYCbCrBasis& basisSrc, bool srcLimitedRange, uint32 colorSpace);

	void pointh(float xoffset, float xfactor, uint32 w);
	void pointv(float yoffset, float yfactor, uint32 h);
	void linearh(float xoffset, float xfactor, uint32 w, bool interpOnly);
	void linearv(float yoffset, float yfactor, uint32 h, bool interpOnly);
	void linear(float xoffset, float xfactor, uint32 w, float yoffset, float yfactor, uint32 h);
	void cubich(float xoffset, float xfactor, uint32 w, float splineFactor, bool interpOnly);
	void cubicv(float yoffset, float yfactor, uint32 h, float splineFactor, bool interpOnly);
	void cubic(float xoffset, float xfactor, uint32 w, float yoffset, float yfactor, uint32 h, float splineFactor);
	void lanczos3h(float xoffset, float xfactor, uint32 w);
	void lanczos3v(float yoffset, float yfactor, uint32 h);
	void lanczos3(float xoffset, float xfactor, uint32 w, float yoffset, float yfactor, uint32 h);

	IVDPixmapBlitter *create();

protected:
	void MarkDependency(IVDPixmapGen *dst, IVDPixmapGen *src);

	struct StackEntry {
		IVDPixmapGen *mpSrc;
		uint32 mSrcIndex;

		StackEntry() {}
		StackEntry(IVDPixmapGen *src, uint32 index) : mpSrc(src), mSrcIndex(index) {}
	};

	vdfastvector<StackEntry> mStack;

	typedef vdfastvector<IVDPixmapGen *> Generators;
	Generators mGenerators;

	struct Dependency {
		int mDstIdx;
		int mSrcIdx;
	};

	vdfastvector<Dependency> mDependencies;

	typedef VDPixmapUberBlitter::SourceEntry SourceEntry;
	vdfastvector<SourceEntry> mSources;
};

void VDPixmapGenerate(void *dst, ptrdiff_t pitch, sint32 bpr, sint32 height, IVDPixmapGen *gen, int genIndex);
IVDPixmapBlitter *VDCreatePixmapUberBlitterDirectCopy(const VDPixmap& dst, const VDPixmap& src);
IVDPixmapBlitter *VDCreatePixmapUberBlitterDirectCopy(const VDPixmapLayout& dst, const VDPixmapLayout& src);

#endif
