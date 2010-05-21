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

#ifndef f_VD2_SYSTEM_DEBUG_H
#define f_VD2_SYSTEM_DEBUG_H

#include <vd2/system/vdtypes.h>

class IVDExternalCallTrap {
public:
	virtual void OnMMXTrap(const wchar_t *context, const char *file, int line) = 0;
	virtual void OnFPUTrap(const wchar_t *context, const char *file, int line, uint16 fpucw) = 0;
	virtual void OnSSETrap(const wchar_t *context, const char *file, int line, uint32 mxcsr) = 0;
};

void VDSetExternalCallTrap(IVDExternalCallTrap *);

bool IsMMXState();
void ClearMMXState();
void VDClearEvilCPUStates();
void VDPreCheckExternalCodeCall(const char *file, int line);
void VDPostCheckExternalCodeCall(const wchar_t *mpContext, const char *mpFile, int mLine);

struct VDSilentExternalCodeBracket {
	VDSilentExternalCodeBracket() {
		VDClearEvilCPUStates();
	}

	~VDSilentExternalCodeBracket() {
		VDClearEvilCPUStates();
	}
};

struct VDExternalCodeBracketLocation {
	VDExternalCodeBracketLocation(const wchar_t *pContext, const char *file, const int line)
		: mpContext(pContext)
		, mpFile(file)
		, mLine(line)
	{
	}

	const wchar_t *mpContext;
	const char *mpFile;
	const int mLine;	
};

struct VDExternalCodeBracket {
	VDExternalCodeBracket(const wchar_t *pContext, const char *file, const int line)
		: mpContext(pContext)
		, mpFile(file)
		, mLine(line)
	{
		VDPreCheckExternalCodeCall(file, line);
	}

	VDExternalCodeBracket(const VDExternalCodeBracketLocation& loc)
		: mpContext(loc.mpContext)
		, mpFile(loc.mpFile)
		, mLine(loc.mLine)
	{
	}

	~VDExternalCodeBracket() {
		VDPostCheckExternalCodeCall(mpContext, mpFile, mLine);
	}

	operator bool() const { return false; }

	const wchar_t *mpContext;
	const char *mpFile;
	const int mLine;
};

#endif
