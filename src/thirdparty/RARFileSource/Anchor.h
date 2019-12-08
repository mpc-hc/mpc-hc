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

#ifndef ANCHOR_H
#define ANCHOR_H

#include <windows.h>

template <class T> class Anchor
{
public:
	Anchor (T **value) : value (value) { }
	Anchor (void) : value (NULL) { }
	~Anchor (void) { if (value) { delete *value; *value = NULL; } }

	void Set (T **value) { this->value = value; }
	void Release (void) { value = NULL; }

private:
	T **value;
};

template <class T> class ArrayAnchor
{
public:
	ArrayAnchor (T **value) : value (value) { }
	ArrayAnchor (void) : value (NULL) { }
	~ArrayAnchor (void) { if (value) { delete [] *value; *value = NULL; } }

	void Set (T **value) { this->value = value; }
	void Release (void) { value = NULL; }

private:
	T **value;
};

template <> class Anchor<HANDLE>
{
public:
	Anchor (HANDLE *value) : value (value) { }
	Anchor (void) : value (NULL) { }
	~Anchor (void) { Close (); }

	void Set (HANDLE *val) { value = val; }
	void Release (void) { value = NULL; }
	void Close (void)
	{
		if (value && *value != INVALID_HANDLE_VALUE)
		{
			CloseHandle (*value);
			*value = INVALID_HANDLE_VALUE;
		}
	}

private:
	HANDLE *value;
};

#endif // ANCHOR_H
