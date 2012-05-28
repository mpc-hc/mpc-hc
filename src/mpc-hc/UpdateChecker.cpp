/*
 * $Id$
 *
 * (C) 2012 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "stdafx.h"
#include "Version.h"
#include "UpdateChecker.h"
#include "UpdateCheckerDlg.h"
#include "SettingsDefines.h"
#include "mplayerc.h"

#include <afxinet.h>

const Version UpdateChecker::MPC_HC_VERSION = { MPC_VERSION_NUM };
const LPCTSTR UpdateChecker::MPC_HC_UPDATE_URL = _T("http://mpc-hc.sourceforge.net/version.txt");

UpdateChecker::UpdateChecker(CString versionFileURL)
	: versionFileURL(versionFileURL)
{
}

UpdateChecker::~UpdateChecker(void)
{
}

Update_Status UpdateChecker::IsUpdateAvailable(const Version& currentVersion)
{
	Update_Status updateAvailable = UPDATER_LATEST_STABLE;

	try {
		CInternetSession internet;
		CHttpFile* versionFile = (CHttpFile*) internet.OpenURL(versionFileURL,
															   1,
															   INTERNET_FLAG_TRANSFER_ASCII | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD,
															   NULL,
															   0);

		if (versionFile) {
			CString latestVersionStr;
			char buffer[101];
			UINT br = 0;

			while ((br = versionFile->Read(buffer, 50)) > 0) {
				buffer[br] = '\0';
				latestVersionStr += buffer;
			}

			if (!ParseVersion(latestVersionStr, latestVersion)) {
				updateAvailable = UPDATER_ERROR;
			} else {
				time_t lastCheck = time(NULL);
				AfxGetApp()->WriteProfileBinary(IDS_R_SETTINGS, IDS_RS_UPDATER_LAST_CHECK, (LPBYTE)&lastCheck, sizeof(time_t));

				int comp = CompareVersion(currentVersion, latestVersion);

				if (comp < 0) {
					CString ignoredVersionStr = AfxGetApp()->GetProfileString(IDS_R_SETTINGS, IDS_RS_UPDATER_IGNORE_VERSION, _T("0.0.0.0"));
					Version ignoredVersion;
					bool ignored = false;

					if (ParseVersion(ignoredVersionStr, ignoredVersion)) {
						ignored = (CompareVersion(ignoredVersion, latestVersion) >= 0);
					}

					updateAvailable = ignored ? UPDATER_UPDATE_AVAILABLE_IGNORED : UPDATER_UPDATE_AVAILABLE;
				} else if (comp > 0) {
					updateAvailable = UPDATER_NEWER_VERSION;
				}
			}

			delete versionFile;
		} else {
			updateAvailable = UPDATER_ERROR;
		}
	} catch (CInternetException* pEx) {
		updateAvailable = UPDATER_ERROR;
		pEx->Delete();
	}

	return updateAvailable;
}

Update_Status UpdateChecker::IsUpdateAvailable()
{
	return IsUpdateAvailable(MPC_HC_VERSION);
}

void UpdateChecker::IgnoreLatestVersion()
{
	CString ignoredVersionStr;
	ignoredVersionStr.Format(_T("%u.%u.%u.%u"), latestVersion.major, latestVersion.minor, latestVersion.patch, latestVersion.revision);

	AfxGetApp()->WriteProfileString(IDS_R_SETTINGS, IDS_RS_UPDATER_IGNORE_VERSION, ignoredVersionStr);
}

bool UpdateChecker::ParseVersion(const CString& versionStr, Version& version)
{
	bool success = false;

	if (!versionStr.IsEmpty()) {
		UINT v[4];
		int curPos = 0;
		UINT i = 0;
		CString resToken = versionStr.Tokenize(_T("."), curPos);

		success = !resToken.IsEmpty();

		while (!resToken.IsEmpty() && i < _countof(v) && success) {
			if (1 != _stscanf_s(resToken, _T("%u"), v + i)) {
				success = false;
			}

			resToken = versionStr.Tokenize(_T("."), curPos);
			i++;
		}

		success = success && (i == _countof(v));

		if (success) {
			version.major = v[0];
			version.minor = v[1];
			version.patch = v[2];
			version.revision = v[3];
		}
	}

	return success;
}

int UpdateChecker::CompareVersion(const Version& v1, const Version& v2)
{
	if (v1.major > v2.major) {
		return 1;
	} else if (v1.major < v2.major) {
		return -1;
	} else if (v1.minor > v2.minor) {
		return 1;
	} else if (v1.minor < v2.minor) {
		return -1;
	} else if (v1.patch > v2.patch) {
		return 1;
	} else if (v1.patch < v2.patch) {
		return -1;
	} else if (v1.revision > v2.revision) {
		return 1;
	} else if (v1.revision < v2.revision) {
		return -1;
	} else {
		return 0;
	}
}

bool UpdateChecker::IsAutoUpdateEnabled()
{
	int& status = AfxGetAppSettings().nUpdaterAutoCheck;

	if (status == AUTOUPDATE_UNKNOWN) { // First run
		status = (AfxMessageBox(IDS_UPDATE_CONFIG_AUTO_CHECK, MB_YESNO) == IDYES) ? AUTOUPDATE_ENABLE : AUTOUPDATE_DISABLE;
	}

	return (status == AUTOUPDATE_ENABLE);
}

bool UpdateChecker::IsTimeToAutoUpdate()
{
	time_t *lastCheck = NULL;
	UINT nRead;

	if (!AfxGetApp()->GetProfileBinary(IDS_R_SETTINGS, IDS_RS_UPDATER_LAST_CHECK, (LPBYTE*)&lastCheck, &nRead) || nRead != sizeof(time_t)) {
		if (lastCheck) {
			delete[] lastCheck;
		}

		return true;
	}

	bool isTimeToAutoUpdate = (time(NULL) >= *lastCheck + AfxGetAppSettings().nUpdaterDelay*24*3600);

	delete[] lastCheck;

	return isTimeToAutoUpdate;
}

UINT RunCheckForUpdateThread(LPVOID pParam)
{
	bool autoCheck = !!pParam;

	if (!autoCheck || UpdateChecker::IsTimeToAutoUpdate()) {
		UpdateChecker updateChecker(UpdateChecker::MPC_HC_UPDATE_URL);

		Update_Status status = updateChecker.IsUpdateAvailable();

		if (!autoCheck || status == UPDATER_UPDATE_AVAILABLE) {
			UpdateCheckerDlg dlg(status, updateChecker.GetLatestVersion());

			if (dlg.DoModal() == IDC_UPDATE_IGNORE_BUTTON) {
				updateChecker.IgnoreLatestVersion();
			}
		}
	}

	return 0;
}

void UpdateChecker::CheckForUpdate(bool autoCheck /*= false*/)
{
	AfxBeginThread(RunCheckForUpdateThread, (LPVOID)autoCheck);
}
