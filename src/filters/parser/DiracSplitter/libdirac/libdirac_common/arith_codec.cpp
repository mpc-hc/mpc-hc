/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: arith_codec.cpp,v 1.17 2007/11/16 04:48:44 asuraparaju Exp $ $Name:  $
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
* Contributor(s):   Tim Borer (Author),
                    Thomas Davies,
                    Scott R Ladd,
                    Peter Bleackley,
                    Steve Bearcroft,
                    Anuradha Suraparaju
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

#include <libdirac_common/arith_codec.h>

namespace dirac
{

const unsigned int Context::lut[256] =
{
    //LUT corresponds to window = 16 @ p0=0.5 & 8 @ p=1.0
    0,    2,    5,    8,   11,   15,   20,   24,
    29,   35,   41,   47,   53,   60,   67,   74,
    82,   89,   97,  106,  114,  123,  132,  141,
    150,  160,  170,  180,  190,  201,  211,  222,
    233,  244,  256,  267,  279,  291,  303,  315,
    327,  340,  353,  366,  379,  392,  405,  419,
    433,  447,  461,  475,  489,  504,  518,  533,
    548,  563,  578,  593,  609,  624,  640,  656,
    672,  688,  705,  721,  738,  754,  771,  788,
    805,  822,  840,  857,  875,  892,  910,  928,
    946,  964,  983, 1001, 1020, 1038, 1057, 1076,
    1095, 1114, 1133, 1153, 1172, 1192, 1211, 1231,
    1251, 1271, 1291, 1311, 1332, 1352, 1373, 1393,
    1414, 1435, 1456, 1477, 1498, 1520, 1541, 1562,
    1584, 1606, 1628, 1649, 1671, 1694, 1716, 1738,
    1760, 1783, 1806, 1828, 1851, 1874, 1897, 1920,
    1935, 1942, 1949, 1955, 1961, 1968, 1974, 1980,
    1985, 1991, 1996, 2001, 2006, 2011, 2016, 2021,
    2025, 2029, 2033, 2037, 2040, 2044, 2047, 2050,
    2053, 2056, 2058, 2061, 2063, 2065, 2066, 2068,
    2069, 2070, 2071, 2072, 2072, 2072, 2072, 2072,
    2072, 2071, 2070, 2069, 2068, 2066, 2065, 2063,
    2060, 2058, 2055, 2052, 2049, 2045, 2042, 2038,
    2033, 2029, 2024, 2019, 2013, 2008, 2002, 1996,
    1989, 1982, 1975, 1968, 1960, 1952, 1943, 1934,
    1925, 1916, 1906, 1896, 1885, 1874, 1863, 1851,
    1839, 1827, 1814, 1800, 1786, 1772, 1757, 1742,
    1727, 1710, 1694, 1676, 1659, 1640, 1622, 1602,
    1582, 1561, 1540, 1518, 1495, 1471, 1447, 1422,
    1396, 1369, 1341, 1312, 1282, 1251, 1219, 1186,
    1151, 1114, 1077, 1037,  995,  952,  906,  857,
    805,  750,  690,  625,  553,  471,  376,  255
};

ArithCodecBase::ArithCodecBase(ByteIO* p_byteio, size_t number_of_contexts):
    m_context_list(number_of_contexts),
    m_scount(0),
    m_byteio(p_byteio),
    m_decode_data_ptr(0)
{
    // nothing needed here
}

ArithCodecBase::~ArithCodecBase()
{
    delete[] m_decode_data_ptr;
}


void ArithCodecBase::InitEncoder()
{
    // Set the m_code word stuff
    m_low_code  = 0;
    m_range = 0xFFFF;
    m_underflow = 0;
}

void ArithCodecBase::FlushEncoder()
{

    // Do final renormalisation and output
    while(((m_low_code + m_range - 1) ^ m_low_code) < 0x8000)
    {
        m_byteio->WriteBit(m_low_code & 0x8000);
        for(; m_underflow > 0; m_underflow--)
            m_byteio->WriteBit(~m_low_code & 0x8000);

        m_low_code  <<= 1;
        m_low_code   &= 0xFFFF;
        m_range <<= 1;
    }

    while((m_low_code & 0x4000) && !((m_low_code + m_range - 1) & 0x4000))
    {
        m_underflow += 1;
        m_low_code  ^= 0x4000;
        m_low_code  <<= 1;
        m_low_code   &= 0xFFFF;
        m_range <<= 1;
    }

    m_byteio->WriteBit(m_low_code & 0x4000);
    while(m_underflow >= 0)
    {
        m_byteio->WriteBit(~m_low_code & 0x4000);
        m_underflow -= 1;
    }

    // byte align
    m_byteio->ByteAlignOutput();
}

void ArithCodecBase::InitDecoder(int num_bytes)
{
    ReadAllData(num_bytes);
    m_input_bits_left = 8;

    m_code = 0;
    m_low_code = 0;

    m_range = 0xFFFF;

    m_code = 0;
    for(int i = 0; i < 16; ++i)
    {
        m_code <<= 1;
        m_code += InputBit();
    }

}

int ArithCodecBase::ByteCount() const
{
    return m_byteio->GetSize();
}

void ArithCodecBase::ReadAllData(int num_bytes)
{
    if(m_decode_data_ptr)
        delete[] m_decode_data_ptr;

    m_decode_data_ptr = new char[num_bytes+2];
    m_byteio->InputBytes(m_decode_data_ptr , num_bytes);
    m_decode_data_ptr[num_bytes] = (char)255;
    m_decode_data_ptr[num_bytes+1] = (char)255;

    m_data_ptr = m_decode_data_ptr;
}

}// namespace dirac
