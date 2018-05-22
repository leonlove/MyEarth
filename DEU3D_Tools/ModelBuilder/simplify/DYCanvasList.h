//////////////////////////////////////////////////////////////////////
//
// DYCanvasList.h: interface for the DYCanvasList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DYCANVASLIST_H__D82B7F23_2AC0_4A38_AF90_F290A560C8BC__INCLUDED_)
#define AFX_DYCANVASLIST_H__D82B7F23_2AC0_4A38_AF90_F290A560C8BC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DYDefine.h"
#include "DYMat3x3.h"

class DYMat3x3;
class DYCanvas;

typedef map<DYString,DYMat3x3> DYFileToMat3x3;
typedef map<DYString,INT     > DYFileToID;

class DYCanvasList
{
protected:
	vector<DYCanvas> m_pCanvases;

public:
	DYCanvasList(const DYString &strDirectory=DY_TEXT(""));
	virtual ~DYCanvasList();

public:
	DYCanvasList(const vector<DYCanvas> &pCanvases);
	DYCanvasList(const DYCanvasList     &aCanvasList);
	DYCanvasList & operator = (const vector<DYCanvas> &pCanvases);
	DYCanvasList & operator = (const DYCanvasList     &aCanvasList);

	DYCanvas       & operator [] (const INT nIndex);
	const DYCanvas & operator [] (const INT nIndex) const;

	DYCanvasList & operator += (const DYCanvasList &aCanvasList);
	DYCanvasList   operator +  (const DYCanvasList &aCanvasList) const;

public:
	BOOL           IsEmpty        (VOID) const;
	INT            GetCount       (VOID) const;
	DYFileToMat3x3 GetFileToMat3x3(VOID) const;
	DYFileToID     GetFileToID    (VOID) const;

public:
	BOOL           Clear          (VOID);
	BOOL           Append         (const DYCanvas &aCanvas);
	INT            Add            (const DYCanvas &aCanvas);
	BOOL           Insert         (const INT nIndex,const DYCanvas &aCanvas);
	BOOL           Erase          (const INT nIndex);

	BOOL           Create         (const DYString &strDirectory);
	BOOL           Union          (const INT nMaxSize=1024);
	BOOL           Save           (const DYString &strDirectory);

public:
	BOOL           Update         (VOID);
	BOOL           SortQ          (VOID);

protected:
	static INT     Compare        (const VOID *pElem1,const VOID *pElem2);

public:
	static BOOL    UnionDIR       (const DYString &strSrcDir,const DYString     &strDstDir,DYFileToMat3x3 &pmapFileToMat3x3,DYFileToID &pmapFileToID,const INT nMaxSize=1024);
	static BOOL    Union1_2       (DYCanvasList   &pCanvas2s,const DYCanvasList &pCanvas1s,const INT nMaxSize=1024,const BOOL      bClearFirst     =TRUE);
};

#endif // !defined(AFX_DYCANVASLIST_H__D82B7F23_2AC0_4A38_AF90_F290A560C8BC__INCLUDED_)
