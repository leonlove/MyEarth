//////////////////////////////////////////////////////////////////////
//
// DYVert.h: interface for the DYVert class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DYVERT_H__FA7FC74B_4F8F_49A6_A4C0_AB8792B7327F__INCLUDED_)
#define AFX_DYVERT_H__FA7FC74B_4F8F_49A6_A4C0_AB8792B7327F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DYDefine.h"
#include "DYVector2F.h"
#include "DYVector3F.h"

class DYVert
{
public:
	DYVector3F Point;
	DYVector3F Normal;

	DYVector2F Texel;

	bool	   valid ;
	bool	   delrepeat ;

public:
	DYVert(const DYVector3F &vPoint=DYVector3F(),const DYVector3F &vNormal=DYVector3F(),const DYVector2F &vTexel=DYVector2F());
	virtual ~DYVert();

public:
	DYVert(const DYVert &aVert);
	DYVert & operator = (const DYVert &aVert);

	DYVert operator + () const;
	DYVert operator - () const;

	DYVert & operator += (const DYVert   &aVert);
	DYVert & operator -= (const DYVert   &aVert);
	DYVert & operator *= (const FLOAT     fRatio);
	DYVert & operator /= (const FLOAT     fRatio);

	DYVert operator + (const DYVert   &aVert   ) const;
	DYVert operator - (const DYVert   &aVert   ) const;
	DYVert operator * (const FLOAT     fRatio  ) const;
	DYVert operator / (const FLOAT     fRatio  ) const;

	friend DYVert operator * (const FLOAT fRatio,const DYVert &aVert);

	BOOL operator == (const DYVert &aVert) const;
	BOOL operator != (const DYVert &aVert) const;

	bool operator <  (const DYVert &aVert) const;
	bool operator >  (const DYVert &aVert) const;
	bool operator <= (const DYVert &aVert) const;
	bool operator >= (const DYVert &aVert) const;

public:
	BOOL        IsEmpty (VOID) const;
};

#endif // !defined(AFX_DYVERT_H__FA7FC74B_4F8F_49A6_A4C0_AB8792B7327F__INCLUDED_)
