#ifndef IVISIBILITY_TOOL_H_INCLUDE
#define IVISIBILITY_TOOL_H_INCLUDE
#include <string>
#include <vector>
#include "IToolBase.h"
#include "IAnalysisBaseTool.h"

enum VISIBILITY_MODE
{
	VM_POINT2POINT,
	VM_POINT2LINE,
	VM_POINT2AREA
};

class IVisibilityAnalysisTool: virtual public IAnalysisBaseTool
{

public:     // virtual methods by itself
	virtual void	setVisibilityMode(VISIBILITY_MODE mode) = 0;
	virtual void	setCenterPoint(double lon,double lat,double height) = 0;
	virtual double	getVisibilityResult() = 0;
};
#endif

