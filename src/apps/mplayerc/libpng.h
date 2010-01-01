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

#pragma once

struct png_t {unsigned char* data; unsigned int size, pos;};

#ifdef __cplusplus

extern "C" unsigned char* DecompressPNG(struct png_t* png, int* w, int* h);

#include <atlimage.h>

class CPngImage : public CImage
{
public:
	bool LoadFromResource(UINT id)
	{
		bool ret = false;

		CStringA str;
		if(LoadResource(id, str, _T("FILE")))
		{
			struct png_t png;
			png.data = (unsigned char*)(LPCSTR)str;
			png.size = str.GetLength();
			int w, h;
			if(BYTE* p = DecompressPNG(&png, &w, &h))
			{
				if(Create(w, -h, 32))
				{
					for(int y = 0; y < h; y++) 
						memcpy(GetPixelAddress(0, y), &p[w*4*y], w*4);
					ret = true;
				}

				free(p);
			}
		}

		return ret;
	}
};
#endif /* __cplusplus */
