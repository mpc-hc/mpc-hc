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

import os
import codecs
import re
from collections import OrderedDict
import itertools
import ConfigParser
from datetime import datetime


def xstr(s):
    return '' if s is None else str(s)


def detectEncoding(filename):
    encoding = 'cp1252'

    with codecs.open(filename, 'rb') as f:
        bytes = min(32, os.path.getsize(filename))
        raw = f.read(bytes)

        if raw.startswith(codecs.BOM_UTF8):
            encoding = 'utf-8-sig'
        elif raw.startswith(codecs.BOM_UTF16_BE):
            encoding = 'utf_16_be'
        elif raw.startswith(codecs.BOM_UTF16_LE):
            encoding = 'utf_16_le'
        elif raw.startswith(codecs.BOM_UTF32_BE):
            encoding = 'utf_32_be'
        elif raw.startswith(codecs.BOM_UTF32_LE):
            encoding = 'utf_32_le'

    return encoding


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
            VALUE "OriginalFilename", "%s"
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

potHeader = u'''# MPC-HC - %s
# Copyright (C) 2002 - 2013 see Authors.txt
# This file is distributed under the same license as the MPC-HC package.
msgid ""
msgstr ""
"Project-Id-Version: MPC-HC\\n"
"POT-Creation-Date: %s+0000\\n"
"PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\\n"
"Last-Translator: FULL NAME <EMAIL@ADDRESS>\\n"
"Language-Team: LANGUAGE <LL@li.org>\\n"
"MIME-Version: 1.0\\n"
"Content-Type: text/plain; charset=UTF-8\\n"
"Content-Transfer-Encoding: 8bit\\n"
"Language: \\n"\n
'''


