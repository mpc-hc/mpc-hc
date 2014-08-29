/*
* (C) 2010-2012 see Authors.txt
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

#pragma once

#include <vector>
#include <string>
#include <d3d9.h>
#include <D3Dcompiler.h>

class CShaderIncludeHandler : public ID3DInclude {
public:

	// ID3DInclude
	STDMETHOD(Open)(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes);
	STDMETHOD(Close)(THIS_ LPCVOID pData);

	CPath GetFullFileName(const CString& pFilename, D3D_INCLUDE_TYPE IncludeType) const;

public:
	CShaderIncludeHandler();
	CShaderIncludeHandler(
		LPCTSTR pSystemDirectory,
		LPCTSTR pSourceFilename);
	virtual ~CShaderIncludeHandler();

	const std::vector<CString>& getIncludedFiles() const;

private:
	// Directory name of the original shader file
	CString m_SourceDirectory;

	// Directory name of the "system-wide" includes
	CString m_SystemDirectory;

	// List of the included files (full paths)
	std::vector<CString> m_IncludedFiles;
};