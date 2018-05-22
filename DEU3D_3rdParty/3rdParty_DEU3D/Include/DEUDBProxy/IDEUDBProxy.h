#ifndef I_DEUDB_PROXY_H_4B258213_B7DA_42AA_890A_E49B9FA93C37_INCLUDE
#define I_DEUDB_PROXY_H_4B258213_B7DA_42AA_890A_E49B9FA93C37_INCLUDE

#include "Export.h"
#include <string>
#include <vector>
#include <OpenSP/Ref.h>
#include <IDProvider/ID.h>

namespace deudbProxy
{
    class IDEUDBProxy : public OpenSP::Ref
    {
    public:
#if _DEBUG
        virtual bool  openDB(const std::string &strDB, UINT_64 nReadBufferSize = 134217728u, UINT_64 nWriteBufferSize = 4194304u) = 0;
#else
        virtual bool  openDB(const std::string &strDB, UINT_64 nReadBufferSize = 134217728u, UINT_64 nWriteBufferSize = 4194304u) = 0;
#endif
        virtual bool  closeDB() = 0;
        virtual bool  isExist(const ID &id) = 0;
        virtual bool  readBlock(const ID &id, void *&pBuffer, unsigned &nLength,const unsigned& nVersion = 0)= 0;
        virtual bool  removeBlock(const ID &id) = 0;
        virtual bool  addBlock(const ID &id, const void *pBuffer, unsigned nBufLen) = 0;
        virtual bool  updateBlock(const ID &id, const void *pBuffer, unsigned nBufLen) = 0;
        virtual bool  replaceBlock(const ID &id, const void *pBuffer, unsigned nBufLen) = 0;
        virtual std::vector<ID> getAllIndices(void) = 0;
        virtual unsigned getBlockCount(void) const = 0;
        virtual void  getIndices(std::vector<ID> &vecIndices, unsigned nOffset = 0u, unsigned nCount = ~0u) const = 0;
        virtual std::vector<unsigned> getVersion(const ID &id) const = 0;
        virtual bool  setClearFlag(bool bFlag) = 0;
    };

    DEUDB_PROXY_EXPORT IDEUDBProxy *createDEUDBProxy(void);
    DEUDB_PROXY_EXPORT void    freeMemory(void *pData);
}

#endif
