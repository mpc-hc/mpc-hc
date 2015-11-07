/*
 * (C) 2015 see Authors.txt
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

#pragma once

#include <mutex>
#include <sys/timeb.h>
#include "AppSettings.h"
#include "mplayerc.h"

enum class LogTargets {
    BDA
};

template <LogTargets>
struct LoggerFile;

template<>
struct LoggerFile<LogTargets::BDA> {
protected:
    const LPCTSTR filename = _T("bda.log");
    virtual ~LoggerFile() = default;
};

template<LogTargets TARGET>
class Logger final : public LoggerFile<TARGET>
{
public:
    static void LOG(LPCTSTR fmt...) {
        static Logger logger;

        va_list args;
        va_start(args, fmt);
        logger.WriteToFile(fmt, args);
        va_end(args);
    };

private:
    const bool m_bLogging = AfxGetAppSettings().bEnableLogging;
    std::mutex m_mutex;

    Logger() {
        // Check if logging is enabled only during initialization
        // to avoid incomplete logs
        ASSERT(AfxGetAppSettings().IsInitialized());
    }

    void WriteToFile(LPCTSTR fmt, va_list& args) {
        if (!m_bLogging) {
            return;
        }

        TCHAR buff[3000];
        FILE* f;
        _timeb timebuffer;
        TCHAR time1[8];
        TCHAR wbuf[26];

        _ftime_s(&timebuffer);
        _tctime_s(wbuf, _countof(wbuf), &timebuffer.time);

        for (size_t i = 0; i < _countof(time1); i++) {
            time1[i] = wbuf[i + 11];
        }

        _vstprintf_s(buff, _countof(buff), fmt, args);

        std::lock_guard<std::mutex> lock(m_mutex);
        if (_tfopen_s(&f, filename, _T("at")) == 0) {
            _ftprintf_s(f, _T("%.8s.%03hu - %s\n"), time1, timebuffer.millitm, buff);
            fclose(f);
        }
    }
};
