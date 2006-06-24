/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: cmd_line.cpp 280 2005-01-30 05:11:46Z gabest $ $Name$
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
* Contributor(s): Scott R Ladd (Original Author), Thomas Davies
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

#include "cmd_line.h"
using namespace dirac;

using namespace std;

CommandLine::CommandLine(int argc, char * argv[], const set<string> & bool_opts)
  : m_options(),
    m_inputs(),
    m_bool_opts(bool_opts)
{
    bool option_active = false;
    vector<option>::iterator active_option;

    for (int i = 1; i < argc; ++i)
    {
        // is it an option?
        if ((strlen(argv[i]) > 1) && (argv[i][0] == '-'))
        {
            // store new key
            string opt_key = string(&argv[i][1]);
            m_options.push_back(option(opt_key));
            
            // active option is now last in list
            active_option = m_options.end();
            --active_option;
            
            // check option list to see if we're looking for an argument
            option_active = (m_bool_opts.find(opt_key) == m_bool_opts.end());
        }
        else
        {
            if (option_active)
                active_option->m_value = string(argv[i]);
            else
                m_inputs.push_back(string(argv[i]));

            option_active = false;
        }
    }
}
