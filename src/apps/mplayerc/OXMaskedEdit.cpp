// Copyright © Dundas Software Ltd. 1997 - 1998, All Rights Reserved

// OXMaskedEdit.cpp : implementation file
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OXMaskedEdit.h"    // COXMaskedEdit

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMaskData

IMPLEMENT_DYNCREATE(CMaskData, CObject)

CMaskData::CMaskData() : 
	m_eType  (MaskDataTypeLITERAL),
	m_chValue(chNULL)
{
}

/////////////////////////////////////////////////////////////////////////////
// CMaskData operations

void CMaskData::operator=(const CMaskData& src)
{
	m_eType=src.m_eType  ;
	m_chValue=src.m_chValue;
}
		
BOOL CMaskData::IsInputData()
{
	BOOL bIsInputData=FALSE;
	switch(m_eType)
	{
	// These are the input types. 
	case MaskDataTypeDIGIT             :
	case MaskDataTypeALPHANUMERIC      :
	case MaskDataTypeALPHABETIC        :
	case MaskDataTypeALPHAETICUPPER    :
	case MaskDataTypeALPHAETICLOWER    :
	case MaskDataTypeCHARACTER         :
		bIsInputData=TRUE ;
		break;
	// These are separators and literals.  They are not input. 
	//case MaskDataTypeDECIMALSEPARATOR  :
	//case MaskDataTypeTHOUSANDSSEPARATOR:
	//case MaskDataTypeTIMESEPARATOR     :
	//case MaskDataTypeDATESEPARATOR     :
	//case MaskDataTypeLITERALESCAPE     :
	//case MaskDataTypeLITERAL           :
	//default:
	//	bIsInputData=FALSE;
	//	break;
	}
	return bIsInputData;
}

BOOL CMaskData::IsValidInput(TCHAR chNewChar)
{
	BOOL bIsValidInput=FALSE;
	switch(m_eType)
	{
	// These are the input types. 
	case MaskDataTypeDIGIT             :
		bIsValidInput=_istdigit(chNewChar);
		break;
	case MaskDataTypeALPHANUMERIC      :
		bIsValidInput=_istalnum(chNewChar);
		break;
	case MaskDataTypeALPHABETIC        :
	case MaskDataTypeALPHAETICUPPER    :
	case MaskDataTypeALPHAETICLOWER    :
		bIsValidInput=_istalpha(chNewChar);
		break;
	case MaskDataTypeCHARACTER         :
		if((chNewChar >=  32) && (chNewChar <= 126))
			bIsValidInput=TRUE ;
		if((chNewChar >= 128) && (chNewChar <= 255))
			bIsValidInput=TRUE ;
		break;
	}
	return bIsValidInput;
}

TCHAR CMaskData::PreProcessChar(TCHAR chNewChar)
{
	TCHAR chProcessedChar=chNewChar;
	switch(m_eType)
	{
	case MaskDataTypeALPHAETICUPPER    :
		chProcessedChar=(TCHAR)_totupper(chNewChar);
		break;
	case MaskDataTypeALPHAETICLOWER    :
		chProcessedChar=(TCHAR)_totlower(chNewChar);
		break;
	}
	return chProcessedChar;
}

#ifdef _DEBUG
void CMaskData::AssertValid() const
{
	CObject::AssertValid();
	ASSERT( (m_eType >= 0) && (m_eType < MASKDATATYPECOUNT));
	ASSERT( m_chValue != chNULL);
}

void CMaskData::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif


/////////////////////////////////////////////////////////////////////////////
// COXMaskedEdit

IMPLEMENT_DYNAMIC(COXMaskedEdit, CEdit)

COXMaskedEdit::COXMaskedEdit(LPCTSTR pszMask/*=NULL*/) : 
	m_bInsertMode(TRUE),
	m_chPromptSymbol(chSPACE),
	m_chIntlDecimal(chPERIOD),
	m_chIntlThousands(chCOMMA),
	m_chIntlTime(chCOLON),
	m_chIntlDate(chSLASH),
	m_bAutoTab(FALSE),
	m_nSetTextSemaphor(0),
	m_bNotifyParent(TRUE)
{
	int nLength;

	nLength=::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, 
		&m_chIntlDecimal, 0);
	if(nLength)
	{
		::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL ,
			&m_chIntlDecimal, nLength);
	}
	nLength=::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, 
		&m_chIntlThousands, 0);
	if(nLength)
	{
		::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, 
			&m_chIntlThousands , nLength);
	}
	nLength=::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIME, &m_chIntlTime, 0);
	if(nLength)
	{
		::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIME, &m_chIntlTime, nLength);
	}
	nLength=::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDATE, &m_chIntlDate, 0);
	if(nLength)
	{
		::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDATE, &m_chIntlDate, nLength);
	}

	SetMask(pszMask);
}

COXMaskedEdit::~COXMaskedEdit()
{
	DeleteContents();
}


BEGIN_MESSAGE_MAP(COXMaskedEdit, CEdit)
	//{{AFX_MSG_MAP(COXMaskedEdit)
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_SETFOCUS()
	ON_CONTROL_REFLECT(EN_KILLFOCUS, OnKillfocus)
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CUT,OnCut)
	ON_MESSAGE(WM_COPY,OnCopy)
	ON_MESSAGE(WM_PASTE,OnPaste)
	ON_MESSAGE(WM_CLEAR,OnClear)
	ON_MESSAGE(WM_SETTEXT,OnSetText)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COXMaskedEdit operations

void COXMaskedEdit::DeleteContents()
{
	if(m_listData.GetCount()==0)
	{
		if(::IsWindow(GetSafeHwnd()))
			SetWindowText(_T(""));
		return;
	}

	CMaskData* pobjData=NULL;
	for(POSITION pos=m_listData.GetHeadPosition(); pos;)
	{
		pobjData=m_listData.GetNext(pos);
		delete pobjData;
		pobjData=NULL;
	}
	m_listData.RemoveAll();
}

