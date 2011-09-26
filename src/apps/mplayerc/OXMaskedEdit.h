// ==========================================================================
// 							Class Specification : COXMaskedEdit
// ==========================================================================

// Copyright © Dundas Software Ltd. 1997 - 1998, All Rights Reserved
                          
// //////////////////////////////////////////////////////////////////////////

// Properties:
//	NO	Abstract class (does not have any objects)
//	YES	Derived from CEdit

//	YES	Is a Cwnd.                     
//	YES	Two stage creation (constructor & Create())
//	YES	Has a message map
//	NO	Needs a resource (template)

//	NO	Persistent objects (saveable on disk)      
//	NO	Uses exceptions

// //////////////////////////////////////////////////////////////////////////

// Desciption :         
//  The Masked Edit control provides restricted data input as well as formatted data output. 
//  This control supplies visual cues about the type of data being entered or displayed.
//  
//  The Masked Edit control generally behaves as a standard text box control with enhancements 
//  for optional masked input and formatted output. If you don't use an input mask, the Masked 
//  Edit control behaves much like a standard text box.
//  
//  If you define an input mask, each character position in the Masked Edit control maps to either 
//  a placeholder of a specified type or a literal character. Literal characters, or literals, can 
//  give visual cues about the type of data being used. For example, the parentheses surrounding 
//  the area code of a telephone number are literals: (206).
//  
//  If you attempt to enter a character that conflicts with the input mask, the control generates a 
//  ValidationError beep. We could throw an exception but I think this is too heavy.  The input mask 
//  prevents you from entering invalid characters into the control.
//  
//  When you define an input mask, the Masked Edit control behaves differently from the standard text box. 
//  The insertion point automatically skips over literals as you enter data or move the insertion point.
//  
//  When you insert or delete a character, all nonliteral characters to the right of the insertion point 
//  are shifted, as necessary. If shifting these characters leads to a validation error, the insertion or 
//  deletion is prevented, and a ValidationError beep is triggered.
//  
//  Suppose the Mask property is defined as "?###", and the current value of the Text property is "A12." 
//  If you attempt to insert the letter "B" to the left of the letter "A," the "A" would shift to the right. 
//  Since the second value of the input mask requires a number, the letter "A" would cause the control to 
//  generate a ValidationError beep.
//  
//  The Masked Edit control also validates the parameter value of the SetInputText function the user passes 
//  at run time. If you use the SetInputText function so that it conflicts with the input mask, the function 
//  will return an errorcode.
//  
//  You may select text in the same way as for a standard text box control. When selected text is deleted, 
//  the control attempts to shift the remaining characters to the right of the selection. However, any 
//  remaining character that might cause a validation error during this shift is deleted, and no 
//  ValidationError beep is generated.
//  
//  Normally, when a selection in the Masked Edit control is copied onto the Clipboard, the entire selection, 
//  including literals, is transferred onto the Clipboard. You can use the SetClipMode function to define the 
//  behavior for transferring only user-entered data onto the Clipboard or not - literal characters that are 
//  part of the input mask are not copied.
//  
//  You are able to attach a COXMaskedEdit to an existing edit control by subclassing the latter. 

//  Mask character  Description 
//  --------------  ----------------------------------------------------------------------------------------------
//        #         Digit placeholder. (0-9)
//  
//        .         Decimal placeholder. The actual character used is the one specified as the decimal placeholder 
//                  in your international settings. This character is treated as a literal for masking purposes.
//  
//        ,         Thousands separator. The actual character used is the one specified as the thousands separator 
//                  in your international settings. This character is treated as a literal for masking purposes.
//  
//        :         Time separator. The actual character used is the one specified as the time separator in your 
//                  international settings. This character is treated as a literal for masking purposes.
//  
//        /         Date separator. The actual character used is the one specified as the date separator in your 
//                  international settings. This character is treated as a literal for masking purposes.
//  
//        A         Alphanumeric character placeholder (0-9 and a-Z)
//  
//        &         Character placeholder. Valid values for this placeholder are ANSI characters in the following 
//                  ranges: 32-126 and 128-255.
//  
//        ?         Alphabetic placeholder (a-Z)
//  
//        >         Alphabetic placeholder, but forces them to Uppercase chars (A-Z)
//  
//        <         Alphabetic placeholder,  but forces them to lowercase (a-z)
//  
//        \         Treat the next character in the mask string as a literal. This allows you to include 
//                  the '#', '&', 'A', '<', '>',and '?' characters in the mask. This character is treated as a 
//                  literal for masking purposes.
//  
//  Literal         All other symbols are displayed as literals; that is, as themselves.

