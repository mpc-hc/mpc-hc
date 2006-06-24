package com.axiosys.bento4;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;

public class TkhdAtom extends Atom {
    private int trackId;
    private int duration;
    private int creationTime;
    private int modificationTime;
    private int width;
    private int height;
    private int layer;
    private int alternateGroup;
    private int volume;
    
    int getDuration() { return duration; }
    int getTrackId()  { return trackId;  }

    public TkhdAtom(int size, RandomAccessFile source) throws IOException {
        super(TYPE_TKHD, size, true, source);
        
        if (version == 0) {
            // we only deal with version 0 for now
            creationTime = source.readInt();
            modificationTime = source.readInt();
            trackId = source.readInt();
            source.skipBytes(4);
            duration = source.readInt();
        } else {
            source.skipBytes(32);
        }

        source.skipBytes(8);
        layer = source.readUnsignedShort();
        alternateGroup = source.readUnsignedShort();
        volume = source.readUnsignedShort();
        source.skipBytes(2+9*4);
        width = source.readInt();
        height = source.readInt();
    }
    
    protected void writeFields(DataOutputStream stream) throws IOException {
        // not implemented yet
        throw new RuntimeException("not implemented yet");
    }

    public String toString(String indentation) {
        StringBuffer result = new StringBuffer(super.toString(indentation));
        result.append("\n" + indentation + " track_id          = " + trackId);
        result.append("\n" + indentation +"  duration          = " + duration);
        result.append("\n" + indentation +"  creation_time     = " + creationTime);
        result.append("\n" + indentation +"  modification_time = " + modificationTime);
        result.append("\n" + indentation +"  width             = " + width);
        result.append("\n" + indentation +"  height            = " + height);
        result.append("\n" + indentation +"  alternate_group   = " + alternateGroup);
        result.append("\n" + indentation +"  layer             = " + layer);
        result.append("\n" + indentation +"  volume            = " + volume);
        
        return result.toString();  
    }
}
