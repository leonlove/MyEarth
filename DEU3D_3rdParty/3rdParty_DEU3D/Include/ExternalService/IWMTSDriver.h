#ifndef _I_WMTS_DRIVER_H_9464331F_0E31_4A8B_97D8_8B7A5F0B0FE7_
#define _I_WMTS_DRIVER_H_9464331F_0E31_4A8B_97D8_8B7A5F0B0FE7_

#include "IDriver.h"
#include "Export.h"
#include "ITileSet.h"

namespace deues
{
    class IWMTSDriver : virtual public IDriver
    {
    public:
        virtual ITileSet* getTileSet(void) = 0;
    };

    DEUES_EXPORT IWMTSDriver* createWMTSDriver(void);
    DEUES_EXPORT void freeMemory(void *p);
}

#endif