// Remark:
//		***

// Prerequisites (necessary conditions):
//		***

/////////////////////////////////////////////////////////////////////////////
#ifndef __OXMaskedEdit_h__
#define __OXMaskedEdit_h__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "OXDllExt.h"


#ifndef __AFXTEMPL_H__
#include <afxtempl.h>
#define __AFXTEMPL_H__
#endif


#ifndef WM_OX_APP
// You can redefine WM_OX_APP by defining it before including this file
#define WM_OX_APP					WM_APP + 100
#endif
// validation notification sent to parent window 
#define OXMEN_VALIDATE				(WM_OX_APP + 210)

// for OXMEN_VALIDATE notification we supply our structure where:
//		hdr					-	standard NMHDR structure
//		bValid				-	result of validation (TRUE - valid, FALSE - invalid)
//		bDefaultValidation	-	if equals TRUE and bValid is TRUE still control defined
//								virtual OnValidate function will be called
//		nPosition			-	real position where cursor should be set if bValid is FALSE
typedef struct tagMENMHDR
{
	NMHDR hdr;
	BOOL bValid;
	BOOL bDefaultValidation;
	int nPosition;
} MENMHDR, *LPMENMHDR;

const TCHAR chNULL   = _T('\0') ;
const TCHAR chCR     = _T('\r') ;
const TCHAR chLF     = _T('\n') ;
const TCHAR chSPACE  = _T(' ' ) ;
const TCHAR chPERIOD = _T('.' ) ;
const TCHAR chCOMMA  = _T(',' ) ;
const TCHAR chCOLON  = _T(':' ) ;
const TCHAR chSLASH  = _T('/' ) ;

/////////////////////////////////////////////////////////////////////////////
// CMaskData object

const TCHAR chMaskPlaceholderDECIMALSEPARATOR   = _T('.')  ;
const TCHAR chMaskPlaceholderTHOUSANDSSEPARATOR = _T(',')  ;
const TCHAR chMaskPlaceholderTIMESEPARATOR      = _T(':')  ;
const TCHAR chMaskPlaceholderDATESEPARATOR      = _T('/')  ;
const TCHAR chMaskPlaceholderDIGIT              = _T('#')  ;
const TCHAR chMaskPlaceholderALPHANUMERIC       = _T('A')  ;
const TCHAR chMaskPlaceholderALPHABETIC         = _T('?')  ;
const TCHAR chMaskPlaceholderALPHABETICUPPER    = _T('>')  ;
const TCHAR chMaskPlaceholderALPHABETICLOWER    = _T('<')  ;
const TCHAR chMaskPlaceholderCHARACTER          = _T('&')  ;
const TCHAR chMaskPlaceholderLITERALESCAPE      = _T('\\') ;

enum enumMaskDataType
{
	MaskDataTypeDECIMALSEPARATOR=0,  // Decimal separator. 
	MaskDataTypeTHOUSANDSSEPARATOR,  // Thousands separator. 
	MaskDataTypeTIMESEPARATOR     ,  // Time separator. 
	MaskDataTypeDATESEPARATOR     ,  // Date separator. 
	MaskDataTypeDIGIT             ,  // # digit placeholder (0-9). 
	MaskDataTypeALPHANUMERIC      ,  // A alphanumeric character placeholder (0-9 and a-Z). 
	MaskDataTypeALPHABETIC        ,  // ? Alphabetic placeholder (a-Z). 
	MaskDataTypeALPHAETICUPPER    ,  // > Alphabetic placeholder, but forced to Uppercase characters (A-Z). 
	MaskDataTypeALPHAETICLOWER    ,  // < Alphabetic placeholder, but forced to Lowercase characters (a-z). 
	MaskDataTypeCHARACTER         ,  // # character placeholder (ANSI characters 32-126 and 128-255). 
	MaskDataTypeLITERALESCAPE     ,  // All other symbols are displayed as themselves. 
	MaskDataTypeLITERAL           ,  // All other symbols are displayed as themselves. 
	MASKDATATYPECOUNT
} ;

