//////////////////////////////////////////////////////////////////////
//
// DYVertMap.h: interface for the DYVertMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DYVERTMAP_H__CA54F14B_C8BC_4358_A539_4F7C197488F1__INCLUDED_)
#define AFX_DYVERTMAP_H__CA54F14B_C8BC_4358_A539_4F7C197488F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DYDefine.h"
#include "DYVert.h"

class DYIntList;

class DYVertMap
{
protected:
	vector<DYVert>  m_pVerts;
	map<DYVert,INT> m_pIndexes;
	map<INT, pair<INT, DYVector3F> > m_pNormal;

public:
	DYVertMap();
	virtual ~DYVertMap();

public:
	DYVertMap(const DYVertMap &aVertMap);
	DYVertMap & operator = (const DYVertMap &aVertMap);

	const DYVert & operator [] (const INT    nIndex) const;
	INT            operator [] (const DYVert &aVert) const;

public:
	BOOL IsEmpty (VOID) const;
	INT  GetCount(VOID) const;

public:
	BOOL Clear   (VOID);
	INT  Append  (const DYVert &aVert);  // assert((*this)[aVert]<0)
	INT  Add     (const DYVert &aVert);
	BOOL Erase   (const DYVert &aVert);
	BOOL Erase   (const INT    nIndex);
	INT  Alter   (const INT    nIndex,const DYVert &aVert);
	VOID CalcRepeatNormal(VOID);

	BOOL Simplify (DYVertMap &aNewVertMap,DYIntList &aNewIndexList,const FLOAT fAlpha,const FLOAT fBeta,const FLOAT fGamma,const FLOAT fDelta,const INT nBias) const;

protected:
	BOOL OnErase (const INT nIndex);
};

#endif // !defined(AFX_DYVERTMAP_H__CA54F14B_C8BC_4358_A539_4F7C197488F1__INCLUDED_)
