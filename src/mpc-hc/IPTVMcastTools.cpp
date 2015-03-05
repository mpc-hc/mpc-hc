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
#include "mplayerc.h"

#define _BSD_SOURCE

typedef int socklen_t;

#define UDP_TX_BUF_SIZE 32768
#define UDP_MAX_PKT_SIZE 65536

#define NUM_OF_PACKETS 4
#define DEFAULT_BUFLEN (1450)
#define DEFAULT_MAX_SP 400
#define DEFAULT_TIMEOUT 3

// Parsing tags:
#define TAG_START_POSITION "<?xml"

#define TAG_SERVICE_PROVIDER "<ServiceProvider"
#define TAG_DOMAIN_NAME "DomainName="
#define TAG_PUSH_ADDRESS "Address="
#define TAG_PORT "Port="
#define TAG_END_SERVICE_PROVIDER "</ServiceProvider>"

#define TAG_SERVICE "<SingleService>"
#define TAG_SS_PORT "Port="
#define TAG_SS_ADDRESS "Address="
#define TAG_SS_SERV_ID "ServiceName="
#define TAG_SS_NAME R"(<Description Language="ENG">)"
#define TAG_SS_SHORTNAME R"(<Name Language="ENG">)"
#define TAG_SS_SHORTNAME2 R"(<NameLanguage="ENG">)"
#define TAG_SS_SERVICE_TYPE "<SI ServiceType="
#define TAG_SS_SERVICE_INFO "ServiceInfo="
#define TAG_END_SERVICE "</SingleService>"

#define TAG_SERVICENAME "<Service>"
#define TAG_SERVICENAME_ID "ServiceName="
#define TAG_LOGICAL_CHANNEL_NUM "<LogicalChannelNumber>"
#define TAG_END_SERVICENAME "</Service>"

//     Example of service list:
//          <IPMulticastAddress Port = "8208" Address = "239.0.0.55" / >
//       < / ServiceLocation>
//       <TextualIdentifier ServiceName = "539" logoURI = "MAY_1/imSer/539.jpg" / >
//       <SI ServiceType = "1" ServiceInfo = "1">
//          <Name Language = "ENG">CLM TV< / Name>
//          <ShortName Language = "ENG">CMT< / ShortName>
//          <Description Language = "ENG">CMT< / Description>
//          <Genre href = "urn:miviewtv:cs:GenreCS:2007:0.1">
//             <urn:Name>CULTURA / ESPECTACULOS< / urn:Name>
//          < / Genre>
//          <ParentalGuidance>
//             <mpeg7:ParentalRating href = "urn:dvb:metadata:cs:ParentalGuidanceCS:2007:1">
//             <mpeg7:Name>No rating available< / mpeg7:Name>
//             < / mpeg7:ParentalRating>
//          < / ParentalGuidance>
//       < / SI>
//    < / SingleService>


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


HRESULT CIPTVMcastTools::GetPacket(char* buf, int buflen)
{
    HRESULT hr = S_OK;
    struct timeval tv;
    fd_set fds;

    sockaddr_in SenderAddr;
    int SenderAddrSize = sizeof(SenderAddr);

    // Set up the file descriptor set.
    FD_ZERO(&fds);
    FD_SET(m_udp_fd, &fds);

    // Set up the struct timeval for the timeout.
    tv.tv_sec = DEFAULT_TIMEOUT;
    tv.tv_usec = 0;

    // Wait until timeout or data received.
    int n = select((int) m_udp_fd + 1, &fds, NULL, NULL, &tv);

    if (n > 0) {
        int iResult = recvfrom(m_udp_fd, buf, buflen, 0, (SOCKADDR*)&SenderAddr, &SenderAddrSize);
        if (iResult < 0) {
            hr = WSAGetLastError();
        }
    } else if (n == 0) {
        // Timeout
        hr = S_FALSE;
    } else if (n == SOCKET_ERROR) {
        // Error
        hr = WSAGetLastError();
    }

    return hr;
}