CString COXMaskedEdit::GetMask() const
{
	CString csMask;
	CMaskData* pobjData=NULL;
	for(POSITION pos=m_listData.GetHeadPosition(); pos;)
	{
		pobjData=m_listData.GetNext(pos);
		switch(pobjData->m_eType)
		{
		case MaskDataTypeDECIMALSEPARATOR  :  csMask += chMaskPlaceholderDECIMALSEPARATOR  ;  break;
		case MaskDataTypeTHOUSANDSSEPARATOR:  csMask += chMaskPlaceholderTHOUSANDSSEPARATOR;  break;
		case MaskDataTypeTIMESEPARATOR     :  csMask += chMaskPlaceholderTIMESEPARATOR     ;  break;
		case MaskDataTypeDATESEPARATOR     :  csMask += chMaskPlaceholderDATESEPARATOR     ;  break;
		case MaskDataTypeDIGIT             :  csMask += chMaskPlaceholderDIGIT             ;  break;
		case MaskDataTypeALPHANUMERIC      :  csMask += chMaskPlaceholderALPHANUMERIC      ;  break;
		case MaskDataTypeALPHABETIC        :  csMask += chMaskPlaceholderALPHABETIC        ;  break;
		case MaskDataTypeALPHAETICUPPER    :  csMask += chMaskPlaceholderALPHABETICUPPER   ;  break;
		case MaskDataTypeALPHAETICLOWER    :  csMask += chMaskPlaceholderALPHABETICLOWER   ;  break;
		case MaskDataTypeCHARACTER         :  csMask += chMaskPlaceholderCHARACTER         ;  break;
		case MaskDataTypeLITERALESCAPE     :
			// Need to add the escape to things that were escaped. 
			csMask += chMaskPlaceholderLITERALESCAPE;
			csMask += pobjData->m_chValue           ;
			break;
		default:
			// Literals and everything else is kept the same. 
			csMask += pobjData->m_chValue;
			break;
		}
	}
	return csMask;
}

void COXMaskedEdit::SetMask(LPCTSTR pszMask)
{
	if(pszMask==NULL)
	{
		pszMask=_T("");
	}
	DeleteContents();
	
	CMaskData* pobjData=NULL;
	for(LPCTSTR pszInsertionPoint=pszMask; *pszInsertionPoint; pszInsertionPoint++)
	{
		TCHAR chNew=*pszInsertionPoint;
		pobjData=new CMaskData();
		if(pobjData)
		{
			m_listData.AddTail(pobjData);
			switch(chNew)
			{
			case chMaskPlaceholderDECIMALSEPARATOR  :
				pobjData->m_eType  =MaskDataTypeDECIMALSEPARATOR  ;
				pobjData->m_chValue=m_chIntlDecimal               ;
				break;
			case chMaskPlaceholderTHOUSANDSSEPARATOR:
				pobjData->m_eType  =MaskDataTypeTHOUSANDSSEPARATOR;
				pobjData->m_chValue=m_chIntlThousands             ;
				break;
			case chMaskPlaceholderTIMESEPARATOR     :
				pobjData->m_eType  =MaskDataTypeTIMESEPARATOR     ;
				pobjData->m_chValue=m_chIntlTime                  ;
				break;
			case chMaskPlaceholderDATESEPARATOR     :
				pobjData->m_eType  =MaskDataTypeDATESEPARATOR     ;
				pobjData->m_chValue=m_chIntlDate                  ;
				break;
			case chMaskPlaceholderDIGIT             :
				pobjData->m_eType  =MaskDataTypeDIGIT             ;
				pobjData->m_chValue=m_chPromptSymbol              ;
				break;
			case chMaskPlaceholderALPHANUMERIC      :
				pobjData->m_eType  =MaskDataTypeALPHANUMERIC      ;
				pobjData->m_chValue=m_chPromptSymbol              ;
				break;
			case chMaskPlaceholderALPHABETIC        :
				pobjData->m_eType  =MaskDataTypeALPHABETIC        ;
				pobjData->m_chValue=m_chPromptSymbol              ;
				break;
			case chMaskPlaceholderALPHABETICUPPER   :
				pobjData->m_eType  =MaskDataTypeALPHAETICUPPER    ;
				pobjData->m_chValue=m_chPromptSymbol              ;
				break;
			case chMaskPlaceholderALPHABETICLOWER   :
				pobjData->m_eType  =MaskDataTypeALPHAETICLOWER    ;
				pobjData->m_chValue=m_chPromptSymbol              ;
				break;
			case chMaskPlaceholderCHARACTER         :
				pobjData->m_eType  =MaskDataTypeCHARACTER         ;
				pobjData->m_chValue=m_chPromptSymbol              ;
				break;
			case chMaskPlaceholderLITERALESCAPE     :
				// It is the next character that is inserted. 
				pszInsertionPoint++;
				chNew=*pszInsertionPoint;
				if(chNew)
				{
					pobjData->m_eType  =MaskDataTypeLITERALESCAPE     ;
					pobjData->m_chValue=chNew                         ;
					break;
				}
				// If there is no character following the escape, 
				// just treat the escape as a literal so that the user 
				// will see the problem. 
			default:
				// Everything else is just a literal. 
				pobjData->m_eType  =MaskDataTypeLITERAL           ;
				pobjData->m_chValue=chNew                         ;
				break;
			}
		}
	}
	ASSERT(GetMask()==pszMask);

	Update();
}

