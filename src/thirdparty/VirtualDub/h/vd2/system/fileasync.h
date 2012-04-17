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

#ifndef f_VD2_SYSTEM_FILEASYNC_H
#define f_VD2_SYSTEM_FILEASYNC_H

#ifdef _MSC_VER
	#pragma once
#endif

#include <vd2/system/vdtypes.h>
#include <vd2/system/file.h>

class VDRTProfileChannel;

class IVDFileAsync {
public:
	enum Mode {
		kModeSynchronous,		///< Use synchronous I/O.
		kModeThreaded,			///< Use multithreaded I/O.
		kModeAsynchronous,		///< Use true asynchronous I/O (Windows NT only).
		kModeBuffered,			///< Use regular buffered synchronous I/O
		kModeCount
	};

	virtual ~IVDFileAsync() {}
	virtual void SetPreemptiveExtend(bool b) = 0;
	virtual bool IsPreemptiveExtendActive() = 0;
	virtual bool IsOpen() = 0;
	virtual void Open(const wchar_t *pszFilename, uint32 count, uint32 bufferSize) = 0;
	virtual void Open(VDFileHandle h, uint32 count, uint32 bufferSize) = 0;
	virtual void Close() = 0;
	virtual void FastWrite(const void *pData, uint32 bytes) = 0;
	virtual void FastWriteEnd() = 0;
	virtual void Write(sint64 pos, const void *pData, uint32 bytes) = 0;
	virtual bool Extend(sint64 pos) = 0;
	virtual void Truncate(sint64 pos) = 0;
	virtual void SafeTruncateAndClose(sint64 pos) = 0;
	virtual sint64 GetFastWritePos() = 0;
	virtual sint64 GetSize() = 0;
};

IVDFileAsync *VDCreateFileAsync(IVDFileAsync::Mode);

#endif
