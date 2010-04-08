/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: transform_byteio.cpp,v 1.10 2008/01/31 11:25:15 tjdwave Exp $ $Name:  $
*
* Version: MPL 1.1/GPL 2.0/LGPL 2.1
*
* The contents of this file are subject to the Mozilla Public License
* Version 1.1 (the "License"); you may not use this file except in compliance
* with the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
* the specific language governing rights and limitations under the License.
*
* The Original Code is BBC Research and Development code.
*
* The Initial Developer of the Original Code is the British Broadcasting
* Corporation.
* Portions created by the Initial Developer are Copyright (C) 2004.
* All Rights Reserved.
*
* Contributor(s): Anuradha Suraparaju (Original Author)
*                 Andrew Kennedy
*
* Alternatively, the contents of this file may be used under the terms of
* the GNU General Public License Version 2 (the "GPL"), or the GNU Lesser
* Public License Version 2.1 (the "LGPL"), in which case the provisions of
* the GPL or the LGPL are applicable instead of those above. If you wish to
* allow use of your version of this file only under the terms of the either
* the GPL or LGPL and not to allow others to use your version of this file
* under the MPL, indicate your decision by deleting the provisions above
* and replace them with the notice and other provisions required by the GPL
* or LGPL. If you do not delete the provisions above, a recipient may use
* your version of this file under the terms of any one of the MPL, the GPL
* or the LGPL.
* ***** END LICENSE BLOCK ***** */

#include <libdirac_byteio/transform_byteio.h>
#include <libdirac_common/dirac_exception.h>

using namespace dirac;

TransformByteIO::TransformByteIO(PictureParams& fparams,
                                 CodecParams& cparams):
    ByteIO(),
    m_fparams(fparams),
    m_cparams(cparams),
    m_default_cparams(cparams.GetVideoFormat(), fparams.GetPictureType(), fparams.Refs().size(), true)
{
}

TransformByteIO::TransformByteIO(ByteIO &byte_io,
                                 PictureParams& fparams,
                                 CodecParams& cparams):
    ByteIO(byte_io),
    m_fparams(fparams),
    m_cparams(cparams),
    m_default_cparams(cparams.GetVideoFormat(), fparams.GetPictureType(), fparams.Refs().size(), true)
{
}

TransformByteIO::~TransformByteIO()
{
    for(size_t index = 0; index < m_component_list.size(); ++index)
        delete m_component_list[index];
}

void TransformByteIO::CollateByteStats(DiracByteStats& dirac_byte_stats)
{
    // set number of component bytes
    for(size_t index = 0; index < m_component_list.size(); ++index)
        m_component_list[index]->CollateByteStats(dirac_byte_stats);
}

int TransformByteIO::GetSize() const
{
    //std::cerr << "Transform Size=" << ByteIO::GetSize() << std::endl;
    int size = 0;
    for(size_t index = 0; index < m_component_list.size(); ++index)
        size += m_component_list[index]->GetSize();
    return ByteIO::GetSize() + size;
}

const std::string TransformByteIO::GetBytes()
{
    std::string bytes;
    for(size_t index = 0; index < m_component_list.size(); ++index)
        bytes += m_component_list[index]->GetBytes();
    return ByteIO::GetBytes() + bytes;
}

void TransformByteIO::Output()
{
    // Zero Transform flag - applies only to inter frames
    if(m_fparams.PicSort().IsInter())
        WriteBit(false);
    // Wavelet index
    WriteUint(m_cparams.TransformFilter());

    // Wavelet Depth
    WriteUint(m_cparams.TransformDepth());

    // Spatial Partition flag
    WriteBit(m_cparams.SpatialPartition());
    if(m_cparams.SpatialPartition())
    {
        for(unsigned int i = 0; i <= m_cparams.TransformDepth(); ++i)
        {
            const CodeBlocks &cb = m_cparams.GetCodeBlocks(i);
            // Number of Horizontal code blocks for level i
            WriteUint(cb.HorizontalCodeBlocks());
            // Number of Vertical code block for level i
            WriteUint(cb.VerticalCodeBlocks());
        }
        // Code block mode index
        WriteUint(m_cparams.GetCodeBlockMode());
    }
    // Flush output for bend alignment
    ByteAlignOutput();
}

void TransformByteIO::Input()
{
    // Byte Alignment
    ByteAlignInput();

    m_cparams.SetZeroTransform(false);
    // Zero transform flag - applies only for inter frames
    if(m_fparams.PicSort().IsInter())
        m_cparams.SetZeroTransform(ReadBool());

    if(m_cparams.ZeroTransform())
        return;

    // Transform filter
    m_cparams.SetTransformFilter(ReadUint());

    // transform depth
    m_cparams.SetTransformDepth(ReadUint());

    // Spatial partition flag
    m_cparams.SetSpatialPartition(ReadBool());

    if(m_cparams.SpatialPartition())
    {
        // Input number of code blocks for each level
        for(unsigned int i = 0; i <= m_cparams.TransformDepth(); ++i)
        {
            // number of horizontal code blocks for level i
            unsigned int hblocks = ReadUint();
            // number of vertical code blocks for level i
            unsigned int vblocks = ReadUint();
            m_cparams.SetCodeBlocks(i, hblocks, vblocks);
        }
        // Code block mode index
        m_cparams.SetCodeBlockMode(ReadUint());
    }
    // Byte Alignment
    ByteAlignInput();
}

void TransformByteIO::AddComponent(ComponentByteIO* component_byteio)
{
    m_component_list.push_back(component_byteio);
}


//-------------private---------------------------------------------------------------
