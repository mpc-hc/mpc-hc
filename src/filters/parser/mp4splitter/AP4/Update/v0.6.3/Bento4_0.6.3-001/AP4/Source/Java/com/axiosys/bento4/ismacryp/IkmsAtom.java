package com.axiosys.bento4.ismacryp;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;

import com.axiosys.bento4.Atom;

public class IkmsAtom extends Atom {
    private final String kmsUri;
    
    public IkmsAtom(int size, RandomAccessFile source) throws IOException {
        super(TYPE_IKMS, size, true, source);
        
        byte[] str = new byte[size-getHeaderSize()];
        source.read(str);
        int str_size = 0;
        while (str[str_size] != 0) str_size++;
        kmsUri = new String(str, 0, str_size, "UTF-8");
    }
    
    public String getKmsUri() { 
        return kmsUri;
    }
    
    protected void writeFields(DataOutputStream stream) throws IOException {
        byte[] bytes = kmsUri.getBytes("UTF-8");
        stream.write(bytes);
        int termination = size-getHeaderSize()-bytes.length;
        for (int i=0; i<termination; i++) {
            stream.writeByte(0);
        }
    }
    
    public String toString(String indentation) {
        StringBuffer result = new StringBuffer(super.toString(indentation));
        result.append("\n");
        result.append(indentation + "  kms_uri = " + kmsUri);
        return result.toString();
    }
}