void CIPTVMcastTools::GetValueFromBuf(const CString strBuf, int iPos, CString* pstrResult, char cDelimiter)
{
    while ((iPos < strBuf.GetLength()) && (strBuf.GetAt(iPos) != cDelimiter)) {
        pstrResult->AppendChar(strBuf[iPos]);
        iPos++;
    }
}

int CIPTVMcastTools::GetStartPosition(char* buf, int buflen, const char* strStart)
{
    int iPos = 0;
    int iStartPosition = -1;
    int strStartLen = (int) strlen(strStart);

    while ((iStartPosition < 0) && (iPos < buflen - strStartLen - 1)) {
        bool bFound = true;
        for (int i = 0; i < strStartLen; i++) {
            bFound = bFound && (buf[iPos + i] == strStart[i]);
            if (!bFound) { 
                break; 
            }
        }
        if (bFound) {
            iStartPosition = iPos;
        }
        iPos++;
    }
    return iStartPosition;
}

HRESULT CIPTVMcastTools::GetServiceProviders(LPCTSTR pstrIP_Address, LPCTSTR pstrPort, HWND hWnd)
{
    char buf[DEFAULT_BUFLEN];
    int buflen = DEFAULT_BUFLEN;
    ZeroMemory(&buf, sizeof(buf));
    HRESULT hr = S_OK;

    UINT32 uRemoteAddr = strIPAddress2UINT32(pstrIP_Address);
    USHORT uPort = strPort2USHORT(pstrPort);

    strServiceProvider SP[DEFAULT_MAX_SP];
    int iSPIndex = 0;

    // Join the group to start receiving packets
    hr = UDPMulticastJoinGroup(uRemoteAddr, uPort);
    if (SUCCEEDED(hr)) {
        int iStartPosition = 0;

        for (int iPacket = 0; iPacket < NUM_OF_PACKETS; iPacket++) {
            hr = GetPacket(buf, buflen);

            if ((iPacket == 0) || (iStartPosition < 0)) {
                // Search start position
                iStartPosition = GetStartPosition(buf, buflen, TAG_START_POSITION);
            }

            if (iStartPosition >= 0) {
                CString strStream;
                for (int i = iStartPosition; i < buflen; i++) {
                    if (buf[i]) {
                        strStream.AppendChar(buf[i]);
                    }
                }
                // The next packet will start at 0
                iStartPosition = 0;

                // Get the Service Providers from the packet
                boolean bError = false;
                do {
                    int iSPBegin = strStream.Find(_T(TAG_SERVICE_PROVIDER));
                    int iDomainPosition = strStream.Find(_T(TAG_DOMAIN_NAME));
                    int iPushAddressPosition = strStream.Find(_T(TAG_PUSH_ADDRESS));
                    int iPortPosition = strStream.Find(_T(TAG_PORT));
                    int iSPEnd = strStream.Find(_T(TAG_END_SERVICE_PROVIDER));

                    if ((iSPBegin > 0 &&
                            (iDomainPosition < iSPBegin || iPushAddressPosition < iSPBegin || iPortPosition < iSPBegin)) &&
                            !((iDomainPosition < iPushAddressPosition) && (iPushAddressPosition < iPortPosition) && (iPortPosition < iSPEnd))) {
                        // Remove wrong content before iSPBegin
                        strStream.Delete(0, iSPBegin);
                        continue;
                    }

                    if (((iSPBegin >= 0) && (iSPEnd > 0) && (iSPEnd > iSPBegin) && (iDomainPosition <= iSPEnd) &&
                            (iPushAddressPosition <= iSPEnd) && (iPortPosition < iSPEnd)) ||
                            ((iDomainPosition < iPushAddressPosition) && (iPushAddressPosition < iPortPosition) && (iPortPosition < iSPEnd))) {

                        // Get the Service Provider information
                        CString strDomainName, strIP_addr, strPort;
                        GetValueFromBuf(strStream, iDomainPosition + sizeof(TAG_DOMAIN_NAME), &strDomainName);
                        GetValueFromBuf(strStream, iPushAddressPosition + sizeof(TAG_PUSH_ADDRESS), &strIP_addr);
                        GetValueFromBuf(strStream, iPortPosition + sizeof(TAG_PORT), &strPort);

                        // Remove information already used
                        strStream.Delete(0, iSPEnd + sizeof(TAG_END_SERVICE_PROVIDER) - 1);
                        boolean bFound = false;
                        int iPos = 0;
                        do {
                            if (strDomainName.Compare(SP[iPos].DomainName) == 0) {
                                bFound = true;
                            }
                            iPos++;
                        } while (!bFound && (iPos <= iSPIndex));

                        if (!bFound) {
                            SP[iSPIndex].DomainName = strDomainName;
                            SP[iSPIndex].IP_addr = strIP_addr;
                            SP[iSPIndex].port = strPort;
                            m_pSP = &SP[iSPIndex];
                            ::SendMessage(hWnd, WM_IPTV_NEW_SERVICEPROVIDER, 0, (LPARAM)m_pSP);
                            if (iSPIndex < DEFAULT_MAX_SP) {
                                iSPIndex++;
                            }
                        }
                    } else {
                        bError = true;
                    }
                } while (!bError && (strStream.GetLength() > 0) && (iSPIndex < 100));
            }
        }
        // Leave the group to stop receiving packets
        UDPMulticastLeaveGroup();
    }

    return hr;
}

