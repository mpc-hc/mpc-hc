package com.axiosys.bento4;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;

public class HdlrAtom extends Atom {
    private int    handlerType;
    private String handlerName;
    
    public HdlrAtom(int size, RandomAccessFile source) throws IOException {
        super(TYPE_HDLR, size, true, source);
        
        source.skipBytes(4);
        handlerType = source.readInt();
        source.skipBytes(12);
        
        // read the name unless it is empty
        int nameSize = size-(FULL_HEADER_SIZE+20);
        if (nameSize > 0) {
            byte[] name = new byte[nameSize];
            source.read(name);
            int nameChars = 0;
            while (nameChars < name.length && name[nameChars] != 0) nameChars++;
            handlerName = new String(name, 0, nameChars, "UTF-8");
        }
    }

    public String getHandlerName() {
        return handlerName;
    }
    
    public int getHandlerType() {
        return handlerType;
    }
    
    protected void writeFields(DataOutputStream stream) throws IOException {
        // not implemented yet
        throw new RuntimeException("not implemented yet");
    }

    public String toString(String indentation) {
        StringBuffer result = new StringBuffer(super.toString(indentation));
        result.append("\n" + indentation + " handler_type      = " + Atom.typeString(handlerType));
        result.append("\n" + indentation +"  handler_name      = " + handlerName);
        
        return result.toString();  
    }
}
