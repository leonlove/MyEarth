//////////////////////////////////////////////////////////////////////
//
// DYVertList.h: interface for the DYVertList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DYVERTLIST_H__2C979BD6_C2D7_47C6_8CAB_C5E07EFF12EB__INCLUDED_)
#define AFX_DYVERTLIST_H__2C979BD6_C2D7_47C6_8CAB_C5E07EFF12EB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DYDefine.h"

class DYVert;

class DYVertList
{
protected:
	vector<DYVert> m_pVerts;

public:
	DYVertList();
	virtual ~DYVertList();

public:
	DYVertList(const vector<DYVert> &pVerts);
	DYVertList(const DYVertList     &aVertList);
	DYVertList & operator = (const vector<DYVert> &pVerts);
	DYVertList & operator = (const DYVertList     &aVertList);

	DYVert       & operator [] (const INT nIndex);
	const DYVert & operator [] (const INT nIndex) const;

	DYVertList & operator += (const DYVertList &aVertList);
	DYVertList   operator +  (const DYVertList &aVertList) const;

public:
	BOOL IsEmpty (VOID) const;
	INT  GetCount(VOID) const;

public:
	BOOL Clear   (VOID);
	BOOL Append  (const DYVert &aVert);
	BOOL Insert  (const INT nIndex,const DYVert &aVert);
	BOOL Erase   (const INT nIndex);
};

#endif // !defined(AFX_DYVERTLIST_H__2C979BD6_C2D7_47C6_8CAB_C5E07EFF12EB__INCLUDED_)
