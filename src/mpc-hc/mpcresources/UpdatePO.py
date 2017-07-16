# (C) 2013, 2015 see Authors.txt
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

from TranslationDataRC import *


def UpdatePO(filename):
    translationDataOld = TranslationDataRC()
    translationDataOld.loadFromPO('PO\\' + filename, 'po')

    translationData = TranslationDataRC()
    translationData.loadFromPO(r'PO\mpc-hc', 'pot')
    translationData.translate(translationDataOld)
    translationData.writePO('PO\\' + filename, 'po')

if __name__ == '__main__':
    if len(sys.argv) != 2:
        RuntimeError('Invalid number of parameters. Usage: UpdatePO.py <filename>')

    UpdatePO(sys.argv[1])