CString COXMaskedEdit::GetInputData() const
{
	CString csInputData;

	if(m_listData.GetCount()==0)
	{
		GetWindowText(csInputData);
		return csInputData;
	}

	CMaskData* pobjData=NULL;
	for(POSITION pos=m_listData.GetHeadPosition(); pos;)
	{
		pobjData=m_listData.GetNext(pos);
		// Ignore everything that is not data. 
		if(pobjData->IsInputData())
			csInputData += pobjData->m_chValue;
	}
	return csInputData;
}

CString COXMaskedEdit::GetInputData(LPCTSTR lpszText) const
{
	CString csInputData=lpszText;
	int nSymbolCount=csInputData.GetLength();
	CMaskData* pobjData=NULL;
	CString sToExclude;
	int nStartPos=-1;
	int nEndPos=-1;
	int nIndex=0;
	int nRemovedCount=0;
	for(POSITION pos=m_listData.GetHeadPosition(); pos;)
	{
		pobjData=m_listData.GetNext(pos);
		if(!pobjData->IsInputData())
		{
			if(nStartPos==-1)
			{
				nStartPos=nIndex;
				sToExclude.Empty();
			}
			sToExclude+=pobjData->m_chValue;
		}
		else
		{
			if(nStartPos!=-1)
			{
				nEndPos=nIndex-1;
				if(csInputData.Mid(nStartPos-nRemovedCount,
					nEndPos-nStartPos+1)==sToExclude)
				{
					csInputData=csInputData.Left(nStartPos-nRemovedCount)+
						csInputData.Mid(nEndPos-nRemovedCount+1);
					nRemovedCount+=nEndPos-nStartPos+1;
				}
				nStartPos=-1;
			}
		}

		nIndex++;
		if(nIndex>=nSymbolCount)
			break;
	}

	return csInputData;
}

BOOL COXMaskedEdit::SetInputData(LPCTSTR pszInputData, int nBeginPos/*=0*/, 
								 BOOL bAllowPrompt/*=TRUE*/)
{
	CString csFullInput;
	// Start with existing data and append the new data. 
	csFullInput=GetInputData();
	csFullInput=csFullInput.Left(nBeginPos);
	if(bAllowPrompt)
	{
		csFullInput+=pszInputData;
	}
	else
	{
		// If the prompt symbol is not valid, then 
		// add the data one-by-one ignoring any prompt symbols. 
		for(; *pszInputData; pszInputData++)
		{
			if(*pszInputData!=m_chPromptSymbol)
				csFullInput+=*pszInputData;
		}
	}
	
	BOOL bCompleteSuccess=TRUE;
	LPCTSTR pszReplaceData=csFullInput;
	CMaskData* pobjData=NULL;
	for(POSITION pos=m_listData.GetHeadPosition(); pos;)
	{
		pobjData=m_listData.GetNext(pos);
		// Ignore everything that is not data. 
		if(pobjData->IsInputData())
		{
			// If we run out of replacement data, then use the prompt symbol. 
			// Make sure we iterate through the entire list so that the 
			// prompt symbol is applied to any empty areas. 
			if(*pszReplaceData)
			{
				// This inner while loop is so that we can re-apply input data 
				// after an error.  This will allow us to skip over invalid 
				// input data and try the next character. 
				while(*pszReplaceData)
				{
					TCHAR chReplace=*pszReplaceData;
					pszReplaceData++;
					
					// Make sure to follow the input validation. 
					// The prompt symbol is always valid at this level. 
					// This allows the user to erase a string by overtyping a space. 
					// On error, just skip the character being inserted. 
					// This will allow the DeleteRange() function to have the remaining 
					// characters validated. 
					if((chReplace==m_chPromptSymbol) || pobjData->IsValidInput(chReplace))
					{
						pobjData->m_chValue=pobjData->PreProcessChar(chReplace);
						break;
					}
					else
						bCompleteSuccess=FALSE;
				}
			}
			else
			{
				pobjData->m_chValue=m_chPromptSymbol;
			}
		}
	}
	
	Update();

	return bCompleteSuccess;
}

TCHAR COXMaskedEdit::GetPromptSymbol()
{
	return m_chPromptSymbol;
}

void COXMaskedEdit::SetPromptSymbol(TCHAR chNewPromptSymbol)
{
	// The prompt symbol must be a valid edit box symbol. 
	ASSERT( (chNewPromptSymbol != chNULL) && (chNewPromptSymbol != chCR) && (chNewPromptSymbol != chLF) && (chNewPromptSymbol != 127));

	if((chNewPromptSymbol != chNULL) && (chNewPromptSymbol != chCR) && (chNewPromptSymbol != chLF) && (chNewPromptSymbol != 127))
	{
		// Just for the heck of it, if the prompt symbol changes, 
		// go through and replace the existing prompts with the new prompt. 
		CMaskData* pobjData=NULL;
		for(POSITION pos=m_listData.GetHeadPosition(); pos;)
		{
			pobjData=m_listData.GetNext(pos);
			if(pobjData->IsInputData())
			{
				if(pobjData->m_chValue==m_chPromptSymbol)
					pobjData->m_chValue=chNewPromptSymbol;
			}
		}
		m_chPromptSymbol=chNewPromptSymbol;
	}
	
	// Don't update the insertion point if just setting the prompt symbol. 
	Update(-1);
}

void COXMaskedEdit::EmptyData(BOOL bOnlyInput/*=FALSE*/)
{
	if(m_listData.GetCount()==0)
	{
		DeleteContents();
		return;
	}

	if(bOnlyInput)
	{
		// If emptying only the data, the iterate through the list 
		// of data and replace input data with the prompt symbol. 
		CMaskData* pobjData=NULL;
		for(POSITION pos=m_listData.GetHeadPosition(); pos;)
		{
			pobjData=m_listData.GetNext(pos);
			if(pobjData->IsInputData())
				pobjData->m_chValue=m_chPromptSymbol;
		}
	}
	else
		DeleteContents();
	
	Update();
}

