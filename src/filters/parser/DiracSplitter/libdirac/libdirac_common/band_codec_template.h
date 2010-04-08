
#include <libdirac_common/band_codec.h>
#include <libdirac_byteio/subband_byteio.h>

using namespace dirac;

//! Constructor for encoding.
template<typename EntropyCodec>
GenericBandCodec<EntropyCodec>::GenericBandCodec(SubbandByteIO* subband_byteio,
        size_t number_of_contexts,
        const SubbandList & band_list,
        int band_num,
        const bool is_intra):
    EntropyCodec(subband_byteio, number_of_contexts),
    m_is_intra(is_intra),
    m_bnum(band_num),
    m_node(band_list(band_num)),
    m_last_qf_idx(m_node.QuantIndex())
{
    if(m_node.Parent() != 0)
        m_pnode = band_list(m_node.Parent());
}


//encoding function
template<typename EntropyCodec>
void GenericBandCodec<EntropyCodec>::DoWorkCode(CoeffArray& in_data)
{

    const TwoDArray<CodeBlock>& block_list(m_node.GetCodeBlocks());

    // coeff blocks can be skipped only if SpatialPartitioning is
    // enabled i.e. more than one code-block per subband
    bool code_skip = (block_list.LengthX() > 1 || block_list.LengthY() > 1);
    // Now loop over the blocks and code
    for(int j = block_list.FirstY() ; j <= block_list.LastY() ; ++j)
    {
        CodeBlock *block = block_list[j];
        for(int i = block_list.FirstX() ; i <= block_list.LastX() ; ++i)
        {
            if(code_skip)
                EntropyCodec::EncodeSymbol(block[i].Skipped() , BLOCK_SKIP_CTX);
            if(!block[i].Skipped())
                CodeCoeffBlock(block[i] , in_data);
            else
                ClearBlock(block[i] , in_data);
        }// i
    }// j

}

template<typename EntropyCodec>
void GenericBandCodec<EntropyCodec>::CodeCoeffBlock(const CodeBlock& code_block , CoeffArray& in_data)
{
    //main coding function, using binarisation

    const int xbeg = code_block.Xstart();
    const int ybeg = code_block.Ystart();
    const int xend = code_block.Xend();
    const int yend = code_block.Yend();

    const int qf_idx = code_block.QuantIndex();

    bool has_parent = m_node.Parent() != 0;

    if(m_node.UsingMultiQuants())
    {
        CodeQuantIndexOffset(qf_idx - m_last_qf_idx);
        m_last_qf_idx = qf_idx;
    }

    m_qf = dirac_quantiser_lists.QuantFactor4(qf_idx);
    if(m_is_intra)
        m_offset =  dirac_quantiser_lists.IntraQuantOffset4(qf_idx);
    else
        m_offset =  dirac_quantiser_lists.InterQuantOffset4(qf_idx);

    for(int ypos = ybeg; ypos < yend ; ++ypos)
    {
        m_pypos = ((ypos - m_node.Yp()) >> 1) + m_pnode.Yp();
        for(int xpos = xbeg; xpos < xend ; ++xpos)
        {
            m_pxpos = ((xpos - m_node.Xp()) >> 1) + m_pnode.Xp();

            m_nhood_nonzero = false;
            if(ypos > m_node.Yp())
                m_nhood_nonzero |= bool(in_data[ypos-1][xpos]);
            if(xpos > m_node.Xp())
                m_nhood_nonzero |= bool(in_data[ypos][xpos-1]);
            if(ypos > m_node.Yp() && xpos > m_node.Xp())
                m_nhood_nonzero |= bool(in_data[ypos-1][xpos-1]);

            if(has_parent)
                m_parent_notzero = static_cast<bool>(in_data[m_pypos][m_pxpos]);
            else
                m_parent_notzero = false;

            CodeCoeff(in_data , xpos , ypos);

        }// xpos
    }// ypos

}