bool CIPTVMcastTools::FindLogicChannelNum(CString strServID, ChannelID_logicalChannel* ChPairs, int iIndex, CString* pstrLogicChannelNum)
{
    bool bRet = false;
    for (int i = 0; i <= iIndex; i++) {
        if (ChPairs[i].strChannelID.Compare(strServID.Trim()) == 0) {
            bRet = true;
            if (pstrLogicChannelNum) {
                pstrLogicChannelNum->Insert(0, ChPairs[i].strLogicalChannel);
            }
        }
    }
    return bRet;
}

void CIPTVMcastTools::BroadcastChannelsDiscover(LPCTSTR pstrIP_Address, LPCTSTR pstrPort, HWND hWnd, std::atomic<bool> &bStopRequested)
{
    char buf[DEFAULT_BUFLEN];
    ChannelID_logicalChannel ChPairs[DEFAULT_MAX_SP];
    int iIndex = 0;

    int buflen = DEFAULT_BUFLEN;
    ZeroMemory(&buf, sizeof(buf));
    HRESULT hr = S_OK;
    CDVBChannel Channel;

    UINT32 remote_addr = strIPAddress2UINT32(pstrIP_Address);
    USHORT port = strPort2USHORT(pstrPort);
    // Join the group to start receiving packets
    hr = UDPMulticastJoinGroup(remote_addr, port);
    if (SUCCEEDED(hr)) {
        int iStartPosition = -1;

        // Find start position
        // PayloadID == 05 (SD&S Package Discovery Information)
        do {
            hr = GetPacket(buf, buflen);
            if (hr == S_OK) {
                iStartPosition = GetStartPosition(buf, buflen, TAG_START_POSITION);
            }
        } while ((hr == S_OK && buf[4] != 5) ||
            (hr == S_OK && buf[4] == 5 && (iStartPosition < 0)) && !(bStopRequested));

        CString strStream;
        while ((hr == S_OK) && buf[4] == 5 && iStartPosition >= 0 && !(bStopRequested)) {
            // Read packets with the available packages
            for (int i = iStartPosition; i < buflen - iStartPosition; i++) {
                if (buf[i]) {
                    strStream.AppendChar(buf[i]);
                }
            }
            strStream.Replace(_T("\n"), _T(""));

            // Get the ServiceName - Logical channel number pairs
            boolean bError = false;
            do {
                int iAPBegin = strStream.Find(_T(TAG_SERVICENAME));
                int iAPServIDPos = strStream.Find(_T(TAG_SERVICENAME_ID));
                int iAPLogChannelPos = strStream.Find(_T(TAG_LOGICAL_CHANNEL_NUM));
                int iAPEnd = strStream.Find(_T(TAG_END_SERVICENAME));
                if (iAPEnd == -1) {
                    if ((iAPBegin >= 0) && (strStream.GetLength() < (buflen - iStartPosition - 1))) {
                        // Append data from the next packet to parse the entire block
                        bError = true;
                        continue;
                    }
                    iAPEnd = strStream.GetLength() - 1;
                }
                if (iAPBegin > 0 &&
                        (iAPServIDPos < iAPBegin || iAPLogChannelPos < iAPBegin || iAPBegin > iAPEnd)) {
                    // Remove wrong content before iSPBegin
                    TRACE(_T("Removing non parsed content while getting ServiceName - Logical channel number pairs: \"%s\"\n"), strStream.Left(iAPBegin));
                    strStream.Delete(0, iAPBegin);
                    continue;
                }

                if ((iAPServIDPos > iAPEnd) || (iAPLogChannelPos > iAPEnd)) {
                    TRACE(_T("Error parsing channel pairs. ServIDPos: \n%s\n"), strStream.Mid(iAPBegin, iAPEnd - iAPBegin + sizeof(TAG_END_SERVICENAME)));
                }

                if ((iAPBegin >= 0) && (iAPEnd > 0) && (iAPEnd > iAPBegin) && (iAPServIDPos <= iAPEnd) &&
                        (iAPLogChannelPos <= iAPEnd) && (iAPServIDPos >= 0) && (iAPLogChannelPos >= 0)) {

                    // Get the Service ID - Logical number pairs:
                    CString strAPServID, strAPLogChannel;
                    GetValueFromBuf(strStream, iAPServIDPos + sizeof(TAG_SERVICENAME_ID), &strAPServID);
                    GetValueFromBuf(strStream, iAPLogChannelPos + sizeof(TAG_LOGICAL_CHANNEL_NUM) - 1, &strAPLogChannel, '<');

                    // Remove data already used
                    strStream.Delete(0, iAPEnd + sizeof(TAG_END_SERVICENAME) - 1);

                    // Insert new channel in the list (and check for duplicates)
                    if (!bError && iIndex < DEFAULT_MAX_SP && !FindLogicChannelNum(strAPServID, ChPairs, iIndex)) {
                        ChPairs[iIndex].strChannelID = strAPServID.Trim();
                        ChPairs[iIndex].strLogicalChannel = strAPLogChannel.Trim();
                        iIndex++;
                    }
                } else {
                    if ((iAPServIDPos == -1) || (iAPLogChannelPos == -1)) {
                        TRACE(_T("Error parsing channel pairs. ServIDPos: %i LogChannelsPos: %i \n"), iAPServIDPos, iAPLogChannelPos);
                    }
                    bError = true;
                }
            } while (!bError && (strStream.GetLength() > 0) && !(bStopRequested));

            if (!bStopRequested) {
                hr = GetPacket(buf, buflen);
            }
        }

        if (!bStopRequested) {
            // Find start position
            // PayloadID == 02 (SD&S Broadcast Discovery Information)
            if (hr == S_OK && buf[4] == 2) {
                iStartPosition = GetStartPosition(buf, buflen, TAG_START_POSITION);
            }
            while ((hr == S_OK && buf[4] != 2) ||
                (hr == S_OK && buf[4] == 2 && (iStartPosition < 0)))
            {
                hr = GetPacket(buf, buflen);
                if (hr == S_OK) {
                    iStartPosition = GetStartPosition(buf, buflen, TAG_START_POSITION);
                }
            }
        }

        strStream.Empty();
        while ((hr == S_OK) && buf[4] == 2 && iStartPosition >= 0 && !(bStopRequested)) {
            // Read packets with broadcast channels information
            int nDestSize = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)buf + iStartPosition, buflen - iStartPosition, nullptr, 0);
            if (nDestSize > 0) {
                CString strStreamNew;
                LPWSTR strResultBuff = strStreamNew.GetBufferSetLength(nDestSize);
                MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)buf + iStartPosition, buflen - iStartPosition, strResultBuff, nDestSize);
                strStream.Append(strResultBuff);
            } else {
                hr = -1;
                TRACE(_T("Conversion to UTF8 failed.\n"));
                continue;
            }

            // Get the Service Providers from the packet
            strStream.Replace(_T("\n"), _T(""));
            boolean bError = false;
            do {
                int iSSBegin = strStream.Find(_T(TAG_SERVICE));
                int iSSPortPos = strStream.Find(_T(TAG_SS_PORT));
                int iSSAddressPos = strStream.Find(_T(TAG_SS_ADDRESS));
                int iSSServiceID = strStream.Find(_T(TAG_SS_SERV_ID));
                int iSSNamePos = strStream.Find(_T(TAG_SS_NAME));
                int iSSShortNamePos = strStream.Find(_T(TAG_SS_SHORTNAME));
                int iSSShortNamePos2 = strStream.Find(_T(TAG_SS_SHORTNAME2));
                int iSSEnd = strStream.Find(_T(TAG_END_SERVICE));
                if (iSSEnd == -1) {
                    if ((iSSBegin >= 0) && (strStream.GetLength() < (buflen - iStartPosition - 1))) {
                        // Append the data from the next packet to avoid cutting blocks
                        bError = true;
                        continue;
                    }
                    iSSEnd = strStream.GetLength() - 1;
                }

                if (iSSBegin > 0 &&
                        (iSSPortPos < iSSBegin || iSSAddressPos < iSSBegin || iSSServiceID < iSSBegin || iSSNamePos < iSSBegin ||
                         iSSShortNamePos < iSSBegin || iSSBegin > iSSEnd)) {
                    // Remove wrong content before iSPBegin
                    TRACE(_T("Removing non parsed content while getting Service Providers: \"%s\"\n"), strStream.Left(iSSBegin));
                    strStream.Delete(0, iSSBegin);
                    continue;
                }

                if ((iSSBegin >= 0) && (iSSEnd > 0) && (iSSEnd > iSSBegin) && (iSSPortPos <= iSSEnd) &&
                        (iSSAddressPos <= iSSEnd) &&
                        ((iSSNamePos <= iSSEnd) || (iSSShortNamePos <= iSSEnd) || (iSSShortNamePos2 <= iSSEnd)) &&
                        (iSSPortPos >= 0) && (iSSAddressPos >= 0) &&
                        ((iSSNamePos >= 0) || (iSSShortNamePos >= 0) || (iSSShortNamePos >= 0))) {

                    // Get the Service Provider information
                    CString strSSPort, strIP, strServiceID, strServiceName, strServiceShortName, strServiceType, strServiceInfo, strLogicChannelNum;
                    GetValueFromBuf(strStream, iSSPortPos + sizeof(TAG_SS_PORT), &strSSPort);
                    GetValueFromBuf(strStream, iSSAddressPos + sizeof(TAG_SS_ADDRESS), &strIP);
                    GetValueFromBuf(strStream, iSSServiceID + sizeof(TAG_SS_SERV_ID), &strServiceID);
                    GetValueFromBuf(strStream, iSSNamePos + sizeof(TAG_SS_NAME) - 1, &strServiceName, '<');
                    if ((iSSShortNamePos >= 0) && (iSSShortNamePos <= iSSEnd)) {
                        GetValueFromBuf(strStream, iSSShortNamePos + sizeof(TAG_SS_SHORTNAME) - 1, &strServiceShortName, '<');
                    } else if ((iSSShortNamePos2 >= 0) && (iSSShortNamePos2 <= iSSEnd)) {
                        GetValueFromBuf(strStream, iSSShortNamePos2 + sizeof(TAG_SS_SHORTNAME2) - 1, &strServiceShortName, '<');
                    } else {
                        strServiceShortName = _T("-");
                        TRACE(_T("Channel shortname not found"));
                    }

                    if (!FindLogicChannelNum(strServiceID, ChPairs, iIndex, &strLogicChannelNum)) {
                        TRACE(_T("Logic Channel \"%s\" not found. \n"), strServiceID);
                    }

                    // Remove data already used
                    strStream.Delete(0, iSSEnd + sizeof(TAG_END_SERVICE) - 1);

                    Channel.SetName(strServiceShortName);
                    Channel.SetUrl(PROTOCOL_RTP + strIP + _T(":") + strSSPort);
                    Channel.SetVideoPID(1);  // Radio channels not considered here
                    Channel.SetDefaultAudio(-1); // Info not relevant for iptv
                    Channel.SetDefaultSubtitle(0); // Info not relevant for iptv
                    CT2CA pszConvertedAnsiString3(strLogicChannelNum);
                    char* sLogicChannelNum = pszConvertedAnsiString3;
                    Channel.SetOriginNumber(atoi(sLogicChannelNum));
                    Channel.SetPrefNumber(atoi(sLogicChannelNum));
                    CT2CA pszConvertedAnsiString4(strServiceID);
                    char* sServiceID = pszConvertedAnsiString4;
                    Channel.SetONID(atoi(sServiceID));
                    ::SendMessage(hWnd, WM_IPTV_NEW_CHANNEL, 0, (LPARAM)(LPCTSTR)Channel.ToString());
                } else {
                    bError = true;
                }
            } while (!bError && (strStream.GetLength() > 0));

            hr = GetPacket(buf, buflen);

            if (hr != S_OK) {
                TRACE(_T("Error reading packet: %u \n"), hr);
            }

        }

        // Leave the group to stop receiving packets
        UDPMulticastLeaveGroup();
    }
}

