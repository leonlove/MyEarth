//////////////////////////////////////////////////////////////////////
//
// DYSimpNode.h: interface for the DYSimpNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DYSIMPNODE_H__65A89379_DC1C_462D_BCA6_E2FDB0DB4C22__INCLUDED_)
#define AFX_DYSIMPNODE_H__65A89379_DC1C_462D_BCA6_E2FDB0DB4C22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DYDefine.h"

#define DY_SIMP_NODE_INDEX_NONE     -1

#define DY_SIMP_NODE_IS_EMPTY(pNode) ((pNode)->Index<0)

class DYSimpNode
{
public:
	INT         Index;

	DYSimpNode *Prev;
	DYSimpNode *Next;

public:
	DYSimpNode(const INT nIndex=DY_SIMP_NODE_INDEX_NONE,DYSimpNode *pPrev=NULL,DYSimpNode *pNext=NULL);
	virtual ~DYSimpNode();

public:
	DYSimpNode(const DYSimpNode &aSimpNode);                // not safe
	DYSimpNode & operator = (const DYSimpNode &aSimpNode);  // not safe

public:
	BOOL IsEmpty(VOID) const;
};

#endif // !defined(AFX_DYSIMPNODE_H__65A89379_DC1C_462D_BCA6_E2FDB0DB4C22__INCLUDED_)
