/*****************************************************************
|
|    AP4 - File Processor
|
|    Copyright 2003 Gilles Boccon-Gibod & Julien Boeuf
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

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4AtomFactory.h"

/*----------------------------------------------------------------------
|       class references
+---------------------------------------------------------------------*/
class AP4_ContainerAtom;
class AP4_Sample;
class AP4_ByteStream;
class AP4_DataBuffer;
class AP4_TrakAtom;

/*----------------------------------------------------------------------
|       AP4_Processor
+---------------------------------------------------------------------*/
class AP4_Processor {
public:
    // types
    class TrackHandler {
    public:
        virtual ~TrackHandler() {}
        virtual AP4_Size   GetProcessedSampleSize(AP4_Sample& sample);
        virtual AP4_Result ProcessTrack() { return AP4_SUCCESS; }
        virtual AP4_Result ProcessSample(AP4_DataBuffer& data_in,
                                         AP4_DataBuffer& data_out) = 0;
    };

    // constructor and destructor
    virtual ~AP4_Processor() {}

    // abstract base class methods
    AP4_Result Process(AP4_ByteStream&  input, 
                       AP4_ByteStream&  output,
                       AP4_AtomFactory& atom_factory = 
                       AP4_AtomFactory::DefaultFactory);

    // overridable methods
    virtual AP4_Result Initialize(AP4_AtomParent& top_level);
    virtual AP4_Result Finalize(AP4_AtomParent& top_level);
    virtual TrackHandler* CreateTrackHandler(AP4_TrakAtom* trak);
};