//   Payload 06: information on EPG address.
//   <BCG Id = "EPG">
//   <Name Language = "eng">MiViewTV ProgramGuide< / Name>
//   <TransportMode>
//   <DVBSTP Source = "EPG_0.imagenio.es" Port = "3937" Address = "239.0.2.130">
//   <PayloadId Id = "0xF1">
//   <Segment ID = "0x430a" Version = "0" / >
//   <Segment ID = "0x4001" Version = "4" / >
//   <Segment ID = "0x4388" Version = "3" / >

HRESULT CIPTVMcastTools::GetEPGInfo(LPCTSTR pstrIP_Address, LPCTSTR pstrPort, LPCTSTR pstrInfo)
{
    char buf[DEFAULT_BUFLEN];

    int buflen = DEFAULT_BUFLEN;
    ZeroMemory(&buf, sizeof(buf));
    HRESULT hr = S_OK;
    CDVBChannel Channel;

    UINT32 uRemoteAddr = strIPAddress2UINT32(pstrIP_Address);
    USHORT uPort = strPort2USHORT(pstrPort);

    // Join the group to start receiving packets
    hr = UDPMulticastJoinGroup(uRemoteAddr, uPort);
    if (SUCCEEDED(hr)) {
        int iStartPosition = -1;

        // Find start position
        // PayloadID == 0xF1 (EPG Information)
        do {
            hr = GetPacket(buf, buflen);
            if (hr == S_OK) {
                iStartPosition = 13;
            }
        } while ((hr == S_OK && buf[4] != 0xF1) ||
                 (hr == S_OK && buf[4] == 0xF1 && (iStartPosition < 0)));

        CString strStream = _T("");
        while ((hr == S_OK) && buf[4] == 0xF1 && iStartPosition >= 0) {
            // Read packets with the available packages
            for (int i = iStartPosition; i < buflen - iStartPosition; i++) {
                if (buf[i]) {
                    strStream.AppendChar(buf[i]);
                }
            }

            // Get the ServiceName - Logical channel number pairs
            boolean bError = false;
            do {
                int iAPBegin = strStream.Find(_T(TAG_SERVICENAME));
                int iAPServIDPos = strStream.Find(_T(TAG_SERVICENAME_ID));
                int iAPLogChannelPos = strStream.Find(_T(TAG_LOGICAL_CHANNEL_NUM));
                int iAPEnd = strStream.Find(_T(TAG_END_SERVICENAME));
                if (iAPEnd == -1) {
                    iAPEnd = strStream.GetLength() - 1;
                }

                if (iAPBegin > 0 &&
                        (iAPServIDPos < iAPBegin || iAPLogChannelPos < iAPBegin || iAPBegin > iAPEnd)) {
                    // Remove wrong content before iSPBegin
                    strStream.Delete(0, iAPBegin);
                    continue;
                }

                if ((iAPBegin >= 0) && (iAPEnd > 0) && (iAPEnd > iAPBegin) && (iAPServIDPos <= iAPEnd) &&
                        (iAPLogChannelPos <= iAPEnd) && (iAPServIDPos >= 0) && (iAPLogChannelPos >= 0)) {

                    // Get the Service ID - Logical number pairs:
                    CString strAPServID, strAPLogChannel;
                    GetValueFromBuf(strStream, iAPServIDPos + sizeof(TAG_SERVICENAME_ID), &strAPServID);
                    GetValueFromBuf(strStream, iAPLogChannelPos + sizeof(TAG_LOGICAL_CHANNEL_NUM) - 1, &strAPLogChannel, '<');

                    // Remove data already used
                    strStream.Delete(0, iAPEnd + sizeof(TAG_END_SERVICENAME) - 1);

                    // Insert new channel in the list
                } else {
                    bError = true;
                }
            } while (!bError && (strStream.GetLength() > 0));

            hr = GetPacket(buf, buflen);
        }
        // Leave the group to stop receiving packets
        UDPMulticastLeaveGroup();
    }

    return S_OK;
}

