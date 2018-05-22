#ifndef VISIBILITY_TOOL_H_INCLUDE
#define VISIBILITY_TOOL_H_INCLUDE
#include "ToolBase.h"
#include "IVisibilityAnalysisTool.h"
#include "AnalysisBaseTool.h"

class VisibilityAnalysisTool: virtual public IVisibilityAnalysisTool, public AnalysisBaseTool
{
public:	
	explicit VisibilityAnalysisTool(const std::string &strName);
	virtual ~VisibilityAnalysisTool(void);

protected: // virtual methods from IToolBase

	virtual const   std::string &getType(void) const;	

public:     // virtual methods by itself
	
	virtual bool    operateOnCulture(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

	virtual void	setVisibilityMode(VISIBILITY_MODE mode);
	virtual void	setCenterPoint(double lon,double lat,double height);
	virtual double	getVisibilityResult();
private:
	double		    renderVisibilityResult(osg::Vec3d& centerPoint,osg::Vec3d& endPoint);
	void	        renderCircle(osg::Vec3d& centerPoint,osg::Vec3d& endPoint);
	osg::ref_ptr<osg::Vec3dArray>   m_pVertexArray;	
	osg::Vec3d						m_CenterPoint;
	osg::Vec3d						m_MovePoint;
	osg::Vec3d						m_EndPoint;
	bool							m_bCenterPoint;
	bool							m_bMovePoint;
	bool							m_bEndPoint;
	float							m_fMouseDownX;
	float							m_fMouseDownY;
	float							m_fMouseUpX;
	float							m_fMouseUpY;
	double							m_dCenterLon;
	double							m_dCenterLat;
	double							m_dCenterHeight;
	VISIBILITY_MODE					m_visibilityMode;
	double							m_dResult;
	



};
#endif

