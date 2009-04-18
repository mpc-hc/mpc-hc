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
#include "NodeFactory.h"
#include "Exception.h"

namespace ssf
{
	NodeFactory::NodeFactory()
		: m_counter(0)
		, m_root(NULL)
		, m_predefined(false)
	{
	}

	NodeFactory::~NodeFactory()
	{
		RemoveAll();
	}

	CStringW NodeFactory::GenName()
	{
		CStringW name;
		name.Format(L"%I64d", m_counter++);
		return name;
	}

	void NodeFactory::RemoveAll()
	{
		m_root = NULL;

		POSITION pos = m_nodes.GetStartPosition();
		while(pos) delete m_nodes.GetNextValue(pos);
		m_nodes.RemoveAll();

		m_newnodes.RemoveAll();
	}

	void NodeFactory::Commit()
	{
		m_newnodes.RemoveAll();
	}

	void NodeFactory::Rollback()
	{
		POSITION pos = m_newnodes.GetTailPosition();
		while(pos)
		{
			if(StringMap<Node*, CStringW>::CPair* p = m_nodes.Lookup(m_newnodes.GetPrev(pos)))
			{
				delete p->m_value; // TODO: remove it from "parent"->m_nodes too
				m_nodes.RemoveKey(p->m_key);
			}
		}
	}

	Reference* NodeFactory::CreateRootRef()
	{
		RemoveAll();
		m_root = CreateRef(NULL);
		return m_root;
	}

	Reference* NodeFactory::GetRootRef() const
	{
		ASSERT(m_root);
		return m_root;
	}

	Reference* NodeFactory::CreateRef(Definition* pParentDef)
	{
		CStringW name = GenName();

		Reference* pRef = DNew Reference(this, name);

		m_nodes.SetAt(name, pRef);
		m_newnodes.AddTail(name);

		if(pParentDef)
		{
			pParentDef->AddTail(pRef);
			pRef->m_parent = pParentDef;
		}

		return pRef;
	}

	Definition* NodeFactory::CreateDef(Reference* pParentRef, CStringW type, CStringW name, NodePriority priority)
	{
		Definition* pDef = NULL;

		if(name.IsEmpty())
		{
			name = GenName();
		}
		else 
		{
			pDef = GetDefByName(name);

			if(pDef)
			{
				if(!pDef->m_predefined)
				{
					throw Exception(_T("redefinition of '%s' is not allowed"), CString(name));
				}

				if(!pDef->IsTypeUnknown() && !pDef->IsType(type))
				{
					throw Exception(_T("cannot redefine type of %s to %s"), CString(name), CString(type));
				}
			}
		}

		if(!pDef)
		{
			pDef = DNew Definition(this, name);

			m_nodes.SetAt(name, pDef);
			m_newnodes.AddTail(name);

			if(pParentRef)
			{
				pParentRef->AddTail(pDef);
				pDef->m_parent = pParentRef;
			}
		}

		pDef->m_type = type;
		pDef->m_priority = priority;
		pDef->m_predefined = m_predefined;

		return pDef;
	}

	Definition* NodeFactory::GetDefByName(CStringW name) const
	{
		Node* pNode = NULL;
		m_nodes.Lookup(name, pNode);
		return dynamic_cast<Definition*>(pNode);
	}

	void NodeFactory::GetNewDefs(CAtlList<Definition*>& defs)
	{
		defs.RemoveAll();

		POSITION pos = m_newnodes.GetHeadPosition();
		while(pos)
		{
			if(Definition* pDef = GetDefByName(m_newnodes.GetNext(pos)))
			{
				defs.AddTail(pDef);
			}
		}
	}

	void NodeFactory::Dump(OutputStream& s) const
	{
		if(!m_root) return;

		POSITION pos = m_root->m_nodes.GetHeadPosition();
		while(pos) m_root->m_nodes.GetNext(pos)->Dump(s);
	}
}
