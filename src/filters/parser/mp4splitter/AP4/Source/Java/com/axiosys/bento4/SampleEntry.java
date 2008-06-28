package com.axiosys.bento4;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;

public class SampleEntry extends ContainerAtom {
    private int dataReferenceIndex;
    
    SampleEntry(int format, int size, RandomAccessFile source, AtomFactory atomFactory) throws IOException, InvalidFormatException {
        super(format, size, false, source);
        
        // read the fields before the children atoms
        int fieldsSize = getFieldsSize();
        readFields(source);

        // read children atoms (ex: esds and maybe others)
        readChildren(atomFactory, source, size-HEADER_SIZE-fieldsSize);
    }
    
    public void write(DataOutputStream stream) throws IOException {
        // write the header
        writeHeader(stream);

        // write the fields
        writeFields(stream);

        // write the children atoms
        writeChildren(stream);
    }
    
    protected int getFieldsSize() {
        return 8;
    }
    
    protected void readFields(RandomAccessFile source) throws IOException {
        source.skipBytes(6);
        dataReferenceIndex = source.readUnsignedShort();    
    }
    
    protected void writeFields(DataOutputStream stream) throws IOException {
        byte[] reserved = new byte[] { 0,0,0,0,0,0 };
        stream.write(reserved);
        stream.writeShort(dataReferenceIndex);
    }
 }
