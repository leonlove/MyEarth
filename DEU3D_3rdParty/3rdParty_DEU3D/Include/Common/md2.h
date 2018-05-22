#ifndef MD2_H_5F17B135_A1D8_4C66_AFDD_0DCB1A191550_INCLUDE
#define MD2_H_5F17B135_A1D8_4C66_AFDD_0DCB1A191550_INCLUDE

#include <string>

#include "Export.h"

namespace cmm
{
CM_EXPORT unsigned int createHashMD2(const void *pBuffer, unsigned nLength);
CM_EXPORT void createHashMD2(const void *input, size_t length, char *out);
}

#endif //__MD2_H__
