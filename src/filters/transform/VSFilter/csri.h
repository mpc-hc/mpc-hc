/*****************************************************************************
 * csri: common subtitle renderer interface
 *****************************************************************************
 * Copyright (C) 2007  David Lamparter
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  - The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 ****************************************************************************/

/** \file csri.h - main CSRI (common subtitle renderer interface) include. */

#ifndef _CSRI_H
/** \cond */
#define _CSRI_H 20070119
/** \endcond */

#include <stddef.h>         /* ptrdiff_t */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CSRIAPI
/** CSRI API attributes.
 *  defaults to \c extern.
*/
#define CSRIAPI extern
#endif

/** \defgroup base CSRI base API. */
/*@{*/

/** pixel format specification for frames */
enum csri_pixfmt {
    CSRI_F_RGBA = 0,
    CSRI_F_ARGB,
    CSRI_F_BGRA,
    CSRI_F_ABGR,

    CSRI_F_RGB_ = 0x100,
    CSRI_F__RGB,
    CSRI_F_BGR_,            /**< Windows "RGB32" */
    CSRI_F__BGR,

    CSRI_F_RGB  = 0x200,
    CSRI_F_BGR,         /**< Windows "RGB24" */

    CSRI_F_AYUV = 0x1000,
    CSRI_F_YUVA,
    CSRI_F_YVUA,

    CSRI_F_YUY2 = 0x1100,

    CSRI_F_YV12A = 0x2011,      /**< planar YUV 2x2 + alpha plane */
    CSRI_F_YV12 = 0x2111        /**< planar YUV 2x2 */
};

#define csri_is_rgb(x) ((x) < 0x1000)
#define csri_is_yuv(x) ((x) >= 0x1000)
#define csri_is_yuv_planar(x) ((x) >= 0x2000)
#define csri_get_yuv_planar_xred(x) (0xf & (x))
#define csri_get_yuv_planar_yred(x) (0xf & ((x) >> 4))
#define csri_is_yuv_packed(x) ((x) >= 0x1000 && (x) < 0x2000)
#define csri_has_alpha(x) (((x) & 0xfff) < 0x100)

/** frame/image format specification pre-fed to the renderer */
struct csri_fmt {
    /** format to be used */
    enum csri_pixfmt pixfmt;
    /** image width, full frame.
     *
     * This should specify the full size of the frame.
     * Specifying the video sub-size (in case of added black
     * borders) is left to an extension.
     */
    unsigned width;
    /** image height */
    unsigned height;
};

/** single frame to be fed to the renderer. */
struct csri_frame {
    /** frame format.
     * It is an application bug if this differs from the one
     * passed in struct #csri_fmt to csri_query_fmt()
     */
    enum csri_pixfmt pixfmt;
    /** the frame's data.
     * Packed formats only use planes[0]; planar formats
     * have the data ordered as Y, U, V[, A].
     *
     * Also note that the topmost line always comes first.
     * The Windows biHeight strange-ity is \a NOT duplicated.
     */
    unsigned char* planes[4];
    /** strides for the individual planes.
     * Stride means full byte offset, i.e. do \a not add
     * frame width
     */
    ptrdiff_t strides[4];
};

#ifndef CSRI_OWN_HANDLES
/** opaque renderer data */
typedef void csri_rend;
/** opaque instance data */
typedef void csri_inst;
#endif

#ifdef DOXYGEN
/** disable the emission of the csri_rend and csri_inst typedefs.
 *  define this if you are in a renderer and are typedef'ing
 *  csri_rend and csri_inst to your own structs.
*/
#define CSRI_OWN_HANDLES
#endif

/** renderer description.
 * \ingroup loader
 */
struct csri_info {
    /** an identifier for the renderer.
     * - MUST match the regular expression
     *   \code ^[a-zA-Z]([a-zA-Z0-9_]*[a-zA-Z0-9])? \endcode
     *   i.e. consists only of letters, numbers and underscores;
     *   must start with a letter and doesnt have an underscore
     *   as the last character.
     */
    const char* name;
    /** an identifier to the exact version of the renderer.
     * most likely a version number or revision identifier.
     *
     * The helper library does a strcmp over this field in order
     * to order multiple instances of the same renderer. Use
     * higher-byte-value strings for newer renderers.
     */
    const char* specific;

    /** a nice name to be presented to the user */
    const char* longname;
    /** the renderer's author */
    const char* author;
    /** a copyright string. Copyright (c) 2042 by Mr. Nice Guy */
    const char* copyright;
};

/** data of extension-dependent type.
 * The field to be used MUST be specified in the extension where it is used.
 */
union csri_vardata {
    long lval;
    double dval;
    const char* utf8val;
    void* otherval;
};

/** extension identifier.
 * This follows reverse DNS syntax, i.e.:
 * \code root.branch.leaf \endcode
 * you can either reverse a registered domain name, e.g.
 * \code com.microsoft.csri.usegdiplus \endcode
 * or ask the CSRI maintainers to assign a namespace to you.
 *
 * currently registered namespaces are:
 *
 * \code
 * csri.* - official extensions
 * asa.*  - custom extensions of the asa renderer
 * \endcode
 */
