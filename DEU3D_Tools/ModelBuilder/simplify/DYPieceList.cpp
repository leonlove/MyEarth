//////////////////////////////////////////////////////////////////////
//
// DYPieceList.cpp: implementation of the DYPieceList class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DYPieceList.h"
#include "DYPiece.h"

DYPieceList::DYPieceList()
{
	Clear();
}

DYPieceList::~DYPieceList()
{
	//Clear();
}

DYPieceList::DYPieceList(const vector<DYPiece> &pPieces)/*:m_pPieces(pPieces)*/
{
	for (size_t i=0; i<pPieces.size(); i++)
	{
		m_pPieces.push_back(pPieces[i]);
	}

	return;
}

DYPieceList::DYPieceList(const DYPieceList &aPieceList)/*:m_pPieces(aPieceList.m_pPieces)*/
{
	for (size_t i=0; i<aPieceList.m_pPieces.size(); i++)
	{
		m_pPieces.push_back(aPieceList.m_pPieces[i]);
	}

	return;
}

DYPieceList & DYPieceList::operator = (const vector<DYPiece> &pPieces)
{
	m_pPieces=pPieces;

	return *this;
}

DYPieceList & DYPieceList::operator = (const DYPieceList &aPieceList)
{
	if(this==&aPieceList)
	{
		return *this;
	}

	m_pPieces=aPieceList.m_pPieces;

	return *this;
}

DYPiece & DYPieceList::operator [] (const INT nIndex)
{
	assert(nIndex>=0 && nIndex<GetCount());

	return m_pPieces[nIndex];
}

const DYPiece & DYPieceList::operator [] (const INT nIndex) const
{
	assert(nIndex>=0 && nIndex<GetCount());

	return m_pPieces[nIndex];
}

DYPieceList & DYPieceList::operator += (const DYPieceList &aPieceList)
{
	const INT nCount=(INT)(aPieceList.m_pPieces.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		m_pPieces.push_back(aPieceList.m_pPieces[nIndex]);
	}

	return *this;
}

DYPieceList DYPieceList::operator + (const DYPieceList &aPieceList) const
{
	DYPieceList aRetPieceList(*this);

	const INT nCount=(INT)(aPieceList.m_pPieces.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		aRetPieceList.m_pPieces.push_back(aPieceList.m_pPieces[nIndex]);
	}

	return aRetPieceList;
}

BOOL DYPieceList::IsEmpty(VOID) const
{
	return m_pPieces.empty();
}

INT DYPieceList::GetCount(VOID) const
{
	return (INT)(m_pPieces.size());
}

BOOL DYPieceList::Clear(VOID)
{
	m_pPieces.clear();

	return TRUE;
}

BOOL DYPieceList::Append(const DYPiece &aPiece)
{
	m_pPieces.push_back(aPiece);

	return TRUE;
}

INT DYPieceList::Add(const DYPiece &aPiece)
{
	const INT nMaxIndex=(INT)(m_pPieces.size());

	m_pPieces.push_back(aPiece);

	return nMaxIndex;
}

BOOL DYPieceList::Insert(const INT nIndex,const DYPiece &aPiece)
{
	m_pPieces.insert(m_pPieces.begin()+nIndex,aPiece);

	return TRUE;
}

BOOL DYPieceList::Erase(const INT nIndex)
{
	assert(nIndex>=0 && nIndex<GetCount());

	m_pPieces.erase(m_pPieces.begin()+nIndex);

	return TRUE;
}
