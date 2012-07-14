/*****************************************************************
|
|    AP4 - Descriptor Factory
|
|    Copyright 2002 Gilles Boccon-Gibod
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
#include "Ap4DescriptorFactory.h"
#include "Ap4EsDescriptor.h"
#include "Ap4DecoderConfigDescriptor.h"
#include "Ap4DecoderSpecificInfoDescriptor.h"
#include "Ap4SLConfigDescriptor.h"
#include "Ap4UnknownDescriptor.h"

/*----------------------------------------------------------------------
|       AP4_DescriptorFactory::CreateDescriptorFromStream
+---------------------------------------------------------------------*/
AP4_Result
AP4_DescriptorFactory::CreateDescriptorFromStream(AP4_ByteStream&  stream, 
                                                  AP4_Descriptor*& descriptor)
{
    AP4_Result result;

    // NULL by default
    descriptor = NULL;

    // remember current stream offset
    AP4_Offset offset;
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

    // create the descriptor
    switch (tag) {
      case AP4_DESCRIPTOR_TAG_ES:
        descriptor = new AP4_EsDescriptor(stream, header_size, payload_size);
        break;

      case AP4_DESCRIPTOR_TAG_DECODER_CONFIG:
        descriptor = new AP4_DecoderConfigDescriptor(stream, header_size, payload_size);
        break;

	  case AP4_DESCRIPTOR_TAG_DECODER_SPECIFIC_INFO:
	    descriptor = new AP4_DecoderSpecificInfoDescriptor(stream, header_size, payload_size);
		break;
          
      case AP4_DESCRIPTOR_TAG_SL_CONFIG:
        if (payload_size != 1) return AP4_ERROR_INVALID_FORMAT;
        descriptor = new AP4_SLConfigDescriptor(header_size);
        break;

      default:
        descriptor = new AP4_UnknownDescriptor(stream, tag, header_size, payload_size);
        break;
    }

    // skip to the end of the descriptor
    stream.Seek(offset+header_size+payload_size);

    return AP4_SUCCESS;
}

