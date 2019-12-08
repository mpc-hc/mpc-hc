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

#include "rar.h"
#include "utils.h"

#define READ_ITEM(item) READ_ITEM2(&item, sizeof(item))

#define READ_ITEM2(item, size) { \
	if (!ReadFile (file, item, size, &read, NULL)) \
	{ \
		ErrorMsg (GetLastError (), L"Could not read RAR header"); \
		return S_FALSE; \
	} \
	if (read < size) return ERROR_HANDLE_EOF; \
	acc += read; }

DWORD ReadHeader (HANDLE file, rar_header_t *dest)
{
	fixed_header_t fh;
	fixed_file_header_t ffh;
	DWORD read, dword;
	LONGLONG acc = 0;

	// Read fixed archive header.
	READ_ITEM(fh);

	dest->ch.crc = fh.crc;
	dest->ch.flags = fh.flags;
	dest->ch.type = fh.type;

	switch (fh.type)
	{
	case HEADER_TYPE_FILE:
		READ_ITEM(ffh);

		dest->ch.size.QuadPart = (LONGLONG) ffh.packedSize + fh.size;
		dest->fh.size.LowPart = ffh.size;
		dest->fh.os = ffh.os;
		dest->fh.crc = ffh.crc;
		dest->fh.timestamp = ffh.timestamp;
		dest->fh.version = ffh.version;
		dest->fh.method = ffh.method;
		dest->fh.name_len = ffh.name_len;
		dest->fh.attributes = ffh.attributes;

		if (fh.flags & LHD_LARGE)
		{
			READ_ITEM(dword); // Packed size high dword
			dest->ch.size.HighPart += dword;
			READ_ITEM(dword); // Unpacked size high dword
			dest->fh.size.HighPart = dword;
		}
		else
			dest->fh.size.HighPart = 0;

		dest->fh.filename = new char [dest->fh.name_len + 1];
		if (!dest->fh.filename)
		{
			ErrorMsg (0, L"Out of memory while reading RAR header.");
			return ERROR_OUTOFMEMORY;
		}
		READ_ITEM2 (dest->fh.filename, dest->fh.name_len);
		dest->fh.filename [dest->fh.name_len] = 0;

		if (acc < fh.size)
		{
			SetFilePointer (file, (LONG) (fh.size - acc), NULL, FILE_CURRENT);
			acc = fh.size;
		}
		break;

	default:
		if (fh.flags & LONG_BLOCK)
		{
			READ_ITEM (dword);
			dest->ch.size.QuadPart = (LONGLONG) dword + fh.size;
		}
		else
		{
			dest->ch.size.HighPart = 0;
			dest->ch.size.LowPart = fh.size;
		}
	}

	if (acc > dest->ch.size.QuadPart)
	{
		ErrorMsg (0, L"Overrun while reading RAR header.");
		return S_FALSE;
	}

	if (fh.type != HEADER_TYPE_FILE && acc < dest->ch.size.QuadPart)
	{
		LARGE_INTEGER li;
		li.QuadPart = dest->ch.size.QuadPart - acc;
		SetFilePointerEx (file, li, NULL, FILE_CURRENT);
		dest->bytesRemaining.QuadPart = 0;
	}
	else
		dest->bytesRemaining.QuadPart = dest->ch.size.QuadPart - acc;

	return ERROR_SUCCESS;
}
