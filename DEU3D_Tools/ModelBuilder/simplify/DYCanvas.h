//////////////////////////////////////////////////////////////////////
//
// DYCanvas.h: interface for the DYCanvas class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DYCANVAS_H__A7A51CC3_5141_4DD1_85F4_9A840AAC1B59__INCLUDED_)
#define AFX_DYCANVAS_H__A7A51CC3_5141_4DD1_85F4_9A840AAC1B59__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DYDefine.h"
#include "DYPieceList.h"

class DYCanvas
{
public:
	DYPieceList Pieces;

	union
	{
		struct
		{
			INT Height;
			INT Width;
		};

		struct
		{
			INT Sizes[2];
		};

		struct
		{
			INT64 Size;
		};
	};

public:
	DYCanvas(const DYPieceList &pPieces=DYPieceList(),const INT nHeight=0,const INT nWidth=0);
	DYCanvas(const DYString &strFile/*full path*/);
	virtual ~DYCanvas();

public:
	DYCanvas(const DYCanvas &aCanvas);
	DYCanvas & operator = (const DYCanvas &aCanvas);

public:
	BOOL IsEmpty(VOID) const;

public:
	BOOL Save   (const DYString &strFile/*full path*/) const;
};

#endif // !defined(AFX_DYCANVAS_H__A7A51CC3_5141_4DD1_85F4_9A840AAC1B59__INCLUDED_)
