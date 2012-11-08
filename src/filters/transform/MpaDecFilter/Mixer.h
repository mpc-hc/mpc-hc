/*
 * (C) 2012 see Authors.txt
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

struct AVAudioResampleContext;

class CMixer
{
protected:
    AVAudioResampleContext* m_pAVRCxt;
    DWORD last_in_layout;
    DWORD last_out_layout;
    enum AVSampleFormat last_in_avsf;

    void Init(DWORD out_layout, DWORD in_layout, enum AVSampleFormat in_sf);

public:
    CMixer();
    ~CMixer();

    HRESULT Mixing(float* pOutput, int out_samples, DWORD out_layout, BYTE* pInput, int in_samples, DWORD in_layout, enum AVSampleFormat in_sf);
};
