#include <stdafx.h>
#include <vd2/system/process.h>
#include <vd2/system/VDString.h>
#include <vd2/system/w32assist.h>

void VDLaunchProgram(const wchar_t *path) {
	VDStringW cmdLine(L"\"");

	cmdLine += path;

	if (cmdLine.back() == L'\\')
		cmdLine.push_back(L'\\');

	cmdLine += L"\"";

	PROCESS_INFORMATION processInfo;
	const DWORD createFlags = CREATE_NEW_PROCESS_GROUP | CREATE_DEFAULT_ERROR_MODE;
	BOOL success;

	if (VDIsWindowsNT()) {
		STARTUPINFOW startupInfoW = { sizeof(STARTUPINFOW) };
		startupInfoW.dwFlags = STARTF_USESHOWWINDOW;
		startupInfoW.wShowWindow = SW_SHOWNORMAL;

		WCHAR winDir[MAX_PATH];
		success = GetWindowsDirectoryW(winDir, MAX_PATH);

		if (success)
			success = CreateProcessW(path, (LPWSTR)cmdLine.c_str(), NULL, NULL, FALSE, createFlags, NULL, winDir, &startupInfoW, &processInfo);
	} else {
		STARTUPINFOA startupInfoA = { sizeof(STARTUPINFOA) };
		startupInfoA.dwFlags = STARTF_USESHOWWINDOW;
		startupInfoA.wShowWindow = SW_SHOWNORMAL;

		const VDStringA& pathA = VDTextWToA(path);
		const VDStringA& cmdLineA = VDTextWToA(cmdLine);
		CHAR winDir[MAX_PATH];

		success = GetWindowsDirectoryA(winDir, MAX_PATH);

		if (success)
			success = CreateProcessA(pathA.c_str(), (LPSTR)cmdLineA.c_str(), NULL, NULL, FALSE, createFlags, NULL, winDir, &startupInfoA, &processInfo);
	}

	if (!success)
		throw MyWin32Error("Unable to launch process: %%s", GetLastError());
}
