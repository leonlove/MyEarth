//////////////////////////////////////////////////////////////////////
//
// DYSimpNode.cpp: implementation of the DYSimpNode class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DYSimpNode.h"

DYSimpNode::DYSimpNode(const INT nIndex,DYSimpNode *pPrev,DYSimpNode *pNext):Index(nIndex),Prev(pPrev),Next(pNext)
{
}

DYSimpNode::~DYSimpNode()
{
}

DYSimpNode::DYSimpNode(const DYSimpNode &aSimpNode):Index(aSimpNode.Index),Prev(aSimpNode.Prev),Next(aSimpNode.Next)
{
}

DYSimpNode & DYSimpNode::operator = (const DYSimpNode &aSimpNode)
{
	if(this==&aSimpNode)
	{
		return *this;
	}

	Index=aSimpNode.Index;

	Prev =aSimpNode.Prev;
	Next =aSimpNode.Next;

	return *this;
}

BOOL DYSimpNode::IsEmpty(VOID) const
{
	return (Index<0);
}
