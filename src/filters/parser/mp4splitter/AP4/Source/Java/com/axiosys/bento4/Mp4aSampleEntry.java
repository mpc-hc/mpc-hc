package com.axiosys.bento4;

import java.io.IOException;
import java.io.RandomAccessFile;

public class Mp4aSampleEntry extends AudioSampleEntry {
    Mp4aSampleEntry(int size, RandomAccessFile source, AtomFactory atomFactory) throws IOException, InvalidFormatException {
        super(TYPE_MP4A, size, source, atomFactory);
    }
}
