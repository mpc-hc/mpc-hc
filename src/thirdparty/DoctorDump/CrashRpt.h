// Copyright (c) 2014, Idol Software, Inc.
// All rights reserved.
//
// This file is part of Doctor Dump SDK.
// 
// Redistribution and use in source and binary forms without modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
//    in the documentation and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Project web site http://drdump.com

#ifndef __CRASH_RPT_H__
#define __CRASH_RPT_H__

#include <windows.h>

/** @file */

#if !(defined CRASHRPT_ENABLE_RELEASE_ASSERTS)
//! To redefine C \b assert macro behavior define macro \b CRASHRPT_ENABLE_RELEASE_ASSERTS as \b 1 and include CrashRpt.h
//! in each translation unit as soon as possible (precompiled header is the best place for that).
//!	\code
//! 	#define CRASHRPT_ENABLE_RELEASE_ASSERTS 1 
//! 	#include <CrashRpt.h>
//!	\endcode
//! When \b _DEBUG define is not defined (it is Release configuration) \b assert macro evaluates an expression and, when the result is false,
//! sends report and continues execution.
//! When \b _DEBUG define is defined (it is Debug configuration) \b assert macro works as standard assert macro from assert.h header.
//! \note All assert calls before CrashRpt.h inclusion would work as standard asserts.
//! \sa crash_rpt::CrashRpt::ExceptionAssertionViolated crash_rpt::CrashRpt::SkipDoctorDump_SendAssertionViolated
#	define CRASHRPT_ENABLE_RELEASE_ASSERTS 1
#	undef CRASHRPT_ENABLE_RELEASE_ASSERTS // this is to make doxygen document CRASHRPT_ENABLE_RELEASE_ASSERTS macro
#elif CRASHRPT_ENABLE_RELEASE_ASSERTS == 1
#	include <assert.h>
#		ifndef _DEBUG
#			undef assert
#			define assert(expr) ((void) (!(expr) && (crash_rpt::SkipDoctorDump_ReportAssertionViolation<__COUNTER__>(__FUNCTION__ ": "#expr " is false" ), true)))
#		endif // !_DEBUG
#endif // CRASHRPT_ENABLE_RELEASE_ASSERTS

namespace crash_rpt {

namespace {

