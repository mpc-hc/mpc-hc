#include "rar.hpp"

#include "hardlinks.cpp"
#include "win32stm.cpp"

#ifdef _WIN_ALL
#include "win32acl.cpp"
#include "win32lnk.cpp"
#endif

#ifdef _UNIX
#include "uowners.cpp"
#ifdef SAVE_LINKS
#include "ulinks.cpp"
#endif
#endif



#ifndef SFX_MODULE
void SetExtraInfo20(CommandData *Cmd,Archive &Arc,wchar *Name)
{
  if (Cmd->Test)
    return;
  switch(Arc.SubBlockHead.SubType)
  {
#ifdef _UNIX
    case UO_HEAD:
      if (Cmd->ProcessOwners)
        ExtractUnixOwner20(Arc,Name);
      break;
#endif
#ifdef _WIN_ALL
    case NTACL_HEAD:
      if (Cmd->ProcessOwners)
        ExtractACL20(Arc,Name);
      break;
    case STREAM_HEAD:
      ExtractStreams20(Arc,Name);
      break;
#endif
  }
}
#endif


void SetExtraInfo(CommandData *Cmd,Archive &Arc,wchar *Name)
{
#ifdef _UNIX
  if (!Cmd->Test && Cmd->ProcessOwners && Arc.Format==RARFMT15 &&
      Arc.SubHead.CmpName(SUBHEAD_TYPE_UOWNER))
    ExtractUnixOwner30(Arc,Name);
#endif
#ifdef _WIN_ALL
  if (!Cmd->Test && Cmd->ProcessOwners && Arc.SubHead.CmpName(SUBHEAD_TYPE_ACL))
    ExtractACL(Arc,Name);
  if (Arc.SubHead.CmpName(SUBHEAD_TYPE_STREAM))
    ExtractStreams(Arc,Name,Cmd->Test);
#endif
}




bool IsRelativeSymlinkSafe(const wchar *SrcName,const wchar *TargetName)
{
  if (IsFullRootPath(SrcName))
    return false;
  int AllowedDepth=0;
  while (*SrcName!=0)
  {
    if (IsPathDiv(SrcName[0]) && SrcName[1]!=0 && !IsPathDiv(SrcName[1]))
    {
      bool Dot=SrcName[1]=='.' && (IsPathDiv(SrcName[2]) || SrcName[2]==0);
      bool Dot2=SrcName[1]=='.' && SrcName[2]=='.' && (IsPathDiv(SrcName[3]) || SrcName[3]==0);
      if (!Dot && !Dot2)
        AllowedDepth++;
    }
    SrcName++;
  }
  if (IsFullRootPath(TargetName)) // Catch root dir based /path/file paths.
    return false;
  for (int Pos=0;*TargetName!=0;Pos++)
  {
    bool Dot2=TargetName[0]=='.' && TargetName[1]=='.' && 
              (IsPathDiv(TargetName[2]) || TargetName[2]==0) &&
              (Pos==0 || IsPathDiv(*(TargetName-1)));
    if (Dot2)
      AllowedDepth--;
    TargetName++;
  }
  return AllowedDepth>=0;
}


bool ExtractSymlink(CommandData *Cmd,ComprDataIO &DataIO,Archive &Arc,const wchar *LinkName)
{
#if defined(SAVE_LINKS) && defined(_UNIX)
  // For RAR 3.x archives we process links even in test mode to skip link data.
  if (Arc.Format==RARFMT15)
    return ExtractUnixLink30(Cmd,DataIO,Arc,LinkName);
  if (Arc.Format==RARFMT50)
    return ExtractUnixLink50(Cmd,LinkName,&Arc.FileHead);
#elif defined _WIN_ALL
  // RAR 5.0 archives store link information in file header, so there is
  // no need to additionally test it if we do not create a file.
  if (Arc.Format==RARFMT50)
    return CreateReparsePoint(Cmd,LinkName,&Arc.FileHead);
#endif
  return false;
}
