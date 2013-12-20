# (C) 2013 see Authors.txt
#
# This file is part of MPC-HC.
#
# MPC-HC is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# MPC-HC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from TranslationData import *


versionInfo = u'''/////////////////////////////////////////////////////////////////////////////
//
// Version
//
VS_VERSION_INFO VERSIONINFO
 FILEVERSION    MPC_VERSION_NUM
 PRODUCTVERSION MPC_VERSION_NUM
 FILEFLAGSMASK  VS_FFI_FILEFLAGSMASK
#ifdef _DEBUG
 FILEFLAGS      VS_FF_DEBUG
#else
 FILEFLAGS      0x0L
#endif
 FILEOS         VOS_NT_WINDOWS32
 FILETYPE       VFT_DLL
 FILESUBTYPE    VFT2_UNKNOWN
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "%0.4x04b0"
        BEGIN
            VALUE "Comments",         "%s"
            VALUE "CompanyName",      MPC_COMP_NAME_STR
            VALUE "FileDescription",  "%s"
            VALUE "FileVersion",      MPC_VERSION_STR
            VALUE "InternalName",     "mpc-hc"
            VALUE "LegalCopyright",   MPC_COPYRIGHT_STR
            VALUE "OriginalFilename", "mpcresources.%s.dll"
            VALUE "ProductName",      "MPC-HC"
            VALUE "ProductVersion",   MPC_VERSION_STR
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", %#x, 1200
    END
END
/////////////////////////////////////////////////////////////////////////////\n\n\n
'''


