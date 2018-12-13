/*

* (C) 2017 see Authors.txt
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

// A custom interface to allow the user to modify audio
// encoder properties
#ifndef __IWAVTRANSFERPERTIES__
#define __IWAVTRANSFERPERTIES__
#include "INITGUID.H"


DEFINE_GUID(IID_IWavTransferProperties, 0x51618802, 0x5d86, 0x4738, 0x83, 0x7f, 0x82, 0xad, 0x24, 0x30, 0x42, 0xe4);

interface __declspec(uuid("51618802-5D86-4738-837F-82AD243042E4"))
	IWavTransferProperties :
	public IUnknown
{

	STDMETHOD(AddBuffer) (THIS_
		LPWAVEHDR pwh, UINT cbwh
		) PURE;
	// Enable/disable PES output
	STDMETHOD(EnablePreview) (THIS_
		bool bpPreview
		) PURE;
	STDMETHOD(set_2Bytes) (THIS_
		bool b2Bytes
		) PURE;

	STDMETHOD(GetCurrentLastTime) (THIS_
		REFERENCE_TIME *pTime
		) PURE;
	STDMETHOD(SetCurrentLastTime) (THIS_
		REFERENCE_TIME pTime
		) PURE;
	STDMETHOD(SetBufferEvent) (

		) PURE;
	// Get target compression bitrate in Kbits/s

};



#endif 



