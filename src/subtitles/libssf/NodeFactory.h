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

#pragma once

#include "Node.h"

namespace ssf
{
	class NodeFactory
	{
		Reference* m_root;
		CAtlStringMap<Node*> m_nodes;
		CAtlList<CString> m_newnodes;
		NodePriority m_priority;

		unsigned __int64 m_counter;
		CString GenName();

	public:
		NodeFactory();
		virtual ~NodeFactory();

		virtual void RemoveAll();

		void SetDefaultPriority(NodePriority priority) {m_priority = priority;}

		void Commit();
		void Rollback();

		Reference* CreateRootRef();
		Reference* GetRootRef() const;
		Reference* CreateRef(Definition* pParentDef);
		Definition* CreateDef(Reference* pParentRef = NULL, CString type = _T(""), CString name = _T(""), NodePriority priority = PNormal);
		Definition* GetDefByName(CString name) const;

		void Dump(NodePriority priority) const;
	};
}
