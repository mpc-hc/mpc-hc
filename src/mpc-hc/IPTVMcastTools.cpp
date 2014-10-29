/*
* (C) 2014 see Authors.txt
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

#include "stdafx.h"
#include <afxmt.h>
#include "IPTVMcastTools.h"
#include <winsock2.h>
#include <Ws2tcpip.h>

#define _BSD_SOURCE

typedef int socklen_t;

#define UDP_TX_BUF_SIZE 32768
#define UDP_MAX_PKT_SIZE 65536



CIPTVMcastTools::CIPTVMcastTools()
    : m_udp_fd(INVALID_SOCKET)
    , m_socket(INVALID_SOCKET)
    , m_is_multicast(false)

{
}

UINT32 CIPTVMcastTools::strIPAddress2UINT32(LPCTSTR pstrIP)
{
    CT2CA pszConvertedAnsiString(pstrIP);
    char* sIPAddr = pszConvertedAnsiString;
    UINT32 uIP_addr = inet_addr(sIPAddr);
    return uIP_addr;
}

USHORT CIPTVMcastTools::strPort2USHORT(LPCTSTR pstrPort)
{
    CT2CA pszConvertedAnsiString2(pstrPort);
    char* str = pszConvertedAnsiString2;
    USHORT uPort = htons((USHORT)atoi(str));
    return uPort;
}

SOCKET CIPTVMcastTools::UDPSocketCreate(UINT32 local_addr, USHORT port)
{
    int iRet = -1;
    SOCKET udp_fd = INVALID_SOCKET;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &m_wsa) == 0) {
        // Create socket
        udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (udp_fd != INVALID_SOCKET) {

            ZeroMemory(&m_sockaddr, sizeof(m_sockaddr));
            m_sockaddr.sin_family = AF_INET;
            m_sockaddr.sin_port = port;
            m_sockaddr.sin_addr.s_addr = local_addr;

            int reuse = 1;
            iRet = setsockopt(udp_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

            if (iRet != SOCKET_ERROR) {
                // bind the socket
                iRet = bind(udp_fd, (SOCKADDR*)&m_sockaddr, sizeof(m_sockaddr));
                if (iRet == SOCKET_ERROR) {
                    HRESULT hr = WSAGetLastError();
                    TRACE(_T("Error when binding socket: %u \n"), hr);
                }
            }
        }
    }

    if (iRet == SOCKET_ERROR) {
        // Something went wrong
        if (udp_fd != INVALID_SOCKET) {
            closesocket(udp_fd);
            ASSERT(FALSE);
        }
        WSACleanup();
        udp_fd = INVALID_SOCKET;
    }
    return udp_fd;
}

void CIPTVMcastTools::UDPSocketClose()
{
    closesocket(m_udp_fd);
    WSACleanup();
    m_udp_fd = INVALID_SOCKET;
    m_socket = INVALID_SOCKET;
    m_is_multicast = false;
}


CString CIPTVMcastTools::GetIPv4Addr(LPCTSTR pstrAddr)
{
    CString sIPAddr = _T("0.0.0.0");

    int iPos0 = ((CString)pstrAddr).Find(_T("@"));
    int iBase = 1;
    if (iPos0 < 0) {
        iPos0 = ((CString)pstrAddr).Find(_T("://"));
        iBase = 3;
    }
    if (iPos0 >= 0) {
        int iPos1 = ((CString)pstrAddr).ReverseFind(_T(':'));
        if (iPos1 < 0) {
            iPos1 = ((CString)pstrAddr).GetLength() - 1;
        }
        sIPAddr = ((CString)pstrAddr).Mid(iBase + iPos0, iPos1 - iPos0 - iBase);
    } else {
        ASSERT(FALSE);
    }
    return sIPAddr;
}

USHORT CIPTVMcastTools::GetIPv4Port(LPCTSTR pstrAddr)
{
    int iPos = ((CString)pstrAddr).ReverseFind(_T(':'));
    CString sPort = ((CString)pstrAddr).Right(((CString)pstrAddr).GetLength() - iPos - 1);
    USHORT uPort = strPort2USHORT(sPort);

    return uPort;
}

bool CIPTVMcastTools::IsMulticastAddress(UINT32 uIPAddr)
{
    USHORT uIPAddrNBO = ntohs((u_short) uIPAddr);
    return ((uIPAddrNBO >= MULTICAST_FROM) && (uIPAddrNBO <= MULTICAST_TO));
}


int CIPTVMcastTools::UDPMulticastJoinGroup(UINT32 ip_addr, USHORT port)
{
    int iResult = 0;

    // checks whether the address is within the range of addresses reserved for multicasting
    m_is_multicast = IsMulticastAddress(ip_addr);

    if (m_is_multicast) {
        m_imr.imr_interface.s_addr = htonl(INADDR_ANY);
        m_imr.imr_multiaddr.s_addr = ip_addr;

        m_udp_fd = UDPSocketCreate(htonl(INADDR_ANY), port);
        if (m_udp_fd == INVALID_SOCKET) {
            iResult = -1;
        } else {
            iResult = JoinSourceGroup(m_udp_fd, m_imr.imr_multiaddr.s_addr, m_imr.imr_interface.s_addr);

            if (iResult != 0) {
                HRESULT hr = WSAGetLastError();
                TRACE(_T("Error when trying to join multicast group: %u \n"), hr);
                WSACleanup();
                ASSERT(FALSE);
            }
        }
    }

    return iResult;
}

int CIPTVMcastTools::UDPMulticastJoinGroup(LPCTSTR pstrAddr)
{
    UINT32 uGrpAddr = strIPAddress2UINT32(GetIPv4Addr(pstrAddr));

    USHORT uPort = GetIPv4Port(pstrAddr);
    int iResult = UDPMulticastJoinGroup(uGrpAddr, uPort);

    return iResult;
}

void CIPTVMcastTools::UDPMulticastLeaveGroup()
{
    if (m_is_multicast && m_udp_fd > 0) {
        LeaveSourceGroup(m_udp_fd, m_imr.imr_multiaddr.s_addr, m_imr.imr_interface.s_addr);
        UDPSocketClose();
    }
}

int CIPTVMcastTools::JoinSourceGroup(SOCKET sd, UINT32 grpaddr, UINT32 iaddr)
{
    struct ip_mreq imr;

    imr.imr_multiaddr.s_addr = grpaddr;
    imr.imr_interface.s_addr = iaddr;
    return setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&imr, sizeof(imr));
}


int CIPTVMcastTools::LeaveSourceGroup(SOCKET sd, UINT32 grpaddr, UINT32 iaddr)
{
    struct ip_mreq_source imr;

    imr.imr_multiaddr.s_addr = grpaddr;
    imr.imr_interface.s_addr = iaddr;
    return setsockopt(sd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&imr, sizeof(imr));
}
