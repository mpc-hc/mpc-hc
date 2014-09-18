/*
* (C) 2003-2006 Gabest
* (C) 2006-2014 see Authors.txt
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

#include "stdafx.h"
#include <vector>

class CPixelShaderTexture {
public:
	CPixelShaderTexture(int pSamplerID, const CString& pFilename);

	const CString& GetFilename() const;
	const int GetSamplerID() const;

private:
	int mSamplerID;
	CString mFilename;
};

class CPixelShaderParameters {
public:
	CPixelShaderParameters();
	explicit CPixelShaderParameters(const CString& pFilename);
	CPixelShaderParameters(const CPixelShaderParameters& pFrom);
	CPixelShaderParameters(CPixelShaderParameters&&) = delete;
	~CPixelShaderParameters();

	CPixelShaderParameters& operator=(const CPixelShaderParameters& pFrom);
	CPixelShaderParameters& operator=(CPixelShaderParameters&& pFrom);

	bool IsInvalidated();
	void Validate();
	int GetTextureCount() const;
	const CPixelShaderTexture& GetTexture(int pId) const;

private:
	CString mFilename;
	bool mValidation;
	std::vector<CPixelShaderTexture> mTextures;
	HANDLE mChangeNotification;

	void InitChangeNotification();
	void StopChangeNotification();
	void LoadParameters();
	void ClearParameters();
};