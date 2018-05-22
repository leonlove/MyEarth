//////////////////////////////////////////////////////////////////////
//
// DYVertMap.cpp: implementation of the DYVertMap class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DYVertMap.h"
#include "DYIntList.h"
#include "DYSimpNode.h"
#include "DYSimpLink.h"

// #ifdef _DEBUG
// #undef THIS_FILE
// static char THIS_FILE[]=__FILE__;
// #define new DEBUG_NEW
// #endif

DYVertMap::DYVertMap()
{
	Clear();
}

DYVertMap::~DYVertMap()
{
	Clear();
}

DYVertMap::DYVertMap(const DYVertMap &aVertMap):m_pVerts(aVertMap.m_pVerts),m_pIndexes(aVertMap.m_pIndexes)
{
}

DYVertMap & DYVertMap::operator = (const DYVertMap &aVertMap)
{
	if(this==&aVertMap)
	{
		return *this;
	}

	m_pVerts  =aVertMap.m_pVerts;
	m_pIndexes=aVertMap.m_pIndexes;

	return *this;
}

const DYVert & DYVertMap::operator [] (const INT nIndex) const
{
	assert(nIndex>=0 && nIndex<GetCount());

	return m_pVerts[nIndex];
}

INT DYVertMap::operator [] (const DYVert &aVert) const
{
	map<DYVert,INT>::const_iterator itIndexes=m_pIndexes.find(aVert);

	if(itIndexes!=m_pIndexes.end())
	{
		return itIndexes->second;
	}
	else
	{
		return -1;
	}
}

BOOL DYVertMap::IsEmpty(VOID) const
{
	return (m_pVerts.empty() || m_pIndexes.empty());
}

INT DYVertMap::GetCount(VOID) const
{
	return (INT)(m_pVerts.size());
}

BOOL DYVertMap::Clear(VOID)
{
	m_pVerts  .clear();
	m_pIndexes.clear();
	m_pNormal .clear();

	return TRUE;
}

INT DYVertMap::Append(const DYVert &aVert)
{
	assert((*this)[aVert]<0);

	const INT nMaxIndex=(INT)(m_pVerts.size());

	m_pVerts  .push_back(aVert);
	m_pIndexes.insert(pair<DYVert,INT>(aVert,nMaxIndex));

	return nMaxIndex;
}

INT DYVertMap::Add(const DYVert &aVert)
{
// 	const INT nIndex=(*this)[aVert];
// 	if(nIndex>-1)
// 	{
// 		return nIndex;
// 	}
// 
// 	const INT nMaxIndex=(INT)(m_pVerts.size());
// 
// 	m_pVerts  .push_back(aVert);
// 	m_pIndexes.insert(pair<DYVert,INT>(aVert,nMaxIndex));
// 
// 	return nMaxIndex;

//     const INT nMaxIndex=(INT)(m_pVerts.size());
//     std::pair< std::map< DYVert, INT>::iterator, bool > ret = m_pIndexes.insert(pair<DYVert,INT>(aVert,nMaxIndex));
//     if (!ret.second)
//     {
//         return ret.first->second;
//     }
// 
//     m_pVerts.push_back(aVert);
// 
//     return nMaxIndex;


	const INT nMaxIndex=(INT)(m_pVerts.size());
	std::pair< std::map< DYVert, INT>::iterator, bool > ret = m_pIndexes.insert(pair<DYVert,INT>(aVert,nMaxIndex));
	if (!ret.second)
	{
		map<INT, pair<INT, DYVector3F> >::iterator itr = m_pNormal.find(ret.first->second);
		itr->second.first++;
		itr->second.second += aVert.Normal;

		return ret.first->second;
	}

	m_pVerts.push_back(aVert);

	DYVector3F DYNormal = ret.first->first.Normal;
	m_pNormal.insert(make_pair(nMaxIndex, pair<INT,DYVector3F>(1,DYNormal)));

	return nMaxIndex;
}