template<typename EntropyCodec>
void GenericBandCodec<EntropyCodec>::CodeCoeff(CoeffArray& in_data, const int xpos, const int ypos)
{
    CodeVal(in_data , xpos , ypos , in_data[ypos][xpos]);
}


/*
Coefficient magnitude value and differential quantiser index magnitude are
coded using interleaved exp-Golomb coding for binarisation. In this scheme, a
value N>=0 is coded by writing N+1 in binary form of a 1 followed by K other
bits: 1bbbbbbb (adding 1 ensures there'll be a leading 1). These K bits ("info
bits") are interleaved with K zeroes ("follow bits") each of which means
"another bit coming", followed by a terminating 1:

    0b0b0b ...0b1

(Conventional exp-Golomb coding has the K zeroes at the beginning, followed
by the 1 i.e 00...01bb .. b, but interleaving allows the decoder to run a
single loop and avoid counting the number of zeroes, sparing a register.)

All bits are arithmetically coded. The follow bits have separate contexts
based on position, and have different contexts from the info bits.
*/

template<typename EntropyCodec>
inline void GenericBandCodec<EntropyCodec>::CodeVal(CoeffArray& in_data ,
        const int xpos ,
        const int ypos ,
        const CoeffType val)
{
    unsigned int abs_val(std::abs(val));
    abs_val <<= 2;
    abs_val /= m_qf;

    const int N = abs_val + 1;
    int num_follow_zeroes = 0;

    while(N >= (1 << num_follow_zeroes))
        ++num_follow_zeroes;
    --num_follow_zeroes;

    for(int i = num_follow_zeroes - 1, c = 1; i >= 0; --i, ++c)
    {
        EntropyCodec::EncodeSymbol(0, ChooseFollowContext(c));
        EntropyCodec::EncodeSymbol(N&(1 << i), ChooseInfoContext());
    }
    EntropyCodec::EncodeSymbol(1, ChooseFollowContext(num_follow_zeroes + 1));

    in_data[ypos][xpos] = static_cast<CoeffType>(abs_val);

    if(abs_val)
    {
        // Must code sign bits and reconstruct
        in_data[ypos][xpos] *= m_qf;
        in_data[ypos][xpos] += m_offset + 2;
        in_data[ypos][xpos] >>= 2;

        if(val > 0)
        {
            EntropyCodec::EncodeSymbol(0 , ChooseSignContext(in_data , xpos , ypos));
        }
        else
        {
            EntropyCodec::EncodeSymbol(1 , ChooseSignContext(in_data , xpos , ypos));
            in_data[ypos][xpos]  = -in_data[ypos][xpos];
        }
    }
}

template<typename EntropyCodec>
void GenericBandCodec<EntropyCodec>::CodeQuantIndexOffset(const int offset)
{

    const int abs_val = std::abs(offset);

    int N = abs_val + 1;
    int num_follow_zeroes = 0;

    while(N >= (1 << num_follow_zeroes))
        ++num_follow_zeroes;
    --num_follow_zeroes;

    for(int i = num_follow_zeroes - 1, c = 1; i >= 0; --i, ++c)
    {
        EntropyCodec::EncodeSymbol(0 , Q_OFFSET_FOLLOW_CTX);
        EntropyCodec::EncodeSymbol(N&(1 << i), Q_OFFSET_INFO_CTX);
    }
    EntropyCodec::EncodeSymbol(1 , Q_OFFSET_FOLLOW_CTX);

    if(abs_val)
    {
        if(offset > 0)
            EntropyCodec::EncodeSymbol(0 , Q_OFFSET_SIGN_CTX);
        else
            EntropyCodec::EncodeSymbol(1 , Q_OFFSET_SIGN_CTX);
    }
}