	// This template should be in anonymous namespace since __COUNTER__ is unique only for a single translation unit (as anonymous namespace items)
	template<unsigned uniqueAssertId>
	__forceinline static void SkipDoctorDump_ReportAssertionViolation(LPCSTR dumpGroup)
	{
		static LONG volatile isAlreadyReported = FALSE;
		if (TRUE == InterlockedCompareExchange(&isAlreadyReported, TRUE, FALSE))
			return;
		::RaiseException(CrashRpt::ExceptionAssertionViolated, 0, 1, reinterpret_cast<ULONG_PTR*>(&dumpGroup));
	}

} // namespace {

//! Information about exception being processed.
struct ExceptionInfo
{
	DWORD	ExceptionInfoSize;			//!< Size of this structure. Should be set to sizeof(ExceptionInfo).
	BOOL	FromSendReport;				//!< Indicates is exception processed using CrashRpt::SendReport() call.
	PEXCEPTION_POINTERS ExceptionPointers; //!< Exception pointers. ExceptionPointers->ExceptionRecord->ExceptionCode contains SEH code or ExceptionAssertionViolated or other Exception* constants for C/C++ special handlers.
	DWORD	ThreadId;					//!< Thread identifier of the crashed thread.
};

//! Stages when crash processing callback called.
enum CrashProcessingCallbackStage
{
	BeforeSendReport,					//!< Callback is called before report send.
	AfterSendReport						//!< Callback is called after the report has been sent.
};

//! Result code for crash processing callback.
enum CrashProcessingCallbackResult
{
	DoDefaultActions,					//!< Default result. Crash handler continues its default processing.
	SkipSendReportReturnDefaultResult,	//!< Crash handler doesn't send the report and returns its default result.
	ContinueExecution,					//!< Explicitly force crash handler to return EXCEPTION_CONTINUE_EXECUTION.
	ContinueSearch,						//!< Explicitly force crash handler to return EXCEPTION_CONTINUE_SEARCH.
	ExecuteHandler						//!< Explicitly force crash handler to return EXCEPTION_EXECUTE_HANDLER.
};

//! Client crash callback function prototype.
typedef CrashProcessingCallbackResult (CALLBACK *PFNCRASHPROCESSINGCALLBACK)(
	CrashProcessingCallbackStage stage,	//!< Current crash processing stage.
	ExceptionInfo* exceptionInfo,		//!< Information about exception being processed.
	LPVOID	userData					//!< Pointer to user-defined data.
	);


namespace custom_data_collection {

//! Result of custom data collection.
enum Result
{
	ShowUploadConfirmDialog, 	//!< Proceed to upload confirm dialog stage.
	DoUpload,					//!< Skip upload confirm dialog and proceed to upload stage.
	CancelUpload,				//!< Cancel upload.
};

//! Information about the exception.
struct ExceptionInfo
{
	DWORD	ExceptionInfoSize;	//!< Size of this structure. Should be set to sizeof(ExceptionInfo).
	LPBYTE	UserData;			//!< Pointer to user-defined data buffer.
	DWORD	UserDataSize;		//!< Size of userData buffer.
	HANDLE	Process;			//!< Handle to the crashed process.
	DWORD	ProcessId;			//!< Process ID of the crashed process.
	LPVOID	ExceptInfo;			//!< Pointer to \b MINIDUMP_EXCEPTION_INFORMATION structure.
	BOOL	WasAssert;			//!< Is it assert?  (see \ref crash_rpt::CrashRpt::SkipDoctorDump_SendAssertionViolated).
	LPCSTR	DumpGroup;			//!< Used-defined dump group (see \ref crash_rpt::CrashRpt::SkipDoctorDump_SendAssertionViolated).
	int		DumpID;				//!< Dump ID of this crash in DrDump service.
	int		DumpGroupID;		//!< DumpGroup ID of this crash in DrDump service.
	int		ProblemID;			//!< Problem ID of this crash in DrDump service.
};

//! This interface used to add custom data to crash report.
struct IDataBag
{
	//! You may add any file to crash report. This file will be read when crash appears and will be sent within the report.
	//! Multiple files may be added. Filename of the file in the report may be changed to any name.
	//! \return If the function succeeds, the return value is \b true.
	virtual bool AddFileToReport(
		LPCWSTR path,						//!< [in] Path to the file, that will be added to the report.
		LPCWSTR reportFileName /* = NULL */	//!< [in] Filename that will be used in report for this file. If parameter is \b NULL, original name from path will be used.
		) = 0;

	//! Remove from report the file that was registered earlier to be sent within report.
	//! \return If the function succeeds, the return value is \b true.
	virtual bool RemoveFileFromReport(
		LPCWSTR path						//!< [in] Path to the file, that will be removed from the report.
		) = 0;

	//! You may add any key/value pair to crash report.
    //! \return If the function succeeds, the return value is \b true.
    virtual bool AddUserInfoToReport(
        LPCWSTR key,                        //!< [in] key string that will be added to the report.
        LPCWSTR value                       //!< [in] value for the key.
        ) = 0;

