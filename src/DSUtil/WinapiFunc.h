/*
 * (C) 2014, 2016 see Authors.txt
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

template <typename>
class WinapiFunc;

template <typename ReturnType, typename...Args>
struct WinapiFunc<ReturnType WINAPI(Args...)> final {
    typedef ReturnType(WINAPI* WinapiFuncType)(Args...);

    WinapiFunc() = delete;
    WinapiFunc(const WinapiFunc&) = delete;
    WinapiFunc& operator=(const WinapiFunc&) = delete;

    inline WinapiFunc(LPCTSTR dll, LPCSTR func)
        : m_hLib(LoadLibrary(dll))
        , m_pWinapiFunc(reinterpret_cast<WinapiFuncType>(GetProcAddress(m_hLib, func))) {
    }

    inline ~WinapiFunc() {
        FreeLibrary(m_hLib);
    }

    inline explicit operator bool() const {
        return !!m_pWinapiFunc;
    }

    inline ReturnType operator()(Args...args) const {
        return m_pWinapiFunc(args...);
    }

private:
    const HMODULE m_hLib;
    const WinapiFuncType m_pWinapiFunc;
};
