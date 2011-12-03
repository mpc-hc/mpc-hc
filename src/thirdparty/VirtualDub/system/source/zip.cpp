//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2004 Avery Lee, All Rights Reserved.
//
//	Beginning with 1.6.0, the VirtualDub system library is licensed
//	differently than the remainder of VirtualDub.  This particular file is
//	thus licensed as follows (the "zlib" license):
//
//	This software is provided 'as-is', without any express or implied
//	warranty.  In no event will the authors be held liable for any
//	damages arising from the use of this software.
//
//	Permission is granted to anyone to use this software for any purpose,
//	including commercial applications, and to alter it and redistribute it
//	freely, subject to the following restrictions:
//
//	1.	The origin of this software must not be misrepresented; you must
//		not claim that you wrote the original software. If you use this
//		software in a product, an acknowledgment in the product
//		documentation would be appreciated but is not required.
//	2.	Altered source versions must be plainly marked as such, and must
//		not be misrepresented as being the original software.
//	3.	This notice may not be removed or altered from any source
//		distribution.

#include "stdafx.h"
#include <vd2/system/zip.h>
#include <vd2/system/error.h>
#include <vd2/system/binary.h>

bool VDDeflateBitReader::refill() {
	sint32 tc = mBytesLeft>kBufferSize?kBufferSize:(sint32)mBytesLeft;

	if (!tc)
		return false;

	mpSrc->Read(mBuffer+kBufferSize-tc, tc);	// might throw

	mBufferPt = -tc;

	mBytesLeftLimited = mBytesLeft > kBigAvailThreshold ? kBigAvailThreshold : (unsigned)mBytesLeft;
	mBytesLeft -= tc;

	return true;
}

void VDDeflateBitReader::readbytes(void *dst, unsigned len) {
	// LAME: OPTIMIZE LATER
	uint8 *dst2 = (uint8 *)dst;
	while(len-->0)
		*dst2++ = getbits(8);
}

///////////////////////////////////////////////////////////////////////////

void VDCRCChecker::Init(uint32 crc) {
	mValue = 0xFFFFFFFF;

	for(int i=0; i<256; ++i) {
		unsigned v = i;
		for(int j=0; j<8; ++j)
			v = (v>>1) ^ (crc & -(sint32)(v&1));

		mTable[i] = v;
	}
}

void VDCRCChecker::Process(const void *src0, sint32 count) {
	const uint8 *src = (const uint8 *)src0;

	uint32 v = mValue;

	// This code is from the PNG spec.
	if (count > 0)
		do {
			v = mTable[(uint8)v ^ *src++] ^ (v >> 8);
		} while(--count);

	mValue = v;
}

uint32 VDCRCChecker::CRC(uint32 crc, const void *src, sint32 len) {
	Init(crc);
	Process(src, len);
	return CRC();
}

///////////////////////////////////////////////////////////////////////////

VDZipStream::VDZipStream()
	: mPos(0)
	, mbCRCEnabled(false)
{
}

VDZipStream::VDZipStream(IVDStream *pSrc, uint64 limit, bool bStored) {
	Init(pSrc, limit, bStored);
}

VDZipStream::~VDZipStream() {
}


void VDZipStream::Init(IVDStream *pSrc, uint64 limit, bool bStored) {
	mBits.init(pSrc, limit);
	mBlockType = kNoBlock;
	mReadPt = mWritePt = mBufferLevel = 0;
	mStoredBytesLeft = 0;
	mbNoMoreBlocks = false;

	if (bStored) {
		mStoredBytesLeft = (uint32)limit;
		mbNoMoreBlocks = true;
		mBlockType = kStoredBlock;
	}
}

const wchar_t *VDZipStream::GetNameForError() {
	return mBits.stream()->GetNameForError();
}

sint64 VDZipStream::Pos() {
	return mPos;
}

void VDZipStream::Read(void *buffer, sint32 bytes) {
	if (bytes != ReadData(buffer, bytes))
		throw MyError("Read error on compressed data");
}