BOOL DYVertMap::Erase(const DYVert &aVert)
{
	map<DYVert,INT>::iterator itIndexes=m_pIndexes.find(aVert);

	if(itIndexes==m_pIndexes.end())
	{
		return FALSE;
	}

	const INT nIndex=itIndexes->second;

	m_pVerts  .erase(m_pVerts.begin()+nIndex);
	m_pIndexes.erase(itIndexes);

	if(!OnErase(nIndex))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL DYVertMap::Erase(const INT nIndex)
{
	assert(nIndex>=0 && nIndex<GetCount());

	vector<DYVert> ::iterator itVerts  =m_pVerts.begin()+nIndex;
	map<DYVert,INT>::iterator itIndexes=m_pIndexes.find(*itVerts);

	assert(itVerts!=m_pVerts.end() && itIndexes!=m_pIndexes.end());

	m_pVerts  .erase(itVerts);
	m_pIndexes.erase(itIndexes);

	if(!OnErase(nIndex))
	{
		return FALSE;
	}

	return TRUE;
}

INT DYVertMap::Alter(const INT nIndex,const DYVert &aVert)
{
	assert(nIndex>=0 && nIndex<GetCount());

	if(m_pVerts[nIndex]==aVert)
	{
		return nIndex;
	}

	const INT nNewIndex=(*this)[aVert];

	if(nNewIndex>-1)
	{
		if(!Erase(nIndex))
		{
			return nNewIndex;
		}

		return nNewIndex;
	}

	m_pIndexes.erase(m_pIndexes.find(m_pVerts[nIndex]));

	m_pVerts[nIndex]=aVert;

	m_pIndexes.insert(pair<DYVert,INT>(aVert,nIndex));

	return nIndex;
}

VOID DYVertMap::CalcRepeatNormal(VOID)
{
	map<INT, pair<INT, DYVector3F> >::iterator itr = m_pNormal.begin();
	for (INT i=0; itr!=m_pNormal.end(); i++,itr++)
	{
		if (itr->second.first <= 1)
		{
			continue;
		}

		m_pVerts[i].Normal = itr->second.second / itr->second.first;
	}
}

BOOL DYVertMap::Simplify(DYVertMap &aNewVertMap,DYIntList &aNewIndexList,const FLOAT fAlpha,const FLOAT fBeta,const FLOAT fGamma,const FLOAT fDelta,const INT nBias) const
{
	if(!aNewVertMap.Clear() || !aNewIndexList.Clear())
	{
		return FALSE;
	}

	const INT nPointCount=GetCount();

	if(nPointCount<1)
	{
		return FALSE;
	}

	const DOUBLE fAlpha_2=(DOUBLE)(fAlpha);
	const DOUBLE fBeta_2 =(DOUBLE)(fBeta);
	const DOUBLE fGamma_2=(DOUBLE)(fGamma);
	const DOUBLE fDelta_2=(DOUBLE)(fDelta);
	const INT    nBias_2 =DY_MAX(nBias,1);

	DYSimpLink aSimpLink(nPointCount);

	assert(!DY_SIMP_LINK_IS_EMPTY(aSimpLink) && aSimpLink.GetLength()==nPointCount);

	DYSimpNode *pCurr1=aSimpLink.Head;

	while(pCurr1!=NULL)
	{
		const DYVector3F &vPoint1=(*this)[pCurr1->Index].Point;

		DYVector3F vSummation2(0.0f,0.0f,0.0f);
		INT        nPointCount2=0;

		vector<DYSimpNode *> ppCurr2s;

		DYSimpNode *pCurr2=aSimpLink.Head;

		while(pCurr2!=NULL)
		{
			if(pCurr2==pCurr1)
			{
				pCurr2=pCurr2->Next;

				continue;
			}

			const DYVector3F &vPoint2=(*this)[pCurr2->Index].Point;

			const DOUBLE fDist2=vPoint1.Dist2_2(vPoint2);

			if(fDist2<=fAlpha_2)
			{
				ppCurr2s.push_back(pCurr2);

				vSummation2+=vPoint2;
				nPointCount2++;
			}

			pCurr2=pCurr2->Next;
		}

		if(nPointCount2<nBias_2)
		{
			pCurr1=pCurr1->Next;

			continue;
		}

		const DYVector3F &vCenter2=vSummation2/((FLOAT)(nPointCount2));

		const DOUBLE fDist2=vPoint1.Dist2_2(vCenter2);

		if(fDist2>fBeta_2)
		{
			pCurr1=pCurr1->Next;

			continue;
		}

		DYSimpLink aSimpLink2;

		assert(DY_SIMP_LINK_IS_EMPTY(aSimpLink2));

		INT nIndex2=0;

		for(nIndex2=0;nIndex2<nPointCount2;nIndex2++)
		{
			DYSimpNode *pCurr2=ppCurr2s[nIndex2];

			assert(pCurr2!=NULL);

			DY_SIMP_LINK_SUB_NODE(aSimpLink,pCurr2);

			DY_SIMP_LINK_ADD_NODE_T(aSimpLink2,pCurr2);
		}

		DYVector3F vSummation3(0.0f,0.0f,0.0f);
		INT        nPointCount3=0;
		DYSimpLink aSimpLink3;

		assert(DY_SIMP_LINK_IS_EMPTY(aSimpLink3));

		DYVector3F *pvSummation2=&vSummation2;
		DYVector3F *pvSummation3=&vSummation3;
		INT        *pPointCount2=&nPointCount2;
		INT        *pPointCount3=&nPointCount3;
		DYSimpLink *pSimpLink2  =&aSimpLink2;
		DYSimpLink *pSimpLink3  =&aSimpLink3;

		assert(pvSummation2!=NULL && pvSummation3!=NULL && pPointCount2!=NULL && pPointCount3!=NULL && pSimpLink2!=NULL && pSimpLink3!=NULL);

		while(!DY_SIMP_LINK_IS_EMPTY(*pSimpLink2))
		{
			pSimpLink3->Clear();

			(*pvSummation3)=(*pvSummation2);
			(*pPointCount3)=(*pPointCount2);

			map<DYSimpNode *,INT> mapCurr3s;

			DYSimpNode *pCurr2=aSimpLink2.Head;

			while(pCurr2!=NULL)
			{
				const DYVector3F &vPoint2=(*this)[pCurr2->Index].Point;

				DYSimpNode *pCurr3=aSimpLink.Head;

				while(pCurr3!=NULL)
				{
					if(pCurr3==pCurr1)
					{
						pCurr3=pCurr3->Next;

						continue;
					}

					const DYVector3F &vPoint3=(*this)[pCurr3->Index].Point;

					const DOUBLE fDist2=vPoint2.Dist2_2(vPoint3);

					if(fDist2<=fGamma_2)
					{
						if(mapCurr3s.count(pCurr3)>0)
						{
							pCurr3=pCurr3->Next;

							continue;
						}

						mapCurr3s[pCurr3]=1;

						(*pvSummation3)+=vPoint3;
						(*pPointCount3)++;
					}

					pCurr3=pCurr3->Next;
				}

				pCurr2=pCurr2->Next;
			}

			if(mapCurr3s.empty())
			{
				DY_SWAP(pvSummation2,pvSummation3,DYVector3F *);
				DY_SWAP(pPointCount2,pPointCount3,INT        *);
				DY_SWAP(pSimpLink2  ,pSimpLink3  ,DYSimpLink *);

				break;
			}

			const DYVector3F &vCenter3=(*pvSummation3)/((FLOAT)((*pPointCount3)));

			const DOUBLE fDist2=vPoint1.Dist2_2(vCenter3);

			if(fDist2>fDelta_2)
			{
				DY_SWAP(pvSummation2,pvSummation3,DYVector3F *);
				DY_SWAP(pPointCount2,pPointCount3,INT        *);
				DY_SWAP(pSimpLink2  ,pSimpLink3  ,DYSimpLink *);

				break;
			}

			map<DYSimpNode *,INT>::iterator itCurr3s=mapCurr3s.begin();

			for(;itCurr3s!=mapCurr3s.end();itCurr3s++)
			{
				DYSimpNode *pCurr3=itCurr3s->first;

				assert(pCurr3!=NULL);

				DY_SIMP_LINK_SUB_NODE(aSimpLink,pCurr3);

				DY_SIMP_LINK_ADD_NODE_T((*pSimpLink3),pCurr3);
			}

			DY_SWAP(pvSummation2,pvSummation3,DYVector3F *);
			DY_SWAP(pPointCount2,pPointCount3,INT        *);
			DY_SWAP(pSimpLink2  ,pSimpLink3  ,DYSimpLink *);
		}

		aNewVertMap.Add((*this)[pCurr1->Index]);

		DYSimpNode *pNext1=pCurr1->Next;

		DY_SIMP_LINK_SUB_NODE(aSimpLink,pCurr1);

		DY_DELETE_PTR(pCurr1);

		pCurr1=pNext1;
	}

	pCurr1=aSimpLink.Head;

	while(pCurr1!=NULL)
	{
//		const DYVector3F &vPoint1=(*this)[pCurr1->Index].Point;

		aNewVertMap.Add((*this)[pCurr1->Index]);

		pCurr1=pCurr1->Next;
	}

	const INT nPointCountN=aNewVertMap.GetCount();

	if(nPointCountN<1)
	{
		return FALSE;
	}

	INT nPointIndex=0;

	for(nPointIndex=0;nPointIndex<nPointCount;nPointIndex++)
	{
		const DYVector3F &vPoint=(*this)[nPointIndex].Point;

		DOUBLE fMinDist2      =DY_DOUBLE_INFINITY_PLUS;
		INT    nMinPointIndexN=-1;

		INT nPointIndexN=0;

		for(nPointIndexN=0;nPointIndexN<nPointCountN;nPointIndexN++)
		{
			const DYVector3F &vPointN=aNewVertMap[nPointIndexN].Point;

			const DOUBLE fDist2=vPoint.Dist2_2(vPointN);

			if(fMinDist2>fDist2)
			{
				fMinDist2      =fDist2;
				nMinPointIndexN=nPointIndexN;
			}
		}

		assert(nMinPointIndexN>=0 && nMinPointIndexN<=nPointCountN);

		if(!aNewIndexList.Append(nMinPointIndexN))
		{
			return FALSE;
		}
	}

	assert(aNewIndexList.GetCount()==nPointCount);

	return TRUE;
}

BOOL DYVertMap::OnErase(const INT nIndex)
{
	map<DYVert,INT>::iterator itIndexes=m_pIndexes.begin();

	for(;itIndexes!=m_pIndexes.end();itIndexes++)
	{
		if(itIndexes->second>nIndex)
		{
			itIndexes->second--;
		}
	}

	return TRUE;
}
