/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
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
#endif
