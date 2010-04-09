/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: enc_picture.cpp,v 1.6 2008/10/01 01:26:47 asuraparaju Exp $ $Name:  $
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

#include <libdirac_encoder/enc_picture.h>
#include <libdirac_common/upconvert.h>

using namespace dirac;

EncPicture::EncPicture( const PictureParams& pp):
    Picture( pp ),
    m_me_data( NULL ),
    m_status( NO_ENC ),
    m_complexity( 0.0 ),
    m_norm_complexity( 1.0 ),
    m_pred_bias(0.5)
{
    for (int c=0; c<3; ++c ){
        m_orig_data[c] = new PicArray( m_pic_data[c]->LengthY(), m_pic_data[c]->LengthX() );
        m_orig_up_data[c] = NULL;
	m_filt_data[c] = NULL;
	m_filt_up_data[c] = NULL;
    }
}

void EncPicture::ClearData(){

    Picture::ClearData();

    for (int c=0;c<3;++c){
        if (m_orig_data[c] != NULL){
            delete m_orig_data[c];
            m_orig_data[c] = NULL;
        }

        if (m_orig_up_data[c] != NULL){
            delete m_orig_up_data[c];
            m_orig_up_data[c] = NULL;
        }

        if (m_filt_data[c] != NULL){
            delete m_filt_data[c];
            m_filt_data[c] = NULL;
        }

        if (m_filt_up_data[c] != NULL){
            delete m_filt_up_data[c];
            m_filt_up_data[c] = NULL;
        }
    }

    if ( m_me_data != NULL )
        delete m_me_data;
}

EncPicture::~EncPicture()
{
    ClearData();
}

void EncPicture::SetOrigData()
{
    for ( int c=0; c<3 ; ++c )
        SetOrigData(c);
}

void EncPicture::SetOrigData( const int c )
{
    if ( m_pic_data[c] != NULL )
        *(m_orig_data[c]) = *(m_pic_data[c]);
}

void EncPicture::InitMEData( const PicturePredParams& predparams , const int num_refs)
{
    if (m_me_data != NULL)
        delete m_me_data;

    m_me_data=new MEData( predparams, num_refs );
}

const PicArray& EncPicture::DataForME( bool combined_me ) const{

    if (combined_me)
        return CombinedData();
    else
        return OrigData( Y_COMP );
}

const PicArray& EncPicture::UpDataForME( bool combined_me ) const{

    if (combined_me)
        return UpCombinedData();
    else
        return UpOrigData( Y_COMP );
}


const PicArray& EncPicture::UpOrigData(CompSort cs) const
{
    const int c = (int) cs;

    if (m_orig_up_data[c] != NULL)
        return *m_orig_up_data[c];
    else
    {//we have to do the upconversion

        m_orig_up_data[c] = new PicArray( 2*m_orig_data[c]->LengthY(),
                                          2*m_orig_data[c]->LengthX() );
        UpConverter* myupconv;
	if (c>0)
            myupconv = new UpConverter(-(1 << (m_pparams.ChromaDepth()-1)), 
                                      (1 << (m_pparams.ChromaDepth()-1))-1,
                                      m_pparams.ChromaXl(), m_pparams.ChromaYl());
        else
            myupconv = new UpConverter(-(1 << (m_pparams.LumaDepth()-1)), 
                                      (1 << (m_pparams.LumaDepth()-1))-1,
                                      m_pparams.Xl(), m_pparams.Yl());

        myupconv->DoUpConverter( *(m_orig_data[c]) , *(m_orig_up_data[c]) );

	delete myupconv;

        return *(m_orig_up_data[c]);

    }
}

const PicArray& EncPicture::FiltData(CompSort cs) const
{
    const int c = (int) cs;

    if (m_filt_data[c] != NULL)
        return *m_filt_data[c];
    else
    {//we have to do the filtering

        if (m_orig_data[c] != NULL )
            m_filt_data[c] = new PicArray( m_orig_data[c]->LengthY(),
                                           m_orig_data[c]->LengthX() );

	AntiAliasFilter( *(m_filt_data[c]), *(m_orig_data[c]));

        return *(m_filt_data[c]);

    }
}

const PicArray& EncPicture::UpFiltData(CompSort cs) const
{
    const int c = (int) cs;

    if (m_filt_up_data[c] != NULL)
        return *m_filt_up_data[c];
    else
    {//we have to do the upconversion

        const PicArray& filt_data = FiltData( cs );

        m_filt_up_data[c] = new PicArray( 2*filt_data.LengthY(),
                                          2*filt_data.LengthX() );
        UpConverter* myupconv;
	if (c>0)
            myupconv = new UpConverter(-(1 << (m_pparams.ChromaDepth()-1)),
                                      (1 << (m_pparams.ChromaDepth()-1))-1,
                                      m_pparams.ChromaXl(), m_pparams.ChromaYl());
        else
            myupconv = new UpConverter(-(1 << (m_pparams.LumaDepth()-1)),
                                      (1 << (m_pparams.LumaDepth()-1))-1,
                                      m_pparams.Xl(), m_pparams.Yl());

        myupconv->DoUpConverter( filt_data , *(m_filt_up_data[c]) );

	delete myupconv;

        return *(m_filt_up_data[c]);

    }
}

