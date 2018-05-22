//////////////////////////////////////////////////////////////////////
//
// DYFaceList.cpp: implementation of the DYFaceList class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DYFaceList.h"
#include "DYIntList.h"
#include "DYVertMap.h"
#include "DYFace.h"
#include "DYCanvasList.h"
#include "DYTriangleIndex.h"

 #include <string>
 #include <set>
 #include <vector>
 #include <map>
 #include <osgDB\fstream>
 #include <sstream>
#include <iomanip>
 //#include "define.h"
 using namespace std;

/*extern*/ std::string g_strTextruePath;
/*extern*/ DYFileToMat3x3 g_mapFileToMat3x3;
/*extern*/ DYFileToID     g_mapFileToID;

// #ifdef _DEBUG
// #undef THIS_FILE
// static char THIS_FILE[]=__FILE__;
// #define new DEBUG_NEW
// #endif

bool DYFaceList::m_bMutlti = false;

DYFaceList::DYFaceList()
{
	Clear();
}

DYFaceList::~DYFaceList()
{
	Clear();
}

DYFaceList::DYFaceList(const vector<DYFace> &pFaces):m_pFaces(pFaces)
{
}

DYFaceList::DYFaceList(const DYFaceList &aFaceList):m_pFaces(aFaceList.m_pFaces)
{
}

DYFaceList & DYFaceList::operator = (const vector<DYFace> &pFaces)
{
	m_pFaces=pFaces;

	return *this;
}

DYFaceList & DYFaceList::operator = (const DYFaceList &aFaceList)
{
	if(this==&aFaceList)
	{
		return *this;
	}

	m_pFaces=aFaceList.m_pFaces;

	return *this;
}

DYFace & DYFaceList::operator [] (const INT nIndex)
{
	assert(nIndex>=0 && nIndex<GetCount());

	return m_pFaces[nIndex];
}

const DYFace & DYFaceList::operator [] (const INT nIndex) const
{
	assert(nIndex>=0 && nIndex<GetCount());

	return m_pFaces[nIndex];
}

DYFaceList & DYFaceList::operator += (const DYFaceList &aFaceList)
{
	const INT nCount=(INT)(aFaceList.m_pFaces.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		m_pFaces.push_back(aFaceList.m_pFaces[nIndex]);
	}

	return *this;
}

DYFaceList DYFaceList::operator + (const DYFaceList &aFaceList) const
{
	DYFaceList aRetFaceList(*this);

	const INT nCount=(INT)(aFaceList.m_pFaces.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		aRetFaceList.m_pFaces.push_back(aFaceList.m_pFaces[nIndex]);
	}

	return aRetFaceList;
}

BOOL DYFaceList::IsEmpty(VOID) const
{
	return m_pFaces.empty();
}

INT DYFaceList::GetCount(VOID) const
{
	return (INT)(m_pFaces.size());
}

BOOL DYFaceList::Clear(VOID)
{
	m_pFaces.clear();

	return TRUE;
}

BOOL DYFaceList::Append(const DYFace &aFace)
{
	m_pFaces.push_back(aFace);

	return TRUE;
}

BOOL DYFaceList::Insert(const INT nIndex,const DYFace &aFace)
{
	m_pFaces.insert(m_pFaces.begin()+nIndex,aFace);

	return TRUE;
}

BOOL DYFaceList::Erase(const INT nIndex)
{
	assert(nIndex>=0 && nIndex<GetCount());

	m_pFaces.erase(m_pFaces.begin()+nIndex);

	return TRUE;
}

