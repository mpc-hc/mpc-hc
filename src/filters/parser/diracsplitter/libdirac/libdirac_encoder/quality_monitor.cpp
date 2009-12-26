/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: quality_monitor.cpp,v 1.33 2008/08/14 01:04:26 asuraparaju Exp $ $Name:  $
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

#include <libdirac_encoder/quality_monitor.h>
#include <libdirac_common/wavelet_utils.h>
using namespace dirac;

using std::log10;

QualityMonitor::QualityMonitor(EncoderParams& encp) :
    m_encparams(encp),
    m_mse_averageY(3),
    m_mse_averageU(3),
    m_mse_averageV(3),
    m_picture_total(3)
{
    ResetAll();
}

QualityMonitor::~QualityMonitor()
{}

void QualityMonitor::ResetAll()
{

    for (int i=0; i<3 ; ++i )
    {
        m_mse_averageY[i] = 0.0;
        m_mse_averageU[i] = 0.0;
        m_mse_averageV[i] = 0.0;
        m_picture_total[i] = 0;
    }// i
    m_totalmse_averageY = 0.0;
    m_totalmse_averageU = 0.0;
    m_totalmse_averageV = 0.0;
    m_allpicture_total = 0;
}

void QualityMonitor::WriteLog()
{
    const double Ymax = double( (1<<m_encparams.LumaDepth())-1 );
    const double UVmax = double( (1<<m_encparams.ChromaDepth())-1 );

    std::cout<<std::endl<<"Overall mean PSNR values";
    std::cout<<std::endl<<"------------------------";
    std::cout<<std::endl<<"Y: ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<10*std::log10(Ymax*Ymax/(m_totalmse_averageY/m_allpicture_total))<<std::endl;
    std::cout<<std::endl<<"U: ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<10*std::log10(UVmax*UVmax/(m_totalmse_averageU/m_allpicture_total))<<std::endl;
    std::cout<<std::endl<<"V: ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<10*std::log10(UVmax*UVmax/(m_totalmse_averageV/m_allpicture_total))<<std::endl;


    std::cout<<std::endl<<"Mean PSNR values by picture type and component";
    std::cout<<std::endl<<"--------------------------------------------";
    std::cout<<std::endl;

    std::cout<<std::endl<<"                 ||       Y       ||       U       ||       V       ||";
    std::cout<<std::endl<<"=================||===================================================";
    std::cout<<std::endl<<"           Intra ||     ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<10*std::log10(Ymax*Ymax/(m_mse_averageY[0]/m_picture_total[0]))<<"     ||     ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<10*std::log10(UVmax*UVmax/(m_mse_averageU[0]/m_picture_total[0]))<<"     ||     ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<10*std::log10(UVmax*UVmax/(m_mse_averageV[0]/m_picture_total[0]))<<"     ||    ";
    std::cout<<std::endl<<"-----------------||---------------------------------------------------";
    std::cout<<std::endl<<"       Inter Ref ||     ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<10*std::log10(Ymax*Ymax/(m_mse_averageY[1]/m_picture_total[1]))<<"     ||     ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<10*std::log10(UVmax*UVmax/(m_mse_averageU[1]/m_picture_total[1]))<<"     ||     ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<10*std::log10(UVmax*UVmax/(m_mse_averageV[1]/m_picture_total[1]))<<"     ||    ";
    std::cout<<std::endl<<"-----------------||---------------------------------------------------";
    std::cout<<std::endl<<"   Inter Non Ref ||     ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<10*std::log10(Ymax*Ymax/(m_mse_averageY[2]/m_picture_total[2]))<<"     ||     ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<10*std::log10(UVmax*UVmax/(m_mse_averageU[2]/m_picture_total[2]))<<"     ||     ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<10*std::log10(UVmax*UVmax/(m_mse_averageV[2]/m_picture_total[2]))<<"     ||     ";
    std::cout<<std::endl<<"-----------------||---------------------------------------------------";
}

void QualityMonitor::UpdateModel(const EncPicture& enc_picture )
{
    const double Ymax = double( (1<<m_encparams.LumaDepth())-1 );
    const double UVmax = double( (1<<m_encparams.ChromaDepth())-1 );

    const PictureSort& psort = enc_picture.GetPparams().PicSort();
    int idx = psort.IsIntra() ? 0 : (psort.IsRef() ? 1 : 2);

    double fmseY, fmseU, fmseV;

    fmseY = QualityVal( enc_picture.Data(Y_COMP) , enc_picture.OrigData(Y_COMP),
                            m_encparams.Xl(), m_encparams.Yl());
    m_mse_averageY[idx] += fmseY;
    m_totalmse_averageY += fmseY;

    fmseU = QualityVal( enc_picture.Data(U_COMP) , enc_picture.OrigData(U_COMP),
                            m_encparams.ChromaXl(),
                            m_encparams.ChromaYl());
    m_mse_averageU[idx] += fmseU;
    m_totalmse_averageU += fmseU;

    fmseV = QualityVal( enc_picture.Data(V_COMP) , enc_picture.OrigData(V_COMP),
                            m_encparams.ChromaXl(),
                            m_encparams.ChromaYl());
    m_mse_averageV[idx] += fmseV;
    m_totalmse_averageV += fmseV;

    m_picture_total[idx]++;
    m_allpicture_total++;

    if (m_encparams.Verbose() )
    {
        std::cout<<std::endl<< (!m_encparams.FieldCoding() ? "Frame" : "Field");
        std::cout << " PSNR: Y="<<10.0 * std::log10( Ymax*Ymax / fmseY );
        std::cout<<", U="<<10.0 * std::log10( UVmax*UVmax / fmseU );
        std::cout<<", V="<<10.0 * std::log10( UVmax*UVmax / fmseV );
    }

}


double QualityMonitor::QualityVal(const PicArray& coded_data,
                                  const PicArray& orig_data,
                                  const int xlen, const int ylen)
{
    long double sum_sq_diff = 0.0;
    double diff;
    for ( int j=0;j<ylen; ++j )
    {
        for ( int i=0;i<xlen; ++i )
        {
            diff = orig_data[j][i] - coded_data[j][i];
            sum_sq_diff += diff*diff;

        }// i
    }// j


    sum_sq_diff /= xlen*ylen;

    return (double) sum_sq_diff;
}
