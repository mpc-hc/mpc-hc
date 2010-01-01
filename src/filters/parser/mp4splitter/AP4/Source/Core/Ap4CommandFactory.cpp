/*****************************************************************
|
|    AP4 - Command Factory
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

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4CommandFactory.h"
#include "Ap4ObjectDescriptor.h"
#include "Ap4Command.h"
#include "Ap4ByteStream.h"

/*----------------------------------------------------------------------
|   AP4_CommandFactory::CreateCommandFromStream
+---------------------------------------------------------------------*/
AP4_Result
AP4_CommandFactory::CreateCommandFromStream(AP4_ByteStream& stream, 
                                            AP4_Command*&   command)
{
    AP4_Result result;

    // NULL by default
    command = NULL;

    // remember current stream offset
    AP4_Position offset;
    stream.Tell(offset);

    // read descriptor tag
    unsigned char tag;
    result = stream.ReadUI08(tag);
    if (AP4_FAILED(result)) {
        stream.Seek(offset);
        return result;
    }
    
    // read descriptor size
    unsigned long payload_size = 0;
    unsigned int  header_size = 1;
    unsigned int  max  = 4;
    unsigned char ext  = 0;
    do {
        header_size++;
        result = stream.ReadUI08(ext);
        if (AP4_FAILED(result)) {
            stream.Seek(offset);
            return result;
        }
        payload_size = (payload_size<<7) + (ext&0x7F);
    } while (--max && (ext&0x80));

    // create the command
    switch (tag) {
      case AP4_COMMAND_TAG_OBJECT_DESCRIPTOR_UPDATE:
      case AP4_COMMAND_TAG_IPMP_DESCRIPTOR_UPDATE:
        command = new AP4_DescriptorUpdateCommand(stream, tag, header_size, payload_size);
        break;

      default:
        command = new AP4_UnknownCommand(stream, tag, header_size, payload_size);
        break;
    }

    // skip to the end of the descriptor
    stream.Seek(offset+header_size+payload_size);

    return AP4_SUCCESS;
}

