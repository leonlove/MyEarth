//////////////////////////////////////////////////////////////////////
//
// DYVector3F.cpp: implementation of the DYVector3F class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DYVector3F.h"
#include "DYVector2F.h"
#include "DYMat4x4.h"

// #ifdef _DEBUG
// #undef THIS_FILE
// static char THIS_FILE[]=__FILE__;
// #define new DEBUG_NEW
// #endif

DYVector3F::DYVector3F(const FLOAT x,const FLOAT y,const FLOAT z):X(x),Y(y),Z(z)
{
}

DYVector3F::~DYVector3F()
{
}

DYVector3F::DYVector3F(const DYVector3F &aVector):X(aVector.X),Y(aVector.Y),Z(aVector.Z)
{
}

DYVector3F & DYVector3F::operator = (const DYVector3F &aVector)
{
	if(this==&aVector)
	{
		return *this;
	}

	X=aVector.X;
	Y=aVector.Y;
	Z=aVector.Z;

	return *this;
}

DYVector3F & DYVector3F::operator = (const FLOAT fData)
{
	X=fData;
	Y=fData;
	Z=fData;

	return *this;
}

DYVector3F DYVector3F::operator + () const
{
	return *this;
}

DYVector3F DYVector3F::operator - () const
{
	return DYVector3F(-X,-Y,-Z);
}

DYVector3F & DYVector3F::operator += (const DYVector3F &aVector)
{
	X+=aVector.X;
	Y+=aVector.Y;
	Z+=aVector.Z;

	return *this;
}

DYVector3F & DYVector3F::operator -= (const DYVector3F &aVector)
{
	X-=aVector.X;
	Y-=aVector.Y;
	Z-=aVector.Z;

	return *this;
}

DYVector3F & DYVector3F::operator *= (const DYMat4x4 &matTrans)
{
	const DYVector3F aVector(*this);

	const FLOAT fInvW=1.0f/(aVector.X*matTrans.M03+aVector.Y*matTrans.M13+aVector.Z*matTrans.M23+matTrans.M33);

	X=(aVector.X*matTrans.M00+aVector.Y*matTrans.M10+aVector.Z*matTrans.M20+matTrans.M30)*fInvW;
	Y=(aVector.X*matTrans.M01+aVector.Y*matTrans.M11+aVector.Z*matTrans.M21+matTrans.M31)*fInvW;
	Z=(aVector.X*matTrans.M02+aVector.Y*matTrans.M12+aVector.Z*matTrans.M22+matTrans.M32)*fInvW;

	return *this;
}

DYVector3F & DYVector3F::operator += (const FLOAT fOffset)
{
	X+=fOffset;
	Y+=fOffset;
	Z+=fOffset;

	return *this;
}

DYVector3F & DYVector3F::operator -= (const FLOAT fOffset)
{
	X-=fOffset;
	Y-=fOffset;
	Z-=fOffset;

	return *this;
}

DYVector3F & DYVector3F::operator *= (const FLOAT fRatio)
{
	X*=fRatio;
	Y*=fRatio;
	Z*=fRatio;

	return *this;
}

DYVector3F & DYVector3F::operator /= (const FLOAT fRatio)
{
	const FLOAT fInvRatio=1.0f/fRatio;

	X*=fInvRatio;
	Y*=fInvRatio;
	Z*=fInvRatio;

	return *this;
}

DYVector3F DYVector3F::operator + (const DYVector3F &aVector) const
{
	return DYVector3F(X+aVector.X,Y+aVector.Y,Z+aVector.Z);
}

DYVector3F DYVector3F::operator - (const DYVector3F &aVector) const
{
	return DYVector3F(X-aVector.X,Y-aVector.Y,Z-aVector.Z);
}

DYVector3F DYVector3F::operator * (const DYMat4x4 &matTrans) const
{
	const FLOAT fInvW=1.0f/(X*matTrans.M03+Y*matTrans.M13+Z*matTrans.M23+matTrans.M33);

	return DYVector3F((X*matTrans.M00+Y*matTrans.M10+Z*matTrans.M20+matTrans.M30)*fInvW,
					  (X*matTrans.M01+Y*matTrans.M11+Z*matTrans.M21+matTrans.M31)*fInvW,
					  (X*matTrans.M02+Y*matTrans.M12+Z*matTrans.M22+matTrans.M32)*fInvW);
}

DYVector3F DYVector3F::operator + (const FLOAT fOffset) const
{
	return DYVector3F(X+fOffset,Y+fOffset,Z+fOffset);
}

DYVector3F DYVector3F::operator - (const FLOAT fOffset) const
{
	return DYVector3F(X-fOffset,Y-fOffset,Z-fOffset);
}

DYVector3F DYVector3F::operator * (const FLOAT fRatio) const
{
	return DYVector3F(fRatio*X,fRatio*Y,fRatio*Z);
}

