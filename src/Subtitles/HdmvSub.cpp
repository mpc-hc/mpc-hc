/*
 * (C) 2006-2014 see Authors.txt
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
#include "HdmvSub.h"
#include "../DSUtil/GolombBuffer.h"
#include <algorithm>

#if (0) // Set to 1 to activate HDMV subtitles traces
#define TRACE_HDMVSUB TRACE
#else
#define TRACE_HDMVSUB __noop
#endif


CHdmvSub::CHdmvSub()
    : CBaseSub(ST_HDMV)
    , m_nCurSegment(NO_SEGMENT)
    , m_pSegBuffer(nullptr)
    , m_nTotalSegBuffer(0)
    , m_nSegBufferPos(0)
    , m_nSegSize(0)
{
}

CHdmvSub::~CHdmvSub()
{
    Reset();

    delete [] m_pSegBuffer;
}


void CHdmvSub::AllocSegment(int nSize)
{
    if (nSize > m_nTotalSegBuffer) {
        delete [] m_pSegBuffer;
        m_pSegBuffer = DEBUG_NEW BYTE[nSize];
        m_nTotalSegBuffer = nSize;
    }
    m_nSegBufferPos = 0;
    m_nSegSize = nSize;
}

POSITION CHdmvSub::GetStartPosition(REFERENCE_TIME rt, double fps)
{
    POSITION pos = m_pPresentationSegments.GetHeadPosition();
    while (pos) {
        HDMV_PRESENTATION_SEGMENT* pPresentationSegment = m_pPresentationSegments.GetAt(pos);
        if (pPresentationSegment->rtStop <= rt) {
            m_pPresentationSegments.GetNext(pos);
        } else {
            break;
        }
    }

    return pos;
}

HRESULT CHdmvSub::ParseSample(IMediaSample* pSample)
{
    CheckPointer(pSample, E_POINTER);
    HRESULT hr;
    REFERENCE_TIME rtStart = INVALID_TIME, rtStop = INVALID_TIME;
    BYTE* pData = nullptr;
    int lSampleLen;

    hr = pSample->GetPointer(&pData);
    if (FAILED(hr) || pData == nullptr) {
        return hr;
    }
    lSampleLen = pSample->GetActualDataLength();

    pSample->GetTime(&rtStart, &rtStop);
    TRACE_HDMVSUB(_T("--------- ParseSample rtStart=%s, rtStop=%s ---------\n"), ReftimeToString(rtStart), ReftimeToString(rtStop));

    if (pData) {
        CGolombBuffer SampleBuffer(pData, lSampleLen);

        while (!SampleBuffer.IsEOF()) {
            if (m_nCurSegment == NO_SEGMENT) {
                HDMV_SEGMENT_TYPE nSegType = (HDMV_SEGMENT_TYPE)SampleBuffer.ReadByte();
                unsigned short nUnitSize = SampleBuffer.ReadShort();
                lSampleLen -= 3;

                switch (nSegType) {
                    case PALETTE:
                    case OBJECT:
                    case PRESENTATION_SEG:
                    case END_OF_DISPLAY:
                        m_nCurSegment = nSegType;
                        AllocSegment(nUnitSize);
                        break;

                    case WINDOW_DEF:
                    case INTERACTIVE_SEG:
                    case HDMV_SUB1:
                    case HDMV_SUB2:
                        // Ignored stuff...
                        SampleBuffer.SkipBytes(nUnitSize);
                        break;
                    default:
                        return VFW_E_SAMPLE_REJECTED;
                }
            }

            if (m_nCurSegment != NO_SEGMENT) {
                if (m_nSegBufferPos < m_nSegSize) {
                    int nSize = std::min(m_nSegSize - m_nSegBufferPos, lSampleLen);
                    SampleBuffer.ReadBuffer(m_pSegBuffer + m_nSegBufferPos, nSize);
                    m_nSegBufferPos += nSize;
                }

                if (m_nSegBufferPos >= m_nSegSize) {
                    CGolombBuffer SegmentBuffer(m_pSegBuffer, m_nSegSize);

                    switch (m_nCurSegment) {
                        case PALETTE:
                            TRACE_HDMVSUB(_T("CHdmvSub:PALETTE            rtStart=%10I64d\n"), rtStart);
                            ParsePalette(&SegmentBuffer, m_nSegSize);
                            break;
                        case OBJECT:
                            TRACE_HDMVSUB(_T("CHdmvSub:OBJECT             %s\n"), ReftimeToString(rtStart));
                            ParseObject(&SegmentBuffer, m_nSegSize);
                            break;
                        case PRESENTATION_SEG:
                            TRACE_HDMVSUB(_T("CHdmvSub:PRESENTATION_SEG   %s (size=%d)\n"), ReftimeToString(rtStart), m_nSegSize);

                            // Update the timestamp for the previous segment
                            UpdateTimeStamp(rtStart);

                            // Parse the new presentation segment
                            ParsePresentationSegment(rtStart, &SegmentBuffer);

                            break;
                        case WINDOW_DEF:
                            //TRACE_HDMVSUB(_T("CHdmvSub:WINDOW_DEF         %s\n"), ReftimeToString(rtStart));
                            break;
                        case END_OF_DISPLAY:
                            TRACE_HDMVSUB(_T("CHdmvSub:END_OF_DISPLAY     %s\n"), ReftimeToString(rtStart));
                            // Enqueue the current presentation segment if any
                            EnqueuePresentationSegment();
                            break;
                        default:
                            TRACE_HDMVSUB(_T("CHdmvSub:UNKNOWN Seg %d     rtStart=0x%10dd\n"), m_nCurSegment, rtStart);
                    }

                    m_nCurSegment = NO_SEGMENT;
                }
            }
        }
    }

    return hr;
}

int CHdmvSub::ParsePresentationSegment(REFERENCE_TIME rt, CGolombBuffer* pGBuffer)
{
    m_pCurrentPresentationSegment = CAutoPtr<HDMV_PRESENTATION_SEGMENT>(DEBUG_NEW HDMV_PRESENTATION_SEGMENT());

    m_pCurrentPresentationSegment->rtStart = rt;
    m_pCurrentPresentationSegment->rtStop = INFINITE_TIME; // Unknown for now

    ParseVideoDescriptor(pGBuffer, &m_pCurrentPresentationSegment->video_descriptor);
    ParseCompositionDescriptor(pGBuffer, &m_pCurrentPresentationSegment->composition_descriptor);
    m_pCurrentPresentationSegment->palette_update_flag = !!(pGBuffer->ReadByte() & 0x80);
    m_pCurrentPresentationSegment->CLUT.id = pGBuffer->ReadByte();
    m_pCurrentPresentationSegment->objectCount = pGBuffer->ReadByte();

    TRACE_HDMVSUB(_T("CHdmvSub::ParsePresentationSegment Size = %d, state = %#x, nObjectNumber = %d\n"), pGBuffer->GetSize(),
                  m_pCurrentPresentationSegment->composition_descriptor.bState, m_pCurrentPresentationSegment->objectCount);

    for (int i = 0; i < m_pCurrentPresentationSegment->objectCount; i++) {
        CompositionObject* pCompositionObject = DEBUG_NEW CompositionObject();
        ParseCompositionObject(pGBuffer, pCompositionObject);
        m_pCurrentPresentationSegment->objects.AddTail(pCompositionObject);
    }

    return m_pCurrentPresentationSegment->objectCount;
}

void CHdmvSub::EnqueuePresentationSegment()
{
    if (m_pCurrentPresentationSegment) {
        // TODO: improve the handling of subtitles without known end time in the queue
        //       so that empty segments can be ignored again safely
        /*if (m_pCurrentPresentationSegment->objectCount > 0) {*/
            m_pCurrentPresentationSegment->CLUT = m_CLUTs[m_pCurrentPresentationSegment->CLUT.id];

            // Get the objects' data
            POSITION pos = m_pCurrentPresentationSegment->objects.GetHeadPosition();
            while (pos) {
                CompositionObject* pObject = m_pCurrentPresentationSegment->objects.GetNext(pos);

                CompositionObject& pObjectData = m_compositionObjects[pObject->m_object_id_ref];

                pObject->m_width = pObjectData.m_width;
                pObject->m_height = pObjectData.m_height;

                if (pObjectData.GetRLEData()) {
                    pObject->SetRLEData(pObjectData.GetRLEData(), pObjectData.GetRLEDataSize(), pObjectData.GetRLEDataSize());
                }
            }

            TRACE_HDMVSUB(_T("CHdmvSub: Enqueue Presentation Segment %d - %s => ?\n"), m_pCurrentPresentationSegment->composition_descriptor.nNumber,
                          ReftimeToString(m_pCurrentPresentationSegment->rtStart));
            m_pPresentationSegments.AddTail(m_pCurrentPresentationSegment.Detach());
        /*} else {
            TRACE_HDMVSUB(_T("CHdmvSub: Delete empty Presentation Segment %d\n"), m_pCurrentPresentationSegment->composition_descriptor.nNumber);
            m_pCurrentPresentationSegment.Free();
        }*/
    }
}

