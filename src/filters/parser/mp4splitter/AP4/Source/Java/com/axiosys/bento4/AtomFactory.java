package com.axiosys.bento4;

import java.io.IOException;
import java.io.RandomAccessFile;

import com.axiosys.bento4.ismacryp.EncaSampleEntry;
import com.axiosys.bento4.ismacryp.EncvSampleEntry;
import com.axiosys.bento4.ismacryp.IkmsAtom;
import com.axiosys.bento4.ismacryp.SchmAtom;

public class AtomFactory {
    public static final AtomFactory DefaultFactory = new AtomFactory();
    
    private int context = 0;
    
    public Atom createAtom(RandomAccessFile file) throws IOException, InvalidFormatException {
        return createAtom(file, new int[] { (int) (file.length()-file.getFilePointer()) });
    }

    Atom createAtom(RandomAccessFile source, int[] bytesAvailable /* by reference */) throws IOException, InvalidFormatException {
        Atom atom = null;
        
        // check that there are enough bytes for at least a header
        if (bytesAvailable[0] < Atom.HEADER_SIZE) return null;

        // remember current file offset
        long start = source.getFilePointer();

        // read atom size
        int size = source.readInt();

        if (size == 0) {
            // atom extends to end of file
            size = (int)(source.length()-start);
        }

        // check the size (we don't handle extended size yet)
        if (size > bytesAvailable[0]) {
            source.seek(start);
            return null;
        }

        if (size < 0) {
            // something is corrupted
            throw new InvalidFormatException("invalid atom size");
        }
        
        // read atom type
        int type = source.readInt();

        // create the atom
        switch (type) {
          case Atom.TYPE_STSD:
            atom = new StsdAtom(size, source, this);
            break;
                
          case Atom.TYPE_SCHM:
            atom = new SchmAtom(size, source);
            break;

          case Atom.TYPE_IKMS:
            atom = new IkmsAtom(size, source);
            break;

          case Atom.TYPE_TRAK:
            atom = new TrakAtom(size, source, this);
            break;
            
          case Atom.TYPE_TKHD:
            atom = new TkhdAtom(size, source);
            break;
            
          case Atom.TYPE_HDLR:
            atom = new HdlrAtom(size, source);
            break;
            
          // container atoms
          case Atom.TYPE_MOOV:
          case Atom.TYPE_HNTI:
          case Atom.TYPE_STBL:
          case Atom.TYPE_MDIA:
          case Atom.TYPE_DINF:
          case Atom.TYPE_MINF:
          case Atom.TYPE_SCHI:
          case Atom.TYPE_SINF:
          case Atom.TYPE_UDTA:
          case Atom.TYPE_ILST:
          case Atom.TYPE_EDTS: {
              int previousContext = context;
              context = type; // set the context for the children
              atom = new ContainerAtom(type, size, false, source, this);
              context = previousContext; // restore the previous context
              break;
          }

          // full container atoms
          case Atom.TYPE_META:
            atom = new ContainerAtom(type, size, false, source, this);
            break;

          // sample entries
          case Atom.TYPE_MP4A:
            atom = new Mp4aSampleEntry(size, source, this);
            break;

          case Atom.TYPE_MP4V:
              atom = new Mp4vSampleEntry(size, source, this);
              break;

          case Atom.TYPE_ENCA:
              atom = new EncaSampleEntry(size, source, this);
              break;

          case Atom.TYPE_ENCV:
              atom = new EncvSampleEntry(size, source, this);
              break;

          default:
              atom = new UnknownAtom(type, size, source);
              break;
        }

        // skip to the end of the atom
        bytesAvailable[0] -= size;
        source.seek(start+size);

        return atom;
    }
}
