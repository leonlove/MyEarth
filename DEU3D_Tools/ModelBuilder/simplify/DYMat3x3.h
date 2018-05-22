//////////////////////////////////////////////////////////////////////
//
// DYMat3x3.h: interface for the DYMat3x3 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DYMAT3X3_H__ACE69924_7BC9_444F_898F_DE31782C6D0E__INCLUDED_)
#define AFX_DYMAT3X3_H__ACE69924_7BC9_444F_898F_DE31782C6D0E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DYDefine.h"

class DYMat3x3
{
public:
	union
	{
		struct
		{
			DOUBLE M00, M01, M02;
			DOUBLE M10, M11, M12;
			DOUBLE M20, M21, M22;
		};

		struct
		{
			DOUBLE M[3][3];
		};

		struct
		{
			DOUBLE A[9];
		};
	};

public:
	DYMat3x3(const DOUBLE fM00=0.0,const DOUBLE fM01=0.0,const DOUBLE fM02=0.0,const DOUBLE fM10=0.0,const DOUBLE fM11=0.0,const DOUBLE fM12=0.0,const DOUBLE fM20=0.0,const DOUBLE fM21=0.0,const DOUBLE fM22=0.0);
	virtual ~DYMat3x3();

public:
	DYMat3x3(const DYMat3x3 &matTrans);
	DYMat3x3 & operator = (const DYMat3x3 &matTrans);
	DYMat3x3 & operator = (const DOUBLE    fData);

	DYMat3x3 operator + () const;
	DYMat3x3 operator - () const;

	DYMat3x3 & operator += (const DYMat3x3 &matTrans);
	DYMat3x3 & operator -= (const DYMat3x3 &matTrans);
	DYMat3x3 & operator *= (const DYMat3x3 &matTrans);
	DYMat3x3 & operator *= (const DOUBLE    fRatio);
	DYMat3x3 & operator /= (const DOUBLE    fRatio);

	DYMat3x3 operator + (const DYMat3x3 &matTrans) const;
	DYMat3x3 operator - (const DYMat3x3 &matTrans) const;
	DYMat3x3 operator * (const DYMat3x3 &matTrans) const;
	DYMat3x3 operator * (const DOUBLE    fRatio  ) const;
	DYMat3x3 operator / (const DOUBLE    fRatio  ) const;

	friend DYMat3x3 operator * (const DOUBLE fRatio,const DYMat3x3 &matTrans);

	BOOL operator == (const DYMat3x3 &matTrans) const;
	BOOL operator != (const DYMat3x3 &matTrans) const;
	BOOL operator == (const DOUBLE    fData   ) const;
	BOOL operator != (const DOUBLE    fData   ) const;

public:
	static DYMat3x3 Identity   (VOID);
};

#endif // !defined(AFX_DYMAT3X3_H__ACE69924_7BC9_444F_898F_DE31782C6D0E__INCLUDED_)
