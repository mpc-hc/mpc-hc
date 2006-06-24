package com.axiosys.bento4;

import java.io.IOException;
import java.io.RandomAccessFile;

public class Mp4vSampleEntry extends VideoSampleEntry {
    Mp4vSampleEntry(int size, RandomAccessFile source, AtomFactory atomFactory) throws IOException, InvalidFormatException {
        super(TYPE_MP4V, size, source, atomFactory);
    }
}
