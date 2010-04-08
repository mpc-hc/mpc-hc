/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: dirac_assertions.h,v 1.3 2004/11/22 14:05:02 asuraparaju Exp $ $Name:  $
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
#ifndef DIRAC_ASSERTIONS_H
#define DIRAC_ASSERTIONS_H
namespace dirac
{

#undef cmpCOND
#define cmpCOND( exp, trueRes, falseRes )    ( (exp) ? (trueRes) : (falseRes) )

#undef ERREXP
#define    ERREXP(exp,errfn,text)    cmpCOND((exp), ((void)0), errfn(__FILE__,__LINE__,text))

#undef ASSERT
#define ASSERT(exp)              ERREXP(exp,dirac_assert,NULL)

#undef ASSERTM
#define ASSERTM(exp,text)        ERREXP(exp,dirac_assert,text)

#undef TEST
#undef TESTM
#undef REPORT
#undef REPORTM

#ifdef DIRAC_DEBUG
#define TEST(exp)               ASSERT(exp)
#define TESTM(exp,text)         ASSERTM(exp,text)
#define REPORT(exp)             ASSERT(exp)
#define REPORTM(exp,text)       ASSERTM(exp,text)
#else
#define TEST(exp)
#define TESTM(exp,text)
#define REPORT(exp)             ERREXP(exp,dirac_report,NULL)
#define REPORTM(exp,text)       ERREXP(exp,dirac_report,text)
#endif


/*! Print a message to standard error and abort if in debug mode */
void dirac_assert(const char *p_fname, int line_number, const char *p_mess);

/*! Print a message to standard error */
void dirac_report(const char *p_fname, int line_number, const char *p_mess);

} // namespace dirac

#endif
