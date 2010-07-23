/*
 *  Copyright (C) 2003-2006 Gabest
 *  http://www.gabest.org
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

#define DSMF_VERSION	0x01

#define DSMSW			0x44534D53ui64
#define DSMSW_SIZE		4

enum dsmp_t
{
	DSMP_FILEINFO		= 0,
	DSMP_STREAMINFO		= 1,
	DSMP_MEDIATYPE		= 2,
	DSMP_CHAPTERS		= 3,
	DSMP_SAMPLE			= 4,
	DSMP_SYNCPOINTS		= 5,
	DSMP_RESOURCE		= 6
};