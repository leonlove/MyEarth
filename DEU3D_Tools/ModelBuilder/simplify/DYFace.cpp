//////////////////////////////////////////////////////////////////////
//
// DYFace.cpp: implementation of the DYFace class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DYFace.h"

// #ifdef _DEBUG
// #undef THIS_FILE
// static char THIS_FILE[]=__FILE__;
// #define new DEBUG_NEW
// #endif

DYFace::DYFace(const DYVert &aVert1,const DYVert &aVert2,const DYVert &aVert3):Vert1(aVert1),Vert2(aVert2),Vert3(aVert3)
{
}

DYFace::~DYFace()
{
}

DYFace::DYFace(const DYVector3F &vPoint1,const DYVector3F &vPoint2,const DYVector3F &vPoint3,const DYVector3F &vNormal):Vert1(vPoint1,vNormal),Vert2(vPoint2,vNormal),Vert3(vPoint3,vNormal)
{
}

DYFace::DYFace(const DYVector3F &vPoint1,const DYVector3F &vPoint2,const DYVector3F &vPoint3,const DYVector3F &vNormal1,const DYVector3F &vNormal2,const DYVector3F &vNormal3):Vert1(vPoint1,vNormal1),Vert2(vPoint2,vNormal2),Vert3(vPoint3,vNormal3)
{
}

DYFace::DYFace(const DYFace &aFace):Vert1(aFace.Vert1),Vert2(aFace.Vert2),Vert3(aFace.Vert3)
{
}

DYFace & DYFace::operator = (const DYFace &aFace)
{
	if(this==&aFace)
	{
		return *this;
	}

	Vert1=aFace.Vert1;
	Vert2=aFace.Vert2;
	Vert3=aFace.Vert3;

	return *this;
}

DYFace DYFace::operator + () const
{
	return *this;
}

DYFace DYFace::operator - () const
{
	return DYFace(-Vert3,-Vert2,-Vert1);
}

BOOL DYFace::operator == (const DYFace &aFace) const
{
	return (Vert1==aFace.Vert1 && Vert2==aFace.Vert2 && Vert3==aFace.Vert3);
}

BOOL DYFace::operator != (const DYFace &aFace) const
{
	return (Vert1!=aFace.Vert1 || Vert2!=aFace.Vert2 || Vert3!=aFace.Vert3);
}

bool DYFace::operator < (const DYFace &aFace) const
{
	return (Vert1<aFace.Vert1 || (Vert1==aFace.Vert1 && Vert2<aFace.Vert2) || (Vert1==aFace.Vert1 && Vert2==aFace.Vert2 && Vert3<aFace.Vert3));
}

bool DYFace::operator > (const DYFace &aFace) const
{
	return (Vert1>aFace.Vert1 || (Vert1==aFace.Vert1 && Vert2>aFace.Vert2) || (Vert1==aFace.Vert1 && Vert2==aFace.Vert2 && Vert3>aFace.Vert3));
}

bool DYFace::operator <= (const DYFace &aFace) const
{
	return (Vert1<aFace.Vert1 || (Vert1==aFace.Vert1 && Vert2<aFace.Vert2) || (Vert1==aFace.Vert1 && Vert2==aFace.Vert2 && Vert3<=aFace.Vert3));
}

bool DYFace::operator >= (const DYFace &aFace) const
{
	return (Vert1>aFace.Vert1 || (Vert1==aFace.Vert1 && Vert2>aFace.Vert2) || (Vert1==aFace.Vert1 && Vert2==aFace.Vert2 && Vert3>=aFace.Vert3));
}

BOOL DYFace::IsEmpty(VOID) const
{
	if(Vert1.IsEmpty() || Vert2.IsEmpty() || Vert3.IsEmpty())
	{
		return TRUE;
	}

	if(Vert1.Point==Vert2.Point || Vert2.Point==Vert3.Point || Vert3.Point==Vert1.Point)
	{
		return TRUE;
	}

	if(CalcArea2()==0.0f)
	{
		return TRUE;
	}

	return FALSE;
}

BOOL DYFace::CalcNormals(VOID)
{
	const DYVector3F &vNormal=CalcNormal();

	Vert1.Normal=vNormal;
	Vert2.Normal=vNormal;
	Vert3.Normal=vNormal;

	return TRUE;
}

DYVector3F DYFace::CalcNormal(VOID) const
{
	const DYVector3F &vRay12=Vert2.Point-Vert1.Point;
	const DYVector3F &vRay13=Vert3.Point-Vert1.Point;

	const DYVector3F &vNormal=vRay12.Cross(vRay13).Normalize();

	return vNormal;
}

FLOAT DYFace::CalcArea(VOID) const
{
	const DYVector3F &vRay12=Vert2.Point-Vert1.Point;
	const DYVector3F &vRay13=Vert3.Point-Vert1.Point;

	const DYVector3F &vCross=vRay12.Cross(vRay13);

	return (0.5f*vCross.Length());
}

FLOAT DYFace::CalcArea2(VOID) const
{
	const DYVector3F &vRay12=Vert2.Point-Vert1.Point;
	const DYVector3F &vRay13=Vert3.Point-Vert1.Point;

	const DYVector3F &vCross=vRay12.Cross(vRay13);

	return (vCross.Length());
}
