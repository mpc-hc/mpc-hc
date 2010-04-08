/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: prefilter.cpp,v 1.9 2008/10/29 02:42:06 asuraparaju Exp $ $Name:  $
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
* Portions created by the Initial Developer are Copyright (C) 2008.
* All Rights Reserved.
*
* Contributor(s): Thomas Davies (Original Author),
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

#include<libdirac_encoder/prefilter.h>
#include<libdirac_common/arrays.h>

using namespace dirac;

void dirac::CWMFilter(Picture& picture, const int strength)
{
    CWMFilterComponent(picture.Data(Y_COMP), strength);
    CWMFilterComponent(picture.Data(U_COMP), strength);
    CWMFilterComponent(picture.Data(V_COMP), strength);
}

void dirac::CWMFilterComponent(PicArray& pic_data, const int strength)
{
    // Do centre-weighted median denoising

    PicArray pic_copy(pic_data);

    const int width(3);
    const int offset((width - 1) / 2);
    const int centre_weight = std::max(1, (width * width + 1) - strength);
    const int list_length = centre_weight + (width * width) - 1;

    ValueType* val_list = new ValueType[list_length];

    for(int j = offset; j < pic_data.LengthY() - offset; ++j)
    {
        for(int i = offset; i < pic_data.LastX() - offset; ++i)
        {
            // Make the value list
            int pos = 0;
            for(; pos < centre_weight - 1; ++pos)
                val_list[pos] = pic_copy[j][i];

            for(int s = -offset; s <= offset; ++s)
            {
                for(int r = -offset; r <= offset; ++r)
                {
                    val_list[pos] = pic_copy[j+s][i+r];
                    pos++;
                }// r
            }// s

            pic_data[j][i] = Median(val_list, list_length);
        }// i
    }// j

    delete[] val_list;
}

ValueType dirac::Median(const ValueType* val_list, const int length)
{


    OneDArray<ValueType> ordered_vals(length);

    // Place the values in order
    int pos = 0;
    ordered_vals[0] = val_list[0];
    for(int i = 1 ; i < length ; ++i)
    {
        for(int k = 0 ; k < i ; ++k)
        {
            if(val_list[i] < ordered_vals[k])
            {
                pos = k;
                break;
            }
            else
                pos = k + 1;
        }// k

        if(pos == i)
            ordered_vals[i] = val_list[i];
        else
        {
            for(int k = i - 1 ; k >= pos ; --k)
            {
                ordered_vals[k+1] = ordered_vals[k];
            }// k
            ordered_vals[pos] = val_list[i];
        }
    }// i

    // return the middle value
    if(length % 2 != 0)
        return ordered_vals[(length-1)/2];
    else
        return (ordered_vals[(length/2)-1] + ordered_vals[length/2] + 1) >> 1;

}

/*************************************************************/


void VFilter(PicArray& pic_data, const OneDArray<int>& filter, const int bits);
void HFilter(PicArray& pic_data, const OneDArray<int>& filter, const int bits);

double sinxoverx(const double val)
{
    if(0.0f == val)
        return 1.0;
    else
        return sin(val) / val;

}

OneDArray<int> MakeLPRectFilter(const float bw, const int bits)
{
    const int tl = 8;
    const float pi = 3.1415926535;

    OneDArray<double> double_filter(Range(-tl, tl));
    OneDArray<int> int_filter(Range(-tl, tl));

    // Use the Hanning window
    for(int i = double_filter.First(); i <= double_filter.Last(); ++i)
    {
        double_filter[i] = cos((pi * i) /
                               (double_filter.Length() + 1));
    }

    // Apply sinc function
    for(int i = double_filter.First(); i <= double_filter.Last(); ++i)
    {
        double_filter[i] *= sinxoverx(pi * 1.0 * bw * i);
    }

    // Get DC gain = 1<<bits
    double sum = 0.0;
    for(int i = double_filter.First(); i <= double_filter.Last(); ++i)
        sum += double_filter[i];

    for(int i = double_filter.First(); i <= double_filter.Last(); ++i)
    {
        double_filter[i] *= double(1 << (bits + 4));
        double_filter[i] /= sum;
    }

    // Turn the float filter into an integer filter
    for(int i = double_filter.First(); i <= double_filter.Last(); ++i)
    {
        int_filter[i] = double_filter[i] > 0 ? int(double_filter[i] + 0.5) : -int(-double_filter[i] + 0.5);
        int_filter[i] = (int_filter[i] + 8) >> 4;
    }

    return int_filter;
}

