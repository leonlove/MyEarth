// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 DDSCONVERTER_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// DDSCONVERTER_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
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