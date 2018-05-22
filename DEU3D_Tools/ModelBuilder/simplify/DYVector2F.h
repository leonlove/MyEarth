//////////////////////////////////////////////////////////////////////
//
// DYVector2F.h: interface for the DYVector2F class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DYVECTOR2F_H__EB4925B2_3954_4C7B_AEC8_30E149797F5C__INCLUDED_)
#define AFX_DYVECTOR2F_H__EB4925B2_3954_4C7B_AEC8_30E149797F5C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DYDefine.h"

class DYVector3F;

class DYVector2F
{
public:
	FLOAT X;
	FLOAT Y;

public:
	DYVector2F(const FLOAT x=0.0f,const FLOAT y=0.0f);
	virtual ~DYVector2F();

public:
	DYVector2F(const DYVector2F  &aVector);
	DYVector2F & operator = (const DYVector2F  &aVector);
	DYVector2F & operator = (const FLOAT        fData);

	DYVector2F operator + () const;
	DYVector2F operator - () const;

	DYVector2F & operator += (const DYVector2F &aVector );
	DYVector2F & operator -= (const DYVector2F &aVector );
	DYVector2F & operator += (const FLOAT       fOffset );
	DYVector2F & operator -= (const FLOAT       fOffset );
	DYVector2F & operator *= (const FLOAT       fRatio  );
	DYVector2F & operator /= (const FLOAT       fRatio  );

	DYVector2F operator + (const DYVector2F &aVector ) const;
	DYVector2F operator - (const DYVector2F &aVector ) const;
	DYVector2F operator + (const FLOAT       fOffset ) const;
	DYVector2F operator - (const FLOAT       fOffset ) const;
	DYVector2F operator * (const FLOAT       fRatio  ) const;
	DYVector2F operator / (const FLOAT       fRatio  ) const;

	friend DYVector2F operator * (const FLOAT fRatio,const DYVector2F &aVector);

	operator DYVector3F  () const;

	BOOL operator == (const DYVector2F &aVector) const;
	BOOL operator != (const DYVector2F &aVector) const;
	BOOL operator == (const FLOAT       fData  ) const;
	BOOL operator != (const FLOAT       fData  ) const;

	bool operator <  (const DYVector2F &aVector) const;
	bool operator >  (const DYVector2F &aVector) const;
	bool operator <= (const DYVector2F &aVector) const;
	bool operator >= (const DYVector2F &aVector) const;

public:
	DYVector2F & Normalize   (VOID);
	DYVector2F & Normalize   (FLOAT &fLength);

	FLOAT        Length      (VOID) const;
	FLOAT        Length2     (VOID) const;
	FLOAT        Dist        (const DYVector2F &aVector) const;
	FLOAT        Dist2       (const DYVector2F &aVector) const;
	DOUBLE       Dist2_2     (const DYVector2F &aVector) const;
};

#endif // !defined(AFX_DYVECTOR2F_H__EB4925B2_3954_4C7B_AEC8_30E149797F5C__INCLUDED_)