	//! You may remove any key that was added previously to crash report by \a AddUserInfoToReport.
	//! \return If the function succeeds, the return value is \b true.
	virtual bool RemoveUserInfoFromReport(
		LPCWSTR key                        //!< [in] key string that will be removed from the report.
		) = 0;
};

//! Client crash callback function prototype.
typedef Result (CALLBACK *PFNCUSTOMDATACOLLECTIONCALLBACK)(
	const ExceptionInfo& exceptionInfo,		//!< Information about exception being processed.
	IDataBag* dataBag						//!< Interface for adding custom data to crash report.
	);

//! Contains data for optional custom data collection after the crash in context of sendrpt.exe process.
struct Settings
{
	DWORD	SettingsSize;					//!< Size of this structure. Should be set to sizeof(CustomDataCollectionSettings).
	LPCWSTR	CollectionLibraryPath;			//!< Path to dll where collection function exported from.
	LPCSTR	CollectionFunctionName;			//!< Name of collection function exported from \ref CollectionLibraryPath. It should have \ref crash_rpt::custom_data_collection::PFNCUSTOMDATACOLLECTIONCALLBACK prototype.
	LPBYTE	UserData;						//!< Pointer to user-defined data buffer.
	DWORD	UserDataSize;					//!< Size of \ref UserData buffer.
};

}

//! Contains data that identifies your application.
struct ApplicationInfo
{
	DWORD   ApplicationInfoSize;        //!< Size of this structure. Should be set to sizeof(ApplicationInfo).
	LPCSTR  ApplicationGUID;            //!< GUID assigned to this application in form 00000000-0000-0000-0000-000000000000.
	LPCSTR  Prefix;                     //!< Prefix that will be used with the dump name: YourPrefix_v1.v2.v3.v4_YYYYMMDD_HHMMSS.mini.dmp.
	LPCWSTR AppName;                    //!< Application name that will be shown in message box.
	LPCWSTR Company;                    //!< Company name that will be shown in message box.
	USHORT  V[4];                       //!< Version of this application.
	USHORT  Hotfix;                     //!< Version of hotfix for this application (reserved for future use, should be 0).
	LPCWSTR PrivacyPolicyUrl;           //!< URL to privacy policy. If NULL default privacy policy is used.
};

//! \brief Contains crash handling behavior customization parameters. 
//!
//! Default values for all parameters is 0/FALSE.
struct HandlerSettings
{
	DWORD   HandlerSettingsSize;        //!< Size of this structure. Should be set to sizeof(HandlerSettings).
	BOOL    LeaveDumpFilesInTempFolder; //!< To leave error reports in temp folder you should set this member to TRUE. Your support or test lab teams can use that reports later.
	BOOL    OpenProblemInBrowser;       //!< To open Web-page belonging to the uploaded report after it was uploaded set this member to TRUE. It is useful for test lab to track the bug or write some comments.
	BOOL    UseWER;                     //!< To continue use Microsoft Windows Error Reporting (WER) set this member to TRUE. In that case after Doctor Dump send report dialog Microsoft send report dialog also will be shown. This can be necessary in case of Windows Logo program.
	DWORD   SubmitterID;                //!< Doctor Dump user ID. Uploaded report will be marked as uploaded by this user. This is useful for Doctor Dump and bug tracking system integration. Set to \b 0 if user using this application is anonymous.
	BOOL    SendAdditionalDataWithoutApproval; //!< To automatically accepted the question "Do you want to send more information about the problem?" set this member to TRUE .
	BOOL    OverrideDefaultFullDumpType;//!< To override default type of data gathered by the library set this member to TRUE and set required type of data in \a FullDumpType.
	DWORD   FullDumpType;               //!< The type of information to be generated when full dump is requested by Doctor Dump. This parameter can be one or more of the values from the MINIDUMP_TYPE enumeration.
	LPCWSTR LangFilePath;               //!< To customize localization set this member to the path to the language file (including file name).
	LPCWSTR SendRptPath;                //!< Set this member to NULL to use default behavior when SendRpt is named sendrpt.exe and exist in same folder with crashrpt.dll. Set to filename if sendrpt.exe has another name but it is in same folder as crashrpt.dll. Set to full path in other cases.
	LPCWSTR DbgHelpPath;                //!< Set this member to NULL to use default behavior when DbgHelp is named dbghelp.dll and exist in same folder with crashrpt.dll. Set to filename if dbghelp.dll has another name but it is in same folder as crashrpt.dll. Set to full path in other cases.
										//!< \note You should use dbghelp.dll that distributed with crashrpt.dll and not the %SystemRoot%\\System32\\dbghelp.dll, because only that dll supports all required functionality. See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms679294(v=vs.85).aspx">DbgHelp Versions</a> for more information.
	PFNCRASHPROCESSINGCALLBACK CrashProcessingCallback; //!< Callback function that will be called when crash reporting occurs. Set to NULL if no special processing needed.
	LPVOID CrashProcessingCallbackUserData; //!< User defined parameter for CrashProcessingCallback. Optional.
	custom_data_collection::Settings* CustomDataCollectionSettings;
										//!< Contains data for optional custom data collection after the crash in context of sendrpt.exe process.
};

//! \brief To enable crash processing you should create an instance of this class.
//!
//! It should be created as global static object and correctly initialized.
//! Also you may instantiate it in your main() or WinMain() function as soon as possible.
class CrashRpt
{
public:
	//! \example Sample.cpp 
	//! This is an example of how to use the CrashRpt class.

	//! CrashRpt constructor. Loads crashrpt.dll and initializes crash handling.
	//! \note The crashrpt.dll is allowed to be missing. In that case there will be no crash handling.
	CrashRpt(
		LPCSTR  applicationGUID,            //!< [in] GUID assigned to this application.
		LPCWSTR appName,                    //!< [in] Application name that will be shown in message box.
		LPCWSTR company                     //!< [in] Company name that will be shown in message box.
		) throw()
	{
		if (!LoadDll())
			return;

		InitCrashRpt(applicationGUID, NULL, appName, company, TRUE);
	}

