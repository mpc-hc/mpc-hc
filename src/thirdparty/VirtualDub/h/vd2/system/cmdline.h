//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2005 Avery Lee, All Rights Reserved.
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

#ifndef f_VD2_SYSTEM_CMDLINE_H
#define f_VD2_SYSTEM_CMDLINE_H

#include <vd2/system/vdstl.h>

class VDStringSpanW;

class VDCommandLineIterator {
	friend class VDCommandLine;
public:
	VDCommandLineIterator() : mIndex(1) {}

private:
	int mIndex;
};

class VDCommandLine {
public:
	VDCommandLine();
	VDCommandLine(const wchar_t *s);
	~VDCommandLine();

	void Init(const wchar_t *s);

	// This splits the cmdline using rules that are closer to Visual C++'s:
	// - 2N+1 backslashes followed by a quote inserts a literal quote.
	// - 2N backslashes followed by a quote toggles the quote state.
	// - Outside of a quote, spaces, tabs, and forward slashes delimit parameters.
	//
	// We still have special switch processing:
	// - A parameter starting with a forward slash followed by a ? or an alphanumeric
	//   char is a switch. A switch is terminated by a non-alphanumeric character or
	//   a colon.
	void InitAlt(const wchar_t *s);

	uint32 GetCount() const;
	const wchar_t *operator[](int index) const;
	const VDStringSpanW operator()(int index) const;

	bool GetNextArgument(VDCommandLineIterator& index, const wchar_t *& token, bool& isSwitch) const;
	bool GetNextNonSwitchArgument(VDCommandLineIterator& index, const wchar_t *& token) const;
	bool GetNextSwitchArgument(VDCommandLineIterator& index, const wchar_t *& token) const;
	bool FindAndRemoveSwitch(const wchar_t *name);
	bool FindAndRemoveSwitch(const wchar_t *name, const wchar_t *& token);

protected:
	void RemoveArgument(int index);

	vdfastvector<wchar_t>	mLine;

	struct Token {
		int mTokenIndex;
		bool mbIsSwitch;
	};

	vdfastvector<Token>	mTokens;
};

#endif
