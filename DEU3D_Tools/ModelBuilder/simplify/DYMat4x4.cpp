//////////////////////////////////////////////////////////////////////
//
// DYMat4x4.cpp: implementation of the DYMat4x4 class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DYMat4x4.h"

// #ifdef _DEBUG
// #undef THIS_FILE
// static char THIS_FILE[]=__FILE__;
// #define new DEBUG_NEW
// #endif

DYMat4x4::DYMat4x4(const FLOAT fM00,const FLOAT fM01,const FLOAT fM02,const FLOAT fM03,const FLOAT fM10,const FLOAT fM11,const FLOAT fM12,const FLOAT fM13,const FLOAT fM20,const FLOAT fM21,const FLOAT fM22,const FLOAT fM23,const FLOAT fM30,const FLOAT fM31,const FLOAT fM32,const FLOAT fM33):M00(fM00),M01(fM01),M02(fM02),M03(fM03),M10(fM10),M11(fM11),M12(fM12),M13(fM13),M20(fM20),M21(fM21),M22(fM22),M23(fM23),M30(fM30),M31(fM31),M32(fM32),M33(fM33)
{
}

DYMat4x4::~DYMat4x4()
{
}

DYMat4x4::DYMat4x4(const DYMat4x4 &matTrans):M00(matTrans.M00),M01(matTrans.M01),M02(matTrans.M02),M03(matTrans.M03),M10(matTrans.M10),M11(matTrans.M11),M12(matTrans.M12),M13(matTrans.M13),M20(matTrans.M20),M21(matTrans.M21),M22(matTrans.M22),M23(matTrans.M23),M30(matTrans.M30),M31(matTrans.M31),M32(matTrans.M32),M33(matTrans.M33)
{
}

DYMat4x4 & DYMat4x4::operator = (const DYMat4x4 &matTrans)
{
	if(this==&matTrans)
	{
		return *this;
	}

	M00=matTrans.M00; M01=matTrans.M01; M02=matTrans.M02; M03=matTrans.M03;
	M10=matTrans.M10; M11=matTrans.M11; M12=matTrans.M12; M13=matTrans.M13;
	M20=matTrans.M20; M21=matTrans.M21; M22=matTrans.M22; M23=matTrans.M23;
	M30=matTrans.M30; M31=matTrans.M31; M32=matTrans.M32; M33=matTrans.M33;

	return *this;
}

DYMat4x4 & DYMat4x4::operator = (const FLOAT fData)
{
	M00=fData; M01=fData; M02=fData; M03=fData;
	M10=fData; M11=fData; M12=fData; M13=fData;
	M20=fData; M21=fData; M22=fData; M23=fData;
	M30=fData; M31=fData; M32=fData; M33=fData;

	return *this;
}

DYMat4x4 DYMat4x4::operator + () const
{
	return *this;
}

DYMat4x4 DYMat4x4::operator - () const
{
	return DYMat4x4(-M00,-M01,-M02,-M03,
					-M10,-M11,-M12,-M13,
					-M20,-M21,-M22,-M23,
					-M30,-M31,-M32,-M33);
}

DYMat4x4 & DYMat4x4::operator += (const DYMat4x4 &matTrans)
{
	M00+=matTrans.M00; M01+=matTrans.M01; M02+=matTrans.M02; M03+=matTrans.M03;
	M10+=matTrans.M10; M11+=matTrans.M11; M12+=matTrans.M12; M13+=matTrans.M13;
	M20+=matTrans.M20; M21+=matTrans.M21; M22+=matTrans.M22; M23+=matTrans.M23;
	M30+=matTrans.M30; M31+=matTrans.M31; M32+=matTrans.M32; M33+=matTrans.M33;

	return *this;
}