class TranslationDataRC(TranslationData):
    dialogStart = re.compile(ur'^([^ ]+) DIALOGEX \d+, \d+, \d+, \d+\r\n', re.UNICODE)
    menuStart = re.compile(ur'^([^ ]+) MENU\r\n', re.UNICODE)
    stringTableStart = re.compile(ur'^STRINGTABLE\r\n', re.UNICODE)
    dialogEntry = re.compile(ur'^\s*[^ ]+\s+"((?:[^"]|"")+)"(?:\s*,\s*([^, ]+)\s*,)?', re.UNICODE)
    menuEntry = re.compile(ur'^\s*(?:POPUP|MENUITEM) "((?:[^"]|"")+)"(?:\s*,\s*([^, ]+)\r\n)?', re.UNICODE)
    stringTableEntry = re.compile(ur'^\s*([^ ]+)\s*(?:\s+"((?:[^"]|"")+)")?\r\n', re.UNICODE)
    excludedStrings = re.compile(ur'(?:http://|\.\.\.|\d+)$', re.UNICODE | re.IGNORECASE)
    versionInfo = versionInfo.replace('\n', '\r\n')

    def loadFromRC(self, filename):
        self.empty()

        with codecs.open(filename, 'r', detectEncoding(filename)) as f:
            for line in f:
                match = TranslationDataRC.dialogStart.match(line)
                if match:
                    self.parseRCDialog(f, match.group(1))
                elif TranslationDataRC.menuStart.match(line):
                    self.parseRCMenu(f)
                elif TranslationDataRC.stringTableStart.match(line):
                    self.parseRCStringTable(f)

    def parseRCDialog(self, f, id):
        inDialog = False
        for line in f:
            if inDialog:
                if line == u'END\r\n':
                    break
                else:
                    match = TranslationDataRC.dialogEntry.match(line)
                    if match and not TranslationDataRC.excludedStrings.match(match.group(1)):
                        self.dialogs[(id + '_' + match.group(2), match.group(1))] = ''
            elif line == u'BEGIN\r\n':
                inDialog = True
            elif line.startswith(u'CAPTION'):
                match = TranslationDataRC.dialogEntry.match(line)
                if match:
                    self.dialogs[(id + '_CAPTION', match.group(1))] = ''

    def parseRCMenu(self, f):
        codecs.openedMenus = 0
        for line in f:
            if line == u'BEGIN\r\n':
                codecs.openedMenus += 1
            elif line == u'END\r\n':
                codecs.openedMenus -= 1
                if codecs.openedMenus <= 0:
                    break
            else:
                match = TranslationDataRC.menuEntry.match(line)
                if match and not TranslationDataRC.excludedStrings.match(match.group(1)):
                    id = match.group(2)
                    if not id:
                        id = 'POPUP'
                    self.menus[(id, match.group(1))] = ''

    def parseRCStringTable(self, f):
        inStringTable = False
        waitingForString = False
        stringId = None
        for line in f:
            if line == u'BEGIN\r\n':
                inStringTable = True
            elif line == u'END\r\n':
                break
            elif inStringTable:
                if waitingForString:
                    s = line.strip(' \r\n"')
                    if not TranslationDataRC.excludedStrings.match(s):
                        self.strings[(stringId, s)] = ''
                    waitingForString = False
                else:
                    match = TranslationDataRC.stringTableEntry.match(line)
                    if match:
                        stringId = match.group(1)
                        s = match.group(2)
                        if s:
                            if not TranslationDataRC.excludedStrings.match(s):
                                self.strings[(stringId, s)] = ''
                        else:
                            waitingForString = True

    def translateRC(self, filenameBase, filenameRC):
        config = ConfigParser.RawConfigParser()
        config.readfp(codecs.open('cfg\\' + filenameRC + '.cfg', 'r', 'utf8'))

        with codecs.open(filenameBase, 'r', detectEncoding(filenameBase)) as fBase, \
                codecs.open(filenameRC + '.rc', 'w', 'utf16') as fOut:
            skipLine = 0
            for line in fBase:
                if skipLine > 0:
                    skipLine -= 1
                    continue

                match = TranslationDataRC.dialogStart.match(line)
                if match:
                    fOut.write(line)
                    self.translateRCDialog(config, fBase, fOut, match.group(1))
                elif TranslationDataRC.menuStart.match(line):
                    fOut.write(line)
                    self.translateRCMenu(fBase, fOut)
                elif TranslationDataRC.stringTableStart.match(line):
                    fOut.write(line)
                    self.translateRCStringTable(fBase, fOut)
                elif line == u'#include "resource.h"\r\n':
                    fOut.write(line)
                    fOut.write('#include "version.h"\r\n')
                elif line == u'// English (United States) resources\r\n':
                    fOut.write(line.replace(u'English (United States)', config.get('Info', 'langName')))
                elif line == u'#if !defined(AFX_RESOURCE_DLL) || defined(AFX_TARG_ENU)\r\n':
                    fOut.write(line.replace(u'AFX_TARG_ENU', config.get('Info', 'langDefineMFC')))
                elif line == u'LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US\r\n':
                    fOut.write(u'LANGUAGE ' + config.get('Info', 'langDefine') + '\r\n')
                    skipLine = 1
                elif line == u'IDB_PLAYERTOOLBAR       BITMAP                  "res\\\\toolbar.bmp"\r\n':
                    fOut.write(line.replace(u'res\\\\', u'..\\\\res\\\\'))
                elif line == u'#endif    // English (United States) resources\r\n':
                    fOut.write(TranslationDataRC.versionInfo % (config.getint('Info', 'langId'),
                                                                config.get('Info', 'fileDesc'),
                                                                config.get('Info', 'fileDesc'),
                                                                config.get('Info', 'langShortName'),
                                                                config.getint('Info', 'langId')))
                    fOut.write(line.replace(u'English (United States)', config.get('Info', 'langName')))

                    for line in fBase:
                        if line != u'#include "res\\mpc-hc.rc2"  // non-Microsoft Visual C++ edited resources\r\n':
                            fOut.write(line)
                elif line != u'    "#include ""res\\\\mpc-hc.rc2""  // non-Microsoft Visual C++ edited resources\\r\\n"\r\n':
                    fOut.write(line)

    def translateRCDialog(self, config, fBase, fOut, id):
        inDialog = False
        for line in fBase:
            if inDialog:
                if line == u'END\r\n':
                    fOut.write(line)
                    break
                else:
                    match = TranslationDataRC.dialogEntry.match(line)
                    if match:
                        s = self.dialogs.get((id + '_' + match.group(2), match.group(1)))
                        if s:
                            line = line[:match.start(1)] + s + line[match.end(1):]
            elif line == u'BEGIN\r\n':
                inDialog = True
            elif line.startswith(u'CAPTION'):
                match = TranslationDataRC.dialogEntry.match(line)
                if match:
                    s = self.dialogs.get((id + '_CAPTION', match.group(1)))
                    if s:
                        line = line[:match.start(1)] + s + line[match.end(1):]
            elif line.startswith(u'FONT'):
                line = config.get('Info', 'font') + u'\r\n'

            fOut.write(line)

    def translateRCMenu(self, fBase, fOut):
        codecs.openedMenus = 0
        for line in fBase:
            if line == u'BEGIN\r\n':
                codecs.openedMenus += 1
            elif line == u'END\r\n':
                codecs.openedMenus -= 1
                if codecs.openedMenus <= 0:
                    fOut.write(line)
                    break
            else:
                match = TranslationDataRC.menuEntry.match(line)
                if match:
                    id = match.group(2)
                    if not id:
                        id = 'POPUP'
                    s = self.menus.get((id, match.group(1)))
                    if s:
                        line = line[:match.start(1)] + s + line[match.end(1):]

            fOut.write(line)

    def translateRCStringTable(self, fBase, fOut):
        inStringTable = False
        waitingForString = False
        stringId = None
        linePrec = None
        for line in fBase:
            if line == u'BEGIN\r\n':
                inStringTable = True
            elif line == u'END\r\n':
                fOut.write(line)
                break
            elif inStringTable:
                if waitingForString:
                    line = line.strip(' \r\n"')
                    s = self.strings.get((stringId, line))
                    if s:
                        line = linePrec + u' "' + s + u'"\r\n'
                    else:
                        line = linePrec + u' "' + line + u'"\r\n'
                    waitingForString = False
                else:
                    match = TranslationDataRC.stringTableEntry.match(line)
                    if match:
                        stringId = match.group(1)
                        s = match.group(2)
                        if s:
                            s = self.strings.get((stringId, s))
                            if s:
                                line = line[:match.start(2)] + s + line[match.end(2):]
                        else:
                            waitingForString = True
                            linePrec = line[:-2]

            if not waitingForString:
                fOut.write(line)
