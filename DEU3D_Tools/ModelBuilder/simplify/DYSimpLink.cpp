//////////////////////////////////////////////////////////////////////
//
// DYSimpLink.cpp: implementation of the DYSimpLink class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DYSimpLink.h"
#include "DYSimpNode.h"

DYSimpLink::DYSimpLink(const INT nLength)
{
	Reset();

	Create(nLength);
}

DYSimpLink::~DYSimpLink()
{
	Release();
}

DYSimpLink::DYSimpLink(const DYSimpLink &aSimpLink)
{
	Reset();

	Clone(aSimpLink);
}

DYSimpLink & DYSimpLink::operator = (const DYSimpLink &aSimpLink)
{
	if(this==&aSimpLink)
	{
		return *this;
	}

	Clone(aSimpLink);

	return *this;
}

BOOL DYSimpLink::IsEmpty(VOID) const
{
	return (Head==NULL || Tail==NULL || Head->Prev!=NULL || Tail->Next!=NULL);
}

BOOL DYSimpLink::IsExist(DYSimpNode *pNode) const
{
	assert(pNode!=NULL);

	DYSimpNode *pCurr=Head;

	while(pCurr!=NULL)
	{
		if(pCurr==pNode)
		{
			return TRUE;
		}

		pCurr=pCurr->Next;
	}

	return FALSE;
}

INT DYSimpLink::GetLength(VOID) const
{
	INT nLength=0;

	DYSimpNode *pCurr=Head;

	while(pCurr!=NULL)
	{
		nLength++;

		pCurr=pCurr->Next;
	}

	return nLength;
}

BOOL DYSimpLink::Create(const INT nLength)
{
	if(!Release())
	{
		return FALSE;
	}

	if(nLength<1)
	{
		return FALSE;
	}

	Head=new DYSimpNode(0,NULL,NULL);

	assert(Head!=NULL);

	Tail=Head;

	INT nIndex=1;

	for(nIndex=1;nIndex<nLength;nIndex++)
	{
		DYSimpNode *pTail=new DYSimpNode(nIndex,Tail,NULL);

		assert(pTail!=NULL);

		Tail->Next=pTail;

		Tail=pTail;
	}

	return TRUE;
}

BOOL DYSimpLink::Clear(VOID)
{
	return Release();
}

BOOL DYSimpLink::AddNodeH(DYSimpNode *pCurr)
{
	assert(pCurr!=NULL);

	if(Head!=NULL)
	{
		assert(Tail!=NULL && Head->Prev==NULL && Tail->Next==NULL);

		Head ->Prev=pCurr;
		pCurr->Next=Head;

		Head=pCurr;
	}
	else
	{
		assert(Tail==NULL);

		Head=pCurr;
		Tail=pCurr;
	}

	return TRUE;
}

BOOL DYSimpLink::AddNodeT(DYSimpNode *pCurr)
{
	assert(pCurr!=NULL);

	if(Tail!=NULL)
	{
		assert(Head!=NULL && Head->Prev==NULL && Tail->Next==NULL);

		pCurr->Prev=Tail;
		Tail ->Next=pCurr;

		Tail=pCurr;
	}
	else
	{
		assert(Head==NULL);

		Head=pCurr;
		Tail=pCurr;
	}

	return TRUE;
}

BOOL DYSimpLink::SubNode(DYSimpNode *pCurr)
{
	assert(Head!=NULL && Tail!=NULL && Head->Prev==NULL && Tail->Next==NULL && pCurr!=NULL);

	if(pCurr==Head)
	{
		Head=Head->Next;

		if(Head!=NULL)
		{
			Head->Prev=NULL;
		}
		else
		{
			Tail=NULL;
		}

		pCurr->Next=NULL;
	}
	else if(pCurr==Tail)
	{
		Tail=Tail->Prev;

		if(Tail!=NULL)
		{
			Tail->Next=NULL;
		}
		else
		{
			Head=NULL;
		}

		pCurr->Prev=NULL;
	}
	else
	{
		assert(pCurr->Prev!=NULL && pCurr->Next!=NULL);

		pCurr->Prev->Next=pCurr->Next;
		pCurr->Next->Prev=pCurr->Prev;

		pCurr->Prev=NULL;
		pCurr->Next=NULL;
	}

	return TRUE;
}

BOOL DYSimpLink::SubNodeH(DYSimpNode **ppCurr)
{
	assert(Head!=NULL && Tail!=NULL && Head->Prev==NULL && Tail->Next==NULL && ppCurr!=NULL && (*ppCurr)==NULL);

	(*ppCurr)=Head;

	Head=Head->Next;

	if(Head!=NULL)
	{
		Head->Prev=NULL;
	}
	else
	{
		Tail=NULL;
	}

	(*ppCurr)->Next=NULL;

	return TRUE;
}

BOOL DYSimpLink::SubNodeT(DYSimpNode **ppCurr)
{
	assert(Head!=NULL && Tail!=NULL && Head->Prev==NULL && Tail->Next==NULL && ppCurr!=NULL && (*ppCurr)==NULL);

	(*ppCurr)=Tail;

	Tail=Tail->Prev;

	if(Tail!=NULL)
	{
		Tail->Next=NULL;
	}
	else
	{
		Head=NULL;
	}

	(*ppCurr)->Prev=NULL;

	return TRUE;
}

BOOL DYSimpLink::Clone(const DYSimpLink &aSimpLink)
{
	assert(this!=&aSimpLink);

	if(!Create(aSimpLink.GetLength()))
	{
		return FALSE;
	}

	DYSimpNode *pCurr1=          Head;
	DYSimpNode *pCurr2=aSimpLink.Head;

	while(pCurr1!=NULL && pCurr2!=NULL)
	{
		pCurr1->Index=pCurr2->Index;

		pCurr1=pCurr1->Next;
		pCurr2=pCurr2->Next;
	}

	return TRUE;
}

BOOL DYSimpLink::Reset(VOID)
{
	Head=NULL;
	Tail=NULL;

	return TRUE;
}

BOOL DYSimpLink::Release(VOID)
{
	DYSimpNode *pCurr=Head;

	while(pCurr!=NULL)
	{
		DYSimpNode *pNext=pCurr->Next;

		DY_DELETE_PTR(pCurr);

		pCurr=pNext;
	}

	Head=NULL;
	Tail=NULL;

	return TRUE;
}
