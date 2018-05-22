//////////////////////////////////////////////////////////////////////
//
// DYCanvasList.cpp: implementation of the DYCanvasList class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DYCanvasList.h"
#include "DYStringList.h"
#include "DYMat3x3.h"
#include "DYPiece.h"
#include "DYCanvas.h"

DYCanvasList::DYCanvasList(const DYString &strDirectory)
{
	Create(strDirectory);
}

DYCanvasList::~DYCanvasList()
{
	Clear();
}

DYCanvasList::DYCanvasList(const vector<DYCanvas> &pCanvases):m_pCanvases(pCanvases)
{
}

DYCanvasList::DYCanvasList(const DYCanvasList &aCanvasList):m_pCanvases(aCanvasList.m_pCanvases)
{
}

DYCanvasList & DYCanvasList::operator = (const vector<DYCanvas> &pCanvases)
{
	m_pCanvases=pCanvases;

	return *this;
}

DYCanvasList & DYCanvasList::operator = (const DYCanvasList &aCanvasList)
{
	if(this==&aCanvasList)
	{
		return *this;
	}

	m_pCanvases=aCanvasList.m_pCanvases;

	return *this;
}

DYCanvas & DYCanvasList::operator [] (const INT nIndex)
{
	assert(nIndex>=0 && nIndex<GetCount());

	return m_pCanvases[nIndex];
}

const DYCanvas & DYCanvasList::operator [] (const INT nIndex) const
{
	assert(nIndex>=0 && nIndex<GetCount());

	return m_pCanvases[nIndex];
}

DYCanvasList & DYCanvasList::operator += (const DYCanvasList &aCanvasList)
{
	const INT nCount=(INT)(aCanvasList.m_pCanvases.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		m_pCanvases.push_back(aCanvasList.m_pCanvases[nIndex]);
	}

	return *this;
}

DYCanvasList DYCanvasList::operator + (const DYCanvasList &aCanvasList) const
{
	DYCanvasList aRetCanvasList(*this);

	const INT nCount=(INT)(aCanvasList.m_pCanvases.size());

	INT nIndex=0;

	for(nIndex=0;nIndex<nCount;nIndex++)
	{
		aRetCanvasList.m_pCanvases.push_back(aCanvasList.m_pCanvases[nIndex]);
	}

	return aRetCanvasList;
}

BOOL DYCanvasList::IsEmpty(VOID) const
{
	return m_pCanvases.empty();
}

INT DYCanvasList::GetCount(VOID) const
{
	return (INT)(m_pCanvases.size());
}

DYFileToMat3x3 DYCanvasList::GetFileToMat3x3(VOID) const
{
	DYFileToMat3x3 mapFileToMat3x3;

	const INT nCanvasCount=GetCount();

	INT nCanvasIndex=0;

	for(nCanvasIndex=0;nCanvasIndex<nCanvasCount;nCanvasIndex++)
	{
		const DYCanvas &aCanvas=m_pCanvases[nCanvasIndex];

		assert(!aCanvas.IsEmpty());
//         if (!(!aCanvas.IsEmpty()))
//         {
//             continue;
//         }

		const INT nPieceCount=aCanvas.Pieces.GetCount();

		INT nPieceIndex=0;

		for(nPieceIndex=0;nPieceIndex<nPieceCount;nPieceIndex++)
		{
			const DYPiece &aPiece=aCanvas.Pieces[nPieceIndex];

			assert(!aPiece.IsEmpty());

			mapFileToMat3x3[aPiece.File]=aPiece.Trans;
		}
	}

	return mapFileToMat3x3;
}