HRESULT CIPTVMcastTools::VerifyChannel(UINT32 remote_addr, USHORT uPort)
{
    char buf[DEFAULT_BUFLEN];
    int buflen = DEFAULT_BUFLEN;
    memset(&buf, 0, sizeof(buf));
    HRESULT hr = S_OK;

    // Join the group to start receiving packets
    hr = UDPMulticastJoinGroup(remote_addr, uPort);
    if (SUCCEEDED(hr)) {
        hr = GetPacket(buf, buflen);
    }
    // Leave the group to stop receiving packets
    UDPMulticastLeaveGroup();
    return hr;
}

HRESULT CIPTVMcastTools::VerifyChannel(LPCTSTR pstrAddr)
{
    HRESULT hr = S_OK;
    UINT32 uRemoteAddr = strIPAddress2UINT32(GetIPv4Addr(pstrAddr));
    USHORT uPort = GetIPv4Port(pstrAddr);
    hr = VerifyChannel(uRemoteAddr, uPort);

    return hr;
}

void CIPTVMcastTools::ScanRangeIPs(LPCTSTR pstrIP_Addr1, LPCTSTR pstrIP_Addr2, LPCTSTR pstrPort, HWND hWnd, std::atomic<bool> &bStopRequested)
{
    UINT32 uIP1 = strIPAddress2UINT32(pstrIP_Addr1);
    UINT32 uIP2 = strIPAddress2UINT32(pstrIP_Addr2);
    uIP1 = ntohl(uIP1);
    uIP2 = ntohl(uIP2);

    USHORT uPort = strPort2USHORT(pstrPort);

    for (UINT32 uCount = uIP1; uCount <= uIP2; uCount++) {
        if (bStopRequested) {
            break;
        }
        UINT32 uIPCurrent = htonl(uCount);
        // Show current IP Address
        struct in_addr address_struct;
        address_struct.s_addr = uIPCurrent;
        CString strIP = inet_ntoa(address_struct);
        strIP.Append(_T(":") + (CString)pstrPort);
        strIP.Insert(0, PROTOCOL_RTP);
        HRESULT hr = VerifyChannel(uIPCurrent, uPort);
        if (hr == S_OK) {
            CDVBChannel Channel;
            Channel.SetName(_T("."));
            Channel.SetUrl(strIP);
            Channel.SetVideoPID(1);  // Radio channels not considered here
            Channel.SetDefaultAudio(-1); // Info not relevant for iptv
            Channel.SetDefaultSubtitle(0); // Info not relevant for iptv
            Channel.SetOriginNumber(0);
            Channel.SetPrefNumber(0);
            Channel.SetONID(0);
            CString strChannel = Channel.ToString();
            ::SendMessage(hWnd, WM_IPTV_NEW_CHANNEL, 0, (LPARAM)(LPCTSTR)Channel.ToString());
        }
    }
}