void CHdmvSub::UpdateTimeStamp(REFERENCE_TIME rtStop)
{
    if (!m_pPresentationSegments.IsEmpty()) {
        HDMV_PRESENTATION_SEGMENT* pPresentationSegment = m_pPresentationSegments.GetTail();

        // Since we drop empty segments we might be trying to update a segment that isn't
        // in the queue so we update the timestamp only if it was previously unknown.
        if (pPresentationSegment->rtStop == INFINITE_TIME) {
            pPresentationSegment->rtStop = rtStop;

            TRACE_HDMVSUB(_T("CHdmvSub: Update Presentation Segment TimeStamp %d - %s => %s\n"), pPresentationSegment->composition_descriptor.nNumber,
                          ReftimeToString(pPresentationSegment->rtStart), ReftimeToString(pPresentationSegment->rtStop));
        }
    }
}

void CHdmvSub::ParsePalette(CGolombBuffer* pGBuffer, unsigned short nSize)  // #497
{
    BYTE palette_id = pGBuffer->ReadByte();
    HDMV_CLUT& CLUT = m_CLUTs[palette_id];

    CLUT.id = palette_id;
    CLUT.version_number = pGBuffer->ReadByte();

    ASSERT((nSize - 2) % sizeof(HDMV_PALETTE) == 0);
    CLUT.size = BYTE((nSize - 2) / sizeof(HDMV_PALETTE));

    for (int i = 0; i < CLUT.size; i++) {
        CLUT.palette[i].entry_id = pGBuffer->ReadByte();

        CLUT.palette[i].Y  = pGBuffer->ReadByte();
        CLUT.palette[i].Cr = pGBuffer->ReadByte();
        CLUT.palette[i].Cb = pGBuffer->ReadByte();
        CLUT.palette[i].T  = pGBuffer->ReadByte();
    }
}

