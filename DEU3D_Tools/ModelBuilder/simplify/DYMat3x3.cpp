//////////////////////////////////////////////////////////////////////
//
// DYMat3x3.cpp: implementation of the DYMat3x3 class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DYMat3x3.h"

DYMat3x3::DYMat3x3(const DOUBLE fM00,const DOUBLE fM01,const DOUBLE fM02,const DOUBLE fM10,const DOUBLE fM11,const DOUBLE fM12,const DOUBLE fM20,const DOUBLE fM21,const DOUBLE fM22):M00(fM00),M01(fM01),M02(fM02),M10(fM10),M11(fM11),M12(fM12),M20(fM20),M21(fM21),M22(fM22)
{
}

DYMat3x3::~DYMat3x3()
{
}

DYMat3x3::DYMat3x3(const DYMat3x3 &matTrans):M00(matTrans.M00),M01(matTrans.M01),M02(matTrans.M02),M10(matTrans.M10),M11(matTrans.M11),M12(matTrans.M12),M20(matTrans.M20),M21(matTrans.M21),M22(matTrans.M22)
{
}

DYMat3x3 & DYMat3x3::operator = (const DYMat3x3 &matTrans)
{
	if(this==&matTrans)
	{
		return *this;
	}

	M00=matTrans.M00; M01=matTrans.M01; M02=matTrans.M02;
	M10=matTrans.M10; M11=matTrans.M11; M12=matTrans.M12;
	M20=matTrans.M20; M21=matTrans.M21; M22=matTrans.M22;

	return *this;
}

DYMat3x3 & DYMat3x3::operator = (const DOUBLE fData)
{
	M00=fData; M01=fData; M02=fData;
	M10=fData; M11=fData; M12=fData;
	M20=fData; M21=fData; M22=fData;

	return *this;
}

DYMat3x3 DYMat3x3::operator + () const
{
	return *this;
}

DYMat3x3 DYMat3x3::operator - () const
{
	return DYMat3x3(-M00,-M01,-M02,
					-M10,-M11,-M12,
					-M20,-M21,-M22);
}

DYMat3x3 & DYMat3x3::operator += (const DYMat3x3 &matTrans)
{
	M00+=matTrans.M00; M01+=matTrans.M01; M02+=matTrans.M02;
	M10+=matTrans.M10; M11+=matTrans.M11; M12+=matTrans.M12;
	M20+=matTrans.M20; M21+=matTrans.M21; M22+=matTrans.M22;

	return *this;
}

DYMat3x3 & DYMat3x3::operator -= (const DYMat3x3 &matTrans)
{
	M00-=matTrans.M00; M01-=matTrans.M01; M02-=matTrans.M02;
	M10-=matTrans.M10; M11-=matTrans.M11; M12-=matTrans.M12;
	M20-=matTrans.M20; M21-=matTrans.M21; M22-=matTrans.M22;

	return *this;
}

DYMat3x3 & DYMat3x3::operator *= (const DYMat3x3 &matTrans)
{
	const DYMat3x3 matPrev(*this);

	M00=matPrev.M00*matTrans.M00+matPrev.M01*matTrans.M10+matPrev.M02*matTrans.M20;
	M01=matPrev.M00*matTrans.M01+matPrev.M01*matTrans.M11+matPrev.M02*matTrans.M21;
	M02=matPrev.M00*matTrans.M02+matPrev.M01*matTrans.M12+matPrev.M02*matTrans.M22;

	M10=matPrev.M10*matTrans.M00+matPrev.M11*matTrans.M10+matPrev.M12*matTrans.M20;
	M11=matPrev.M10*matTrans.M01+matPrev.M11*matTrans.M11+matPrev.M12*matTrans.M21;
	M12=matPrev.M10*matTrans.M02+matPrev.M11*matTrans.M12+matPrev.M12*matTrans.M22;

	M20=matPrev.M20*matTrans.M00+matPrev.M21*matTrans.M10+matPrev.M22*matTrans.M20;
	M21=matPrev.M20*matTrans.M01+matPrev.M21*matTrans.M11+matPrev.M22*matTrans.M21;
	M22=matPrev.M20*matTrans.M02+matPrev.M21*matTrans.M12+matPrev.M22*matTrans.M22;

	return *this;
}

DYMat3x3 & DYMat3x3::operator *= (const DOUBLE fRatio)
{
	M00*=fRatio; M01*=fRatio; M02*=fRatio;
	M10*=fRatio; M11*=fRatio; M12*=fRatio;
	M20*=fRatio; M21*=fRatio; M22*=fRatio;

    return *this;
}

