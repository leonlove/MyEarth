// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� DDSCONVERTER_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// DDSCONVERTER_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef DDSCONVERTER_EXPORTS
#define DDSCONVERTER_API __declspec(dllexport)
#else
#define DDSCONVERTER_API __declspec(dllimport)
#endif


enum PixelOrder
{
    PO_RGBA,
    PO_BGRA,
    PO_RGB,
    PO_BGR
};

bool __stdcall RGB_2_DXT1(const unsigned char *pImageData, unsigned nImgWidth, unsigned nImgHeight, PixelOrder eOrder, unsigned char **pDDSData, unsigned *nLength);
bool __stdcall RGBA_2_DXT1(const unsigned char *pImageData, unsigned nImgWidth, unsigned nImgHeight, PixelOrder eOrder, unsigned char **pDDSData, unsigned *nLength);
bool __stdcall RGBA_2_DXT3(const unsigned char *pImageData, unsigned nImgWidth, unsigned nImgHeight, PixelOrder eOrder, unsigned char **pDDSData, unsigned *nLength);
bool __stdcall RGBA_2_DXT5(const unsigned char *pImageData, unsigned nImgWidth, unsigned nImgHeight, PixelOrder eOrder, unsigned char **pDDSData, unsigned *nLength);
bool __stdcall DXT_2_RGBA(const unsigned char *pDDSData, unsigned nLength, unsigned char **pRGBAPixels, unsigned int *nWidth, unsigned int *nHeight, unsigned int *nBandCount, PixelOrder *eOrder);
void __stdcall freeDXTMemory(void *pBuffer);