package com.axiosys.bento4;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.ArrayList;

public class StsdAtom extends Atom implements AtomParent {
    private ArrayList entries = new ArrayList();

    public StsdAtom(int size, RandomAccessFile source, AtomFactory atomFactory) throws IOException, InvalidFormatException {
        super(TYPE_STSD, size, true, source);
        
        // read the number of entries
        int entryCount = source.readInt();
        int[] bytesAvailable = new int[] { size-FULL_HEADER_SIZE-4 };
        for (int i=0; i<entryCount; i++) {
            Atom atom = atomFactory.createAtom(source, bytesAvailable);
            if (atom != null) {
                entries.add(atom);
            }
        }
    }    
    
    public void writeFields(DataOutputStream stream) throws IOException {
        stream.writeInt(entries.size());
        
        for (int i=0; i<entries.size(); i++) {
            Atom atom = (Atom)entries.get(i);
            atom.write(stream);
        }        
    }
    

    public int getChildrenCount() {
        return entries.size();
    }

    public Atom getChild(int index) {
        return (Atom)entries.get(index);
    }

    public Atom getChild(int type, int index) {
        return AtomUtils.findChild(entries, type, index);
    }

    public String toString(String indentation) {
        StringBuffer result = new StringBuffer();
        result.append(super.toString(indentation));
        for (int i=0; i<entries.size(); i++) {
            result.append("\n");
            result.append(((Atom)entries.get(i)).toString(indentation+"  "));
        }
        
        return result.toString();  
    }
    
    public String toString() {
        return toString("");
    }
}
