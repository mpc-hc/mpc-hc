//	VirtualDub - Video processing and capture application
//	Copyright (C) 1998-2002 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//
//	FILTER EXEMPTION:
//
//	As a special exemption to the GPL in order to permit creation of
//	filters that work with multiple programs as well as VirtualDub,
//	compiling with this header file shall not be considered creation
//	of a derived work; that is, the act of compiling with this header
//	file does not require your source code or the resulting module
//	to be released in source code form or under a GPL-compatible
//	license according to parts (2) and (3) of the GPL.  A filter built
//	using this header file may thus be licensed or dual-licensed so
//	that it may be used with VirtualDub as well as an alternative
//	product whose license is incompatible with the GPL.
//
//	Nothing in this exemption shall be construed as applying to
//	VirtualDub itself -- that is, this exemption does not give you
//	permission to use parts of VirtualDub's source besides this
//	header file, or to dynamically link with VirtualDub as part
//	of the filter load process, in a fashion not permitted by the
//	GPL.

#ifndef f_SYLIA_SCRIPTERROR_H
#define f_SYLIA_SCRIPTERROR_H

class CScriptError {
public:
	int err;

	enum {
		PARSE_ERROR=1,
		SEMICOLON_EXPECTED,
		IDENTIFIER_EXPECTED,

		TYPE_INT_REQUIRED,
		TYPE_ARRAY_REQUIRED,
		TYPE_FUNCTION_REQUIRED,
		TYPE_OBJECT_REQUIRED,

		OBJECT_MEMBER_NAME_REQUIRED,
		FUNCCALLEND_EXPECTED,
		TOO_MANY_PARAMS,
		DIVIDE_BY_ZERO,
		VAR_NOT_FOUND,
		MEMBER_NOT_FOUND,
		OVERLOADED_FUNCTION_NOT_FOUND,
		IDENT_TOO_LONG,
		OPERATOR_EXPECTED,
		CLOSEPARENS_EXPECTED,
		CLOSEBRACKET_EXPECTED,

		VAR_UNDEFINED,

		OUT_OF_STRING_SPACE,
		OUT_OF_MEMORY,
		INTERNAL_ERROR,
		EXTERNAL_ERROR,

		FCALL_OUT_OF_RANGE,
		FCALL_INVALID_PTYPE,
		FCALL_UNKNOWN_STR,
	};

	CScriptError(int err_num) : err(err_num) { }

	int getErr() { return err; }
};

#define SCRIPT_ERROR(x)			throw CScriptError(CScriptError::##x)

extern "C" __declspec(dllexport) char * __stdcall TranslateScriptError(int);

char inline *TranslateScriptError(CScriptError cse) {
	return TranslateScriptError(cse.getErr());
}

#endif
