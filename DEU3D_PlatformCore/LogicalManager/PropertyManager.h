#ifndef PROPERTY_MANAGER_H_D318B953_7CC4_42B2_A5E5_89FFC663AF42_INCLUDE
#define PROPERTY_MANAGER_H_D318B953_7CC4_42B2_A5E5_89FFC663AF42_INCLUDE

#include "IPropertyManager.h"
#include <map>
#include <OpenSP/sp.h>
#include <Common/DEUBson.h>
#include <DEUDBProxy/IDEUDBProxy.h>
#include <Network/IDEUNetwork.h>

namespace logical
{

class Property;
class PropertyManager : public IPropertyManager
{
public:
    explicit PropertyManager(void);
    virtual ~PropertyManager(void);

public:     // Methods from IPropertyManager
    virtual bool        addLocalPropertyServer(const std::string &strServer);
    virtual bool        removeLocalPropertyServer(const std::string &strServer);
    virtual IProperty  *findProperty(const ID &id, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
    virtual IProperty  *createPropertyByBsonStream(const void *pBsonStream, unsigned int nLen);
    virtual IProperty  *createPropertyByJsonStream(const std::string &strJsonCode);
    virtual bool        saveProperties2Local(const std::string &strSaveName, IDList &idList);

public:
    bool    login(const std::string &strAuthHost, const std::string &strAuthPort, const std::string &strUserName, const std::string &strUserPwd);
    bool    logout();
    bool    initialize(const std::string &strHost, const std::string &strPort, const std::string &strLocalCache);

    enum FindTarget
    {
        FT_Auto,
        FT_LocalOnly,
        FT_RemoteOnly
    };
    IProperty          *findProperty(const ID &id, FindTarget eFindTarget, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);
    void                fetchPropertyData(const ID &id, void *&pData, unsigned int &nDataLen, FindTarget eFindTarget = FT_Auto, OpenSP::sp<cmm::IDEUException> pOutExcep = NULL);

protected:
    void                closeLocalDB();
    Property           *analyseProperties(const bson::bsonElement *pBsonEle);
    Property           *analyseProperties(const bson::bsonDocument *pBsonDoc);

protected:
    OpenSP::sp<deunw::IDEUNetwork>      m_pDEUNetwork;

    OpenThreads::Mutex      m_mtxPropertyServers;
    std::map< std::string, OpenSP::sp<deudbProxy::IDEUDBProxy> >    m_mapPropertyServer;
};

}
#endif