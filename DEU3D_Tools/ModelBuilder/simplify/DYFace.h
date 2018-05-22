//////////////////////////////////////////////////////////////////////
//
// DYFace.h: interface for the DYFace class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DYFACE_H__CD3EB268_C48D_44CA_9957_03E8CA4F989A__INCLUDED_)
#define AFX_DYFACE_H__CD3EB268_C48D_44CA_9957_03E8CA4F989A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DYDefine.h"
#include "DYVert.h"

class DYFace
{
public:
	union
	{
		struct
		{
			DYVert Verts[3];
		};

		struct
		{
			DYVert Vert1;
			DYVert Vert2;
			DYVert Vert3;
		};
	};

public:
	DYFace(const DYVert &aVert1=DYVert(),const DYVert &aVert2=DYVert(),const DYVert &aVert3=DYVert());
	virtual ~DYFace();

public:
	DYFace(const DYVector3F &vPoint1,const DYVector3F &vPoint2,const DYVector3F &vPoint3,const DYVector3F &vNormal=DYVector3F());
	DYFace(const DYVector3F &vPoint1,const DYVector3F &vPoint2,const DYVector3F &vPoint3,const DYVector3F &vNormal1,const DYVector3F &vNormal2,const DYVector3F &vNormal3);
	DYFace(const DYFace &aFace);
	DYFace & operator = (const DYFace &aFace);

	DYFace operator + () const;
	DYFace operator - () const;

	BOOL operator == (const DYFace &aFace) const;
	BOOL operator != (const DYFace &aFace) const;

	bool operator <  (const DYFace &aFace) const;
	bool operator >  (const DYFace &aFace) const;
	bool operator <= (const DYFace &aFace) const;
	bool operator >= (const DYFace &aFace) const;

public:
	BOOL       IsEmpty    (VOID) const;

public:
	BOOL       CalcNormals(VOID);
	DYVector3F CalcNormal (VOID) const;
	FLOAT      CalcArea   (VOID) const;
	FLOAT      CalcArea2  (VOID) const;
};

#endif // !defined(AFX_DYFACE_H__CD3EB268_C48D_44CA_9957_03E8CA4F989A__INCLUDED_)
