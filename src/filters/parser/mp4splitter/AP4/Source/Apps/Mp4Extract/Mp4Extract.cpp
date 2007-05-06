/*****************************************************************
|
|    AP4 - MP4 File Processor
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
#include <stdio.h>
#include <stdlib.h>

#include "Ap4.h"
#include "Ap4FileByteStream.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
#define BANNER "MP4 Atom Extractor - Version 0.5a\n"\
               "(c) 2003-2005 Gilles Boccon-Gibod & Julien Boeuf"
 
/*----------------------------------------------------------------------
|       PrintUsageAndExit
+---------------------------------------------------------------------*/
static void
PrintUsageAndExit()
{
    fprintf(stderr, 
            BANNER 
            "\n\nusage: mp4extract [options] <atom_path> <input> <output>\n"
            "    options:\n"
            "    --payload-only : ommit the atom header\n");
    exit(1);
}

/*----------------------------------------------------------------------
|       main
+---------------------------------------------------------------------*/
int
main(int argc, char** argv)
{
    if (argc < 4) {
        PrintUsageAndExit();
    }
    
    // parse arguments
    const char* atom_path       = NULL;
    const char* input_filename  = NULL;
    const char* output_filename = NULL;
    bool        payload_only    = false;
    char* arg;
    while ((arg = *++argv)) {
        if (!strcmp(arg, "--payload-only")) {
            payload_only = true;
        } else if (atom_path == NULL) {
            atom_path = arg;
        } else if (input_filename == NULL) {
            input_filename = arg;
        } else if (output_filename == NULL) {
            output_filename = arg;
        } else {
            fprintf(stderr, "ERROR: invalid command line argument (%s)\n", arg);
            return 1;
        }
    }

    // check arguments
    if (atom_path == NULL) {
        fprintf(stderr, "ERROR: missing atom path\n");
        return 1;
    }
    if (input_filename == NULL) {
        fprintf(stderr, "ERROR: missing input filename\n");
        return 1;
    }
    if (output_filename == NULL) {
        fprintf(stderr, "ERROR: missing output filename\n");
        return 1;
    }

	// create the input stream
    AP4_ByteStream* input;
    try {
        input = new AP4_FileByteStream(input_filename,
                        AP4_FileByteStream::STREAM_MODE_READ);
    } catch (AP4_Exception) {
        fprintf(stderr, "ERROR: cannot open input file (%s)\n", input_filename);
        return 1;
    }

    // parse the atoms
    AP4_AtomParent top_level;
    AP4_Atom* atom;
    AP4_AtomFactory& atom_factory = AP4_AtomFactory::DefaultFactory;
    while (atom_factory.CreateAtomFromStream(*input, atom) == AP4_SUCCESS) {
        top_level.AddChild(atom);
    }

    // release the input
    input->Release();

    // find the atom
    atom = top_level.FindChild(atom_path);
    if (atom == NULL) {
        fprintf(stderr, "ERROR: atom '%s' not found\n", atom_path);
        return 1;
    }

    // create the output stream
    AP4_ByteStream* output;
    try {
        output = new AP4_FileByteStream(output_filename,
                         AP4_FileByteStream::STREAM_MODE_WRITE);
    } catch (AP4_Exception) {
        fprintf(stderr, "ERROR: cannot open output file (%s)\n", output_filename);
        return 1;
    }

    // write the atom
    if (payload_only) {
        atom->WriteFields(*output);
    } else {
        atom->Write(*output);
    }

    // cleanup
    output->Release();

    return 0;
}