BOOL DYFaceList::Parse(DYVertMap &aVertMap,DYIntList &aIndexList) const
{
	if(!aVertMap.Clear() || !aIndexList.Clear())
	{
		return FALSE;
	}

	const INT nFaceCount=(INT)(m_pFaces.size());

	INT nFaceIndex=0;

	for(nFaceIndex=0;nFaceIndex<nFaceCount;nFaceIndex++)
	{
		const DYFace &aFace=m_pFaces[nFaceIndex];

		const INT nIndex1=aVertMap.Add(aFace.Vert1);
		const INT nIndex2=aVertMap.Add(aFace.Vert2);
		const INT nIndex3=aVertMap.Add(aFace.Vert3);

		if(!aIndexList.Append(nIndex1))
		{
			return FALSE;
		}

		if(!aIndexList.Append(nIndex2))
		{
			return FALSE;
		}

		if(!aIndexList.Append(nIndex3))
		{
			return FALSE;
		}
	}

	if(aVertMap.IsEmpty() || aIndexList.IsEmpty())
	{
		return FALSE;
	}

	return TRUE;
}

BOOL DYFaceList::ParseDelRepeat(DYVertMap &aVertMap,DYIntList &aIndexList)
{
	if(!aVertMap.Clear() || !aIndexList.Clear())
	{
		return FALSE;
	}

	const INT nFaceCount=(INT)(m_pFaces.size());

	INT nFaceIndex=0;

	for(nFaceIndex=0;nFaceIndex<nFaceCount;nFaceIndex++)
	{
		DYFace &aFace=m_pFaces[nFaceIndex];
		
		aFace.Vert1.delrepeat = true;
		const INT nIndex1=aVertMap.Add(aFace.Vert1);

		aFace.Vert2.delrepeat = true;
		const INT nIndex2=aVertMap.Add(aFace.Vert2);

		aFace.Vert3.delrepeat = true;
		const INT nIndex3=aVertMap.Add(aFace.Vert3);

		if(!aIndexList.Append(nIndex1))
		{
			return FALSE;
		}

		if(!aIndexList.Append(nIndex2))
		{
			return FALSE;
		}

		if(!aIndexList.Append(nIndex3))
		{
			return FALSE;
		}
	}

	if(aVertMap.IsEmpty() || aIndexList.IsEmpty())
	{
		return FALSE;
	}

	return TRUE;
}

