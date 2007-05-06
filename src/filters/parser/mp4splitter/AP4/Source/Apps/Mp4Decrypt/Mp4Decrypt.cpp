/*****************************************************************
|
|    AP4 - MP4 Decrypter
|
|    Copyright 2005 Gilles Boccon-Gibod
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
#include "Ap4Atom.h"
#include "Ap4File.h"
#include "Ap4Sample.h"
#include "Ap4SampleDescription.h"
#include "Ap4IsmaCryp.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
#define BANNER "MP4 Decrypter - Version 0.1a\n"\
               "(c) 2002-2005 Gilles Boccon-Gibod & Julien Boeuf"
 
/*----------------------------------------------------------------------
|       PrintUsageAndExit
+---------------------------------------------------------------------*/
static void
PrintUsageAndExit()
{
    fprintf(stderr, 
            BANNER 
            "\n\n"
            "usage: mp4decrypt [--key <n>:<k>:<salt>] <input> <output>\n"
            "    where <n> is a track index, <k> a 128-bit key in hex\n"
            "    and <salt> a 128-bit salting key\n"
            "    (several --key options can be used, one for each track)\n");
    exit(1);
}

/*----------------------------------------------------------------------
|       main
+---------------------------------------------------------------------*/
int
main(int argc, char** argv)
{
    if (argc < 3) {
        PrintUsageAndExit();
    }

    // create a decrypting processor
    AP4_IsmaDecryptingProcessor processor;

    // parse options
    const char* input_filename = NULL;
    const char* output_filename = NULL;

    char* arg;
    while ((arg = *++argv)) {
        if (!strcmp(arg, "--key")) {
            arg = *++argv;
            if (arg == NULL) {
                fprintf(stderr, "ERROR: missing argument after --key option\n");
                return 1;
            }
            char* track_ascii = NULL;
            char* key_ascii = NULL;
            char* salt_ascii = NULL;
            if (AP4_SplitArgs(arg, track_ascii, key_ascii, salt_ascii)) {
                fprintf(stderr, "ERROR: invalid argument for --key option\n");
                return 1;
            }
            unsigned char key[16];
            unsigned char salt[16];
            unsigned int track = strtoul(track_ascii, NULL, 10);
            if (AP4_ParseHex(key_ascii, key, 16)) {
                fprintf(stderr, "ERROR: invalid hex format for key\n");
            }
            if (AP4_ParseHex(salt_ascii, salt, 8)) {
                fprintf(stderr, "ERROR: invalid hex format for salt\n");
            }
            // set the key in the map
            processor.GetKeyMap().SetKey(track, key, salt);
        } else if (input_filename == NULL) {
            input_filename = arg;
        } else if (output_filename == NULL) {
            output_filename = arg;
        } else {
            fprintf(stderr, "ERROR: unexpected argument (%s)\n", arg);
            return 1;
        }
    }

    // check the arguments
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
    try{
        input = new AP4_FileByteStream(input_filename,
            AP4_FileByteStream::STREAM_MODE_READ);
    } catch (AP4_Exception) {
        fprintf(stderr, "ERROR: cannot open input file (%s)\n", input_filename);
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

    // process/decrypt the file
    processor.Process(*input, *output);

    // cleanup
    input->Release();
    output->Release();

    return 0;
}
