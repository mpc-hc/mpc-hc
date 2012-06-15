/*
 * $Id$
 *
 * (C) 2006-2012 see Authors.txt
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

#include "stdafx.h"
#include "../../../DSUtil/DSUtil.h"
#include "DXVADecoderH264.h"
#include "MPCVideoDecFilter.h"
#include "VideoDecDXVAAllocator.h"
#include "ffmpeg/libavcodec/avcodec.h"

#include "FfmpegContext.h"

#if 0
#define TRACE_H264 TRACE
#else
#define TRACE_H264(...)
#endif

CDXVADecoderH264::CDXVADecoderH264(CMPCVideoDecFilter* pFilter, IAMVideoAccelerator*  pAMVideoAccelerator, DXVAMode nMode, int nPicEntryNumber)
    : CDXVADecoder(pFilter, pAMVideoAccelerator, nMode, nPicEntryNumber)
{
    m_bUseLongSlice = (GetDXVA1Config()->bConfigBitstreamRaw != 2);
    Init();
}

CDXVADecoderH264::CDXVADecoderH264(CMPCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDirectXVideoDec, DXVAMode nMode, int nPicEntryNumber, DXVA2_ConfigPictureDecode* pDXVA2Config)
    : CDXVADecoder(pFilter, pDirectXVideoDec, nMode, nPicEntryNumber, pDXVA2Config)
{
    m_bUseLongSlice = (m_pFilter->GetDXVA2Config()->ConfigBitstreamRaw != 2);
    Init();
}

CDXVADecoderH264::~CDXVADecoderH264()
{
}

void CDXVADecoderH264::Init()
{
    memset(&m_DXVAPicParams,   0, sizeof(m_DXVAPicParams));
    memset(&m_DXVAPicParams,   0, sizeof(DXVA_PicParams_H264));
    memset(&m_pSliceLong,      0, sizeof(DXVA_Slice_H264_Long) *MAX_SLICES);
    memset(&m_pSliceShort,     0, sizeof(DXVA_Slice_H264_Short)*MAX_SLICES);

    m_DXVAPicParams.MbsConsecutiveFlag                  = 1;
    if (m_pFilter->GetPCIVendor() == PCIV_Intel) {
        m_DXVAPicParams.Reserved16Bits                  = 0x534c;
    } else {
        m_DXVAPicParams.Reserved16Bits                  = 0;
    }
    m_DXVAPicParams.ContinuationFlag                    = 1;
    m_DXVAPicParams.Reserved8BitsA                      = 0;
    m_DXVAPicParams.Reserved8BitsB                      = 0;
    m_DXVAPicParams.MinLumaBipredSize8x8Flag            = 1;    // Improve accelerator performances
    m_DXVAPicParams.StatusReportFeedbackNumber          = 0;    // Use to report status

    for (int i = 0; i < 16; i++) {
        m_DXVAPicParams.RefFrameList[i].AssociatedFlag  = 1;
        m_DXVAPicParams.RefFrameList[i].bPicEntry       = 255;
        m_DXVAPicParams.RefFrameList[i].Index7Bits      = 127;
    }


    m_nNALLength        = 4;
    m_nMaxSlices        = 0;

    switch (GetMode()) {
        case H264_VLD :
            AllocExecuteParams(3);
            break;
        default :
            ASSERT(FALSE);
    }
}

void CDXVADecoderH264::CopyBitstream(BYTE* pDXVABuffer, BYTE* pBuffer, UINT& nSize)
{
    CH264Nalu   Nalu;
    int         nDummy;
    int         nSlices     = 0;
    UINT        m_nSize     = nSize;
    int         slice_step  = 1;
    int         nDxvaNalLength;

    while (!nSlices && slice_step <= 2) {
        Nalu.SetBuffer(pBuffer, m_nSize, slice_step == 1 ? m_nNALLength : 0);
        nSize = 0;
        while (Nalu.ReadNext()) {
            switch (Nalu.GetType()) {
                case NALU_TYPE_SLICE:
                case NALU_TYPE_IDR:
                    // Skip the NALU if the data length is below 0
                    if (Nalu.GetDataLength() < 0) {
                        break;
                    }

                    // For AVC1, put startcode 0x000001
                    pDXVABuffer[0] = pDXVABuffer[1] = 0; pDXVABuffer[2] = 1;
                    if (Nalu.GetDataLength() < 0) {
                        break;
                    }

                    // Copy NALU
                    __try {
                        memcpy(pDXVABuffer + 3, Nalu.GetDataBuffer(), Nalu.GetDataLength());
                    } __except (EXCEPTION_EXECUTE_HANDLER) { break; }

                    // Update slice control buffer
                    nDxvaNalLength                                  = Nalu.GetDataLength() + 3;
                    m_pSliceShort[nSlices].BSNALunitDataLocation    = nSize;
                    m_pSliceShort[nSlices].SliceBytesInBuffer       = nDxvaNalLength;

                    nSize                                          += nDxvaNalLength;
                    pDXVABuffer                                    += nDxvaNalLength;
                    nSlices++;
                    break;
            }
        }
        slice_step++;
    }

    // Complete with zero padding (buffer size should be a multiple of 128)
    nDummy  = 128 - (nSize % 128);

    memset(pDXVABuffer, 0, nDummy);
    m_pSliceShort[nSlices - 1].SliceBytesInBuffer += nDummy;
    nSize                                       += nDummy;
}

void CDXVADecoderH264::Flush()
{
    ClearRefFramesList();
    m_DXVAPicParams.UsedForReferenceFlags   = 0;
    m_nOutPOC                               = INT_MIN;
    m_nPrevOutPOC                           = INT_MIN;

    m_nfield_pic_flag                       = 1;
    m_nRefPicFlag                           = 1;
    m_nBrokenFramesFlag                     = 0;
    m_nBrokenFramesFlag_POC                 = 0;

    __super::Flush();
}

HRESULT CDXVADecoderH264::DecodeFrame(BYTE* pDataIn, UINT nSize, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop)
{
    HRESULT                     hr              = S_FALSE;
    UINT                        nSlices         = 0;
    int                         nSurfaceIndex   = -1;
    int                         nFieldType      = -1;
    int                         nSliceType      = -1;
    int                         nFramePOC       = INT_MIN;
    int                         nOutPOC         = INT_MIN;
    REFERENCE_TIME              rtOutStart      = _I64_MIN;
    CH264Nalu                   Nalu;
    UINT                        nNalOffset      = 0;
    CComPtr<IMediaSample>       pSampleToDeliver;
    CComQIPtr<IMPCDXVA2Sample>  pDXVA2Sample;
    int                         slice_step      = 1;

    if (FFH264DecodeBuffer(m_pFilter->GetAVCtx(), pDataIn, nSize, &nFramePOC, &nOutPOC, &rtOutStart) == -1) {
        return S_FALSE;
    }

    while (!nSlices && slice_step <= 2) {
        Nalu.SetBuffer(pDataIn, nSize, slice_step == 1 ? m_nNALLength : 0);
        while (Nalu.ReadNext()) {
            switch (Nalu.GetType()) {
                case NALU_TYPE_SLICE:
                case NALU_TYPE_IDR:
                    if (m_bUseLongSlice) {
                        m_pSliceLong[nSlices].BSNALunitDataLocation = nNalOffset;
                        m_pSliceLong[nSlices].SliceBytesInBuffer    = Nalu.GetDataLength() + 3; //.GetRoundedDataLength();
                        m_pSliceLong[nSlices].slice_id              = nSlices;
                        FF264UpdateRefFrameSliceLong(&m_DXVAPicParams, &m_pSliceLong[nSlices], m_pFilter->GetAVCtx());

                        if (nSlices > 0) {
                            m_pSliceLong[nSlices - 1].NumMbsForSlice = m_pSliceLong[nSlices].NumMbsForSlice = m_pSliceLong[nSlices].first_mb_in_slice - m_pSliceLong[nSlices - 1].first_mb_in_slice;
                        }
                    }
                    nSlices++;
                    nNalOffset += (UINT)(Nalu.GetDataLength() + 3);
                    if (nSlices > MAX_SLICES) {
                        break;
                    }
                    break;
            }
        }
        slice_step++;
    }

    if (!nSlices) {
        return S_FALSE;
    }

    m_nMaxWaiting = min(max(m_DXVAPicParams.num_ref_frames, 3), 8);

    // If parsing fail (probably no PPS/SPS), continue anyway it may arrived later (happen on truncated streams)
    if (FAILED(FFH264BuildPicParams(&m_DXVAPicParams, &m_DXVAScalingMatrix, &nFieldType, &nSliceType, m_pFilter->GetAVCtx(), m_pFilter->GetPCIVendor()))) {
        return S_FALSE;
    }

    TRACE_H264("CDXVADecoderH264::DecodeFrame() : nFramePOC = %11d, nOutPOC = %11d[%11d], [%d - %d], rtOutStart = [%20I64d]\n", nFramePOC, nOutPOC, m_nOutPOC, m_DXVAPicParams.field_pic_flag, m_DXVAPicParams.RefPicFlag, rtOutStart);

    // Wait I frame after a flush
    if (m_bFlushed && !m_DXVAPicParams.IntraPicFlag) {
        TRACE_H264("CDXVADecoderH264::DecodeFrame() : Flush - wait I frame\n");
        m_nBrokenFramesFlag     = 0;
        m_nBrokenFramesFlag_POC = 0;
        m_nfield_pic_flag       = m_DXVAPicParams.field_pic_flag;
        m_nRefPicFlag           = m_DXVAPicParams.RefPicFlag;
        m_nPrevOutPOC           = INT_MIN;
        return S_FALSE;
    }

    // Some magic code for detecting the incorrect decoding of interlaced frames ...
    // TODO : necessary to make it better, and preferably on the side of ffmpeg ...
    if (m_nfield_pic_flag && m_nfield_pic_flag == m_DXVAPicParams.field_pic_flag && m_nRefPicFlag == m_DXVAPicParams.RefPicFlag) {
        if (m_nPrevOutPOC == m_nOutPOC && m_nOutPOC == INT_MIN) {
            m_nBrokenFramesFlag_POC++;
        }
        m_nBrokenFramesFlag++;
    } else {
        m_nBrokenFramesFlag     = 0;
        m_nBrokenFramesFlag_POC = 0;
    }
    m_nfield_pic_flag   = m_DXVAPicParams.field_pic_flag;
    m_nRefPicFlag       = m_DXVAPicParams.RefPicFlag;
    m_nPrevOutPOC       = m_nOutPOC;

    if (m_nBrokenFramesFlag > 4) {
        m_nBrokenFramesFlag = 0;
        if (m_nBrokenFramesFlag_POC > 1) {
            TRACE_H264("CDXVADecoderH264::DecodeFrame() : Detected broken frames ... flush data\n");
            m_nBrokenFramesFlag_POC = 0;
            Flush();
            return S_FALSE;
        }
    }
    //

    CHECK_HR(GetFreeSurfaceIndex(nSurfaceIndex, &pSampleToDeliver, rtStart, rtStop));
    FFH264SetCurrentPicture(nSurfaceIndex, &m_DXVAPicParams, m_pFilter->GetAVCtx());

    CHECK_HR(BeginFrame(nSurfaceIndex, pSampleToDeliver));

    m_DXVAPicParams.StatusReportFeedbackNumber++;

    // Send picture parameters
    CHECK_HR(AddExecuteBuffer(DXVA2_PictureParametersBufferType, sizeof(m_DXVAPicParams), &m_DXVAPicParams));
    CHECK_HR(Execute());

    // Add bitstream, slice control and quantization matrix
    CHECK_HR(AddExecuteBuffer(DXVA2_BitStreamDateBufferType, nSize, pDataIn, &nSize));

    if (m_bUseLongSlice) {
        CHECK_HR(AddExecuteBuffer(DXVA2_SliceControlBufferType,  sizeof(DXVA_Slice_H264_Long)*nSlices, m_pSliceLong));
    } else {
        CHECK_HR(AddExecuteBuffer(DXVA2_SliceControlBufferType, sizeof(DXVA_Slice_H264_Short)*nSlices, m_pSliceShort));
    }

    CHECK_HR(AddExecuteBuffer(DXVA2_InverseQuantizationMatrixBufferType, sizeof(DXVA_Qmatrix_H264), (void*)&m_DXVAScalingMatrix));

    // Decode bitstream
    CHECK_HR(Execute());

    CHECK_HR(EndFrame(nSurfaceIndex));

#if defined(_DEBUG) && 0
    DisplayStatus();
#endif

    bool bAdded = AddToStore(nSurfaceIndex, pSampleToDeliver, m_DXVAPicParams.RefPicFlag, rtStart, rtStop,
                             m_DXVAPicParams.field_pic_flag, (FF_FIELD_TYPE)nFieldType,
                             (FF_SLICE_TYPE)nSliceType, nFramePOC);

    FFH264UpdateRefFramesList(&m_DXVAPicParams, m_pFilter->GetAVCtx());
    ClearUnusedRefFrames();

    if (bAdded) {
        hr = DisplayNextFrame();
    }

    if (nOutPOC != INT_MIN) {
        m_nOutPOC       = nOutPOC;
        m_rtOutStart    = rtOutStart;
    }

    m_bFlushed = false;
    return hr;
}

void CDXVADecoderH264::RemoveUndisplayedFrame(int nPOC)
{
    // Find frame with given POC, and free the slot
    for (int i = 0; i < m_nPicEntryNumber; i++) {
        if (m_pPictureStore[i].bInUse && m_pPictureStore[i].nCodecSpecific == nPOC) {
            m_pPictureStore[i].bDisplayed = true;
            RemoveRefFrame(i);
            return;
        }
    }
}

void CDXVADecoderH264::ClearUnusedRefFrames()
{
    // Remove old reference frames (not anymore a short or long ref frame)
    for (int i = 0; i < m_nPicEntryNumber; i++) {
        if (m_pPictureStore[i].bRefPicture && m_pPictureStore[i].bDisplayed) {
            if (!FFH264IsRefFrameInUse(i, m_pFilter->GetAVCtx())) {
                RemoveRefFrame(i);
            }
        }
    }
}

void CDXVADecoderH264::SetExtraData(BYTE* pDataIn, UINT nSize)
{
    AVCodecContext* pAVCtx  = m_pFilter->GetAVCtx();
    m_nNALLength            = pAVCtx->nal_length_size;

    FFH264DecodeBuffer(pAVCtx, pDataIn, nSize, NULL, NULL, NULL);
    FFH264SetDxvaSliceLong(pAVCtx, m_pSliceLong);
}

void CDXVADecoderH264::ClearRefFramesList()
{
    for (int i = 0; i < m_nPicEntryNumber; i++) {
        if (m_pPictureStore[i].bInUse) {
            m_pPictureStore[i].bDisplayed = true;
            RemoveRefFrame(i);
        }
    }
}

HRESULT CDXVADecoderH264::DisplayStatus()
{
    HRESULT             hr = E_INVALIDARG;
    DXVA_Status_H264    Status;

    memset(&Status, 0, sizeof(Status));
    CHECK_HR(hr = CDXVADecoder::QueryStatus(&Status, sizeof(Status)));

    TRACE_H264("CDXVADecoderH264::DisplayStatus() : Status for the frame %u : bBufType = %u, bStatus = %u, wNumMbsAffected = %u\n",
               Status.StatusReportFeedbackNumber,
               Status.bBufType,
               Status.bStatus,
               Status.wNumMbsAffected);

    return hr;
}

int CDXVADecoderH264::FindOldestFrame()
{
    int             nPos  = -1;
    REFERENCE_TIME  rtPos = _I64_MAX;

    for (int i = 0; i < m_nPicEntryNumber; i++) {
        if (m_pPictureStore[i].bInUse && !m_pPictureStore[i].bDisplayed) {
            if ((m_pPictureStore[i].nCodecSpecific == m_nOutPOC) && (m_pPictureStore[i].rtStart < rtPos) && (m_pPictureStore[i].rtStart >= m_rtOutStart)) {
                nPos  = i;
                rtPos = m_pPictureStore[i].rtStart;
            }
        }
    }

    if (nPos != -1) {
        m_pPictureStore[nPos].rtStart   = m_rtOutStart;
        m_pFilter->UpdateFrameTime(m_pPictureStore[nPos].rtStart, m_pPictureStore[nPos].rtStop);
        m_pFilter->ReorderBFrames(m_pPictureStore[nPos].rtStart, m_pPictureStore[nPos].rtStop);
    }

    return nPos;
}