DYMat3x3 & DYMat3x3::operator /= (const DOUBLE fRatio)
{
	const DOUBLE fInvRatio=1.0/fRatio;

	M00*=fInvRatio; M01*=fInvRatio; M02*=fInvRatio;
	M10*=fInvRatio; M11*=fInvRatio; M12*=fInvRatio;
	M20*=fInvRatio; M21*=fInvRatio; M22*=fInvRatio;

    return *this;
}

DYMat3x3 DYMat3x3::operator + (const DYMat3x3 &matTrans) const
{
	return DYMat3x3(M00+matTrans.M00,M01+matTrans.M01,M02+matTrans.M02,
					M10+matTrans.M10,M11+matTrans.M11,M12+matTrans.M12,
					M20+matTrans.M20,M21+matTrans.M21,M22+matTrans.M22);
}

DYMat3x3 DYMat3x3::operator - (const DYMat3x3 &matTrans) const
{
	return DYMat3x3(M00-matTrans.M00,M01-matTrans.M01,M02-matTrans.M02,
					M10-matTrans.M10,M11-matTrans.M11,M12-matTrans.M12,
					M20-matTrans.M20,M21-matTrans.M21,M22-matTrans.M22);
}

DYMat3x3 DYMat3x3::operator * (const DYMat3x3 &matTrans) const
{
	DYMat3x3 matResult;

	matResult.M00=M00*matTrans.M00+M01*matTrans.M10+M02*matTrans.M20;
	matResult.M01=M00*matTrans.M01+M01*matTrans.M11+M02*matTrans.M21;
	matResult.M02=M00*matTrans.M02+M01*matTrans.M12+M02*matTrans.M22;

	matResult.M10=M10*matTrans.M00+M11*matTrans.M10+M12*matTrans.M20;
	matResult.M11=M10*matTrans.M01+M11*matTrans.M11+M12*matTrans.M21;
	matResult.M12=M10*matTrans.M02+M11*matTrans.M12+M12*matTrans.M22;

	matResult.M20=M20*matTrans.M00+M21*matTrans.M10+M22*matTrans.M20;
	matResult.M21=M20*matTrans.M01+M21*matTrans.M11+M22*matTrans.M21;
	matResult.M22=M20*matTrans.M02+M21*matTrans.M12+M22*matTrans.M22;

	return matResult;
}

DYMat3x3 DYMat3x3::operator * (const DOUBLE fRatio) const
{
	return DYMat3x3(fRatio*M00,fRatio*M01,fRatio*M02,
					fRatio*M10,fRatio*M11,fRatio*M12,
					fRatio*M20,fRatio*M21,fRatio*M22);
}

DYMat3x3 DYMat3x3::operator / (const DOUBLE fRatio) const
{
	const DOUBLE fInvRatio=1.0/fRatio;

	return DYMat3x3(fInvRatio*M00,fInvRatio*M01,fInvRatio*M02,
					fInvRatio*M10,fInvRatio*M11,fInvRatio*M12,
					fInvRatio*M20,fInvRatio*M21,fInvRatio*M22);
}

DYMat3x3 operator * (const DOUBLE fRatio,const DYMat3x3 &matTrans)
{
	return DYMat3x3(fRatio*matTrans.M00,fRatio*matTrans.M01,fRatio*matTrans.M02,
					fRatio*matTrans.M10,fRatio*matTrans.M11,fRatio*matTrans.M12,
					fRatio*matTrans.M20,fRatio*matTrans.M21,fRatio*matTrans.M22);
}

BOOL DYMat3x3::operator == (const DYMat3x3 &matTrans) const
{
	return (M00==matTrans.M00 && M01==matTrans.M01 && M02==matTrans.M02 &&
			M10==matTrans.M10 && M11==matTrans.M11 && M12==matTrans.M12 &&
			M20==matTrans.M20 && M21==matTrans.M21 && M22==matTrans.M22);
}

BOOL DYMat3x3::operator != (const DYMat3x3 &matTrans) const
{
	return (M00!=matTrans.M00 || M01!=matTrans.M01 || M02!=matTrans.M02 ||
			M10!=matTrans.M10 || M11!=matTrans.M11 || M12!=matTrans.M12 ||
			M20!=matTrans.M20 || M21!=matTrans.M21 || M22!=matTrans.M22);
}

BOOL DYMat3x3::operator == (const DOUBLE fData) const
{
	return (M00==fData && M01==fData && M02==fData &&
			M10==fData && M11==fData && M12==fData &&
			M20==fData && M21==fData && M22==fData);
}

BOOL DYMat3x3::operator != (const DOUBLE fData) const
{
	return (M00!=fData || M01!=fData || M02!=fData ||
			M10!=fData || M11!=fData || M12!=fData ||
			M20!=fData || M21!=fData || M22!=fData);
}

DYMat3x3 DYMat3x3::Identity(VOID)
{
	return DYMat3x3(1.0,0.0,0.0,
					0.0,1.0,0.0,
					0.0,0.0,1.0);
}
