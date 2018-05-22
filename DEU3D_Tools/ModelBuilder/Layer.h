#pragma once

#include "Typedefs.h"
#include <ParameterSys\IPointParameter.h>
#include "ParamPoint.h"
#include "Line.h"
#include "Face.h"
#include "Model.h"

class Layer
{
public:
    Layer();
    ~Layer(void);

    void reset();
    ID   getID();
    cmm::IDEUException *createPointFromModelSymbol(Model &m, deudbProxy::IDEUDBProxy *db);
    //IDEUException addParamPointsAndImages(ParamPointList& points, list<string>& image_files, IDEUDB *db);
    cmm::IDEUException *addParam(::Parameter *param, deudbProxy::IDEUDBProxy *db);
    cmm::IDEUException *writeToDB(deudbProxy::IDEUDBProxy* db, const std::string &filename);

    //以下代码邱鑫添加 2014.06.19
    void                addPointParam(param::IPointParameter *pPointParamter);
    //以上代码邱鑫添加 2014.06.19

private:
    ID                  _id;
    cmm::math::Sphered  _bs;
    std::vector<ID>     _model_ids;
};