typedef const char* csri_ext_id;

/** script loading parameters.
 * each flag MUST have an associated extension, which can be queried
 * with csri_query_ext(). If the open flag constitutes an extension on its
 * sole own, csri_query_ext() can return a meaningless non-NULL value for
 * it.
 *
 * The data field used must be specified.
 *
 * An extension can have multiple flags. In that case, the flags should have
 * the extension name as common prefix, separated with a dot.
 *
 * A renderer MUST ignore unknown open flags. It MUST NOT return an error
 * just because it does not support a particular flag.
 */
struct csri_openflag {
    /** flag name */
    csri_ext_id name;
    /** flag data argument */
    union csri_vardata data;
    /** link to next flag */
    struct csri_openflag* next;
};

/** load a script from a file.
 * \param renderer the handle to the renderer
 * \param filename the path to the file to be loaded. \n
 *   The filename should be encoded as UTF-8. Windows renderers are
 *   expected to convert it to UTF-16 and use the Unicode Windows API
 *   functions.
 * \param flags a linked list of open flags. \n
 *   The caller manages memory allocation, i.e. static allocation is OK.
 * \return the renderer instance handle, or NULL on error.
 */
CSRIAPI csri_inst* csri_open_file(csri_rend* renderer,
                                  const char* filename, struct csri_openflag* flags);

/** load a script from memory.
 * \param renderer the handle to the renderer
 * \param data pointer to the first data byte. \n
 *   The caller manages memory allocation and should free the data after
 *   calling csri_open_mem(). If the renderer needs to keep the data, it
 *   must copy it. \n
 *   The renderer is not allowed to write to the data.
 * \param length length, in bytes, of the data
 * \param flags see csri_open_file()
 * \return the render instance handle, or NULL on error.
 */

CSRIAPI csri_inst* csri_open_mem(csri_rend* renderer,
                                 const void* data, size_t length, struct csri_openflag* flags);

/** close a renderer instance.
 * \param inst the instance handle.
 */
CSRIAPI void csri_close(csri_inst* inst);


/** query / set the image format and size.
 * \param inst the renderer instance handle
 * \param fmt the format and image size to be used
 * \return 0 if the format was successfully set,
 *   any other value in case of error.
 */
CSRIAPI int csri_request_fmt(csri_inst* inst, const struct csri_fmt* fmt);

/** render a single frame
 * \param inst the renderer instance handle
 * \param frame frame data to render to
 * \param time associated timestamp of the frame
 */
CSRIAPI void csri_render(csri_inst* inst, struct csri_frame* frame,
                         double time);


/** query for an extension.
 * \param rend the renderer handle
 * \param extname the extension's identifier
 * \return NULL if the extension is not supported,
 *   a pointer to extension-specific data otherwise
 *
 * The data pointed to by the return value does not neccessarily need to
 * have any meaning; An extension that does not need to return data
 * can return a pointer to whatever it wants, as long as that pointer is
 * not NULL.
 *
 * In the usual case, the pointer is supposed to point to a struct with
 * function pointers and other information as needed.
 */
CSRIAPI void* csri_query_ext(csri_rend* rend, csri_ext_id extname);

/*@}*/

/** \defgroup loader CSRI loader API.
 *
 * These functions locate renderers based on given parameters.
 *
 * <b>Renderers must implement these functions as well.</b>
 *
 * They are used by the library to grab renderer information
 * from a shared object; and also this way a single renderer
 * can be linked directly into an appliaction.
 */
/*@{*/

/** get renderer information
 * \param rend the renderer handle
 * \return information about the renderer, or NULL in case the renderer
 *   encountered an internal error.
 */
CSRIAPI struct csri_info* csri_renderer_info(csri_rend* rend);

/** try to load a given renderer
 * \param name the name of the renderer, as in csri_info.name
 * \param specific the specific version of the renderer,
 *   as in csri_info.specific;\n
 *   alternatively NULL if any version of the renderer is ok.
 * \return a handle to the renderer if it was successfully loaded,
 *   NULL otherwise.
 */
CSRIAPI csri_rend* csri_renderer_byname(const char* name,
                                        const char* specific);

/** try to find an implementation of the given extensions.
 * \param next number of extensions pointed to by ext
 * \param ext array of extensions to search for
 * \return a handle to a renderer supporting ALL of the
 *   extensions, NULL if none was found.
 */
CSRIAPI csri_rend* csri_renderer_byext(unsigned n_ext, csri_ext_id* ext);

/** get the default (highest priority) renderer
 * \return a handle to the default renderer, or NULL if
 *   no renderer is installed.
 *
 * Together with csri_renderer_next(), this can be used
 * to enumerate all installed renderers.
 */
CSRIAPI csri_rend* csri_renderer_default();

/** get the next lower priority renderer
 * \param prev the current renderer
 * \return the renderer with the next lower priority than
 *   the one named in prev, or NULL if prev is the last
 *   renderer installed.
 */
CSRIAPI csri_rend* csri_renderer_next(csri_rend* prev);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* _CSRI_H */