DYFileToID DYCanvasList::GetFileToID(VOID) const
{
	DYFileToID mapFileToID;

	const INT nCanvasCount=GetCount();

	INT nCanvasIndex=0;

	for(nCanvasIndex=0;nCanvasIndex<nCanvasCount;nCanvasIndex++)
	{
		const DYCanvas &aCanvas=m_pCanvases[nCanvasIndex];

		assert(!aCanvas.IsEmpty());
//         if (!(!aCanvas.IsEmpty()))
//         {
//             continue;
//         }

		const INT nPieceCount=aCanvas.Pieces.GetCount();

		INT nPieceIndex=0;

		for(nPieceIndex=0;nPieceIndex<nPieceCount;nPieceIndex++)
		{
			const DYPiece &aPiece=aCanvas.Pieces[nPieceIndex];

			assert(!aPiece.IsEmpty());

			mapFileToID[aPiece.File]=nCanvasIndex+1;
		}
	}

	return mapFileToID;
}

BOOL DYCanvasList::Clear(VOID)
{
	m_pCanvases.clear();

	return TRUE;
}

BOOL DYCanvasList::Append(const DYCanvas &aCanvas)
{
	m_pCanvases.push_back(aCanvas);

	return TRUE;
}

INT DYCanvasList::Add(const DYCanvas &aCanvas)
{
	const INT nMaxIndex=(INT)(m_pCanvases.size());

	m_pCanvases.push_back(aCanvas);

	return nMaxIndex;
}

BOOL DYCanvasList::Insert(const INT nIndex,const DYCanvas &aCanvas)
{
	m_pCanvases.insert(m_pCanvases.begin()+nIndex,aCanvas);

	return TRUE;
}

BOOL DYCanvasList::Erase(const INT nIndex)
{
	assert(nIndex>=0 && nIndex<GetCount());

	m_pCanvases.erase(m_pCanvases.begin()+nIndex);

	return TRUE;
}

BOOL DYCanvasList::Create(const DYString &strDirectory)
{
	if(!Clear())
	{
		return FALSE;
	}

	if(strDirectory.empty())
	{
		return FALSE;
	}

	DYStringList pstrFiles;

	EnumFiles(strDirectory.c_str(),pstrFiles);

	const INT nFilecount=pstrFiles.GetCount();

	INT nFileIndex=0;

	for(nFileIndex=0;nFileIndex<nFilecount;nFileIndex++)
	{
		const DYCanvas aCanvas(pstrFiles[nFileIndex]);

		if(!aCanvas.IsEmpty() && !Append(aCanvas))
		{
			return FALSE;
		}
	}

	if(IsEmpty())
	{
		return FALSE;
	}

	return TRUE;
}

