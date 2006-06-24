package com.axiosys.bento4.ismacryp;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;

import com.axiosys.bento4.Atom;

public class SchmAtom extends Atom {
    private int    schemeType;
    private int    schemeVersion;
    private String schemeUri;

    public SchmAtom(int size, RandomAccessFile source) throws IOException {
        super(TYPE_SCHM, size, true, source);
        
        schemeType = source.readInt();
        schemeVersion = source.readInt();
        
        byte[] str = new byte[size-getHeaderSize()-8];
        source.read(str);
        int str_size = 0;
        while (str_size < str.length && str[str_size] != 0) str_size++;
        schemeUri = new String(str, 0, str_size, "UTF-8");
        
    }

    public int getSchemeType() {
        return schemeType;
    }

    public String getSchemeUri() {
        return schemeUri;
    }

    public int getSchemeVersion() {
        return schemeVersion;
    }
    
    protected void writeFields(DataOutputStream stream) throws IOException {
        stream.writeInt(schemeType);
        stream.writeInt(schemeVersion);
        byte[] uri_bytes = schemeUri.getBytes("UTF-8");
        stream.write(uri_bytes);
        int termination = size-getHeaderSize()-8-uri_bytes.length;
        for (int i=0; i<termination; i++) {
            stream.writeByte(0);
        }
    }
    
    public String toString(String indentation) {
        StringBuffer result = new StringBuffer(super.toString(indentation));
        result.append("\n" + indentation + "  scheme_type    = " + Atom.typeString(schemeType));
        result.append("\n" + indentation + "  scheme_version = " + schemeVersion);
        result.append("\n" + indentation + "  scheme_uri     = " + schemeUri);
        return result.toString();
    }
}
