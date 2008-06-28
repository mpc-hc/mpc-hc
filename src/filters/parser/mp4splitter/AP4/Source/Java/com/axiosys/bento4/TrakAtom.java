package com.axiosys.bento4;

import java.io.IOException;
import java.io.RandomAccessFile;

public class TrakAtom extends ContainerAtom {
    public TrakAtom(int size, RandomAccessFile source, AtomFactory atomFactory) throws IOException, InvalidFormatException {
        super(TYPE_TRAK, size, false, source, atomFactory);
    }
    
    public int getId() {
        TkhdAtom tkhd = (TkhdAtom)getChild(TYPE_TKHD, 0);
        if (tkhd == null) {
            return 0;
        } else {
            return tkhd.getTrackId();
        }
    }
}