BOOL COXMaskedEdit::IsInputEmpty()
{
	if(m_listData.GetCount()==0)
	{
		CString csInputData;
		GetWindowText(csInputData);
		return csInputData.IsEmpty();
	}

	CMaskData* pobjData=NULL;
	for(POSITION pos=m_listData.GetHeadPosition(); pos;)
	{
		pobjData=m_listData.GetNext(pos);
		if (pobjData->IsInputData() && pobjData->m_chValue!=m_chPromptSymbol)
			return FALSE;
	}
	return TRUE;
}

BOOL COXMaskedEdit::GetInsertMode() const
{
	// The standard CEdit control does not support over-typing. 
	// This flag is used to manage over-typing internally. 
	return m_bInsertMode;
}

void COXMaskedEdit::SetInsertMode(BOOL bInsertMode)
{
	// The standard CEdit control does not support over-typing. 
	// This flag is used to manage over-typing internally. 
	m_bInsertMode=bInsertMode;
}

BOOL COXMaskedEdit::GetAutoTab() const
{
	// The standard CEdit control does not support AutoTab mode. 
	// This flag is used to manage AutoTab mode internally. 
	return m_bAutoTab;
}

void COXMaskedEdit::SetAutoTab(BOOL bAutoTab)
{
	// The standard CEdit control does not support AutoTab mode. 
	// This flag is used to manage AutoTab mode internally. 
	m_bAutoTab=bAutoTab;
}

CString COXMaskedEdit::ShowMask() const
{
	CString csShow;
	CMaskData* pobjData=NULL;
	for(POSITION pos=m_listData.GetHeadPosition(); pos;)
	{
		pobjData=m_listData.GetNext(pos);
		ASSERT_VALID( pobjData);
		
		// There is no need to do any fancy string building because 
		// all validation is done when characters are inserted into the list. 
		// Literals and placeholders are converted properly at that time 
		// so all we have to do here is get the value. 
		csShow += pobjData->m_chValue;
	}
	return csShow;
}

BOOL COXMaskedEdit::IsInputData(int nPosition) const
{
	if(m_listData.GetCount()==0)
	{
		return TRUE;
	}

	// We frequently need to know if a position refers to 
	// input data or to a literal. 
	BOOL bIsInputData=FALSE;
	if(nPosition >= 0)
	{
		POSITION pos=m_listData.FindIndex(nPosition);
		if(pos)
		{
			CMaskData* pobjData=m_listData.GetAt(pos);
			if(pobjData)
			{
				bIsInputData=pobjData->IsInputData();
			}
		}
	}
	return bIsInputData;
}

int COXMaskedEdit::DeleteRange(int nSelectionStart, int nSelectionEnd)
{
	// In order to delete properly, we must count the number of 
	// input characters that are selected and only delete that many. 
	// This is because the selection can include literals. 
	int nCharIndex  =0;
	int nDeleteCount=0;
	CString csInputData;
	CMaskData* pobjData=NULL;
	for(POSITION pos=m_listData.GetHeadPosition(); pos; nCharIndex++)
	{
		pobjData=m_listData.GetNext(pos);
		// Ignore everything that is not data. 
		// This is just like we do in GetInputData except that we 
		// will ignore the input data within the selection range. 
		if(pobjData->IsInputData())
		{
			if((nCharIndex < nSelectionStart) || (nCharIndex >= nSelectionEnd))
			{
				// The SetInputData() function will take care of validating 
				// the shifted characters. 
				csInputData += pobjData->m_chValue;
			}
			else
				nDeleteCount++;
		}
	}
	// Now apply the filtered data stream. 
	SetInputData(csInputData);
	// return the deleted count so that an error can be generated 
	// if none were deleted. 
	return nDeleteCount;
}

int COXMaskedEdit::InsertAt(int nSelectionStart, TCHAR chNewChar)
{
	// Although we could have some complex, yet efficient, routine 
	// that would error if inserting pushed an existing character 
	// into an invalid region.  Instead, just save the current 
	// state and restore it on error. 
	CString csPreviousInput=GetInputData();

	int nCharIndex=0;
	int nInsertionPoint=-1;
	CString csInputData;
	CMaskData* pobjData=NULL;
	for(POSITION pos=m_listData.GetHeadPosition(); pos; nCharIndex++)
	{
		pobjData=m_listData.GetNext(pos);
		// Ignore everything that is not data. 
		// This is just like we do in GetInputData except that we 
		// will ignore the input data within the selection range. 
		if(pobjData->IsInputData())
		{
			// Wait until a valid insertion point and 
			// only make sure to insert once. 
			if((nInsertionPoint < 0) && (nCharIndex >= nSelectionStart))
			{
				csInputData += chNewChar;
				nInsertionPoint=nCharIndex;
			}
			csInputData += pobjData->m_chValue;
		}
	}
	// Now apply the filtered data stream and check if it was successful. 
	if(!SetInputData(csInputData))
	{
		// If not successful, then restore the previous input and return -1. 
		SetInputData(csPreviousInput);
		return -1;
	}
	return nInsertionPoint;
}

int COXMaskedEdit::SetAt(int nSelectionStart, TCHAR chNewChar)
{
	if(nSelectionStart >= 0)
	{
		POSITION pos=m_listData.FindIndex(nSelectionStart);
		if(pos)
		{
			CMaskData* pobjData=m_listData.GetAt(pos);
			if(pobjData)
			{
				if(pobjData->IsInputData())
				{
					if((chNewChar==m_chPromptSymbol) || pobjData->IsValidInput(chNewChar))
						pobjData->m_chValue=pobjData->PreProcessChar(chNewChar);
					else
						return -1;  // Input value is invalid or not allowed. 
				}
			}
		}
	}
	return nSelectionStart;
}

