//////////////////////////////////////////////////////////////////////
//
// DYStringList.h: interface for the DYStringList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DYSTRINGLIST_H__5CC18623_BD42_4AEF_AD45_669E07193D0C__INCLUDED_)
#define AFX_DYSTRINGLIST_H__5CC18623_BD42_4AEF_AD45_669E07193D0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DYDefine.h"

class DYStringList
{
protected:
	vector<DYString> m_pStrings;

public:
	DYStringList();
	virtual ~DYStringList();

public:
	DYStringList(const vector<DYString> &pStrings);
	DYStringList(const DYStringList     &aStringList);
	DYStringList & operator = (const vector<DYString> &pStrings);
	DYStringList & operator = (const DYStringList     &aStringList);

	DYString       & operator [] (const INT nIndex);
	const DYString & operator [] (const INT nIndex) const;

	DYStringList & operator += (const DYStringList &aStringList);
	DYStringList   operator +  (const DYStringList &aStringList) const;

public:
	BOOL IsEmpty (VOID) const;
	INT  GetCount(VOID) const;

public:
	BOOL Clear   (VOID);
	BOOL Append  (const DYString &aString);
	BOOL Insert  (const INT nIndex,const DYString &aString);
	BOOL Erase   (const INT nIndex);
};

#endif // !defined(AFX_DYSTRINGLIST_H__5CC18623_BD42_4AEF_AD45_669E07193D0C__INCLUDED_)
