/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: quality_monitor.cpp,v 1.23 2007/04/11 08:08:49 tjdwave Exp $ $Name: Dirac_0_7_0 $
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

QualityMonitor::QualityMonitor(EncoderParams& encp, 
                               const SeqParams& sparams)
:
    m_encparams(encp),
    m_sparams( sparams ),
    m_quality_averageY(3),
    m_quality_averageU(3),
    m_quality_averageV(3),
    m_frame_total(3)
{
    ResetAll();
}

QualityMonitor::~QualityMonitor()
{}

void QualityMonitor::ResetAll()
{

    for (int i=0; i<3 ; ++i )
    {
        m_quality_averageY[i] = 0.0;
        m_quality_averageU[i] = 0.0;
        m_quality_averageV[i] = 0.0;
        m_frame_total[i] = 0;
    }// i
    m_totalquality_averageY = 0.0;
    m_totalquality_averageU = 0.0;
    m_totalquality_averageV = 0.0;
    m_allframe_total = 0;
}

void QualityMonitor::WriteLog()
{
    std::cout<<std::endl<<"Overall mean PSNR values";
    std::cout<<std::endl<<"------------------------";
    std::cout<<std::endl<<"Y: ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<m_totalquality_averageY/m_allframe_total<<std::endl;
    std::cout<<std::endl<<"U: ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<m_totalquality_averageU/m_allframe_total<<std::endl;
    std::cout<<std::endl<<"V: ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<m_totalquality_averageV/m_allframe_total<<std::endl;

     
    std::cout<<std::endl<<"Mean PSNR values by frame type and component";
    std::cout<<std::endl<<"--------------------------------------------";
    std::cout<<std::endl;
    
    std::cout<<std::endl<<"                 ||       Y       ||       U       ||       V       ||";
    std::cout<<std::endl<<"=================||===================================================";
    std::cout<<std::endl<<"           Intra ||     ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<m_quality_averageY[0]/m_frame_total[0]<<"     ||     ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<m_quality_averageU[0]/m_frame_total[0]<<"     ||     ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<m_quality_averageV[0]/m_frame_total[0]<<"     ||    ";
    std::cout<<std::endl<<"-----------------||---------------------------------------------------";
    std::cout<<std::endl<<"       Inter Ref ||     ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<m_quality_averageY[1]/m_frame_total[1]<<"     ||     ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<m_quality_averageU[1]/m_frame_total[1]<<"     ||     ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<m_quality_averageV[1]/m_frame_total[1]<<"     ||    ";
    std::cout<<std::endl<<"-----------------||---------------------------------------------------";
    std::cout<<std::endl<<"   Inter Non Ref ||     ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<m_quality_averageY[2]/m_frame_total[2]<<"     ||     ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<m_quality_averageU[2]/m_frame_total[2]<<"     ||     ";
    std::cout.width(5);std::cout.precision(4);
    std::cout<<m_quality_averageV[2]/m_frame_total[2]<<"     ||     ";
    std::cout<<std::endl<<"-----------------||---------------------------------------------------";
}

void QualityMonitor::UpdateModel(const Frame& ld_frame, const Frame& orig_frame )
{
    const FrameSort& fsort = ld_frame.GetFparams().FSort();    
    int idx = fsort.IsIntra() ? 0 : (fsort.IsRef() ? 1 : 2);

    double fqualityY, fqualityU, fqualityV;
    
    fqualityY = QualityVal( ld_frame.Ydata() , orig_frame.Ydata(), 
                            m_sparams.Xl(), m_sparams.Yl() );
    m_quality_averageY[idx] += fqualityY;
    m_totalquality_averageY += fqualityY;

    fqualityU = QualityVal( ld_frame.Udata() , orig_frame.Udata(), 
                          m_sparams.ChromaWidth(), m_sparams.ChromaHeight() );
    m_quality_averageU[idx] += fqualityU;
    m_totalquality_averageU += fqualityU;    

    fqualityV = QualityVal( ld_frame.Vdata() , orig_frame.Vdata(), 
                          m_sparams.ChromaWidth(), m_sparams.ChromaHeight() );
    m_quality_averageV[idx] += fqualityV;
    m_totalquality_averageV += fqualityV;    

    m_frame_total[idx]++;
    m_allframe_total++;
    
    if (m_encparams.Verbose() )
    {
        std::cout<<std::endl<<"Frame PSNR: Y="<<fqualityY;
        std::cout<<", U="<<fqualityU;
        std::cout<<", V="<<fqualityV;
    }

}


double QualityMonitor::QualityVal(const PicArray& coded_data, const PicArray& orig_data, 
const int xlen, const int ylen )
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

    const double max = double( (1<<m_sparams.GetVideoDepth())-1 );

    sum_sq_diff /= xlen*ylen;

    return static_cast<double> ( 10.0 * std::log10( max*max / sum_sq_diff ) );
}
