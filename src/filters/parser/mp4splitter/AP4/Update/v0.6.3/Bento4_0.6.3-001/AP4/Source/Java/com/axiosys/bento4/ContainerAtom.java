package com.axiosys.bento4;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.ArrayList;

public class ContainerAtom extends Atom implements AtomParent {
    protected final ArrayList children = new ArrayList();
    
    public ContainerAtom(int type, int size, boolean isFull, RandomAccessFile source) throws IOException {
        super(type, size, isFull, source);
    }

    public ContainerAtom(int type, int size, boolean isFull, RandomAccessFile source, AtomFactory atomFactory) throws IOException, InvalidFormatException {
        super(type, size, isFull, source);
        readChildren(atomFactory, source, getPayloadSize());
    }

    public Atom findAtom(String path) {
        return AtomUtils.findAtom(this, path);
    }
    
    protected void writeFields(DataOutputStream stream) throws IOException {
        writeChildren(stream);
    }
    
    protected void readChildren(AtomFactory atomFactory, RandomAccessFile source, int size) throws IOException, InvalidFormatException {
        int[] bytesAvailable = new int[] { size };
        Atom atom;
        do {
            atom = atomFactory.createAtom(source, bytesAvailable);
            if (atom != null) children.add(atom);
        } while (atom != null);
    }

    protected void writeChildren(DataOutputStream stream) throws IOException {
        for (int i=0; i<children.size(); i++) {
            Atom atom = (Atom)children.get(i);
            atom.write(stream);
        }        
    }

    public int getChildrenCount() {
        return children.size();
    }
    
    public Atom getChild(int index) {
        return (Atom)children.get(index);
    }

    public Atom getChild(int type, int index) {
        return AtomUtils.findChild(children, type, index);
    }

    public String toString(String indentation) {
        StringBuffer result = new StringBuffer();
        result.append(super.toString(indentation));
        for (int i=0; i<children.size(); i++) {
            result.append("\n");
            result.append(((Atom)children.get(i)).toString(indentation+"  "));
        }
        
        return result.toString();  
    }
}
