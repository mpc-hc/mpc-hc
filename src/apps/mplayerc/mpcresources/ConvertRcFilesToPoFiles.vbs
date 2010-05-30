' Run "cscript.exe //X "ConvertRcFilesToPoFiles.vbs"" for debugging

Option Explicit
''
' This script converts the language RC files to the language PO files.
'
' Copyright (C) 2007 by Tim Gerundt
' Released under the "GNU General Public License"
'
' ID line follows -- this is updated by SVN
' $Id$

Const ForReading = 1

Const NO_BLOCK = 0
Const MENU_BLOCK = 1
Const DIALOGEX_BLOCK = 2
Const STRINGTABLE_BLOCK = 3
Const VERSIONINFO_BLOCK = 4

Dim oFSO, bRunFromCmd

Set oFSO = CreateObject("Scripting.FileSystemObject")

bRunFromCmd = False
If (LCase(Right(Wscript.FullName, 11))) = "cscript.exe" Then
  bRunFromCmd = True
End If

Call Main

''
' ...
Sub Main
  Dim oLanguages, sLanguage
  Dim oOriginalTranslations, oLanguageTranslations, oMergedTranslations
  Dim StartTime, EndTime, Seconds
  
  StartTime = Time
  
  Wscript.Echo "Warning: " & Wscript.ScriptName & " can take several minutes to finish!"
  
  If (oFSO.FileExists("English.pot") = True) Then 'If the master POT file exists...
	Set oOriginalTranslations = GetTranslationsFromRcFile("../mplayerc.rc")

	'~ Set oLanguageTranslations = GetTranslationsFromRcFile("German\MergeGerman.rc")
	'~ Set oMergedTranslations = MergeTranslations(oOriginalTranslations, oLanguageTranslations)
	'~ CreatePoFileWithTranslations "English.pot", "German\German.po", oMergedTranslations

	'~ Set oLanguageTranslations = GetTranslationsFromRcFile("ChineseSimplified\MergeChineseSimplified.rc")
	'~ Set oMergedTranslations = MergeTranslations(oOriginalTranslations, oLanguageTranslations)
	'~ CreatePoFileWithTranslations "English.pot", "ChineseSimplified\ChineseSimplified.po", oMergedTranslations

	Set oLanguages = GetLanguages
	For Each sLanguage In oLanguages.Keys 'For all languages...
	  If (bRunFromCmd = True) Then 'If run from command line...
	    Wscript.Echo sLanguage
	  End If
	  Set oLanguageTranslations = GetTranslationsFromRcFile(oLanguages(sLanguage))
	  Set oMergedTranslations = MergeTranslations(oOriginalTranslations, oLanguageTranslations)
	  If (oMergedTranslations.Count > 0) Then 'If translations exists...
	    CreatePoFileWithTranslations "English.pot", sLanguage & ".po", oMergedTranslations
	  End If
	Next
  End If
 
  EndTime = Time
  Seconds = DateDiff("s", StartTime, EndTime)
  
  Wscript.Echo Wscript.ScriptName & " finished after " & Seconds & " seconds!"
End Sub

''
' ...
Function GetLanguages()
  Dim oLanguages, oFile, name, ext
  
  Set oLanguages = CreateObject("Scripting.Dictionary")

  For Each oFile In oFSO.GetFolder(".").Files 'For all subfolders in the current folder...
    ext = oFSO.GetExtensionName(oFile)
    name = oFSO.GetBaseName(oFile)
    If (ext = "rc") Then
      oLanguages.Add name, oFile.Name
    End If
  Next
  Set GetLanguages = oLanguages
End Function

