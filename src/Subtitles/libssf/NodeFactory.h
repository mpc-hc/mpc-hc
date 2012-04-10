/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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

#pragma once

#include "Node.h"

namespace ssf
{
	class NodeFactory
	{
		Reference* m_root;
		StringMapW<Node*> m_nodes;
		CAtlList<CStringW> m_newnodes;
		bool m_predefined;

		unsigned __int64 m_counter;
		CStringW GenName();

	public:
		NodeFactory();
		virtual ~NodeFactory();

		virtual void RemoveAll();

		void SetPredefined(bool predefined) {
			m_predefined = predefined;
		}

		void Commit();
		void Rollback();

		Reference* CreateRootRef();
		Reference* GetRootRef() const;
		Reference* CreateRef(Definition* pParentDef);
		Definition* CreateDef(Reference* pParentRef = NULL, CStringW type = L"", CStringW name = L"", NodePriority priority = PNormal);
		Definition* GetDefByName(CStringW name) const;
		void GetNewDefs(CAtlList<Definition*>& defs);

		void Dump(OutputStream& s) const;
	};
}
