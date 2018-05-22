//////////////////////////////////////////////////////////////////////
//
// DYModel.cpp: implementation of the DYModel class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DYModel.h"
#include "DYMat4x4.h"
#include "DYFace.h"
#include "DYFaceList.h"
#include "DYModelList.h"

// #ifdef _DEBUG
// #undef THIS_FILE
// static char THIS_FILE[]=__FILE__;
// #define new DEBUG_NEW
// #endif

DYModel::DYModel(const INT nType,const DYVertList &aVertList):Type(nType),VertList(aVertList)
{
}

DYModel::~DYModel()
{
}

DYModel::DYModel(const DYModel &aModel):Type(aModel.Type),VertList(aModel.VertList)
{
}

DYModel & DYModel::operator = (const DYModel &aModel)
{
	if(this==&aModel)
	{
		return *this;
	}

	Type    =aModel.Type;

	VertList=aModel.VertList;

	return *this;
}

BOOL DYModel::IsEmpty(VOID) const
{
	if(DY_MODEL_IS_INVALID_TYPE(Type))
	{
		return TRUE;
	}

	const INT nVertCount=VertList.GetCount();

	if(nVertCount<3)
	{
		return TRUE;
	}

	switch(Type)
	{
	case DY_MODEL_TYPE_TRIANGLE_LIST:
		{
			return (nVertCount%3!=0)?(TRUE):(FALSE);

			break;
		}
	case DY_MODEL_TYPE_TRIANGLE_STRIP:
		{
			return FALSE;

			break;
		}
	case DY_MODEL_TYPE_TRIANGLE_FAN:
		{
			return FALSE;

			break;
		}
	default:
		{
			return TRUE;

			break;
		}
	}
}

BOOL DYModel::Parse(DYFaceList &aFaceList) const
{
	if(IsEmpty())
	{
		return FALSE;
	}

	const INT nVertCount=VertList.GetCount();

	assert(nVertCount>=3);

	switch(Type)
	{
	case DY_MODEL_TYPE_TRIANGLE_LIST:
		{
			assert(nVertCount%3==0);

			INT nVertIndex=0;

			for(nVertIndex=0;nVertIndex<nVertCount;nVertIndex+=3)
			{
				const DYFace aFace(VertList[nVertIndex],VertList[nVertIndex+1],VertList[nVertIndex+2]);

				if(!aFace.IsEmpty() && !aFaceList.Append(aFace))
				{
					return FALSE;
				}
			}

			break;
		}
	case DY_MODEL_TYPE_TRIANGLE_STRIP:
		{
			INT nVertIndex=2;

			for(nVertIndex=2;nVertIndex<nVertCount;nVertIndex++)
			{
				const DYFace aFace(VertList[nVertIndex-2],VertList[nVertIndex-1],VertList[nVertIndex]);

				if(!aFace.IsEmpty() && !aFaceList.Append(aFace))
				{
					return FALSE;
				}
			}

			break;
		}
	case DY_MODEL_TYPE_TRIANGLE_FAN:
		{
			INT nVertIndex=2;

			for(nVertIndex=2;nVertIndex<nVertCount;nVertIndex++)
			{
				const DYFace aFace(VertList[0],VertList[nVertIndex-1],VertList[nVertIndex]);

				if(!aFace.IsEmpty() && !aFaceList.Append(aFace))
				{
					return FALSE;
				}
			}

			break;
		}
	default:
		{
			return FALSE;

			break;
		}
	}

	return TRUE;
}

BOOL DYModel::CalcParams(const DYMat4x4 &matViewToScr,const FLOAT zView,const FLOAT fPixel,FLOAT &fAlpha,FLOAT &fBeta,FLOAT &fGamma,FLOAT &fDelta,INT &nBias)
{
	const DYVector2F &vPoint1=(DYVector2F)(DYVector3F(-1.0f, 0.0f,zView)*matViewToScr);
	const DYVector2F &vPoint2=(DYVector2F)(DYVector3F( 1.0f, 0.0f,zView)*matViewToScr);
	const DYVector2F &vPoint3=(DYVector2F)(DYVector3F( 0.0f,-1.0f,zView)*matViewToScr);
	const DYVector2F &vPoint4=(DYVector2F)(DYVector3F( 0.0f, 1.0f,zView)*matViewToScr);

	const FLOAT xDist=vPoint1.Dist(vPoint2);
	const FLOAT yDist=vPoint3.Dist(vPoint4);

	const FLOAT fDist=DY_SQRT(xDist*xDist+yDist*yDist);

	const FLOAT fPrec=fPixel/fDist;

	fAlpha=fPrec*fPrec;

	fBeta =0.25f  *fAlpha;
	fGamma=0.25f  *fAlpha;
	fDelta=0.0625f*fAlpha;

	nBias =2;

	return TRUE;
}

DYModel DYModel::FromSimplify(const DYModel &aModel,std::vector<DYVert>& vecDYVertOut, DYIntList& indexListOut, vector<pair<int, DYIntList> >& vecModeAndIndexList, const FLOAT fAlpha,const FLOAT fBeta,const FLOAT fGamma,const FLOAT fDelta,const INT nBias,const BOOL bCalcNormals,const BOOL bAveNormals,const BOOL bUseAreaWeight)
{
	DYFaceList aFaceList;

	if(!aModel.Parse(aFaceList))
	{
		return DYModel();
	}

	return FromFaceList(DYFaceList::FromSimplify(aFaceList,vecDYVertOut,vecModeAndIndexList,fAlpha,fBeta,fGamma,fDelta,nBias,bCalcNormals,bAveNormals,bUseAreaWeight));
}

DYModel DYModel::FromSimplify(const DYModelList &aModelList,const FLOAT fAlpha,const FLOAT fBeta,const FLOAT fGamma,const FLOAT fDelta,const INT nBias,const BOOL bCalcNormals,const BOOL bAveNormals,const BOOL bUseAreaWeight)
{
	DYFaceList aFaceList;

	if(!aModelList.Parse(aFaceList))
	{
		return DYModel();
	}

	std::vector<DYVert> vecDYVertOut;
	vector<pair<int, DYIntList> > vecModeAndIndexList;
	return FromFaceList(DYFaceList::FromSimplify(aFaceList,vecDYVertOut,vecModeAndIndexList,fAlpha,fBeta,fGamma,fDelta,nBias,bCalcNormals,bAveNormals,bUseAreaWeight));
}

DYModel DYModel::FromFaceList(const DYFaceList &aFaceList)
{
	DYModel aModel(DY_MODEL_TYPE_TRIANGLE_LIST,DYVertList());

	const INT nFaceCount=aFaceList.GetCount();

	INT nFaceIndex=0;

	for(nFaceIndex=0;nFaceIndex<nFaceCount;nFaceIndex++)
	{
		const DYFace &aFace=aFaceList[nFaceIndex];

		if(aFace.IsEmpty())
		{
			continue;
		}

		if(!aModel.VertList.Append(aFace.Vert1) || !aModel.VertList.Append(aFace.Vert2) || !aModel.VertList.Append(aFace.Vert3))
		{
			return DYModel();
		}
	}

	return aModel;
}
