// SymbolManager.cpp : 定义 DLL 应用程序的导出函数。
//
#include "SymbolManager.h"
#include "SymbolCategory.h"
#include <Common/DEUBson.h>

#include <IDProvider/Definer.h>

namespace logical
{
    ISymbolManager *createSymbolManager(void)
    {
        OpenSP::sp<logical::SymbolManager> pSymbolManager = new SymbolManager();
        SymbolCategory::s_pSymbolManager = pSymbolManager.get();

        return pSymbolManager.release();
    }


    SymbolManager::SymbolManager(void)
        : m_pNetWork(NULL)
        , m_pPropertyManager(NULL)
        , m_pRootCategory(NULL)
        , m_pRootLocalCategory(NULL)
    {
    }


    SymbolManager::~SymbolManager(void)
    {
    }


    bool SymbolManager::initialize(const std::string &strHost, const std::string &strPort)
    {
        if (m_pRootCategory != NULL)
        {
            m_pRootCategory = NULL;
        }

        if (m_pPropertyManager != NULL)
        {
            m_pPropertyManager = NULL;
        }

//         m_pPropertyManager = new PropertyManager;
//         m_pPropertyManager->initialize(strHost, strPort);
// 
         ID idRootCategory = ID::getSymbolCategoryRootID();
         m_pRootCategory   = new SymbolCategory(idRootCategory);
 
         ((SymbolCategory*)m_pRootCategory.get())->initNetwork(idRootCategory);

        return true;
    }


    ISymbolCategory *SymbolManager::getRootCategory(void)
    {
        return m_pRootCategory.get();
    }


    const ISymbolCategory *SymbolManager::getRootCategory(void) const
    {
        return m_pRootCategory.get();
    }


    void SymbolManager::gatherDBs(std::vector<std::string>& vecDBs) const
    {
        //初始化数组
        vecDBs.clear();

        unsigned int n = m_pRootCategory->getSubCategoriesCount();
        //for(unsigned p=0; p<n; p++)
        {
            SymbolCategory* pCategory = dynamic_cast<SymbolCategory*>(m_pRootCategory.get());
            SymbolCategory::ItMapDB _ItMapDB(pCategory->m_mapDBs.begin());
            for(;_ItMapDB != pCategory->m_mapDBs.end(); _ItMapDB++)
            {
                vecDBs.push_back(_ItMapDB->first);
            }
        }
    }


    //导入
    bool SymbolManager::importLocalCategory(const std::string &strCategoryFile)
    {
        bson::bsonDocument doc;
        char* bsonbuf = NULL;
        int n=0;
        FILE *pfile;
        fopen_s(&pfile, strCategoryFile.c_str(), "r");
        if(pfile)
        {
            fseek(pfile, 0, SEEK_END);
            unsigned int nlen = ftell(pfile);
            bsonbuf = new char[nlen+1];
            memset(bsonbuf, 0, nlen+1);
            fseek(pfile, 0, SEEK_SET);

            fread(bsonbuf, 1, nlen, pfile);
            fclose(pfile);

            doc.FromJsonString(bsonbuf);
        }

        if (m_pRootCategory == NULL)
        {
            ID idRootCategory = ID::getSymbolCategoryRootID();
            m_pRootCategory   = new SymbolCategory(idRootCategory);
        }

        if (m_pRootLocalCategory == NULL)
        {
            ID idLocalCategory = ID::genNewID();
            idLocalCategory.ObjectID.m_nDataSetCode = 6;
            idLocalCategory.ObjectID.m_nType        = SYMBOL_CATEGORY_ID;
            m_pRootLocalCategory = new SymbolCategory(idLocalCategory);
            m_pRootLocalCategory->setName("本地符号库");

            m_pRootCategory->addSubCategory(m_pRootLocalCategory);
        }

        ((SymbolCategory*)m_pRootLocalCategory.get())->LocalBsonToObj(doc);

        if(bsonbuf)
        {
            delete []bsonbuf;
        }

        return true;
    }
}