	//! CrashRpt constructor. Loads crashrpt.dll and initializes crash handling.
	//! \note The crashrpt.dll is allowed to be missing. In that case there will be no crash handling.
	CrashRpt(
		LPCSTR  applicationGUID,            //!< [in] GUID assigned to this application.
		LPCSTR  prefix,                     //!< [in] Prefix that will be used with the dump name: YourPrefix_v1.v2.v3.v4_YYYYMMDD_HHMMSS.mini.dmp.
		LPCWSTR appName,                    //!< [in] Application name that will be shown in message box.
		LPCWSTR company,                    //!< [in] Company name that will be shown in message box.
		BOOL    ownProcess = TRUE           //!< [in] If you own the process your code running in set this option to \b TRUE. If don't (for example you write
											//!<      a plugin to some external application) set this option to \b FALSE. In that case you need to explicitly
											//!<      catch exceptions. See \ref SendReport for more information.
		) throw()
	{
		if (!LoadDll())
			return;

		InitCrashRpt(applicationGUID, prefix, appName, company, ownProcess);
	}

	//! CrashRpt constructor. Loads crashrpt.dll and initializes crash handling.
	//! \note The crashrpt.dll is allowed to be missing. In that case there will be no crash handling.
	CrashRpt(
		LPCWSTR crashrptPath,				//!< [in] Path to crashrpt.dll file. File may be renamed.
		LPCSTR  applicationGUID,            //!< [in] GUID assigned to this application.
		LPCSTR  prefix,                     //!< [in] Prefix that will be used with the dump name: YourPrefix_v1.v2.v3.v4_YYYYMMDD_HHMMSS.mini.dmp.
		LPCWSTR appName,                    //!< [in] Application name that will be shown in message box.
		LPCWSTR company,                    //!< [in] Company name that will be shown in message box.
		BOOL    ownProcess = TRUE           //!< [in] If you own the process your code running in set this option to \b TRUE. If don't (for example you write
		//!<      a plugin to some external application) set this option to \b FALSE. In that case you need to explicitly
		//!<      catch exceptions. See \ref SendReport for more information.
		) throw()
	{
		if (!LoadDll(crashrptPath))
			return;

		InitCrashRpt(applicationGUID, prefix, appName, company, ownProcess);
	}

	//! CrashRpt constructor. Loads crashrpt.dll and initializes crash handling.
	//! \note The crashrpt.dll is allowed to be missing. In that case there will be no crash handling.
	CrashRpt(
		ApplicationInfo* applicationInfo,	//!< [in] Pointer to the ApplicationInfo structure that identifies your application.
		HandlerSettings* handlerSettings,	//!< [in] Pointer to the HandlerSettings structure that customizes crash handling behavior. This parameter can be \b NULL.
		BOOL    ownProcess = TRUE           //!< [in] If you own the process your code running in set this option to \b TRUE. If don't (for example you write
											//!<      a plugin to some external application) set this option to \b FALSE. In that case you need to explicitly
											//!<      catch exceptions. See \ref SendReport for more information.
		) throw()
	{
		if (!LoadDll())
			return;

		InitCrashRpt(applicationInfo, handlerSettings, ownProcess);
	}

	//! CrashRpt constructor. Loads crashrpt.dll and initializes crash handling.
	//! \note The crashrpt.dll is allowed to be missing. In that case there will be no crash handling.
	CrashRpt(
		LPCWSTR crashrptPath,				//!< [in] Path to crashrpt.dll file. File may be renamed.
		ApplicationInfo* applicationInfo,	//!< [in] Pointer to the ApplicationInfo structure that identifies your application.
		HandlerSettings* handlerSettings,	//!< [in] Pointer to the HandlerSettings structure that customizes crash handling behavior. This parameter can be \b NULL.
		BOOL    ownProcess = TRUE           //!< [in] If you own the process your code running in set this option to \b TRUE. If don't (for example you write
											//!<      a plugin to some external application) set this option to \b FALSE. In that case you need to explicitly
											//!<      catch exceptions. See \ref SendReport for more information.
		) throw()
	{
		if (!LoadDll(crashrptPath))
			return;

		InitCrashRpt(applicationInfo, handlerSettings, ownProcess);
	}


