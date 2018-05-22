#pragma once

#include "Typedefs.h"
#include <ParameterSys\IFaceParameter.h>
#include "Parameter.h"

class ParamFace:public Parameter
{
public:
    ParamFace(param::IFaceParameter *base);
    ParamFace(unsigned short dataset_code);
    ~ParamFace(void);
       
    //bool            create(Shape2D *shape, const list<FaceStyle> &styles);

    const ID&               getID(void)const;
    cmm::math::Sphered     getBoundingSphere(void);
    bool                    writeToDB(const ID &parentID, deudbProxy::IDEUDBProxy *db);
    param::IParameter*      base();
private:
    OpenSP::sp<param::IFaceParameter> _base;
    ID                 _id;
};

