//////////////////////////////////////////////////////////////////////
//
// DYSimpLink.h: interface for the DYSimpLink class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DYSIMPLINK_H__B2DA7CB0_CD67_4231_BD90_848FDC0B9DB6__INCLUDED_)
#define AFX_DYSIMPLINK_H__B2DA7CB0_CD67_4231_BD90_848FDC0B9DB6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DYDefine.h"

#define DY_SIMP_LINK_IS_EMPTY(aSimpLink) ((aSimpLink).Head==NULL)  // test simply

#define DY_SIMP_LINK_ADD_NODE_H(aSimpLink,pCurr)\
{\
	assert((pCurr)!=NULL);\
\
	if((aSimpLink).Head!=NULL)\
	{\
		assert((aSimpLink).Tail!=NULL && (aSimpLink).Head->Prev==NULL && (aSimpLink).Tail->Next==NULL);\
\
		(aSimpLink).Head ->Prev=(pCurr);\
		          (pCurr)->Next=(aSimpLink).Head;\
\
		(aSimpLink).Head=(pCurr);\
	}\
	else\
	{\
		assert((aSimpLink).Tail==NULL);\
\
		(aSimpLink).Head=(pCurr);\
		(aSimpLink).Tail=(pCurr);\
	}\
}

#define DY_SIMP_LINK_ADD_NODE_T(aSimpLink,pCurr)\
{\
	assert((pCurr)!=NULL);\
\
	if((aSimpLink).Tail!=NULL)\
	{\
		assert((aSimpLink).Head!=NULL && (aSimpLink).Head->Prev==NULL && (aSimpLink).Tail->Next==NULL);\
\
		          (pCurr)->Prev=(aSimpLink).Tail;\
		(aSimpLink).Tail ->Next=(pCurr);\
\
		(aSimpLink).Tail=(pCurr);\
	}\
	else\
	{\
		assert((aSimpLink).Head==NULL);\
\
		(aSimpLink).Head=(pCurr);\
		(aSimpLink).Tail=(pCurr);\
	}\
}

#define DY_SIMP_LINK_SUB_NODE(aSimpLink,pCurr)\
{\
	assert((aSimpLink).Head!=NULL && (aSimpLink).Tail!=NULL && (aSimpLink).Head->Prev==NULL && (aSimpLink).Tail->Next==NULL && (pCurr)!=NULL);\
\
	if((pCurr)==(aSimpLink).Head)\
	{\
		(aSimpLink).Head=(aSimpLink).Head->Next;\
\
		if((aSimpLink).Head!=NULL)\
		{\
			(aSimpLink).Head->Prev=NULL;\
		}\
		else\
		{\
			(aSimpLink).Tail=NULL;\
		}\
\
		(pCurr)->Next=NULL;\
	}\
	else if((pCurr)==(aSimpLink).Tail)\
	{\
		(aSimpLink).Tail=(aSimpLink).Tail->Prev;\
\
		if((aSimpLink).Tail!=NULL)\
		{\
			(aSimpLink).Tail->Next=NULL;\
		}\
		else\
		{\
			(aSimpLink).Head=NULL;\
		}\
\
		(pCurr)->Prev=NULL;\
	}\
	else\
	{\
		assert((pCurr)->Prev!=NULL && (pCurr)->Next!=NULL);\
\
		(pCurr)->Prev->Next=(pCurr)->Next;\
		(pCurr)->Next->Prev=(pCurr)->Prev;\
\
		(pCurr)->Prev=NULL;\
		(pCurr)->Next=NULL;\
	}\
}

#define DY_SIMP_LINK_SUB_NODE_H(aSimpLink,ppCurr)\
{\
	assert((aSimpLink).Head!=NULL && (aSimpLink).Tail!=NULL && (aSimpLink).Head->Prev==NULL && (aSimpLink).Tail->Next==NULL && (ppCurr)!=NULL && (*(ppCurr))==NULL);\
\
	(*(ppCurr))=(aSimpLink).Head;\
\
	(aSimpLink).Head=(aSimpLink).Head->Next;\
\
	if((aSimpLink).Head!=NULL)\
	{\
		(aSimpLink).Head->Prev=NULL;\
	}\
	else\
	{\
		(aSimpLink).Tail=NULL;\
	}\
\
	(*(ppCurr))->Next=NULL;\
}

#define DY_SIMP_LINK_SUB_NODE_T(aSimpLink,ppCurr)\
{\
	assert((aSimpLink).Head!=NULL && (aSimpLink).Tail!=NULL && (aSimpLink).Head->Prev==NULL && (aSimpLink).Tail->Next==NULL && (ppCurr)!=NULL && (*(ppCurr))==NULL);\
\
	(*(ppCurr))=(aSimpLink).Tail;\
\
	(aSimpLink).Tail=(aSimpLink).Tail->Prev;\
\
	if((aSimpLink).Tail!=NULL)\
	{\
		(aSimpLink).Tail->Next=NULL;\
	}\
	else\
	{\
		(aSimpLink).Head=NULL;\
	}\
\
	(*(ppCurr))->Prev=NULL;\
}

class DYSimpNode;

class DYSimpLink
{
public:
	DYSimpNode *Head;
	DYSimpNode *Tail;

public:
	DYSimpLink(const INT nLength=0);
	virtual ~DYSimpLink();

public:
	DYSimpLink(const DYSimpLink &aSimpLink);
	DYSimpLink & operator = (const DYSimpLink &aSimpLink);

public:
	BOOL IsEmpty  (VOID) const;               // test fully
	BOOL IsExist  (DYSimpNode *pNode) const;  // very slow
	INT  GetLength(VOID) const;               // very slow

public:
	BOOL Create   (const INT nLength=0);
	BOOL Clear    (VOID);

	BOOL AddNodeH (DYSimpNode *pCurr);
	BOOL AddNodeT (DYSimpNode *pCurr);
	BOOL SubNode  (DYSimpNode *pCurr);
	BOOL SubNodeH (DYSimpNode **ppCurr);
	BOOL SubNodeT (DYSimpNode **ppCurr);

protected:
	BOOL Clone    (const DYSimpLink &aSimpLink);
	BOOL Reset    (VOID);
	BOOL Release  (VOID);
};

#endif // !defined(AFX_DYSIMPLINK_H__B2DA7CB0_CD67_4231_BD90_848FDC0B9DB6__INCLUDED_)
