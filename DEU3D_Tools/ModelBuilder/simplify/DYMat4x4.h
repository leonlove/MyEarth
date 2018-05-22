//////////////////////////////////////////////////////////////////////
//
// DYMat4x4.h: interface for the DYMat4x4 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DYMAT4X4_H__A332A18B_C116_44D1_8054_390F38934964__INCLUDED_)
#define AFX_DYMAT4X4_H__A332A18B_C116_44D1_8054_390F38934964__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DYDefine.h"

class DYMat4x4
{
public:
	union
	{
		struct
		{
			FLOAT M00, M01, M02, M03;
			FLOAT M10, M11, M12, M13;
			FLOAT M20, M21, M22, M23;
			FLOAT M30, M31, M32, M33;
		};

		struct
		{
			FLOAT M[4][4];
		};

		struct
		{
			FLOAT A[16];
		};
	};

public:
	DYMat4x4(const FLOAT fM00=0.0f,const FLOAT fM01=0.0f,const FLOAT fM02=0.0f,const FLOAT fM03=0.0f,const FLOAT fM10=0.0f,const FLOAT fM11=0.0f,const FLOAT fM12=0.0f,const FLOAT fM13=0.0f,const FLOAT fM20=0.0f,const FLOAT fM21=0.0f,const FLOAT fM22=0.0f,const FLOAT fM23=0.0f,const FLOAT fM30=0.0f,const FLOAT fM31=0.0f,const FLOAT fM32=0.0f,const FLOAT fM33=0.0f);
	virtual ~DYMat4x4();

public:
	DYMat4x4(const DYMat4x4   &matTrans);
	DYMat4x4 & operator = (const DYMat4x4   &matTrans);
	DYMat4x4 & operator = (const FLOAT       fData);

	DYMat4x4 operator + () const;
	DYMat4x4 operator - () const;

	DYMat4x4 & operator += (const DYMat4x4 &matTrans);
	DYMat4x4 & operator -= (const DYMat4x4 &matTrans);
	DYMat4x4 & operator *= (const DYMat4x4 &matTrans);
	DYMat4x4 & operator *= (const FLOAT     fRatio);
	DYMat4x4 & operator /= (const FLOAT     fRatio);

	DYMat4x4 operator + (const DYMat4x4 &matTrans) const;
	DYMat4x4 operator - (const DYMat4x4 &matTrans) const;
	DYMat4x4 operator * (const DYMat4x4 &matTrans) const;
	DYMat4x4 operator * (const FLOAT     fRatio  ) const;
	DYMat4x4 operator / (const FLOAT     fRatio  ) const;

	friend DYMat4x4 operator * (const FLOAT fRatio,const DYMat4x4 &matTrans);

	BOOL operator == (const DYMat4x4 &matTrans) const;
	BOOL operator != (const DYMat4x4 &matTrans) const;
	BOOL operator == (const FLOAT     fData   ) const;
	BOOL operator != (const FLOAT     fData   ) const;
};

#endif // !defined(AFX_DYMAT4X4_H__A332A18B_C116_44D1_8054_390F38934964__INCLUDED_)
