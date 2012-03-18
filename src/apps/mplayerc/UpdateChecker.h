/*
 * $Id$
 *
 * (C) 2012 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#pragma once

#include <afxwin.h>

struct Version
{
	UINT major;
	UINT minor;
	UINT patch;
	UINT revision;
};

class UpdateChecker
{
public:
	UpdateChecker(CString versionFileURL);
	~UpdateChecker(void);

	bool isUpdateAvailable(const Version& currentVersion);
	bool isUpdateAvailable();
	const Version& getLatestVersion() const { return latestVersion; };

private :
	static const Version MPC_VERSION;

	CString versionFileURL;
	Version latestVersion;

	bool parseVersion(const CString& versionStr);
	bool isHigherVersion(const Version& v1, const Version& v2) const;
};
