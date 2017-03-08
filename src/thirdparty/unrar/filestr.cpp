#include "rar.hpp"

bool ReadTextFile(
  const wchar *Name,
  StringList *List,
  bool Config,
  bool AbortOnError,
  RAR_CHARSET SrcCharset,
  bool Unquote,
  bool SkipComments,
  bool ExpandEnvStr)
{
  wchar FileName[NM];
  *FileName=0;

  if (Name!=NULL)
    if (Config)
      GetConfigName(Name,FileName,ASIZE(FileName),true,false);
    else
      wcsncpyz(FileName,Name,ASIZE(FileName));

  File SrcFile;
  if (*FileName!=0)
  {
    bool OpenCode=AbortOnError ? SrcFile.WOpen(FileName):SrcFile.Open(FileName,0);

    if (!OpenCode)
    {
      if (AbortOnError)
        ErrHandler.Exit(RARX_OPEN);
      return false;
    }
  }
  else
    SrcFile.SetHandleType(FILE_HANDLESTD);

  uint DataSize=0,ReadSize;
  const int ReadBlock=4096;

  // Our algorithm below needs at least two trailing zeroes after data.
  // So for Unicode we provide 2 Unicode zeroes and one more byte
  // in case read Unicode data contains uneven number of bytes.
  const size_t ZeroPadding=5;

  Array<byte> Data(ReadBlock+ZeroPadding);
  while ((ReadSize=SrcFile.Read(&Data[DataSize],ReadBlock))!=0)
  {
    DataSize+=ReadSize;
    Data.Add(ReadSize); // Always have ReadBlock available for next data.
  }
  
  memset(&Data[DataSize],0,ZeroPadding); // Provide at least 2 Unicode zero bytes.

  Array<wchar> WideStr;

  int LowEndian=Data[0]==255 && Data[1]==254 ? 1:0;
  int BigEndian=Data[0]==254 && Data[1]==255 ? 1:0;

  bool IsUnicode=false;
  if (LowEndian || BigEndian)  
    for (size_t I=2;I<DataSize;I++)
      if (Data[I]<32 && Data[I]!='\r' && Data[I]!='\n')
      {
        IsUnicode=true; // High byte in UTF-16 char is found.
        break;
      }

  if (SrcCharset==RCH_UNICODE || SrcCharset==RCH_DEFAULT && IsUnicode)
  {
    size_t Start=2; // Skip byte order mark.
    if (!LowEndian && !BigEndian) // No byte order mask.
    {
      Start=0;
      LowEndian=1;
    }
    
    Array<wchar> DataW(Data.Size()/2+1);
    for (size_t I=Start;I<Data.Size()-1;I+=2)
      DataW[(I-Start)/2]=Data[I+BigEndian]+Data[I+LowEndian]*256;

    wchar *CurStr=&DataW[0];

    while (*CurStr!=0)
    {
      wchar *NextStr=CurStr,*CmtPtr=NULL;
      while (*NextStr!='\r' && *NextStr!='\n' && *NextStr!=0)
      {
        if (SkipComments && NextStr[0]=='/' && NextStr[1]=='/')
        {
          *NextStr=0;
          CmtPtr=NextStr;
        }
        NextStr++;
      }
      *NextStr=0;
      for (wchar *SpacePtr=(CmtPtr ? CmtPtr:NextStr)-1;SpacePtr>=CurStr;SpacePtr--)
      {
        if (*SpacePtr!=' ' && *SpacePtr!='\t')
          break;
        *SpacePtr=0;
      }
      if (*CurStr!=0)
      {
        size_t Length=wcslen(CurStr);

        if (Unquote && *CurStr=='\"' && CurStr[Length-1]=='\"')
        {
          CurStr[Length-1]=0;
          CurStr++;
        }

        bool Expanded=false;
#ifdef _WIN_ALL
        if (ExpandEnvStr && *CurStr=='%')
        {
          // Expanding environment variables in Windows version.

          wchar ExpName[NM];
          *ExpName=0;
          DWORD Result=ExpandEnvironmentStrings(CurStr,ExpName,ASIZE(ExpName));
          Expanded=Result!=0 && Result<ASIZE(ExpName);
          if (Expanded)
            List->AddString(ExpName);
        }
#endif
        if (!Expanded)
          List->AddString(CurStr);
      }
      CurStr=NextStr+1;
      while (*CurStr=='\r' || *CurStr=='\n')
        CurStr++;
    }
  }
  else
  {
    char *CurStr=(char *)&Data[0];
    while (*CurStr!=0)
    {
      char *NextStr=CurStr,*CmtPtr=NULL;
      while (*NextStr!='\r' && *NextStr!='\n' && *NextStr!=0)
      {
        if (SkipComments && NextStr[0]=='/' && NextStr[1]=='/')
        {
          *NextStr=0;
          CmtPtr=NextStr;
        }
        NextStr++;
      }
      *NextStr=0;
      for (char *SpacePtr=(CmtPtr ? CmtPtr:NextStr)-1;SpacePtr>=CurStr;SpacePtr--)
      {
        if (*SpacePtr!=' ' && *SpacePtr!='\t')
          break;
        *SpacePtr=0;
      }
      if (*CurStr!=0)
      {
        if (Unquote && *CurStr=='\"')
        {
          size_t Length=strlen(CurStr);
          if (CurStr[Length-1]=='\"')
          {
            CurStr[Length-1]=0;
            CurStr++;
          }
        }
#if defined(_WIN_ALL)
        if (SrcCharset==RCH_OEM)
          OemToCharA(CurStr,CurStr);
#endif

        bool Expanded=false;

        WideStr.Alloc(strlen(CurStr)+1);
        CharToWide(CurStr,&WideStr[0],WideStr.Size());
#ifdef _WIN_ALL
        if (ExpandEnvStr && *CurStr=='%')
        {
          // Expanding environment variables in Windows version.
          wchar ExpName[NM];
          DWORD Result=ExpandEnvironmentStringsW(&WideStr[0],ExpName,ASIZE(ExpName));
          Expanded=Result!=0 && Result<ASIZE(ExpName);
          if (Expanded)
            List->AddString(ExpName);
        }
#endif
        if (!Expanded)
          List->AddString(&WideStr[0]);
      }
      CurStr=NextStr+1;
      while (*CurStr=='\r' || *CurStr=='\n')
        CurStr++;
    }
  }
  return true;
}
