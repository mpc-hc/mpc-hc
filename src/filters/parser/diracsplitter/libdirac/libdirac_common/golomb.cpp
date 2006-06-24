/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: golomb.cpp 280 2005-01-30 05:11:46Z gabest $ $Name$
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
* Contributor(s): Thomas Davies (Original Author)
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

#include <libdirac_common/golomb.h>
#include <libdirac_common/bit_manager.h>
using namespace dirac;

#include <cmath>
#include <cstdlib>
#include <iostream>

using std::vector;
namespace dirac
{
void UnsignedGolombCode(BasicOutputManager& bitman, const unsigned int val)
{
    unsigned int M = 0;
    unsigned int info;
    unsigned int val2 = val;

    val2++;
    while (val2>1)
        {//get the log base 2 of val.
        val2 >>= 1;
        M++;        
    }
    info = val - (1<<M) + 1;

    //add length M+1 prefix
    for ( unsigned int i=1 ; i<=M ; ++i)
        bitman.OutputBit(0);
    
    bitman.OutputBit(1);

    //add info bits
    for (unsigned int i=0 ; i<M ;++i)
        bitman.OutputBit( info & (1<<i) );        
    

}
void UnsignedGolombCode(std::vector<bool>& bitvec, const unsigned int val)
{
    unsigned int M = 0;
    unsigned int info;
    unsigned int val2 = val;

    bitvec.clear();
    val2++;
    while ( val2>1 )
    {//get the log base 2 of val.
        val2 >>= 1;
        M++;        
    }
    info = val - (1<<M) + 1;

    //add length M+1 prefix
    for ( unsigned int i=1 ; i<=M ; ++i)
        bitvec.push_back(0);
    
    bitvec.push_back(1);

    //add info bits
    for ( unsigned int i=0 ; i<M ; ++i)
        bitvec.push_back( info & (1<<i) );

}

void GolombCode(BasicOutputManager& bitman, const int val)
{

    //code the magnitude
    UnsignedGolombCode(bitman,(unsigned int) abs(val));

    //do sign
    if (val>0) bitman.OutputBit(1);
    else if (val<0) bitman.OutputBit(0);
}

void GolombCode(vector<bool>& bitvec, const int val)
{

    //code the magnitude
    UnsignedGolombCode(bitvec,(unsigned int) abs(val));

    //do sign
    if (val>0) bitvec.push_back(1);
    else if (val<0) bitvec.push_back(0);
}

unsigned int UnsignedGolombDecode(BitInputManager& bitman)
{    
    unsigned int M = 0;
    unsigned int info = 0;
    bool bit = 0;
    unsigned int val = 0;

    do
    {
        bit = bitman.InputBit();
        if ( !bit )
            M++;
    }
    while( !bit && M<64 );//terminate if the number is too big!

    //now get the M info bits    
    for ( unsigned int i=0 ; i<M ; ++i)
    {
        bit = bitman.InputBit();
        if ( bit )
            info |= (1<<i);
    }// i    
    val = (1<<M) -1 + info;

    return val;
}

unsigned int UnsignedGolombDecode(const std::vector<bool>& bitvec)
{
    unsigned int M = 0;
    unsigned int info = 0;
    bool bit = 0;
    unsigned int val = 0;

    unsigned int index = 0;//index into bitvec

    do
    {
        bit = bitvec[++index];
        if (!bit)
            M++;
    }
    while( !bit && M<64 );//terminate if the number is too big!

    //now get the M info bits    
    for ( unsigned int i=0 ;i<M ; ++i)
    {
        bit = bitvec[++index];
        if (bit)
            info |= (1<<i);
    }    
    val = (1<<M) - 1 + info;

    return val;
}

int GolombDecode(BitInputManager& bitman)
{

    int val = int(UnsignedGolombDecode(bitman));
    bool bit;

     //get the sign
    if (val != 0)
    {
        bit = bitman.InputBit();
        if ( !bit )
            val = -val;
    }
    return val;        
}

int GolombDecode(const vector<bool>& bitvec)
{

    int val = int(UnsignedGolombDecode(bitvec));
    bool bit;

     //get sign
    if (val != 0)
    {
        bit = bitvec[bitvec.size()-1];
        if (!bit)
            val = -val;
    }
    return val;        
}
} // namespace dirac
