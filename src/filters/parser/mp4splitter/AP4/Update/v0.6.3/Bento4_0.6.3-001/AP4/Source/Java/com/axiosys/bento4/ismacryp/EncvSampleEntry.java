package com.axiosys.bento4.ismacryp;

import java.io.IOException;
import java.io.RandomAccessFile;

import com.axiosys.bento4.AtomFactory;
import com.axiosys.bento4.InvalidFormatException;
import com.axiosys.bento4.VideoSampleEntry;

public class EncvSampleEntry extends VideoSampleEntry {
    public EncvSampleEntry(int size, RandomAccessFile source, AtomFactory atomFactory) throws IOException, InvalidFormatException {
        super(TYPE_ENCV, size, source, atomFactory);
    }
}
