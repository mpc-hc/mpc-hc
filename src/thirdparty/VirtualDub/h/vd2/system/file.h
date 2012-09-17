//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2004 Avery Lee, All Rights Reserved.
//
//	Beginning with 1.6.0, the VirtualDub system library is licensed
//	differently than the remainder of VirtualDub.  This particular file is
//	thus licensed as follows (the "zlib" license):
//
//	This software is provided 'as-is', without any express or implied
//	warranty.  In no event will the authors be held liable for any
//	damages arising from the use of this software.
//
//	Permission is granted to anyone to use this software for any purpose,
//	including commercial applications, and to alter it and redistribute it
//	freely, subject to the following restrictions:
//
//	1.	The origin of this software must not be misrepresented; you must
//		not claim that you wrote the original software. If you use this
//		software in a product, an acknowledgment in the product
//		documentation would be appreciated but is not required.
//	2.	Altered source versions must be plainly marked as such, and must
//		not be misrepresented as being the original software.
//	3.	This notice may not be removed or altered from any source
//		distribution.

#ifndef f_VD2_SYSTEM_FILE_H
#define f_VD2_SYSTEM_FILE_H

#ifdef _MSC_VER
	#pragma once
#endif

#include <limits.h>
#include <stdarg.h>
#include <vd2/system/vdtypes.h>
#include <vd2/system/vdalloc.h>
#include <vd2/system/vdstl.h>

#ifdef WIN32
	typedef void *VDFileHandle;				// this needs to match wtypes.h definition for HANDLE
#else
	#error No operating system target declared??
#endif

namespace nsVDFile {
	enum eSeekMode {
		kSeekStart=0, kSeekCur, kSeekEnd
	};

	enum eFlags {
		kRead			= 0x00000001,
		kWrite			= 0x00000002,
		kReadWrite		= kRead | kWrite,

		kDenyNone		= 0x00000000,
		kDenyRead		= 0x00000010,
		kDenyWrite		= 0x00000020,
		kDenyAll		= kDenyRead | kDenyWrite,

		kOpenExisting		= 0x00000100,
		kOpenAlways			= 0x00000200,
		kCreateAlways		= 0x00000300,
		kCreateNew			= 0x00000400,
		kTruncateExisting	= 0x00000500,		// not particularly useful, really
		kCreationMask		= 0x0000FF00,

		kSequential			= 0x00010000,
		kRandomAccess		= 0x00020000,
		kUnbuffered			= 0x00040000,		// much faster on Win32 thanks to the crappy cache, but possibly bad in Unix?
		kWriteThrough		= 0x00080000,

		kAllFileFlags		= 0xFFFFFFFF
	};
};

class VDFile {
protected:
	VDFileHandle	mhFile;
	vdautoptr2<wchar_t>	mpFilename;
	sint64			mFilePosition;

private:
	VDFile(const VDFile&);
	const VDFile& operator=(const VDFile& f);

public:
	VDFile() : mhFile(NULL) {}
	VDFile(const char *pszFileName, uint32 flags = nsVDFile::kRead | nsVDFile::kDenyWrite | nsVDFile::kOpenExisting);
	VDFile(const wchar_t *pwszFileName, uint32 flags = nsVDFile::kRead | nsVDFile::kDenyWrite | nsVDFile::kOpenExisting);
	VDFile(VDFileHandle h);
	~VDFile();

	// The "NT" functions are non-throwing and return success/failure; the regular functions throw exceptions
	// when something bad happens.

	void	open(const char *pszFileName, uint32 flags = nsVDFile::kRead | nsVDFile::kDenyWrite | nsVDFile::kOpenExisting);
	void	open(const wchar_t *pwszFileName, uint32 flags = nsVDFile::kRead | nsVDFile::kDenyWrite | nsVDFile::kOpenExisting);

	bool	openNT(const wchar_t *pwszFileName, uint32 flags = nsVDFile::kRead | nsVDFile::kDenyWrite | nsVDFile::kOpenExisting);

	bool	closeNT();
	void	close();
	bool	truncateNT();
	void	truncate();

	// extendValid() pushes the valid threshold of a file out, so that the system allocates
	// space for a file without ensuring that it is cleared.  It is mainly useful for
	// preallocating a file without waiting for the system to clear all of it.  The caveats:
	//
	// - only required on NTFS
	// - requires Windows XP or Windows Server 2003
	// - does not work on compressed or sparse files
	//
	// As such, it shouldn't normally be relied upon, and extendValidNT() should be the call
	// of choice.
	//
	// enableExtendValid() must be called beforehand, as SeVolumeNamePrivilege must be
	// enabled on the process before the file is opened!

	bool	extendValidNT(sint64 pos);
	void	extendValid(sint64 pos);
	static bool enableExtendValid();

	sint64	size();
	void	read(void *buffer, long length);
	long	readData(void *buffer, long length);
	void	write(const void *buffer, long length);
	long	writeData(const void *buffer, long length);
	bool	seekNT(sint64 newPos, nsVDFile::eSeekMode mode = nsVDFile::kSeekStart);
	void	seek(sint64 newPos, nsVDFile::eSeekMode mode = nsVDFile::kSeekStart);
	bool	skipNT(sint64 delta);
	void	skip(sint64 delta);
	sint64	tell();

	bool	flushNT();
	void	flush();

	bool	isOpen();
	VDFileHandle	getRawHandle();

	const wchar_t *getFilenameForError() const { return mpFilename; }

	// unbuffered I/O requires aligned buffers ("unbuffers")
	static void *AllocUnbuffer(size_t nBytes);
	static void FreeUnbuffer(void *p);

protected:
	bool	open_internal(const char *pszFilename, const wchar_t *pwszFilename, uint32 flags, bool throwOnError);
};

