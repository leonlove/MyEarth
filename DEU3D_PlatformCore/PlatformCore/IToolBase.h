#ifndef I_TOOL_BASE_H_5F691929_951F_4E09_B07A_638D7B001779_INCLUDE
#define I_TOOL_BASE_H_5F691929_951F_4E09_B07A_638D7B001779_INCLUDE

#include <OpenSP/Ref.h>
#include <IDProvider/ID.h>
#include <string>
#include <vector>

const std::string POINT_TOOL        = "PointTool";
const std::string LINE_TOOL         = "LineTool";
const std::string POLYLINE_TOOL     = "PolylineTool";
const std::string RECT_TOOL         = "RectTool";
const std::string ELLIPSE_TOOL      = "EllipseTool";
const std::string POLYGON_TOOL      = "PolygonTool";
const std::string MEASURE_TOOL      = "MeasureTool";
const std::string AREA_TOOL         = "AreaTool";
const std::string EDITING_TOOL      = "EditingTool";
const std::string VISIBILITYANALYSIS_TOOL    = "VisibilityAnalysisTool";

class IToolBase : virtual public OpenSP::Ref
{
public:
    virtual const   std::string &getName(void) const    = 0;
    virtual const   std::string &getType(void) const    = 0;
    virtual bool    getMap2Screen(void) const           = 0;
    virtual void    setMap2Screen(bool bMap)            = 0;
    virtual bool    getArtifactTopmost(void) const      = 0;
    virtual void    setArtifactTopmost(bool bTopmost)   = 0;
    virtual void    clearArtifact(void)                 = 0;
};


#endif
