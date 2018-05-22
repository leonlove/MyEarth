//////////////////////////////////////////////////////////////////////
//
// DYVector2F.cpp: implementation of the DYVector2F class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DYVector2F.h"
#include "DYVector3F.h"

// #ifdef _DEBUG
// #undef THIS_FILE
// static char THIS_FILE[]=__FILE__;
// #define new DEBUG_NEW
// #endif

DYVector2F::DYVector2F(const FLOAT x,const FLOAT y):X(x),Y(y)
{
}

DYVector2F::~DYVector2F()
{
}

DYVector2F::DYVector2F(const DYVector2F &aVector):X(aVector.X),Y(aVector.Y)
{
}

DYVector2F & DYVector2F::operator = (const DYVector2F &aVector)
{
	if(this==&aVector)
	{
		return *this;
	}

	X=aVector.X;
	Y=aVector.Y;

	return *this;
}

DYVector2F & DYVector2F::operator = (const FLOAT fData)
{
	X=fData;
	Y=fData;

	return *this;
}

DYVector2F DYVector2F::operator + () const
{
	return *this;
}

DYVector2F DYVector2F::operator - () const
{
	return DYVector2F(-X,-Y);
}

DYVector2F & DYVector2F::operator += (const DYVector2F &aVector)
{
	X+=aVector.X;
	Y+=aVector.Y;

	return *this;
}

DYVector2F & DYVector2F::operator -= (const DYVector2F &aVector)
{
	X-=aVector.X;
	Y-=aVector.Y;

	return *this;
}

DYVector2F & DYVector2F::operator += (const FLOAT fOffset)
{
	X+=fOffset;
	Y+=fOffset;

	return *this;
}

DYVector2F & DYVector2F::operator -= (const FLOAT fOffset)
{
	X-=fOffset;
	Y-=fOffset;

	return *this;
}

DYVector2F & DYVector2F::operator *= (const FLOAT fRatio)
{
	X*=fRatio;
	Y*=fRatio;

	return *this;
}

DYVector2F & DYVector2F::operator /= (const FLOAT fRatio)
{
	const FLOAT fInvRatio=1.0f/fRatio;

	X*=fInvRatio;
	Y*=fInvRatio;

	return *this;
}

DYVector2F DYVector2F::operator + (const DYVector2F &aVector) const
{
	return DYVector2F(X+aVector.X,Y+aVector.Y);
}

DYVector2F DYVector2F::operator - (const DYVector2F &aVector) const
{
	return DYVector2F(X-aVector.X,Y-aVector.Y);
}

DYVector2F DYVector2F::operator + (const FLOAT fOffset) const
{
	return DYVector2F(X+fOffset,Y+fOffset);
}

DYVector2F DYVector2F::operator - (const FLOAT fOffset) const
{
	return DYVector2F(X-fOffset,Y-fOffset);
}

DYVector2F DYVector2F::operator * (const FLOAT fRatio) const
{
	return DYVector2F(fRatio*X,fRatio*Y);
}

DYVector2F DYVector2F::operator / (const FLOAT fRatio) const
{
	const FLOAT fInvRatio=1.0f/fRatio;

	return DYVector2F(fInvRatio*X,fInvRatio*Y);
}

DYVector2F operator * (const FLOAT fRatio,const DYVector2F &aVector)
{
	return DYVector2F(fRatio*aVector.X,fRatio*aVector.Y);
}

DYVector2F::operator DYVector3F () const
{
	return DYVector3F(X,Y,0.0f);
}

BOOL DYVector2F::operator == (const DYVector2F &aVector) const
{
	/*return (X==aVector.X && Y==aVector.Y);*/
    return (fabs(X - aVector.X) < 1e-6  && fabs(Y - aVector.Y) < 1e-6);
}

BOOL DYVector2F::operator != (const DYVector2F &aVector) const
{
	/*return (X!=aVector.X || Y!=aVector.Y);*/
    return (fabs(X - aVector.X) >= 1e-6 || fabs(Y - aVector.Y) >= 1e-6);
}

BOOL DYVector2F::operator == (const FLOAT fData) const
{
	/*return (X==fData && Y==fData);*/
    return (fabs(X - fData) < 1e-6  && fabs(Y - fData) < 1e-6);
}

BOOL DYVector2F::operator != (const FLOAT fData) const
{
	/*return (X!=fData || Y!=fData);*/
    return (fabs(X - fData) >= 1e-6 || fabs(Y - fData) >= 1e-6);
}

bool DYVector2F::operator < (const DYVector2F &aVector) const
{
	return (X<aVector.X || (X==aVector.X && Y<aVector.Y));
}

bool DYVector2F::operator > (const DYVector2F &aVector) const
{
	return (X>aVector.X || (X==aVector.X && Y>aVector.Y));
}

bool DYVector2F::operator <= (const DYVector2F &aVector) const
{
	return (X<aVector.X || (X==aVector.X && Y<=aVector.Y));
}

bool DYVector2F::operator >= (const DYVector2F &aVector) const
{
	return (X>aVector.X || (X==aVector.X && Y>=aVector.Y));
}

DYVector2F & DYVector2F::Normalize(VOID)
{
	const FLOAT fLength=DY_SQRT(X*X+Y*Y);

	if(fLength>0.0f)
	{
		const FLOAT fInvLength=1.0f/fLength;

		X*=fInvLength;
		Y*=fInvLength;
	}

	return *this;
}

DYVector2F & DYVector2F::Normalize(FLOAT &fLength)
{
	fLength=DY_SQRT(X*X+Y*Y);

	if(fLength>0.0f)
	{
		const FLOAT fInvLength=1.0f/fLength;

		X*=fInvLength;
		Y*=fInvLength;
	}

	return *this;
}

FLOAT DYVector2F::Length(VOID) const
{
	return DY_SQRT(X*X+Y*Y);
}

FLOAT DYVector2F::Length2(VOID) const
{
	return (X*X+Y*Y);
}

FLOAT DYVector2F::Dist(const DYVector2F &aVector) const
{
	const FLOAT dx=X-aVector.X;
	const FLOAT dy=Y-aVector.Y;

	return DY_SQRT(dx*dx+dy*dy);
}

FLOAT DYVector2F::Dist2(const DYVector2F &aVector) const
{
	const FLOAT dx=X-aVector.X;
	const FLOAT dy=Y-aVector.Y;

	return (dx*dx+dy*dy);
}

DOUBLE DYVector2F::Dist2_2(const DYVector2F &aVector) const
{
	const DOUBLE dx=(DOUBLE)(X-aVector.X);
	const DOUBLE dy=(DOUBLE)(Y-aVector.Y);

	return (dx*dx+dy*dy);
}