void CHdmvSub::ParseObject(CGolombBuffer* pGBuffer, unsigned short nUnitSize)   // #498
{
    short object_id = pGBuffer->ReadShort();
    ASSERT(object_id < _countof(m_compositionObjects));

    CompositionObject& pObject = m_compositionObjects[object_id];

    pObject.m_version_number = pGBuffer->ReadByte();
    BYTE m_sequence_desc = pGBuffer->ReadByte();

    if (m_sequence_desc & 0x80) {
        int object_data_length = (int)pGBuffer->BitRead(24);

        pObject.m_width = pGBuffer->ReadShort();
        pObject.m_height = pGBuffer->ReadShort();

        pObject.SetRLEData(pGBuffer->GetBufferPos(), nUnitSize - 11, object_data_length - 4);

        TRACE_HDMVSUB(_T("CHdmvSub:ParseObject %d (size=%ld, %dx%d)\n"), object_id, object_data_length, pObject.m_width, pObject.m_height);
    } else {
        pObject.AppendRLEData(pGBuffer->GetBufferPos(), nUnitSize - 4);
    }
}

void CHdmvSub::ParseCompositionObject(CGolombBuffer* pGBuffer, CompositionObject* pCompositionObject)
{
    BYTE bTemp;
    pCompositionObject->m_object_id_ref = pGBuffer->ReadShort();
    pCompositionObject->m_window_id_ref = pGBuffer->ReadByte();
    bTemp = pGBuffer->ReadByte();
    pCompositionObject->m_object_cropped_flag = !!(bTemp & 0x80);
    pCompositionObject->m_forced_on_flag = !!(bTemp & 0x40);
    pCompositionObject->m_horizontal_position = pGBuffer->ReadShort();
    pCompositionObject->m_vertical_position = pGBuffer->ReadShort();

    if (pCompositionObject->m_object_cropped_flag) {
        pCompositionObject->m_cropping_horizontal_position = pGBuffer->ReadShort();
        pCompositionObject->m_cropping_vertical_position = pGBuffer->ReadShort();
        pCompositionObject->m_cropping_width = pGBuffer->ReadShort();
        pCompositionObject->m_cropping_height = pGBuffer->ReadShort();
    }
}

