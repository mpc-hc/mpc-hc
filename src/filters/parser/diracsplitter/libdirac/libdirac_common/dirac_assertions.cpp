/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: dirac_assertions.cpp 280 2005-01-30 05:11:46Z gabest $ $Name$
*
* Version: MPL 1.1/GPL 2.0/LGPL 2.1
*
* The contents of this file are subject to the Mozilla Public License
* Version 1.1 (the "License"); you may not use this file except in compliance
* with the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
* the specific language governing rights and limitations under the License.
*
* The Original Code is BBC Research and Development code.
*
* The Initial Developer of the Original Code is the British Broadcasting
* Corporation.
* Portions created by the Initial Developer are Copyright (C) 2004.
* All Rights Reserved.
*
* Contributor(s): Anuradha Suraparaju (Original Author)
*
* Alternatively, the contents of this file may be used under the terms of
* the GNU General Public License Version 2 (the "GPL"), or the GNU Lesser
* Public License Version 2.1 (the "LGPL"), in which case the provisions of
* the GPL or the LGPL are applicable instead of those above. If you wish to
* allow use of your version of this file only under the terms of the either
* the GPL or LGPL and not to allow others to use your version of this file
* under the MPL, indicate your decision by deleting the provisions above
* and replace them with the notice and other provisions required by the GPL
* or LGPL. If you do not delete the provisions above, a recipient may use
* your version of this file under the terms of any one of the MPL, the GPL
* or the LGPL.
* ***** END LICENSE BLOCK ***** */

#include <stdlib.h>
#include <string>
#include <iostream>

#include "dirac_assertions.h"
using namespace dirac;

namespace dirac
{
void dirac_assert( const char *p_fname, int line_number, const char *p_mess )
{
    dirac_report( p_fname, line_number, p_mess );
    // dump core
    abort ();
}

void dirac_report( const char *p_fname, int line_number, const char *p_mess )
{
    std::string errMess("Assertion ");
    
    if ( p_mess )
        errMess = errMess +  "^ " + std::string(p_mess) + " ^" +" failed";
    else
        errMess += " failure";

    std::cerr << errMess << " in file " << p_fname << " at line " << line_number << std::endl;
}
} // namespace dirac