int COXMaskedEdit::GetNextInputLocation(int nSelectionStart)
{
	// One of the functions of this edit control is that it skips over literals. 
	// We need a function to help skip to the next position. 
	int nNextInputLocation=nSelectionStart;
	if(nNextInputLocation < 0)
		nNextInputLocation=0;
	
	CMaskData* pobjData=NULL;
	for(POSITION pos=m_listData.FindIndex(nNextInputLocation); pos; nNextInputLocation++)
	{
		pobjData=m_listData.GetNext(pos);
		if(pobjData->IsInputData())
		{
			break;
		}
	}

	return nNextInputLocation;
}

int COXMaskedEdit::GetPreviousInputLocation(int nSelectionStart)
{
	// One of the functions of this edit control is that it skips over literals. 
	// We need a function to help skip to the next position. 
	int nNextInputLocation=nSelectionStart;
	if(nNextInputLocation < 0)
		nNextInputLocation=0;
	// Need to determine if we moved to a previous location. 
	// There will need to be some correction. 
	int nInitialInputLocation=nNextInputLocation;
	
	CMaskData* pobjData=NULL;
	for(POSITION pos=m_listData.FindIndex(nNextInputLocation); pos; nNextInputLocation--)
	{
		pobjData=m_listData.GetPrev(pos);
		if(pobjData->IsInputData())
		{
			if(nInitialInputLocation != nNextInputLocation)
			{
				// If we find a valid previous location, then move to the right of it. 
				// This backup, then move forward is typical when seeking backward. 
				nNextInputLocation++;
			}
			break;
		}
	}
	// If there is no input data to the left of the selection, 
	// then seek forward to the next location. 
	if(nNextInputLocation < 0)
		return GetNextInputLocation(nSelectionStart);
	return nNextInputLocation;
}

int COXMaskedEdit::GetEmptyInputLocation(int nSelectionStart)
{
	int nEmptyInputLocation=nSelectionStart;
	if(nEmptyInputLocation < 0)
		nEmptyInputLocation=0;
	
	CMaskData* pobjData=NULL;
	for(POSITION pos=m_listData.FindIndex(nEmptyInputLocation); pos; nEmptyInputLocation++)
	{
		pobjData=m_listData.GetNext(pos);
		if(pobjData->IsInputData())
		{
			if(pobjData->m_chValue==m_chPromptSymbol)
				break;
		}
	}
	return nEmptyInputLocation;
}

void COXMaskedEdit::Update(int nSelectionStart/*=0*/)
{
	// Update the edit control if it exists. 
	if(::IsWindow(m_hWnd))
	{
		m_nSetTextSemaphor++;
		CString sText=ShowMask();
		SetWindowText(sText);
		m_nSetTextSemaphor--;
		// We usually need to update the insertion point. 
		if(nSelectionStart >= 0)
			UpdateInsertionPointForward(nSelectionStart);
	}
}

void COXMaskedEdit::UpdateInsertionPointForward(int nSelectionStart)
{
	int nNewInsertionPoint=GetNextInputLocation(nSelectionStart);

	if(m_bAutoTab && nNewInsertionPoint==m_listData.GetCount())
	{
		CWnd* pParentWnd=GetParent();
		ASSERT(pParentWnd);
		CWnd* pNextTabCtrl=pParentWnd->GetNextDlgTabItem(this);
		if(pNextTabCtrl && pNextTabCtrl!=this)
		{
			pNextTabCtrl->SetFocus();
		}
	}
	else
	{
		SetSel(nNewInsertionPoint, nNewInsertionPoint);
	}
}

void COXMaskedEdit::UpdateInsertionPointBackward(int nSelectionStart)
{
	int nNewInsertionPoint=GetPreviousInputLocation(nSelectionStart);
	SetSel(nNewInsertionPoint, nNewInsertionPoint);
}

void COXMaskedEdit::ValidationError()
{
	::MessageBeep(MB_ICONEXCLAMATION);
}

/////////////////////////////////////////////////////////////////////////////
// COXMaskedEdit overrides

void COXMaskedEdit::PreSubclassWindow() 
{
	CEdit::PreSubclassWindow();
	// As of 01/07/98, this masked edit control was only designed 
	// to handle single lines.  At some point, the code can be reviewed 
	// to see if it can handle multiple lines. 
	ASSERT( !(GetStyle() & ES_MULTILINE));
	
	// This is a great place to update the control as it is 
	// the first function called after a successful subclass. 
	// Don't update if there is no data. 
	if(m_listData.GetCount() != 0)
		Update();
}

/////////////////////////////////////////////////////////////////////////////
// COXMaskedEdit message handlers

BOOL COXMaskedEdit::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	// We override the create function so that we can update the text 
	// if a mask was provided in the constructor. 
	BOOL bReturn=CEdit::Create(dwStyle, rect, pParentWnd, nID);
	if(bReturn)
	{
		// Don't update if there is no data. 
		if(m_listData.GetCount() != 0)
			Update();
	}
	return bReturn;
}

void COXMaskedEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	TRACE(_T("COXMaskedEdit::OnKeyDown(\'%c\' (0x%02x), %d, 0x%04x)\n"), nChar, nChar, nRepCnt, nFlags);
	// If there is no mask, then exit quickly performing the default operation. 
	if(m_listData.GetCount()==0)
	{
		CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}

	// Keep the OnKeyDown processing to a minimum.  This is because the edit 
	// control does lots of processing before OnChar() is sent and we want 
	// to let it continue. 
	BOOL bIsShiftKeyDown=::GetAsyncKeyState(VK_SHIFT)< 0;
	if(nChar==VK_DELETE)
	{
		int nSelectionStart=0;
		int nSelectionEnd  =0;
		GetSel(nSelectionStart, nSelectionEnd);
		TRACE(_T("  %2d, %2d  Before\n"), nSelectionStart, nSelectionEnd);

		// Delete has two functions, it can delete the selection and
		// it can delete characters to the right.
		if(nSelectionStart==nSelectionEnd)
		{
			nSelectionEnd++; // Do the equivalent of a selection.
			if(DeleteRange(nSelectionStart, nSelectionEnd))
			{
				Update(nSelectionStart);
			}
			else	// Must be on a literal, so continue moving to right
					// and re-attempting the delete until we either delete
					// a character or run out of characters.
			{
				while (nSelectionEnd != m_listData.GetCount())
				{
					nSelectionStart++;
					nSelectionEnd++; // Do the equivalent of a selection.
					if(DeleteRange(nSelectionStart, nSelectionEnd))
					{
						Update(nSelectionStart);
						break;
					}
				}
			}
		}
		else if(DeleteRange(nSelectionStart, nSelectionEnd))
		{
			Update(nSelectionStart);
		}
		else	// Must be on a literal, so continue moving to right
				// and reattempting the delete until we either delete
				// a character or run out of characters.
		{
			while (nSelectionEnd != m_listData.GetCount())
			{
				nSelectionStart++;
				nSelectionEnd++; // Do the equivalent of a selection.
				if(DeleteRange(nSelectionStart, nSelectionEnd))
				{
					Update(nSelectionStart);
					break;
				}
			}
		}
	}
	else if(nChar==VK_HOME)
	{
		// If the shift key is not down, then HOME is a navigation and we need to 
		// move the insertion point to the first available position. 
		CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
		if(!bIsShiftKeyDown)
		{
			int nSelectionStart=0;
			int nSelectionEnd  =0;
			GetSel(nSelectionStart, nSelectionEnd);
			TRACE(_T("  %2d, %2d  Before\n"), nSelectionStart, nSelectionEnd);
			
			UpdateInsertionPointForward(nSelectionStart);
		}
	}
	else if(nChar==VK_LEFT)
	{
		// If the shift key is not down, then LEFT is a navigation and we need to 
		// move the insertion point to the previous available position. 
		CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
		if(!bIsShiftKeyDown)
		{
			int nSelectionStart=0;
			int nSelectionEnd  =0;
			GetSel(nSelectionStart, nSelectionEnd);
			TRACE(_T("  %2d, %2d  Before\n"), nSelectionStart, nSelectionEnd);
			
			UpdateInsertionPointBackward(nSelectionStart);
		}
	}
	else if(nChar==VK_UP)
	{
		// If the shift key is not down, then UP is a navigation and we need to 
		// move the insertion point to the previous available position. 
		CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
		if(!bIsShiftKeyDown)
		{
			int nSelectionStart=0;
			int nSelectionEnd  =0;
			GetSel(nSelectionStart, nSelectionEnd);
			TRACE( _T("  %2d, %2d  Before\n"), nSelectionStart, nSelectionEnd);
			
			UpdateInsertionPointBackward(nSelectionStart);
		}
	}
	else if(nChar==VK_RIGHT)
	{
		// If the shift key is not down, then RIGHT is a navigation and we need to 
		// move the insertion point to the next available position. 
		CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
		if(!bIsShiftKeyDown)
		{
			int nSelectionStart=0;
			int nSelectionEnd  =0;
			GetSel(nSelectionStart, nSelectionEnd);
			TRACE( _T("  %2d, %2d  Before\n"), nSelectionStart, nSelectionEnd);
			
			UpdateInsertionPointForward(nSelectionStart);
		}
	}
	else if(nChar==VK_DOWN)
	{
		// If the shift key is not down, then DOWN is a navigation and we need to 
		// move the insertion point to the next available position. 
		CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
		if(!bIsShiftKeyDown)
		{
			int nSelectionStart=0;
			int nSelectionEnd  =0;
			GetSel(nSelectionStart, nSelectionEnd);
			TRACE(_T("  %2d, %2d  Before\n"), nSelectionStart, nSelectionEnd);
			
			UpdateInsertionPointForward(nSelectionStart);
		}
	}
	else if(nChar==VK_INSERT)
	{
	// The standard CEdit control does not support over-typing. 
	// This flag is used to manage over-typing internally. 
		BOOL bOldInsertMode=GetInsertMode();
		BOOL bNewInsertMode=bOldInsertMode ? FALSE : TRUE;
		
		SetInsertMode(bNewInsertMode);
	}
	else
	{
		CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

void COXMaskedEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	TRACE(_T("COXMaskedEdit::OnChar(\'%c\' (%d), %d, 0x%04x)\n"), 
		nChar, nChar, nRepCnt, nFlags);
	// If there is no mask, then exit quickly performing the default operation. 
	if(m_listData.GetCount()==0)
	{
		CEdit::OnChar(nChar, nRepCnt, nFlags);
		return;
	}
	
	int nSelectionStart=0;
	int nSelectionEnd  =0;
	GetSel(nSelectionStart, nSelectionEnd);
	TRACE(_T("  %2d, %2d  Before\n"), nSelectionStart, nSelectionEnd);
	
	// If character value is above 32, then it is ANSI or Extended. 
	// Below 32 are control and navigation characters. 
	if(nChar >= 32)
	{
		if(nSelectionStart==nSelectionEnd)
		{
			if(IsInputData(nSelectionStart))
			{
				int nActualInsertionPoint=nSelectionStart;
				if(m_bInsertMode)
					nActualInsertionPoint=InsertAt(nSelectionStart, (TCHAR)nChar);
				else
					nActualInsertionPoint=SetAt   (nSelectionStart, (TCHAR)nChar);
				
				// InsertAt will return -1 if the character cannot be inserted here. 
				if(nActualInsertionPoint >= 0)
					nSelectionStart=nActualInsertionPoint + 1;
				else
					ValidationError();
				
				Update(nSelectionStart);
			}
			else
			{
				// Beep if trying to type over a literal. 
				ValidationError();
				UpdateInsertionPointForward(nSelectionStart);
			}
		}
		else
		{
			// First delete the remaining selection. 
			// The function will return a valid count if 
			// some input characters were deleted. We use 
			// this value to determine if it makes sense to insert. 
			if(DeleteRange(nSelectionStart, nSelectionEnd))
			{
				// InsertAt will place the character at the next available position, 
				// then return that positition
				int nActualInsertionPoint=nSelectionStart;
				nActualInsertionPoint=InsertAt(nSelectionStart, (TCHAR)nChar);
				
				// InsertAt will return -1 if the character cannot be inserted here. 
				if(nActualInsertionPoint >= 0)
					nSelectionStart=nActualInsertionPoint + 1;
				else
					ValidationError();
				
				Update(nSelectionStart);
			}
			else  // Must be on a literal, so beep and move to a valid location. 
			{
				ValidationError();
				UpdateInsertionPointForward(nSelectionStart);
			}
		}
	}
	else
	{
		if(nChar==VK_BACK)
		{
			// Backspace performs two functions, if there is a selection,
			// then the backspace is the same as deleting the selection.
			// If there is no selection, then the backspace deletes the
			// first non-literal character to the left.
			if(nSelectionStart==nSelectionEnd)
			{
				if (nSelectionStart >= 1)
				{
					while (nSelectionStart>=0)
					{
						nSelectionStart--; // Do the equivalent of a backspace.

						if (DeleteRange(nSelectionStart, nSelectionEnd))
						{
							Update(nSelectionStart);
							break;
						}

						nSelectionEnd--;
					}
				}
			}
			else if(DeleteRange(nSelectionStart, nSelectionEnd))
			{
				Update(nSelectionStart);
			}
			else	// Must be on a literal, so continue moving to left
					// and re-attempting the delete until we either delete
					// a character or run out of characters.
			{
				if (nSelectionStart >= 1)
				{
					while (nSelectionStart>=0)
					{
						nSelectionStart--; // Do the equivalent of a backspace.

						if (DeleteRange(nSelectionStart, nSelectionEnd))
						{
							Update(nSelectionStart);
							break;
						}
						
						nSelectionEnd--;
					}
				}
			}
		}
		else
			// let edit control to do its job 
			CEdit::OnChar(nChar, nRepCnt, nFlags);
	}
}