void CHdmvSub::ParseVideoDescriptor(CGolombBuffer* pGBuffer, VIDEO_DESCRIPTOR* pVideoDescriptor)
{
    pVideoDescriptor->nVideoWidth = pGBuffer->ReadShort();
    pVideoDescriptor->nVideoHeight = pGBuffer->ReadShort();
    pVideoDescriptor->bFrameRate = pGBuffer->ReadByte();
}

void CHdmvSub::ParseCompositionDescriptor(CGolombBuffer* pGBuffer, COMPOSITION_DESCRIPTOR* pCompositionDescriptor)
{
    pCompositionDescriptor->nNumber = pGBuffer->ReadShort();
    pCompositionDescriptor->bState  = pGBuffer->ReadByte() >> 6;
}

void CHdmvSub::Render(SubPicDesc& spd, REFERENCE_TIME rt, RECT& bbox)
{
    bool bRendered = false;

    RemoveOldSegments(rt);

    HDMV_PRESENTATION_SEGMENT* pPresentationSegment = FindPresentationSegment(rt);

    if (pPresentationSegment) {
        bool BT709 = m_infoSourceTarget.sourceMatrix == BT_709 ? true : m_infoSourceTarget.sourceMatrix == NONE ? (pPresentationSegment->video_descriptor.nVideoWidth > 720) : false;

        TRACE_HDMVSUB(_T("CHdmvSub:Render Presentation segment %d --> %s - %s\n"), pPresentationSegment->composition_descriptor.nNumber,
                      ReftimeToString(pPresentationSegment->rtStart), (pPresentationSegment->rtStop == INFINITE_TIME) ? _T("?") : ReftimeToString(pPresentationSegment->rtStop));

        bbox.left = bbox.top = LONG_MAX;
        bbox.right = bbox.bottom = 0;

        POSITION pos = pPresentationSegment->objects.GetHeadPosition();
        while (pos) {
            CompositionObject* pObject = pPresentationSegment->objects.GetNext(pos);

            if (pObject->GetRLEDataSize() && pObject->m_width > 0 && pObject->m_height > 0
                    && spd.w >= (pObject->m_horizontal_position + pObject->m_width) && spd.h >= (pObject->m_vertical_position + pObject->m_height)) {
                pObject->SetPalette(pPresentationSegment->CLUT.size, pPresentationSegment->CLUT.palette, BT709,
                                    m_infoSourceTarget.sourceBlackLevel, m_infoSourceTarget.sourceWhiteLevel, m_infoSourceTarget.targetBlackLevel, m_infoSourceTarget.targetWhiteLevel);
                bbox.left   = std::min(pObject->m_horizontal_position, bbox.left);
                bbox.top    = std::min(pObject->m_vertical_position, bbox.top);
                bbox.right  = std::max(pObject->m_horizontal_position + pObject->m_width, bbox.right);
                bbox.bottom = std::max(pObject->m_vertical_position + pObject->m_height, bbox.bottom);

                TRACE_HDMVSUB(_T(" --> Object %d (Pos=%dx%d, Res=%dx%d, SPDRes=%dx%d)\n"),
                              pObject->m_object_id_ref, pObject->m_horizontal_position, pObject->m_vertical_position, pObject->m_width, pObject->m_height, spd.w, spd.h);
                pObject->RenderHdmv(spd);

                bRendered = true;
            } else {
                TRACE_HDMVSUB(_T(" --> Invalid object %d\n"), pObject->m_object_id_ref);
            }
        }
    }

    if (!bRendered) {
        bbox = { 0, 0, 0, 0 };
    }
}

