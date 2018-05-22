#ifndef ANALSYSIBASE_TOOL_H_INCLUDE
#define ANALSYSIBASE_TOOL_H_INCLUDE
#include "ISceneViewer.h"
#include "ToolBase.h"
#include "IAnalysisBaseTool.h"

class AnalysisBaseTool : virtual public IAnalysisBaseTool,  public ToolBase
{
public:	
	explicit AnalysisBaseTool(const std::string &strName);	
	virtual ~AnalysisBaseTool(void);

public: 
	//ToolBase必须要实现的虚方法
	virtual bool    operateOnScreen(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);
	//virtual bool    operateOnCulture(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa) = 0;
 //  
	//分析自己的方法
	virtual void	setSceneViewer(ISceneViewer* pSceneViewer);

protected:
	OpenSP::sp<ISceneViewer>  m_pSceneViewer;
};
#endif

