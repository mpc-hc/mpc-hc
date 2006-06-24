#include "stdafx.h"
#include "ISDb.h"
#include "mplayerc.h"

bool hash(LPCTSTR fn, filehash& fh)
{
	CFile f;
	CFileException fe;
	if(!f.Open(fn, CFile::modeRead|CFile::osSequentialScan|CFile::shareDenyNone, &fe))
        return false;

	CPath p(fn);
	p.StripPath();
	fh.name = (LPCTSTR)p;

	fh.size = f.GetLength();
	
	fh.hash = fh.size;
	for(UINT64 tmp = 0, i = 0; i < 65536/sizeof(tmp) && f.Read(&tmp, sizeof(tmp)); fh.hash += tmp, i++);
	f.Seek(max(0, (INT64)fh.size - 65536), CFile::begin);
	for(UINT64 tmp = 0, i = 0; i < 65536/sizeof(tmp) && f.Read(&tmp, sizeof(tmp)); fh.hash += tmp, i++);

	return true;
}

void hash(CPlaylist& pl, CList<filehash>& fhs)
{
	fhs.RemoveAll();

	POSITION pos = pl.GetHeadPosition();
	while(pos)
	{
		CString fn = pl.GetNext(pos).m_fns.GetHead();
		if(AfxGetAppSettings().Formats.FindExt(CPath(fn).GetExtension().MakeLower(), true))
			continue;

		filehash fh;
		if(!hash(fn, fh))
			continue;

		fhs.AddTail(fh);
	}
}

CStringA makeargs(CPlaylist& pl)
{
	CList<filehash> fhs;
	hash(pl, fhs);

	CAtlList<CStringA> args;

	POSITION pos = fhs.GetHeadPosition();
	for(int i = 0; pos; i++)
	{
		filehash& fh = fhs.GetNext(pos);

		CStringA str;
		str.Format("name[%d]=%s&size[%d]=%016I64x&hash[%d]=%016I64x",
			i, UrlEncode(CStringA(fh.name)), 
			i, fh.size,
			i, fh.hash);

		args.AddTail(str);
	}

	return Implode(args, '&');
}

bool OpenUrl(CInternetSession& is, CString url, CStringA& str)
{
	str.Empty();

	try
	{
		CAutoPtr<CStdioFile> f(is.OpenURL(url, 1, INTERNET_FLAG_TRANSFER_BINARY|INTERNET_FLAG_EXISTING_CONNECT));

		char buff[1024];
		for(int len; (len = f->Read(buff, sizeof(buff))) > 0; str += CStringA(buff, len));

		f->Close(); // must close it because the desctructor doesn't seem to do it and we will get an exception when "is" is destroying
	}
	catch(CInternetException* ie)
	{
		ie->Delete();
		return false;
	}

	return true;
}