	//! CrashRpt constructor. Loads crashrpt.dll. You should call \ref InitCrashRpt to turn on crash handling.
	//! \note The crashrpt.dll is allowed to be missing. In that case there will be no crash handling.
	CrashRpt(
		LPCWSTR crashrptPath = NULL			//!< [in] Path to crashrpt.dll file. File may be renamed.
		) throw()
	{
		LoadDll(crashrptPath);
	}

	//! CrashRpt destructor.
	//! \note It doesn't unload crashrpt.dll and doesn't disable crash handling since crash may appear on very late phase of application exit.
	//!	      For example destructor of some static variable that is called after return from main() may crash.
	~CrashRpt()
	{
		if (!m_IsReadyToExit)
			return;

		// If crash has happen not in main thread we should wait here until report will be sent
		// or else program will be terminated after return from main() and report sending will be halted.
		while (!m_IsReadyToExit())
			::Sleep(100);

#if _WIN32_WINNT >= 0x0501 /*_WIN32_WINNT_WINXP*/
		if (m_bSkipAssertsAdded)
			RemoveVectoredExceptionHandler(SkipAsserts);
#endif
	}

	//! Checks that crash handling was enabled.
	//! \return Return \b true if crash handling was enabled.
	bool IsCrashHandlingEnabled() const
	{
		return m_bWorking;
	}

	//! Initializes crash handler.
	//! \note You may call this function multiple times if some data has changed.
	//! \return Return \b true if crash handling was enabled.
	bool InitCrashRpt(
		ApplicationInfo* applicationInfo,	//!< [in] Pointer to the ApplicationInfo structure that identifies your application.
		HandlerSettings* handlerSettings,	//!< [in] Pointer to the HandlerSettings structure that customizes crash handling behavior. This parameter can be \b NULL.
		BOOL    ownProcess = TRUE           //!< [in] If you own the process your code running in set this option to \b TRUE. If don't (for example you write
											//!<      a plugin to some external application) set this option to \b FALSE. In that case you need to explicitly
											//!<      catch exceptions. See \ref SendReport for more information.
		) throw()
	{
		if (!m_InitCrashRpt)
			return false;

		m_bWorking = m_InitCrashRpt(applicationInfo, handlerSettings, ownProcess) != FALSE;

		return m_bWorking;
	}

	//! Initializes crash handler.
	//! \return Return \b true if crash handling was enabled.
	//! \warning This method is deprecated. Use \ref InitCrashRpt instead.
	__declspec(deprecated)
	bool InitCrashHandler(
		ApplicationInfo* applicationInfo,	//!< [in] Pointer to the ApplicationInfo structure that identifies your application.
		HandlerSettings* handlerSettings,	//!< [in] Pointer to the HandlerSettings structure that customizes crash handling behavior. This parameter can be \b NULL.
		BOOL ownProcess = TRUE           	//!< [in] If you own the process your code running in set this option to \b TRUE. If don't (for example you write
											//!<      a plugin to some external application) set this option to \b FALSE. In that case you need to explicitly
											//!<      catch exceptions. See \ref SendReport for more information.
		) throw()
	{
		return InitCrashRpt(applicationInfo, handlerSettings, ownProcess);
	}
	

	//! Initializes crash handler.
	//! \note You may call this function multiple times if some data has changed.
	//! \return Return \b true if crash handling was enabled.
	bool InitCrashRpt(
		LPCSTR  applicationGUID,            //!< [in] GUID assigned to this application.
		LPCSTR  prefix,                     //!< [in] Prefix that will be used with the dump name: YourPrefix_v1.v2.v3.v4_YYYYMMDD_HHMMSS.mini.dmp.
		LPCWSTR appName,                    //!< [in] Application name that will be shown in message box.
		LPCWSTR company,                    //!< [in] Company name that will be shown in message box.
		BOOL    ownProcess = TRUE           //!< [in] If you own the process your code running in set this option to \b TRUE. If don't (for example you write
											//!<      a plugin to some external application) set this option to \b FALSE. In that case you need to explicitly
											//!<      catch exceptions. See \ref SendReport for more information.
		) throw()
	{
		if (!m_GetVersionFromApp)
			return false;

		ApplicationInfo appInfo;
		memset(&appInfo, 0, sizeof(appInfo));
		appInfo.ApplicationInfoSize = sizeof(appInfo);
		appInfo.ApplicationGUID = applicationGUID;
		appInfo.Prefix = prefix;
		appInfo.AppName = appName;
		appInfo.Company = company;
		if (!m_GetVersionFromApp(&appInfo))
			appInfo.V[0] = 1;

		HandlerSettings handlerSettings;
		memset(&handlerSettings, 0, sizeof(handlerSettings));
		handlerSettings.HandlerSettingsSize = sizeof(handlerSettings);
		handlerSettings.OpenProblemInBrowser = TRUE;

		return InitCrashRpt(&appInfo, &handlerSettings, ownProcess);
	}

