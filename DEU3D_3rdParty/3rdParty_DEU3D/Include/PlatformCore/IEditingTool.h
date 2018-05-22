#ifndef I_EDITING_TOOL_H_BE0B7DD6_EFEA_4B9E_819F_2C60D9B11767_INCLUDE
#define I_EDITING_TOOL_H_BE0B7DD6_EFEA_4B9E_819F_2C60D9B11767_INCLUDE

#include <OpenSP/Ref.h>
#include <common/deuMath.h>
#include "Export.h"
#include "IToolBase.h"

const std::string TRANSLATION_EDIT = "Translation";
const std::string SCALE_EDIT = "Scale";

class IEditingTool : virtual public IToolBase
{
public:
    virtual bool setEditingTarget(const ID &strID) = 0;
    virtual const ID &getEditingTarget(void) const = 0;
    virtual void setUnit(double dblUnit)    = 0;
    virtual double getUnit(void)            = 0;
    //virtual void setEditingType(const std::string &strEditingType) = 0;
    //virtual const std::string &getEditingType(void) const = 0;
    //virtual bool setTranslation(const cmm::math::Point3d &ptTrans) = 0;
    //virtual bool setScale(const cmm::math::Point3d &ptScale) = 0;
};

#endif