BOOL DYFaceList::CalcNormals(VOID)
{
	const INT nCount=(INT)(m_pFaces.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		DYFace &aFace=m_pFaces[nIndex];

		if(!aFace.CalcNormals())
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL DYFaceList::AveNormals(const BOOL bUseAreaWeight)
{
	typedef vector<DYVector3F *> DYVectorPtrs3F;

	map<DYVector3F,DYVectorPtrs3F> aPointNormalPtsMap;

	aPointNormalPtsMap.clear();

	const INT nCount=(INT)(m_pFaces.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		DYFace &aFace=m_pFaces[nIndex];

		if(bUseAreaWeight)
		{
			const FLOAT fWeight=aFace.CalcArea2();

			aFace.Vert1.Normal*=fWeight;
			aFace.Vert2.Normal*=fWeight;
			aFace.Vert3.Normal*=fWeight;
		}

		map<DYVector3F,DYVectorPtrs3F>::iterator itPointNormalPtsMap1=aPointNormalPtsMap.find(aFace.Vert1.Point);

		if(itPointNormalPtsMap1!=aPointNormalPtsMap.end())
		{
			DYVectorPtrs3F &pNormalPtrs=itPointNormalPtsMap1->second;

			assert(!pNormalPtrs.empty());

			pNormalPtrs.push_back(&(aFace.Vert1.Normal));
		}
		else
		{
			DYVectorPtrs3F pNormalPtrs;

			pNormalPtrs.clear();

			pNormalPtrs.push_back(&(aFace.Vert1.Normal));

			aPointNormalPtsMap.insert(pair<DYVector3F,DYVectorPtrs3F>(aFace.Vert1.Point,pNormalPtrs));
		}

		map<DYVector3F,DYVectorPtrs3F>::iterator itPointNormalPtsMap2=aPointNormalPtsMap.find(aFace.Vert2.Point);

		if(itPointNormalPtsMap2!=aPointNormalPtsMap.end())
		{
			DYVectorPtrs3F &pNormalPtrs=itPointNormalPtsMap2->second;

			assert(!pNormalPtrs.empty());

			pNormalPtrs.push_back(&(aFace.Vert2.Normal));
		}
		else
		{
			DYVectorPtrs3F pNormalPtrs;

			pNormalPtrs.clear();

			pNormalPtrs.push_back(&(aFace.Vert2.Normal));

			aPointNormalPtsMap.insert(pair<DYVector3F,DYVectorPtrs3F>(aFace.Vert2.Point,pNormalPtrs));
		}

		map<DYVector3F,DYVectorPtrs3F>::iterator itPointNormalPtsMap3=aPointNormalPtsMap.find(aFace.Vert3.Point);

		if(itPointNormalPtsMap3!=aPointNormalPtsMap.end())
		{
			DYVectorPtrs3F &pNormalPtrs=itPointNormalPtsMap3->second;

			assert(!pNormalPtrs.empty());

			pNormalPtrs.push_back(&(aFace.Vert3.Normal));
		}
		else
		{
			DYVectorPtrs3F pNormalPtrs;

			pNormalPtrs.clear();

			pNormalPtrs.push_back(&(aFace.Vert3.Normal));

			aPointNormalPtsMap.insert(pair<DYVector3F,DYVectorPtrs3F>(aFace.Vert3.Point,pNormalPtrs));
		}
	}

	map<DYVector3F,DYVectorPtrs3F>::iterator itPointNormalPtsMap=aPointNormalPtsMap.begin();

	for(;itPointNormalPtsMap!=aPointNormalPtsMap.end();itPointNormalPtsMap++)
	{
		DYVectorPtrs3F &pNormalPtrs=itPointNormalPtsMap->second;

		assert(!pNormalPtrs.empty());

		DYVector3F vWorldNormal;

		const INT nCount=(INT)(pNormalPtrs.size());

		INT nIndex=0;

		for(nIndex=0;nIndex<nCount;nIndex++)
		{
			DYVector3F *pNormal=pNormalPtrs[nIndex];

			assert(pNormal!=NULL);

			vWorldNormal+=(*pNormal);
		}

		vWorldNormal.Normalize();

//		assert(vWorldNormal!=0.0f);

		if(vWorldNormal!=0.0f)
		{
			for(nIndex=0;nIndex<nCount;nIndex++)
			{
				DYVector3F *pNormal=pNormalPtrs[nIndex];

				assert(pNormal!=NULL);

				(*pNormal)=vWorldNormal;
			}
		}
		else
		{
			for(nIndex=0;nIndex<nCount;nIndex++)
			{
				DYVector3F *pNormal=pNormalPtrs[nIndex];

				assert(pNormal!=NULL);

				pNormal->Normalize();
			}
		}
	}

	return TRUE;
}

FLOAT DYFaceList::CalcArea(VOID) const
{
	FLOAT fArea=0.0f;

	const INT nCount=(INT)(m_pFaces.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		const DYFace &aFace=m_pFaces[nIndex];

		fArea+=aFace.CalcArea();
	}

	return fArea;
}

FLOAT DYFaceList::CalcArea2(VOID) const
{
	FLOAT fArea2=0.0f;

	const INT nCount=(INT)(m_pFaces.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		const DYFace &aFace=m_pFaces[nIndex];

		fArea2+=aFace.CalcArea2();
	}

	return fArea2;
}

bool DYFaceList::FromDelRepeat(DYFaceList &aWorldFaceList, std::vector<DYVert>& vecDYVertOut, vector<pair<int, DYIntList> >& vecModeAndIndexList)
{
    DYVertMap aVertMap;
    DYIntList aIndexList;

    if(!aWorldFaceList.ParseDelRepeat(aVertMap,aIndexList))
    {
        return false;
    }
	aVertMap.CalcRepeatNormal();

    for (int i=0; i<aVertMap.GetCount(); i++)
    {
        vecDYVertOut.push_back(aVertMap[i]);
    }

    vecModeAndIndexList.push_back(make_pair(4, aIndexList));
    
    return true;
}

DYFaceList DYFaceList::FromSimplify(const DYFaceList &aWorldFaceList,std::vector<DYVert>& vecDYVertOut, vector<pair<int, DYIntList> >& vecModeAndIndexList, const FLOAT fAlpha,const FLOAT fBeta,const FLOAT fGamma,const FLOAT fDelta,const INT nBias,const BOOL bCalcNormals,const BOOL bAveNormals,const BOOL bUseAreaWeight)
{
	DYVertMap aVertMap;
	DYIntList aIndexList;
    DYIntList indexListOut;

	if(!aWorldFaceList.Parse(aVertMap,aIndexList))
	{
		return DYFaceList();
	}
	aVertMap.CalcRepeatNormal();

	DYVertMap aNewVertMap;
	DYIntList aNewIndexList;
	if(!aVertMap.Simplify(aNewVertMap,aNewIndexList,fAlpha,fBeta,fGamma,fDelta,nBias))
	{
		return DYFaceList();
	}

	DYFaceList aRetWorldFaceList;
	INT nIndex=0, nNewIndex=0, nWorldFaceIndex=0;
	const INT nWorldFaceCount=aWorldFaceList.GetCount();
	 
	for(nWorldFaceIndex=0;nWorldFaceIndex<nWorldFaceCount;nWorldFaceIndex++)
	{
		const DYFace &aWorldFace=aWorldFaceList[nWorldFaceIndex];
		DYFace aRetWorldFace(aWorldFace);
	 
		int n1,n2,n3;
		n1 = aNewIndexList[aIndexList[nIndex++]];
		aRetWorldFace.Vert1=aNewVertMap[n1];
	 
		n2 = aNewIndexList[aIndexList[nIndex++]];
		aRetWorldFace.Vert2=aNewVertMap[n2];
	 
		n3 = aNewIndexList[aIndexList[nIndex++]];
		aRetWorldFace.Vert3=aNewVertMap[n3];
	 
		if (!aRetWorldFace.IsEmpty())
		{
	 		indexListOut.Append(n1);
	 		indexListOut.Append(n2);
	 		indexListOut.Append(n3);
		}
	}
	 
	for (int i=0; i<aNewVertMap.GetCount(); i++)
	{
		vecDYVertOut.push_back(aNewVertMap[i]);
	}

	if (indexListOut.GetCount() > 3)
	{
		vecModeAndIndexList.push_back(make_pair(4, indexListOut));
	}

	return aRetWorldFaceList;
}

// DYFaceList DYFaceList::FromSimplify(const DYFaceList &aWorldFaceList,std::vector<DYVert>& vecDYVertOut, vector<pair<int, DYIntList> >& vecModeAndIndexList, const FLOAT fAlpha,const FLOAT fBeta,const FLOAT fGamma,const FLOAT fDelta,const INT nBias,const BOOL bCalcNormals,const BOOL bAveNormals,const BOOL bUseAreaWeight)
// {
// 	DYVertMap aVertMap;
// 	DYIntList aIndexList;
// 	DYIntList indexListOut;
// 
// 	if(!aWorldFaceList.Parse(aVertMap,aIndexList))
// 	{
// 		return DYFaceList();
// 	}
// 
// 	DYVertMap aNewVertMap;
// 	DYIntList     aNewIndexList;
// 
// 	if(!aVertMap.Simplify(aNewVertMap,aNewIndexList,fAlpha,fBeta,fGamma,fDelta,nBias))
// 	{
// 		return DYFaceList();
// 	}
// 
// 	DYFaceList aRetWorldFaceList;
// 
// 	INT nIndex=0;
// 
// 	const INT nWorldFaceCount=aWorldFaceList.GetCount();
// 
// 	INT nWorldFaceIndex=0;
// 
// 	int nMyNewIndex=0;
// 	vector<DYVert> aLastVertList;
// 	DYIntList aLastIndexList;
// 	for(nWorldFaceIndex=0;nWorldFaceIndex<nWorldFaceCount;nWorldFaceIndex++)
// 	{
// 		const DYFace &aWorldFace=aWorldFaceList[nWorldFaceIndex];
// 
// 		DYFace aRetWorldFace(aWorldFace);
// 
// 		int n1,n2,n3;
// 		n1 = aNewIndexList[aIndexList[nIndex++]];
// 		aRetWorldFace.Vert1=aNewVertMap[n1];
// 
// 		n2 = aNewIndexList[aIndexList[nIndex++]];
// 		aRetWorldFace.Vert2=aNewVertMap[n2];
// 
// 		n3 = aNewIndexList[aIndexList[nIndex++]];
// 		aRetWorldFace.Vert3=aNewVertMap[n3];
// 
// 
// 		if (!aRetWorldFace.IsEmpty())
// 		{
// 			indexListOut.Append(n1);
// 			indexListOut.Append(n2);
// 			indexListOut.Append(n3);
// 		}
// 		else
// 		{
// 			indexListOut.Append(-1);
// 		}
// 	}
// 	indexListOut.Append(-1);		//- 末尾追加一个标记，方便后续算法
// 
// 	//- vertex
// 	for (int i=0; i<aNewVertMap.GetCount(); i++)
// 	{
// 		vecDYVertOut.push_back(aNewVertMap[i]);
// 	}
// 
// 	//- index
// 	//- 删除相邻相同的标记
// 	int nIndexCnt = indexListOut.GetCount();
// 	for (int i=1; i<nIndexCnt; i++)
// 	{
// 		if(indexListOut[i]==-1 && indexListOut[i-1]==-1)
// 		{
// 			indexListOut.Erase(i);
// 			i--;
// 			nIndexCnt--;
// 		}
// 	}
// 
// 	//- 根据标记将索引进行三角网及条带分组
// 	nIndexCnt = indexListOut.GetCount();
// 	vector<DYTriangleIndex> vec;
// 	vector<DYTriangleIndex> vecSeperateTriangle;
// 	vector< vector<DYTriangleIndex> > vecSeperateStripVector;
// 	for (int i=0; i<nIndexCnt; i++)
// 	{
// 		if (indexListOut[i] == -1)
// 		{
// 			if (vec.size() != 0)
// 			{
// 				//- 1条索引段中size为1则为三角网
// 				if (vec.size() == 1)	vecSeperateTriangle.push_back(vec[0]);
// 
// 				//- 否则进行拆分，找出三角网及条带
// 				else
// 				{
// 					vector<DYTriangleIndex> vecSeperateStrip;
// 					for (size_t j=0; j<vec.size(); j++)
// 					{
// 						if (j == vec.size()-1)	break;
// 						if (vec[j].isStrip(vec[j+1]))
// 						{
// 							vecSeperateStrip.push_back(vec[j]);
// 							if (j+1 == vec.size()-1)	
// 							{
// 								vecSeperateStrip.push_back(vec[j+1]);
// 							}
// 						}
// 						else
// 						{
// 							if (vecSeperateStrip.size() == 0)
// 							{
// 								vecSeperateTriangle.push_back(vec[j]);
// 							}
// 							else
// 							{
// 								vecSeperateStrip.push_back(vec[j]);
// 								vecSeperateStripVector.push_back(vecSeperateStrip);
// 								vecSeperateStrip.clear();
// 							}
// 
// 							if (j+1 == vec.size()-1)	
// 							{
// 								vecSeperateTriangle.push_back(vec[j+1]);
// 							}
// 						}
// 					}
// 					if (vecSeperateStrip.size() > 0)
// 					{
// 						vecSeperateStripVector.push_back(vecSeperateStrip);
// 						vecSeperateStrip.clear();
// 					}
// 				}
// 				vec.clear();
// 			}
// 			continue;
// 		}
// 
// 		vec.push_back(DYTriangleIndex(indexListOut[i], indexListOut[i+1], indexListOut[i+2]));
// 		i+=2;
// 	}
// 	indexListOut.Clear();
// 
// 	for (size_t i=0; i<vecSeperateStripVector.size(); i++)
// 	{
// 		DYIntList dyIndexList;
// 		vector<DYTriangleIndex> vecTemp = vecSeperateStripVector[i];
// 		for (size_t j=0; j<vecTemp.size(); j++)
// 		{
// 			if (j == 0)
// 			{
// 				dyIndexList.Append(vecTemp[j].n1);
// 				dyIndexList.Append(vecTemp[j].n2);
// 				dyIndexList.Append(vecTemp[j].n3);
// 			}
// 			else
// 			{
// 				int nIndex = vecTemp[j-1].getStripIndex(vecTemp[j]);
// 				if (nIndex != -1)
// 					dyIndexList.Append(nIndex);
// 				else
// 					vecSeperateTriangle.push_back(vecTemp[j]);
// 				//dyIndexList.Append(nIndex);
// 			}
// 		}
// 
// 		if (dyIndexList.GetCount() > 3)
// 		{
// 			vecModeAndIndexList.push_back(make_pair(5, dyIndexList));
// 		}
// 		//vecModeAndIndexList.push_back(make_pair(5, dyIndexList));
// 		dyIndexList.Clear();
// 	}
// 
// 	DYIntList dyIndexList;
// 	for (size_t i=0; i<vecSeperateTriangle.size(); i++)
// 	{
// 		dyIndexList.Append(vecSeperateTriangle[i].n1);
// 		dyIndexList.Append(vecSeperateTriangle[i].n2);
// 		dyIndexList.Append(vecSeperateTriangle[i].n3);
// 	}
// 	if (dyIndexList.GetCount() > 0)
// 	{
// 		vecModeAndIndexList.push_back(make_pair(4, dyIndexList));
// 		dyIndexList.Clear();
// 	}
// 
// 	//=====================================================================
// 	//  	for (int i=0; i<vecDYVertOut.size(); i++)
// 	//  	{
// 	// 		//- 如果该点是无效点，表示已经比较过
// 	//  		if (!vecDYVertOut[i].valid)
// 	//  		{
// 	//  			continue;
// 	//  		}
// 	//  		for (int j=i+1; j<vecDYVertOut.size(); j++)
// 	//  		{
// 	// 			//- 首先找到相同点，将后面所有与该点相同的点设置为无效点，
// 	// 			//- 只保留最前面一个点为有效点，并将无效点的索引值指向第一个有效点的索引
// 	//  			if (vecDYVertOut[i] == vecDYVertOut[j])
// 	//  			{
// 	//  				vecDYVertOut[j].valid = false;
// 	//  				indexListOut[j] = indexListOut[i];
// 	//  			}
// 	//  		}
// 	//  	}
// 	//  
// 	//  	for (int i=0; i<vecDYVertOut.size(); i++)
// 	//  	{
// 	//  		if (!vecDYVertOut[i].valid)
// 	//  		{
// 	//   			vector<DYVert>::iterator itr = vecDYVertOut.begin() + i;
// 	//   			itr = vecDYVertOut.erase(itr);
// 	//   
// 	//   			for (int j=0; j<indexListOut.GetCount(); j++)
// 	//   			{
// 	// 				//- 减少一个顶点，所有大于当前顶点对应索引的索引序列自减一次
// 	//   				if (indexListOut[j] > i)
// 	//   				{
// 	//   					indexListOut[j] -= 1;
// 	//   				}
// 	//   			}
// 	// 
// 	// 			//- 如果连续的顶点都是无效点
// 	//  			if (itr!=vecDYVertOut.end() && !itr->valid)	 i--;
// 	//  
// 	//  			continue;
// 	//  		}
// 	//  	}
// 	//=====================================================================
// 
// 	return aRetWorldFaceList;
// }
