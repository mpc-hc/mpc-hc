package com.axiosys.bento4;

import java.io.IOException;
import java.io.RandomAccessFile;

public class MpegSampleEntry extends SampleEntry {
    MpegSampleEntry(int format, int size, RandomAccessFile source, AtomFactory atomFactory) throws IOException, InvalidFormatException {
        super(format, size, source, atomFactory);
    }
}
