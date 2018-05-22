//////////////////////////////////////////////////////////////////////
//
// DYIntList.h: interface for the DYIntList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DYINTLIST_H__2F77FB33_C80D_4FA1_A548_F51D88910AA1__INCLUDED_)
#define AFX_DYINTLIST_H__2F77FB33_C80D_4FA1_A548_F51D88910AA1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DYDefine.h"

class DYIntList
{
protected:
	vector<INT> m_pDatas;

public:
	DYIntList();
	virtual ~DYIntList();

public:
	DYIntList(const vector<INT> &pDatas);
	DYIntList(const DYIntList   &aIntList);
	DYIntList & operator = (const vector<INT> &pDatas);
	DYIntList & operator = (const DYIntList   &aIntList);

	INT       & operator [] (const INT nIndex);
	const INT & operator [] (const INT nIndex) const;

	DYIntList & operator += (const DYIntList &aIntList);
	DYIntList   operator +  (const DYIntList &aIntList) const;

	DYIntList & operator += (const INT nOffset);
	DYIntList & operator -= (const INT nOffset);
	DYIntList & operator *= (const INT  nRatio);
	DYIntList & operator /= (const INT  nRatio);

	DYIntList operator + (const INT nOffset) const;
	DYIntList operator - (const INT nOffset) const;
	DYIntList operator * (const INT  nRatio) const;
	DYIntList operator / (const INT  nRatio) const;

	friend DYIntList operator * (const INT nRatio,const DYIntList &aIntList);

public:
	BOOL             IsEmpty      (VOID) const;
	INT              GetCount     (VOID) const;

public:
	BOOL             Clear        (VOID);
	BOOL             Append       (const INT nData);
	BOOL             Insert       (const INT nIndex,const INT nData);
	BOOL             Erase        (const INT nIndex);
};

#endif // !defined(AFX_DYINTLIST_H__2F77FB33_C80D_4FA1_A548_F51D88910AA1__INCLUDED_)
