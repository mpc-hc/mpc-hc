#ifndef _RAR_RAWREAD_
#define _RAR_RAWREAD_

class RawRead
{
  private:
    Array<byte> Data;
    File *SrcFile;
    size_t DataSize;
    size_t ReadPos;
#ifndef SHELL_EXT
    CryptData *Crypt;
#endif
  public:
    RawRead();
    RawRead(File *SrcFile);
    void Reset();
    size_t Read(size_t Size);
    void Read(byte *SrcData,size_t Size);
    byte   Get1();
    ushort Get2();
    uint   Get4();
    uint64 Get8();
    uint64 GetV();
    uint   GetVSize(size_t Pos);
    size_t GetB(void *Field,size_t Size);
    void GetW(wchar *Field,size_t Size);
    uint GetCRC15(bool ProcessedOnly);
    uint GetCRC50();
    byte* GetDataPtr() {return &Data[0];}
    size_t Size() {return DataSize;}
    size_t PaddedSize() {return Data.Size()-DataSize;}
    size_t DataLeft() {return DataSize-ReadPos;}
    size_t GetPos() {return ReadPos;}
    void SetPos(size_t Pos) {ReadPos=Pos;}
    void Skip(size_t Size) {ReadPos+=Size;}
    void Rewind() {SetPos(0);}
#ifndef SHELL_EXT
    void SetCrypt(CryptData *Crypt) {RawRead::Crypt=Crypt;}
#endif
};

uint64 RawGetV(const byte *Data,uint &ReadPos,uint DataSize,bool &Overflow);

inline uint RawGet2(const void *Data)
{
  byte *D=(byte *)Data;
  return D[0]+(D[1]<<8);
}

inline uint RawGet4(const void *Data)
{
  byte *D=(byte *)Data;
#if defined(BIG_ENDIAN) || !defined(ALLOW_MISALIGNED) || !defined(PRESENT_INT32)
  return D[0]+(D[1]<<8)+(D[2]<<16)+(D[3]<<24);
#else
  return GET_UINT32(*(uint32 *)D);
#endif
}

inline uint64 RawGet8(const void *Data)
{
  byte *D=(byte *)Data;
  return INT32TO64(RawGet4(D+4),RawGet4(D));
}


// We need these "put" functions also in UnRAR code. This is why they are
// in rawread.hpp file even though they are "write" functions.
inline void RawPut2(uint Field,void *Data)
{
  byte *D=(byte *)Data;
  D[0]=(byte)(Field);
  D[1]=(byte)(Field>>8);
}


inline void RawPut4(uint Field,void *Data)
{
  byte *D=(byte *)Data;
#if defined(BIG_ENDIAN) || !defined(ALLOW_MISALIGNED) || !defined(PRESENT_INT32)
  D[0]=(byte)(Field);
  D[1]=(byte)(Field>>8);
  D[2]=(byte)(Field>>16);
  D[3]=(byte)(Field>>24);
#else
  *(uint32 *)D=Field;
#endif
}


inline void RawPut8(uint64 Field,void *Data)
{
  byte *D=(byte *)Data;
  D[0]=(byte)(Field);
  D[1]=(byte)(Field>>8);
  D[2]=(byte)(Field>>16);
  D[3]=(byte)(Field>>24);
  D[4]=(byte)(Field>>32);
  D[5]=(byte)(Field>>40);
  D[6]=(byte)(Field>>48);
  D[7]=(byte)(Field>>56);
}

#endif
