//////////////////////////////////////////////////////////////////////
//
// DYStringList.cpp: implementation of the DYStringList class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DYStringList.h"

DYStringList::DYStringList()
{
	Clear();
}

DYStringList::~DYStringList()
{
	Clear();
}

DYStringList::DYStringList(const vector<DYString> &pStrings):m_pStrings(pStrings)
{
}

DYStringList::DYStringList(const DYStringList &aStringList):m_pStrings(aStringList.m_pStrings)
{
}

DYStringList & DYStringList::operator = (const vector<DYString> &pStrings)
{
	m_pStrings=pStrings;

	return *this;
}

DYStringList & DYStringList::operator = (const DYStringList &aStringList)
{
	if(this==&aStringList)
	{
		return *this;
	}

	m_pStrings=aStringList.m_pStrings;

	return *this;
}

DYString & DYStringList::operator [] (const INT nIndex)
{
	assert(nIndex>=0 && nIndex<GetCount());

	return m_pStrings[nIndex];
}

const DYString & DYStringList::operator [] (const INT nIndex) const
{
	assert(nIndex>=0 && nIndex<GetCount());

	return m_pStrings[nIndex];
}

DYStringList & DYStringList::operator += (const DYStringList &aStringList)
{
	const INT nCount=(INT)(aStringList.m_pStrings.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		m_pStrings.push_back(aStringList.m_pStrings[nIndex]);
	}

	return *this;
}

DYStringList DYStringList::operator + (const DYStringList &aStringList) const
{
	DYStringList aRetStringList(*this);

	const INT nCount=(INT)(aStringList.m_pStrings.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		aRetStringList.m_pStrings.push_back(aStringList.m_pStrings[nIndex]);
	}

	return aRetStringList;
}

BOOL DYStringList::IsEmpty(VOID) const
{
	return m_pStrings.empty();
}

INT DYStringList::GetCount(VOID) const
{
	return (INT)(m_pStrings.size());
}

BOOL DYStringList::Clear(VOID)
{
	m_pStrings.clear();

	return TRUE;
}

BOOL DYStringList::Append(const DYString &aString)
{
	m_pStrings.push_back(aString);

	return TRUE;
}

BOOL DYStringList::Insert(const INT nIndex,const DYString &aString)
{
	m_pStrings.insert(m_pStrings.begin()+nIndex,aString);

	return TRUE;
}

BOOL DYStringList::Erase(const INT nIndex)
{
	assert(nIndex>=0 && nIndex<GetCount());

	m_pStrings.erase(m_pStrings.begin()+nIndex);

	return TRUE;
}