HRESULT CHdmvSub::GetTextureSize(POSITION pos, SIZE& MaxTextureSize, SIZE& VideoSize, POINT& VideoTopLeft)
{
    HDMV_PRESENTATION_SEGMENT* pPresentationSegment = m_pPresentationSegments.GetAt(pos);
    if (pPresentationSegment) {
        MaxTextureSize.cx = VideoSize.cx = pPresentationSegment->video_descriptor.nVideoWidth;
        MaxTextureSize.cy = VideoSize.cy = pPresentationSegment->video_descriptor.nVideoHeight;

        // The subs will be directly rendered into the proper position!
        VideoTopLeft.x = 0; //pObject->m_horizontal_position;
        VideoTopLeft.y = 0; //pObject->m_vertical_position;

        return S_OK;
    }

    ASSERT(FALSE);
    return E_INVALIDARG;
}

void CHdmvSub::Reset()
{
    m_nSegBufferPos = m_nSegSize = 0;
    m_nCurSegment = NO_SEGMENT;
    m_pCurrentPresentationSegment.Free();
    while (!m_pPresentationSegments.IsEmpty()) {
        delete m_pPresentationSegments.RemoveHead();
    }
    for (int i = 0; i < _countof(m_compositionObjects); i++) {
        m_compositionObjects[i].Reset();
    }
}

void CHdmvSub::RemoveOldSegments(REFERENCE_TIME rt)
{
    // Cleanup the old presentation segments. We keep a 2 min buffer to play nice with the queue.
    while (!m_pPresentationSegments.IsEmpty()
            && m_pPresentationSegments.GetHead()->rtStop != INFINITE_TIME
            && m_pPresentationSegments.GetHead()->rtStop + 120 * 10000000i64 < rt) {
        HDMV_PRESENTATION_SEGMENT* pPresentationSegment = m_pPresentationSegments.RemoveHead();
        TRACE_HDMVSUB(_T("CHdmvSub::RemoveOldSegments Remove presentation segment %d %s => %s (rt=%s)\n"),
                      pPresentationSegment->composition_descriptor.nNumber,
                      ReftimeToString(pPresentationSegment->rtStart), ReftimeToString(pPresentationSegment->rtStop), ReftimeToString(rt));
        delete pPresentationSegment;
    }
}

CHdmvSub::HDMV_PRESENTATION_SEGMENT* CHdmvSub::FindPresentationSegment(REFERENCE_TIME rt)
{
    POSITION pos = m_pPresentationSegments.GetHeadPosition();

    while (pos) {
        HDMV_PRESENTATION_SEGMENT* pPresentationSegment = m_pPresentationSegments.GetNext(pos);

        if (pPresentationSegment->rtStart <= rt && pPresentationSegment->rtStop > rt) {
            return pPresentationSegment;
        }
    }

    return nullptr;
}
