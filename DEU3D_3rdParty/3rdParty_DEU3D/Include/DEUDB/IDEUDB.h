#ifndef I_DEUDB_H_D0D38C6E_BF64_4C42_840D_3E0019D9F7A6_INCLUDE
#define I_DEUDB_H_D0D38C6E_BF64_4C42_840D_3E0019D9F7A6_INCLUDE

#include "Export.h"
#include <string>
#include <vector>
#include <OpenSP/Ref.h>
#include <IDProvider/ID.h>

namespace deudb
{
    class IDEUDB : public OpenSP::Ref
    {
    public:
        virtual bool        openDB(const std::string &strDB, UINT_64 nReadBufferSize = 134217728u, UINT_64 nWriteBufferSize = 67108864u) = 0;
        virtual void        closeDB(void) = 0;
        virtual bool        isOpen(void) const = 0;
        virtual bool        isExist(const ID &id) const = 0;
        virtual bool        readBlock(const ID &id, void *& pBuffer, unsigned &nLength,const unsigned &nVersion = 0) = 0;
        virtual bool        removeBlock(const ID &id) = 0;
        virtual bool        addBlock(const ID &id, const void *pBuffer, unsigned nBufLen) = 0;
        virtual bool        updateBlock(const ID &id, const void *pBuffer, unsigned nBufLen) = 0;
        virtual bool        replaceBlock(const ID &id, const void *pBuffer, unsigned nBufLen) = 0;
        virtual unsigned    getBlockCount(void) const = 0;
        virtual void        getIndices(std::vector<ID> &vecIndices, unsigned nOffset = 0u, unsigned nCount = ~0u) const = 0;
        virtual std::vector<unsigned> getVersion(const ID &id) const = 0;
    };

    DEUDB_EXPORT IDEUDB *createDEUDB(void);
    DEUDB_EXPORT void    freeMemory(void *pData);
}

#endif
