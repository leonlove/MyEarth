#ifndef _I_TILESET_H_F8CA3EEE_CA3B_4E28_A34C_0DFA0C12D1BF_
#define _I_TILESET_H_F8CA3EEE_CA3B_4E28_A34C_0DFA0C12D1BF_

#include <OpenSP/Ref.h>
#include <OpenSP/sp.h>
#include <IDProvider/ID.h>
#include "DEUDefine.h"
#include <string>
#include "Export.h"

namespace deues
{
    class ITileSet : public OpenSP::Ref
    {
    public:
        virtual bool    getTopInfo(void*& pBuffer,unsigned int& nLength) const = 0;
        virtual bool    queryData(const ID& id,void*& pBuffer,unsigned int& nLength,int& nError) const= 0;
        virtual const   ID &getID(void) = 0;
        virtual unsigned __int64 getUniqueFlag(void) const = 0;
        virtual unsigned getDataSetCode() const = 0;
// 		virtual void setTileWidth(int nWidth) const = 0;
// 		virtual void setTileHeight(int nHeight) const = 0;
// 		virtual void setImageFormat(std::string strImgFormat) const = 0;
    };
}

#endif