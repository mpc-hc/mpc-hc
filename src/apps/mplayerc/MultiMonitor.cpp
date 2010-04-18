/*
 * $Id$
 *
 * Author: Donald Kackman
 * Email: don@itsEngineering.com
 * Copyright 2002, Donald Kackman
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "MultiMonitor.h"
#include "Monitors.h"

// CMonitor

// constucts a monitor class not attached to any handle
CMonitor::CMonitor() : m_hMonitor( NULL )
{
}


// copy constructor
CMonitor::CMonitor( const CMonitor& monitor  )
{
    m_hMonitor = (HMONITOR)monitor;
}

CMonitor::~CMonitor()
{
}



void CMonitor::Attach( const HMONITOR hMonitor )
{
    ASSERT( CMonitors::IsMonitor( hMonitor ) );

    m_hMonitor = hMonitor;
}

HMONITOR CMonitor::Detach()
{
    HMONITOR hMonitor = m_hMonitor;
    m_hMonitor = NULL;
    return hMonitor;
}

// creates an HDC for the monitor
// it is up to the client to call DeleteDC
//
// for normal multimonitor drawing it is not necessary to get a
// dc for each monitor. Windows takes care of drawing correctly
// on all monitors
//
// Only very exacting applications would need a DC for each monitor
HDC CMonitor::CreateDC() const
{
    ASSERT( IsMonitor() );

    CString name;
    GetName( name );

    //create a dc for this display
    HDC hdc = ::CreateDC( name, name, NULL, NULL );
    ASSERT( hdc != NULL );

    //set the viewport based on the monitor rect's relation to the primary monitor
    CRect rect;
    GetMonitorRect( &rect );

    ::SetViewportOrgEx( hdc, -rect.left, -rect.top, NULL );
    ::SetViewportExtEx( hdc, rect.Width(), rect.Height(), NULL );

    return hdc;
}

int CMonitor::GetBitsPerPixel() const
{
    HDC hdc = CreateDC();
    int ret = ::GetDeviceCaps( hdc, BITSPIXEL ) * ::GetDeviceCaps( hdc, PLANES );
    VERIFY( ::DeleteDC( hdc ) );

    return ret;
}

void CMonitor::GetName( CString& string ) const
{
    ASSERT( IsMonitor() );

    MONITORINFOEX mi;
    mi.cbSize = sizeof( mi );
    ::GetMonitorInfo( m_hMonitor, &mi );

    string = mi.szDevice;
}

//
// these methods return true if any part of the item intersects the monitor rect
BOOL CMonitor::IsOnMonitor( const POINT pt ) const
{
    CRect rect;
    GetMonitorRect( rect );

    return rect.PtInRect( pt );
}

BOOL CMonitor::IsOnMonitor( const CWnd* pWnd ) const
{
    CRect rect;
    GetMonitorRect( rect );

    ASSERT( ::IsWindow( pWnd->GetSafeHwnd() ) );
    CRect wndRect;
    pWnd->GetWindowRect( &wndRect );

    return rect.IntersectRect( rect, wndRect );
}

BOOL CMonitor::IsOnMonitor( const LPRECT lprc ) const
{
    CRect rect;
    GetMonitorRect( rect );

    return rect.IntersectRect( rect, lprc );
}


void CMonitor::GetMonitorRect( LPRECT lprc ) const
{
    ASSERT( IsMonitor() );

    MONITORINFO mi;
    RECT        rc;

    mi.cbSize = sizeof( mi );
    ::GetMonitorInfo( m_hMonitor, &mi );
    rc = mi.rcMonitor;

    ::SetRect( lprc, rc.left, rc.top, rc.right, rc.bottom );
}

//
// the work area does not include the start bar
void CMonitor::GetWorkAreaRect( LPRECT lprc ) const
{
    ASSERT( IsMonitor() );

    MONITORINFO mi;
    RECT        rc;

    mi.cbSize = sizeof( mi );
    ::GetMonitorInfo( m_hMonitor, &mi );
    rc = mi.rcWork;

    ::SetRect( lprc, rc.left, rc.top, rc.right, rc.bottom );
}

//these two center methods are adapted from David Campbell's
//MSJ article (see comment at the top of the header file)
void CMonitor::CenterRectToMonitor( LPRECT lprc, const BOOL UseWorkAreaRect ) const
{
    int  w = lprc->right - lprc->left;
    int  h = lprc->bottom - lprc->top;

    CRect rect;
    if ( UseWorkAreaRect )
        GetWorkAreaRect( &rect );
    else
        GetMonitorRect( &rect );

    lprc->left = rect.left + ( rect.Width() - w ) / 2;
    lprc->top = rect.top + ( rect.Height() - h ) / 2;
    lprc->right	= lprc->left + w;
    lprc->bottom = lprc->top + h;
}

void CMonitor::CenterWindowToMonitor( CWnd* const pWnd, const BOOL UseWorkAreaRect ) const
{
    ASSERT( IsMonitor() );
    ASSERT( pWnd );
    ASSERT( ::IsWindow( pWnd->m_hWnd ) );

    CRect rect;
    pWnd->GetWindowRect( &rect );
    CenterRectToMonitor( &rect, UseWorkAreaRect );
    pWnd->SetWindowPos( NULL, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
}

void CMonitor::ClipRectToMonitor( LPRECT lprc, const BOOL UseWorkAreaRect ) const
{
    int w = lprc->right - lprc->left;
    int h = lprc->bottom - lprc->top;

    CRect rect;
    if ( UseWorkAreaRect )
        GetWorkAreaRect( &rect );
    else
        GetMonitorRect( &rect );

    lprc->left = max( rect.left, min( rect.right - w, lprc->left ) );
    lprc->top = max( rect.top, min( rect.bottom - h, lprc->top ) );
    lprc->right = lprc->left + w;
    lprc->bottom = lprc->top  + h;
}

//
// is the instance the primary monitor
BOOL CMonitor::IsPrimaryMonitor() const
{
    ASSERT( IsMonitor() );

    MONITORINFO mi;

    mi.cbSize = sizeof( mi );
    ::GetMonitorInfo( m_hMonitor, &mi );

    return mi.dwFlags == MONITORINFOF_PRIMARY;
}

//
// is the instance currently attached to a valid monitor handle
BOOL CMonitor::IsMonitor() const
{
    return CMonitors::IsMonitor( m_hMonitor );
}
