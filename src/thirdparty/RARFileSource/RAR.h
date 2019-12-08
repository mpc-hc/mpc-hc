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

#ifndef RAR_H
#define RAR_H

#include "RAR_defines.h"

#define HEADER_TYPE_MARKER		0x72
#define HEADER_TYPE_ARCHIVE		0x73
#define HEADER_TYPE_FILE		0x74
#define HEADER_TYPE_SUBBLOCK	0x77
#define HEADER_TYPE_RECOVERY	0x78
#define HEADER_TYPE_NEWSUBLOCK	0x7a
#define HEADER_TYPE_END			0x7b

#pragma pack (push, 1)

// Fixed size parts of the in file headers.

typedef struct
{
	WORD crc;
	BYTE type;
	WORD flags;
	WORD size;
} fixed_header_t;

typedef struct
{
	DWORD packedSize;
	DWORD size;
	BYTE  os;
	DWORD crc;
	DWORD timestamp;
	BYTE  version;
	BYTE  method;
	WORD  name_len;
	DWORD attributes;
} fixed_file_header_t;

#pragma pack (pop)

// In memory representation

typedef struct
{
	WORD crc;
	BYTE type;
	WORD flags;
	LARGE_INTEGER size;
} common_header_t;

typedef struct
{
	WORD reserved1;
	DWORD reserved2;
} archive_header_t;

typedef struct
{
	LARGE_INTEGER size;

	DWORD   crc;
	DWORD   timestamp;
	DWORD   timestamp_ext;
	DWORD   attributes;
	DWORD   name_len;

	char    *filename;

	BYTE    os;
	BYTE    version;
	BYTE    method;

} file_header_t;

typedef struct
{
	DWORD crc;
	WORD  volume_number;
} end_header_t;

typedef struct
{
	common_header_t ch;

	union
	{
		archive_header_t ah;
		file_header_t fh;
		end_header_t eh;
	};

	LARGE_INTEGER bytesRemaining;
} rar_header_t;

DWORD ReadHeader (HANDLE file, rar_header_t *dest);

#endif // RAR_H