''
' ...
Function GetTranslationsFromRcFile(ByVal sRcPath)
  Dim oTranslations, oTextFile, sLine
  Dim oMatch, iBlockType, sKey1, sKey2, iPosition, sValue
  Dim sLang, sSubLang, sCodePage
  
  Set oTranslations = CreateObject("Scripting.Dictionary")
  
  If (oFSO.FileExists(sRcPath) = True) Then
    iBlockType = NO_BLOCK
    sKey1 = ""
    sKey2 = ""
    iPosition = 0
    sCodePage = ""
    Set oTextFile = oFSO.OpenTextFile(sRcPath, ForReading, False, -1)
    Do Until oTextFile.AtEndOfStream = True
      sLine = Trim(oTextFile.ReadLine)
      
      sValue = ""
      
      If (FoundRegExpMatch(sLine, "(IDR_.*) MENU", oMatch) = True) Then 'MENU...
        iBlockType = MENU_BLOCK
        sKey1 = oMatch.SubMatches(0)
        iPosition = 0
      ElseIf (FoundRegExpMatch(sLine, "(IDD_.*) DIALOGEX", oMatch) = True) Then 'DIALOGEX...
        iBlockType = DIALOGEX_BLOCK
        sKey1 = oMatch.SubMatches(0)
        iPosition = 0
      ElseIf (sLine = "STRINGTABLE") Then 'STRINGTABLE...
        iBlockType = STRINGTABLE_BLOCK
        sKey1 = "STRINGTABLE"
        'iPosition = 0
      ElseIf (FoundRegExpMatch(sLine, "(VS_.*) VERSIONINFO", oMatch) = True) Then 'VERSIONINFO...
        iBlockType = VERSIONINFO_BLOCK
        sKey1 = "VERSIONINFO"
        iPosition = 0
      ElseIf (sLine = "END") Then 'END...
        If (iBlockType = STRINGTABLE_BLOCK) Then 'If inside stringtable...
          iBlockType = NO_BLOCK
          sKey1 = ""
          'iPosition = 0
        End If
      ElseIf (sLine <> "") Then 'If NOT empty line...
        Select Case iBlockType
          Case NO_BLOCK:
            If (FoundRegExpMatch(sLine, "LANGUAGE (LANG_\w*), (SUBLANG_\w*)", oMatch) = True) Then 'LANGUAGE...
              sLang = oMatch.SubMatches(0)
              sSubLang = oMatch.SubMatches(1)
            ElseIf (FoundRegExpMatch(sLine, "code_page\(([\d]+)\)", oMatch) = True) Then 'code_page...
              sCodePage = oMatch.SubMatches(0)
            End If
            
          Case MENU_BLOCK:
            If (FoundRegExpMatch(sLine, "POPUP ""(.*)""", oMatch) = True) Then 'POPUP...
              If (InStr(oMatch.SubMatches(0), "_POPUP_") = 0) Then
                sKey2 = iPosition
                iPosition = iPosition + 1
                sValue = oMatch.SubMatches(0)
              End If
            ElseIf (FoundRegExpMatch(sLine, "MENUITEM.*""(.*)"".*(ID_.*)", oMatch) = True) Then 'MENUITEM...
              sKey2 = oMatch.SubMatches(1)
              sValue = oMatch.SubMatches(0)
            End If
            
          Case DIALOGEX_BLOCK:
            If (FoundRegExpMatch(sLine, "CAPTION.*""(.*)""", oMatch) = True) Then 'CAPTION...
              sKey2 = "CAPTION"
              sValue = oMatch.SubMatches(0)
            ElseIf (FoundRegExpMatch(sLine, "PUSHBUTTON.*""(.*)"",(\w+)", oMatch) = True) Then 'DEFPUSHBUTTON/PUSHBUTTON...
              sKey2 = oMatch.SubMatches(1)
              sValue = oMatch.SubMatches(0)
            ElseIf (FoundRegExpMatch(sLine, "[L|R|C]TEXT.*""(.*)"",(\w+)", oMatch) = True) Then 'LTEXT/RTEXT...
              If (oMatch.SubMatches(0) <> "") And (oMatch.SubMatches(0) <> "Static") Then
                If (oMatch.SubMatches(1) <> "IDC_STATIC") Then
                  sKey2 = oMatch.SubMatches(1)
                Else
                  sKey2 = iPosition & "_TEXT"
                  iPosition = iPosition + 1
                End If
                sValue = oMatch.SubMatches(0)
              End If
            ElseIf (FoundRegExpMatch(sLine, "[L|R]TEXT.*""(.*)"",", oMatch) = True) Then 'LTEXT/RTEXT (without ID)...
              sKey2 = iPosition & "_TEXT"
              iPosition = iPosition + 1
              sValue = oMatch.SubMatches(0)
            ElseIf (FoundRegExpMatch(sLine, "CONTROL +""(.*?)"",(\w+)", oMatch) = True) Then 'CONTROL...
              If (oMatch.SubMatches(0) <> "Dif") And (oMatch.SubMatches(0) <> "Btn") And (oMatch.SubMatches(0) <> "Button1") Then
                sKey2 = oMatch.SubMatches(1)
                sValue = oMatch.SubMatches(0)
              End If
            ElseIf (FoundRegExpMatch(sLine, "CONTROL +""(.*?)"",", oMatch) = True) Then 'CONTROL (without ID)...
              sKey2 = iPosition & "_CONTROL"
              iPosition = iPosition + 1
              sValue = oMatch.SubMatches(0)
            ElseIf (FoundRegExpMatch(sLine, "GROUPBOX +""(.*?)"",(\w+)", oMatch) = True) Then 'GROUPBOX...
              If (oMatch.SubMatches(1) <> "IDC_STATIC") Then
                sKey2 = oMatch.SubMatches(1)
              Else
                sKey2 = iPosition & "_GROUPBOX"
                iPosition = iPosition + 1
              End If
              sValue = oMatch.SubMatches(0)
            End If
            
          Case STRINGTABLE_BLOCK:
            If (FoundRegExpMatch(sLine, "(\w+).*""(.*)""", oMatch) = True) Then 'String...
              sKey2 = oMatch.SubMatches(0)
              sValue = oMatch.SubMatches(1)
            ElseIf (FoundRegExpMatch(sLine, """(.*)""", oMatch) = True) Then 'String (without ID)...
              sKey2 = iPosition
              iPosition = iPosition + 1
              sValue = oMatch.SubMatches(0)
            End If
            
          Case VERSIONINFO_BLOCK:
            If (FoundRegExpMatch(sLine, "BLOCK ""([0-9A-F]+)""", oMatch) = True) Then 'StringFileInfo.Block...
              sKey2 = "STRINGFILEINFO_BLOCK"
              sValue = oMatch.SubMatches(0)
            ElseIf (FoundRegExpMatch(sLine, "VALUE ""(.*?)"", ""(.*?)\\?0?""", oMatch) = True) Then 'StringFileInfo.Value...
              sKey2 = "STRINGFILEINFO_" & oMatch.SubMatches(0)
              sValue = oMatch.SubMatches(1)
            ElseIf (FoundRegExpMatch(sLine, "VALUE ""Translation"", (.*?)$", oMatch) = True) Then 'VarFileInfo.Translation...
              sKey2 = "VARFILEINFO_TRANSLATION"
              sValue = oMatch.SubMatches(0)
            End If
            
        End Select
      End If
      
      If (sValue <> "") Then
        Dim key
        key = sKey1 & "." & sKey2
        If (not oTranslations.Exists(key)) Then
            oTranslations.Add key, sValue
        Else
            Dim newKey, oldValue, iNum
            iNum = 1
            newKey = key
            Do Until not oTranslations.Exists(newKey)
                oldValue = oTranslations(newKey)
                iNum = iNum + 1
                newKey = key & iNum
            Loop
            oTranslations.Add newKey, sValue
        End If
      End If
    Loop
    oTextFile.Close
    
    oTranslations.Add "__LANGUAGE__", sLang & ", " & sSubLang
    oTranslations.Add "__CODEPAGE__", sCodePage
  End If
  Set GetTranslationsFromRcFile = oTranslations
