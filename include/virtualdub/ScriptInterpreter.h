#ifndef f_SYLIA_SCRIPTINTERPRETER_H
#define f_SYLIA_SCRIPTINTERPRETER_H

class CScriptValue;
class CScriptError;
struct CScriptObject;
class IScriptInterpreter;

typedef CScriptValue (*ScriptRootHandlerPtr)(IScriptInterpreter *,char *,void *);

class IScriptInterpreter {
public:
	virtual	void Destroy()										=0;

	virtual void SetRootHandler(ScriptRootHandlerPtr, void *)	=0;

	virtual void ExecuteLine(char *s)							=0;

	virtual void ScriptError(int e)								=0;
	virtual char* TranslateScriptError(CScriptError& cse)		=0;
	virtual char** AllocTempString(long l)						=0;

	virtual CScriptValue LookupObjectMember(CScriptObject *obj, void *, char *szIdent) = 0;
};

extern "C" __declspec(dllexport) IScriptInterpreter * __stdcall CreateScriptInterpreter();

#define GETPROC_CREATESCRIPTINTERPRETER(hInst)	((IScriptInterpreter *(__stdcall *)())GetProcAddress(hInst, "_CreateScriptInterpreter@0"))

#define EXT_SCRIPT_ERROR(x)	(isi->ScriptError((CScriptError::x)))

#endif
