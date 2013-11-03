/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#define DSMF_VERSION    0x01

#define DSMSW           0x44534D53ui64
#define DSMSW_SIZE      4

enum dsmp_t {
    DSMP_FILEINFO   = 0,
    DSMP_STREAMINFO = 1,
    DSMP_MEDIATYPE  = 2,
    DSMP_CHAPTERS   = 3,
    DSMP_SAMPLE     = 4,
    DSMP_SYNCPOINTS = 5,
    DSMP_RESOURCE   = 6
};