template<typename EntropyCodec>
void GenericBandCodec<EntropyCodec>::DoWorkDecode(CoeffArray& out_data)
{
    const TwoDArray<CodeBlock>& block_list(m_node.GetCodeBlocks());

    // coeff blocks can be skipped only if SpatialPartitioning is
    // enabled i.e. more than one code-block per subband
    bool decode_skip = (block_list.LengthX() > 1 || block_list.LengthY() > 1);
    // Now loop over the blocks and decode
    for(int j = block_list.FirstY() ; j <= block_list.LastY() ; ++j)
    {
        CodeBlock *block = block_list[j];
        for(int i = block_list.FirstX() ; i <= block_list.LastX() ; ++i)
        {
            if(decode_skip)
                block[i].SetSkip(EntropyCodec::DecodeSymbol(BLOCK_SKIP_CTX));
            if(!block[i].Skipped())
                DecodeCoeffBlock(block[i] , out_data);
            else
                ClearBlock(block[i] , out_data);

        }// i
    }// j

}

template<typename EntropyCodec>
void GenericBandCodec<EntropyCodec>::DecodeCoeffBlock(const CodeBlock& code_block , CoeffArray& out_data)
{


    const int xbeg = code_block.Xstart();
    const int ybeg = code_block.Ystart();
    const int xend = code_block.Xend();
    const int yend = code_block.Yend();

    int qf_idx = m_node.QuantIndex();

    bool has_parent = m_node.Parent() != 0;

    if(m_node.UsingMultiQuants())
    {
        qf_idx = m_last_qf_idx + DecodeQuantIndexOffset();
        m_last_qf_idx = qf_idx;
    }

    if(qf_idx > (int)dirac_quantiser_lists.MaxQuantIndex())
    {
        std::ostringstream errstr;
        errstr << "Quantiser index out of range [0.."
               << (int)dirac_quantiser_lists.MaxQuantIndex() << "]";
        DIRAC_THROW_EXCEPTION(
            ERR_UNSUPPORTED_STREAM_DATA,
            errstr.str(),
            SEVERITY_PICTURE_ERROR);
    }

    m_qf = dirac_quantiser_lists.QuantFactor4(qf_idx);

    if(m_is_intra)
        m_offset =  dirac_quantiser_lists.IntraQuantOffset4(qf_idx);
    else
        m_offset =  dirac_quantiser_lists.InterQuantOffset4(qf_idx);

    //Work

    for(int ypos = ybeg; ypos < yend ; ++ypos)
    {
        m_pypos = ((ypos - m_node.Yp()) >> 1) + m_pnode.Yp();
        CoeffType *p_out_data = NULL;
        if(has_parent)
            p_out_data = out_data[m_pypos];
        CoeffType *c_out_data_1 = NULL;
        if(ypos != m_node.Yp())
            c_out_data_1 = out_data[ypos-1];
        CoeffType *c_out_data_2 = out_data[ypos];
        for(int xpos = xbeg; xpos < xend ; ++xpos)
        {
            m_pxpos = ((xpos - m_node.Xp()) >> 1) + m_pnode.Xp();

            m_nhood_nonzero = false;
            /* c_out_data_1 is the line above the current
             * c_out_data_2 is the current line */
            if(ypos > m_node.Yp())
                m_nhood_nonzero |= bool(c_out_data_1[xpos]);
            if(xpos > m_node.Xp())
                m_nhood_nonzero |= bool(c_out_data_2[xpos-1]);
            if(ypos > m_node.Yp() && xpos > m_node.Xp())
                m_nhood_nonzero |= bool(c_out_data_1[xpos-1]);

            if(has_parent)
                m_parent_notzero = (p_out_data[m_pxpos] != 0);
            else
                m_parent_notzero = false;

            DecodeCoeff(out_data , xpos , ypos);

        }// xpos
    }// ypos
}

template<typename EntropyCodec>
void GenericBandCodec<EntropyCodec>::DecodeCoeff(CoeffArray& in_data, const int xpos, const int ypos)
{
    DecodeVal(in_data , xpos , ypos);
}


