#pragma once

#pragma warning(disable : 4200)

#include <atlbase.h>
#include <atlcoll.h>
//#include <winioctl.h> // platform sdk
#include <winddk\ntddcdvd.h>

class CDVDSession
{
protected:
	HANDLE m_hDrive;

	DVD_SESSION_ID m_session;
	bool BeginSession();
	void EndSession();

	BYTE m_SessionKey[5];
	bool Authenticate();

	BYTE m_DiscKey[6], m_TitleKey[6];
	bool GetDiscKey();
	bool GetTitleKey(int lba, BYTE* pKey);

public:
	CDVDSession();
	virtual ~CDVDSession();

	bool Open(LPCTSTR path);
	void Close();

	operator HANDLE() {return m_hDrive;}
	operator DVD_SESSION_ID() {return m_session;}

	bool SendKey(DVD_KEY_TYPE KeyType, BYTE* pKeyData);
	bool ReadKey(DVD_KEY_TYPE KeyType, BYTE* pKeyData, int lba = 0);
};

class CLBAFile : private CFile
{
public:
	CLBAFile();
	virtual ~CLBAFile();

	bool IsOpen();

	bool Open(LPCTSTR path);
	void Close();

	int GetLength();
	int GetPosition();
	int Seek(int lba);
	bool Read(BYTE* buff);
};

class CVobFile : public CDVDSession
{
	// all files
	typedef struct {CString fn; int size;} file_t;
	CAtlArray<file_t> m_files;
	int m_iFile;
	int m_pos, m_size, m_offset;

	// currently opened file
	CLBAFile m_file;

	// attribs
	bool m_fDVD, m_fHasDiscKey, m_fHasTitleKey;

public:
	CVobFile();
	virtual ~CVobFile();

	bool IsDVD();
	bool HasDiscKey(BYTE* key);
	bool HasTitleKey(BYTE* key);

	bool Open(CString fn, CAtlList<CString>& files /* out */); // vts ifo
	bool Open(CAtlList<CString>& files, int offset = -1); // vts vobs, video vob offset in lba
	void Close();

	int GetLength();
	int GetPosition();
	int Seek(int pos);
	bool Read(BYTE* buff);
};
