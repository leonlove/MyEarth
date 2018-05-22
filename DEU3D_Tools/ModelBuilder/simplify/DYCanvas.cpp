//////////////////////////////////////////////////////////////////////
//
// DYCanvas.cpp: implementation of the DYCanvas class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DYCanvas.h"
#include "DYPiece.h"
//#include "DeuTextureConvert.h"

DYCanvas::DYCanvas(const DYPieceList &pPieces,const INT nHeight,const INT nWidth):Pieces(pPieces),Height(nHeight),Width(nWidth)
{
}

DYCanvas::DYCanvas(const DYString &strFile):Pieces(),Size(0)
{
	const DYPiece aPiece(strFile,0,0,DYMat3x3::Identity());

	if(!aPiece.IsEmpty() && Pieces.Append(aPiece))
	{
		Height=::Normalize(aPiece.Height);
		Width =::Normalize(aPiece.Width );

		assert(Height>=aPiece.Height);
		assert(Width >=aPiece.Width );
	}
}

DYCanvas::~DYCanvas()
{
}

DYCanvas::DYCanvas(const DYCanvas &aCanvas):Pieces(aCanvas.Pieces),Size(aCanvas.Size)
{
}

DYCanvas & DYCanvas::operator = (const DYCanvas &aCanvas)
{
	if(this==&aCanvas)
	{
		return *this;
	}

	Pieces=aCanvas.Pieces;

	Size  =aCanvas.Size;

	return *this;
}

BOOL DYCanvas::IsEmpty(VOID) const
{
	return (Pieces.IsEmpty() || Height<DY_PIECE_MIN_HEIGHT_DEFAULT || Width<DY_PIECE_MIN_WIDTH_DEFAULT);
}

BOOL DYCanvas::Save(const DYString &strFile) const
{
	if(IsEmpty() || strFile.empty())
	{
		return FALSE;
	}

	assert(Width>=4 && Width==::Normalize(Width) && DY_PIECE_CHANNEL_DEFAULT==3);

	const INT nCount=Height*Width*DY_PIECE_CHANNEL_DEFAULT;

	assert(nCount>0 && sizeof(UCHAR)==1);

	UCHAR *pColors=new UCHAR[nCount];

	assert(pColors!=NULL);

	memset(pColors,0,nCount*sizeof(UCHAR));

	const INT nPieceCount=Pieces.GetCount();

	INT nPieceIndex=0;

	for(nPieceIndex=0;nPieceIndex<nPieceCount;nPieceIndex++)
	{
		const DYPiece &aPiece=Pieces[nPieceIndex];

		assert(!aPiece.IsEmpty());

		UCHAR *pColor1s=pColors+aPiece.Row*Width*DY_PIECE_CHANNEL_DEFAULT+aPiece.Col*DY_PIECE_CHANNEL_DEFAULT;

		assert(pColor1s!=NULL);

		UCHAR *pColor2s=NULL;

		if(!DYPiece::GetColors(aPiece/*.File*/,&pColor2s))
		{
			DY_DELETE_ARR(pColor2s);

			return FALSE;
		}

		assert(pColor2s!=NULL);

		UCHAR *pColor3s=pColor2s;

		assert(pColor3s!=NULL);

		const INT nPitch1=       Width*DY_PIECE_CHANNEL_DEFAULT;
		const INT nPitch2=aPiece.Width*DY_PIECE_CHANNEL_DEFAULT;
		const INT nByte2s=aPiece.Width*DY_PIECE_CHANNEL_DEFAULT*sizeof(UCHAR);

		INT nRow=0;

		for(nRow=0;nRow<aPiece.Height;nRow++)
		{
			memcpy(pColor1s,pColor3s,nByte2s);

			pColor1s+=nPitch1;
			pColor3s+=nPitch2;
		}

		DY_DELETE_ARR(pColor2s);
	}

	BITMAPFILEHEADER aBitmapFileHeader;
	BITMAPINFOHEADER aBitmapInfoHeader;

	memset(&aBitmapFileHeader,0,sizeof(BITMAPFILEHEADER));
	memset(&aBitmapInfoHeader,0,sizeof(BITMAPINFOHEADER));

	PALETTEENTRY *pPalettes=NULL;

	aBitmapFileHeader.bfType         =0x4D42;
	aBitmapFileHeader.bfSize         =54+nCount;
	aBitmapFileHeader.bfReserved1    =0;
	aBitmapFileHeader.bfReserved2    =0;
	aBitmapFileHeader.bfOffBits      =54;

	aBitmapInfoHeader.biSize         =40;
	aBitmapInfoHeader.biWidth        = Width;
	aBitmapInfoHeader.biHeight       = Height;
	aBitmapInfoHeader.biPlanes       =1;
	aBitmapInfoHeader.biBitCount     =8*DY_PIECE_CHANNEL_DEFAULT;
	aBitmapInfoHeader.biCompression  =BI_RGB;
	aBitmapInfoHeader.biSizeImage    =nCount;
	aBitmapInfoHeader.biXPelsPerMeter=3780;
	aBitmapInfoHeader.biYPelsPerMeter=3780;
	aBitmapInfoHeader.biClrUsed      =0;
	aBitmapInfoHeader.biClrImportant =0;

	FILE *fp=fopen(strFile.c_str(),DY_TEXT("wb"));

	if(fp==NULL)
	{
		DY_DELETE_ARR(pPalettes);
		DY_DELETE_ARR(pColors);

		return FALSE;
	}

	if(fwrite((void *)(&aBitmapFileHeader),(size_t)(sizeof(BITMAPFILEHEADER)),(size_t)(1),(FILE *)(fp))<(size_t)(1))
	{
		DY_CLOSE_FILE(fp);

		DY_DELETE_ARR(pPalettes);
		DY_DELETE_ARR(pColors);

		return FALSE;
	}

	if(fwrite((void *)(&aBitmapInfoHeader),(size_t)(sizeof(BITMAPINFOHEADER)),(size_t)(1),(FILE *)(fp))<(size_t)(1))
	{
		DY_CLOSE_FILE(fp);

		DY_DELETE_ARR(pPalettes);
		DY_DELETE_ARR(pColors);

		return FALSE;
	}

	if(pPalettes!=NULL && fwrite((void *)(pPalettes),(size_t)(sizeof(PALETTEENTRY)),(size_t)(256),(FILE *)(fp))<(size_t)(256))
	{
		DY_CLOSE_FILE(fp);

		DY_DELETE_ARR(pPalettes);
		DY_DELETE_ARR(pColors);

		return FALSE;
	}

	if(pColors!=NULL && fwrite((void *)(pColors),(size_t)(sizeof(UCHAR)),(size_t)(nCount),(FILE *)(fp))<(size_t)(nCount))
	{
		DY_CLOSE_FILE(fp);

		DY_DELETE_ARR(pPalettes);
		DY_DELETE_ARR(pColors);

		return FALSE;
	}

// 	DeuTextureInfo* pTextureInfo = new DeuTextureInfo;
// 	pTextureInfo->width = Width;
// 	pTextureInfo->height = Height;
// 	pTextureInfo->depth = 24;
// 	pTextureInfo->pData = pColors;
// 
// 	DeuTextureConvert pDeuTextureConvert;
// 	pDeuTextureConvert.WritePNG("C:\\tt.png", pTextureInfo);

	DY_CLOSE_FILE(fp);

	DY_DELETE_ARR(pPalettes);
	DY_DELETE_ARR(pColors);

	return TRUE;
}
