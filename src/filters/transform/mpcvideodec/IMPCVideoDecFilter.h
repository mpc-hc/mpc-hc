/* 
 * $Id: IMPCVideoDecFilter.h 249 2007-09-26 11:07:22Z casimir666 $
 *
 * (C) 2006-2007 see AUTHORS
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


[uuid("CDC3B5B3-A8B0-4c70-A805-9FC80CDEF262")]
interface IMPCVideoDecFilter : public IUnknown
{
	STDMETHOD(SetThreadNumber(int nValue)) = 0;
	STDMETHOD_(int, GetThreadNumber()) = 0;

	STDMETHOD(SetEnableDXVA(bool fValue)) = 0;
	STDMETHOD_(bool, GetEnableDXVA()) = 0;

	STDMETHOD(SetDiscardMode(int nValue)) = 0;
	STDMETHOD_(int, GetDiscardMode()) = 0;

	STDMETHOD(SetErrorResilience(int nValue)) = 0;
	STDMETHOD_(int, GetErrorResilience()) = 0;

	STDMETHOD(SetIDCTAlgo(int nValue)) = 0;
	STDMETHOD_(int, GetIDCTAlgo()) = 0;
};
