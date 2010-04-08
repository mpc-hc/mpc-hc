/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: dirac_exception.h,v 1.6 2008/01/31 11:25:16 tjdwave Exp $ $Name:  $
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
* Contributor(s): Andrew Kennedy (Original Author)
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

/**
 * Definition of class DiracException.
 */
#ifndef DiracException_h
#define DiracException_h


// SYSTEM INCLUDES
//
#include <string>                           // has a string
#include <iostream>                          // has an ostringstream


namespace dirac
{

/**
 * Enumeration of Dirac-defined error codes.
 *
 *
 */
enum DiracErrorCode
{
    ERR_UNSUPPORTED_STREAM_DATA = 0,
    ERR_END_OF_STREAM,
    ERR_INVALID_VIDEO_FORMAT,
    ERR_INVALID_CHROMA_FORMAT,
    ERR_INVALID_PICTURE_RATE,
    ERR_INVALID_SIGNAL_RANGE,
    ERR_INVALID_PIXEL_ASPECT_RATIO,
    ERR_INVALID_VIDEO_DEPTH,
    ERR_INVALID_MOTION_VECTOR_PRECISION,
    ERR_INVALID_INIT_DATA

};

/**
* Error-severity states
*/
enum DiracSeverityCode
{
    SEVERITY_NO_ERROR = 0,
    SEVERITY_WARNING,
    SEVERITY_PICTURE_ERROR,
    SEVERITY_ACCESSUNIT_ERROR,
    SEVERITY_SEQUENCE_ERROR,
    SEVERITY_TERMINATE
};

/**
 * DiracException is the class which should be used for all exceptions
 * within Dirac.
 */
class DiracException
{
public:


    /**
     * Construct from error source ID, error code, and message.
     *
     * @param errorCode    The error code.
     * @param errorMessage The error message.
     * @param severityCode The error source ID.
     */
    DiracException(
        const DiracErrorCode& errorCode,
        const std::string& errorMessage,
        const DiracSeverityCode& severityCode);

    /**
     * Copy constructor.
     */
    DiracException(
        const DiracException& src);

    /**
     * Destructor.
     */
    virtual ~DiracException();


// OPERATORS


// ACCESS

    /**
     * Get the error code of this exception.
     *
     * @return The error code of this exception.
     */
    DiracErrorCode GetErrorCode() const;

    /**
     * Get the severity level of this exception.
     *
     * @return The severity level of this exception.
     */
    DiracSeverityCode GetSeverityCode() const;

    /**
     * Get the error message of this exception.
     *
     * @return The error message of this exception.
     */
    std::string GetErrorMessage() const;



private:

// ATTRIBUTES

    /**
     * The error code of this exception.
     */
    DiracErrorCode mErrorCode;

    /**
     * Severity of exception
     */
    DiracSeverityCode mSeverityCode;

    /**
     * The error message.
     */
    std::string mErrorMessage;



// UNIMPLEMENTED METHODS
    DiracException& operator=(const DiracException&);

}; // class DiracException





// GLOBAL OPERATORS
//
std::ostream& operator<<(std::ostream& dst, const DiracException& exception);


// MACROS FOR LOGGING AND THROWING DIRAC EXCEPTIONS
//


/**
 * Write an exception to the log
 */
#define DIRAC_LOG_EXCEPTION(exception)                           \
    {                                                                   \
       if(exception.GetSeverityCode()!=SEVERITY_NO_ERROR)       \
            std::cerr << exception.GetErrorMessage();          \
    }

/**
 * Construct an exception from 3 arguments, log it, and throw it.
 */
#define DIRAC_THROW_EXCEPTION(arg1,arg2,arg3)                          \
    {                                                                   \
        DiracException exception(arg1,arg2, arg3);      \
        DIRAC_LOG_EXCEPTION(exception)  \
        throw exception;                        \
    }

/**
 * Catch a DiracException, log it, and rethrow it.
 */
#define DIRAC_CATCH_AND_RETHROW()                                 \
    catch (const DiracException& e) {                                     \
        DiracException exception(e);           \
        DIRAC_LOG_EXCEPTION(exception)\
        throw exception;                     \
    }

} // namespace Dirac

#endif // DiracException_h
