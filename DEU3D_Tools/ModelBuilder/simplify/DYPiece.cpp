//////////////////////////////////////////////////////////////////////
//
// DYPiece.cpp: implementation of the DYPiece class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DYPiece.h"
//#include "DeuTextureConvert.h"

void cpu_rotate(UCHAR ** inbuf, UCHAR ** outbuf, int w, int h, float angle)
{
	if(w % 2 == 0) w --;
	if(h % 2 == 0) h --;
	int i, j;
	int xc = w/2;
	int yc = h/2;

	float cosTheta = cos(3.1415926535897932384626433832795 * angle/180);
	float sinTheta = sin(3.1415926535897932384626433832795 * angle/180);

	for(i = 0; i < h; i++)
	{
		for(j=0; j< w; j++)
		{
			int xpos = (int)( (i-xc)*cosTheta  + (j-yc)*sinTheta + xc );    
			int ypos = (int)( -(i-xc)*sinTheta + (j-yc)*cosTheta + yc ); 

			if(xpos>=0 && ypos>=0 && xpos<h && ypos < w )
				outbuf[xpos][ypos] = inbuf[i][j];
		}
	}
}


DYPiece::DYPiece(const DYString &strFile,const INT nRow,const INT nCol,const DYMat3x3 &matTrans)
{
	INT nHeight=0;
	INT nWidth =0;

	DYPiece::GetSizes(strFile,nHeight,nWidth);

	File  =strFile;

	Height=nHeight;
	Width =nWidth;

	Row   =nRow;
	Col   =nCol;

	Trans =matTrans;
}

DYPiece::DYPiece(const DYString &strFile,const INT nHeight,const INT nWidth,const INT nRow,const INT nCol,const DYMat3x3 &matTrans):File(strFile),Height(nHeight),Width(nWidth),Row(nRow),Col(nCol),Trans(matTrans)
{
}

DYPiece::~DYPiece()
{
}

DYPiece::DYPiece(const DYPiece &aPiece):File(aPiece.File),Size(aPiece.Size),Pos(aPiece.Pos),Trans(aPiece.Trans)
{
}

DYPiece & DYPiece::operator = (const DYPiece &aPiece)
{
	if(this==&aPiece)
	{
		return *this;
	}

	File =aPiece.File;

	Size =aPiece.Size;

	Pos  =aPiece.Pos;

	Trans=aPiece.Trans;

	return *this;
}

BOOL DYPiece::IsEmpty(VOID) const
{
	return (File.empty() || Height<DY_PIECE_MIN_HEIGHT_DEFAULT || Width<DY_PIECE_MIN_WIDTH_DEFAULT || Row<0 || Col<0 || Trans==0.0);
}

BOOL DYPiece::GetSizes(const DYString &strFile,INT &nHeight,INT &nWidth)
{
	nHeight=0;
	nWidth =0;

	if(strFile.empty())
	{
		return FALSE;
	}

	FILE *fp=fopen(strFile.c_str(),DY_TEXT("rb"));

	if(fp==NULL)
	{
		return FALSE;
	}

	BITMAPFILEHEADER aBitmapFileHeader;

	if(fread((void *)(&aBitmapFileHeader),(size_t)(sizeof(BITMAPFILEHEADER)),(size_t)(1),(FILE *)(fp))<(size_t)(1))
	{
		DY_CLOSE_FILE(fp);

		return FALSE;
	}

	if(aBitmapFileHeader.bfType!=0x4D42)
	{
		DY_CLOSE_FILE(fp);

		return FALSE;
	}

	BITMAPINFOHEADER aBitmapInfoHeader;

	if(fread((void *)(&aBitmapInfoHeader),(size_t)(sizeof(BITMAPINFOHEADER)),(size_t)(1),(FILE *)(fp))<(size_t)(1))
	{
		DY_CLOSE_FILE(fp);

		return FALSE;
	}

	INT nChannel=0;

	nHeight =DY_ABS(aBitmapInfoHeader.biHeight);
	nWidth  =aBitmapInfoHeader.biWidth;
	nChannel=aBitmapInfoHeader.biBitCount>>3;

	if(nHeight<DY_PIECE_MIN_HEIGHT_DEFAULT || nHeight!=::Normalize(nHeight) || nWidth<DY_PIECE_MIN_WIDTH_DEFAULT || nWidth!=::Normalize(nWidth) || nChannel!=DY_PIECE_CHANNEL_DEFAULT)
	{
		nHeight=0;
		nWidth =0;

		DY_CLOSE_FILE(fp);

		return FALSE;
	}

	DY_CLOSE_FILE(fp);

	return TRUE;
}