void dirac::LPFilter(PicArray& pic_data, const float qf, const int strength)
{
    float bw = (std::min(std::max(qf + 3.0f - float(strength), 1.0f), 10.0f)) / 10.0;

    // filter with 14-bit accuracy
    OneDArray<int> filter = MakeLPRectFilter(bw, 14);

    HFilter(pic_data, filter, 14);
    VFilter(pic_data, filter, 14);

}

void HFilter(PicArray& pic_data, const OneDArray<int>& filter, const int bits)
{
    ValueType* line_data = new ValueType[pic_data.LengthX()];
    const int offset = (1 << (bits - 1));

    int sum;

    for(int j = 0; j < pic_data.LengthY(); ++j)
    {
        // Do the first bit
        for(int i = 0; i < filter.Last(); ++i)
        {
            sum = offset;
            for(int k = filter.Last(); k >= filter.First(); --k)
                sum += filter[k] * pic_data[j][std::max(i-k, 0)];
            sum >>= bits;
            sum = std::min(127, std::max(-128, sum));
            line_data[i] = ValueType(sum);
        }// i
        // Do the middle bit
        for(int i = filter.Last(); i <= pic_data.LastX() + filter.First(); ++i)
        {
            sum = offset;
            for(int k = filter.Last(); k >= filter.First(); --k)
                sum += filter[k] * pic_data[j][i-k];
            sum >>= bits;
            sum = std::min(127, std::max(-128, sum));
            line_data[i] = ValueType(sum);
        }// i
        // Do the last bit
        for(int i = pic_data.LastX() + filter.First() + 1; i < pic_data.LengthX(); ++i)
        {
            sum = offset;
            for(int k = filter.Last(); k >= filter.First(); --k)
                sum += filter[k] * pic_data[j][std::min(i-k, pic_data.LastX())];
            sum >>= bits;
            sum = std::min(127, std::max(-128, sum));
            line_data[i] = ValueType(sum);
        }// i

        // Copy data back

        for(int i = 0; i < pic_data.LengthX(); ++i)
            pic_data[j][i] = line_data[i];

    }// j

    delete[] line_data;
}

void VFilter(PicArray& pic_data, const OneDArray<int>& filter, const int bits)
{
    PicArray tmp_data(pic_data);
    const int offset = (1 << (bits - 1));

    int sum;

    // Do the first bit
    for(int j = 0; j < filter.Last(); ++j)
    {

        for(int i = 0; i < pic_data.LengthX(); ++i)
        {
            sum = offset;
            for(int k = filter.Last(); k >= filter.First(); --k)
                sum += filter[k] * pic_data[std::max(j-k, 0)][i];
            sum >>= bits;
            sum = std::min(127, std::max(-128, sum));
            tmp_data[j][i] = ValueType(sum);
        }// i

    }// j

    // Do the middle bit
    for(int j = filter.Last(); j <= pic_data.LastY() + filter.First(); ++j)
    {

        for(int i = 0; i < pic_data.LengthX(); ++i)
        {
            sum = offset;
            for(int k = filter.Last(); k >= filter.First(); --k)
                sum += filter[k] * pic_data[j-k][i];
            sum >>= bits;
            sum = std::min(127, std::max(-128, sum));
            tmp_data[j][i] = ValueType(sum);
        }// i

    }// j

    // Do the last bit
    for(int j = pic_data.LastY() + filter.First() + 1; j < pic_data.LengthY(); ++j)
    {

        for(int i = 0; i < pic_data.LengthX(); ++i)
        {
            sum = offset;
            for(int k = filter.Last(); k >= filter.First(); --k)
                sum += filter[k] * pic_data[std::min(j-k, pic_data.LastY())][i];
            sum >>= bits;
            sum = std::min(127, std::max(-128, sum));
            tmp_data[j][i] = ValueType(sum);
        }// i

    }// j

    // Copy data across
    pic_data = tmp_data;

}

