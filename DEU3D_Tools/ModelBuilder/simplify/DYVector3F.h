//////////////////////////////////////////////////////////////////////
//
// DYVector3F.h: interface for the DYVector3F class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DYVECTOR3F_H__155AB827_4CA5_4CF8_B784_4B3C8553066D__INCLUDED_)
#define AFX_DYVECTOR3F_H__155AB827_4CA5_4CF8_B784_4B3C8553066D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DYDefine.h"

class DYVector2F;
class DYMat4x4;

class DYVector3F
{
public:
			FLOAT X;
			FLOAT Y;
			FLOAT Z;

public:
	DYVector3F(const FLOAT x=0.0f,const FLOAT y=0.0f,const FLOAT z=0.0f);
	virtual ~DYVector3F();

public:
	DYVector3F(const DYVector3F  &aVector);
	DYVector3F & operator = (const DYVector3F  &aVector);
	DYVector3F & operator = (const FLOAT        fData);

	DYVector3F operator + () const;
	DYVector3F operator - () const;

	DYVector3F & operator += (const DYVector3F &aVector );
	DYVector3F & operator -= (const DYVector3F &aVector );
	DYVector3F & operator *= (const DYMat4x4   &matTrans);
	DYVector3F & operator += (const FLOAT       fOffset );
	DYVector3F & operator -= (const FLOAT       fOffset );
	DYVector3F & operator *= (const FLOAT       fRatio  );
	DYVector3F & operator /= (const FLOAT       fRatio  );

	DYVector3F operator + (const DYVector3F &aVector ) const;
	DYVector3F operator - (const DYVector3F &aVector ) const;
	DYVector3F operator * (const DYMat4x4   &matTrans) const;
	DYVector3F operator + (const FLOAT       fOffset ) const;
	DYVector3F operator - (const FLOAT       fOffset ) const;
	DYVector3F operator * (const FLOAT       fRatio  ) const;
	DYVector3F operator / (const FLOAT       fRatio  ) const;

	friend DYVector3F operator * (const FLOAT fRatio,const DYVector3F &aVector);

	operator DYVector2F  () const;

	BOOL operator == (const DYVector3F &aVector) const;
	BOOL operator != (const DYVector3F &aVector) const;
	BOOL operator == (const FLOAT       fData  ) const;
	BOOL operator != (const FLOAT       fData  ) const;

	bool operator <  (const DYVector3F &aVector) const;
	bool operator >  (const DYVector3F &aVector) const;
	bool operator <= (const DYVector3F &aVector) const;
	bool operator >= (const DYVector3F &aVector) const;

public:
	DYVector3F & Normalize   (VOID);
	DYVector3F & Normalize   (FLOAT &fLength);

	FLOAT        Length      (VOID) const;
	FLOAT        Length2     (VOID) const;
	FLOAT        Dist        (const DYVector3F &aVector) const;
	FLOAT        Dist2       (const DYVector3F &aVector) const;
	DOUBLE       Dist2_2     (const DYVector3F &aVector) const;
	DYVector3F   Cross       (const DYVector3F &aVector) const;
};

#endif // !defined(AFX_DYVECTOR3F_H__155AB827_4CA5_4CF8_B784_4B3C8553066D__INCLUDED_)
