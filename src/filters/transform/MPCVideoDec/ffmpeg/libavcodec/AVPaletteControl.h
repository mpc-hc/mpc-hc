/*
 * copyright (c) 2001 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef AVCODEC_AVPALETTECONTROL_H
#define AVCODEC_AVPALETTECONTROL_H

/**
 * @file AVPaletteControl.h
 * This in a part of avcodec.h in the original project
 */

/**
 * AVPaletteControl
 * This structure defines a method for communicating palette changes
 * between and demuxer and a decoder.
 *
 * @deprecated Use AVPacket to send palette changes instead.
 * This is totally broken.
 */
#define AVPALETTE_SIZE 1024
#define AVPALETTE_COUNT 256
typedef struct AVPaletteControl {

    /* Demuxer sets this to 1 to indicate the palette has changed;
     * decoder resets to 0. */
    int palette_changed;

    /* 4-byte ARGB palette entries, stored in native byte order; note that
     * the individual palette components should be on a 8-bit scale; if
     * the palette data comes from an IBM VGA native format, the component
     * data is probably 6 bits in size and needs to be scaled. */
    unsigned int palette[AVPALETTE_COUNT];

} AVPaletteControl /* attribute_deprecated */;

#endif