/*
Coefficient magnitude value and differential quantiser index value is coded
using interleaved exp-Golomb coding for binarisation. In this scheme, a value
N>=0 is coded by writing N+1 in binary form of a 1 followed by K other bits:
1bbbbbbb (adding 1 ensures there'll be a leading 1). These K bits ("info bits")
are interleaved with K zeroes ("follow bits") each of which means "another bit
coming", followed by a terminating 1:

    0b0b0b ...0b1

(Conventional exp-Golomb coding has the K zeroes at the beginning, followed
by the 1 i.e 00...01bb .. b, but interleaving allows the decoder to run a
single loop and avoid counting the number of zeroes, sparing a register.)

All bits are arithmetically coded. The follow bits have separate contexts
based on position, and have different contexts from the info bits.
*/
template<typename EntropyCodec>
inline void GenericBandCodec<EntropyCodec>::DecodeVal(CoeffArray& out_data , const int xpos , const int ypos)
{

    CoeffType& out_pixel = out_data[ypos][xpos];

    out_pixel = 1;
    int bit_count = 1;

    while(!EntropyCodec::DecodeSymbol(ChooseFollowContext(bit_count)))
    {
        out_pixel <<= 1;
        out_pixel |= EntropyCodec::DecodeSymbol(ChooseInfoContext());
        bit_count++;
    };
    --out_pixel;

    if(out_pixel)
    {
        out_pixel *= m_qf;
        out_pixel += m_offset + 2;
        out_pixel >>= 2;

        if(EntropyCodec::DecodeSymbol(ChooseSignContext(out_data, xpos, ypos)))
            out_pixel = -out_pixel;
    }
}

template<typename EntropyCodec>
inline int GenericBandCodec<EntropyCodec>::ChooseFollowContext(const int bin_number) const
{
    //condition on neighbouring values and parent values

    if(!m_parent_notzero)
    {
        switch(bin_number)
        {
        case 1 :
            if(m_nhood_nonzero == false)
                return Z_FBIN1z_CTX;

            return Z_FBIN1nz_CTX;

        case 2 :
            return Z_FBIN2_CTX;
        case 3 :
            return Z_FBIN3_CTX;
        case 4 :
            return Z_FBIN4_CTX;
        case 5 :
            return Z_FBIN5_CTX;
        default :
            return Z_FBIN6plus_CTX;
        }
    }
    else
    {
        switch(bin_number)
        {
        case 1 :
            if(m_nhood_nonzero == false)
                return NZ_FBIN1z_CTX;

            return NZ_FBIN1nz_CTX;

        case 2 :
            return NZ_FBIN2_CTX;
        case 3 :
            return NZ_FBIN3_CTX;
        case 4 :
            return NZ_FBIN4_CTX;
        case 5 :
            return NZ_FBIN5_CTX;
        default :
            return NZ_FBIN6plus_CTX;
        }

    }

    /* not reachable, but dumb compilers can't spot that */
    return 0;
}

template<typename EntropyCodec>
inline int GenericBandCodec<EntropyCodec>::ChooseInfoContext() const
{
    return INFO_CTX;
}

template<typename EntropyCodec>
inline int GenericBandCodec<EntropyCodec>::ChooseSignContext(const CoeffArray& data , const int xpos , const int ypos) const
{
    if(m_node.Yp() == 0 && m_node.Xp() != 0)
    {
        //we're in a vertically oriented subband
        if(ypos == 0)
            return SIGN0_CTX;
        else
        {
            if(data[ypos-1][xpos] > 0)
                return SIGN_POS_CTX;
            else if(data[ypos-1][xpos] < 0)
                return SIGN_NEG_CTX;
            else
                return SIGN0_CTX;
        }
    }
    else if(m_node.Xp() == 0 && m_node.Yp() != 0)
    {
        //we're in a horizontally oriented subband
        if(xpos == 0)
            return SIGN0_CTX;
        else
        {
            if(data[ypos][xpos-1] > 0)
                return SIGN_POS_CTX;
            else if(data[ypos][xpos-1] < 0)
                return SIGN_NEG_CTX;
            else
                return SIGN0_CTX;
        }
    }
    else
        return SIGN0_CTX;
}