BOOL DYPiece::GetColors(const DYPiece &aPiece,UCHAR **ppColors)
{
	const DYString &strFile = aPiece.File;
	assert(ppColors!=NULL);

	DY_DELETE_ARR(*ppColors);

	if(strFile.empty())
	{
		return FALSE;
	}

 	FILE *fp=fopen(strFile.c_str(),DY_TEXT("rb"));
 
 	if(fp==NULL)
 	{
 		return FALSE;
 	}
 
 	BITMAPFILEHEADER aBitmapFileHeader;
 
 	if(fread((void *)(&aBitmapFileHeader),(size_t)(sizeof(BITMAPFILEHEADER)),(size_t)(1),(FILE *)(fp))<(size_t)(1))
 	{
 		DY_CLOSE_FILE(fp);
 
 		return FALSE;
 	}
 
 	if(aBitmapFileHeader.bfType!=0x4D42)
 	{
 		DY_CLOSE_FILE(fp);
 
 		return FALSE;
 	}
 
 	BITMAPINFOHEADER aBitmapInfoHeader;
 
 	if(fread((void *)(&aBitmapInfoHeader),(size_t)(sizeof(BITMAPINFOHEADER)),(size_t)(1),(FILE *)(fp))<(size_t)(1))
 	{
 		DY_CLOSE_FILE(fp);
 
 		return FALSE;
 	}
 
 	const INT nHeight =DY_ABS(aBitmapInfoHeader.biHeight);
 	const INT nWidth  =aBitmapInfoHeader.biWidth;
 	const INT nChannel=aBitmapInfoHeader.biBitCount>>3;
 
 	if(nHeight<DY_PIECE_MIN_HEIGHT_DEFAULT || nHeight!=::Normalize(nHeight) || nWidth<DY_PIECE_MIN_WIDTH_DEFAULT || nWidth!=::Normalize(nWidth) || nChannel!=DY_PIECE_CHANNEL_DEFAULT)
 	{
 		DY_CLOSE_FILE(fp);
 
 		return FALSE;
 	}
 
 	const INT nCount=nHeight*(((nWidth*nChannel+3)>>2)<<2);
 
 	assert(nCount>0 && sizeof(UCHAR)==1);
 
 	UCHAR *pColors=new UCHAR[nCount];
 
 	assert(pColors!=NULL);
 
 	if(fread((void *)(pColors),(size_t)(sizeof(UCHAR)),(size_t)(nCount),(FILE *)(fp))<(size_t)(nCount))
 	{
 		DY_CLOSE_FILE(fp);
 
 		DY_DELETE_ARR(pColors);
 
 		return FALSE;
 	}

	UCHAR *pColorsOut=new UCHAR[aPiece.Width*aPiece.Height*3];

	for (int i=0; i<aPiece.Height; i++)
	{
		memcpy_s(pColorsOut+(aPiece.Height-i-1)*aPiece.Width*3, aPiece.Width*3, pColors+i*aPiece.Width*3, aPiece.Width*3);
	}
 
 	DY_CLOSE_FILE(fp);

 	DY_SWAP((*ppColors),/*pColorsOut*/pColors,UCHAR *);
 
 	DY_DELETE_ARR(pColors);
	DY_DELETE_ARR(pColorsOut);

	return TRUE;
}