class OX_CLASS_DECL CMaskData : public CObject
{
	DECLARE_DYNCREATE(CMaskData)
	
public:
	CMaskData() ;

// Attributes
public:
	enumMaskDataType m_eType   ;
	TCHAR            m_chValue ;
	
// Operations
public:
	void  operator=(const CMaskData& src) ;
	BOOL  IsInputData   () ;
	BOOL  IsValidInput  (TCHAR chNewChar) ;
	TCHAR PreProcessChar(TCHAR chNewChar) ;
	
	// Diagnostic Support
#ifdef _DEBUG
public:
	virtual void AssertValid() const ;
	virtual void Dump(CDumpContext& dc) const ;
#endif
};

/////////////////////////////////////////////////////////////////////////////
// COXMaskedEdit window

class OX_CLASS_DECL COXMaskedEdit : public CEdit
{
	DECLARE_DYNAMIC(COXMaskedEdit)

// Data members -------------------------------------------------------------
protected:
	CTypedPtrList<CObList, CMaskData*> m_listData;
	BOOL m_bInsertMode;
	TCHAR m_chPromptSymbol;
	TCHAR m_chIntlDecimal;
	TCHAR m_chIntlThousands;
	TCHAR m_chIntlTime;
	TCHAR m_chIntlDate;
	BOOL m_bAutoTab;	

	int m_nSetTextSemaphor;
	int m_bNotifyParent;
	
// Member functions ---------------------------------------------------------
public:
	// --- In     : pszMask - A Mask can already be specified at construction time.
	// --- Out    :
	// --- Returns:
	// --- Effect : Constructs the object
	COXMaskedEdit(LPCTSTR pszMask=_T("")) ;
	
	// --- In     :
	// --- Out    :
	// --- Returns:
	// --- Effect : Destructor of the object
	virtual ~COXMaskedEdit() ;
	
	// --- In     : dwStyle    - edit control styles. 
	//            : rect       - edit control size and position. 
	//            : pParentWnd - edit control parent window. 
	//            : nID        - edit control ID. 
	// --- Out    : 
	// --- Returns:	True on success. 
	// --- Effect : Same as CEdit create except that it sets the mask after creation. 
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID) ;
	
public:
	// --- In     :
	// --- Out    : 
	// --- Returns:	the current mask. 
	// --- Effect : 
	CString GetMask() const ;
	
	// --- In     : pszMask - new mask. 
	// --- Out    :
	// --- Returns:
	// --- Effect : Changing the mask may empty a possible filled masked edit. 
	void    SetMask(LPCTSTR pszMask=_T("")) ;

	// --- In     :
	// --- Out    :
	// --- Returns: The fully formated mask with all input data. 
	// --- Effect : 
	CString ShowMask() const ;

	// --- In     :
	// --- Out    :
	// --- Returns:	string that represent pure data that was typed in excluding literals
	// --- Effect : 
	CString GetInputData() const ;
	
	// --- In     :	lpszText	-	string to be stripped by applying the current mask
	// --- Out    :
	// --- Returns:	string that represent pure data in the specified string 
	//				excluding literals
	// --- Effect : 
	CString GetInputData(LPCTSTR lpszText) const ;
	
	// --- In     : pszInputData - Each character is entered into the control as if the user typed it in. 
	//            : nBeginPos    - Begin position for inserting or overwriting the Insert symbols in the mask. 
	//            : bAllowPrompt - whether or not the prompt symbol is a valid input character. 
	// --- Out    :
	// --- Returns: True if some data was inserted. 
	// --- Effect : Use to programmatically insert pszInputData into the masked edit control. 
	//            : The mask is applied to the Inputdata. 
	BOOL    SetInputData(LPCTSTR pszInputData, int nBeginPos=0, BOOL bAllowPrompt=TRUE) ;

	// --- In     :
	// --- Out    :
	// --- Returns: the current prompt symbol. 
	// --- Effect : 
	TCHAR   GetPromptSymbol() ;
	
	// --- In     : chNewPromptSymbol - new prompt symbol cannot be null, carriage return or line feed. 
	// --- Out    :
	// --- Returns:
	// --- Effect : The default is a space. 
	void    SetPromptSymbol(TCHAR chNewPromptSymbol) ;
	
	// --- In     : bOnlyInput - set to true to clear just the data. 
	// --- Out    :
	// --- Returns:
	// --- Effect : Clears the contents of the masked edit.  Depending on the bOnlyInput clears all data (mask + input) or only input data
	void    EmptyData(BOOL bOnlyInput=FALSE) ;

	// --- In     : 
	// --- Out    :
	// --- Returns:	TRUE if the control has only mask and prompt symbols, 
	//				or FALSE otherwise
	// --- Effect : Retrieves the flag that specifies whether the control
	//				is populated with some text other than mask or prompt 
	//				symbols or not
	BOOL	IsInputEmpty() ;

	// --- In     :
	// --- Out    :
	// --- Returns: The insert mode. 
	// --- Effect : 
	BOOL    GetInsertMode() const ;
	
	// --- In     :
	// --- Out    :
	// --- Returns:
	// --- Effect : Changes the insert mode. 
	void    SetInsertMode(BOOL bInsertMode) ;

	// --- In     :
	// --- Out    :
	// --- Returns:
	// --- Effect : This function gets called when a user error occurs. 
	virtual void ValidationError() ;
	
	// --- In     :
	// --- Out    :
	// --- Returns: if data is valid  
	// --- Effect : This function gets called when control lost focus. User can 
	// ---			validate data typed in the control
	virtual BOOL OnValidate() ;
	
	// --- In     :
	// --- Out    :
	// --- Returns: TRUE if AutoTab mode is set, or FALSE otherwise. 
	// --- Effect : 
	BOOL    GetAutoTab() const ;
	
	// --- In     :	bAutoTab	-	if TRUE then set the control in AutoTab mode,
	//								orthewise set control in Normal mode	
	// --- Out    :
	// --- Returns:
	// --- Effect : Changes the AutoTab mode. If AutoTab mode is set then when 
	//				the last allowed symbol is typed the focus goes to next 
	//				control with WS_TABSTOP style. AutoTab mode is not set by default
	void    SetAutoTab(BOOL bAutoTab) ;

	// --- In     :	nRealPos	-	real position in edit control (taking into account 
	//								all symbols including literals)
	// --- Out    :
	// --- Returns: corresponding logical position (taking into account only 
	//				non-literals) or -1 if real position corresponds to literal. 
	// --- Effect : convert real position within masked edit control to corresponding
	//				logical one
	int    RPtoLP(int nRealPos) const ;
	
	// --- In     :	nLogicalPos	-	logical position in edit control (taking into 
	//								account only non-literals)
	// --- Out    :
	// --- Returns: corresponding real position (taking into account all symbols 
	//				including literals) or -1 if nLogicalPos is not valid logical 
	//				position. 
	// --- Effect : convert logical position within masked edit control to 
	//				corresponding real one
	int    LPtoRP(int nLogicalPos) const ;

