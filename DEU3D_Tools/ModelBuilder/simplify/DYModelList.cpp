//////////////////////////////////////////////////////////////////////
//
// DYModelList.cpp: implementation of the DYModelList class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DYModelList.h"
#include "DYModel.h"

// #ifdef _DEBUG
// #undef THIS_FILE
// static char THIS_FILE[]=__FILE__;
// #define new DEBUG_NEW
// #endif

DYModelList::DYModelList()
{
	Clear();
}

DYModelList::~DYModelList()
{
	Clear();
}

DYModelList::DYModelList(const vector<DYModel> &pModels):m_pModels(pModels)
{
}

DYModelList::DYModelList(const DYModelList &aModelList):m_pModels(aModelList.m_pModels)
{
}

DYModelList & DYModelList::operator = (const vector<DYModel> &pModels)
{
	m_pModels=pModels;

	return *this;
}

DYModelList & DYModelList::operator = (const DYModelList &aModelList)
{
	if(this==&aModelList)
	{
		return *this;
	}

	m_pModels=aModelList.m_pModels;

	return *this;
}

DYModel & DYModelList::operator [] (const INT nIndex)
{
	assert(nIndex>=0 && nIndex<GetCount());

	return m_pModels[nIndex];
}

const DYModel & DYModelList::operator [] (const INT nIndex) const
{
	assert(nIndex>=0 && nIndex<GetCount());

	return m_pModels[nIndex];
}

DYModelList & DYModelList::operator += (const DYModelList &aModelList)
{
	const INT nCount=(INT)(aModelList.m_pModels.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		m_pModels.push_back(aModelList.m_pModels[nIndex]);
	}

	return *this;
}

DYModelList DYModelList::operator + (const DYModelList &aModelList) const
{
	DYModelList aRetModelList(*this);

	const INT nCount=(INT)(aModelList.m_pModels.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		aRetModelList.m_pModels.push_back(aModelList.m_pModels[nIndex]);
	}

	return aRetModelList;
}

BOOL DYModelList::IsEmpty(VOID) const
{
	return m_pModels.empty();
}

INT DYModelList::GetCount(VOID) const
{
	return (INT)(m_pModels.size());
}

BOOL DYModelList::Clear(VOID)
{
	m_pModels.clear();

	return TRUE;
}

BOOL DYModelList::Append(const DYModel &aModel)
{
	m_pModels.push_back(aModel);

	return TRUE;
}

BOOL DYModelList::Insert(const INT nIndex,const DYModel &aModel)
{
	m_pModels.insert(m_pModels.begin()+nIndex,aModel);

	return TRUE;
}

BOOL DYModelList::Erase(const INT nIndex)
{
	assert(nIndex>=0 && nIndex<GetCount());

	m_pModels.erase(m_pModels.begin()+nIndex);

	return TRUE;
}

BOOL DYModelList::Parse(DYFaceList &aFaceList) const
{
	const INT nModelCount=GetCount();

	INT nModelIndex=0;

	for(nModelIndex=0;nModelIndex<nModelCount;nModelIndex++)
	{
		if(m_pModels[nModelIndex].Parse(aFaceList))
		{
			return FALSE;
		}
	}

	return TRUE;
}
