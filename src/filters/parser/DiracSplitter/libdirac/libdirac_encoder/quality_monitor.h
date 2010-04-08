/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: quality_monitor.h,v 1.19 2008/08/14 02:30:50 asuraparaju Exp $ $Name:  $
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

#ifndef _QUALITY_MONITOR_H_
#define _QUALITY_MONITOR_H_

#include <libdirac_common/common.h>
#include <libdirac_encoder/enc_picture.h>
#include <libdirac_common/wavelet_utils.h>
namespace dirac
{

//! Class to monitor the quality of pictures and adjust coding parameters appropriately
class QualityMonitor
{
public:

    //! Constructor. Sets up initial Lagrangian values
    /*
       Constructor sets up initial Lagrangian values.
    */
    QualityMonitor(EncoderParams& ep);


    //! Destructor
    ~QualityMonitor();

    ////////////////////////////////////////////////////////////
    //                                                        //
    //    Assumes default copy constructor,  assignment =     //
    //                 and destructor                         //
    ////////////////////////////////////////////////////////////

    //! Update the mse factors, returning true if we need to recode
    /*!
        Update the mse factors, returning true if we need to recode
        \param enc_picture the picture being encoded
    */
    void UpdateModel(const EncPicture& enc_picture);

    //! Reset the quality factors (say if there's been a cut)
    void ResetAll();

    //! Write a log of the quality to date
    void WriteLog();

private:
    //functions


    //! Calculate the quality of coded wrt original picture
    double QualityVal(const PicArray& coded_data ,
                      const PicArray& orig_data,
                      const int xlen,
                      const int ylen);

    //member variables//
    ////////////////////

    //! A reference to the encoder parameters
    EncoderParams& m_encparams;

    //! The overall average Y mse
    long double m_totalmse_averageY;

    //! The overall average U mse
    long double m_totalmse_averageU;

    //! The overall average V mse
    long double m_totalmse_averageV;

    //! The total number of pictures coded
    int m_allpicture_total;

    //! The average Y mse for the picture types
    OneDArray<long double> m_mse_averageY;

    //! The average U mse for the picture types
    OneDArray<long double> m_mse_averageU;

    //! The average V mse for the picture types
    OneDArray<long double> m_mse_averageV;

    //! The number of pictures of each type
    OneDArray<int> m_picture_total;
};

} // namespace dirac

#endif
