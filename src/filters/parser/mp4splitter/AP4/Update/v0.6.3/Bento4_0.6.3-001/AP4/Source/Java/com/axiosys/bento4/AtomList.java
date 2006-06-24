package com.axiosys.bento4;

import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.ArrayList;

public class AtomList implements AtomParent {
    private final ArrayList atoms = new ArrayList();
    
    public AtomList(String filename) throws IOException, InvalidFormatException {
        RandomAccessFile input = new RandomAccessFile(filename, "r");
        Atom atom;
        do {
            atom = AtomFactory.DefaultFactory.createAtom(input);
            if (atom != null) atoms.add(atom);
        } while (atom != null);
        //input.close(); do not close the input here as some atoms may need to read from it later
    }
    
    public int getChildrenCount() {
        return atoms.size();
    }

    public Atom getChild(int index) {
        return (Atom)atoms.get(index);
    }

    public Atom getChild(int type, int index) {
        return AtomUtils.findChild(atoms, type, index);
    }
    
    public String toString() {
        StringBuffer result = new StringBuffer();
        String sep = "";
        for (int i=0; i<atoms.size(); i++) {
            Atom atom = (Atom)atoms.get(i);
            result.append(atom.toString() + sep);
            sep = "\n";
        }
        
        return result.toString();
    }
}