	//! \note This function is experimental and may not be available and may not be support by Doctor Dump in the future.
	//! You may set custom information for your possible report.
	//! This text will be available on Doctor Dump dumps page.
	//! The text should not contain private information.
	//! \return If the function succeeds, the return value is \b true.
	bool SetCustomInfo(
		LPCWSTR text                        //!< [in] custom info for the report. The text will be cut to 100 characters.
		)
	{
		if (!m_SetCustomInfo)
			return false;
		m_SetCustomInfo(text);
		return true;
	}

    //! You may add any key/value pair to crash report.
    //! \return If the function succeeds, the return value is \b true.
	//! \note This function is thread safe.
    bool AddUserInfoToReport(
        LPCWSTR key,                        //!< [in] key string that will be added to the report.
        LPCWSTR value                       //!< [in] value for the key.
        ) throw()
    {
        if (!m_AddUserInfoToReport)
            return false;
        m_AddUserInfoToReport(key, value);
        return true;
    }

	//! You may remove any key that was added previously to crash report by \a AddUserInfoToReport.
	//! \return If the function succeeds, the return value is \b true.
	//! \note This function is thread safe.
	bool RemoveUserInfoFromReport(
		LPCWSTR key                        //!< [in] key string that will be removed from the report.
		)
	{
		if (!m_RemoveUserInfoFromReport)
			return false;
		m_RemoveUserInfoFromReport(key);
		return true;
	}

	//! You may add any file to crash report. This file will be read when crash appears and will be sent within the report.
	//! Multiple files may be added. Filename of the file in the report may be changed to any name.
	//! \return If the function succeeds, the return value is \b true.
	//! \note This function is thread safe.
	bool AddFileToReport(
		LPCWSTR path,						//!< [in] Path to the file, that will be added to the report.
		LPCWSTR reportFileName /* = NULL */	//!< [in] Filename that will be used in report for this file. If parameter is \b NULL, original name from path will be used.
		) throw()
	{
		if (!m_AddFileToReport)
			return false;
		m_AddFileToReport(path, reportFileName);
		return true;
	}

	//! Remove from report the file that was registered earlier to be sent within report.
	//! \return If the function succeeds, the return value is \b true.
	//! \note This function is thread safe.
	bool RemoveFileFromReport(
		LPCWSTR path	//!< [in] Path to the file, that will be removed from the report.
		) throw()
	{
		if (!m_RemoveFileFromReport)
			return false;
		m_RemoveFileFromReport(path);
		return true;
	}

	//! Fills version field (V) of ApplicationInfo with product version
	//! found in the executable file of the current process.
	//! \return If the function succeeds, the return value is \b true.
	bool GetVersionFromApp(
		ApplicationInfo* appInfo //!< [out] Pointer to ApplicationInfo structure. Its version field (V) will be set to product version.
		) throw()
	{
		if (!m_GetVersionFromApp)
			return false;
		return m_GetVersionFromApp(appInfo) != FALSE;
	}

	//! Fill version field (V) of ApplicationInfo with product version found in the file specified.
	//! \return If the function succeeds, the return value is \b true.
	bool GetVersionFromFile(
		LPCWSTR path,				//!< [in] Path to the file product version will be extracted from.
		ApplicationInfo* appInfo	//!< [out] Pointer to ApplicationInfo structure. Its version field (V) will be set to product version.
		) throw()
	{
		if (!m_GetVersionFromFile)
			return false;
		return m_GetVersionFromFile(path, appInfo) != FALSE;
	}

