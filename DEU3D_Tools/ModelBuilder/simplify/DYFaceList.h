//////////////////////////////////////////////////////////////////////
//
// DYFaceList.h: interface for the DYFaceList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DYFACELIST_H__9E9D57E8_59EA_4C49_9363_F2796A6005BB__INCLUDED_)
#define AFX_DYFACELIST_H__9E9D57E8_59EA_4C49_9363_F2796A6005BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DYDefine.h"
#include "DYVert.h"

// #include <string>
// #include <set>
// #include <vector>
// #include <map>
// #include <osgDB\fstream>
// #include <sstream>
// //#include "define.h"
// using namespace std;

class DYIntList;
class DYVertMap;
class DYFace;

class DYFaceList
{
protected:
	vector<DYFace> m_pFaces;

public:
	DYFaceList();
	virtual ~DYFaceList();

public:
	DYFaceList(const vector<DYFace> &pFaces);
	DYFaceList(const DYFaceList     &aFaceList);
	DYFaceList & operator = (const vector<DYFace> &pFaces);
	DYFaceList & operator = (const DYFaceList     &aFaceList);

	DYFace       & operator [] (const INT nIndex);
	const DYFace & operator [] (const INT nIndex) const;

	DYFaceList & operator += (const DYFaceList &aFaceList);
	DYFaceList   operator +  (const DYFaceList &aFaceList) const;

public:
	BOOL              IsEmpty       (VOID) const;
	INT               GetCount      (VOID) const;

public:
	BOOL              Clear         (VOID);
	BOOL              Append        (const DYFace &aFace);
	BOOL              Insert        (const INT nIndex,const DYFace &aFace);
	BOOL              Erase         (const INT nIndex);

	BOOL              Parse         (DYVertMap     &aVertMap ,DYIntList &aIndexList) const;
	BOOL              ParseDelRepeat(DYVertMap     &aVertMap ,DYIntList &aIndexList);

	BOOL              CalcNormals   (VOID);
	BOOL              AveNormals    (const BOOL bUseAreaWeight=TRUE);
	FLOAT             CalcArea      (VOID) const;
	FLOAT             CalcArea2     (VOID) const;

public:
	static bool m_bMutlti;
    static bool FromDelRepeat (DYFaceList &aWorldFaceList,std::vector<DYVert>& vecDYVertOut, vector<pair<int, DYIntList> >& vecModeAndIndexList);
	static DYFaceList FromSimplify (const DYFaceList &aWorldFaceList,std::vector<DYVert>& vecDYVertOut, vector<pair<int, DYIntList> >& vecModeAndIndexList, const FLOAT fAlpha=1.0f,const FLOAT fBeta=1.0f,const FLOAT fGamma=1.0f,const FLOAT fDelta=1.0f,const INT nBias=2,const BOOL bCalcNormals=TRUE,const BOOL bAveNormals=TRUE,const BOOL bUseAreaWeight=TRUE);
};

#endif // !defined(AFX_DYFACELIST_H__9E9D57E8_59EA_4C49_9363_F2796A6005BB__INCLUDED_)
