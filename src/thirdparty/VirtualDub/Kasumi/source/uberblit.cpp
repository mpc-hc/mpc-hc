#include <vd2/Kasumi/pixmap.h>
#include <vd2/Kasumi/pixmaputils.h>
#include "uberblit.h"
#include "uberblit_gen.h"

uint32 VDPixmapGetFormatTokenFromFormat(int format) {
	using namespace nsVDPixmap;
	switch(format) {
	case kPixFormat_Pal1:			return kVDPixType_1 | kVDPixSamp_444 | kVDPixSpace_Pal;
	case kPixFormat_Pal2:			return kVDPixType_2 | kVDPixSamp_444 | kVDPixSpace_Pal;
	case kPixFormat_Pal4:			return kVDPixType_4 | kVDPixSamp_444 | kVDPixSpace_Pal;
	case kPixFormat_Pal8:			return kVDPixType_8 | kVDPixSamp_444 | kVDPixSpace_Pal;
	case kPixFormat_XRGB1555:		return kVDPixType_1555_LE | kVDPixSamp_444 | kVDPixSpace_BGR;
	case kPixFormat_RGB565:			return kVDPixType_565_LE | kVDPixSamp_444 | kVDPixSpace_BGR;
	case kPixFormat_RGB888:			return kVDPixType_888 | kVDPixSamp_444 | kVDPixSpace_BGR;
	case kPixFormat_XRGB8888:		return kVDPixType_8888 | kVDPixSamp_444 | kVDPixSpace_BGR;
	case kPixFormat_Y8:				return kVDPixType_8 | kVDPixSamp_444 | kVDPixSpace_Y_601;
	case kPixFormat_YUV422_UYVY:	return kVDPixType_B8G8_R8G8 | kVDPixSamp_422 | kVDPixSpace_YCC_601;
	case kPixFormat_YUV422_YUYV:	return kVDPixType_G8B8_G8R8 | kVDPixSamp_422 | kVDPixSpace_YCC_601;
	case kPixFormat_YUV444_XVYU:	return kVDPixType_8888 | kVDPixSamp_444 | kVDPixSpace_YCC_601;
	case kPixFormat_YUV444_Planar:	return kVDPixType_8_8_8 | kVDPixSamp_444 | kVDPixSpace_YCC_601;
	case kPixFormat_YUV422_Planar:	return kVDPixType_8_8_8 | kVDPixSamp_422 | kVDPixSpace_YCC_601;
	case kPixFormat_YUV422_Planar_16F:	return kVDPixType_16F_16F_16F_LE | kVDPixSamp_422 | kVDPixSpace_YCC_601;
	case kPixFormat_YUV420_Planar:	return kVDPixType_8_8_8 | kVDPixSamp_420_MPEG2 | kVDPixSpace_YCC_601;
	case kPixFormat_YUV411_Planar:	return kVDPixType_8_8_8 | kVDPixSamp_411 | kVDPixSpace_YCC_601;
	case kPixFormat_YUV410_Planar:	return kVDPixType_8_8_8 | kVDPixSamp_410 | kVDPixSpace_YCC_601;
	case kPixFormat_YUV422_Planar_Centered:	return kVDPixType_8_8_8 | kVDPixSamp_422_JPEG | kVDPixSpace_YCC_601;
	case kPixFormat_YUV420_Planar_Centered:	return kVDPixType_8_8_8 | kVDPixSamp_420_MPEG1 | kVDPixSpace_YCC_601;
	case kPixFormat_YUV422_V210:	return kVDPixType_V210 | kVDPixSamp_422 | kVDPixSpace_YCC_601;
	case kPixFormat_YUV422_UYVY_709:	return kVDPixType_B8G8_R8G8 | kVDPixSamp_422 | kVDPixSpace_YCC_709;
	case kPixFormat_YUV420_NV12:	return kVDPixType_8_B8R8 | kVDPixSamp_420_MPEG2 | kVDPixSpace_YCC_601;
	default:
		VDASSERT(false);
		return 0;
	}
}