BOOL DYCanvasList::Union(const INT nMaxSize)
{
	if(IsEmpty())
	{
		return FALSE;
	}

	DYCanvasList pCanvases(*this);

	if(pCanvases.IsEmpty())
	{
		return FALSE;
	}

	if(!Clear())
	{
		return FALSE;
	}

	if(!DYCanvasList::Union1_2(*this,pCanvases,nMaxSize,TRUE))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL DYCanvasList::Save(const DYString &strDirectory)
{
	if(IsEmpty() || strDirectory.empty())
	{
		return FALSE;
	}

	CHAR szDirectory[DY_MAX_PATH];

	if(strDirectory[(INT)(strlen(strDirectory.c_str()))-1]!='\\')
	{
		sprintf(szDirectory,DY_TEXT("%s\\"),strDirectory.c_str());
	}
	else
	{
		strcpy(szDirectory,strDirectory.c_str());
	}

	CreateFolder(szDirectory);

	const INT nCanvasCount=GetCount();

	INT nCanvasIndex=0;

	for(nCanvasIndex=0;nCanvasIndex<nCanvasCount;nCanvasIndex++)
	{
		const DYCanvas &aCanvas=m_pCanvases[nCanvasIndex];

		assert(!aCanvas.IsEmpty());
//         if (!(!aCanvas.IsEmpty()))
//         {
//             continue;
//         }

		CHAR szFile[DY_MAX_PATH];

		sprintf(szFile,DY_TEXT(/*"%s%010d.bmp"*/"%s%d.bmp"),szDirectory,nCanvasIndex+1);

		aCanvas.Save(szFile);
	}

	return TRUE;
}

BOOL DYCanvasList::Update(VOID)
{
	const INT nCanvasCount=GetCount();

	INT nCanvasIndex=0;

	for(nCanvasIndex=0;nCanvasIndex<nCanvasCount;nCanvasIndex++)
	{
		DYCanvas &aCanvas=m_pCanvases[nCanvasIndex];

		assert(!aCanvas.IsEmpty());
//         if (!(!aCanvas.IsEmpty()))
//         {
//             continue;
//         }

		const INT nPieceCount=aCanvas.Pieces.GetCount();

		INT nPieceIndex=0;

		for(nPieceIndex=0;nPieceIndex<nPieceCount;nPieceIndex++)
		{
			DYPiece &aPiece=aCanvas.Pieces[nPieceIndex];

			assert(!aPiece.IsEmpty());

			const DOUBLE yScale =((DOUBLE)(aPiece.Height))/((DOUBLE)(aCanvas.Height  ));
			const DOUBLE xScale =((DOUBLE)(aPiece.Width ))/((DOUBLE)(aCanvas.Width   ));
			const DOUBLE yOffset=((DOUBLE)(aPiece.Row   ))/((DOUBLE)(aCanvas.Height));
			const DOUBLE xOffset=((DOUBLE)(aPiece.Col   ))/((DOUBLE)(aCanvas.Width ));

			aPiece.Trans=DYMat3x3(xScale ,0.0    ,0.0,
								  0.0    ,yScale ,0.0,
								  xOffset,yOffset,1.0);

// 			DYMat3x3 Trans1=DYMat3x3(1.0, 0.0, 0.0,
// 				0.0, -1.0 ,0.0,
// 				0.0, 0.0 ,1.0);
// 
// 			aPiece.Trans *= Trans1;
// 
// 			DYMat3x3 Trans2=DYMat3x3(1.0, 0.0, 0.0,
// 				0.0, 1.0 ,0.0,
// 				0.0, 1.0 ,1.0);
// 
// 			aPiece.Trans *= Trans2;
		}
	}

	return TRUE;
}

BOOL DYCanvasList::SortQ(VOID)
{
	const INT nCanvasCount=GetCount();

	if(nCanvasCount<1)
	{
		return FALSE;
	}

	qsort((VOID *)(&(m_pCanvases[0])),nCanvasCount,sizeof(DYCanvas),DYCanvasList::Compare);

	return TRUE;
}

INT DYCanvasList::Compare(const VOID *pElem1,const VOID *pElem2)
{
	const DYCanvas *pCanvas1=(const DYCanvas *)(pElem1);
	const DYCanvas *pCanvas2=(const DYCanvas *)(pElem2);

	assert(pCanvas1!=NULL && pCanvas2!=NULL && !pCanvas1->IsEmpty() && !pCanvas2->IsEmpty());

	if(pCanvas1->Size<pCanvas2->Size)
	{
		return -1;
	}
	else if(pCanvas1->Size>pCanvas2->Size)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

BOOL DYCanvasList::UnionDIR(const DYString &strSrcDir,const DYString &strDstDir,DYFileToMat3x3 &pmapFileToMat3x3,DYFileToID &pmapFileToID,const INT nMaxSize)
{
// 	if(pmapFileToMat3x3!=NULL)
// 	{
// 		pmapFileToMat3x3->clear();
// 	}
	pmapFileToMat3x3.clear();

// 	if(pmapFileToID!=NULL)
// 	{
// 		pmapFileToID->clear();
// 	}
	pmapFileToID.clear();

	DYCanvasList pCanvases(strSrcDir);

	if(pCanvases.IsEmpty())
	{
		return FALSE;
	}

	if(!pCanvases.Union(nMaxSize))
	{
		return FALSE;
	}

	if(!pCanvases.Save(strDstDir))
	{
		return FALSE;
	}

// 	if(pmapFileToMat3x3!=NULL)
// 	{
// 		(*pmapFileToMat3x3)=pCanvases.GetFileToMat3x3();
// 	}
	/*DYFileToMat3x3 mapFileToMat3x3*/pmapFileToMat3x3 = pCanvases.GetFileToMat3x3();

// 	if(pmapFileToID!=NULL)
// 	{
// 		(*pmapFileToID)=pCanvases.GetFileToID();
// 	}
	/*DYFileToID mapFileToID*/pmapFileToID = pCanvases.GetFileToID();

	return TRUE;
}

BOOL DYCanvasList::Union1_2(DYCanvasList &pCanvas2s,const DYCanvasList &pCanvas1s,const INT nMaxSize,const BOOL bClearFirst)
{
	if(bClearFirst && !pCanvas2s.Clear())
	{
		return FALSE;
	}

	const INT nCanvasCount1=pCanvas1s.GetCount();

	if(nCanvasCount1<1)
	{
		return FALSE;
	}

	DYCanvasList pCanvas11s(pCanvas1s),pCanvas12s;

	assert(pCanvas11s.GetCount()==nCanvasCount1 && pCanvas12s.IsEmpty());

	DYCanvasList *ppCanvas11s=&pCanvas11s;
	DYCanvasList *ppCanvas12s=&pCanvas12s;

	assert(ppCanvas11s!=NULL && ppCanvas12s!=NULL);

	#define DY_CANVAS_LIST_UNION_SIZE_COUNT 12

	const INT  nSizeCount=DY_CANVAS_LIST_UNION_SIZE_COUNT;
	const INT  pSizes    [DY_CANVAS_LIST_UNION_SIZE_COUNT]={4,8,16,32,64,128,256,512,1024,2048,4096,8192};

	INT nLoopIndex=0;

	for(nLoopIndex=0;nLoopIndex<2;nLoopIndex++)
	{
		INT nSizeIndex=0;

		for(nSizeIndex=0;nSizeIndex<nSizeCount;nSizeIndex++)
		{
			const INT nCanvasCount11=ppCanvas11s->GetCount();

			if(nCanvasCount11<2)
			{
				break;
			}

			const INT nSize=pSizes[nSizeIndex];

			assert(::Normalize(nSize)==nSize);

			if((nSize<<1)>nMaxSize)
			{
				break;
			}

			if(!ppCanvas12s->Clear())
			{
				return FALSE;
			}

			DYCanvasList pCanvas111s,pCanvas112s;

			INT nCanvasIndex11=0;

			for(nCanvasIndex11=0;nCanvasIndex11<nCanvasCount11;nCanvasIndex11++)
			{
				const DYCanvas &aCanvas11=(*ppCanvas11s)[nCanvasIndex11];

				assert(!aCanvas11.IsEmpty());
//                 if (!(!aCanvas11.IsEmpty()))
//                 {
//                     continue;
//                 }

				if(aCanvas11.Height==nSize && aCanvas11.Width==nSize)
				{
					if(!pCanvas111s.Append(aCanvas11))
					{
						return FALSE;
					}

					if(pCanvas111s.GetCount()<4)
					{
						continue;
					}

					const INT nCanvasCount111=pCanvas111s.GetCount();

					assert(nCanvasCount111==4);

					DYCanvas aCanvas112(DYPieceList(),(nSize<<1),(nSize<<1));

					assert(aCanvas112.IsEmpty());

					INT nCanvasIndex111=0;

					for(nCanvasIndex111=0;nCanvasIndex111<nCanvasCount111;nCanvasIndex111++)
					{
						DYCanvas &aCanvas111=pCanvas111s[nCanvasIndex111];

						assert(!aCanvas111.IsEmpty() && aCanvas111.Height==nSize && aCanvas111.Width==nSize);

						const INT dRow=(nCanvasIndex111 >>1)*nSize;
						const INT dCol=(nCanvasIndex111 & 1)*nSize;

						const INT nPieceCount111=aCanvas111.Pieces.GetCount();

						INT nPieceIndex111=0;

						for(nPieceIndex111=0;nPieceIndex111<nPieceCount111;nPieceIndex111++)
						{
							DYPiece &aPiece111=aCanvas111.Pieces[nPieceIndex111];

							assert(!aPiece111.IsEmpty());

							aPiece111.Row+=dRow;
							aPiece111.Col+=dCol;
						}

						aCanvas112.Pieces+=aCanvas111.Pieces;
					}

					assert(!aCanvas112.IsEmpty());

					if(!pCanvas112s.Append(aCanvas112))
					{
						return FALSE;
					}

					if(!pCanvas111s.Clear())
					{
						return FALSE;
					}
				}
				else
				{
					if(!pCanvas112s.Append(aCanvas11))
					{
						return FALSE;
					}
				}
			}

			const INT nCanvasCount111=pCanvas111s.GetCount();

			assert(nCanvasCount111<4);

//			if(nCanvasCount111==3)
			if(0)
			{
				DYCanvas aCanvas112(DYPieceList(),(nSize<<1),(nSize<<1));

				assert(aCanvas112.IsEmpty());

				INT nCanvasIndex111=0;

				for(nCanvasIndex111=0;nCanvasIndex111<nCanvasCount111;nCanvasIndex111++)
				{
					DYCanvas &aCanvas111=pCanvas111s[nCanvasIndex111];

					assert(!aCanvas111.IsEmpty() && aCanvas111.Height==nSize && aCanvas111.Width==nSize);

					const INT dRow=(nCanvasIndex111 >>1)*nSize;
					const INT dCol=(nCanvasIndex111 & 1)*nSize;

					const INT nPieceCount111=aCanvas111.Pieces.GetCount();

					INT nPieceIndex111=0;

					for(nPieceIndex111=0;nPieceIndex111<nPieceCount111;nPieceIndex111++)
					{
						DYPiece &aPiece111=aCanvas111.Pieces[nPieceIndex111];

						assert(!aPiece111.IsEmpty());

						aPiece111.Row+=dRow;
						aPiece111.Col+=dCol;
					}

					aCanvas112.Pieces+=aCanvas111.Pieces;
				}

				assert(!aCanvas112.IsEmpty());

				if(!pCanvas112s.Append(aCanvas112))
				{
					return FALSE;
				}

				if(!pCanvas111s.Clear())
				{
					return FALSE;
				}
			}

			pCanvas112s+=pCanvas111s;

			if(!pCanvas111s.Clear())
			{
				return FALSE;
			}

			const INT nCanvasCount112=pCanvas112s.GetCount();

			assert(nCanvasCount112>0);
//             if (!(nCanvasCount112>0))
//             {
//                 continue;
//             }

			INT *pProcFlag112s=new INT[nCanvasCount112];

			assert(pProcFlag112s!=NULL);

			memset(pProcFlag112s,0,nCanvasCount112*sizeof(INT));

			INT nCanvasIndex1121=0;

			for(nCanvasIndex1121=0;nCanvasIndex1121<nCanvasCount112;nCanvasIndex1121++)
			{
				if(pProcFlag112s[nCanvasIndex1121])
				{
					continue;
				}

				DYCanvas &aCanvas1121=pCanvas112s[nCanvasIndex1121];

				assert(!aCanvas1121.IsEmpty());

				if(aCanvas1121.Height!=nSize && aCanvas1121.Width!=nSize)
				{
					if(!ppCanvas12s->Append(aCanvas1121))
					{
						DY_DELETE_ARR(pProcFlag112s);

						return FALSE;
					}

					pProcFlag112s[nCanvasIndex1121]=1;

					continue;
				}

				if(aCanvas1121.Height==nSize && aCanvas1121.Width==nSize)
				{
					if(!pCanvas111s.Append(aCanvas1121))
					{
						DY_DELETE_ARR(pProcFlag112s);

						return FALSE;
					}

					pProcFlag112s[nCanvasIndex1121]=1;

					continue;
				}

				INT nCanvasIndex1122=nCanvasIndex1121+1;

				for(nCanvasIndex1122=nCanvasIndex1121+1;nCanvasIndex1122<nCanvasCount112;nCanvasIndex1122++)
				{
					if(pProcFlag112s[nCanvasIndex1122])
					{
						continue;
					}

					DYCanvas &aCanvas1122=pCanvas112s[nCanvasIndex1122];

					assert(!aCanvas1122.IsEmpty());

					if(aCanvas1122.Height!=aCanvas1121.Height || aCanvas1122.Width!=aCanvas1121.Width)
					{
						continue;
					}

					if(aCanvas1121.Height==nSize)
					{
						assert(aCanvas1121.Height==nSize && aCanvas1121.Width!=nSize);

						aCanvas1121.Height<<=1;

						const INT nPieceCount1122=aCanvas1122.Pieces.GetCount();

						INT nPieceIndex1122=0;

						for(nPieceIndex1122=0;nPieceIndex1122<nPieceCount1122;nPieceIndex1122++)
						{
							DYPiece &aPiece1122=aCanvas1122.Pieces[nPieceIndex1122];

							assert(!aPiece1122.IsEmpty());

							aPiece1122.Row+=nSize;
						}

						aCanvas1121.Pieces+=aCanvas1122.Pieces;
					}
					else
					{
						assert(aCanvas1121.Height!=nSize && aCanvas1121.Width==nSize);

						aCanvas1121.Width<<=1;

						const INT nPieceCount1122=aCanvas1122.Pieces.GetCount();

						INT nPieceIndex1122=0;

						for(nPieceIndex1122=0;nPieceIndex1122<nPieceCount1122;nPieceIndex1122++)
						{
							DYPiece &aPiece1122=aCanvas1122.Pieces[nPieceIndex1122];

							assert(!aPiece1122.IsEmpty());

							aPiece1122.Col+=nSize;
						}

						aCanvas1121.Pieces+=aCanvas1122.Pieces;
					}

					assert(!aCanvas1121.IsEmpty());

					if(!ppCanvas12s->Append(aCanvas1121))
					{
						DY_DELETE_ARR(pProcFlag112s);

						return FALSE;
					}

					pProcFlag112s[nCanvasIndex1121]=1;
					pProcFlag112s[nCanvasIndex1122]=1;

					break;
				}

				if(pProcFlag112s[nCanvasIndex1121])
				{
					continue;
				}

				if(!pCanvas111s.Append(aCanvas1121))
				{
					DY_DELETE_ARR(pProcFlag112s);

					return FALSE;
				}

				pProcFlag112s[nCanvasIndex1121]=1;
			}

			DY_DELETE_ARR(pProcFlag112s);

			if(pCanvas111s.IsEmpty())
			{
				DY_SWAP(ppCanvas11s,ppCanvas12s,DYCanvasList *);

				continue;
			}

			INT nIndex=0;

			for(nIndex=0;nIndex<2;nIndex++)
			{
				DYCanvasList pCanvas113s;

				const INT nCanvasCount111=pCanvas111s.GetCount();

				INT nCanvasIndex111=nCanvasCount111-1;

				for(nCanvasIndex111=nCanvasCount111-1;nCanvasIndex111>=0;nCanvasIndex111--)
				{
					const DYCanvas &aCanvas111=pCanvas111s[nCanvasIndex111];

					assert(!aCanvas111.IsEmpty() && (aCanvas111.Height==nSize || aCanvas111.Width==nSize));
//                     if (!(!aCanvas111.IsEmpty() && (aCanvas111.Height==nSize || aCanvas111.Width==nSize)))
//                     {
//                         continue;
//                     }

					if(aCanvas111.Sizes[nIndex]!=nSize)
					{
						continue;
					}

					if(!pCanvas113s.Append(aCanvas111))
					{
						return FALSE;
					}

					if(!pCanvas111s.Erase(nCanvasIndex111))
					{
						return FALSE;
					}
				}

				const INT nCanvasCount113=pCanvas113s.GetCount();

				if(nCanvasCount113<2)
				{
					pCanvas111s+=pCanvas113s;

					continue;
				}

				if(!pCanvas113s.SortQ())
				{
					return FALSE;
				}

				assert(nCanvasCount113>1);

				INT nSize1=pCanvas113s[nCanvasCount113-1].Sizes[1-nIndex];
				INT nSize2=pCanvas113s[nCanvasCount113-2].Sizes[1-nIndex];

				if((nSize2<<1)<(nSize1))
				{
					pCanvas111s+=pCanvas113s;

					continue;
				}

				if(nCanvasCount113==2)
				{
					pCanvas111s+=pCanvas113s;

					continue;
				}

				INT nCanvasIndex113=nCanvasCount113-3;

				for(nCanvasIndex113=nCanvasCount113-3;nCanvasIndex113>=0;nCanvasIndex113--)
				{
					nSize2+=pCanvas113s[nCanvasIndex113].Sizes[1-nIndex];
				}

				if(nLoopIndex==0)
				{
					if(nSize2<nSize1)
					{
						pCanvas111s+=pCanvas113s;

						continue;
					}
				}
				else
				{
					if((nSize2)<((nSize1>>2)*3))
					{
						pCanvas111s+=pCanvas113s;

						continue;
					}
				}

				DYCanvas aCanvas12(pCanvas113s[nCanvasCount113-1]);

				assert(!aCanvas12.IsEmpty());

				if(!pCanvas113s.Erase(nCanvasCount113-1))
				{
					return FALSE;
				}

				assert(aCanvas12.Sizes[nIndex]==nSize);

				aCanvas12.Sizes[nIndex]<<=1;

				INT rSize=aCanvas12.Sizes[1-nIndex];
				INT dSize=0;

				nCanvasIndex113=nCanvasCount113-2;

				for(nCanvasIndex113=nCanvasCount113-2;nCanvasIndex113>=0;nCanvasIndex113--)
				{
					DYCanvas &aCanvas113=pCanvas113s[nCanvasIndex113];

					assert(!aCanvas113.IsEmpty() && aCanvas113.Sizes[nIndex]==nSize && aCanvas113.Sizes[1-nIndex]<=aCanvas12.Sizes[1-nIndex]);

					if(rSize<aCanvas113.Sizes[1-nIndex])
					{
						continue;
					}

					const INT nPieceCount113=aCanvas113.Pieces.GetCount();

					INT nPieceIndex113=0;

					for(nPieceIndex113=0;nPieceIndex113<nPieceCount113;nPieceIndex113++)
					{
						DYPiece &aPiece113=aCanvas113.Pieces[nPieceIndex113];

						assert(!aPiece113.IsEmpty());

						aPiece113.Poses[  nIndex]+=nSize;
						aPiece113.Poses[1-nIndex]+=dSize;
					}

					aCanvas12.Pieces+=aCanvas113.Pieces;

					rSize-=aCanvas113.Sizes[1-nIndex];
					dSize+=aCanvas113.Sizes[1-nIndex];

					if(!pCanvas113s.Erase(nCanvasIndex113))
					{
						return FALSE;
					}
				}

				assert(!aCanvas12.IsEmpty());

				if(!ppCanvas12s->Append(aCanvas12))
				{
					return FALSE;
				}

				pCanvas111s+=pCanvas113s;
			}

			(*ppCanvas12s)+=pCanvas111s;

			DY_SWAP(ppCanvas11s,ppCanvas12s,DYCanvasList *);
		}
	}

	pCanvas2s+=(*ppCanvas11s);

	if(pCanvas2s.IsEmpty())
	{
		return FALSE;
	}

	if(!pCanvas2s.Update())
	{
		return FALSE;
	}

	return TRUE;
}