void EncPicture::AntiAliasFilter( PicArray& out_data, const PicArray& in_data ) const{

    //Special case for first row
    for (int i = in_data.FirstX(); i <= in_data.LastX(); ++i)
    {
        out_data[in_data.FirstY()][i] = (3*in_data[in_data.FirstY()][i] +
                                  in_data[in_data.FirstY()+1][i] +2 )>>2;
    }
    //Middle section
    for (int j = in_data.FirstY()+1; j < in_data.LastY(); ++j)
    {
        for (int i = in_data.FirstX(); i <= in_data.LastX(); ++i)
        {
            out_data[j][i] = (in_data[j-1][i] + 2*in_data[j][i] + in_data[j+1][i] + 2)>>2;
        }
    }
    //Special case for last row
    for (int i = in_data.FirstX(); i <= in_data.LastX(); ++i)
    {
        out_data[in_data.LastY()][i] = (in_data[in_data.LastY()-1][i] +
                                 3*in_data[in_data.LastY()][i] + 2)>>2;
    }
}

const PicArray& EncPicture::CombinedData() const
{

    if (m_filt_data[Y_COMP] != NULL)
        return *m_filt_data[Y_COMP];
    else
    {//we have to do the combining

        if (m_orig_data[Y_COMP] != NULL )
            m_filt_data[Y_COMP] = new PicArray( m_orig_data[Y_COMP]->LengthY(),
                                           m_orig_data[Y_COMP]->LengthX() );

	Combine( *(m_filt_data[Y_COMP]), *(m_orig_data[Y_COMP]),
	         *(m_orig_data[U_COMP]), *(m_orig_data[V_COMP])
	);

        return *(m_filt_data[Y_COMP]);

    }
}

const PicArray& EncPicture::UpCombinedData() const
{
    if (m_filt_up_data[Y_COMP] != NULL)
        return *m_filt_up_data[Y_COMP];
    else
    {//we have to do the upconversion

        const PicArray& filt_data = CombinedData();

        m_filt_up_data[Y_COMP] = new PicArray( 2*filt_data.LengthY(),
                                          2*filt_data.LengthX() );
        UpConverter* myupconv;
        myupconv = new UpConverter(-(1 << (m_pparams.LumaDepth()-1)),
                                      (1 << (m_pparams.LumaDepth()-1))-1,
                                      m_pparams.Xl(), m_pparams.Yl());

        myupconv->DoUpConverter( filt_data , *(m_filt_up_data[Y_COMP]) );

	delete myupconv;

        return *(m_filt_up_data[Y_COMP]);

    }
}



void EncPicture::Combine( PicArray& comb_data, const PicArray& y_data,
                          const PicArray& u_data, const PicArray& v_data ) const
{
    int hcr = y_data.LengthX()/u_data.LengthX();
    int vcr = y_data.LengthY()/u_data.LengthY();

    float val, valc, valy;

    if (vcr==1){
        for (int j=0; j<comb_data.LengthY(); ++j) {
            if (hcr==1){// 444 format
                for (int i=0; i<comb_data.LengthX(); ++i ){
                    val = float(u_data[j][i]);
	            val *= val;
	            valc = val;

	            val = float(v_data[j][i]);
	            val *= val;
	            valc += val;

                    valy = float(y_data[j][i]) + 128.0;
	            valy *= valy;
		    comb_data[j][i] = ValueType( std::sqrt(valc+valy)-128.0 );
                }// i
	    }
	    else{ // 422 format
                for (int i=0; i<comb_data.LengthX(); i+=2 ){

                    val = float(u_data[j][i>>1]);
	            val *= val;
	            valc = val;

	            val = float(v_data[j][i>>1]);
	            val *= val;
	            valc += val;

                    valy = float(y_data[j][i]) + 128.0;
	            valy *= valy;
	            comb_data[j][i] = ValueType( std::sqrt(valc+valy)-128.0 );

		    valy = float(y_data[j][i+1]) + 128.0;
	            valy *= valy;

		    comb_data[j][i+1] = ValueType( std::sqrt(valc+valy)-128.0 );

	        }// i
	    }
        }// j
    }
    else{ // 420 format
        for (int j=0; j<comb_data.LengthY(); j+=2 ) {

            for (int i=0; i<comb_data.LengthX(); i+=2 ){

                val = float(u_data[j>>1][i>>1]);
	        val *= val;
	        valc = val;

	        val = float(v_data[j>>1][i>>1]);
	        val *= val;
	        valc += val;

                valy = float(y_data[j][i]) + 128.0;
	        valy *= valy;
	        comb_data[j][i] = ValueType( std::sqrt(valc+valy)-128.0 );

	        valy = float(y_data[j][i+1]) + 128.0;
	        valy *= valy;
		comb_data[j][i+1] = ValueType( std::sqrt(valc+valy)-128.0 );

                valy = float(y_data[j+1][i]) + 128.0;
	        valy *= valy;
		comb_data[j+1][i] = ValueType( std::sqrt(valc+valy)-128.0 );

	        valy = float(y_data[j+1][i+1]) + 128.0;
	        valy *= valy;
		comb_data[j+1][i+1] = ValueType( std::sqrt(valc+valy)-128.0 );

	    }// i
        }// j

    }


}


void EncPicture::DropRef( int rindex ){

    std::vector<int>& refs = m_pparams.Refs();

    if (rindex==1 || rindex==2 )
        refs.erase( refs.begin()+rindex-1 );

    // Now reconfigure the motion data
    if ( m_me_data!=NULL )
        m_me_data->DropRef( rindex );

}