/***************************************************************************/

ValueType DiagFilterBchkD(const PicArray& pic,
                          const int xpos, const int ypos,
                          const TwoDArray<int>& filter,
                          const int shift)
{
    // Half the filter length
    const int len2 = 6;

    const int height = pic.LengthY();
    const int width = pic.LengthX();

    int uplus, uneg, vplus, vneg;
    int val = (1 << (shift - 1));

    // Do 0 position horizontally
    val += filter[0][0] * pic[ypos][xpos];

    for(int i = 1; i <= len2; ++i)
    {
        uplus = xpos + i;
        uplus = (uplus >= width ? width - 1 : uplus);
        uneg = xpos - i;
        uneg = (uneg < 0 ? 0 : uneg);
        val += filter[0][i] * (pic[ypos][uplus] + pic[ypos][uneg]);
    }

    // Do other positions vertically//
    //////////////////////////////////

    for(int j = 1; j <= len2; ++j)
    {
        vplus = ypos + j;
        vplus = (vplus >= height ? height - 1 : vplus);
        vneg = ypos - j;
        vneg = (vneg < 0 ? 0 : vneg);

        // Do 0 position horizontally
        val += filter[j][0] * (pic[vneg][xpos] + pic[vplus][xpos]);
        for(int i = 1; i <= len2; ++i)
        {
            uplus = xpos + i;
            uplus = (uplus >= width ? width - 1 : uplus);
            uneg = xpos - i;
            uneg = (uneg < 0 ? 0 : uneg);
            val += filter[j][i] * (pic[vneg][uplus] + pic[vneg][uneg] +
                                   pic[vplus][uplus] + pic[vplus][uneg]);
        }
    }

    val >>= shift;

    return ValueType(val);
}

ValueType DiagFilterD(const PicArray& pic,
                      const int xpos, const int ypos,
                      const TwoDArray<int>& filter,
                      const int shift)
{
    // Half the filter length
    const int len2 = 6;

    int uplus, uneg, vplus, vneg;
    int val = (1 << (shift - 1));

    // Do 0 position horizontally
    val += filter[0][0] * pic[ypos][xpos];

    for(int i = 1; i <= len2; ++i)
    {
        uplus = xpos + i;
        uneg = xpos - i;
        val += filter[0][i] * (pic[ypos][uplus] + pic[ypos][uneg]);
    }

    // Do other positions vertically//
    //////////////////////////////////

    for(int j = 1; j <= len2; ++j)
    {
        vplus = ypos + j;
        vneg = ypos - j;

        // Do 0 position horizontally
        val += filter[j][0] * (pic[vneg][xpos] + pic[vplus][xpos]);
        for(int i = 1; i <= len2; ++i)
        {
            uplus = xpos + i;
            uneg = xpos - i;
            val += filter[j][i] * (pic[vneg][uplus] + pic[vneg][uneg] +
                                   pic[vplus][uplus] + pic[vplus][uneg]);
        }
    }

    val >>= shift;

    return ValueType(val);
}


