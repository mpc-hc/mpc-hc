package com.axiosys.bento4;

import java.io.IOException;
import java.io.RandomAccessFile;

public class AudioSampleEntry extends MpegSampleEntry {
    private int sampleRate;           
    private int channelCount;
    private int sampleSize;

    protected AudioSampleEntry(int format, int size, RandomAccessFile source, AtomFactory atomFactory) throws IOException, InvalidFormatException {
        super(format, size, source, atomFactory);
    }

    int getSampleRate()   { return (sampleRate>>16)&0xFFFF; }
    int getSampleSize()   { return sampleSize;     }
    int getChannelCount() { return channelCount;   }

    protected void readFields(RandomAccessFile source) throws IOException {
        super.readFields(source);

        source.skipBytes(8);
        channelCount = source.readUnsignedShort();
        sampleSize = source.readUnsignedShort();
        source.skipBytes(4);
        sampleRate = source.readInt();
    }
    
    public String toString(String indentation) {
        StringBuffer result = new StringBuffer();
        result.append(indentation+"[" + typeString(type) + "] size=" + getHeaderSize() + "+" + getPayloadSize());
        result.append("\n" + indentation + "  sample_rate   = " + getSampleRate());
        result.append("\n" + indentation + "  sample_size   = " + sampleSize);
        result.append("\n" + indentation + "  channel_count = " + channelCount);
        for (int i=0; i<children.size(); i++) {
            result.append("\n");
            result.append(((Atom)children.get(i)).toString(indentation+"  "));
        }
        
        return result.toString();  
    }
}
