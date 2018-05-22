//////////////////////////////////////////////////////////////////////
//
// DYModel.h: interface for the DYModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DYMODEL_H__7AC5FC61_DC06_4649_8E17_3394F6764831__INCLUDED_)
#define AFX_DYMODEL_H__7AC5FC61_DC06_4649_8E17_3394F6764831__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DYDefine.h"
#include "DYVertList.h"
#include "DYIntList.h"

#include <vector>

#define DY_MODEL_TYPE_NONE              0
#define DY_MODEL_TYPE_TRIANGLE_LIST     1
#define DY_MODEL_TYPE_TRIANGLE_STRIP    2
#define DY_MODEL_TYPE_TRIANGLE_FAN      3

#define DY_MODEL_IS_VALID_TYPE(nType)   ((nType)>=DY_MODEL_TYPE_TRIANGLE_LIST && (nType)<=DY_MODEL_TYPE_TRIANGLE_FAN)
#define DY_MODEL_IS_INVALID_TYPE(nType) ((nType)< DY_MODEL_TYPE_TRIANGLE_LIST || (nType)> DY_MODEL_TYPE_TRIANGLE_FAN)

class DYMat4x4;
class DYFaceList;
class DYModelList;

class DYModel
{
public:
	INT        Type;

	DYVertList VertList;

public:
	DYModel(const INT nType=DY_MODEL_TYPE_NONE,const DYVertList &aVertList=DYVertList());
	virtual ~DYModel();

public:
	DYModel(const DYModel &aModel);
	DYModel & operator = (const DYModel &aModel);

public:
	BOOL           IsEmpty     (VOID) const;

public:
	BOOL           Parse       (DYFaceList &aFaceList) const;

public:
	static BOOL    CalcParams  (const DYMat4x4 &matViewToScr,const FLOAT zView,const FLOAT fPixel,FLOAT &fAlpha,FLOAT &fBeta,FLOAT &fGamma,FLOAT &fDelta,INT &nBias);

public:
	static DYModel FromSimplify(const DYModel     &aModel    , std::vector<DYVert>& vecDYVertOut, DYIntList& indexListOut, vector<pair<int, DYIntList> >& vecModeAndIndexList, const FLOAT fAlpha=1.0f,const FLOAT fBeta=1.0f,const FLOAT fGamma=1.0f,const FLOAT fDelta=1.0f,const INT nBias=2,const BOOL bCalcNormals=TRUE,const BOOL bAveNormals=TRUE,const BOOL bUseAreaWeight=TRUE);
	static DYModel FromSimplify(const DYModelList &aModelList,const FLOAT fAlpha=1.0f,const FLOAT fBeta=1.0f,const FLOAT fGamma=1.0f,const FLOAT fDelta=1.0f,const INT nBias=2,const BOOL bCalcNormals=TRUE,const BOOL bAveNormals=TRUE,const BOOL bUseAreaWeight=TRUE);

	static DYModel FromFaceList(const DYFaceList &aFaceList);
};

#endif // !defined(AFX_DYMODEL_H__7AC5FC61_DC06_4649_8E17_3394F6764831__INCLUDED_)