sint32 VDZipStream::ReadData(void *dst0, sint32 bytes) {
	sint32 actual = 0;

	uint8 *dst = (uint8 *)dst0;

	while(bytes > 0) {
		if (mBufferLevel > 0) {
			unsigned tc = std::min<unsigned>(mBufferLevel, bytes);
			unsigned bp = 65536 - mReadPt;

			if (bp < tc) {
				memcpy(dst, mBuffer+mReadPt, bp);
				memcpy(dst+bp, mBuffer, tc-bp);
				mReadPt = tc-bp;
			} else {
				memcpy(dst, mBuffer+mReadPt, tc);
				mReadPt += tc;
			}
			mBufferLevel -= tc;
			dst += tc;
			bytes -= tc;
			actual += tc;
		} else {
			uint32 origWritePt = mWritePt;
			uint32 origBufferLevel = mBufferLevel;

			if (!Inflate())
				break;

			if (mbCRCEnabled && mBufferLevel != origBufferLevel) {
				if (mWritePt <= origWritePt) {
					mCRCChecker.Process(mBuffer+origWritePt, 65536 - origWritePt);
					mCRCChecker.Process(mBuffer, mWritePt);
				} else {
					mCRCChecker.Process(mBuffer+origWritePt, mWritePt - origWritePt);
				}
			}
		}
	}

	mPos += actual;
	return actual;
}

void VDZipStream::Write(const void *buffer, sint32 bytes) {
	throw MyError("Zip streams are read-only.");
}

bool VDZipStream::Inflate() {
	if (mBlockType == kNoBlock)
		if (mbNoMoreBlocks || !ParseBlockHeader())
			return false;

	if (mBlockType == kStoredBlock) {
		while(mBufferLevel < 65536) {
			if (mStoredBytesLeft <= 0) {
				mBlockType = kNoBlock;
				break;
			}
			uint32 tc = std::min<uint32>(65536 - mWritePt, std::min<uint32>(65536 - mBufferLevel, mStoredBytesLeft));

			mBits.readbytes(mBuffer + mWritePt, tc);

			mWritePt = (mWritePt + tc) & 65535;
			mStoredBytesLeft -= tc;
			mBufferLevel += tc;
		}
	} else {
		static const unsigned len_tbl[32]={
			3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,
			131,163,195,227,258
		};

		static const unsigned char len_bits_tbl[32]={
			0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0
		};

		static const unsigned char dist_bits_tbl[]={
			0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13
		};

		static const unsigned dist_tbl[]={
			1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,
			6145,8193,12289,16385,24577
		};

		while(mBufferLevel < 65024) {
			unsigned code, bits;

			code	= mCodeDecode[mBits.peek() & 0x7fff];
			bits	= mCodeLengths[code];

			if (!mBits.consume(bits))
				return false;

			if (code == 256) {
				mBlockType = kNoBlock;
				break;
			} else if (code >= 257) {
				unsigned	dist, len;

				code -= 257;

				len = len_tbl[code] + mBits.getbits(len_bits_tbl[code]);

				if (len < 3)
					return false;	// can happen with a bad static block

				code = mDistDecode[mBits.peek() & 0x7fff];
				bits = mCodeLengths[code + 288];

				if (!mBits.consume(bits))
					return false;

				dist = dist_tbl[code] + mBits.getbits(dist_bits_tbl[code]);

				unsigned copysrc = (mWritePt - dist) & 65535;

				mBufferLevel += len;

				// NOTE: This can be a self-replicating copy.  It must be ascending and it must
				//		 be by bytes.
//				printf("%08lx: distance %04x count %d\n", mWritePt, dist, len);
				do {
					mBuffer[mWritePt++] = mBuffer[copysrc++];
					mWritePt &= 65535;
					copysrc &= 65535;
				} while(--len);
			} else {
//				printf("%08lx: literal %02x\n", mWritePt, code);
				mBuffer[mWritePt++] = code;
				mWritePt &= 65535;
				++mBufferLevel;
			}
		}
	}

	return true;
}

namespace {
	static unsigned revword8(unsigned x) {
		x = (unsigned char )((x << 4) + (x >> 4));
		x = ((x << 2) & 0xcc) + ((x >> 2) & 0x33);
		return ((x << 1) & 0xaa) + ((x >> 1) & 0x55);
	}

	static unsigned revword15(unsigned x) {
		x = ((x << 8) & 0xff00) + ((x >> 8) & 0x00ff);
		x = ((x << 4) & 0xf0f0) + ((x >> 4) & 0x0f0f);
		x = ((x << 2) & 0xcccc) + ((x >> 2) & 0x3333);
		return (x & 0x5555) + ((x >> 2) & 0x2aaa);
	}

