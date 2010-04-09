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

#ifndef f_SYLIA_SCRIPTVALUE_H
#define f_SYLIA_SCRIPTVALUE_H

class CScriptArray;
struct CScriptObject;
class CScriptValue;
class IScriptInterpreter;
class VariableTableEntry;

typedef CScriptValue (*ScriptObjectLookupFuncPtr)(IScriptInterpreter *, CScriptObject *, void *lpVoid, char *szName);
typedef CScriptValue (*ScriptFunctionPtr)(IScriptInterpreter *, void *, CScriptValue *, int);
typedef void (*ScriptVoidFunctionPtr)(IScriptInterpreter *, void *, CScriptValue *, int);
typedef int (*ScriptIntFunctionPtr)(IScriptInterpreter *, void *, CScriptValue *, int);
typedef CScriptValue (*ScriptArrayFunctionPtr)(IScriptInterpreter *, void *, int);

typedef struct ScriptFunctionDef {
	ScriptFunctionPtr func_ptr;
	char *name;
	char *arg_list;
} ScriptFunctionDef;

typedef struct ScriptObjectDef {
	char *name;
	CScriptObject *obj;
} ScriptObjectDef;

typedef struct CScriptObject {
	ScriptObjectLookupFuncPtr Lookup;
	ScriptFunctionDef		*func_list;
	ScriptObjectDef			*obj_list;
} CScriptObject;

class CScriptValue {
public:
	enum { T_VOID, T_INT, T_PINT, T_STR, T_ARRAY, T_OBJECT, T_FNAME, T_FUNCTION, T_VARLV } type;
	CScriptObject *thisPtr;
	union {
		int i;
		int *pi;
		char **s;
		ScriptArrayFunctionPtr ary;
		CScriptObject *obj;
		ScriptFunctionPtr func;
		ScriptFunctionDef *fname;
		VariableTableEntry *vte;
	} u;
	void *lpVoid;

	CScriptValue()						{ type = T_VOID; }
	CScriptValue(int i)					{ type = T_INT;			u.i = i; }
	CScriptValue(int *pi)				{ type = T_PINT;		u.pi = pi; }
	CScriptValue(char **s)				{ type = T_STR;			u.s = s; }
	CScriptValue(CScriptObject *obj)	{ type = T_OBJECT;		u.obj = obj; }
	CScriptValue(CScriptObject *_thisPtr, ScriptArrayFunctionPtr csa) {
		type = T_ARRAY;
		u.ary = csa;
		thisPtr = _thisPtr;
	}
	CScriptValue(CScriptObject *_thisPtr, ScriptFunctionDef *sfd)	{
		type = T_FNAME;
		u.fname = sfd;
		thisPtr = _thisPtr;
	}
	CScriptValue(CScriptObject *_thisPtr, ScriptFunctionPtr fn)	{
		type = T_FUNCTION;
		u.func = fn;
		thisPtr = _thisPtr;
	}
	CScriptValue(VariableTableEntry *vte) { type = T_VARLV;		u.vte = vte; }

	bool isVoid()			{ return type == T_VOID; }
	bool isInt()			{ return type == T_INT; }
	bool isString()			{ return type == T_STR; }
	bool isArray()			{ return type == T_ARRAY; }
	bool isObject()			{ return type == T_OBJECT; }
	bool isFName()			{ return type == T_FNAME; }
	bool isFunction()		{ return type == T_FUNCTION; }
	bool isVarLV()			{ return type == T_VARLV; }

	int					asInt()			{ return u.i; }
	char **				asString()		{ return u.s; }
	ScriptArrayFunctionPtr		asArray()		{ return u.ary; }
	CScriptObject *		asObject()		{ return u.obj; }
	ScriptFunctionPtr	asFunction()	{ return u.func; }
	VariableTableEntry*	asVarLV()		{ return u.vte; }
};

#endif
