#include "stdafx.h"
#include "PixelShaderParameters.h"
#include "../../../mpc-hc/picojson.h"
#include <fstream>

CPixelShaderTexture::CPixelShaderTexture(int pSamplerID, const CString& pFilename) :
	mSamplerID(pSamplerID),
	mFilename(pFilename)
{
}

const CString& CPixelShaderTexture::GetFilename() const
{
	return mFilename;
}

const int CPixelShaderTexture::GetSamplerID() const
{
	return mSamplerID;
}

CPixelShaderParameters::CPixelShaderParameters():
	mFilename(),
	mValidation(true),
	mTextures(),
	mChangeNotification(INVALID_HANDLE_VALUE)
{
}

CPixelShaderParameters::CPixelShaderParameters(const CString& pFilename) :
	mFilename(pFilename),
	mValidation(false),
	mTextures(),
	mChangeNotification(INVALID_HANDLE_VALUE)
{
	LoadParameters();
	InitChangeNotification();
}

CPixelShaderParameters::CPixelShaderParameters(const CPixelShaderParameters& pFrom):
	mFilename(pFrom.mFilename),
	mValidation(false),
	mTextures(pFrom.mTextures),
	mChangeNotification(INVALID_HANDLE_VALUE)
{
	InitChangeNotification();
}

CPixelShaderParameters& CPixelShaderParameters::operator=(const CPixelShaderParameters& pFrom)
{
	if (this != &pFrom)
	{
		StopChangeNotification();

		mFilename = pFrom.mFilename;
		mValidation = false;
		mTextures = pFrom.mTextures;

		InitChangeNotification();
	}
	return *this;
}

CPixelShaderParameters& CPixelShaderParameters::operator=(CPixelShaderParameters&& pFrom)
{
	if (this != &pFrom)
	{
		pFrom.StopChangeNotification();

		mFilename = pFrom.mFilename;
		mTextures = pFrom.mTextures;

		pFrom.mFilename = CString();
		pFrom.mTextures.clear();
		pFrom.mValidation = false;

		mValidation = false;
		InitChangeNotification();
	}
	return *this;
}

CPixelShaderParameters::~CPixelShaderParameters()
{
	StopChangeNotification();
}

void CPixelShaderParameters::LoadParameters()
{
	ClearParameters();

	// Read
	std::ifstream lFile(mFilename, std::ios_base::in | std::ios::binary);
	lFile.seekg(0, std::ios_base::end);
	std::size_t lSize = lFile.tellg();
	lFile.seekg(0, std::ios_base::beg);
	std::vector<char> lData(lSize+1);
	lFile.read(&lData.front(), lSize);
	lData.back() = 0x00;
	lFile.close();

	// Parse
	picojson::value v;
	std::string err;
	picojson::parse(v, &lData.front(), &lData.back(), &err);
	if (!err.empty())
		return;

	if (v.is<picojson::object>() == false)
		return; // ill-formed file
	const auto& lRootObj = v.get<picojson::object>();

	// Read textures
	auto lTexRootIt = lRootObj.find("textures");
	if (lTexRootIt != lRootObj.end())
	{
		if (lTexRootIt->second.is<picojson::array>() == false)
			return; // ill-formed file

		const auto& lTexRootObj = lTexRootIt->second.get<picojson::array>();
		for (const auto& lTex : lTexRootObj)
		{
			if (lTex.is<picojson::object>() == false)
				return; // ill-formed file

			const auto& lTexObj = lTex.get<picojson::object>();
			auto lTexIdIt = lTexObj.find("id");
			auto lTexFilenameIt = lTexObj.find("filename");

			if ((lTexIdIt == lTexObj.end()) || (lTexFilenameIt == lTexObj.end()))
				continue; // ill-formed file

			int lTexId = (int)lTexIdIt->second.get<double>();
			std::string lTexFilename = lTexFilenameIt->second.get<std::string>();

			CPixelShaderTexture lTexture(lTexId, CString(lTexFilename.c_str()));
			mTextures.push_back(lTexture);
		}
	}
}

void CPixelShaderParameters::ClearParameters()
{
	mTextures.clear();
}

bool CPixelShaderParameters::IsInvalidated()
{
	// detects if a file modification notitication was sent to us
	if (mChangeNotification != INVALID_HANDLE_VALUE)
	{
		DWORD lWaitStatus = WaitForSingleObject(mChangeNotification, 0);

		if (lWaitStatus != WAIT_TIMEOUT)
		{
			LoadParameters();
			mValidation = false;
			FindNextChangeNotification(mChangeNotification);
		}
	}

	return (!mValidation);
}

void CPixelShaderParameters::Validate()
{
	mValidation = true;
}

int CPixelShaderParameters::GetTextureCount() const
{
	return(int) mTextures.size();
}

const CPixelShaderTexture& CPixelShaderParameters::GetTexture(int pId) const
{
	return mTextures.at(pId);
}

void CPixelShaderParameters::InitChangeNotification()
{
	CPath lDirname(mFilename);
	lDirname.RemoveFileSpec();

	mChangeNotification = FindFirstChangeNotification(lDirname, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
}

void CPixelShaderParameters::StopChangeNotification()
{
	if (mChangeNotification == INVALID_HANDLE_VALUE)
		return;

	FindCloseChangeNotification(mChangeNotification);
	mChangeNotification = INVALID_HANDLE_VALUE;
}