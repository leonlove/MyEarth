#ifndef LINE_H_63D671E4_D80D_4523_A94B_E964A9D4E5E1_INCLUDE
#define LINE_H_63D671E4_D80D_4523_A94B_E964A9D4E5E1_INCLUDE

#include "Typedefs.h"
#include <ParameterSys\ILineParameter.h>
#include "Parameter.h"

class ParamLine:public Parameter
{
public:
    ParamLine(param::ILineParameter* base);
    ParamLine(unsigned short dataset_code);
    ~ParamLine(void);

    //bool            create(Shape2D *shape, const list<LineStyle>&  lod_styles);

    const ID&           getID()const;
    cmm::math::Sphered  getBoundingSphere(void);
    bool                writeToDB(const ID &parentID, deudbProxy::IDEUDBProxy *db);

    param::IParameter*  base();

private:
    OpenSP::sp<param::ILineParameter> _base;
    ID                                _id;
};


#endif