void COXMaskedEdit::OnSetFocus(CWnd* pOldWnd) 
{
	TRACE(_T("COXMaskedEdit::OnSetFocus()\n"));
	CEdit::OnSetFocus(pOldWnd);
	// The default behavior is to highlight the entire string. 
	// If this is the case, then move the insertion to the first input position. 
	int nSelectionStart=0;
	int nSelectionEnd  =0;
	GetSel(nSelectionStart, nSelectionEnd);
	TRACE(_T("  %2d, %2d  Before\n"), nSelectionStart, nSelectionEnd);
	if((nSelectionStart==0) && (nSelectionEnd==GetWindowTextLength()))
	{
		// Only update the insertion point if the entire string is selected. 
		// This will allow the mouse to be used to set the cursor without our interfering. 
		UpdateInsertionPointForward(0);
	}
}


LONG COXMaskedEdit::OnCut(UINT wParam, LONG lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	TRACE(_T("COXMaskedEdit::OnCut\n"));

	if(m_listData.GetCount()==0)
	{
		return CEdit::Default();
	}

	int nSelectionStart=0;
	int nSelectionEnd  =0;
	GetSel(nSelectionStart, nSelectionEnd);

	// First do our version of the cut. 
	int nDeleteCount=DeleteRange(nSelectionStart, nSelectionEnd);

	// Before updating, let the control do its normal thing. 
	// This will save us the effort of filling the clipboard. 
	CEdit::Default();

	// Now we update with our standard mask. 
	Update(nSelectionStart);
	if(nDeleteCount==0)
	{
		// I don't think we want to beep if no input characters were cut. 
		//ValidationError();
	}

	return 0;
}

LONG COXMaskedEdit::OnCopy(UINT wParam, LONG lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	TRACE(_T("COXMaskedEdit::OnCopy\n"));

	// Just let copy do its thing and copy the selected text. 
	CEdit::Default();

	return 0;
}

LONG COXMaskedEdit::OnPaste(UINT wParam, LONG lParam) 
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	TRACE(_T("COXMaskedEdit::OnPaste()\n"));

	if(m_listData.GetCount()==0)
	{
		return CEdit::Default();
	}

	int nSelectionStart=0;
	int nSelectionEnd  =0;
	GetSel(nSelectionStart, nSelectionEnd);

	CEdit::Default();

	// This is a real dump paste routine that expects SetInputData do 
	// do the filtering.  There is probably no easy solution to this 
	// task because anything can be pasted.  We could try and match 
	// the literals, but maybe we will get to that later. 
	CString csNewString;
	GetWindowText(csNewString);
	// It is very important that we do not allow the prompt character 
	// in this scenario.  This is because we expect the pasted text 
	// to contain lots of literals and spaces. 
	SetInputData(csNewString, 0, FALSE);
	Update(-1);
	// Setting the insertion point after a paste is tricky because the 
	// expected location is after the last valid pasted character. 
	// Try and determine this location by setting the insertion point 
	// to the first empty location after the specified starting point. 
	int nNewInsertionPoint=GetEmptyInputLocation(nSelectionStart);
	SetSel(nNewInsertionPoint, nNewInsertionPoint);

	return 0;
}

