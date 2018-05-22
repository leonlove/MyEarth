#ifndef SOURCE_3D_9A6109E1_1A64_4BE0_9339_EBB525840971_INCLUDE
#define SOURCE_3D_9A6109E1_1A64_4BE0_9339_EBB525840971_INCLUDE

#include "Typedefs.h"
#include "Model.h"
#include "Layer.h"

class Source3D
{
public:
    bool createSymbols(const std::string &ivefile, unsigned short dataset_code, bson::bsonArrayEle *pSymbolIDs, deudbProxy::IDEUDBProxy *db);

    cmm::IDEUException *createSymbolsWithOffset(const std::string &ivefile, unsigned short dataset_code, osg::Vec3d offset, const SpatialRefInfo& sri, deudbProxy::IDEUDBProxy *db, Layer &layer);

protected:
    bool    load(const std::string &ivefile);
    double  computeRadiusOfBoundChecking(osg::Node *pNode) const;
    bool    changeIntoLOD(osg::MatrixTransform *mt, Model &m, bool texture_shared);
    int     changeTextureIntoLOD(osg::Geode *pGeode, osg::Geode *pTempGeode, unsigned int nLevel, bool texture_shared);

    std::vector<osg::ref_ptr<osg::MatrixTransform>>  _mts;
    SharedTexs _shared_textures;

    unsigned short _dataset_code;

    //以下代码邱鑫添加 2014.06.18
public:
    class FindGeodeVisitor : public osg::NodeVisitor
    {
    public:
        explicit FindGeodeVisitor(void): osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), m_bHasSharedGeode(false)
        {
        }

    public:
        const unsigned int getGeodesNum() const
        {
            return m_setGeode.size();
        }
        bool hasSharedGedoe() const
        {
            return m_bHasSharedGeode;
        }
        std::set<osg::Geode *> &getGeodes()
        {
            return m_setGeode;
        }
    protected:
        virtual void apply(osg::Geode &geode)
        {
            if(geode.referenceCount() > 1)
            {
                m_bHasSharedGeode = true;
            }

            m_setGeode.insert(&geode);
        }
    protected:
        std::set<osg::Geode *>   m_setGeode;
        bool m_bHasSharedGeode;
    };
protected:
    bool loadIVE(const std::string &strIVE, unsigned short nDatasetCode, osg::Vec3d offset, const SpatialRefInfo& sri, deudbProxy::IDEUDBProxy *pDB, Layer &layer);
    osg::ref_ptr<FindGeodeVisitor> m_pFindGeodeVisitor;
    //以上代码邱鑫添加 2014.06.18

};

#endif