const VDPixmapSamplingInfo& VDPixmapGetSamplingInfo(uint32 samplingToken) {
	static const VDPixmapSamplingInfo kPixmapSamplingInfo[]={
		/* Null			*/ {  0,  0,  0,  0,  0 },
		/* 444			*/ {  0,  0,  0,  0,  0 },
		/* 422			*/ { -4,  0,  0,  1,  0 },
		/* 422_JPEG		*/ {  0,  0,  0,  1,  0 },
		/* 420_MPEG2	*/ { -4,  0,  0,  1,  1 },
		/* 420_MPEG2INT	*/ { -4,  0,  0,  1,  1 },
		/* 420_MPEG1	*/ {  0,  0,  0,  1,  1 },
		/* 420_DVPAL	*/ { -4,  0,  0,  1,  1 },
		/* 411			*/ { -6,  0,  0,  2,  0 },
		/* 410			*/ { -6,  0,  0,  2,  2 }
	};

	uint32 index = (samplingToken & kVDPixSamp_Mask) >> kVDPixSamp_Bits;

	return index >= sizeof(kPixmapSamplingInfo)/sizeof(kPixmapSamplingInfo[0]) ? kPixmapSamplingInfo[0] : kPixmapSamplingInfo[index];
}

namespace {
	uint32 BlitterConvertSampling(VDPixmapUberBlitterGenerator& gen, uint32 srcToken, uint32 dstSamplingToken, sint32 w, sint32 h) {
		// if the source type is 16F, we have to convert to 32F
		if ((srcToken & kVDPixType_Mask) == kVDPixType_16F_16F_16F_LE) {
			// 0 1 2
			gen.conv_16F_to_32F();
			gen.swap(1);
			// 1 0 2
			gen.conv_16F_to_32F();
			gen.swap(2);
			// 2 0 1
			gen.conv_16F_to_32F();
			gen.swap(2);
			gen.swap(1);
			srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_32F_32F_32F_LE;
		}

		// look up sampling info
		const VDPixmapSamplingInfo& srcInfo = VDPixmapGetSamplingInfo(srcToken);
		const VDPixmapSamplingInfo& dstInfo = VDPixmapGetSamplingInfo(dstSamplingToken);

		// convert destination chroma origin to luma space
		int c_x = ((8 + dstInfo.mCXOffset16) << dstInfo.mCXBits) - 8;
		int cr_y = ((8 + dstInfo.mCrYOffset16) << dstInfo.mCYBits) - 8;
		int cb_y = ((8 + dstInfo.mCbYOffset16) << dstInfo.mCYBits) - 8;

		// convert luma chroma location to source chroma space
		c_x = ((8 + c_x) >> srcInfo.mCXBits) - 8 - srcInfo.mCXOffset16;
		cr_y = ((8 + cr_y) >> srcInfo.mCYBits) - 8 - srcInfo.mCrYOffset16;
		cb_y = ((8 + cb_y) >> srcInfo.mCYBits) - 8 - srcInfo.mCbYOffset16;

		float cxo = c_x / 16.0f + 0.5f;
		float cxf = ((16 << dstInfo.mCXBits) >> srcInfo.mCXBits) / 16.0f;
		float cyf = ((16 << dstInfo.mCYBits) >> srcInfo.mCYBits) / 16.0f;
		sint32 cw = -(-w >> dstInfo.mCXBits);
		sint32 ch = -(-h >> dstInfo.mCYBits);

		gen.swap(2);
		gen.linear(cxo, cxf, cw, cb_y / 16.0f + 0.5f, cyf, ch);
		gen.swap(2);
		gen.linear(cxo, cxf, cw, cr_y / 16.0f + 0.5f, cyf, ch);

		return (srcToken & ~kVDPixSamp_Mask) | (dstSamplingToken & kVDPixSamp_Mask);
	}