class TranslationData:
    dialogStart = re.compile(ur'^([^ ]+) DIALOGEX \d+, \d+, \d+, \d+\r\n', re.UNICODE)
    menuStart = re.compile(ur'^([^ ]+) MENU\r\n', re.UNICODE)
    stringTableStart = re.compile(ur'^STRINGTABLE\r\n', re.UNICODE)
    dialogEntry = re.compile(ur'^\s*[^ ]+\s+"((?:[^"]|"")+)"(?:\s*,\s*([^, ]+)\s*,)?', re.UNICODE)
    menuEntry = re.compile(ur'^\s*(?:POPUP|MENUITEM) "((?:[^"]|"")+)"(?:\s*,\s*([^, ]+)\r\n)?', re.UNICODE)
    stringTableEntry = re.compile(ur'^\s*([^ ]+)\s*(?:\s+"((?:[^"]|"")+)")?\r\n', re.UNICODE)
    poLine = re.compile(ur'^\s*(?:(msgctxt|msgid|msgstr)\s+)?"((?:[^"]|\\")*)"\r?\n', re.UNICODE)
    versionInfo = versionInfo.replace('\n', '\r\n')
    potHeader = potHeader.replace('\n', '\r\n')
    excludedStrings = re.compile(ur'(?:http://|\.\.\.|\d+)$', re.UNICODE | re.IGNORECASE)

    def __init__(self):
        self.empty()

    def empty(self):
        self.dialogsHeader = None
        self.menusHeader = None
        self.stringsHeader = None
        self.dialogs = OrderedDict()
        self.menus = OrderedDict()
        self.strings = OrderedDict()

    def loadFromRC(self, filename):
        self.empty()

        with codecs.open(filename, 'r', detectEncoding(filename)) as f:
            for line in f:
                match = TranslationData.dialogStart.match(line)
                if match:
                    self.parseRCDialog(f, match.group(1))
                elif TranslationData.menuStart.match(line):
                    self.parseRCMenu(f)
                elif TranslationData.stringTableStart.match(line):
                    self.parseRCStringTable(f)

    def parseRCDialog(self, f, id):
        inDialog = False
        for line in f:
            if inDialog:
                if line == u'END\r\n':
                    break
                else:
                    match = TranslationData.dialogEntry.match(line)
                    if match and not TranslationData.excludedStrings.match(match.group(1)):
                        self.dialogs[(id + '_' + match.group(2), match.group(1))] = ''
            elif line == u'BEGIN\r\n':
                inDialog = True
            elif line.startswith(u'CAPTION'):
                match = TranslationData.dialogEntry.match(line)
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
                match = TranslationData.menuEntry.match(line)
                if match and not TranslationData.excludedStrings.match(match.group(1)):
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
                    if not TranslationData.excludedStrings.match(s):
                        self.strings[(stringId, s)] = ''
                    waitingForString = False
                else:
                    match = TranslationData.stringTableEntry.match(line)
                    if match:
                        stringId = match.group(1)
                        s = match.group(2)
                        if s:
                            if not TranslationData.excludedStrings.match(s):
                                self.strings[(stringId, s)] = ''
                        else:
                            waitingForString = True

    def loadFromPO(self, filename, ext):
        self.empty()

        self.loadDataFromPO(filename + '.dialogs.' + ext,
                            self.dialogs, 'dialogsHeader')
        self.loadDataFromPO(filename + '.menus.' + ext,
                            self.menus, 'menusHeader')
        self.loadDataFromPO(filename + '.strings.' + ext,
                            self.strings, 'stringsHeader')

    def loadDataFromPO(self, filename, data, header=None):
        with codecs.open(filename, 'r', 'utf8') as f:
            if header:
                setattr(self, header, self.readHeaderFromPO(f))

            hasNext = True
            while hasNext:
                poEntry = self.readPOEntry(f, True)
                if poEntry:
                    data[(poEntry[0], poEntry[1])] = poEntry[2]
                else:
                    hasNext = False

    def readHeaderFromPO(self, f):
        header = None
        line = f.readline()
        if line.startswith(u'#'):
            header = []
            header.append(line)
            for line in f:
                header.append(line)
                if line.startswith(u'\r') or line.startswith(u'\n'):
                    break
            header = ''.join(header)
        else:
            f.seek(0)
        return header

    def readPOEntry(self, f, needContext):
        context = None
        id = None
        str = None
        show = False

        prevLineType = None
        for line in f:
            match = TranslationData.poLine.match(line)
            if match:
                lineType = match.group(1)
                if lineType is None:
                    lineType = prevLineType

                data = match.group(2).replace('\\"', '""')
                if lineType == u'msgctxt':
                    context = xstr(context) + data
                elif lineType == u'msgid':
                    id = xstr(id) + data
                elif lineType == u'msgstr':
                    str = xstr(str) + data

                prevLineType = lineType
            elif (not needContext or context is not None) and id is not None and str is not None:
                return (context, id, str)

        return (context, id, str) if (not needContext or context is not None) and id is not None and str is not None else None

    def writePO(self, filename, ext, output=(True, True, True)):
        self.prepareHeaders(ext)

        if output[0]:
            self.writePOData(filename + '.dialogs.' + ext,
                             self.dialogs, self.dialogsHeader)
        if output[1]:
            self.writePOData(filename + '.menus.' + ext,
                             self.menus, self.menusHeader)
        if output[2]:
            self.writePOData(filename + '.strings.' + ext,
                             self.strings, self.stringsHeader)

    def prepareHeaders(self, ext):
        if ext == 'pot':
            self.dialogsHeader = self.menusHeader = self.stringsHeader = None

        utcnow = datetime.utcnow().replace(microsecond=0).isoformat(' ')

        if not self.dialogsHeader:
            self.dialogsHeader = TranslationData.potHeader % ('Strings extracted from dialogs', utcnow)

        if not self.menusHeader:
            self.menusHeader = TranslationData.potHeader % ('Strings extracted from menus', utcnow)

        if not self.stringsHeader:
            self.stringsHeader = TranslationData.potHeader % ('Strings extracted from string tables', utcnow)

    def writePOData(self, filename, data, header=None):
        with codecs.open(filename, 'w', 'utf8') as f:
            if header:
                f.write(header)

            for dataID in data:
                f.write('msgctxt "')
                f.write(dataID[0].replace('""', '\\"'))    # msgctxt
                f.write('"\r\n')
                f.write('msgid "')
                f.write(dataID[1].replace('""', '\\"'))    # msgid
                f.write('"\r\n')
                f.write('msgstr "')
                f.write(data[dataID].replace('""', '\\"')) # msgstr
                f.write('"\r\n\r\n')

    def areEqualsSections(self, translationData):
        return (self.dialogs == translationData.dialogs,
                self.menus == translationData.menus,
                self.strings == translationData.strings)

    # This shouldn't be used in general since it isn't safe at all
    # but we need it to migrate from our old system.
    def translateFromTemplate(self, translationData):
        for dataPair in ((self.dialogs, translationData.dialogs), (self.menus, translationData.menus), (self.strings, translationData.strings)):
            for (dataID, dataIDTranslated) in itertools.izip(*dataPair):
                if dataID[1] != dataIDTranslated[1]:
                    dataPair[0][dataID] = dataIDTranslated[1]

    def translate(self, translationData):
        self.dialogsHeader = translationData.dialogsHeader
        self.menusHeader = translationData.menusHeader
        self.stringsHeader = translationData.stringsHeader

        for dataPair in ((self.dialogs, translationData.dialogs), (self.menus, translationData.menus), (self.strings, translationData.strings)):
            for dataID in dataPair[0]:
                if dataID in dataPair[1]:
                    dataPair[0][dataID] = dataPair[1][dataID]

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

                match = TranslationData.dialogStart.match(line)
                if match:
                    fOut.write(line)
                    self.translateRCDialog(config, fBase, fOut, match.group(1))
                elif TranslationData.menuStart.match(line):
                    fOut.write(line)
                    self.translateRCMenu(fBase, fOut)
                elif TranslationData.stringTableStart.match(line):
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
                    fOut.write(TranslationData.versionInfo % (config.getint('Info', 'langId'),
                                                              config.get('Info', 'fileDesc'),
                                                              config.get('Info', 'fileDesc'),
                                                              config.get('Info', 'fileName'),
                                                              config.getint('Info', 'langId')))
                    fOut.write(line.replace(u'English (United States)', config.get('Info', 'langName')))

                    for line in fBase:
                        if line != u'#include "res\\mplayerc.rc2"  // non-Microsoft Visual C++ edited resources\r\n':
                            fOut.write(line)
                elif line != u'    "#include ""res\\\\mplayerc.rc2""  // non-Microsoft Visual C++ edited resources\\r\\n"\r\n':
                    fOut.write(line)

    def translateRCDialog(self, config, fBase, fOut, id):
        inDialog = False
        for line in fBase:
            if inDialog:
                if line == u'END\r\n':
                    fOut.write(line)
                    break
                else:
                    match = TranslationData.dialogEntry.match(line)
                    if match:
                        s = self.dialogs.get((id + '_' + match.group(2), match.group(1)))
                        if s:
                            line = line[:match.start(1)] + s + line[match.end(1):]
            elif line == u'BEGIN\r\n':
                inDialog = True
            elif line.startswith(u'CAPTION'):
                match = TranslationData.dialogEntry.match(line)
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
                match = TranslationData.menuEntry.match(line)
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
                    match = TranslationData.stringTableEntry.match(line)
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
