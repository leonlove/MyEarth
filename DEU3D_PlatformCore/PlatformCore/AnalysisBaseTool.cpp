#include "AnalysisBaseTool.h"

AnalysisBaseTool::AnalysisBaseTool(const std::string &strName)
	:ToolBase(strName),
	m_pSceneViewer(NULL)
{
 
}

AnalysisBaseTool::~AnalysisBaseTool(void)
{
}


bool AnalysisBaseTool::operateOnScreen(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
	return false;
}



void AnalysisBaseTool::setSceneViewer(ISceneViewer* pSceneViewer)
{
	m_pSceneViewer = pSceneViewer;
}
