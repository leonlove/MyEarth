//////////////////////////////////////////////////////////////////////
//
// DYModelList.h: interface for the DYModelList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DYMODELLIST_H__943A9700_0851_4F1C_8BE9_CEFEAB954595__INCLUDED_)
#define AFX_DYMODELLIST_H__943A9700_0851_4F1C_8BE9_CEFEAB954595__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DYDefine.h"

class DYFaceList;
class DYModel;

class DYModelList
{
protected:
	vector<DYModel> m_pModels;

public:
	DYModelList();
	virtual ~DYModelList();

public:
	DYModelList(const vector<DYModel> &pModels);
	DYModelList(const DYModelList     &aModelList);
	DYModelList & operator = (const vector<DYModel> &pModels);
	DYModelList & operator = (const DYModelList     &aModelList);

	DYModel       & operator [] (const INT nIndex);
	const DYModel & operator [] (const INT nIndex) const;

	DYModelList & operator += (const DYModelList &aModelList);
	DYModelList   operator +  (const DYModelList &aModelList) const;

public:
	BOOL IsEmpty (VOID) const;
	INT  GetCount(VOID) const;

public:
	BOOL Clear   (VOID);
	BOOL Append  (const DYModel &aModel);
	BOOL Insert  (const INT nIndex,const DYModel &aModel);
	BOOL Erase   (const INT nIndex);

	BOOL Parse   (DYFaceList &aFaceList) const;
};

#endif // !defined(AFX_DYMODELLIST_H__943A9700_0851_4F1C_8BE9_CEFEAB954595__INCLUDED_)
