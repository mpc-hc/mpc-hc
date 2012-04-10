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

#include "Stream.h"
#include "StringMap.h"

namespace ssf
{
	class Definition;
	class NodeFactory;

	enum NodePriority {PLow, PNormal, PHigh};

	class Node
	{
	protected:
		NodeFactory* m_pnf;

	public:
		Node* m_parent;
		CAtlList<Node*> m_nodes;
		StringMapW<Node*> m_name2node;
		CStringW m_type, m_name;
		NodePriority m_priority;
		bool m_predefined;

		Node(NodeFactory* pnf, CStringW name);
		virtual ~Node() {}

		bool IsNameUnknown();
		bool IsTypeUnknown();
		bool IsType(CStringW type);

		virtual void AddTail(Node* pNode);
		virtual void GetChildDefs(CAtlList<Definition*>& l, LPCWSTR type = NULL, bool fFirst = true);
		virtual void Dump(OutputStream& s, int level = 0, bool fLast = false) = 0;
	};

	class Reference : public Node
	{
	public:
		Reference(NodeFactory* pnf, CStringW name);
		virtual ~Reference();

		void GetChildDefs(CAtlList<Definition*>& l, LPCWSTR type = NULL, bool fFirst = true);
		void Dump(OutputStream& s, int level = 0, bool fLast = false);
	};

	class Definition : public Node
	{
	public:
		template<typename T> struct Number {
			T value;
			int sign;
			CStringW unit;
		};
		struct Time {
			Number<float> start, stop;
		};

		enum status_t {node, string, number, boolean, block};

	private:
		status_t m_status;
		bool m_autotype;
		CStringW m_value, m_unit;
		Number<float> m_num;
		CStringW m_num_string;

		StringMapW<Definition*> m_type2def;
		void RemoveFromCache(LPCWSTR type = NULL);

		template<typename T>
		void GetAsNumber(Number<T>& n, StringMapW<T>* n2n = NULL);

	public:
		Definition(NodeFactory* pnf, CStringW name);
		virtual ~Definition();

		bool IsVisible(Definition* pDef);

		void AddTail(Node* pNode);
		void Dump(OutputStream& s, int level = 0, bool fLast = false);

		Definition& operator[] (LPCWSTR type);

		bool IsValue(status_t s = (status_t)0);

		void SetAsValue(status_t s, CStringW v, CStringW u = L"");
		void SetAsNumber(CStringW v, CStringW u = L"");

		void GetAsString(CStringW& str);
		void GetAsNumber(Number<int>& n, StringMapW<int>* n2n = NULL);
		void GetAsNumber(Number<DWORD>& n, StringMapW<DWORD>* n2n = NULL);
		void GetAsNumber(Number<float>& n, StringMapW<float>* n2n = NULL);
		template<typename T>
		void GetAsNumber(T& t, StringMapW<T>* n2n = NULL) {
			Number<T> n;
			GetAsNumber(n, n2n);
			t = n.value;
		}
		void GetAsBoolean(bool& b);
		bool GetAsTime(Time& t, StringMapW<float>& offset, StringMapW<float>* n2n = NULL, int default_id = 0);

		operator LPCWSTR();
		operator float();
		operator bool();

		Definition* SetChildAsValue(CStringW path, status_t s, CStringW v, CStringW u = L"");
		Definition* SetChildAsNumber(CStringW path, CStringW v, CStringW u = L"");
	};
}
