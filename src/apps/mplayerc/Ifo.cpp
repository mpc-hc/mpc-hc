/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
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

#include "stdafx.h"
#include "Ifo.h"


#ifdef WORDS_BIGENDIAN
#define bswap_16(x) (x)
#define bswap_32(x) (x)
#define bswap_64(x) (x)
#else

// code from bits/byteswap.h (C) 1997, 1998 Free Software Foundation, Inc.
#define bswap_16(x) \
     ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))

// code from bits/byteswap.h (C) 1997, 1998 Free Software Foundation, Inc.
#define bswap_32(x) \
     ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | \
      (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#define bswap_64(x) \
     (__extension__						\
      ({ union { __extension__ unsigned long long int __ll;	\
                 unsigned long int __l[2]; } __w, __r;		\
         __w.__ll = (x);					\
         __r.__l[0] = bswap_32 (__w.__l[1]);			\
         __r.__l[1] = bswap_32 (__w.__l[0]);			\
         __r.__ll; }))
#endif

#ifdef WORDS_BIGENDIAN
#define be2me_16(x) (x)
#define be2me_32(x) (x)
#define be2me_64(x) (x)
#define le2me_16(x) bswap_16(x)
#define le2me_32(x) bswap_32(x)
#define le2me_64(x) bswap_64(x)
#else
#define be2me_16(x) bswap_16(x)
#define be2me_32(x) bswap_32(x)
#define be2me_64(x) bswap_64(x)
#define le2me_16(x) (x)
#define le2me_32(x) (x)
#define le2me_64(x) (x)
#endif

#define DVD_VIDEO_LB_LEN	2048
#define IFO_HDR_LEN			   8
#define LU_SUB_LEN			   8

extern HANDLE (__stdcall * Real_CreateFileW)(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);


unsigned __int32 get4bytes (const BYTE* buf)
{
	return be2me_32 (*((unsigned __int32 *)buf));
}


// VMG files
#define OFF_VMGM_PGCI_UT(buf)        get4bytes (buf + 200)

// VTS files
#define OFF_VTSM_PGCI_UT(buf)        get4bytes (buf + 208)
#define OFF_VTS_PGCIT(buf)           get4bytes (buf + 204)


CIfo::CIfo()
{
	m_pBuffer	= NULL;
	m_pPGCI		= NULL;
	m_pPGCIT	= NULL;
	m_dwSize	= 0;
}


int CIfo::GetMiscPGCI (CIfo::ifo_hdr_t *hdr, int title, uint8_t **ptr)
{
	pgci_sub_t *pgci_sub;

	*ptr	  = (uint8_t *) hdr;
	*ptr	 += IFO_HDR_LEN;
	pgci_sub  = (pgci_sub_t *) *ptr + title;

	*ptr = (uint8_t *) hdr + be2me_32 (pgci_sub->start);

	return 0;
}

void CIfo::RemovePgciUOPs (uint8_t *ptr)
{
	ifo_hdr_t*	hdr = (ifo_hdr_t *) ptr;
	uint16_t	num;
	int			i;

	ptr += IFO_HDR_LEN;
	num  = be2me_16(hdr->num);

	for (i=1; i<=num; i++) {
		lu_sub_t *lu_sub = (lu_sub_t *) ptr;

		ptr += LU_SUB_LEN;
	}

	for (i=0; i<be2me_16(hdr->num); i++) {
		uint8_t *ptr;

		if (GetMiscPGCI (hdr, i, &ptr) >= 0)
		{
			pgc_t*		pgc = (pgc_t*) ptr;
			pgc->prohibited_ops = 0;
		}
	}
}

CIfo::pgc_t* CIfo::GetFirstPGC()
{
	if (m_pBuffer)
		return (pgc_t*) (m_pBuffer + 0x0400);
	else
		return NULL;
}

CIfo::pgc_t* CIfo::GetPGCI(const int title, const ifo_hdr_t* hdr)
{
	CIfo::pgci_sub_t *pgci_sub;
	uint8_t *ptr;

	ptr = (uint8_t *) hdr;
	ptr += IFO_HDR_LEN;

	pgci_sub = (pgci_sub_t *) ptr + title;

	ptr = (uint8_t *) hdr + be2me_32 (pgci_sub->start);

	/* jdw */
	if ( ptr >= ( (uint8_t *) hdr + be2me_32 ( hdr->len )))
	{
		return NULL ;
	}
	/* /jdw */

	return (pgc_t *) ptr;
}


bool CIfo::IsVTS()
{
	if (m_dwSize<12 || (strncmp ((char*)m_pBuffer, "DVDVIDEO-VTS", 12)!=0)) 
		return false;

	return true;
}


bool CIfo::IsVMG()
{
	if (m_dwSize<12 || (strncmp ((char*)m_pBuffer, "DVDVIDEO-VMG", 12)!=0))
		return false;

	return true;
}

bool CIfo::OpenFile (LPCTSTR strFile)
{
	bool	bRet = false;
	HANDLE	hFile;

	hFile	 = Real_CreateFileW((LPTSTR) strFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT (hFile != INVALID_HANDLE_VALUE);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD		dwSize = GetFileSize (hFile, NULL);
		m_pBuffer = DNew BYTE [dwSize];
		ReadFile (hFile, m_pBuffer, dwSize, &m_dwSize, NULL);
		CloseHandle (hFile);

		if (IsVTS() && (OFF_VTSM_PGCI_UT(m_pBuffer)!=0))
		{
			m_pPGCI  = (ifo_hdr_t*)(m_pBuffer + OFF_VTSM_PGCI_UT(m_pBuffer) * DVD_VIDEO_LB_LEN);
			m_pPGCIT = (ifo_hdr_t*)(m_pBuffer + OFF_VTS_PGCIT(m_pBuffer)    * DVD_VIDEO_LB_LEN);			
		}
		else if (IsVMG() && (OFF_VMGM_PGCI_UT(m_pBuffer)!=0))
			m_pPGCI = (ifo_hdr_t*)(m_pBuffer + OFF_VMGM_PGCI_UT(m_pBuffer) * DVD_VIDEO_LB_LEN);

		bRet = (m_pPGCI != NULL);
	}

	return bRet;
}

bool CIfo::RemoveUOPs()
{
	pgc_t*	pgc;

	if (m_pPGCI)
	{
		pgc	= GetFirstPGC();
		pgc->prohibited_ops = 0;

		for (int i=0; i<be2me_16(m_pPGCI->num); i++)
		{
			if (pgc = GetPGCI(i, m_pPGCI))
				RemovePgciUOPs ((uint8_t*)pgc);
		}
	}
	if (m_pPGCIT)
	{
		for (int i=0; i<be2me_16(m_pPGCIT->num); i++)
		{
			if (pgc = GetPGCI(i, m_pPGCIT))
				pgc->prohibited_ops = 0;
		}
	}
	return true;
}

bool CIfo::SaveFile (LPCTSTR strFile)
{
	bool	bRet = false;
	HANDLE	m_hFile;

	if (m_pBuffer)
	{
		m_hFile	 = Real_CreateFileW((LPTSTR) strFile, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
									NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		ASSERT (m_hFile != INVALID_HANDLE_VALUE);

		if (m_hFile != INVALID_HANDLE_VALUE)
		{
			DWORD		dwSize;
			WriteFile (m_hFile, m_pBuffer, m_dwSize, &dwSize, NULL);
			CloseHandle(m_hFile);
			bRet = true;
		}
	}

	return bRet;
}


CIfo::~CIfo(void)
{
	delete[] m_pBuffer;
}
