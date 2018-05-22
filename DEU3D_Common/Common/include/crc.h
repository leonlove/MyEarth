#ifndef CRC_H_2C732A38_DC23_44CC_ABE1_5BDBEA8DB0A3_INCLUDE
#define CRC_H_2C732A38_DC23_44CC_ABE1_5BDBEA8DB0A3_INCLUDE

#include "Export.h"
#include <string>

namespace cmm
{
    CM_EXPORT unsigned int createHashCRC32(const void *pBuffer, unsigned nLength);
}

#endif
