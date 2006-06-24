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

[uuid("165BE9D6-0929-4363-9BA3-580D735AA0F6")]
interface IGraphBuilder2 : public IFilterGraph2
{
	STDMETHOD(IsPinDirection) (IPin* pPin, PIN_DIRECTION dir) = 0;
	STDMETHOD(IsPinConnected) (IPin* pPin) = 0;
	STDMETHOD(ConnectFilter) (IBaseFilter* pBF, IPin* pPinIn) = 0;
	STDMETHOD(ConnectFilter) (IPin* pPinOut, IBaseFilter* pBF) = 0;
	STDMETHOD(ConnectFilterDirect) (IPin* pPinOut, IBaseFilter* pBF, const AM_MEDIA_TYPE* pmt) = 0;
	STDMETHOD(NukeDownstream) (IUnknown* pUnk) = 0;
	STDMETHOD(FindInterface) (REFIID iid, void** ppv, BOOL bRemove) = 0;
	STDMETHOD(AddToROT) () = 0;
	STDMETHOD(RemoveFromROT) () = 0;
};

// private use only
[uuid("43CDA93D-6A4E-4A07-BD3E-49D161073EE7")]
interface IGraphBuilderDeadEnd : public IUnknown
{
	STDMETHOD_(size_t, GetCount)() = 0;
	STDMETHOD(GetDeadEnd) (int iIndex, CAtlList<CStringW>& path, CAtlList<CMediaType>& mts) = 0;
};