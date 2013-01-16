/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// VFR translation functions for OverLua

#include "stdafx.h"
#include "vfr.h"
#include <stdio.h>
#include <string.h>
#include <vector>


// Work with seconds per frame (spf) here instead of fps since that's more natural for the translation we're doing

class TimecodesV1 : public VFRTranslator
{
private:
    // Used when sections run out
    double default_spf;
    double first_non_section_timestamp;
    int first_non_section_frame;

    // Also generate sections for unspecified ones
    // (use the default framerate then)
    struct FrameRateSection {
        double start_time;
        double spf;
        int start_frame;
        int end_frame;
    };
    std::vector<FrameRateSection> sections;

public:
    virtual double TimeStampFromFrameNumber(int n) {
        // Find correct section
        for (size_t i = 0; i < sections.size(); i++) {
            FrameRateSection& sect = sections[i];
            if (n >= sect.start_frame && n <= sect.end_frame) {
                return sect.start_time + (n - sect.start_frame) * sect.spf;
            }
        }
        // Not in a section
        if (n < 0) {
            return 0.0;
        }
        return first_non_section_timestamp + (n - first_non_section_frame) * default_spf;
    }

    TimecodesV1(FILE* vfrfile) {
        char buf[100];

        default_spf = -1;
        double cur_time = 0.0;

        FrameRateSection temp_section;
        temp_section.start_time = 0.0;
        temp_section.spf = -1;
        temp_section.start_frame = 0;
        temp_section.end_frame = 0;

        while (fgets(buf, 100, vfrfile)) {
            // Comment?
            if (buf[0] == '#') {
                continue;
            }

            if (_strnicmp(buf, "Assume ", 7) == 0 && default_spf < 0) {
                char* num = buf + 7;
                default_spf = atof(num);
                if (default_spf > 0) {
                    default_spf = 1 / default_spf;
                } else {
                    default_spf = -1;
                }
                temp_section.spf = default_spf;
                continue;
            }

            int start_frame, end_frame;
            float fps;
            if (sscanf_s(buf, "%d,%d,%f", &start_frame, &end_frame, &fps) == 3) {
                // Finish the current temp section
                temp_section.end_frame = start_frame - 1;
                if (temp_section.end_frame >= temp_section.start_frame) {
                    cur_time += (temp_section.end_frame - temp_section.start_frame + 1) * temp_section.spf;
                    sections.push_back(temp_section);
                }
                // Insert the section corresponding to this line
                temp_section.spf = 1 / fps;
                temp_section.start_frame = start_frame;
                temp_section.end_frame = end_frame;
                temp_section.start_time = cur_time;
                cur_time += (end_frame - start_frame + 1) / fps;
                sections.push_back(temp_section);
                // Begin new temp section
                temp_section.spf = default_spf;
                temp_section.start_frame = end_frame + 1;
                temp_section.end_frame = end_frame; // yes, negative duration
                temp_section.start_time = cur_time;
            }
        }

        first_non_section_timestamp = cur_time;
        first_non_section_frame = temp_section.start_frame;
    }

};

class TimecodesV2 : public VFRTranslator
{
private:
    // Main data
    std::vector<double> timestamps;
    // For when data are exhausted (well, they shouldn't, then the vfr file is bad)
    int last_known_frame;
    double last_known_timestamp;
    double assumed_spf;

public:
    virtual double TimeStampFromFrameNumber(int n) {
        if (n < (int)timestamps.size() && n >= 0) {
            return timestamps[n];
        }
        if (n < 0) {
            return 0.0;
        }
        return last_known_timestamp + (n - last_known_frame) * assumed_spf;
    }

    TimecodesV2(FILE* vfrfile) {
        char buf[50];

        timestamps.reserve(8192); // should be enough for most cases

        while (fgets(buf, _countof(buf), vfrfile)) {
            // Comment?
            if (buf[0] == '#') {
                continue;
            }
            // Otherwise assume it's a good timestamp
            timestamps.push_back(atof(buf) / 1000);
        }

        last_known_frame = (int)timestamps.size() - 1;
        last_known_timestamp = timestamps[last_known_frame];
        assumed_spf = last_known_timestamp - timestamps[last_known_frame - 1];
    }

};

VFRTranslator* GetVFRTranslator(const char* vfrfile)
{
    char buf[32];
    buf[19] = 0; // In "# timecode format v1" the version number is character index 19
    FILE* f;
    fopen_s(&f, vfrfile, "r"); // errno_t err = <-- TODO: Check if fopen_s fails
    VFRTranslator* res = 0;
    if (fgets(buf, _countof(buf), f) && buf[0] == '#') {
        // So do some really shoddy parsing here, assume the file is good
        if (buf[19] == '1') {
            res = DEBUG_NEW TimecodesV1(f);
        } else if (buf[19] == '2') {
            res = DEBUG_NEW TimecodesV2(f);
        }
    }
    fclose(f);
    return res;
}
