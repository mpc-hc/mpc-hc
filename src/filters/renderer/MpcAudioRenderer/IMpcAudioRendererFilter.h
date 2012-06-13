/*
 * $Id$
 *
 * (C) 2010-2012 see Authors.txt
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


interface __declspec(uuid("495D2C66-D430-439b-9DEE-40F9B7929BBA"))
IMpcAudioRendererFilter :
public IUnknown {
    STDMETHOD(Apply()) = 0;

    STDMETHOD(SetWasapiMode(BOOL nValue)) = 0;
    STDMETHOD_(BOOL, GetWasapiMode()) = 0;
    STDMETHOD(SetMuteFastForward(BOOL nValue)) = 0;
    STDMETHOD_(BOOL, GetMuteFastForward()) = 0;
    STDMETHOD(SetSoundDevice(CString nValue)) = 0;
    STDMETHOD_(CString, GetSoundDevice()) = 0;
};
