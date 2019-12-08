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

#ifndef LIST_H
#define LIST_H

template <class T> class CRFSList;

template <class T> class CRFSNode
{
	friend class CRFSList<T>;

public:
	CRFSNode (void) : next (NULL), prev (NULL) { }

	void Unlink (void)
	{
		next->prev = prev;
		prev->next = next;
	}

private:
	CRFSNode (CRFSNode *next, CRFSNode *prev) : next (next), prev (prev) { }

	CRFSNode *next;
	CRFSNode *prev;
};

template <class T> class CRFSList
{
public:
	CRFSList (bool auto_clear = false) : anchor (&anchor, &anchor), clear (auto_clear) { }
	~CRFSList () { if (clear) Clear (); }

	bool IsEmpty (void) { return anchor.next == &anchor; }

	T *First (void) { return Next (&anchor); }
	T *Last (void) { return Prev (&anchor); }

	void InsertFirst (CRFSNode<T> *n);
	void InsertLast (CRFSNode<T> *n);

	T *UnlinkFirst (void);
	T *UnlinkLast (void);

	T *Next (CRFSNode<T> *n);
	T *Prev (CRFSNode<T> *n);

	void Clear (void);

private:
	CRFSNode<T> anchor;
	bool clear;
};

#include "List.cpp"

#endif // LIST_H
