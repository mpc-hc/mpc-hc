/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: enc_queue.h,v 1.3 2008/08/14 02:30:50 asuraparaju Exp $ $Name:  $
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
* Contributor(s): Thomas Davies (Original Author),
*                 Anuradha Suraparaju
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

#ifndef _ENC_QUEUE_H_
#define _ENC_QUEUE_H_

#include <vector>
#include <map>
#include <libdirac_encoder/enc_picture.h>
#include <libdirac_common/common.h>

namespace dirac
{
    //! Holds pictures both for reference and to overcome reordering delay
    /*!
        The buffer holds pictures in a stack to overcome both reordering due to
        bi-directional prediction and use as references for subsequence motion
        estimation. Pictures, and components of pictures, can be accessed by their
        picture numbers. GOP parameters can be included in the constructors so
        that pictures can be given types (I picture, L1 picture or L2 picture) on
        being pushed onto the stack; alternatively, these parameters can be
        overridden.
    */
    class EncQueue{
    public:
        //! Default Constructor
        EncQueue();

        //! Constructor
        /*!
            Creates a EncQueue using the chroma format. Suitable for
            compressing when there are no L2 pictures, or when the temporal
            prediction structure is to be determined on the fly.

            \param   cf    the Chroma format of pictures in the buffer
            \param   xlen  the luma width of pictures in the buffer
            \param   ylen  the luma height of pictures in the buffer
            \param   luma_depth the video depth of the luma comp in the buffer
            \param   chroma_depth the video depth of the chroma comp in the buffer
            \param   using_ac   True if using Arithmetic coding to code coefficient data

        */
        EncQueue(ChromaFormat cf,
                    const int xlen,
                    const int ylen,
                    const unsigned int luma_depth,
                    const unsigned int chroma_depth,
                    bool using_ac);

        //! Constructor
        /*!
            Creates a EncQueue using the chroma format, the number of L1
            pictures between I pictures and the separation in pictures between L1
            pictures. Suitable for compressing when there is a full GOP structure
            or when the temporal prediction structure is to be determined on
            the fly.

            \param  cf    the Chroma format of pictures in the buffer
            \param  numL1    the number of Layer 1 pictures before the next I picture. 0 means that there is only one I picture.
            \param  L1sep    the number of Layer 2 pictures between Layer 1 pictures
            \param  xlen  the luma width of pictures in the buffer
            \param  ylen  the luma height of pictures in the buffer
            \param   luma_depth the video depth of the luma comp in the buffer
            \param   chroma_depth the video depth of the chroma comp in the buffer
            \param   interlace Set true if material is being coded in interlaced mode
            \param   using_ac   True if using Arithmetic coding to code coefficient data
        */
        EncQueue(ChromaFormat cf,
                    const int numL1,
                    const int L1sep,
                    const int xlen,
                    const int ylen,
                    const unsigned int luma_depth,
                    const unsigned int chroma_depth,
                    bool interlace,
                    bool using_ac);

        //! Copy constructor
        /*!
            Copy constructor. Removes the current contents of the pictureture buffer
            and copies in the contents of the initialising buffer.
        */
        EncQueue(const EncQueue& cpy);

        //! Operator=.
        /*!
            Operator=. Assigns all elements of the rhs to the lhs.
        */
        EncQueue& operator=(const EncQueue& rhs);

        //! Destructor
        ~EncQueue();

        //! Get picture with a given picture number (NOT with a given position in the buffer)
        EncPicture& GetPicture(const unsigned int pnum );

        //! Get picture with a given picture number (NOT with a given position in the buffer)
        const EncPicture& GetPicture(const unsigned int pnum) const;

        //! Get picture with a given picture number, setting a flag to true if it's there
        EncPicture& GetPicture(const unsigned int pnum, bool& is_present);

        //! Get picture with a given picture number, setting a flag to true if it's there
        const EncPicture& GetPicture(const unsigned int pnum, bool& is_present) const;

        //! Return true if picture with the particular picture number is available else return false
        bool IsPictureAvail(const unsigned int pnum) const;

        //! Returns a list of member pictures
        std::vector<int> Members() const;

	//! Returns the size of the queue
	int Size() const { return m_pic_data.size(); }

        //! Put a new picture into the top of the buffer
        /*!
            Put a new picture into the top of the buffer. EncPicture parameters
            associated with the picture will be as given by the picture parameter
            object.
        */
        void PushPicture(const PictureParams& pp);

        //! Put a copy of a new picture into the buffer
        /*!
            Put a copy of a new picture into the buffer.
        */
        void CopyPicture( const EncPicture& picture );

        //! Sets the reference picture number that will be cleaned
        /*!
            Indicate which picture which has been output and which is no longer
            required for reference. Expiry times are set in each picture's
            picture parameters.
            \param show_pnum             picture number in display order that can be output
            \param current_coded_pnum    picture number in display order of picture currently being coded
        */
        void SetRetiredPictureNum(const int show_pnum, const int current_coded_pnum);

        //! Delete all expired pictures
        /*!
            Delete pictures which have been output and which are no longer
            required for reference. Expiry times are set in each picture's
            picture parameters.
            \param show_pnum             picture number in display order that can be output
            \param current_coded_pnum    picture number in display order of picture currently being coded
        */
        void CleanAll(const int show_pnum, const int current_coded_pnum);

        //! Delete retired reference pictures and expired non-ref pictures
        /*!
            Delete pictures which have been output and retired reference pictures.
            Expiry times are set in each picture's picture parameters.
            \param show_pnum             picture number in display order that can be output
            \param current_coded_pnum    picture number in display order of picture currently being coded
        */
        void CleanRetired(const int show_pnum, const int current_coded_pnum);

        //! Delete picture
        /*!
            Delete picture.
            \param pnum             picture number in display order to be deleted from picture buffer
        */
        void Remove(int pnum);

    private:
        //! Clear internal data slot number pos
        /*!
            Clear internal data slot number pos
        */
        void ClearSlot(const unsigned int pos);

    private:

//        //! the count of the number of reference pictures in the buffer
//        int m_ref_count;

        //! the buffer storing all the values
        std::vector<EncPicture*> m_pic_data;

        //!the map from picture numbers to position in the buffer
        std::map<unsigned int,unsigned int> m_pnum_map;

    };

} // namespace dirac

#endif
