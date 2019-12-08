/*
 * Copyright (C) 2008-2012, OctaneSnail <os@v12pwr.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <windows.h>
#include <streams.h>
#include <strsafe.h>

#include "Utils.h"
#include "RFS.h"

void ErrorMsg (DWORD errorCode, const wchar_t *format, ...)
{
	wchar_t buffer [1024];
	wchar_t *end;
	size_t remaining;
	va_list argptr;

//	__asm int 3;

	va_start (argptr, format);
	StringCchVPrintfEx (buffer, 1024, &end, &remaining, 0, format, argptr);
	va_end (argptr);

	if (errorCode && remaining > 9)
	{
		wcscpy (end, L"\n\nError: ");
		end += 9;
		remaining -= 9;

		FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL, errorCode, MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT), end, (DWORD) remaining, NULL);
	}

	DbgLog((LOG_ERROR, 0, L"%s", buffer));
#ifdef STANDALONE_FILTER
	MessageBox (NULL, buffer, RARFileSourceName, MB_OK | MB_ICONERROR);
#endif
}

#ifdef _DEBUG

static const wchar_t *const mhd_flags [] =
{
	L"MHD_VOLUME",
	L"MHD_COMMENT",
	L"MHD_LOCK",
	L"MHD_SOLID",
	L"MHD_NEWNUMBERING",
	L"MHD_AV",
	L"MHD_PROTECT",
	L"MHD_PASSWORD",
	L"MHD_FIRSTVOLUME",
	L"MHD_ENCRYPTVER"
};

static const wchar_t *const lhd_flags1 [] =
{
	L"LHD_SPLIT_BEFORE",
	L"LHD_SPLIT_AFTER",
	L"LHD_PASSWORD",
	L"LHD_COMMENT",
	L"LHD_SOLID"
};

static const wchar_t *const lhd_flags2 [] =
{
	L"LHD_LARGE",
	L"LHD_UNICODE",
	L"LHD_SALT",
	L"LHD_VERSION",
	L"LHD_EXTTIME",
	L"LHD_EXTFLAGS"
};

static const wchar_t *const earc_flags [] =
{
	L"EARC_NEXT_VOLUME",
	L"EARC_DATACRC",
	L"EARC_REVSPACE",
	L"EARC_VOLNUMBER"
};

void LogHeader (rar_header_t *rh)
{
	int i;
	WORD flags = rh->ch.flags;

	DbgLog((LOG_TRACE, 2, L"Header CRC %04hx  TYPE %02x  FLAGS %04hx  SIZE %08lx %08lx",
		rh->ch.crc, rh->ch.type, rh->ch.flags, rh->ch.size.HighPart, rh->ch.size.LowPart));

	switch (rh->ch.type)
	{
	case HEADER_TYPE_MARKER:
		DbgLog((LOG_TRACE, 2, L"  HEADER_TYPE_MARKER"));
		break;

	case HEADER_TYPE_ARCHIVE:
		DbgLog((LOG_TRACE, 2, L"  HEADER_TYPE_ARCHIVE"));

		for (i = 0; i < 10; i ++)
		{
			if (flags & 1)
				DbgLog((LOG_TRACE, 2, L"  %s", mhd_flags [i]));

			flags >>= 1;
		}
		break;

	case HEADER_TYPE_FILE:
		DbgLog((LOG_TRACE, 2, L"  HEADER_TYPE_FILE"));

		for (i = 0; i < 5; i ++)
		{
			if (flags & 1)
				DbgLog((LOG_TRACE, 2, L"  %s", lhd_flags1 [i]));

			flags >>= 1;
		}

		flags >>= 3;

		for (i = 0; i < 6; i ++)
		{
			if (flags & 1)
				DbgLog((LOG_TRACE, 2, L"  %s", lhd_flags2 [i]));

			flags >>= 1;
		}
		break;

	case HEADER_TYPE_SUBBLOCK:
		DbgLog((LOG_TRACE, 2, L"  HEADER_TYPE_SUBBLOCK"));
		break;

	case HEADER_TYPE_RECOVERY:
		DbgLog((LOG_TRACE, 2, L"  HEADER_TYPE_RECOVERY"));
		break;

	case HEADER_TYPE_NEWSUBLOCK:
		DbgLog((LOG_TRACE, 2, L"  HEADER_TYPE_NEWSUBLOCK"));
		break;

	case HEADER_TYPE_END:
		DbgLog((LOG_TRACE, 2, L"  HEADER_TYPE_END"));

		for (i = 0; i < 4; i ++)
		{
			if (flags & 1)
				DbgLog((LOG_TRACE, 2, L"  %s", earc_flags [i]));

			flags >>= 1;
		}
		break;

	default:
		DbgLog((LOG_TRACE, 2, L"  HEADER_TYPE_UNKNOWN"));
	}

	if (rh->ch.flags & SKIP_IF_UNKNOWN)
		DbgLog((LOG_TRACE, 2, L"  SKIP_IF_UNKNOWN"));

	if (rh->ch.flags & LONG_BLOCK)
		DbgLog((LOG_TRACE, 2, L"  LONG_BLOCK"));
}

#endif // _DEBUG
