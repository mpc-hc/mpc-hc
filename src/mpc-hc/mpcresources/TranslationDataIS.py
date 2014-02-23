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


class TranslationDataIS(TranslationData):
    sectionEntry = re.compile(ur'^\[(.*)\]\r\n', re.UNICODE)
    stringEntry = re.compile(ur'^en\.([^=]+)=(.+)\r\n', re.UNICODE)

    def loadFromIS(self, filename):
        self.empty()

        with codecs.open(filename, 'r', detectEncoding(filename)) as f:
            section = None
            for line in f:
                match = TranslationDataIS.stringEntry.match(line)
                if section and match:
                    if match.group(1) != u'langid':
                        self.strings[(section + '_' + match.group(1), match.group(2).replace(u'%n', ur'\n'))] = ''
                else:
                    match = TranslationDataIS.sectionEntry.match(line)
                    if match:
                        section = match.group(1)

    @staticmethod
    def translateIS(translationsConfigAndData, filenameBase, filenameRC):
        encoding = detectEncoding(filenameBase)
        with codecs.open(filenameBase, 'r', encoding) as fBase, \
                codecs.open(filenameRC, 'w', encoding) as fOut:
            section = None
            sectionData = []

            for line in fBase:
                match = TranslationDataIS.sectionEntry.match(line)
                if match or line == u'#if localize == "true"\r\n':
                    if section:
                        # Remove empty line at the end of a section
                        end = len(sectionData)
                        while end > 0 and sectionData[end - 1] == u'\r\n':
                            end -= 1
                        sectionData = sectionData[:end]

                        fOut.write(u'[' + section + u']')
                        for config, translationData in translationsConfigAndData:
                            fOut.write(u'\r\n')
                            translationData.translateISSection(config, section, sectionData, fOut)

                    if match:
                        if section:
                            fOut.write(u'\r\n\r\n')
                        section = match.group(1)
                        sectionData = []
                    else:
                        break
                elif section:
                    sectionData.append(line)
                else:
                    fOut.write(line)

    def translateISSection(self, config, section, sectionData, fOut):
        for line in sectionData:
            match = TranslationDataIS.stringEntry.match(line)
            if match:
                if match.group(1) != u'langid':
                    s = self.strings.get((section + '_' + match.group(1), match.group(2).replace(u'%n', ur'\n')))
                    if not s:
                        s = match.group(2)
                    line = '%s.%s=%s\r\n' % (config.get('Info', 'langShortName'), match.group(1), s.replace(ur'\n', u'%n'))
                else:
                    line = '%s.langid=%0.8d\r\n' % (config.get('Info', 'langShortName'), config.getint('Info', 'langId'))

                fOut.write(line)
            elif line == u'; English\r\n':
                fOut.write(u'; ' + config.get('Info', 'langName') + u'\r\n')
            elif line != u'WelcomeLabel1=[name/ver]\r\n':
                fOut.write(line)
