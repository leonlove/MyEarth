#ifndef SYMBOL_CATEGORY_H_02640D78_9D17_4665_A4B7_1EBEC0B8F6D8_INCLUDE
#define SYMBOL_CATEGORY_H_02640D78_9D17_4665_A4B7_1EBEC0B8F6D8_INCLUDE

#include "SymbolManager.h"
#include "ISymbolCategory.h"
//#include "ParameterSys/ISymbol.h"
#include <OpenSP/sp.h>
#include "DEUDBProxy/IDEUDBProxy.h"
#include <iostream>

#include <algorithm>

namespace logical
{
    class SymbolCategory : public ISymbolCategory
    {
    public:
        SymbolCategory(ID& id);
        ~SymbolCategory(void);

        typedef std::map<std::string, OpenSP::sp<deudbProxy::IDEUDBProxy>>::iterator ItMapDB;
        std::map<std::string, OpenSP::sp<deudbProxy::IDEUDBProxy>> m_mapDBs;

    public:
        const std::string&      getName(void) const;
        void                    setName(const std::string &strName);
        unsigned                getSymbolsCount(void) const;
        const ID&               getSymbol(unsigned nIndex) const;
        bool                    addSymbol(const ID &id);
        bool                    removeSymbol(const ID &id);
        bool                    addSymbolDB(const std::string &strDB);
        unsigned                getSubCategoriesCount(void) const;
        ISymbolCategory*        getSubCategory(unsigned nIndex);
        const ISymbolCategory*  getSubCategory(unsigned nIndex) const;
        bool                    addSubCategory(ISymbolCategory *pCategory);
        ISymbolCategory*        createSubCategory(const std::string &strName);
        bool                    removeSubCategory(const ISymbolCategory *pCategory);
        bool                    addParent(ISymbolCategory *pCategory);
        unsigned                getParentCount(void) const;
        ISymbolCategory*        getParent(unsigned nIndex);
        const ISymbolCategory*  getParent(unsigned nIndex) const;
        const ID&               getID(void) const;
        const std::string&      getDescription(void) const;
        void                    setDescription(const std::string& strDesc);

        void LocalBsonToObj(bson::bsonDocument& Doc);
        void LocalObjToBson(bson::bsonDocument& Doc);

        bool initNetwork(const ID& id);

        void ParseToBson(bson::bsonDocument& Doc);
        bool saveLocalCategory(const std::string &strCategoryFile);

    public:
        static SymbolManager* s_pSymbolManager;

    private:
        struct Finder
        {
            ID m_idFound;
            ISymbolCategory* m_pISymbolCategory;
            Finder(const ID &id) : m_idFound(id){}
            Finder(ISymbolCategory* pISymbolCategory) : m_pISymbolCategory(pISymbolCategory){}
            bool operator()(const ID& id) const
            {
                return (m_idFound == id);
            }

            bool operator()(ISymbolCategory* pISymbolCategory) const
            {
                return (m_pISymbolCategory == pISymbolCategory);
            }
        };

        std::string m_strName, m_strDesc;
        ID m_id;
        std::vector<ID>                                        m_vecSymbols;
        std::vector<OpenSP::sp<logical::ISymbolCategory>>      m_vecSubSymbolCategory;
        std::vector<OpenSP::sp<logical::ISymbolCategory>>      m_vecParentSymbolCategory;
        std::map<std::string, OpenSP::sp<deunw::IDEUNetwork>>  m_mapLocalDBs;
    };
}
#endif