# (C) 2013, 2016-2017 see Authors.txt
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
    return '' if s is None else s


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


potHeader = u'''# MPC-HC - %s
# Copyright (C) 2002 - 2017 see Authors.txt
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
    poLine = re.compile(ur'^\s*(?:(msgctxt|msgid|msgstr)\s+)?"((?:[^"]|\\")*)"\r?\n', re.UNICODE)
    potHeader = potHeader.replace('\n', '\r\n')

    def __init__(self):
        self.empty()

    def empty(self):
        self.dialogsHeader = None
        self.menusHeader = None
        self.stringsHeader = None
        self.dialogs = OrderedDict()
        self.menus = OrderedDict()
        self.strings = OrderedDict()

    def loadFromPO(self, filename, ext, input=(True, True, True)):
        self.empty()

        if input[0]:
            self.loadDataFromPO(filename + '.dialogs.' + ext,
                                self.dialogs, 'dialogsHeader')
        if input[1]:
            self.loadDataFromPO(filename + '.menus.' + ext,
                                self.menus, 'menusHeader')
        if input[2]:
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
            header.append(line.rstrip(u'\r\n'))
            for line in f:
                line = line.rstrip(u'\r\n')
                header.append(line)
                if not line:
                    # Ensure a final line-break is added
                    header.append('')
                    break
            header = '\r\n'.join(header)
        else:
            f.seek(0)
        return header

    def readPOEntry(self, f, needContext):
        context = None
        id = None
        str = None

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
                f.write(dataID[0].replace('""', '\\"'))     # msgctxt
                f.write('"\r\n')
                f.write('msgid "')
                f.write(dataID[1].replace('""', '\\"'))     # msgid
                f.write('"\r\n')
                f.write('msgstr "')
                f.write(data[dataID].replace('""', '\\"'))  # msgstr
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
