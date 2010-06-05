/*****************************************************************
|
|    AP4 - File Processor
|
|    Copyright 2002-2008 Axiomatic Systems, LLC
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
****************************************************************/

#ifndef _AP4_PROCESSOR_H_
#define _AP4_PROCESSOR_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4AtomFactory.h"
#include "Ap4File.h"
#include "Ap4Track.h"
#include "Ap4Sample.h"

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
class AP4_ContainerAtom;
class AP4_ByteStream;
class AP4_DataBuffer;
class AP4_TrakAtom;
struct AP4_MoofLocator;

/*----------------------------------------------------------------------
|   AP4_Processor
+---------------------------------------------------------------------*/
class AP4_Processor {
public:
    /**
     * Abstract class that defines the interface implemented by progress
     * listeners. A progress listener is called during the AP4_Processor::Process()
     * method to notify of progres information.
     */
    class ProgressListener {
    public:
        virtual ~ProgressListener() {}

        /**
         * This method is called during the call to AP4_Processor::Process() to
         * notify of the progress of the operation. If this method returns an 
         * error result, processing is aborted.
         * @param step Ordinal of the current progress step.
         * @param total Total number of steps.
         * @return A result code. If this method returns AP4_SUCCESS, the 
         * processing continues. If an error code is returned, the processing
         * is aborted.
         */
        virtual AP4_Result OnProgress(unsigned int step, 
                                      unsigned int total) = 0;
    };

    /**
     * Abstract class that defines the interface implemented by concrete
     * track handlers. A track handler is responsible for processing a 
     * track and its media samples.
     */
    class TrackHandler {
    public:
        /**
         * Default destructor.
         */
        virtual ~TrackHandler() {}

        /**
         * A track handler may override this method if it needs to modify
         * the track atoms before processing the track samples.
         */
        virtual AP4_Result ProcessTrack() { return AP4_SUCCESS; }

        /**
         * Returns the size of a sample after processing.
         * @param sample Sample of which the processed size is requested.
         * @return Size of the sample data after processing.
         */
        virtual AP4_Size GetProcessedSampleSize(AP4_Sample& sample) { return sample.GetSize(); }

        /**
         * Process the data of one sample.
         * @param data_in Data buffer with the data of the sample to process.
         * @param data_out Data buffer in which the processed sample data is
         * returned.
         */
        virtual AP4_Result ProcessSample(AP4_DataBuffer& data_in,
                                         AP4_DataBuffer& data_out) = 0;
    };

    /**
     * Abstract class that defines the interface implemented by concrete
     * fragment handlers. A fragment handler is responsible for processing a 
     * fragment and its media samples.
     */
    class FragmentHandler {
    public:
        /**
         * Default destructor.
         */
        virtual ~FragmentHandler() {}

        /**
         * A fragment handler may override this method if it needs to modify
         * the fragment atoms before processing the fragment samples.
         */
        virtual AP4_Result ProcessFragment() { return AP4_SUCCESS; }

        /**
         * A fragment handler may override this method if it needs to modify
         * the fragment atoms after processing the fragment samples.
         * NOTE: this method MUST NOT change the size of any of the atoms.
         */
        virtual AP4_Result FinishFragment() { return AP4_ERROR_NOT_SUPPORTED; }

        /**
         * Returns the size of a sample after processing.
         * @param sample Sample of which the processed size is requested.
         * @return Size of the sample data after processing.
         */
        virtual AP4_Size   GetProcessedSampleSize(AP4_Sample& sample) { return sample.GetSize(); }

        /**
         * Process the data of one sample.
         * @param data_in Data buffer with the data of the sample to process.
         * @param data_out Data buffer in which the processed sample data is
         * returned.
         */
        virtual AP4_Result ProcessSample(AP4_DataBuffer& data_in,
                                         AP4_DataBuffer& data_out) = 0;
    };

    /**
     *  Default destructor
     */
    virtual ~AP4_Processor() { m_ExternalTrackData.DeleteReferences(); }

    /**
     * Process the input stream into an output stream.
     * @param input Reference to the file to process.
     * @param output Output stream to which the processed input
     * will be written.
     * @param listener Pointer to a listener, or NULL. The listener
     * will be called one or more times before this method returns, 
     * with progress information.
     */
    AP4_Result Process(AP4_ByteStream&   input, 
                       AP4_ByteStream&   output,
                       ProgressListener* listener = NULL,
                       AP4_AtomFactory&  atom_factory = 
                           AP4_DefaultAtomFactory::Instance);

    /**
     * This method can be overridden by concrete subclasses.
     * It is called just after the input stream has been parsed into
     * an atom tree, before the processing of the tracks.
     * @param top_level Container atom containing all the atoms parsed
     * from the input stream. Note that this atom does not actually 
     * exist in the file; it is a synthetised container created for the
     * purpose of holding together all the input's top-level atoms.
     */
    virtual AP4_Result Initialize(AP4_AtomParent&   top_level,
                                  AP4_ByteStream&   stream,
                                  ProgressListener* listener = NULL);

    /**
     * This method can be overridden by concrete subclasses.
     * It is called just after the tracks have been processed.
     */
    virtual AP4_Result Finalize(AP4_AtomParent&   top_level,
                                ProgressListener* listener = NULL);

    /**
     * This method can be overridden by concrete subclasses.
     * It is called once for each track in the input file.
     * @param track Pointer to the track for which a handler should be
     * created. 
     * @return A pointer to a track handler, or NULL if no handler 
     * needs to be created for that track.
     */
    virtual TrackHandler* CreateTrackHandler(AP4_TrakAtom* /*trak*/) { return NULL; }

    /**
     * This method can be overridden by concrete subclasses.
     * It is called once for each fragment in the input file.
     * @param track Pointer to the fragment for which a handler should be
     * created. 
     * @return A pointer to a fragment handler, or NULL if no handler 
     * needs to be created for that fragment.
     */
    virtual FragmentHandler* CreateFragmentHandler(AP4_ContainerAtom* /*traf*/) { return NULL; }
    
protected:
    class ExternalTrackData {
    public:
        ExternalTrackData(unsigned int track_id, AP4_ByteStream* media_data) :
            m_TrackId(track_id), m_MediaData(media_data) {
            media_data->AddReference();
        }
        ~ExternalTrackData() { m_MediaData->Release(); }
        unsigned int    m_TrackId;
        AP4_ByteStream* m_MediaData;
    };

    AP4_Result ProcessFragments(AP4_MoovAtom*              moov, 
                                AP4_List<AP4_MoofLocator>& moofs, 
                                AP4_ContainerAtom*         mfra,
                                AP4_ByteStream&            input, 
                                AP4_ByteStream&            output);
    
    
    AP4_List<ExternalTrackData> m_ExternalTrackData;
};

#endif // _AP4_PROCESSOR_H_
