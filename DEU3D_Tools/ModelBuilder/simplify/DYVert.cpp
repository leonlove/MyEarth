//////////////////////////////////////////////////////////////////////
//
// DYVert.cpp: implementation of the DYVert class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DYVert.h"

// #ifdef _DEBUG
// #undef THIS_FILE
// static char THIS_FILE[]=__FILE__;
// #define new DEBUG_NEW
// #endif

DYVert::DYVert(const DYVector3F &vPoint,const DYVector3F &vNormal,const DYVector2F &vTexel)
: Point(vPoint)
, Normal(vNormal)
, Texel(vTexel)
, valid(true)
, delrepeat(false)
{
}

DYVert::~DYVert()
{
}

DYVert::DYVert(const DYVert &aVert)
: Point(aVert.Point)
, Normal(aVert.Normal)
, Texel(aVert.Texel)
, valid(aVert.valid)
, delrepeat(aVert.delrepeat)
{
    
}

DYVert & DYVert::operator = (const DYVert &aVert)
{
	if(this==&aVert)
	{
		return *this;
	}

	Point =aVert.Point;
	Normal=aVert.Normal;

	Texel =aVert.Texel;
	valid = aVert.valid;
	delrepeat = aVert.delrepeat;

	return *this;
}

DYVert DYVert::operator + () const
{
	return *this;
}

DYVert DYVert::operator - () const
{
	return DYVert(Point,-Normal,Texel);
}

DYVert & DYVert::operator += (const DYVert &aVert)
{
	Point +=aVert.Point;
	Normal+=aVert.Normal;

	Texel +=aVert.Texel;

	Normal.Normalize();

	return *this;
}

DYVert & DYVert::operator -= (const DYVert &aVert)
{
	Point -=aVert.Point;
	Normal-=aVert.Normal;

	Texel -=aVert.Texel;

	Normal.Normalize();

	return *this;
}

DYVert & DYVert::operator *= (const FLOAT fRatio)
{
	Point *=fRatio;
	Normal*=fRatio;

	Texel *=fRatio;

	return *this;
}

DYVert & DYVert::operator /= (const FLOAT fRatio)
{
	const FLOAT fInvRatio=1.0f/fRatio;

	Point *=fInvRatio;
	Normal*=fInvRatio;

	Texel *=fInvRatio;

	return *this;
}

DYVert DYVert::operator + (const DYVert &aVert) const
{
	return DYVert(Point+aVert.Point,(Normal+aVert.Normal).Normalize(),Texel+aVert.Texel);
}

DYVert DYVert::operator - (const DYVert &aVert) const
{
	return DYVert(Point-aVert.Point,(Normal-aVert.Normal).Normalize(),Texel-aVert.Texel);
}

DYVert DYVert::operator * (const FLOAT fRatio) const
{
	return DYVert(fRatio*Point,(fRatio*Normal).Normalize(),fRatio*Texel);
}

DYVert DYVert::operator / (const FLOAT fRatio) const
{
	const FLOAT fInvRatio=1.0f/fRatio;

	return DYVert(fInvRatio*Point,(fInvRatio*Normal).Normalize(),fInvRatio*Texel);
}

DYVert operator * (const FLOAT fRatio,const DYVert &aVert)
{
	return DYVert(fRatio*aVert.Point,(fRatio*aVert.Normal).Normalize(),fRatio*aVert.Texel);
}

BOOL DYVert::operator == (const DYVert &aVert) const
{
	if (delrepeat)
	{
		//return (Point==aVert.Point && Normal == aVert.Normal && Texel == aVert.Texel);
		return (Point==aVert.Point && Texel == aVert.Texel);		//- Èô¸ß¾«¸ËËþTexel»»³ÉNormal
	}
	
	return (Point==aVert.Point);
}

BOOL DYVert::operator != (const DYVert &aVert) const
{
	return (Point!=aVert.Point);
}

bool DYVert::operator < (const DYVert &aVert) const
{
	if (delrepeat)
	{
		//return (Point<aVert.Point || (Point==aVert.Point && Normal<aVert.Normal) || (Point==aVert.Point && Normal==aVert.Normal && Texel<aVert.Texel));
		return (Point<aVert.Point || (Point==aVert.Point && Texel<aVert.Texel));
	}
	
	return (Point<aVert.Point);
}

bool DYVert::operator > (const DYVert &aVert) const
{
	return (Point>aVert.Point)?(true):(false);
}

bool DYVert::operator <= (const DYVert &aVert) const
{
	return (Point<=aVert.Point)?(true):(false);
}

bool DYVert::operator >= (const DYVert &aVert) const
{
	return (Point>=aVert.Point)?(true):(false);
}

BOOL DYVert::IsEmpty(VOID) const
{
	return FALSE;
}
