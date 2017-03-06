# (C) 2015-2016 see Authors.txt
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
import os
import fnmatch
import traceback

from multiprocessing import Pool
from UpdateISPOT import *
from UpdateISPO import *
from UpdateIS import *


def processPO(file):
    ret = 'Updating PO file ' + file + '\n'
    result = True
    try:
        UpdateISPO(file)
    except Exception:
        ret += ''.join(traceback.format_exception(*sys.exc_info()))
        result = False

    ret += '----------------------'
    return result, ret

if __name__ == '__main__':
    print 'Updating POT file'
    UpdateISPOT()
    print '----------------------'

    pool = Pool()
    results = []
    for file in os.listdir('PO'):
        if fnmatch.fnmatch(file, 'mpc-hc.installer.*.strings.po'):
            results.append(pool.apply_async(processPO, [os.path.splitext(file)[0]]))

    pool.close()

    for result in results:
        ret = result.get(15)
        print ret[1]
        if (not ret[0]):
            os.system('pause')

    print 'Updating IS file'
    UpdateIS(False)
    print '----------------------'
