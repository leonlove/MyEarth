#include "DEUDefine.h"
#include "export.h"


class DATAKEEPER_API DEUUtils
{
public:
    DEUUtils(void);
    ~DEUUtils(void);
public:
    static void         getRcdFromBson(const std::string strRcd,std::map<unsigned int,RcdInfo>& rcdMap);
    static void         getHostFromBson(const std::string strHost,std::map<std::string,HostInfo>& hostMap);

    static std::string  getHostBsonString(const std::map<std::string,HostInfo>& hostMap);
    static std::string  getRcdBsonString(const std::map<unsigned int,RcdInfo>& rcdMap);
};

