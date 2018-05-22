//////////////////////////////////////////////////////////////////////
//
// DYPiece.h: interface for the DYPiece class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DYPIECE_H__21740176_BB29_4B7E_8B4F_8B56E7103A48__INCLUDED_)
#define AFX_DYPIECE_H__21740176_BB29_4B7E_8B4F_8B56E7103A48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DYDefine.h"
#include "DYMat3x3.h"

#define DY_PIECE_CHANNEL_DEFAULT    3
#define DY_PIECE_MIN_HEIGHT_DEFAULT 4
#define DY_PIECE_MIN_WIDTH_DEFAULT  4

class DYPiece
{
public:
	DYString File;  // full path

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

	union
	{
		struct
		{
			INT Row;
			INT Col;
		};

		struct
		{
			INT Poses[2];
		};

		struct
		{
			INT64 Pos;
		};
	};

	DYMat3x3 Trans;

public:
	DYPiece(const DYString &strFile=DY_TEXT(""),                       const INT nRow=0,const INT nCol=0,const DYMat3x3 &matTrans=DYMat3x3::Identity());
	DYPiece(const DYString &strFile,const INT nHeight,const INT nWidth,const INT nRow  ,const INT nCol  ,const DYMat3x3 &matTrans=DYMat3x3::Identity());
	virtual ~DYPiece();

public:
	DYPiece(const DYPiece &aPiece);
	DYPiece & operator = (const DYPiece &aPiece);

public:
	BOOL        IsEmpty  (VOID) const;

public:
	static BOOL GetSizes (const DYString &strFile/*full path*/,INT &nHeight,INT &nWidth);
	static BOOL GetColors(const DYPiece &aPiece/*full path*/,UCHAR **ppColors);
};

#endif // !defined(AFX_DYPIECE_H__21740176_BB29_4B7E_8B4F_8B56E7103A48__INCLUDED_)