	uint32 BlitterConvertType(VDPixmapUberBlitterGenerator& gen, uint32 srcToken, uint32 dstToken, sint32 w, sint32 h) {
		uint32 dstType = dstToken & kVDPixType_Mask;

		while((srcToken ^ dstToken) & kVDPixType_Mask) {
			uint32 srcType = srcToken & kVDPixType_Mask;
			uint32 targetType = dstType;

	type_reconvert:
			switch(targetType) {
				case kVDPixType_1555_LE:
					switch(srcType) {
						case kVDPixType_565_LE:
							gen.conv_565_to_555();
							srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_1555_LE;
							break;

						case kVDPixType_8888:
							gen.conv_8888_to_555();
							srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_1555_LE;
							break;
						case kVDPixType_B8G8_R8G8:
						case kVDPixType_G8B8_G8R8:
							targetType = kVDPixType_8_8_8;
							goto type_reconvert;
						default:
							targetType = kVDPixType_8888;
							goto type_reconvert;
					}
					break;

				case kVDPixType_565_LE:
					switch(srcType) {
						case kVDPixType_1555_LE:
							gen.conv_555_to_565();
							srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_565_LE;
							break;
						case kVDPixType_8888:
							gen.conv_8888_to_565();
							srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_565_LE;
							break;
						case kVDPixType_B8G8_R8G8:
						case kVDPixType_G8B8_G8R8:
							targetType = kVDPixType_8_8_8;
							goto type_reconvert;
						default:
							targetType = kVDPixType_8888;
							goto type_reconvert;
					}
					break;

				case kVDPixType_888:
					switch(srcType) {
						case kVDPixType_8888:
							gen.conv_8888_to_888();
							srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_888;
							break;
						default:
							targetType = kVDPixType_8888;
							goto type_reconvert;
					}
					break;

				case kVDPixType_8888:
					switch(srcType) {
						case kVDPixType_1555_LE:
							gen.conv_555_to_8888();
							srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_8888;
							break;
						case kVDPixType_565_LE:
							gen.conv_565_to_8888();
							srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_8888;
							break;
						case kVDPixType_888:
							gen.conv_888_to_8888();
							srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_8888;
							break;
						case kVDPixType_32Fx4_LE:
							gen.conv_X32F_to_8888();
							srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_8888;
							break;
						case kVDPixType_8_8_8:
							if ((srcToken & kVDPixSamp_Mask) != kVDPixSamp_444)
								srcToken = BlitterConvertSampling(gen, srcToken, kVDPixSamp_444, w, h);
							gen.interleave_X8R8G8B8();
							srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_8888;
							break;
						default:
							VDASSERT(false);
							break;
					}
					break;

				case kVDPixType_8:
					switch(srcType) {
						case kVDPixType_8_8_8:
							gen.pop();
							gen.swap(1);
							gen.pop();
							srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_8;
							break;

						case kVDPixType_16F_LE:
							targetType = kVDPixType_32F_LE;
							goto type_reconvert;

						case kVDPixType_32F_LE:
							gen.conv_32F_to_8();
							srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_8;
							break;

						default:
							targetType = kVDPixType_8_8_8;
							goto type_reconvert;
					}
					break;

				case kVDPixType_8_8_8:
					switch(srcType) {
						case kVDPixType_B8G8_R8G8:
							gen.dup();
							gen.dup();
							gen.extract_8in32(2, (w + 1) >> 1, h);
							gen.swap(2);
							gen.extract_8in16(1, w, h);
							gen.swap(1);
							gen.extract_8in32(0, (w + 1) >> 1, h);
							srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSamp_Mask)) | kVDPixType_8_8_8 | kVDPixSamp_422;
							break;
						case kVDPixType_G8B8_G8R8:
							gen.dup();
							gen.dup();
							gen.extract_8in32(3, (w + 1) >> 1, h);
							gen.swap(2);
							gen.extract_8in16(0, w, h);
							gen.swap(1);
							gen.extract_8in32(1, (w + 1) >> 1, h);
							srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSamp_Mask)) | kVDPixType_8_8_8 | kVDPixSamp_422;
							break;
						case kVDPixType_16F_16F_16F_LE:
						case kVDPixType_V210:
							targetType = kVDPixType_32F_32F_32F_LE;
							goto type_reconvert;
						case kVDPixType_32F_32F_32F_LE:
							// 0 1 2
							gen.conv_32F_to_8();
							gen.swap(1);
							// 1 0 2
							gen.conv_32F_to_8();
							gen.swap(2);
							// 2 0 1
							gen.conv_32F_to_8();
							gen.swap(2);
							gen.swap(1);
							srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_8_8_8;
							break;
						case kVDPixType_8_B8R8:
							{
								const VDPixmapSamplingInfo& sampInfo = VDPixmapGetSamplingInfo(srcToken);
								int cw = -(-w >> sampInfo.mCXBits);
								int ch = -(-h >> sampInfo.mCYBits);

								gen.dup();
								gen.extract_8in16(1, cw, ch);
								gen.swap(2);
								gen.swap(1);
								gen.extract_8in16(0, cw, ch);
								srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_8_8_8;
							}
							break;
						default:
							VDASSERT(false);
							break;
					}
					break;

				case kVDPixType_B8G8_R8G8:
					switch(srcType) {
					case kVDPixType_8_8_8:
						if ((srcToken ^ dstToken) & kVDPixSamp_Mask)
							srcToken = BlitterConvertSampling(gen, srcToken, dstToken, w, h);

						gen.interleave_B8G8_R8G8();
						srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSamp_Mask)) | kVDPixType_B8G8_R8G8;
						break;
					case kVDPixType_G8B8_G8R8:
						gen.swap_8in16(w, h, w*2);
						srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSamp_Mask)) | kVDPixType_B8G8_R8G8;
						break;
					default:
						targetType = kVDPixType_8_8_8;
						goto type_reconvert;
					}
					break;

				case kVDPixType_G8B8_G8R8:
					switch(srcType) {
					case kVDPixType_8_8_8:
						if ((srcToken ^ dstToken) & kVDPixSamp_Mask)
							srcToken = BlitterConvertSampling(gen, srcToken, dstToken, w, h);

						gen.interleave_G8B8_G8R8();
						srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSamp_Mask)) | kVDPixType_G8B8_G8R8;
						break;
					case kVDPixType_B8G8_R8G8:
						gen.swap_8in16(w, h, w*2);
						srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSamp_Mask)) | kVDPixType_G8B8_G8R8;
						break;
					default:
						targetType = kVDPixType_8_8_8;
						goto type_reconvert;
					}
					break;

				case kVDPixType_16F_16F_16F_LE:
					switch(srcType) {
						case kVDPixType_32F_32F_32F_LE:
							// 0 1 2
							gen.conv_32F_to_16F();
							gen.swap(1);
							// 1 0 2
							gen.conv_32F_to_16F();
							gen.swap(2);
							// 2 0 1
							gen.conv_32F_to_16F();
							gen.swap(2);
							gen.swap(1);
							srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_16F_16F_16F_LE;
							break;

						default:
							targetType = kVDPixType_32F_32F_32F_LE;
							goto type_reconvert;
					}
					break;

				case kVDPixType_32F_32F_32F_LE:
					switch(srcType) {
						case kVDPixType_8_8_8:
							// 0 1 2
							gen.conv_8_to_32F();
							gen.swap(1);
							// 1 0 2
							gen.conv_8_to_32F();
							gen.swap(2);
							// 2 0 1
							gen.conv_8_to_32F();
							gen.swap(2);
							gen.swap(1);
							srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_32F_32F_32F_LE;
							break;

						case kVDPixType_16F_16F_16F_LE:
							// 0 1 2
							gen.conv_16F_to_32F();
							gen.swap(1);
							// 1 0 2
							gen.conv_16F_to_32F();
							gen.swap(2);
							// 2 0 1
							gen.conv_16F_to_32F();
							gen.swap(2);
							gen.swap(1);
							srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_32F_32F_32F_LE;
							break;

						case kVDPixType_B8G8_R8G8:
						case kVDPixType_G8B8_G8R8:
						case kVDPixType_8_B8R8:
							targetType = kVDPixType_8_8_8;
							goto type_reconvert;

						case kVDPixType_V210:
							gen.conv_V210_to_32F();
							srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_32F_32F_32F_LE;
							break;

						default:
							VDASSERT(false);
					}
					break;

				case kVDPixType_V210:
					switch(srcType) {
						case kVDPixType_32F_32F_32F_LE:
							if ((srcToken & kVDPixSamp_Mask) != kVDPixSamp_422)
								srcToken = BlitterConvertSampling(gen, srcToken, kVDPixSamp_422, w, h);

							gen.conv_32F_to_V210();
							srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_V210;
							break;

						case kVDPixType_16F_16F_16F_LE:
							targetType = kVDPixType_32F_32F_32F_LE;
							goto type_reconvert;

						case kVDPixType_8_8_8:
							if ((srcToken & kVDPixSamp_Mask) != kVDPixSamp_422)
								srcToken = BlitterConvertSampling(gen, srcToken, kVDPixSamp_422, w, h);

							targetType = kVDPixType_32F_32F_32F_LE;
							goto type_reconvert;

						case kVDPixType_B8G8_R8G8:
						case kVDPixType_G8B8_G8R8:
						case kVDPixType_8_B8R8:
							targetType = kVDPixType_8_8_8;
							goto type_reconvert;

						default:
							VDASSERT(false);
					}
					break;

				case kVDPixType_32F_LE:
					switch(srcType) {
						case kVDPixType_8:
							gen.conv_8_to_32F();
							srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_32F_LE;
							break;
						case kVDPixType_16F_LE:
							gen.conv_16F_to_32F();
							srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_32F_LE;
							break;
						default:
							VDASSERT(false);
					}
					break;

				case kVDPixType_8_B8R8:
					switch(srcType) {
						case kVDPixType_8_8_8:
							gen.swap(1);
							gen.swap(2);
							gen.interleave_B8R8();
							srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_8_B8R8;
							break;
						default:
							VDASSERT(false);
							break;
					}
					break;

				default:
					VDASSERT(false);
					break;
			}
		}

		return srcToken;
	}
}

