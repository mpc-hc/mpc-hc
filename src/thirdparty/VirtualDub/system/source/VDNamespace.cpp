//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2004 Avery Lee, All Rights Reserved.
//
//	Beginning with 1.6.0, the VirtualDub system library is licensed
//	differently than the remainder of VirtualDub.  This particular file is
//	thus licensed as follows (the "zlib" license):
//
//	This software is provided 'as-is', without any express or implied
//	warranty.  In no event will the authors be held liable for any
//	damages arising from the use of this software.
//
//	Permission is granted to anyone to use this software for any purpose,
//	including commercial applications, and to alter it and redistribute it
//	freely, subject to the following restrictions:
//
//	1.	The origin of this software must not be misrepresented; you must
//		not claim that you wrote the original software. If you use this
//		software in a product, an acknowledgment in the product
//		documentation would be appreciated but is not required.
//	2.	Altered source versions must be plainly marked as such, and must
//		not be misrepresented as being the original software.
//	3.	This notice may not be removed or altered from any source
//		distribution.

#include "stdafx.h"
#include <string.h>
#include <ctype.h>

#include <vd2/system/list.h>
#include <vd2/system/VDNamespace.h>

///////////////////////////////////////////////////////////////////////////
//
//	Group
//
///////////////////////////////////////////////////////////////////////////

VDNamespaceGroup::VDNamespaceGroup(const char *_pszName, VDNamespaceGroup *parent)
: VDNamespaceNode(namedup(_pszName),parent)
{
	const char *t = strchr(_pszName,'/');

	if (t) {

	} else
		strcpy((char *)pszName, _pszName);
}

VDNamespaceGroup::~VDNamespaceGroup() {
	delete[] (char *)pszName;
}

const char *VDNamespaceGroup::namedup(const char *s) {
	const char *t = strchr(s,'/');
	char *mem;

	if (t) {
		mem = new char[(t-s)+1];

		memcpy(mem, s, (t-s));
		mem[t-s] = 0;

		return mem;
	} else {
		mem = new char[strlen(s)+1];

		return strcpy(mem, s);
	}
}

///////////////////////////////////////////////////////////////////////////
//
// Item
//
///////////////////////////////////////////////////////////////////////////

VDNamespaceItem::VDNamespaceItem(const char *_pszName, VDNamespaceGroup *parent, const void *src)
: VDNamespaceNode(_pszName,parent), object(src)
{}

VDNamespaceItem::~VDNamespaceItem() {}

///////////////////////////////////////////////////////////////////////////
//
//	VDNamespace
//
///////////////////////////////////////////////////////////////////////////

bool VDNamespaceCompare(const char *psz1, const char *psz2) {
	char c, d;

	while((!!(c=toupper(*psz1++)) & !!(d=toupper(*psz2++))) && c!='/' && d!='/' && c==d)
		;

	if (c=='/') c=0;
	if (d=='/') d=0;

	return c==d;
}

VDNamespace::VDNamespace() : root("", NULL) {
}

VDNamespace::~VDNamespace() {
}

VDNamespaceGroup *VDNamespace::_lookupGroup(const char *pszName, bool fCreate, bool fIsFilter) {
	const char *pszNameLimit = pszName;
	const char *slash = NULL;
	VDNamespaceGroup *pGroup = &root, *pGroupNext;

	while(*pszNameLimit) {
		if (*pszNameLimit++ == '/')
			slash = pszNameLimit - 1;
	}

	if (fIsFilter)
		pszNameLimit = slash;

	while(pszName < pszNameLimit) {
		VDNamespaceGroup *pGroupParent = pGroup;

		pGroup = pGroup->listGroups.AtHead();

		while(pGroupNext = pGroup->NextFromHead()) {
			if (VDNamespaceCompare(pszName, pGroup->pszName))
				break;

			pGroup = pGroupNext;
		}

		if (!pGroupNext && fCreate) {
			pGroupNext = pGroup = new VDNamespaceGroup(pszName, pGroupParent);

			pGroupParent->listGroups.AddTail(pGroup);
		}

		// group not found?

		if (!pGroupNext) {
			return NULL;
		}

		// advance to next slash

		while(*pszName && *pszName++!='/')
			;
	}

	return pGroup;
}

void VDNamespace::clear() {
	root.listGroups.dispose();
	root.listItems.dispose();
}

void VDNamespace::add(const char *pszGroup, const char *pszName, const void *pDef) {
	VDNamespaceGroup *pGroup = _lookupGroup(pszGroup, true, false);
	
	pGroup->listItems.AddTail(new VDNamespaceItem(pszName, pGroup, pDef));
}

const void *VDNamespace::lookup(const char *pszName) {
	VDNamespaceGroup *pGroup = _lookupGroup(pszName, false, true);

	if (!pGroup)
		return NULL;

	const char *pszNameBase = pszName;

	while(*pszName++)
		if (pszName[-1]=='/')
			pszNameBase = pszName;

	for(ListAlloc<VDNamespaceItem >::fwit it = pGroup->listItems.begin(); it; ++it)
		if (!_stricmp(it->pszName, pszNameBase))
			return it->object;

	return NULL;
}

bool VDNamespace::enumerateGroups(const VDNamespaceGroup *pGroupRoot, tGroupEnumerator pEnum, void *pvData) {
	VDNamespaceGroup *pGroup, *pGroupNext;

	pGroup = (pGroupRoot ? pGroupRoot : &root)->listGroups.AtHead();
	while(pGroupNext = pGroup->NextFromHead()) {
		if (!pEnum(this, pGroup->pszName, pGroup, pvData))
			return false;

		pGroup = pGroupNext;
	}

	return true;
}

bool VDNamespace::enumerateItems(const VDNamespaceGroup *pGroupRoot, tItemEnumerator pEnum, void *pvData) {
	VDNamespaceItem *pEntry, *pEntryNext;

	pEntry = pGroupRoot->listItems.AtHead();
	while(pEntryNext = pEntry->NextFromHead()) {
		if (!pEnum(this, pEntry->pszName, pEntry->object, pvData))
			return false;

		pEntry = pEntryNext;
	}

	return true;
}

VDNamespaceItem *VDNamespace::_findItemByObject(const VDNamespaceGroup *pGroup, const void *pObj) {
	for(ListAlloc<VDNamespaceItem>::fwit it=pGroup->listItems.begin(); it; ++it) {
		if (it->object == pObj) {
			return it;
		}
	}

	for(ListAlloc<VDNamespaceGroup>::fwit it2=pGroup->listGroups.begin(); it2; ++it2) {
		VDNamespaceItem *v;

		if (v = _findItemByObject(it2, pObj))
			return v;
	}

	return NULL;
}

bool VDNamespace::_getPathByItem(const VDNamespaceNode *pEntry, char *buf, int maxlen) {
	if (!pEntry)
		return false;

	if (maxlen < (int)strlen(pEntry->pszName)+2)
		return false;

	if (pEntry->pParent && pEntry->pParent->pParent) {
		if (!_getPathByItem(pEntry->pParent, buf, maxlen))
			return false;

		while(*buf)
			++buf, --maxlen;

		*buf++ = '/';
	}

	strcpy(buf, pEntry->pszName);

	return true;
}

bool VDNamespace::getPathByItem(const void *pObj, char *buf, int maxlen) {
	return _getPathByItem(_findItemByObject(&root, pObj), buf, maxlen);
}
