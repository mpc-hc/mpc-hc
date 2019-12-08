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

#ifndef FILE_H
#define FILE_H

#include "List.h"

class CRFSFilePart
{
public:
	CRFSFilePart (void) : next (NULL), file (INVALID_HANDLE_VALUE),
		in_rar_offset (0), in_file_offset (0), size (0) { }
	~CRFSFilePart (void) { if (file != INVALID_HANDLE_VALUE) CloseHandle (file); }

	CRFSFilePart *next;

	HANDLE file;

	LONGLONG in_rar_offset;
	LONGLONG in_file_offset;
	LONGLONG size;
};

class CRFSFile : public CRFSNode<CRFSFile>
{
public:
	CRFSFile (void) : size (0), parts (0), list (NULL), array (NULL), m_prev_part (NULL), filename (NULL),
		type_known (false), unsupported (false) { }

	~CRFSFile (void)
	{
		CRFSFilePart *fp = list;
		while (fp)
		{
			CRFSFilePart *tmp = fp;
			fp = fp->next;
			delete tmp;
		}
		delete [] array;
		delete [] filename;
	}

	int FindStartPart (LONGLONG position);
	HRESULT SyncRead (LONGLONG llPosition, DWORD lLength, BYTE* pBuffer, LONG *cbActual);

	CMediaType media_type;
	LONGLONG size;
	int parts;

	CRFSFilePart *list;
	CRFSFilePart *array;
	CRFSFilePart *m_prev_part;

	char *filename;
	bool type_known;
	bool unsupported;
};

#endif // FILE_H
