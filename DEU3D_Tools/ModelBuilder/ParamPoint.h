#pragma once
#include "Typedefs.h"
#include <ParameterSys\PointParameter.h>
#include "Parameter.h"
#include "Model.h"

class ParamPoint : public Parameter
{
public:
    ParamPoint(param::IPointParameter *base);
    ParamPoint(unsigned short dataset_code);
    ParamPoint(const cmm::math::Point3d &pt, unsigned short dataset_code);

    bool            fromModelSymbol(Model &m);

    const ID&               getID() const;
    cmm::math::Sphered     getBoundingSphere(void);
    bool                    writeToDB(const ID &parentID, deudbProxy::IDEUDBProxy *db);
    param::IParameter*      base();

private:
    OpenSP::sp<param::IPointParameter> _base;
    ID                  _id;
    bool                _contains_model;
};

