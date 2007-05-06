package com.axiosys.bento4;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;

public class UnknownAtom extends Atom {
    private final static int       CHUNK_SIZE = 4096;

    private final RandomAccessFile source;
    private final int              offset;
    
    public UnknownAtom(int type, int size, RandomAccessFile source, int offset) {
        super(type, size, false);
        this.source = source;
        this.offset = offset;
    }
    
    public UnknownAtom(int type, int size, RandomAccessFile source) throws IOException {
        this(type, size, source, (int)source.getFilePointer());
    }
    
    public void writeFields(DataOutputStream stream) throws IOException {
        int position = offset;
        byte[] buffer = new byte[CHUNK_SIZE];
        int toCopy = getPayloadSize();
        while (toCopy > 0) {
            int chunk = toCopy > CHUNK_SIZE ? CHUNK_SIZE : toCopy;
            source.seek(position);
            source.readFully(buffer, 0, chunk);
            stream.write(buffer, 0, chunk);
            toCopy -= chunk;
            position += chunk;
        }
    }
}
