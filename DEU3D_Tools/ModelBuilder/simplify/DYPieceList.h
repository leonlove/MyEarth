 //////////////////////////////////////////////////////////////////////
//
// DYPieceList.h: interface for the DYPieceList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DYPIECELIST_H__42F93B06_9F48_4170_9DE4_7233690C9B9D__INCLUDED_)
#define AFX_DYPIECELIST_H__42F93B06_9F48_4170_9DE4_7233690C9B9D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DYDefine.h"

class DYPiece;

class DYPieceList
{
protected:
	vector<DYPiece> m_pPieces;

public:
	DYPieceList();
	virtual ~DYPieceList();

public:
	DYPieceList(const vector<DYPiece> &pPieces);
	DYPieceList(const DYPieceList     &aPieceList);
	DYPieceList & operator = (const vector<DYPiece> &pPieces);
	DYPieceList & operator = (const DYPieceList     &aPieceList);

	DYPiece       & operator [] (const INT nIndex);
	const DYPiece & operator [] (const INT nIndex) const;

	DYPieceList & operator += (const DYPieceList &aPieceList);
	DYPieceList   operator +  (const DYPieceList &aPieceList) const;

public:
	BOOL IsEmpty (VOID) const;
	INT  GetCount(VOID) const;

public:
	BOOL Clear   (VOID);
	BOOL Append  (const DYPiece &aPiece);
	INT  Add     (const DYPiece &aPiece);
	BOOL Insert  (const INT nIndex,const DYPiece &aPiece);
	BOOL Erase   (const INT nIndex);
};

#endif // !defined(AFX_DYPIECELIST_H__42F93B06_9F48_4170_9DE4_7233690C9B9D__INCLUDED_)
