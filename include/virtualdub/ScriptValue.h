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
