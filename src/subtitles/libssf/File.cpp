/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "stdafx.h"
#include "File.h"

#ifndef _istcsym
#define _istcsym(c) (_istalnum(c) || (c) == '_')
#endif

namespace ssf
{
	File::File()
	{
	}

	File::~File()
	{
	}

	void File::Parse(Stream& s, LPCTSTR predef)
	{
		Reference* pRef = CreateRootRef();

		SetDefaultPriority(PLow);

		try {ParseDefs(CharacterStream(predef), pRef);}
		catch(Exception& e) {ASSERT(0); TRACE(_T("%s\n"), e.ToString());}

		SetDefaultPriority(PNormal);

		ParseDefs(s, pRef);

		Commit();

		if(s.PeekChar() != Stream::EOS)
		{
			TRACE(_T("Warning: parsing ended before EOF!\n"));
		}
	}

	void File::ParseDefs(Stream& s, Reference* pParentRef)
	{
		while(s.SkipWhiteSpace(_T(";")) != '}' && s.PeekChar() != Stream::EOS)
		{
			NodePriority priority = PNormal;
			CAtlList<CString> types;
			CString name;

			int c = s.SkipWhiteSpace();

			if(c == '*') {s.GetChar(); priority = PLow;}
			else if(c == '!') {s.GetChar(); priority = PHigh;}

			ParseTypes(s, types);

			if(s.SkipWhiteSpace() == '#')
			{
				s.GetChar();
				ParseName(s, name);
			}

			if(types.IsEmpty())
			{
				if(name.IsEmpty()) s.ThrowError(_T("syntax error"));
				types.AddTail(_T("?"));
			}

			Reference* pRef = pParentRef;

			while(types.GetCount() > 1)
				pRef = CreateRef(CreateDef(pRef, types.RemoveHead()));

			Definition* pDef = NULL;

			if(!types.IsEmpty())
				pDef = CreateDef(pRef, types.RemoveHead(), name, priority);

			c = s.SkipWhiteSpace(_T(":="));

			if(c == '"' || c == '\'') ParseQuotedString(s, pDef);
			else if(_istdigit(c) || c == '+' || c == '-') ParseNumber(s, pDef);
			else if(pDef->IsType(_T("@"))) ParseBlock(s, pDef);
			else ParseRefs(s, pDef);
		}

		s.GetChar();
	}

	void File::ParseTypes(Stream& s, CAtlList<CString>& types)
	{
		types.RemoveAll();

		CString str;

		for(int c = s.SkipWhiteSpace(); _istcsym(c) || c == '.' || c == '@'; c = s.PeekChar())
		{
			c = s.GetChar();

			if(c == '.') 
			{
				if(str.IsEmpty()) s.ThrowError(_T("'type' cannot be an empty string"));
				if(!_istcsym(s.PeekChar())) s.ThrowError(_T("unexpected dot after type '%s'"), str);

				types.AddTail(str);
				str.Empty();
			}
			else
			{
				if(str.IsEmpty() && _istdigit(c)) s.ThrowError(_T("'type' cannot start with a number"));
				if((!str.IsEmpty() || !types.IsEmpty()) && c == '@') s.ThrowError(_T("unexpected @ in 'type'"));

				str += (TCHAR)c;
			}
		}

		if(!str.IsEmpty())
		{
			types.AddTail(str);
		}
	}

	void File::ParseName(Stream& s, CString& name)
	{
		name.Empty();

		for(int c = s.SkipWhiteSpace(); _istcsym(c); c = s.PeekChar())
		{
			if(name.IsEmpty() && _istdigit(c)) s.ThrowError(_T("'name' cannot start with a number"));
			name += (TCHAR)s.GetChar();
		}
	}

	void File::ParseQuotedString(Stream& s, Definition* pDef)
	{
		CString v;

		int quote = s.SkipWhiteSpace();
		if(quote != '"' && quote != '\'') s.ThrowError(_T("expected qouted string"));
		s.GetChar();

		for(int c = s.PeekChar(); c != Stream::EOS; c = s.PeekChar())
		{
			c = s.GetChar();
			if(c == quote) {pDef->SetAsValue(Definition::string, v); return;}
			if(c == '\n') s.ThrowError(_T("qouted string terminated unexpectedly by a new-line character"));
			if(c == '\\') c = s.GetChar();
			if(c == Stream::EOS) s.ThrowError(_T("qouted string terminated unexpectedly by EOS"));
			v += (TCHAR)c;
		}

		s.ThrowError(_T("unterminated quoted string"));
	}

	void File::ParseNumber(Stream& s, Definition* pDef)
	{
		CString v, u;

		for(int c = s.SkipWhiteSpace(); _istxdigit(c) || _tcschr(_T("+-.x:"), c); c = s.PeekChar())
		{
			if((c == '+' || c == '-') && !v.IsEmpty()
			|| (c == '.' && (v.IsEmpty() || v.Find('.') >= 0 || v.Find(_T("x")) >= 0))
			|| (c == 'x' && v != _T("0"))
			|| (c >= 'a' && c <= 'f' || c >= 'A' && c <= 'F') && v.Find(_T("0x")) != 0
			|| (c == ':' && v.IsEmpty()))
			{
				s.ThrowError(_T("unexpected character '%c' in number"), (TCHAR)c);
			}

			v += (TCHAR)s.GetChar();
		}

		if(v.IsEmpty()) s.ThrowError(_T("invalid number"));

		for(int c = s.SkipWhiteSpace(); _istcsym(c); c = s.PeekChar())
		{
			u += (TCHAR)s.GetChar();
		}

		pDef->SetAsNumber(v, u);
	}

	void File::ParseBlock(Stream& s, Definition* pDef)
	{
		CString v;

		int c = s.SkipWhiteSpace(_T(":="));
		if(c != '{') s.ThrowError(_T("expected '{'"));
		s.GetChar();

		int depth = 0;

		for(int c = s.PeekChar(); c != Stream::EOS; c = s.PeekChar())
		{
			c = s.GetChar();
			if(c == '}' && depth == 0) {pDef->SetAsValue(Definition::block, v); return;}
			if(c == '\\') {v += (TCHAR)c; c = s.GetChar();}
			else if(c == '{') depth++;
			else if(c == '}') depth--;
			if(c == Stream::EOS) s.ThrowError(_T("block terminated unexpectedly by EOS"));
			v += (TCHAR)c;
		}

		s.ThrowError(_T("unterminated block"));
	}

	void File::ParseRefs(Stream& s, Definition* pParentDef, LPCTSTR term)
	{
		int c = s.SkipWhiteSpace();

		do
		{
			if(pParentDef->IsValue()) s.ThrowError(_T("cannot mix references with other values"));

			if(c == '{')
			{
				s.GetChar();
				ParseDefs(s, CreateRef(pParentDef));
			}
			else if(_istcsym(c))
			{
				CString str;
				ParseName(s, str);

				// TODO: allow spec references: parent.<type>, self.<type>, child.<type>

				Definition* pDef = GetDefByName(str);
				if(!pDef) s.ThrowError(_T("cannot find definition of '%s'"), str);

				if(!pParentDef->IsVisible(pDef)) s.ThrowError(_T("cannot access '%s' from here"), str);

				pParentDef->AddTail(pDef);
			}
			else if(!_tcschr(term, c) && c != Stream::EOS)
			{
				s.ThrowError(_T("unexpected character '%c'"), (TCHAR)c);
			}

			c = s.SkipWhiteSpace();
		}
		while(!_tcschr(term, c) && c != Stream::EOS);
	}
}