DYVector3F DYVector3F::operator / (const FLOAT fRatio) const
{
	const FLOAT fInvRatio=1.0f/fRatio;

	return DYVector3F(fInvRatio*X,fInvRatio*Y,fInvRatio*Z);
}

DYVector3F operator * (const FLOAT fRatio,const DYVector3F &aVector)
{
	return DYVector3F(fRatio*aVector.X,fRatio*aVector.Y,fRatio*aVector.Z);
}

DYVector3F::operator DYVector2F () const
{
	return DYVector2F(X,Y);
}

BOOL DYVector3F::operator == (const DYVector3F &aVector) const
{
	/*return (X==aVector.X && Y==aVector.Y && Z==aVector.Z);*/
    return (fabs(X - aVector.X) < 1e-6 && fabs(Y - aVector.Y) < 1e-6 && fabs(Z - aVector.Z) < 1e-6);
}

BOOL DYVector3F::operator != (const DYVector3F &aVector) const
{
	/*return (X!=aVector.X || Y!=aVector.Y || Z!=aVector.Z);*/
    return (fabs(X - aVector.X) >= 1e-6 || fabs(Y - aVector.Y) >= 1e-6 || fabs(Z - aVector.Z) >= 1e-6);
}

BOOL DYVector3F::operator == (const FLOAT fData) const
{
	/*return (X==fData && Y==fData && Z==fData);*/
    return (fabs(X - fData) < 1e-6 && fabs(Y - fData) < 1e-6 && fabs(Z - fData) < 1e-6);
}

BOOL DYVector3F::operator != (const FLOAT fData) const
{
	/*return (X!=fData || Y!=fData || Z!=fData);*/
    return (fabs(X - fData) >= 1e-6 || fabs(Y - fData) >= 1e-6 || fabs(Z - fData) >= 1e-6);
}

bool DYVector3F::operator < (const DYVector3F &aVector) const
{
	return (X<aVector.X || (X==aVector.X && Y<aVector.Y) || (X==aVector.X && Y==aVector.Y && Z<aVector.Z));
}

bool DYVector3F::operator > (const DYVector3F &aVector) const
{
	return (X>aVector.X || (X==aVector.X && Y>aVector.Y) || (X==aVector.X && Y==aVector.Y && Z>aVector.Z));
}

bool DYVector3F::operator <= (const DYVector3F &aVector) const
{
	return (X<aVector.X || (X==aVector.X && Y<aVector.Y) || (X==aVector.X && Y==aVector.Y && Z<=aVector.Z));
}

bool DYVector3F::operator >= (const DYVector3F &aVector) const
{
	return (X>aVector.X || (X==aVector.X && Y>aVector.Y) || (X==aVector.X && Y==aVector.Y && Z>=aVector.Z));
}

DYVector3F & DYVector3F::Normalize(VOID)
{
	const FLOAT fLength=DY_SQRT(X*X+Y*Y+Z*Z);

	if(fLength>0.0f)
	{
		const FLOAT fInvLength=1.0f/fLength;

		X*=fInvLength;
		Y*=fInvLength;
		Z*=fInvLength;
	}

	return *this;
}

DYVector3F & DYVector3F::Normalize(FLOAT &fLength)
{
	fLength=DY_SQRT(X*X+Y*Y+Z*Z);

	if(fLength>0.0f)
	{
		const FLOAT fInvLength=1.0f/fLength;

		X*=fInvLength;
		Y*=fInvLength;
		Z*=fInvLength;
	}

	return *this;
}

FLOAT DYVector3F::Length(VOID) const
{
	return DY_SQRT(X*X+Y*Y+Z*Z);
}

FLOAT DYVector3F::Length2(VOID) const
{
	return (X*X+Y*Y+Z*Z);
}

FLOAT DYVector3F::Dist(const DYVector3F &aVector) const
{
	const FLOAT dx=X-aVector.X;
	const FLOAT dy=Y-aVector.Y;
	const FLOAT dz=Z-aVector.Z;

	return DY_SQRT(dx*dx+dy*dy+dz*dz);
}

FLOAT DYVector3F::Dist2(const DYVector3F &aVector) const
{
	const FLOAT dx=X-aVector.X;
	const FLOAT dy=Y-aVector.Y;
	const FLOAT dz=Z-aVector.Z;

	return (dx*dx+dy*dy+dz*dz);
}

DOUBLE DYVector3F::Dist2_2(const DYVector3F &aVector) const
{
	const DOUBLE dx=(DOUBLE)(X-aVector.X);
	const DOUBLE dy=(DOUBLE)(Y-aVector.Y);
	const DOUBLE dz=(DOUBLE)(Z-aVector.Z);

	return (dx*dx+dy*dy+dz*dz);
}

DYVector3F DYVector3F::Cross(const DYVector3F &aVector) const
{
	return DYVector3F(Y*aVector.Z-Z*aVector.Y,Z*aVector.X-X*aVector.Z,X*aVector.Y-Y*aVector.X);
}