DYMat4x4 & DYMat4x4::operator -= (const DYMat4x4 &matTrans)
{
	M00-=matTrans.M00; M01-=matTrans.M01; M02-=matTrans.M02; M03-=matTrans.M03;
	M10-=matTrans.M10; M11-=matTrans.M11; M12-=matTrans.M12; M13-=matTrans.M13;
	M20-=matTrans.M20; M21-=matTrans.M21; M22-=matTrans.M22; M23-=matTrans.M23;
	M30-=matTrans.M30; M31-=matTrans.M31; M32-=matTrans.M32; M33-=matTrans.M33;

	return *this;
}

DYMat4x4 & DYMat4x4::operator *= (const DYMat4x4 &matTrans)
{
	const DYMat4x4 matPrev(*this);

	M00=matPrev.M00*matTrans.M00+matPrev.M01*matTrans.M10+matPrev.M02*matTrans.M20+matPrev.M03*matTrans.M30;
	M01=matPrev.M00*matTrans.M01+matPrev.M01*matTrans.M11+matPrev.M02*matTrans.M21+matPrev.M03*matTrans.M31;
	M02=matPrev.M00*matTrans.M02+matPrev.M01*matTrans.M12+matPrev.M02*matTrans.M22+matPrev.M03*matTrans.M32;
	M03=matPrev.M00*matTrans.M03+matPrev.M01*matTrans.M13+matPrev.M02*matTrans.M23+matPrev.M03*matTrans.M33;

	M10=matPrev.M10*matTrans.M00+matPrev.M11*matTrans.M10+matPrev.M12*matTrans.M20+matPrev.M13*matTrans.M30;
	M11=matPrev.M10*matTrans.M01+matPrev.M11*matTrans.M11+matPrev.M12*matTrans.M21+matPrev.M13*matTrans.M31;
	M12=matPrev.M10*matTrans.M02+matPrev.M11*matTrans.M12+matPrev.M12*matTrans.M22+matPrev.M13*matTrans.M32;
	M13=matPrev.M10*matTrans.M03+matPrev.M11*matTrans.M13+matPrev.M12*matTrans.M23+matPrev.M13*matTrans.M33;

	M20=matPrev.M20*matTrans.M00+matPrev.M21*matTrans.M10+matPrev.M22*matTrans.M20+matPrev.M23*matTrans.M30;
	M21=matPrev.M20*matTrans.M01+matPrev.M21*matTrans.M11+matPrev.M22*matTrans.M21+matPrev.M23*matTrans.M31;
	M22=matPrev.M20*matTrans.M02+matPrev.M21*matTrans.M12+matPrev.M22*matTrans.M22+matPrev.M23*matTrans.M32;
	M23=matPrev.M20*matTrans.M03+matPrev.M21*matTrans.M13+matPrev.M22*matTrans.M23+matPrev.M23*matTrans.M33;

	M30=matPrev.M30*matTrans.M00+matPrev.M31*matTrans.M10+matPrev.M32*matTrans.M20+matPrev.M33*matTrans.M30;
	M31=matPrev.M30*matTrans.M01+matPrev.M31*matTrans.M11+matPrev.M32*matTrans.M21+matPrev.M33*matTrans.M31;
	M32=matPrev.M30*matTrans.M02+matPrev.M31*matTrans.M12+matPrev.M32*matTrans.M22+matPrev.M33*matTrans.M32;
	M33=matPrev.M30*matTrans.M03+matPrev.M31*matTrans.M13+matPrev.M32*matTrans.M23+matPrev.M33*matTrans.M33;

	return *this;
}

DYMat4x4 & DYMat4x4::operator *= (const FLOAT fRatio)
{
	M00*=fRatio; M01*=fRatio; M02*=fRatio; M03*=fRatio;
	M10*=fRatio; M11*=fRatio; M12*=fRatio; M13*=fRatio;
	M20*=fRatio; M21*=fRatio; M22*=fRatio; M23*=fRatio;
	M30*=fRatio; M31*=fRatio; M32*=fRatio; M33*=fRatio;

    return *this;
}

