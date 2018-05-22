#ifndef MODEL_H_3C9819ED_19CD_4001_B4A5_E0FDDD52EECF_INCLUDE
#define MODEL_H_3C9819ED_19CD_4001_B4A5_E0FDDD52EECF_INCLUDE

#include "Typedefs.h"
#include <vector>
#include <osg\Geode>
#include <osg\BoundingSphere>
#include <osg\Image>
#include <DEUDBProxy/IDEUDBProxy.h>
#include <ParameterSys\PointParameter.h>

class Model
{
public:
    Model(unsigned short dataset_code, osg::MatrixTransform *mt);
    ~Model(void);

    osg::MatrixTransform*    mt();
    unsigned short      getDatasetCode(){return _dataset_code;}

    bool                writeToDB(deudbProxy::IDEUDBProxy *db);
    bool                transformOntoGlobe(const osg::Vec3d &offset, const SpatialRefInfo&  sri);
    bool                changeIntoLOD(bool texture_shared, SharedTexs &shared_textures, deudbProxy::IDEUDBProxy *db);
    void                getDetail(param::IPointParameter *p);
protected:
    OpenSP::sp<osg::MatrixTransform> _mt;
    bool                _shared_texture;
    unsigned short      _dataset_code;

    
    std::vector<OpenSP::sp<osg::Geode>>   _geodes;
    std::vector<double>      _ranges;
    std::vector<ID>          _ids;
    std::vector<ID>          _detail_ids;

    std::vector<OpenSP::sp<osg::Image>>   _imgs;
    std::vector<ID>          _id_imgs;
    
    void addGeode(osg::Geode* g, ID &id, double range);
    void saveDetail(deudbProxy::IDEUDBProxy *db);
    int  changeTextureIntoLOD(osg::Geode *pGeode, osg::Geode *pTempGeode, unsigned int nLevel, bool texture_shared, SharedTexs &shared_textures);
};

typedef std::list<Model> ModelSymbolList;

#endif