IVDPixmapBlitter *VDPixmapCreateBlitter(const VDPixmap& dst, const VDPixmap& src) {
	const VDPixmapLayout& dstlayout = VDPixmapToLayoutFromBase(dst, dst.data);
	const VDPixmapLayout& srclayout = VDPixmapToLayoutFromBase(src, src.data);

	return VDPixmapCreateBlitter(dstlayout, srclayout);
}

IVDPixmapBlitter *VDPixmapCreateBlitter(const VDPixmapLayout& dst, const VDPixmapLayout& src) {
	if (src.format == dst.format) {
		return VDCreatePixmapUberBlitterDirectCopy(dst, src);
	}

	uint32 srcToken = VDPixmapGetFormatTokenFromFormat(src.format);
	uint32 dstToken = VDPixmapGetFormatTokenFromFormat(dst.format);

	VDPixmapUberBlitterGenerator gen;

	// load source channels
	int w = src.w;
	int h = src.h;

	switch(srcToken & kVDPixType_Mask) {
	case kVDPixType_1:
		gen.ldsrc(0, 0, 0, 0, w, h, srcToken, (w + 7) >> 3);
		break;

	case kVDPixType_2:
		gen.ldsrc(0, 0, 0, 0, w, h, srcToken, (w + 3) >> 2);
		break;

	case kVDPixType_4:
		gen.ldsrc(0, 0, 0, 0, w, h, srcToken, (w + 1) >> 1);
		break;

	case kVDPixType_8:
		gen.ldsrc(0, 0, 0, 0, w, h, srcToken, w);
		break;

	case kVDPixType_555_LE:
	case kVDPixType_565_LE:
	case kVDPixType_1555_LE:
		gen.ldsrc(0, 0, 0, 0, w, h, srcToken, w*2);
		break;

	case kVDPixType_888:
		gen.ldsrc(0, 0, 0, 0, w, h, srcToken, w*3);
		break;

	case kVDPixType_8888:
	case kVDPixType_32F_LE:
		gen.ldsrc(0, 0, 0, 0, w, h, srcToken, w*4);
		break;

	case kVDPixType_32Fx4_LE:
		gen.ldsrc(0, 0, 0, 0, w, h, srcToken, w*16);
		break;

	case kVDPixType_B8G8_R8G8:
	case kVDPixType_G8B8_G8R8:
		gen.ldsrc(0, 0, 0, 0, w, h, srcToken, ((w + 1) & ~1)*2);
		break;

	case kVDPixType_8_8_8:
		{
			uint32 ytoken = (srcToken & ~kVDPixType_Mask) | kVDPixType_8;
			uint32 cbtoken = (srcToken & ~kVDPixType_Mask) | kVDPixType_8;
			uint32 crtoken = (srcToken & ~kVDPixType_Mask) | kVDPixType_8;

			const VDPixmapSamplingInfo& sampInfo = VDPixmapGetSamplingInfo(srcToken);

			int cxbits = sampInfo.mCXBits;
			int cybits = sampInfo.mCYBits;
			int w2 = -(-w >> cxbits);
			int h2 = -(-h >> cybits);
			gen.ldsrc(0, 2, 0, 0, w2, h2, cbtoken, w2);
			gen.ldsrc(0, 0, 0, 0, w, h, srcToken, w);
			gen.ldsrc(0, 1, 0, 0, w2, h2, crtoken, w2);
		}
		break;

	case kVDPixType_16F_16F_16F_LE:
		{
			uint32 ytoken = (srcToken & ~kVDPixType_Mask) | kVDPixType_16F_LE;
			uint32 cbtoken = (srcToken & ~kVDPixType_Mask) | kVDPixType_16F_LE;
			uint32 crtoken = (srcToken & ~kVDPixType_Mask) | kVDPixType_16F_LE;

			const VDPixmapSamplingInfo& sampInfo = VDPixmapGetSamplingInfo(srcToken);

			int cxbits = sampInfo.mCXBits;
			int cybits = sampInfo.mCYBits;
			int w2 = -(-w >> cxbits);
			int h2 = -(-h >> cybits);
			gen.ldsrc(0, 2, 0, 0, w2, h2, cbtoken, w2 * 2);
			gen.ldsrc(0, 0, 0, 0, w, h, srcToken, w*2);
			gen.ldsrc(0, 1, 0, 0, w2, h2, crtoken, w2 * 2);
		}
		break;

	case kVDPixType_32F_32F_32F_LE:
		{
			uint32 ytoken = (srcToken & ~kVDPixType_Mask) | kVDPixType_32F_LE;
			uint32 cbtoken = (srcToken & ~kVDPixType_Mask) | kVDPixType_32F_LE;
			uint32 crtoken = (srcToken & ~kVDPixType_Mask) | kVDPixType_32F_LE;

			const VDPixmapSamplingInfo& sampInfo = VDPixmapGetSamplingInfo(srcToken);

			int cxbits = sampInfo.mCXBits;
			int cybits = sampInfo.mCYBits;
			int w2 = -(-w >> cxbits);
			int h2 = -(-h >> cybits);
			gen.ldsrc(0, 2, 0, 0, w2, h2, cbtoken, w2 * 4);
			gen.ldsrc(0, 0, 0, 0, w, h, srcToken, w*4);
			gen.ldsrc(0, 1, 0, 0, w2, h2, crtoken, w2 * 4);
		}
		break;

	case kVDPixType_V210:
		gen.ldsrc(0, 0, 0, 0, w, h, srcToken, ((w + 5) / 6) * 4);
		break;

	case kVDPixType_8_B8R8:
		{
			uint32 ytoken = (srcToken & ~kVDPixType_Mask) | kVDPixType_8;
			uint32 ctoken = (srcToken & ~kVDPixType_Mask) | kVDPixType_B8R8;

			const VDPixmapSamplingInfo& sampInfo = VDPixmapGetSamplingInfo(srcToken);

			int cxbits = sampInfo.mCXBits;
			int cybits = sampInfo.mCYBits;
			int w2 = -(-w >> cxbits);
			int h2 = -(-h >> cybits);
			gen.ldsrc(0, 0, 0, 0, w, h, srcToken, w);
			gen.ldsrc(0, 1, 0, 0, w2, h2, ctoken, w2*2);
		}
		break;

	default:
		VDASSERT(false);
	}

	// check if we need a color space change
	if ((srcToken ^ dstToken) & kVDPixSpace_Mask) {
		// first, if we're dealing with an interleaved format, deinterleave it
		switch(srcToken & kVDPixType_Mask) {
		case kVDPixType_B8G8_R8G8:
			gen.dup();
			gen.dup();
			gen.extract_8in32(2, (w + 1) >> 1, h);
			gen.swap(2);
			gen.extract_8in16(1, w, h);
			gen.swap(1);
			gen.extract_8in32(0, (w + 1) >> 1, h);
			srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_8_8_8;
			break;

		case kVDPixType_G8B8_G8R8:
			gen.dup();
			gen.dup();
			gen.extract_8in32(3, (w + 1) >> 1, h);
			gen.swap(2);
			gen.extract_8in16(0, w, h);
			gen.swap(1);
			gen.extract_8in32(1, (w + 1) >> 1, h);
			srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_8_8_8;
			break;

		case kVDPixType_8_B8R8:
			gen.dup();
			gen.extract_8in16(1, (w + 1) >> 1, (h + 1) >> 1);
			gen.swap(2);
			gen.swap(1);
			gen.extract_8in16(0, (w + 1) >> 1, (h + 1) >> 1);
			srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_8_8_8;
			break;

		case kVDPixType_V210:
			gen.conv_V210_to_32F();
			srcToken = (srcToken & ~kVDPixType_Mask) | kVDPixType_32F_32F_32F_LE;
			break;
		}

		// if the source is subsampled, converge on 4:4:4 subsampling, but only if we actually need
		// the auxiliary channels
		const VDPixmapSamplingInfo& sampInfo = VDPixmapGetSamplingInfo(srcToken);

		if ((dstToken & kVDPixSpace_Mask) != kVDPixSpace_Y_601 && (dstToken & kVDPixSpace_Mask) != kVDPixSpace_Y_709) {
			if (sampInfo.mCXBits | sampInfo.mCYBits | sampInfo.mCXOffset16 | sampInfo.mCbYOffset16 | sampInfo.mCrYOffset16)
				srcToken = BlitterConvertSampling(gen, srcToken, kVDPixSamp_444, w, h);
		}

		// change color spaces
		uint32 dstSpace = dstToken & kVDPixSpace_Mask;
		while((srcToken ^ dstToken) & kVDPixSpace_Mask) {
			uint32 srcSpace = srcToken & kVDPixSpace_Mask;
			uint32 targetSpace = dstSpace;

space_reconvert:
			switch(targetSpace) {
				case kVDPixSpace_BGR:
					switch(srcSpace) {
					case kVDPixSpace_YCC_709:
						switch(srcToken & kVDPixType_Mask) {
							case kVDPixType_8_8_8:
								gen.ycbcr709_to_rgb32();
								srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixSpace_BGR | kVDPixType_8888;
								break;

							case kVDPixType_16F_16F_16F_LE:
								srcToken = BlitterConvertType(gen, srcToken, kVDPixType_32F_32F_32F_LE, w, h);
								gen.ycbcr709_to_rgb32_32f();
								srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixSpace_BGR | kVDPixType_32Fx4_LE;
								break;

							case kVDPixType_32F_32F_32F_LE:
								gen.ycbcr709_to_rgb32_32f();
								srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixSpace_BGR | kVDPixType_32Fx4_LE;
								break;

							default:
								VDASSERT(false);
								break;
						}
						break;

					case kVDPixSpace_YCC_601:
						switch(srcToken & kVDPixType_Mask) {
							case kVDPixType_8_8_8:
								gen.ycbcr601_to_rgb32();
								srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixSpace_BGR | kVDPixType_8888;
								break;

							case kVDPixType_16F_16F_16F_LE:
								srcToken = BlitterConvertType(gen, srcToken, kVDPixType_32F_32F_32F_LE, w, h);
								gen.ycbcr601_to_rgb32_32f();
								srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixSpace_BGR | kVDPixType_32Fx4_LE;
								break;

							case kVDPixType_32F_32F_32F_LE:
								gen.ycbcr601_to_rgb32_32f();
								srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixSpace_BGR | kVDPixType_32Fx4_LE;
								break;

							default:
								VDASSERT(false);
								break;
						}
						break;

					case kVDPixSpace_Y_601:
						targetSpace = kVDPixSpace_YCC_601;
						goto space_reconvert;

					case kVDPixSpace_Pal:
						switch(srcToken & kVDPixType_Mask) {
							case kVDPixType_1:
								gen.conv_Pal1_to_8888(0);
								srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixSpace_BGR | kVDPixType_8888;
								break;

							case kVDPixType_2:
								gen.conv_Pal2_to_8888(0);
								srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixSpace_BGR | kVDPixType_8888;
								break;

							case kVDPixType_4:
								gen.conv_Pal4_to_8888(0);
								srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixSpace_BGR | kVDPixType_8888;
								break;

							case kVDPixType_8:
								gen.conv_Pal8_to_8888(0);
								srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixSpace_BGR | kVDPixType_8888;
								break;

							default:
								VDASSERT(false);
								break;
						}
						break;

					default:
						VDASSERT(false);
						break;
					}
					break;
				case kVDPixSpace_Y_601:
					if (srcSpace == kVDPixSpace_YCC_601) {
						gen.pop();
						gen.swap(1);
						gen.pop();
						switch(srcToken & kVDPixType_Mask) {
							case kVDPixType_32F_32F_32F_LE:
								srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixSpace_Y_601 | kVDPixType_32F_LE;
								break;
							case kVDPixType_16F_16F_16F_LE:
								srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixSpace_Y_601 | kVDPixType_16F_LE;
								break;
							case kVDPixType_8_8_8:
								srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixSpace_Y_601 | kVDPixType_8;
								break;

							default:
								VDASSERT(false);
						}
						srcToken = BlitterConvertType(gen, srcToken, kVDPixType_8, w, h);
						break;
					} else if (srcSpace == kVDPixSpace_YCC_709) {
						gen.pop();
						gen.swap(1);
						gen.pop();
						switch(srcToken & kVDPixType_Mask) {
							case kVDPixType_32F_32F_32F_LE:
								srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixSpace_Y_709 | kVDPixType_32F_LE;
								break;
							case kVDPixType_16F_16F_16F_LE:
								srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixSpace_Y_709 | kVDPixType_16F_LE;
								break;
							case kVDPixType_8_8_8:
								srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixSpace_Y_709 | kVDPixType_8;
								break;

							default:
								VDASSERT(false);
						}
						srcToken = BlitterConvertType(gen, srcToken, kVDPixType_8, w, h);
						break;
					}
					// fall through
				case kVDPixSpace_YCC_601:
					switch(srcSpace) {
					case kVDPixSpace_BGR:
						srcToken = BlitterConvertType(gen, srcToken, kVDPixType_8888, w, h);
						gen.rgb32_to_ycbcr601();
						srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixSpace_YCC_601 | kVDPixType_8_8_8;
						break;
					case kVDPixSpace_Y_601:
					case kVDPixSpace_Y_709:
						srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixSpace_YCC_601 | kVDPixType_8;

						{
							const VDPixmapSamplingInfo& sinfo = VDPixmapGetSamplingInfo(dstToken);
							int cw = ((w - 1) >> sinfo.mCXBits) + 1;
							int ch = ((h - 1) >> sinfo.mCYBits) + 1;

							gen.ldconst(0x80, cw, cw, ch, srcToken);
						}

						gen.dup();
						gen.swap(2);
						gen.swap(1);
						srcToken = kVDPixSpace_YCC_601 | kVDPixType_8_8_8 | (dstToken & kVDPixSamp_Mask);
						break;
					case kVDPixSpace_YCC_709:
						VDASSERT((srcToken & kVDPixType_Mask) == kVDPixType_8_8_8);
						gen.ycbcr709_to_ycbcr601();
						srcToken = (srcToken & ~kVDPixSpace_Mask) | kVDPixSpace_YCC_601;
						break;

					case kVDPixSpace_Pal:
						targetSpace = kVDPixSpace_BGR;
						goto space_reconvert;

					default:
						VDASSERT(false);
						break;
					}
					break;
				case kVDPixSpace_YCC_709:
					switch(srcSpace) {
					case kVDPixSpace_BGR:
						srcToken = BlitterConvertType(gen, srcToken, kVDPixType_8888, w, h);
						gen.rgb32_to_ycbcr709();
						srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixSpace_YCC_709 | kVDPixType_8_8_8;
						break;
					case kVDPixSpace_Y_709:
					case kVDPixSpace_Y_601:
						srcToken = (srcToken & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixSpace_YCC_709 | kVDPixType_8;

						{
							const VDPixmapSamplingInfo& sinfo = VDPixmapGetSamplingInfo(dstToken);
							int cw = ((w - 1) >> sinfo.mCXBits) + 1;
							int ch = ((h - 1) >> sinfo.mCYBits) + 1;

							gen.ldconst(0x80, cw, cw, ch, srcToken);
						}

						gen.dup();
						gen.swap(2);
						gen.swap(1);
						srcToken = kVDPixSpace_YCC_709 | kVDPixType_8_8_8 | (dstToken & kVDPixSamp_Mask);
						break;
					case kVDPixSpace_YCC_601:
						VDASSERT((srcToken & kVDPixType_Mask) == kVDPixType_8_8_8 || (srcToken & kVDPixType_Mask) == kVDPixType_32F_32F_32F_LE);
						gen.ycbcr601_to_ycbcr709();
						srcToken = (srcToken & ~kVDPixSpace_Mask) | kVDPixSpace_YCC_709;
						break;
					case kVDPixSpace_Pal:
						targetSpace = kVDPixSpace_BGR;
						goto space_reconvert;
					default:
						VDASSERT(false);
						break;
					}
					break;

				default:
					VDASSERT(false);
					break;
			}
		}
	}

	// check if we need a type change
	//
	// Note: If the sampling is also different, we have to be careful about what types we
	// target. The type conversion may itself involve a sampling conversion, so things get
	// VERY tricky here.
	if ((srcToken ^ dstToken) & kVDPixType_Mask) {
		bool samplingDifferent = 0 != ((srcToken ^ dstToken) & kVDPixSamp_Mask);
		uint32 intermediateTypeToken = dstToken & kVDPixType_Mask;

		if (samplingDifferent) {
			switch(dstToken & kVDPixType_Mask) {
				case kVDPixType_16F_16F_16F_LE:
					intermediateTypeToken = kVDPixType_32F_32F_32F_LE;
					break;
				case kVDPixType_8_B8R8:
					intermediateTypeToken = kVDPixType_8_8_8;
					break;
			}
		}

		srcToken = BlitterConvertType(gen, srcToken, (dstToken & ~kVDPixType_Mask) | intermediateTypeToken, w, h);
	}

	// convert subsampling if necessary
	switch(srcToken & kVDPixType_Mask) {
		case kVDPixType_8_8_8:
		case kVDPixType_16F_16F_16F_LE:
		case kVDPixType_32F_32F_32F_LE:
			if ((srcToken ^ dstToken) & kVDPixSamp_Mask)
				srcToken = BlitterConvertSampling(gen, srcToken, dstToken, w, h);
			break;
	}

	// check if we need a type change (possible with 16F)
	srcToken = BlitterConvertType(gen, srcToken, dstToken, w, h);

	return gen.create();
}