	static bool InflateExpandTable256(unsigned char *dst, unsigned char *lens, unsigned codes) {
		unsigned	k;
		unsigned	ki;
		unsigned	base=0;

		for(unsigned i=1; i<16; ++i) {
			ki = 1<<i;

			for(unsigned j=0; j<codes; ++j) {
				if (lens[j] == i) {
					for(k=base; k<0x100; k+=ki)
						dst[k] = j;

					base = revword8((revword8(base)+(0x100 >> i)) & 0xff);
				}
			}
		}

		return !base;
	}

	static bool InflateExpandTable32K(unsigned short *dst, unsigned char *lens, unsigned codes) {
		unsigned	k;
		unsigned	ki;
		unsigned	base=0;

		for(int i=1; i<16; ++i) {
			ki = 1<<i;

			for(unsigned j=0; j<codes; ++j) {
				if (lens[j] == i) {
					for(k=base; k<0x8000; k+=ki)
						dst[k] = j;

					base = revword15(revword15(base)+(0x8000 >> i));
				}
			}
		}

		return !base;
	}
}

bool VDZipStream::ParseBlockHeader() {
	unsigned char ltbl_lengths[20];
	unsigned char ltbl_decode[256];

	if (mBits.getbit())
		mbNoMoreBlocks = true;

	unsigned type = mBits.getbits(2);

	switch(type) {
	case 0:		// stored
		{
			mBits.align();
			if (mBits.avail() < 32)
				return false;

			mStoredBytesLeft = mBits.getbits(16);

			uint32 invCount = mBits.getbits(16);

			if ((uint16)~invCount != mStoredBytesLeft)
				return false;

			if (mBits.bytesleft() < mStoredBytesLeft)
				return false;

			mBlockType = kStoredBlock;
		}
		break;
	case 1:		// static trees
		{
			int i;

			for(i=0; i<144; ++i) mCodeLengths[i] = 8;
			for(   ; i<256; ++i) mCodeLengths[i] = 9;
			for(   ; i<280; ++i) mCodeLengths[i] = 7;
			for(   ; i<288; ++i) mCodeLengths[i] = 8;
			for(i=0; i< 32; ++i) mCodeLengths[i+288] = 5;

			if (!InflateExpandTable32K(mCodeDecode, mCodeLengths, 288)) {
				VDASSERT(false);		// code table bad
				return false;
			}
			if (!InflateExpandTable32K(mDistDecode, mCodeLengths+288, 32)) {
				VDASSERT(false);		// distance table bad
				return false;
			}

			mBlockType = kDeflatedBlock;
		}
		break;
	case 2:		// dynamic trees
		{
			if (mBits.avail() < 16)
				return false;

			const unsigned	code_count	= mBits.getbits(5) + 257;
			const unsigned	dist_count	= mBits.getbits(5) + 1;
			const unsigned	total_count	= code_count + dist_count;
			const unsigned	ltbl_count	= mBits.getbits(4) + 4;

			// decompress length table tree

			if (mBits.bitsleft() < 3*ltbl_count)
				return false;

			memset(ltbl_lengths, 0, sizeof ltbl_lengths);

			static const unsigned char hclen_tbl[]={
				16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15
			};

			for(unsigned i=0; i<ltbl_count; ++i) {
				ltbl_lengths[hclen_tbl[i]] = mBits.getbits(3);
			}

			if (!InflateExpandTable256(ltbl_decode, ltbl_lengths, 20)) {
				VDASSERT(false);	// tree table bad
				return false;
			}

			// decompress length table

			unsigned j=0;
			unsigned last = 0;
			while(j < total_count) {
				unsigned k = ltbl_decode[0xff & mBits.peek()];
				unsigned run = 1;

				if (!mBits.consume(ltbl_lengths[k]))
					return false;

				switch(k) {
				case 16:	// last run of 3-6
					if (mBits.avail() < 2)
						return false;
					run = mBits.getbits(2) + 3;
					break;
				case 17:	// zero run of 3-10
					if (mBits.avail() < 3)
						return false;
					run = mBits.getbits(3) + 3;
					last = 0;
					break;
				case 18:	// zero run of 11-138
					if (mBits.avail() < 7)
						return false;
					run = mBits.getbits(7) + 11;
					last = 0;
					break;
				default:
					last = k;
				}

				if (run+j > total_count) {
					VDASSERT(false);	// tree table bad
					return false;
				}

				do {
					mCodeLengths[j++] = last;
				} while(--run);
			}

			memmove(mCodeLengths + 288, mCodeLengths + code_count, dist_count);

			if (!InflateExpandTable32K(mCodeDecode, mCodeLengths, code_count)) {
				VDASSERT(false);	// code table bad
				return false;
			}
			if (!InflateExpandTable32K(mDistDecode, mCodeLengths+288, dist_count)) {
				VDASSERT(false);	// data table bad
				return false;
			}
			mBlockType = kDeflatedBlock;
		}
		break;
	default:
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////

#pragma pack(push, 2)

namespace {
	enum {
		kZipMethodStore		= 0,
		kZipMethodDeflate	= 8
	};

	struct ZipFileHeader {
		enum { kSignature = 0x04034b50 };
		uint32		signature;
		uint16		version_required;
		uint16		flags;
		uint16		method;
		uint16		mod_time;
		uint16		mod_date;
		uint32		crc32;
		uint32		compressed_size;
		uint32		uncompressed_size;
		uint16		filename_len;
		uint16		extrafield_len;
	};

	struct ZipFileEntry {
		enum { kSignature = 0x02014b50 };
		uint32		signature;
		uint16		version_create;
		uint16		version_required;
		uint16		flags;
		uint16		method;
		uint16		mod_time;
		uint16		mod_date;
		uint32		crc32;
		uint32		compressed_size;
		uint32		uncompressed_size;
		uint16		filename_len;
		uint16		extrafield_len;
		uint16		comment_len;
		uint16		diskno;
		uint16		internal_attrib;
		uint32		external_attrib;
		uint32		reloff_localhdr;
	};

	struct ZipCentralDir {
		enum { kSignature = 0x06054b50 };

		uint32		signature;
		uint16		diskno;
		uint16		diskno_dir;
		uint16		dirents;
		uint16		dirents_total;
		uint32		dirsize;
		uint32		diroffset;
		uint16		comment_len;
	};
}

#pragma pack(pop)

VDZipArchive::VDZipArchive() {
}

VDZipArchive::~VDZipArchive() {
}

void VDZipArchive::Init(IVDRandomAccessStream *pSrc) {
	mpStream = pSrc;

	// First, see if the central directory is at the end (common case).
	const sint64 streamLen = mpStream->Length();
	mpStream->Seek(streamLen - sizeof(ZipCentralDir));

	ZipCentralDir cdirhdr;

	mpStream->Read(&cdirhdr, sizeof cdirhdr);
	if (cdirhdr.signature != ZipCentralDir::kSignature) {
		// Okay, the central directory isn't at the end. Read the last 64K of the file
		// and see if we can spot it. 
		uint32 buflen = 65536 + sizeof(ZipCentralDir);

		if ((sint64)buflen > streamLen)
			buflen = (uint32)streamLen;

		vdfastvector<uint8> buf(buflen);
		const uint8 *bufp = buf.data();

		const sint64 bufOffset = streamLen - buflen;
		mpStream->Seek(bufOffset);
		mpStream->Read(buf.data(), buflen);

		// Search for valid end-of-central-dir signature.
		const uint32 kNativeEndSig = VDFromLE32(ZipCentralDir::kSignature);
		const uint32 kNativeStartSig = VDFromLE32(ZipFileEntry::kSignature);

		for(uint32 i=0; i<buflen-4; ++i) {
			if (VDReadUnalignedU32(bufp + i) == kNativeEndSig) {
				const uint32 diroffset = VDReadUnalignedLEU32(bufp + i + offsetof(ZipCentralDir, diroffset));
				const uint32 dirsize = VDReadUnalignedLEU32(bufp + i + offsetof(ZipCentralDir, dirsize));

				if (diroffset + dirsize == bufOffset + i) {
					uint32 testsig;
					mpStream->Seek(diroffset);
					mpStream->Read(&testsig, 4);

					if (testsig == kNativeStartSig) {
						memcpy(&cdirhdr, bufp + i, sizeof(ZipCentralDir));
						goto found_directory;
					}
				}
			}
		}

		throw MyError("Zip file has missing or bad central directory");
	}

found_directory:
	mDirectory.resize(cdirhdr.dirents_total);

	mpStream->Seek(cdirhdr.diroffset);

	for(int i=0; i<cdirhdr.dirents_total; ++i) {
		FileInfoInternal& fii = mDirectory[i];
		ZipFileEntry ent;

		mpStream->Read(&ent, sizeof ent);
		if (ent.signature != ZipFileEntry::kSignature)
			throw MyError("Zip directory is bad");

		if (ent.method != kZipMethodStore && ent.method != kZipMethodDeflate)
			throw MyError("Unsupported compression method in zip archive");

		fii.mDataStart			= ent.reloff_localhdr;
		fii.mCompressedSize		= ent.compressed_size;
		fii.mUncompressedSize	= ent.uncompressed_size;
		fii.mCRC32				= ent.crc32;
		fii.mbPacked			= ent.method == kZipMethodDeflate;
		fii.mFileName.resize(ent.filename_len);

		mpStream->Read(&*fii.mFileName.begin(), ent.filename_len);
		
		mpStream->Seek(mpStream->Pos() + ent.extrafield_len + ent.comment_len);
	}
}

sint32 VDZipArchive::GetFileCount() {
	return mDirectory.size();
}

const VDZipArchive::FileInfo& VDZipArchive::GetFileInfo(sint32 idx) {
	VDASSERT((size_t)idx < mDirectory.size());
	return mDirectory[idx];
}

IVDStream *VDZipArchive::OpenRawStream(sint32 idx) {
	const FileInfoInternal& fi = mDirectory[idx];

	mpStream->Seek(fi.mDataStart);

	ZipFileHeader hdr;
	mpStream->Read(&hdr, sizeof hdr);

	if (hdr.signature != ZipFileHeader::kSignature)
		throw MyError("Bad header for file in zip archive");

	mpStream->Seek(fi.mDataStart + sizeof(hdr) + hdr.filename_len + hdr.extrafield_len);

	return mpStream;
}

///////////////////////////////////////////////////////////////////////////

VDGUnzipStream::VDGUnzipStream() {
}

VDGUnzipStream::VDGUnzipStream(IVDStream *pSrc, uint64 limit) {
	Init(pSrc, limit);
}

VDGUnzipStream::~VDGUnzipStream() {
}

void VDGUnzipStream::Init(IVDStream *pSrc, uint64 limit) {
	// See RFC1952 for a description of the gzip header format.
	uint8 hdr[10];

	uint32 gzipContainerBytes = 10 + 8;	// header + footer
	pSrc->Read(hdr, 10);
	if (hdr[0] != 0x1f || hdr[1] != 0x8b)
		throw MyError("Source stream is not in gzip format.");

	if (hdr[2] != 0x08)
		throw MyError("Gzip stream uses an unsupported compression method.");

	enum {
		FLG_FTEXT		= 0x01,
		FLG_FHCRC		= 0x02,
		FLG_FEXTRA		= 0x04,
		FLG_FNAME		= 0x08,
		FLG_FCOMMENT	= 0x10
	};

	const uint8 flg = hdr[3];

	if (flg & FLG_FEXTRA) {
		uint8 xlendat[2];
		pSrc->Read(xlendat, 2);

		uint32 xlen = VDReadUnalignedLEU16(xlendat);
		uint8 buf[256];

		gzipContainerBytes += xlen + 2;

		while(xlen) {
			uint32 tc = xlen > 256 ? 256 : xlen;
			pSrc->Read(buf, tc);
			xlen -= tc;
		}
	}

	if (flg & FLG_FNAME) {
		// ugh
		uint8 c;
		for(;;) {
			pSrc->Read(&c, 1);
			++gzipContainerBytes;

			if (!c)
				break;

			mFilename += c;
		} 
	}

	if (flg & FLG_FCOMMENT) {
		// ugh
		uint8 c;
		do {
			pSrc->Read(&c, 1);
			++gzipContainerBytes;
		} while(c);
	}

	if (flg & FLG_FHCRC) {
		uint16 crc16;

		pSrc->Read(&crc16, 2);
		gzipContainerBytes += 2;
	}

	if (gzipContainerBytes > limit)
		throw MyError("The gzip compressed data is invalid.");

	limit -= gzipContainerBytes;

	VDZipStream::Init(pSrc, limit, false);
}

