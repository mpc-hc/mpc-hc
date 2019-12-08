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

#include "File.h"
#include "Utils.h"

static int compare (const void *pos, const void *part)
{
	if (*((LONGLONG *) pos) < ((CRFSFilePart *) part)->in_file_offset)
		return -1;

	if (*((LONGLONG *) pos) >= ((CRFSFilePart *) part)->in_file_offset + ((CRFSFilePart *) part)->size)
		return 1;

	return 0;
}

int CRFSFile::FindStartPart (LONGLONG position)
{
	if (position > size)
		return -1;

	// Check if the previous lookup up still matches.
	if (m_prev_part && !compare (&position, m_prev_part))
		return (int) (m_prev_part - array);

	m_prev_part = (CRFSFilePart *) bsearch (&position, array, parts, sizeof (CRFSFilePart), compare);

	if (!m_prev_part)
		return -1;

	return (int) (m_prev_part - array);
}

HRESULT CRFSFile::SyncRead (LONGLONG llPosition, DWORD lLength, BYTE* pBuffer, LONG *cbActual)
{
	OVERLAPPED o;
	LARGE_INTEGER offset;
	DWORD to_read, read, acc = 0;
	LONGLONG offset2;
	int pos;
#ifdef _DEBUG
	static int last_pos = -1;
#endif

	if (!pBuffer)
		return E_POINTER;

	pos = FindStartPart (llPosition);
	if (pos == -1)
	{
		DbgLog((LOG_TRACE, 2, L"FindStartPart bailed length = %lu, pos = %lld", lLength, llPosition));
		return S_FALSE;
	}

#ifdef _DEBUG
	if (pos != last_pos)
	{
		DbgLog((LOG_TRACE, 2, L"Now reading volume %d.", pos));
		last_pos = pos;
	}
#endif
	CRFSFilePart *part = array + pos;

	offset2 = llPosition - part->in_file_offset;
	offset.QuadPart = part->in_rar_offset + offset2;

	memset (&o, 0, sizeof (o));

	if (!(o.hEvent = CreateEvent (NULL, FALSE, FALSE, NULL)))
	{
		ErrorMsg (GetLastError (), L"CRFSOutputPin::SyncRead - CreateEvent");
		return S_FALSE;
	}

	while (true)
	{
		read = 0;
		to_read = min (lLength, (DWORD) (part->size - offset2));

		o.Offset = offset.LowPart;
		o.OffsetHigh = offset.HighPart;

		if (!ReadFile (part->file, pBuffer + acc, to_read, NULL, &o))
		{
			DWORD err = GetLastError ();

			if (err != ERROR_IO_PENDING)
			{
				ErrorMsg (err, L"CRFSOutputPin::SyncRead - ReadFile");
				break;
			}
		}
		if (!GetOverlappedResult (part->file, &o, &read, TRUE))
		{
			ErrorMsg (GetLastError (), L"CRFSOutputPin::SyncRead - GetOverlappedResult");
			break;
		}
		lLength -= read;
		acc += read;

		if (lLength == 0)
		{
			CloseHandle (o.hEvent);
			if (cbActual)
				*cbActual = acc;
			return S_OK;
		}

		pos ++;

		if (pos >= parts)
			break;

		part ++;
		offset2 = 0;
		offset.QuadPart = part->in_rar_offset;
	}

	CloseHandle (o.hEvent);
	if (cbActual)
		*cbActual = acc;
	return S_FALSE;
}
