#pragma once
//////////////////////////////////////////////////
// CMonitor - wrapper to Win32 multi-monitor API
//
// Author: Donald Kackman
// Email:  don@itsEngineering.com
// Copyright 2002, Donald Kackman
//
// You may freely use or modify this code provided this
// Copyright is included in all derived versions.
//
///////////////////////////////////////////////////

//
//David Campbell's article
//How to Exploit Multiple Monitor Support in Memphis and Windows NT 5.0
//is very helpful for multimonitor api calls
//http://www.microsoft.com/msj/defaultframe.asp?page=/msj/0697/monitor/monitor.htm&nav=/msj/0697/newnav.htm
//



//include multimon.h here to get the types
//multimon.cpp includes this with the COMPILESTUBS define to get the implementations

// CMonitor 

class CMonitor : public CObject
{
public:
//construction destruction
	CMonitor();
	CMonitor( const CMonitor& monitor );
	virtual ~CMonitor();

//operations
	void Attach( const HMONITOR hMonitor );
	HMONITOR Detach();

	void ClipRectToMonitor( LPRECT lprc, const BOOL UseWorkAreaRect = FALSE ) const;
	void CenterRectToMonitor( LPRECT lprc, const BOOL UseWorkAreaRect = FALSE ) const;
	void CenterWindowToMonitor( CWnd* const pWnd, const BOOL UseWorkAreaRect = FALSE ) const;

	HDC CreateDC() const;

//properties
	void GetMonitorRect( LPRECT lprc ) const;
	void GetWorkAreaRect( LPRECT lprc ) const;

	void GetName( CString& string ) const;

	int GetBitsPerPixel() const;

	BOOL IsOnMonitor( const POINT pt ) const;
	BOOL IsOnMonitor( const CWnd* pWnd ) const;
	BOOL IsOnMonitor( const LPRECT lprc ) const;

	BOOL IsPrimaryMonitor() const;
	BOOL IsMonitor() const;

//operators
	operator HMONITOR() const
	{
		return this == NULL ? NULL : m_hMonitor;
	}

	BOOL operator ==( const CMonitor& monitor ) const
	{
		return m_hMonitor == (HMONITOR)monitor;
	}

	BOOL operator !=( const CMonitor& monitor ) const
	{
		return !( *this == monitor );
	}

	void operator =( const CMonitor& monitor ) 
	{
		m_hMonitor = (HMONITOR)monitor;
	}


private:
	HMONITOR m_hMonitor;

};


