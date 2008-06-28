package com.axiosys.bento4.ismacryp;

import java.io.IOException;
import java.io.RandomAccessFile;

import com.axiosys.bento4.AtomFactory;
import com.axiosys.bento4.AudioSampleEntry;
import com.axiosys.bento4.InvalidFormatException;

public class EncaSampleEntry extends AudioSampleEntry {
    public EncaSampleEntry(int size, RandomAccessFile source, AtomFactory atomFactory) throws IOException, InvalidFormatException {
        super(TYPE_ENCA, size, source, atomFactory);
    }
}