	//! If you do not own the process your code running in (for example you write a plugin to some
	//! external application) you need to properly initialize CrashRpt using \b ownProcess option.
	//! Also you need to explicitly catch all exceptions in all entry points to your code and in all
	//! threads you create. To do so use this construction:
	//! \code
	//! bool SomeEntryPoint(PARAM p)
	//! {
	//!		__try
	//!		{
	//!			return YouCode(p);
	//!		}
	//!		__except (CrashRpt::SendReport(GetExceptionInformation()))
	//!		{
	//!			::ExitProcess(0); // It is better to stop the process here or else corrupted data may incomprehensibly crash it later.
	//!			return false;
	//!		}
	//! }
	//! \endcode
	LONG SendReport(
		EXCEPTION_POINTERS* exceptionPointers	//!< [in] Pointer to EXCEPTION_POINTERS structure. You should get it using GetExceptionInformation()
												//!<      function inside __except keyword.
		)
	{
		if (!m_SendReport)
		{
			// There is no crash handler but asserts should not crash application anyway, so let's continue
			if (exceptionPointers->ExceptionRecord->ExceptionCode == ExceptionAssertionViolated)
				return EXCEPTION_CONTINUE_EXECUTION;
			return EXCEPTION_CONTINUE_SEARCH;
		}
		return m_SendReport(exceptionPointers);
	}

	//! To send a report about violated assertion you can throw exception with this exception code
	//! using: \code RaiseException(CrashRpt::ExceptionAssertionViolated, 0, 0, NULL); \endcode
	//! Execution will continue after report will be sent (EXCEPTION_CONTINUE_EXECUTION would be used).
	//! You may pass grouping string as first parameter (see \a SkipDoctorDump_SendAssertionViolated).
	//! \note If you called CrashRpt constructor and crashrpt.dll was missing you still may using this exception.
	//!		  It will be caught, ignored and execution will continue. \ref SendReport function also works safely
	//!       when crashrpt.dll was missing.
	//! \sa CRASHRPT_ENABLE_RELEASE_ASSERTS SkipDoctorDump_SendAssertionViolated
	static const DWORD ExceptionAssertionViolated = ((DWORD)0xCCE17000);

	//! Exception code for Terminate()/SIGABRT call in crash callback.
	static const DWORD ExceptionCppTerminate = ((DWORD)0xCCE17001);

	//! Exception code for pure virtual call in crash callback.
	static const DWORD ExceptionPureCall = ((DWORD)0xCCE17002);

	//! Exception code for CRT invalid parameter call in crash callback.
	static const DWORD ExceptionInvalidParameter = ((DWORD)0xCCE17003);

	//! Sends assertion violation report from this point and continue execution.
	//! \sa ExceptionAssertionViolated
	//! \note Functions containing "SkipDoctorDump" will be ignored in stack parsing.
	//! \sa CRASHRPT_ENABLE_RELEASE_ASSERTS ExceptionAssertionViolated
	void SkipDoctorDump_SendAssertionViolated(
		LPCSTR dumpGroup = NULL		//!< [in] All dumps with that group will be separated from dumps with same stack but another group. Set parameter to \b NULL if no grouping is required. 
		) const
	{
		if (!m_bWorking)
			return;
		if (dumpGroup)
			::RaiseException(CrashRpt::ExceptionAssertionViolated, 0, 1, reinterpret_cast<ULONG_PTR*>(&dumpGroup));
		else
			::RaiseException(CrashRpt::ExceptionAssertionViolated, 0, 0, NULL);
	}

private:
	bool LoadDll(LPCWSTR crashrptPath = NULL) throw()
	{
		m_bLoaded = false;
		m_bWorking = false;
		m_bSkipAssertsAdded = false;
		m_InitCrashRpt = NULL;
		m_SendReport = NULL;
		m_IsReadyToExit = NULL;
		m_SetCustomInfo = NULL;
		m_AddUserInfoToReport = NULL;
		m_RemoveUserInfoFromReport = NULL;
		m_AddFileToReport = NULL;
		m_RemoveFileFromReport = NULL;
		m_GetVersionFromApp = NULL;
		m_GetVersionFromFile = NULL;

		// hCrashrptDll should not be unloaded, crash may appear even after return from main().
		// So hCrashrptDll is not saved after construction.
		HMODULE hCrashrptDll = ::LoadLibraryW(crashrptPath ? crashrptPath : L"crashrpt.dll");
		if (hCrashrptDll != NULL)
		{
			m_InitCrashRpt = (pfnInitCrashRpt) GetProcAddress(hCrashrptDll, "InitCrashRpt");
			m_SendReport = (pfnSendReport) GetProcAddress(hCrashrptDll, "SendReport");
			m_IsReadyToExit = (pfnIsReadyToExit) GetProcAddress(hCrashrptDll, "IsReadyToExit");
			m_SetCustomInfo = (pfnSetCustomInfo) GetProcAddress(hCrashrptDll, "SetCustomInfo");
            m_AddUserInfoToReport = (pfnAddUserInfoToReport) GetProcAddress(hCrashrptDll, "AddUserInfoToReport");
			m_RemoveUserInfoFromReport = (pfnRemoveUserInfoFromReport) GetProcAddress(hCrashrptDll, "RemoveUserInfoFromReport");
			m_AddFileToReport = (pfnAddFileToReport) GetProcAddress(hCrashrptDll, "AddFileToReport");
			m_RemoveFileFromReport = (pfnRemoveFileFromReport) GetProcAddress(hCrashrptDll, "RemoveFileFromReport");
			m_GetVersionFromApp = (pfnGetVersionFromApp) GetProcAddress(hCrashrptDll, "GetVersionFromApp");
			m_GetVersionFromFile = (pfnGetVersionFromFile) GetProcAddress(hCrashrptDll, "GetVersionFromFile");

			m_bLoaded = m_InitCrashRpt
				&& m_SendReport
				&& m_IsReadyToExit
				&& m_SetCustomInfo
                && m_AddUserInfoToReport
				&& m_RemoveUserInfoFromReport
				&& m_AddFileToReport
				&& m_RemoveFileFromReport
				&& m_GetVersionFromApp
				&& m_GetVersionFromFile;
		}

#if _WIN32_WINNT >= 0x0501 /*_WIN32_WINNT_WINXP*/
		// if no crash processing was started, we need to ignore ExceptionAssertionViolated exceptions.
		if (!m_bLoaded)
		{
			::AddVectoredExceptionHandler(TRUE, SkipAsserts);
			m_bSkipAssertsAdded = true;
		}
#endif

		return m_bLoaded;
	}

