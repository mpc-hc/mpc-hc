/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2011 see AUTHORS
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

#pragma once

#include <atlcoll.h>
#include <afxinet.h>
#include "Playlist.h"


#define ISDb_PROTOCOL_VERSION 1


struct isdb_subtitle {
	int id, discs, disc_no;
	CStringA name, format, language, iso639_2, nick, email;
	struct isdb_subtitle() {
		reset();
	}
	void reset() {
		id = discs = disc_no = 0;
		format = language = nick = email = "";
	}
};

struct isdb_movie {
	CAtlList<CStringA> titles;
	CAtlList<isdb_subtitle> subs;
	void reset() {
		titles.RemoveAll();
		subs.RemoveAll();
	}
	isdb_movie& operator = (const struct isdb_movie& m) {
		if (this != &m) {
			titles.RemoveAll();
			titles.AddTailList(&m.titles);
			subs.RemoveAll();
			subs.AddTailList(&m.subs);
		}
		return *this;
	}
};

struct filehash {
	CString name;
	UINT64 size, hash;
};

extern bool hash(LPCTSTR fn, filehash& fh);
extern void hash(CPlaylist& pl, CList<filehash>& fhs);
extern CStringA makeargs(CPlaylist& pl);
extern bool OpenUrl(CInternetSession& is, CString url, CStringA& str);