DYMat4x4 & DYMat4x4::operator /= (const FLOAT fRatio)
{
	const FLOAT fInvRatio=1.0f/fRatio;

	M00*=fInvRatio; M01*=fInvRatio; M02*=fInvRatio; M03*=fInvRatio;
	M10*=fInvRatio; M11*=fInvRatio; M12*=fInvRatio; M13*=fInvRatio;
	M20*=fInvRatio; M21*=fInvRatio; M22*=fInvRatio; M23*=fInvRatio;
	M30*=fInvRatio; M31*=fInvRatio; M32*=fInvRatio; M33*=fInvRatio;

    return *this;
}

DYMat4x4 DYMat4x4::operator + (const DYMat4x4 &matTrans) const
{
	return DYMat4x4(M00+matTrans.M00,M01+matTrans.M01,M02+matTrans.M02,M03+matTrans.M03,
					M10+matTrans.M10,M11+matTrans.M11,M12+matTrans.M12,M13+matTrans.M13,
					M20+matTrans.M20,M21+matTrans.M21,M22+matTrans.M22,M23+matTrans.M23,
					M30+matTrans.M30,M31+matTrans.M31,M32+matTrans.M32,M33+matTrans.M33);
}

DYMat4x4 DYMat4x4::operator - (const DYMat4x4 &matTrans) const
{
	return DYMat4x4(M00-matTrans.M00,M01-matTrans.M01,M02-matTrans.M02,M03-matTrans.M03,
					M10-matTrans.M10,M11-matTrans.M11,M12-matTrans.M12,M13-matTrans.M13,
					M20-matTrans.M20,M21-matTrans.M21,M22-matTrans.M22,M23-matTrans.M23,
					M30-matTrans.M30,M31-matTrans.M31,M32-matTrans.M32,M33-matTrans.M33);
}

DYMat4x4 DYMat4x4::operator * (const DYMat4x4 &matTrans) const
{
	DYMat4x4 matResult;

	matResult.M00=M00*matTrans.M00+M01*matTrans.M10+M02*matTrans.M20+M03*matTrans.M30;
	matResult.M01=M00*matTrans.M01+M01*matTrans.M11+M02*matTrans.M21+M03*matTrans.M31;
	matResult.M02=M00*matTrans.M02+M01*matTrans.M12+M02*matTrans.M22+M03*matTrans.M32;
	matResult.M03=M00*matTrans.M03+M01*matTrans.M13+M02*matTrans.M23+M03*matTrans.M33;

	matResult.M10=M10*matTrans.M00+M11*matTrans.M10+M12*matTrans.M20+M13*matTrans.M30;
	matResult.M11=M10*matTrans.M01+M11*matTrans.M11+M12*matTrans.M21+M13*matTrans.M31;
	matResult.M12=M10*matTrans.M02+M11*matTrans.M12+M12*matTrans.M22+M13*matTrans.M32;
	matResult.M13=M10*matTrans.M03+M11*matTrans.M13+M12*matTrans.M23+M13*matTrans.M33;

	matResult.M20=M20*matTrans.M00+M21*matTrans.M10+M22*matTrans.M20+M23*matTrans.M30;
	matResult.M21=M20*matTrans.M01+M21*matTrans.M11+M22*matTrans.M21+M23*matTrans.M31;
	matResult.M22=M20*matTrans.M02+M21*matTrans.M12+M22*matTrans.M22+M23*matTrans.M32;
	matResult.M23=M20*matTrans.M03+M21*matTrans.M13+M22*matTrans.M23+M23*matTrans.M33;

	matResult.M30=M30*matTrans.M00+M31*matTrans.M10+M32*matTrans.M20+M33*matTrans.M30;
	matResult.M31=M30*matTrans.M01+M31*matTrans.M11+M32*matTrans.M21+M33*matTrans.M31;
	matResult.M32=M30*matTrans.M02+M31*matTrans.M12+M32*matTrans.M22+M33*matTrans.M32;
	matResult.M33=M30*matTrans.M03+M31*matTrans.M13+M32*matTrans.M23+M33*matTrans.M33;

	return matResult;
}