End Function

''
' ...
Function MergeTranslations(ByVal oOriginalTranslations, ByVal oLanguageTranslations)
  Dim oMergedTranslations, sKey
  Dim sOriginalTranslation, sLanguageTranslation
  
  Set oMergedTranslations = CreateObject("Scripting.Dictionary")
  For Each sKey In oOriginalTranslations.Keys 'For all original translations...
    sOriginalTranslation = oOriginalTranslations(sKey)
    sLanguageTranslation = oLanguageTranslations(sKey)
    
    If (sOriginalTranslation <> "") And (sOriginalTranslation <> sLanguageTranslation) Then
      If (oMergedTranslations.Exists(sOriginalTranslation) = False) Then
        oMergedTranslations.Add oOriginalTranslations(sKey), oLanguageTranslations(sKey)
      End If
    End If
  Next
  oMergedTranslations.Add "__CODEPAGE__", oLanguageTranslations("__CODEPAGE__")
  Set MergeTranslations = oMergedTranslations
End Function

''
' ...
Sub CreatePoFileWithTranslations(ByVal sMasterPotPath, ByVal sLanguagePoPath, ByVal oTranslations)
  Dim oMasterPotFile, sMasterLine
  Dim oLanguagePoFile, sLanguageLine
  Dim oMatch, sMsgId, sMsgStr, sKey
  
  If (oFSO.FileExists(sMasterPotPath) = True) Then 'If the master POT file exists...
    sMsgId = ""
    sMsgStr = ""
    Set oMasterPotFile = oFSO.OpenTextFile(sMasterPotPath, ForReading, False, -1)
    Set oLanguagePoFile = oFSO.CreateTextFile(sLanguagePoPath, True, True)
    Do Until oMasterPotFile.AtEndOfStream = True 'For all lines...
      sMasterLine = oMasterPotFile.ReadLine
      sLanguageLine = sMasterLine
      
      If (Trim(sMasterLine) <> "") Then 'If NOT empty line...
        If (FoundRegExpMatch(sMasterLine, "msgid ""(.*)""", oMatch) = True) Then 'If "msgid"...
          sMsgId = oMatch.SubMatches(0)
          If (oTranslations.Exists(sMsgId) = True) Then 'If translation located...
            sMsgStr = oTranslations(sMsgId)
          End If
        ElseIf (FoundRegExpMatch(sMasterLine, "msgstr """"", oMatch) = True) Then 'If "msgstr"...
          If (sMsgId = "1252") And (sMsgStr = "") Then 'If same codepage...
            sMsgStr = oTranslations("__CODEPAGE__")
          End If
          If (sMsgStr <> "") Then 'If translated...
            sLanguageLine = Replace(sMasterLine, "msgstr """"", "msgstr """ & sMsgStr & """")
          End If
        ElseIf (FoundRegExpMatch(sMasterLine, "CP1252", oMatch) = True) Then 'If "Codepage"...
          sLanguageLine = Replace(sMasterLine, "CP1252", "CP" & oTranslations("__CODEPAGE__"))
        ElseIf (FoundRegExpMatch(sMasterLine, "English", oMatch) = True) Then 'If "English"...
          sLanguageLine = Replace(sMasterLine, "English", oFSO.GetBaseName(sLanguagePoPath))
        End If
      Else 'If empty line
        sMsgId = ""
        sMsgStr = ""
      End If
      
      oLanguagePoFile.WriteLine sLanguageLine
    Loop
    oMasterPotFile.Close
    oLanguagePoFile.Close
  End If
End Sub

''
' ...
Function FoundRegExpMatch(ByVal sString, ByVal sPattern, ByRef oMatchReturn)
  Dim oRegExp, oMatches
  
  Set oRegExp = New RegExp
  oRegExp.Pattern = sPattern
  oRegExp.IgnoreCase = True
  
  oMatchReturn = Null
  FoundRegExpMatch = False
  If (oRegExp.Test(sString) = True) Then
    Set oMatches = oRegExp.Execute(sString)
    Set oMatchReturn = oMatches(0)
    FoundRegExpMatch = True
  End If
End Function