// Internal Member functions ------------------------------------------------
protected:
	void    DeleteContents() ;
	
	BOOL    IsInputData(int nPosition) const ;

	int     DeleteRange             (int nSelectionStart, int nSelectionEnd) ;
	int     InsertAt                (int nSelectionStart, TCHAR chNewChar  ) ;
	int     SetAt                   (int nSelectionStart, TCHAR chNewChar  ) ;
	int     GetNextInputLocation    (int nSelectionStart) ;
	int     GetPreviousInputLocation(int nSelectionStart) ;
	int     GetEmptyInputLocation   (int nSelectionStart) ;

	void    Update                      (int nSelectionStart=0) ;  // UpdateInsertionPoint is automatic. 
	void    UpdateInsertionPointForward (int nSelectionStart) ;
	void    UpdateInsertionPointBackward(int nSelectionStart) ;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COXMaskedEdit)
protected:
	virtual void PreSubclassWindow() ;
	//}}AFX_VIRTUAL

	BOOL NotifyParent(UINT nNotificationID);

	
// Generated message map functions
protected:
	//{{AFX_MSG(COXMaskedEdit)
	afx_msg void OnKeyDown (UINT nChar, UINT nRepCnt, UINT nFlags) ;
	afx_msg void OnChar    (UINT nChar, UINT nRepCnt, UINT nFlags) ;
	afx_msg void OnSetFocus(CWnd* pOldWnd) ;
	afx_msg void OnKillfocus();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg LRESULT OnCut(WPARAM wParam, LPARAM lParam) ;
	afx_msg LRESULT OnCopy(WPARAM wParam, LPARAM lParam) ;
	afx_msg LRESULT OnPaste(WPARAM wParam, LPARAM lParam) ;
	afx_msg LRESULT OnClear(WPARAM wParam, LPARAM lParam) ;
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam) ;
	
	DECLARE_MESSAGE_MAP()
};

#endif // __OXMaskedEdit_h__
/////////////////////////////////////////////////////////////////////////////