///////////////////////////////////////////////////////////////////////////

template<class T>
class VDFileUnbufferAllocator {
public:
	typedef	size_t		size_type;
	typedef	ptrdiff_t	difference_type;
	typedef	T*			pointer;
	typedef	const T*	const_pointer;
	typedef	T&			reference;
	typedef	const T&	const_reference;
	typedef	T			value_type;

	template<class U> struct rebind { typedef VDFileUnbufferAllocator<U> other; };

	pointer			address(reference x) const			{ return &x; }
	const_pointer	address(const_reference x) const	{ return &x; }

	pointer			allocate(size_type n, void *p = 0)	{ return (pointer)VDFile::AllocUnbuffer(n * sizeof(T)); }
	void			deallocate(pointer p, size_type n)	{ VDFile::FreeUnbuffer(p); }
	size_type		max_size() const throw()			{ return INT_MAX; }

	void			construct(pointer p, const T& val)	{ new((void *)p) T(val); }
	void			destroy(pointer p)					{ ((T*)p)->~T(); }

#if defined(_MSC_VER) && _MSC_VER < 1300
	char *			_Charalloc(size_type n)				{ return (char *)allocate((n + sizeof(T) - 1) / sizeof(T)); }
#endif
};

///////////////////////////////////////////////////////////////////////////

class IVDStream {
public:
	virtual const wchar_t *GetNameForError() = 0;
	virtual sint64	Pos() = 0;
	virtual void	Read(void *buffer, sint32 bytes) = 0;
	virtual sint32	ReadData(void *buffer, sint32 bytes) = 0;
	virtual void	Write(const void *buffer, sint32 bytes) = 0;
};

class IVDRandomAccessStream : public IVDStream {
public:
	virtual sint64	Length() = 0;
	virtual void	Seek(sint64 offset) = 0;
};

class VDFileStream : public VDFile, public IVDRandomAccessStream {
private:
	VDFileStream(const VDFile&);
	const VDFileStream& operator=(const VDFileStream& f);

public:
	VDFileStream() {}
	VDFileStream(const char *pszFileName, uint32 flags = nsVDFile::kRead | nsVDFile::kDenyWrite | nsVDFile::kOpenExisting)
		: VDFile(pszFileName, flags) {}
	VDFileStream(const wchar_t *pwszFileName, uint32 flags = nsVDFile::kRead | nsVDFile::kDenyWrite | nsVDFile::kOpenExisting)
		: VDFile(pwszFileName, flags) {}
	VDFileStream(VDFileHandle h) : VDFile(h) {}
	~VDFileStream();

	const wchar_t *GetNameForError();
	sint64	Pos();
	void	Read(void *buffer, sint32 bytes);
	sint32	ReadData(void *buffer, sint32 bytes);
	void	Write(const void *buffer, sint32 bytes);
	sint64	Length();
	void	Seek(sint64 offset);
};

class VDMemoryStream : public IVDRandomAccessStream {
public:
	VDMemoryStream(const void *pSrc, uint32 len);

	const wchar_t *GetNameForError();
	sint64	Pos();
	void	Read(void *buffer, sint32 bytes);
	sint32	ReadData(void *buffer, sint32 bytes);
	void	Write(const void *buffer, sint32 bytes);
	sint64	Length();
	void	Seek(sint64 offset);

protected:
	const char		*mpSrc;
	const uint32	mLength;
	uint32			mPos;
};

class VDBufferedStream : public IVDRandomAccessStream {
public:
	VDBufferedStream(IVDRandomAccessStream *pSrc, uint32 bufferSize);
	~VDBufferedStream();

	const wchar_t *GetNameForError();
	sint64	Pos();
	void	Read(void *buffer, sint32 bytes);
	sint32	ReadData(void *buffer, sint32 bytes);
	void	Write(const void *buffer, sint32 bytes);

	sint64	Length();
	void	Seek(sint64 offset);

	void	Skip(sint64 size);

protected:
	IVDRandomAccessStream *mpSrc;
	vdblock<char>	mBuffer;
	sint64		mBasePosition;
	uint32		mBufferOffset;
	uint32		mBufferValidSize;
};

class VDTextStream {
public:
	VDTextStream(IVDStream *pSrc);
	~VDTextStream();

	const char *GetNextLine();

protected:
	IVDStream	*mpSrc;
	uint32		mBufferPos;
	uint32		mBufferLimit;
	enum {
		kFetchLine,
		kEatNextIfCR,
		kEatNextIfLF
	} mState;

	enum {
		kFileBufferSize = 4096
	};

	vdfastvector<char>	mLineBuffer;
	vdblock<char>		mFileBuffer;
};

class VDTextInputFile {
public:
	VDTextInputFile(const wchar_t *filename, uint32 flags = nsVDFile::kOpenExisting);
	~VDTextInputFile();

	inline const char *GetNextLine() {
		return mTextStream.GetNextLine();
	}

protected:
	VDFileStream	mFileStream;
	VDTextStream	mTextStream;
};

class VDTextOutputStream {
public:
	VDTextOutputStream(IVDStream *stream);
	~VDTextOutputStream();

	void Flush();

	void Write(const char *s);
	void Write(const char *s, int len);
	void PutLine();
	void PutLine(const char *s);
	void Format(const char *format, ...);
	void FormatLine(const char *format, ...);

protected:
	void Format2(const char *format, va_list val);
	void PutData(const char *s, int len);

	enum { kBufSize = 4096 };

	int			mLevel;
	IVDStream	*mpDst;
	char		mBuf[kBufSize];
};

#endif
