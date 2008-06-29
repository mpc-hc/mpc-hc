// Monitors.cpp : implementation file
//

#include "stdafx.h"
#include "Monitors.h"
#include "MultiMonitor.h"


// CMonitors

CMonitors::CMonitors()
{
	m_MonitorArray.SetSize( GetMonitorCount() );

	ADDMONITOR addMonitor;
	addMonitor.pMonitors = &m_MonitorArray;
	addMonitor.currentIndex = 0;

	::EnumDisplayMonitors( NULL, NULL, AddMonitorsCallBack, (LPARAM)&addMonitor );
}

CMonitors::~CMonitors()
{
	for ( int i = 0; i < m_MonitorArray.GetSize(); i++ )
		delete m_MonitorArray.GetAt( i );
}


// CMonitors member functions

BOOL CALLBACK CMonitors::AddMonitorsCallBack( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData )
{
	LPADDMONITOR pAddMonitor = (LPADDMONITOR)dwData;

	CMonitor* pMonitor = new CMonitor;
	pMonitor->Attach( hMonitor );

	pAddMonitor->pMonitors->SetAt( pAddMonitor->currentIndex, pMonitor );
	pAddMonitor->currentIndex++;

	return TRUE;
}
//
// returns the primary monitor
CMonitor CMonitors::GetPrimaryMonitor()
{
	//the primary monitor always has its origin at 0,0
	HMONITOR hMonitor = ::MonitorFromPoint( CPoint( 0,0 ), MONITOR_DEFAULTTOPRIMARY );
	ASSERT( IsMonitor( hMonitor ) );

	CMonitor monitor;
	monitor.Attach( hMonitor );
	ASSERT( monitor.IsPrimaryMonitor() );

	return monitor;
}

//
// is the given handle a valid monitor handle
BOOL CMonitors::IsMonitor( const HMONITOR hMonitor )
{
	if ( hMonitor == NULL )
		return FALSE;

	MATCHMONITOR match;
	match.target = hMonitor;
	match.foundMatch = FALSE;

	::EnumDisplayMonitors( NULL, NULL, FindMatchingMonitorHandle, (LPARAM)&match );

	return match.foundMatch;
}



//this is the callback method that gets called via IsMontior
BOOL CALLBACK CMonitors::FindMatchingMonitorHandle( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData )
{
	LPMATCHMONITOR pMatch = (LPMATCHMONITOR)dwData;

	if ( hMonitor == pMatch->target )
	{
		//found a monitor with the same handle we are looking for		
		pMatch->foundMatch = TRUE;
		return FALSE; //stop enumerating
	}

	//haven't found a match yet
	pMatch->foundMatch = FALSE;
	return TRUE;	//keep enumerating
}


BOOL CMonitors::AllMonitorsShareDisplayFormat()
{
	return ::GetSystemMetrics( SM_SAMEDISPLAYFORMAT );
}

//
// the number of monitors on the system
int CMonitors::GetMonitorCount()
{ 
	return ::GetSystemMetrics(SM_CMONITORS);
}


CMonitor CMonitors::GetMonitor( const int index ) const
{
#if _MFC_VER >= 0x0700
	ASSERT( index >= 0 && index < m_MonitorArray.GetCount() ); 
#else
	ASSERT( index >= 0 && index < m_MonitorArray.GetSize() );
#endif

	CMonitor* pMonitor = (CMonitor*)m_MonitorArray.GetAt( index );

	return *pMonitor;
}

//
// returns the rectangle that is the union of all active monitors
void CMonitors::GetVirtualDesktopRect( LPRECT lprc )
{
	::SetRect( lprc, 
				::GetSystemMetrics( SM_XVIRTUALSCREEN ),
				::GetSystemMetrics( SM_YVIRTUALSCREEN ),
				::GetSystemMetrics( SM_CXVIRTUALSCREEN ),
				::GetSystemMetrics( SM_CYVIRTUALSCREEN ) );
	
}

//
// these methods determine wheter the given item is
// visible on any monitor
BOOL CMonitors::IsOnScreen( const LPRECT lprc )
{
	return ::MonitorFromRect( lprc, MONITOR_DEFAULTTONULL ) != NULL;
}

BOOL CMonitors::IsOnScreen( const POINT pt )
{
	return ::MonitorFromPoint( pt, MONITOR_DEFAULTTONULL ) != NULL;
}

BOOL CMonitors::IsOnScreen( const CWnd* pWnd )
{
	return ::MonitorFromWindow( pWnd->GetSafeHwnd(), MONITOR_DEFAULTTONULL ) != NULL;
}

CMonitor CMonitors::GetNearestMonitor( const LPRECT lprc )
{
	CMonitor monitor;
	monitor.Attach( ::MonitorFromRect( lprc, MONITOR_DEFAULTTONEAREST ) );

	return monitor;

}

CMonitor CMonitors::GetNearestMonitor( const POINT pt )
{
	CMonitor monitor;
	monitor.Attach( ::MonitorFromPoint( pt, MONITOR_DEFAULTTONEAREST ) );

	return monitor;
}

CMonitor CMonitors::GetNearestMonitor( const CWnd* pWnd )
{
	ASSERT( pWnd );
	ASSERT( ::IsWindow( pWnd->m_hWnd ) );

	CMonitor monitor;
	monitor.Attach( ::MonitorFromWindow( pWnd->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST ) );

	return monitor;
}


