/*
 *	Extended MFC message boxes -- Version 1.2 Lite
 *	Copyright (c) 2004 Michael P. Mehl. All rights reserved.
 *
 *	The contents of this file are subject to the Mozilla Public License
 *	Version 1.1a (the "License"); you may not use this file except in
 *	compliance with the License. You may obtain a copy of the License at 
 *	http://www.mozilla.org/MPL/.
 *
 *	Software distributed under the License is distributed on an "AS IS" basis,
 *	WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 *	for the specific language governing rights and limitations under the
 *	License. 
 *
 *	The Original Code is Copyright (c) 2004 Michael P. Mehl. All rights
 *	reserved. The Initial Developer of the Original Code is Michael P. Mehl
 *	<michael.mehl@web.de>.
 *
 *	Alternatively, the contents of this file may be used under the terms of
 *	the GNU Lesser General Public License Version 2.1 (the "LGPL License"),
 *	in which case the provisions of LGPL License are applicable instead of
 *	those above. If you wish to allow use of your version of this file only
 *	under the terms of the LGPL License and not to allow others to use your
 *	version of this file under the MPL, indicate your decision by deleting
 *	the provisions above and replace them with the notice and other provisions
 *	required by the LGPL License. If you do not delete the provisions above,
 *	a recipient may use your version of this file under either the MPL or
 *	the LGPL License.
 */

#pragma once
#include "../mpc-hc/CMPCThemeStatic.h"
/* mpc-hc modified to use CMPCThemeStatic*/

//////////////////////////////////////////////////////////////////////////////
// Class definition.

class CMessageBoxDialog : public CDialog
{

	DECLARE_DYNAMIC(CMessageBoxDialog)

public:

	//////////////////////////////////////////////////////////////////////////
	// Constructors and destructors of the class.

	// Constructor of the class for direct providing of the message strings.
	CMessageBoxDialog ( CWnd* pParent, CString strMessage, 
		CString strTitle = _T(""), UINT nStyle = MB_OK, UINT nHelp = 0 );

	// Constructor of the class for loading the strings from the resources.
	CMessageBoxDialog ( CWnd* pParent, UINT nMessageID, UINT nTitleID = 0,
		UINT nStyle = MB_OK, UINT nHelp = 0 );

	// Default destructor of the class.
	virtual ~CMessageBoxDialog ( );

public:

	//////////////////////////////////////////////////////////////////////////
	// Method for adding a button to the list of buttons.
	void AddButton ( UINT nID, UINT nTitle, BOOL bIsDefault = FALSE,
		BOOL bIsEscape = FALSE );

	// Method for adding a button to the list of buttons.
	void AddButton ( UINT nID, CString strTitle, BOOL bIsDefault = FALSE,
		BOOL bIsEscape = FALSE );

	// Method for setting the style of the message box.
	void SetStyle ( UINT nStyle );

	// Method for retrieving the style of the message box.
	UINT GetStyle ( );

	// Methods for setting the message to be displayed in the message box.
	void SetMessage ( CString strMessage );
	void SetMessage ( UINT nMessageID );

	// Method for retrieving the message to be displayed in the message box.
	CString GetMessage ( );

	// Methods for setting the title to be displayed in the message box.
	void SetTitle ( CString strTitle );
	void SetTitle ( UINT nTitleID );

	// Method for retrieving the title to be displayed in the message box.
	CString GetTitle ( );

	// Methods for setting the icon to be displayed in the message box.
	void SetMessageIcon ( HICON hIcon );
	void SetMessageIcon ( LPCTSTR lpszIconName, BOOL bStandard = TRUE);

	void SetMessageBeep ( UINT uType );

	// Method for retrieving the icon to be displayed in the message box.
	HICON GetMessageIcon ( );

public:

	//////////////////////////////////////////////////////////////////////////
	// Methods for handling common window functions.

	// Method for displaying the dialog.
	virtual INT_PTR DoModal ( );

	// Method for initializing the dialog.
	virtual BOOL OnInitDialog ( );

	// Method for handling command messages.
	virtual BOOL OnCmdMsg ( UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo );

	// Method for handling messages before dispatching them.
	virtual BOOL PreTranslateMessage ( MSG* pMsg );

protected:

	//////////////////////////////////////////////////////////////////////////
	// Other methods for handling common window functions.

	// Method for handling window messages.
	virtual BOOL OnWndMsg ( UINT message, WPARAM wParam, LPARAM lParam,
		LRESULT* pResult );
    int buttonAreaY; //mpc-hc used for painting bg
private:

	//////////////////////////////////////////////////////////////////////////
	// Private member variables of this dialog.

	CString		m_strMessage;		// Message to be displayed.
	CString		m_strTitle;			// Title to be used.
	UINT		m_nStyle;			// Style of the message box.
	UINT		m_nHelp;			// Help context of the message box.

	HICON		m_hIcon;			// Icon to be displayed in the dialog.

private:
	CSimpleMap<int,CString> m_aButtons; // List of all buttons in the dialog.

	int			m_nDefaultButton;	// ID of the default button.
	int			m_nEscapeButton;	// ID of the escape button.


    CStatic	m_stcIcon;			// Static control for the icon.
    //mpc-hc use themed static instead (for message only--themed static has no impl for icons)
    CMPCThemeStatic	m_stcMessage;		// Static control for the message.

	CWnd*		m_pParent;

	UINT		m_uBeepType;

private:

	//////////////////////////////////////////////////////////////////////////
	// Size handling variables.

	CSize		m_sDialogUnit;		// Variable for the size of a dialog unit.
	CSize		m_sIcon;			// Variable for the size of the icon.
	CSize		m_sMessage;			// Variable for the size of the message.
	CSize		m_sButton;			// Variable for the size of a button.

private:

	//////////////////////////////////////////////////////////////////////////
	// Helper methods.
	
	// Methods for converting a dialog units to a pixel values.
	int XDialogUnitToPixel ( int x );
	int YDialogUnitToPixel ( int y );

	// Method for parsing the given style.
	void ParseStyle ( );

	// Method for creating the icon control.
	void CreateIconControl ( );

	// Method for creating the message control.
	void CreateMessageControl ( );

	// Method for creating the button controls.
	void CreateButtonControls ( );

	// Method for defining the layout of the dialog.
	void DefineLayout ( );

};