DYMat4x4 DYMat4x4::operator * (const FLOAT fRatio) const
{
	return DYMat4x4(fRatio*M00,fRatio*M01,fRatio*M02,fRatio*M03,
					fRatio*M10,fRatio*M11,fRatio*M12,fRatio*M13,
					fRatio*M20,fRatio*M21,fRatio*M22,fRatio*M23,
					fRatio*M30,fRatio*M31,fRatio*M32,fRatio*M33);
}

DYMat4x4 DYMat4x4::operator / (const FLOAT fRatio) const
{
	const FLOAT fInvRatio=1.0f/fRatio;

	return DYMat4x4(fInvRatio*M00,fInvRatio*M01,fInvRatio*M02,fInvRatio*M03,
					fInvRatio*M10,fInvRatio*M11,fInvRatio*M12,fInvRatio*M13,
					fInvRatio*M20,fInvRatio*M21,fInvRatio*M22,fInvRatio*M23,
					fInvRatio*M30,fInvRatio*M31,fInvRatio*M32,fInvRatio*M33);
}

DYMat4x4 operator * (const FLOAT fRatio,const DYMat4x4 &matTrans)
{
	return DYMat4x4(fRatio*matTrans.M00,fRatio*matTrans.M01,fRatio*matTrans.M02,fRatio*matTrans.M03,
					fRatio*matTrans.M10,fRatio*matTrans.M11,fRatio*matTrans.M12,fRatio*matTrans.M13,
					fRatio*matTrans.M20,fRatio*matTrans.M21,fRatio*matTrans.M22,fRatio*matTrans.M23,
					fRatio*matTrans.M30,fRatio*matTrans.M31,fRatio*matTrans.M32,fRatio*matTrans.M33);
}

BOOL DYMat4x4::operator == (const DYMat4x4 &matTrans) const
{
	return (M00==matTrans.M00 && M01==matTrans.M01 && M02==matTrans.M02 && M03==matTrans.M03 &&
			M10==matTrans.M10 && M11==matTrans.M11 && M12==matTrans.M12 && M13==matTrans.M13 &&
			M20==matTrans.M20 && M21==matTrans.M21 && M22==matTrans.M22 && M23==matTrans.M23 &&
			M30==matTrans.M30 && M31==matTrans.M31 && M32==matTrans.M32 && M33==matTrans.M33);
}

BOOL DYMat4x4::operator != (const DYMat4x4 &matTrans) const
{
	return (M00!=matTrans.M00 || M01!=matTrans.M01 || M02!=matTrans.M02 || M03!=matTrans.M03 ||
			M10!=matTrans.M10 || M11!=matTrans.M11 || M12!=matTrans.M12 || M13!=matTrans.M13 ||
			M20!=matTrans.M20 || M21!=matTrans.M21 || M22!=matTrans.M22 || M23!=matTrans.M23 ||
			M30!=matTrans.M30 || M31!=matTrans.M31 || M32!=matTrans.M32 || M33!=matTrans.M33);
}

BOOL DYMat4x4::operator == (const FLOAT fData) const
{
	return (M00==fData && M01==fData && M02==fData && M03==fData &&
			M10==fData && M11==fData && M12==fData && M13==fData &&
			M20==fData && M21==fData && M22==fData && M23==fData &&
			M30==fData && M31==fData && M32==fData && M33==fData);
}

BOOL DYMat4x4::operator != (const FLOAT fData) const
{
	return (M00!=fData || M01!=fData || M02!=fData || M03!=fData ||
			M10!=fData || M11!=fData || M12!=fData || M13!=fData ||
			M20!=fData || M21!=fData || M22!=fData || M23!=fData ||
			M30!=fData || M31!=fData || M32!=fData || M33!=fData);
}