LONG COXMaskedEdit::OnClear(UINT wParam, LONG lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	TRACE(_T("COXMaskedEdit::OnClear\n"));

	if(m_listData.GetCount()==0)
	{
		return CEdit::Default();
	}

	int nSelectionStart=0;
	int nSelectionEnd=0;
	GetSel(nSelectionStart, nSelectionEnd);

	// First do our version of the cut. 
	int nDeleteCount=DeleteRange(nSelectionStart, nSelectionEnd);

	// Before updating, let the control do its normal thing. 
	CEdit::Default();

	// Now we update with our standard mask. 
	Update(nSelectionStart);
	if(nDeleteCount==0)
	{
		// I don't think we want to beep if no input characters were cut. 
		//ValidationError();
	}

	return 0;
}


void COXMaskedEdit::OnKillfocus() 
{
	// TODO: Add your control notification handler code here

	// send OXMEN_VALIDATE notification to parent to validate typed information
	// if the notification was handled, the return value have to be one of these:
	//
	//		-1	-	if typed info is invalid
	//		0	-	typed info is valid but virtual OnValidate function will be 
	//				called to verify typed info
	//		1	-	typed info is valid and OnValidate function won't be called

	CWnd* pParentWnd=GetParent();
	ASSERT(pParentWnd);

	MENMHDR MENMHdr;
	memset(&MENMHdr,0,sizeof(MENMHdr));
	MENMHdr.hdr.hwndFrom=GetSafeHwnd();
	MENMHdr.hdr.idFrom=GetDlgCtrlID();
	MENMHdr.hdr.code=OXMEN_VALIDATE;
	MENMHdr.bValid=TRUE;
	MENMHdr.bDefaultValidation=TRUE;
	MENMHdr.nPosition=0;
	
	pParentWnd->SendMessage(WM_NOTIFY,MENMHdr.hdr.idFrom,(LPARAM)&MENMHdr);

	if(!MENMHdr.bValid || !(MENMHdr.bDefaultValidation ? OnValidate() : TRUE))
	{
		SetFocus();
		ValidationError();
		// set insertion point at the first input location
		UpdateInsertionPointForward(MENMHdr.nPosition);
	}
}

BOOL COXMaskedEdit::OnValidate() 
{
	// by default return TRUE
	// one can overwrite this function to provide validation capability
	// in COXMaskedEdit derived class
	return TRUE;
}

void COXMaskedEdit::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(::GetFocus()!=GetSafeHwnd() && IsInputEmpty())
	{
		SetFocus();
		UpdateInsertionPointForward(0);
	}
	else
	{
		CEdit::OnLButtonDown(nFlags,point);
	}
}

LONG COXMaskedEdit::OnSetText(UINT wParam, LONG lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	if(m_listData.GetCount()==0)
	{
		return CEdit::Default();
	}

	CString csNewString=(LPCTSTR)lParam;
	if(m_nSetTextSemaphor>0)
	{
		LONG result=CEdit::Default();
		NotifyParent(EN_UPDATE);
		if(m_bNotifyParent)
			NotifyParent(EN_CHANGE);
		return result;
	}
	else
	{
		ASSERT(m_nSetTextSemaphor==0);
		m_bNotifyParent=FALSE;
		csNewString=GetInputData(csNewString);
		SetInputData(csNewString,0,TRUE);
		m_bNotifyParent=TRUE;
		return TRUE;
	}
}

int COXMaskedEdit::RPtoLP(int nRealPos) const 
{
	// All COXMaskedEdit functions that take cusor position as argument interpret it
	// as real position within edit control (taking into account all symbols including 
	// literals). But sometimes we want to know which non-literal symbol is at  
	// particular real position. In that case this function is really useful

	if(nRealPos<0 || nRealPos>=m_listData.GetCount())
		return -1;
	
	int nLogicalPos=-1;
	CMaskData* pobjData=NULL;
	int nNextInputLocation=0;
	for(POSITION pos=m_listData.FindIndex(nNextInputLocation); pos; nNextInputLocation++)
	{
		pobjData=m_listData.GetNext(pos);
		if(pobjData->IsInputData())
		{
			nLogicalPos++;
		}
		if(nNextInputLocation==nRealPos)
		{
			return pobjData->IsInputData() ? nLogicalPos : -1;
		}
	}

	return -1;
}
		
int COXMaskedEdit::LPtoRP(int nLogicalPos) const 
{
	// All COXMaskedEdit functions that take cusor position as argument interpret it
	// as real position within edit control (taking into account all symbols including 
	// literals). But sometimes we want to set cursor at position before or after 
	// particular non-literal symbol. In that case this function is really useful

	if(nLogicalPos<0 || nLogicalPos>=m_listData.GetCount())
		return -1;
	
	int nRealPos=-1;
	int nNonLiterals=-1;
	CMaskData* pobjData=NULL;
	int nNextInputLocation=0;
	for(POSITION pos=m_listData.FindIndex(nNextInputLocation); pos; nNextInputLocation++)
	{
		pobjData=m_listData.GetNext(pos);
		nRealPos++;
		if(pobjData->IsInputData())
		{
			nNonLiterals++;
			if(nNonLiterals==nLogicalPos)
			{
				return nRealPos;
			}
		}
	}

	return -1;
}


BOOL COXMaskedEdit::NotifyParent(UINT nNotificationID)
{
	CWnd* pParentWnd=GetParent();
	if(pParentWnd==NULL)
		return FALSE;

	pParentWnd->SendMessage(WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(),nNotificationID),
		(LPARAM)GetSafeHwnd());
	return TRUE;
}