	static LONG CALLBACK SkipAsserts(EXCEPTION_POINTERS* pExceptionInfo)
	{
		if (pExceptionInfo->ExceptionRecord->ExceptionCode == ExceptionAssertionViolated)
			return EXCEPTION_CONTINUE_EXECUTION;
		return EXCEPTION_CONTINUE_SEARCH;
	}

	bool m_bLoaded;
	bool m_bWorking;
	bool m_bSkipAssertsAdded;

	typedef BOOL (*pfnInitCrashRpt)(ApplicationInfo* applicationInfo, HandlerSettings* handlerSettings, BOOL ownProcess);
	typedef LONG (*pfnSendReport)(EXCEPTION_POINTERS* exceptionPointers);
	typedef BOOL (*pfnIsReadyToExit)();
	typedef void (*pfnSetCustomInfo)(LPCWSTR text);
	typedef void (*pfnAddUserInfoToReport)(LPCWSTR key, LPCWSTR value);
	typedef void (*pfnRemoveUserInfoFromReport)(LPCWSTR key);
	typedef void (*pfnAddFileToReport)(LPCWSTR path, LPCWSTR reportFileName /* = NULL */);
	typedef void (*pfnRemoveFileFromReport)(LPCWSTR path);
	typedef BOOL (*pfnGetVersionFromApp)(ApplicationInfo* appInfo);
	typedef BOOL (*pfnGetVersionFromFile)(LPCWSTR path, ApplicationInfo* appInfo);

	pfnInitCrashRpt m_InitCrashRpt;
	pfnSendReport m_SendReport;
	pfnIsReadyToExit m_IsReadyToExit;
	pfnSetCustomInfo m_SetCustomInfo;
    pfnAddUserInfoToReport m_AddUserInfoToReport;
	pfnRemoveUserInfoFromReport m_RemoveUserInfoFromReport;
	pfnAddFileToReport m_AddFileToReport;
	pfnRemoveFileFromReport m_RemoveFileFromReport;
	pfnGetVersionFromApp m_GetVersionFromApp;
	pfnGetVersionFromFile m_GetVersionFromFile;
};

} // namespace crash_rpt

__declspec(deprecated) typedef crash_rpt::CrashRpt CrashHandler; //!< Deprecated. Use crash_rpt::CrashRpt class instead.
__declspec(deprecated) typedef crash_rpt::ApplicationInfo ApplicationInfo; //!< Deprecated. Use crash_rpt::ApplicationInfo class instead.
__declspec(deprecated) typedef crash_rpt::HandlerSettings HandlerSettings; //!< Deprecated. Use crash_rpt::HandlerSettings class instead.

#endif // __CRASH_RPT_H__