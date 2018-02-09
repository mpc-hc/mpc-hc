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

#pragma once

// Multicast range:
#define MULTICAST_FROM ntohs((u_short) inet_addr("224.0.0.0"))
#define MULTICAST_TO ntohs((u_short) inet_addr("239.255.255.255"))

// Special address:
#define IPTV_NULL_ADDRESS _T("rtp://0.0.0.0")

// Protocol encapsulation:
#define PROTOCOL_RTP _T("rtp://")

#include <atomic>

class CServiceProvider
{
    CServiceProvider() : SP_addr(0), SP_port(0), strDescription(_T("")) {}
    UINT32 SP_addr;
    USHORT SP_port;
    CString strDescription;
};

struct strServiceProvider {
    strServiceProvider() : DomainName(_T("")), IP_addr(_T("")), port(_T("")) {}
    CString DomainName;
    CString IP_addr;
    CString port;
};

struct ChannelID_logicalChannel {
    CString strChannelID;
    CString strLogicalChannel;
};

class CIPTVMcastTools
{
public:
    CIPTVMcastTools();

    // Membership management
    int UDPMulticastJoinGroup(LPCTSTR pstrAddr);
    int UDPMulticastJoinGroup(UINT32 ip_addr, USHORT port);
    void UDPMulticastLeaveGroup();

    // DVBSTP Services
    HRESULT GetServiceProviders(LPCTSTR pstrIP_Address, LPCTSTR pstrPort, HWND hWnd);
    void BroadcastChannelsDiscover(LPCTSTR pstrIP_Address, LPCTSTR pstrPort, HWND hWnd, std::atomic<bool> &bStopRequested);
    void ScanRangeIPs(LPCTSTR pstrIP_Addr1, LPCTSTR pstrIP_Addr2, LPCTSTR pstrPort, HWND hWnd, std::atomic<bool> &bStopRequested);
    HRESULT GetEPGInfo(LPCTSTR pstrIP_Address, LPCTSTR pstrPort, LPCTSTR pstrInfo);
    HRESULT VerifyChannel(LPCTSTR pstrAddr);
    HRESULT VerifyChannel(UINT32 remote_addr, USHORT uPort);

    strServiceProvider* m_pSP;


private:
    SOCKET m_udp_fd;
    boolean m_is_multicast;
    SOCKET m_socket;
    struct sockaddr_in m_sockaddr;
    WSADATA m_wsa;
    struct ip_mreq m_imr;

    // IP tools
    UINT32 strIPAddress2UINT32(LPCTSTR pstrIP);
    USHORT strPort2USHORT(LPCTSTR pstrPort);

    // Socket management
    SOCKET UDPSocketCreate(UINT32 local_addr, USHORT port);
    void UDPSocketClose();

    // Membership management
    int JoinSourceGroup(SOCKET sd, UINT32 grpaddr, UINT32 iaddr);
    int LeaveSourceGroup(SOCKET sd, UINT32 grpaddr, UINT32 iaddr);
    CString GetIPv4Addr(LPCTSTR pstrAddr);
    USHORT GetIPv4Port(LPCTSTR pstrAddr);
    bool IsMulticastAddress(UINT32 uIPAddr);

    // Receiving streams
    HRESULT GetPacket(char* buf, int buflen);
    int GetStartPosition(char* buf, int buflen, const char* strStart);

    void GetValueFromBuf(const CString strBuf, int iPos, CString* pstrResult, char cDelimiter = '"');
    bool FindLogicChannelNum(const CString strServID, ChannelID_logicalChannel* ChPairs, int iIndex, CString* pstrLogicChannelNum = nullptr);
};

