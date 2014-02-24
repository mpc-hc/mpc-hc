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

import sys
import glob

from TranslationDataIS import *

if __name__ == '__main__':
    translationsConfigAndData = []
    for cfgPath in glob.glob(r'cfg\*.cfg'):
        config = ConfigParser.RawConfigParser({'installerIsTranslated': 'True'})
        config.readfp(codecs.open(cfgPath, 'r', 'utf8'))

        if config.getboolean('Info', 'installerIsTranslated'):
            poPath = r'PO\mpc-hc.installer.' + config.get('Info', 'langShortName')
            translationData = TranslationDataIS()
            translationData.loadFromPO(poPath, 'po', (False, False, True))
            # Write back the PO file to ensure it's properly normalized
            translationData.writePO(poPath, 'po', (False, False, True))

            translationsConfigAndData.append((config, translationData))

    TranslationDataIS.translateIS(translationsConfigAndData, r'..\..\..\distrib\custom_messages.iss', r'..\..\..\distrib\custom_messages_translated.iss')