TwoDArray<int> GetDiagLPFilter(const float bw)
{
    TwoDArray<int> f(7, 7);

    // Bandwidth quantised to range 0.2-1
    int qbf = int(bw * 10.0 + 0.5);
    qbf = std::min(std::max(qbf, 2) , 10);

    switch(qbf)
    {

    case 1 :
        f[0][0] = 1651;
        f[0][1] = 1544;
        f[0][2] = 1259;
        f[0][3] = 887;
        f[0][4] = 530;
        f[0][5] = 260;
        f[0][6] = 99;
        f[1][0] = 1544;
        f[1][1] = 1442;
        f[1][2] = 1170;
        f[1][3] = 817;
        f[1][4] = 480;
        f[1][5] = 229;
        f[1][6] = 83;
        f[2][0] = 1259;
        f[2][1] = 1170;
        f[2][2] = 935;
        f[2][3] = 634;
        f[2][4] = 354;
        f[2][5] = 153;
        f[2][6] = 45;
        f[3][0] = 887;
        f[3][1] = 817;
        f[3][2] = 634;
        f[3][3] = 405;
        f[3][4] = 202;
        f[3][5] = 70;
        f[3][6] = 11;
        f[4][0] = 530;
        f[4][1] = 480;
        f[4][2] = 354;
        f[4][3] = 202;
        f[4][4] = 80;
        f[4][5] = 15;
        f[4][6] = 0;
        f[5][0] = 260;
        f[5][1] = 229;
        f[5][2] = 153;
        f[5][3] = 70;
        f[5][4] = 15;
        f[5][5] = 0;
        f[5][6] = 0;
        f[6][0] = 99;
        f[6][1] = 83;
        f[6][2] = 45;
        f[6][3] = 11;
        f[6][4] = 0;
        f[6][5] = 0;
        f[6][6] = 0;

        break;

    case 2:

        f[0][0] = 2855;
        f[0][1] = 2540;
        f[0][2] = 1775;
        f[0][3] = 947;
        f[0][4] = 364;
        f[0][5] = 89;
        f[0][6] = 10;
        f[1][0] = 2540;
        f[1][1] = 2251;
        f[1][2] = 1551;
        f[1][3] = 804;
        f[1][4] = 290;
        f[1][5] = 59;
        f[1][6] = 1;
        f[2][0] = 1775;
        f[2][1] = 1551;
        f[2][2] = 1020;
        f[2][3] = 475;
        f[2][4] = 130;
        f[2][5] = 3;
        f[2][6] = -10;
        f[3][0] = 947;
        f[3][1] = 804;
        f[3][2] = 475;
        f[3][3] = 165;
        f[3][4] = 5;
        f[3][5] = -22;
        f[3][6] = -6;
        f[4][0] = 364;
        f[4][1] = 290;
        f[4][2] = 130;
        f[4][3] = 5;
        f[4][4] = -28;
        f[4][5] = -10;
        f[4][6] = 0;
        f[5][0] = 89;
        f[5][1] = 59;
        f[5][2] = 3;
        f[5][3] = -22;
        f[5][4] = -10;
        f[5][5] = 0;
        f[5][6] = 0;
        f[6][0] = 10;
        f[6][1] = 1;
        f[6][2] = -10;
        f[6][3] = -6;
        f[6][4] = 0;
        f[6][5] = 0;
        f[6][6] = 0;

        break;

    case 3:

        f[0][0] = 5767;
        f[0][1] = 4718;
        f[0][2] = 2498;
        f[0][3] = 745;
        f[0][4] = 72;
        f[0][5] = 5;
        f[0][6] = 23;
        f[1][0] = 4718;
        f[1][1] = 3796;
        f[1][2] = 1875;
        f[1][3] = 423;
        f[1][4] = -58;
        f[1][5] = -41;
        f[1][6] = 7;
        f[2][0] = 2498;
        f[2][1] = 1875;
        f[2][2] = 643;
        f[2][3] = -146;
        f[2][4] = -241;
        f[2][5] = -88;
        f[2][6] = -9;
        f[3][0] = 745;
        f[3][1] = 423;
        f[3][2] = -146;
        f[3][3] = -367;
        f[3][4] = -220;
        f[3][5] = -51;
        f[3][6] = -2;
        f[4][0] = 72;
        f[4][1] = -58;
        f[4][2] = -241;
        f[4][3] = -220;
        f[4][4] = -78;
        f[4][5] = -5;
        f[4][6] = 0;
        f[5][0] = 5;
        f[5][1] = -41;
        f[5][2] = -88;
        f[5][3] = -51;
        f[5][4] = -5;
        f[5][5] = 0;
        f[5][6] = 0;
        f[6][0] = 23;
        f[6][1] = 7;
        f[6][2] = -9;
        f[6][3] = -2;
        f[6][4] = 0;
        f[6][5] = 0;
        f[6][6] = 0;

        break;

    case 4:

        f[0][0] = 10534;
        f[0][1] = 7642;
        f[0][2] = 2603;
        f[0][3] = 194;
        f[0][4] = 56;
        f[0][5] = 120;
        f[0][6] = 28;
        f[1][0] = 7642;
        f[1][1] = 5237;
        f[1][2] = 1218;
        f[1][3] = -383;
        f[1][4] = -153;
        f[1][5] = 40;
        f[1][6] = 2;
        f[2][0] = 2603;
        f[2][1] = 1218;
        f[2][2] = -771;
        f[2][3] = -958;
        f[2][4] = -269;
        f[2][5] = -3;
        f[2][6] = -7;
        f[3][0] = 194;
        f[3][1] = -383;
        f[3][2] = -958;
        f[3][3] = -541;
        f[3][4] = -18;
        f[3][5] = 48;
        f[3][6] = 4;
        f[4][0] = 56;
        f[4][1] = -153;
        f[4][2] = -269;
        f[4][3] = -18;
        f[4][4] = 96;
        f[4][5] = 22;
        f[4][6] = 0;
        f[5][0] = 120;
        f[5][1] = 40;
        f[5][2] = -3;
        f[5][3] = 48;
        f[5][4] = 22;
        f[5][5] = 0;
        f[5][6] = 0;
        f[6][0] = 28;
        f[6][1] = 2;
        f[6][2] = -7;
        f[6][3] = 4;
        f[6][4] = 0;
        f[6][5] = 0;
        f[6][6] = 0;

        break;

    case 5 :

        f[0][0] = 16421;
        f[0][1] = 10159;
        f[0][2] = 1716;
        f[0][3] = 33;
        f[0][4] = 325;
        f[0][5] = 57;
        f[0][6] = 6;
        f[1][0] = 10159;
        f[1][1] = 5309;
        f[1][2] = -580;
        f[1][3] = -747;
        f[1][4] = 44;
        f[1][5] = -43;
        f[1][6] = -25;
        f[2][0] = 1716;
        f[2][1] = -580;
        f[2][2] = -2310;
        f[2][3] = -763;
        f[2][4] = 100;
        f[2][5] = -19;
        f[2][6] = -12;
        f[3][0] = 33;
        f[3][1] = -747;
        f[3][2] = -763;
        f[3][3] = 308;
        f[3][4] = 326;
        f[3][5] = 27;
        f[3][6] = 1;
        f[4][0] = 325;
        f[4][1] = 44;
        f[4][2] = 100;
        f[4][3] = 326;
        f[4][4] = 84;
        f[4][5] = -14;
        f[4][6] = 0;
        f[5][0] = 57;
        f[5][1] = -43;
        f[5][2] = -19;
        f[5][3] = 27;
        f[5][4] = -14;
        f[5][5] = 0;
        f[5][6] = 0;
        f[6][0] = 6;
        f[6][1] = -25;
        f[6][2] = -12;
        f[6][3] = 1;
        f[6][4] = 0;
        f[6][5] = 0;
        f[6][6] = 0;

        break;

    case 6 :

        f[0][0] = 23511;
        f[0][1] = 11883;
        f[0][2] = 566;
        f[0][3] = 524;
        f[0][4] = 231;
        f[0][5] = 18;
        f[0][6] = 41;
        f[1][0] = 11883;
        f[1][1] = 3647;
        f[1][2] = -2496;
        f[1][3] = -361;
        f[1][4] = -96;
        f[1][5] = -97;
        f[1][6] = 1;
        f[2][0] = 566;
        f[2][1] = -2496;
        f[2][2] = -2329;
        f[2][3] = 459;
        f[2][4] = 152;
        f[2][5] = -7;
        f[2][6] = 18;
        f[3][0] = 524;
        f[3][1] = -361;
        f[3][2] = 459;
        f[3][3] = 979;
        f[3][4] = 33;
        f[3][5] = -28;
        f[3][6] = 3;
        f[4][0] = 231;
        f[4][1] = -96;
        f[4][2] = 152;
        f[4][3] = 33;
        f[4][4] = -184;
        f[4][5] = -15;
        f[4][6] = 0;
        f[5][0] = 18;
        f[5][1] = -97;
        f[5][2] = -7;
        f[5][3] = -28;
        f[5][4] = -15;
        f[5][5] = 0;
        f[5][6] = 0;
        f[6][0] = 41;
        f[6][1] = 1;
        f[6][2] = 18;
        f[6][3] = 3;
        f[6][4] = 0;
        f[6][5] = 0;
        f[6][6] = 0;

        break;

    case 7 :

        f[0][0] = 32188;
        f[0][1] = 12652;
        f[0][2] = 3;
        f[0][3] = 921;
        f[0][4] = 1;
        f[0][5] = 128;
        f[0][6] = 0;
        f[1][0] = 12652;
        f[1][1] = 295;
        f[1][2] = -3414;
        f[1][3] = -2;
        f[1][4] = -343;
        f[1][5] = -1;
        f[1][6] = -37;
        f[2][0] = 3;
        f[2][1] = -3414;
        f[2][2] = -212;
        f[2][3] = 1273;
        f[2][4] = 1;
        f[2][5] = 98;
        f[2][6] = 0;
        f[3][0] = 921;
        f[3][1] = -2;
        f[3][2] = 1273;
        f[3][3] = 110;
        f[3][4] = -363;
        f[3][5] = 0;
        f[3][6] = -8;
        f[4][0] = 1;
        f[4][1] = -343;
        f[4][2] = 1;
        f[4][3] = -363;
        f[4][4] = -29;
        f[4][5] = 29;
        f[4][6] = 0;
        f[5][0] = 128;
        f[5][1] = -1;
        f[5][2] = 98;
        f[5][3] = 0;
        f[5][4] = 29;
        f[5][5] = 0;
        f[5][6] = 0;
        f[6][0] = 0;
        f[6][1] = -37;
        f[6][2] = 0;
        f[6][3] = -8;
        f[6][4] = 0;
        f[6][5] = 0;
        f[6][6] = 0;

        break;

    case 8 :

        f[0][0] = 41902;
        f[0][1] = 12084;
        f[0][2] = 435;
        f[0][3] = 610;
        f[0][4] = 188;
        f[0][5] = 34;
        f[0][6] = 37;
        f[1][0] = 12084;
        f[1][1] = -4268;
        f[1][2] = -2715;
        f[1][3] = -286;
        f[1][4] = -144;
        f[1][5] = -84;
        f[1][6] = -2;
        f[2][0] = 435;
        f[2][1] = -2715;
        f[2][2] = 2809;
        f[2][3] = 640;
        f[2][4] = 127;
        f[2][5] = 10;
        f[2][6] = 17;
        f[3][0] = 610;
        f[3][1] = -286;
        f[3][2] = 640;
        f[3][3] = -1250;
        f[3][4] = -45;
        f[3][5] = -26;
        f[3][6] = 2;
        f[4][0] = 188;
        f[4][1] = -144;
        f[4][2] = 127;
        f[4][3] = -45;
        f[4][4] = 259;
        f[4][5] = -8;
        f[4][6] = 0;
        f[5][0] = 34;
        f[5][1] = -84;
        f[5][2] = 10;
        f[5][3] = -26;
        f[5][4] = -8;
        f[5][5] = 0;
        f[5][6] = 0;
        f[6][0] = 37;
        f[6][1] = -2;
        f[6][2] = 17;
        f[6][3] = 2;
        f[6][4] = 0;
        f[6][5] = 0;
        f[6][6] = 0;

        break;

    case 9 :

        f[0][0] = 53098;
        f[0][1] = 10449;
        f[0][2] = 1546;
        f[0][3] = 73;
        f[0][4] = 342;
        f[0][5] = 38;
        f[0][6] = 12;
        f[1][0] = 10449;
        f[1][1] = -9060;
        f[1][2] = -873;
        f[1][3] = -727;
        f[1][4] = 52;
        f[1][5] = -65;
        f[1][6] = -20;
        f[2][0] = 1546;
        f[2][1] = -873;
        f[2][2] = 4261;
        f[2][3] = -627;
        f[2][4] = 137;
        f[2][5] = -27;
        f[2][6] = -7;
        f[3][0] = 73;
        f[3][1] = -727;
        f[3][2] = -627;
        f[3][3] = -804;
        f[3][4] = 328;
        f[3][5] = 14;
        f[3][6] = 2;
        f[4][0] = 342;
        f[4][1] = 52;
        f[4][2] = 137;
        f[4][3] = 328;
        f[4][4] = -83;
        f[4][5] = -20;
        f[4][6] = 0;
        f[5][0] = 38;
        f[5][1] = -65;
        f[5][2] = -27;
        f[5][3] = 14;
        f[5][4] = -20;
        f[5][5] = 0;
        f[5][6] = 0;
        f[6][0] = 12;
        f[6][1] = -20;
        f[6][2] = -7;
        f[6][3] = 2;
        f[6][4] = 0;
        f[6][5] = 0;
        f[6][6] = 0;

        break;

    default:// case 10

        for(int j = 0; j < f.LengthY(); ++j)
        {
            for(int i = 0; i < f.LengthX(); ++i)
            {
                f[j][i] = 0;
            }
        }
        f[0][0] = 65536;

    }

    return f;

}


