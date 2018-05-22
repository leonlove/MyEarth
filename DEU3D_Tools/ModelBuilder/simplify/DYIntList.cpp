//////////////////////////////////////////////////////////////////////
//
// DYIntList.cpp: implementation of the DYIntList class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DYIntList.h"

// #ifdef _DEBUG
// #undef THIS_FILE
// static char THIS_FILE[]=__FILE__;
// #define new DEBUG_NEW
// #endif

DYIntList::DYIntList()
{
	Clear();
}

DYIntList::~DYIntList()
{
	Clear();
}

DYIntList::DYIntList(const vector<INT> &pDatas):m_pDatas(pDatas)
{
}

DYIntList::DYIntList(const DYIntList &aIntList):m_pDatas(aIntList.m_pDatas)
{
}

DYIntList & DYIntList::operator = (const vector<INT> &pDatas)
{
	m_pDatas=pDatas;

	return *this;
}

DYIntList & DYIntList::operator = (const DYIntList &aIntList)
{
	if(this==&aIntList)
	{
		return *this;
	}

	m_pDatas=aIntList.m_pDatas;

	return *this;
}

INT & DYIntList::operator [] (const INT nIndex)
{
	assert(nIndex>=0 && nIndex<GetCount());

	return m_pDatas[nIndex];
}

const INT & DYIntList::operator [] (const INT nIndex) const
{
	assert(nIndex>=0 && nIndex<GetCount());

	return m_pDatas[nIndex];
}

DYIntList & DYIntList::operator += (const DYIntList &aIntList)
{
	const INT nCount=(INT)(aIntList.m_pDatas.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		m_pDatas.push_back(aIntList.m_pDatas[nIndex]);
	}

	return *this;
}

DYIntList DYIntList::operator + (const DYIntList &aIntList) const
{
	DYIntList aRetIntList(*this);

	const INT nCount=(INT)(aIntList.m_pDatas.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		aRetIntList.m_pDatas.push_back(aIntList.m_pDatas[nIndex]);
	}

	return aRetIntList;
}

DYIntList & DYIntList::operator += (const INT nOffset)
{
	const INT nCount=(INT)(m_pDatas.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		m_pDatas[nIndex]+=nOffset;
	}

	return *this;
}

DYIntList & DYIntList::operator -= (const INT nOffset)
{
	const INT nCount=(INT)(m_pDatas.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		m_pDatas[nIndex]-=nOffset;
	}

	return *this;
}

DYIntList & DYIntList::operator *= (const INT nRatio)
{
	const INT nCount=(INT)(m_pDatas.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		m_pDatas[nIndex]*=nRatio;
	}

	return *this;
}

DYIntList & DYIntList::operator /= (const INT nRatio)
{
	const INT nCount=(INT)(m_pDatas.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		m_pDatas[nIndex]/=nRatio;
	}

	return *this;
}

DYIntList DYIntList::operator + (const INT nOffset) const
{
	DYIntList aRetIntList(*this);

	const INT nCount=(INT)(aRetIntList.m_pDatas.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		aRetIntList.m_pDatas[nIndex]+=nOffset;
	}

	return aRetIntList;
}

DYIntList DYIntList::operator - (const INT nOffset) const
{
	DYIntList aRetIntList(*this);

	const INT nCount=(INT)(aRetIntList.m_pDatas.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		aRetIntList.m_pDatas[nIndex]-=nOffset;
	}

	return aRetIntList;
}

DYIntList DYIntList::operator * (const INT nRatio) const
{
	DYIntList aRetIntList(*this);

	const INT nCount=(INT)(aRetIntList.m_pDatas.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		aRetIntList.m_pDatas[nIndex]*=nRatio;
	}

	return aRetIntList;
}

DYIntList DYIntList::operator / (const INT nRatio) const
{
	DYIntList aRetIntList(*this);

	const INT nCount=(INT)(aRetIntList.m_pDatas.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		aRetIntList.m_pDatas[nIndex]/=nRatio;
	}

	return aRetIntList;
}

DYIntList operator * (const INT nRatio,const DYIntList &aIntList)
{
	DYIntList aRetIntList(aIntList);

	const INT nCount=(INT)(aRetIntList.m_pDatas.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		aRetIntList.m_pDatas[nIndex]*=nRatio;
	}

	return aRetIntList;
}

BOOL DYIntList::IsEmpty(VOID) const
{
	return m_pDatas.empty();
}

INT DYIntList::GetCount(VOID) const
{
	return (INT)(m_pDatas.size());
}

BOOL DYIntList::Clear(VOID)
{
	m_pDatas.clear();

	return TRUE;
}

BOOL DYIntList::Append(const INT nData)
{
	m_pDatas.push_back(nData);

	return TRUE;
}

BOOL DYIntList::Insert(const INT nIndex,const INT nData)
{
	m_pDatas.insert(m_pDatas.begin()+nIndex,nData);

	return TRUE;
}

BOOL DYIntList::Erase(const INT nIndex)
{
	assert(nIndex>=0 && nIndex<GetCount());

	m_pDatas.erase(m_pDatas.begin()+nIndex);

	return TRUE;
}
