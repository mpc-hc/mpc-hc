package com.axiosys.bento4;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;

public abstract class Atom {
    public final static int TYPE_MOOV = 0x6d6f6f76;
    public final static int TYPE_TRAK = 0x7472616b;
    public final static int TYPE_HNTI = 0x686e7469;
    public final static int TYPE_STBL = 0x7374626c;
    public final static int TYPE_MDIA = 0x6d646961;
    public final static int TYPE_DINF = 0x64696e66;
    public final static int TYPE_MINF = 0x6d696e66;
    public final static int TYPE_SCHI = 0x73636869;
    public final static int TYPE_SINF = 0x73696e66;
    public final static int TYPE_UDTA = 0x75647461;
    public final static int TYPE_ILST = 0x696c7374;
    public final static int TYPE_EDTS = 0x65647473;
    public final static int TYPE_META = 0x6d657461;
    public final static int TYPE_STSD = 0x73747364;    
    public final static int TYPE_MP4A = 0x6d703461;
    public final static int TYPE_ENCA = 0x656e6361;
    public final static int TYPE_MP4V = 0x6d703476;
    public final static int TYPE_ENCV = 0x656e6376;
    public final static int TYPE_IKMS = 0x694b4d53;
    public final static int TYPE_TKHD = 0x746b6864;
    public final static int TYPE_SCHM = 0x7363686d;
    public final static int TYPE_HDLR = 0x68646c72;
    
    public final static int HEADER_SIZE      = 8;
    public final static int FULL_HEADER_SIZE = 12;
    
    // members
    protected int type;
    protected int size;
    protected int flags;
    protected int version;
    protected boolean isFull;
    
    public static String typeString(int type) {
        StringBuffer result = new StringBuffer(4);
    
        result.append((char)((type>>24)&0xFF));
        result.append((char)((type>>16)&0xFF));
        result.append((char)((type>> 8)&0xFF));
        result.append((char)((type    )&0xFF));

        return result.toString();
    }
    
    public static int nameToType(String name) {
        return ((name.charAt(0)&0xFF)<<24) |
               ((name.charAt(1)&0xFF)<<16) |
               ((name.charAt(2)&0xFF)<< 8) |
               ((name.charAt(3)&0xFF));
    }
        
    public Atom(int type, int size, boolean isFull) {
        this.type = type;
        this.size = size;
        this.isFull = isFull;        
    }
    
    public Atom(int type, int size, boolean isFull, RandomAccessFile source) throws IOException {
        this(type, size, isFull);
        if (isFull) {
            // read the version and flags
            int extension = source.readInt();
            version = (extension>>24)&0xFF;
            flags   =  extension&0xFFFFFF;
        } else {
            this.flags = 0;
            this.version = 0;
        }
    }

    public int getType()        { return type;                                }
    public int getSize()        { return size;                                }
    public int getHeaderSize()  { return isFull?FULL_HEADER_SIZE:HEADER_SIZE; }
    public int getPayloadSize() { return size-getHeaderSize();                }
    
    public void write(DataOutputStream stream) throws IOException {
        // write the header
        writeHeader(stream);

        // write the fields
        writeFields(stream);
    }
    
    public void writeHeader(DataOutputStream stream) throws IOException {
        // write the size
        stream.writeInt(size);

        // write the type
        stream.writeInt(type);

        // for full atoms, write version and flags
        if (isFull) {
            stream.writeInt(version<<24 | flags);
        }        
    }
    
    protected abstract void writeFields(DataOutputStream stream) throws IOException;
    
    public byte[] toBytes() throws IOException {
        ByteArrayOutputStream stream = new ByteArrayOutputStream();
        DataOutputStream output = new DataOutputStream(stream);
        write(output);
        
        return stream.toByteArray();
    }
    
    public byte[] getPayload() throws IOException {
        byte[] bytes = toBytes();
        byte[] result = new byte[getPayloadSize()];
        System.arraycopy(bytes, getHeaderSize(), result, 0, result.length);
        
        return result;
    }
    
    public String toString(String indentation) {
        return indentation+"[" + typeString(type) + "] size=" + getHeaderSize() + "+" + getPayloadSize();
    }
    
    public String toString() {
        return toString("");
    }
}