// Does a diagnonal prefilter
void dirac::DiagFilter(PicArray& pic_data, const float qf, const int strength)
{
    // One quadrant of the filter taps


    float ffactor = (8.0 + strength - 4.0 - qf) / 5.0;
    int factor = std::max(0, std::min(256, int(ffactor * 256.0))) ;

    float bw = (1.0 - ffactor) * 0.6 + 0.4;

    //std::cout<<std::endl<<"Diagonal prefiltering with bandwidth = "<<bw;

    if(bw > 0.9)
        return;

    TwoDArray<int> filter = GetDiagLPFilter(bw);

    filter[0][0] = (factor * filter[0][0] + ((1 << 8) - factor) * (1 << 16) + (1 << 7)) >> 8;

    for(int i = 1;
        i < 7;
        ++i)
        filter[0][i] = (factor * filter[0][i] + (1 << 7)) >> 8;

    for(int j = 1; j < 7; ++j)
        for(int i = 0; i < 7; ++i)
            filter[j][i] = (factor * filter[j][i] + (1 << 7)) >> 8;


    PicArray tmp_data(pic_data.LengthY(), pic_data.LengthX(), pic_data.CSort());

    const int shift = 16;

    for(int j = 0; j < 7; ++j)
        for(int i = 0; i < pic_data.LengthX(); ++i)
            tmp_data[j][i] = DiagFilterBchkD(pic_data, i, j, filter, shift);

    for(int j = 7; j < pic_data.LengthY() - 7; ++j)
    {
        for(int i = 0; i < 7; ++i)
            tmp_data[j][i] = DiagFilterBchkD(pic_data, i, j, filter, shift);

        for(int i = 7; i < pic_data.LengthX() - 7; ++i)
            tmp_data[j][i] = DiagFilterD(pic_data, i, j, filter, shift);

        for(int i = pic_data.LengthX() - 7; i < pic_data.LengthX(); ++i)
            tmp_data[j][i] = DiagFilterBchkD(pic_data, i, j, filter, shift);

    }

    for(int j = pic_data.LengthY() - 7; j < pic_data.LengthY(); ++j)
        for(int i = 0; i < pic_data.LengthX(); ++i)
            tmp_data[j][i] = DiagFilterBchkD(pic_data, i, j, filter, shift);

    pic_data = tmp_data;

}

