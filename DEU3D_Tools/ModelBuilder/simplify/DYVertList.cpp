//////////////////////////////////////////////////////////////////////
//
// DYVertList.cpp: implementation of the DYVertList class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DYVertList.h"
#include "DYVert.h"

// #ifdef _DEBUG
// #undef THIS_FILE
// static char THIS_FILE[]=__FILE__;
// #define new DEBUG_NEW
// #endif

DYVertList::DYVertList()
{
	Clear();
}

DYVertList::~DYVertList()
{
	Clear();
}

DYVertList::DYVertList(const vector<DYVert> &pVerts):m_pVerts(pVerts)
{
}

DYVertList::DYVertList(const DYVertList &aVertList):m_pVerts(aVertList.m_pVerts)
{
}

DYVertList & DYVertList::operator = (const vector<DYVert> &pVerts)
{
	m_pVerts=pVerts;

	return *this;
}

DYVertList & DYVertList::operator = (const DYVertList &aVertList)
{
	if(this==&aVertList)
	{
		return *this;
	}

	m_pVerts=aVertList.m_pVerts;

	return *this;
}

DYVert & DYVertList::operator [] (const INT nIndex)
{
	assert(nIndex>=0 && nIndex<GetCount());

	return m_pVerts[nIndex];
}

const DYVert & DYVertList::operator [] (const INT nIndex) const
{
	assert(nIndex>=0 && nIndex<GetCount());

	return m_pVerts[nIndex];
}

DYVertList & DYVertList::operator += (const DYVertList &aVertList)
{
	const INT nCount=(INT)(aVertList.m_pVerts.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		m_pVerts.push_back(aVertList.m_pVerts[nIndex]);
	}

	return *this;
}

DYVertList DYVertList::operator + (const DYVertList &aVertList) const
{
	DYVertList aRetVertList(*this);

	const INT nCount=(INT)(aVertList.m_pVerts.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		aRetVertList.m_pVerts.push_back(aVertList.m_pVerts[nIndex]);
	}

	return aRetVertList;
}

BOOL DYVertList::IsEmpty(VOID) const
{
	return m_pVerts.empty();
}

INT DYVertList::GetCount(VOID) const
{
	return (INT)(m_pVerts.size());
}

BOOL DYVertList::Clear(VOID)
{
	m_pVerts.clear();

	return TRUE;
}

BOOL DYVertList::Append(const DYVert &aVert)
{
	m_pVerts.push_back(aVert);

	return TRUE;
}

BOOL DYVertList::Insert(const INT nIndex,const DYVert &aVert)
{
	m_pVerts.insert(m_pVerts.begin()+nIndex,aVert);

	return TRUE;
}

BOOL DYVertList::Erase(const INT nIndex)
{
	assert(nIndex>=0 && nIndex<GetCount());

	m_pVerts.erase(m_pVerts.begin()+nIndex);

	return TRUE;
}