template<typename EntropyCodec>
int GenericBandCodec<EntropyCodec>::DecodeQuantIndexOffset()
{
    int offset = 1;

    while(!EntropyCodec::DecodeSymbol(Q_OFFSET_FOLLOW_CTX))
    {
        offset <<= 1;
        offset |= EntropyCodec::DecodeSymbol(Q_OFFSET_INFO_CTX);
    }
    --offset;

    if(offset)
    {
        if(EntropyCodec::DecodeSymbol(Q_OFFSET_SIGN_CTX))
            offset = -offset;
    }
    return offset;
}

template<typename EntropyCodec>
void GenericBandCodec<EntropyCodec>::SetToVal(const CodeBlock& code_block , CoeffArray& pic_data , const CoeffType val)
{
    for(int j = code_block.Ystart() ; j < code_block.Yend() ; j++)
    {
        for(int i = code_block.Xstart() ; i < code_block.Xend() ; i++)
        {
            pic_data[j][i] = val;
        }// i
    }// j
}

template<typename EntropyCodec>
void GenericBandCodec<EntropyCodec>::ClearBlock(const CodeBlock& code_block , CoeffArray& coeff_data)
{
    for(int j = code_block.Ystart() ; j < code_block.Yend() ; j++)
    {
        CoeffType *pic = &coeff_data[j][code_block.Xstart()];
        memset(pic, 0, (code_block.Xend() - code_block.Xstart())*sizeof(CoeffType));
    }// j

}

/*  Decode a single coefficient using error-feedback DC quantization */
template<typename EntropyCodec>
void GenericIntraDCBandCodec<EntropyCodec>::DecodeCoeffBlock(const CodeBlock& code_block , CoeffArray& out_data)
{
    GenericBandCodec<EntropyCodec>::DecodeCoeffBlock(code_block, out_data);
    /* do prediction for this block */
    for(int ypos = code_block.Ystart() ; ypos < code_block.Yend() ; ++ypos)
    {
        for(int xpos = code_block.Xstart() ; xpos < code_block.Xend() ; ++xpos)
        {
            out_data[ypos][xpos] += GetPrediction(out_data , xpos , ypos);
        }
    }
}

/* after coding a skipped DC codeblock, reconstruct in_data by predicting the values
 * and not adding any error term (they were all skipped).  This is required to correctly
 * predict the values in the next codeblock */

template<typename EntropyCodec>
void GenericIntraDCBandCodec<EntropyCodec>::ClearBlock(const CodeBlock& code_block , CoeffArray& coeff_data)
{
    for(int ypos = code_block.Ystart() ; ypos < code_block.Yend() ; ++ypos)
    {
        for(int xpos = code_block.Xstart() ; xpos < code_block.Xend() ; ++xpos)
        {
            /* NB, it is correct to overwrite the old value */
            coeff_data[ypos][xpos] = GetPrediction(coeff_data , xpos , ypos);
        } // i
    } // j
}

template<typename EntropyCodec>
CoeffType GenericIntraDCBandCodec<EntropyCodec>::GetPrediction(const CoeffArray& data , const int xpos , const int ypos) const
{
    /* NB, 4.5.3 integer division
     * numbers are rounded down towards -ve infinity, differing from
     * C's convention that rounds towards 0
    */
    if(ypos != 0)
    {
        if(xpos != 0)
        {
            int sum = data[ypos][xpos-1] + data[ypos-1][xpos-1] + data[ypos-1][xpos] + 3 / 2;
            if(sum < 0)
                return (sum - 2) / 3;
            else
                return sum / 3;
        }
        else
            return data[ypos - 1][0];
    }
    else
    {
        if(xpos != 0)
            return data[0][xpos - 1];
        else
            return 0;
    }
}
