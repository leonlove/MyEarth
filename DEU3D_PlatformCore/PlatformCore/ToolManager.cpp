// ToolManager.cpp : 定义 DLL 应用程序的导出函数。
//

#include <algorithm>
#include "ToolManager.h"
#include "PointTool.h"
#include "LineTool.h"
#include "PolylineTool.h"
#include "RectTool.h"
#include "EllipseTool.h"
#include "PolygonTool.h"
#include "MeasureTool.h"
#include "AreaTool.h"
#include "EditingTool.h"
#include <EventAdapter/IEventObject.h>
#include "VisibilityAnalysisTool.h"


ToolManager::ToolManager(ea::IEventAdapter *pEventAdapter)
    : m_pEventAdapter(pEventAdapter),
	m_pSceneViewer(NULL)
{
    m_bAutoClear     = false;
    m_pToolNodesRoot = new osg::Group;
}


ToolManager::~ToolManager(void)
{
}


IToolBase *ToolManager::createTool(const std::string &strToolType, const std::string &strName)
{
    if(strName.empty())
    {
        return NULL;
    }
    if(findToolByName(strName))
    {
        return NULL;
    }

    ToolBase *pNewTool = NULL;
    if(strToolType.compare(POINT_TOOL) == 0)
    {
        pNewTool = new PointTool(strName);
    }
    else if(strToolType.compare(LINE_TOOL) == 0)
    {
        pNewTool = new LineTool(strName);
    }
    else if(strToolType.compare(POLYLINE_TOOL) == 0)
    {
        pNewTool = new PolylineTool(strName);
    }
    else if(strToolType.compare(RECT_TOOL) == 0)
    {
        pNewTool = new RectTool(strName);
    }
    else if(strToolType.compare(ELLIPSE_TOOL) == 0)
    {
        pNewTool = new EllipseTool(strName);
    }
    else if(strToolType.compare(POLYGON_TOOL) == 0)
    {
        pNewTool = new PolygonTool(strName);
    }
    else if(strToolType.compare(MEASURE_TOOL) == 0)
    {
        pNewTool = new MeasureTool(strName);
    }
    else if(strToolType.compare(AREA_TOOL) == 0)
    {
        pNewTool = new AreaTool(strName);
    }
    else if(strToolType.compare(EDITING_TOOL) == 0)
    {
        pNewTool = new EditingTool(strName);
    }
	else if(strToolType.compare(VISIBILITYANALYSIS_TOOL) == 0)
	{
		VisibilityAnalysisTool* tool = new VisibilityAnalysisTool(strName);		
		tool->setSceneViewer(m_pSceneViewer);
		pNewTool = /*dynamic_cast<ToolBase*>(tool)*/tool;
	}

    if(NULL == pNewTool)
    {
        return NULL;
    }

    pNewTool->setSceneGraphOperator(m_pSceneGraphOperator);
    if(m_pEventAdapter.valid())
    {
        pNewTool->setEventAdapter(m_pEventAdapter.get());
    }
    if(m_pOperationTargetNode.valid())
    {
        pNewTool->setOperationTargetNode(m_pOperationTargetNode.get());
    }

    osg::Group *pToolNode = pNewTool->getToolNode();
    m_pToolNodesRoot->addChild(pToolNode);

    pNewTool->setAutoClear(m_bAutoClear);

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxTools);
        m_mapAllTools[strName] = pNewTool;
    }

    return pNewTool;
}


IToolBase *ToolManager::getTool(const std::string &strName)
{
    IToolBase *pTool = findToolByName(strName);
    return pTool;
}


const IToolBase *ToolManager::getTool(const std::string &strName) const
{
    IToolBase *pTool = findToolByName(strName);
    return pTool;
}


ToolBase *ToolManager::findToolByName(const std::string &strName) const
{
    ToolBase   *pFindTool = NULL;

    std::map<std::string, OpenSP::sp<ToolBase> >::const_iterator itorFind = m_mapAllTools.begin();
    for( ; itorFind != m_mapAllTools.end(); ++itorFind)
    {
        const std::string &strFindName = itorFind->first;
        if(strFindName == strName)
        {
            pFindTool = itorFind->second.get();
            break;
        }
    }
    return pFindTool;
}


void ToolManager::setActiveTool(const std::string &strName)
{
    OpenSP::sp<ToolBase>    pActiveTool;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxTools);
        pActiveTool = findToolByName(strName);
        if(!pActiveTool.valid())
        {
            return;
        }
    }

    deactiveTool();

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxTools);
        m_strActiveTool = strName;
    }
    pActiveTool->onActive();
}


const std::string &ToolManager::getActiveTool(void) const
{
    return m_strActiveTool;
}


bool ToolManager::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    OpenSP::sp<ToolBase>   pActiveTool;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxTools);
        pActiveTool = findToolByName(m_strActiveTool);
        if(!pActiveTool.valid())
        {
            return false;
        }
    }

    return pActiveTool->handleEvent(ea, aa);
}


void ToolManager::deactiveTool()
{
    OpenSP::sp<ToolBase>    pActiveTool;

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxTools);

        pActiveTool = findToolByName(m_strActiveTool);
        if(!pActiveTool.valid())
        {
            return;
        }
        m_strActiveTool.clear();
    }
    pActiveTool->onDeactive();
}


void ToolManager::setOperationTargetNode(osg::Node *pNode)
{
    m_pOperationTargetNode = pNode;
    Tools::iterator itorTool = m_mapAllTools.begin();
    for( ; itorTool != m_mapAllTools.end(); ++itorTool)
    {
        OpenSP::sp<ToolBase> &pTool = itorTool->second;
        pTool->setOperationTargetNode(pNode);
    }
}

void ToolManager::setAutoClear(bool bAutoClear)
{
    if(m_bAutoClear == bAutoClear)
    {
        return;
    }

    m_bAutoClear = bAutoClear;

    OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxTools);
    Tools::iterator itorTool = m_mapAllTools.begin();
    for( ; itorTool != m_mapAllTools.end(); ++itorTool)
    {
        OpenSP::sp<ToolBase> &pTool = itorTool->second;
        pTool->setAutoClear(m_bAutoClear);
    }
}

bool ToolManager::getAutoClear(void) const
{
    return m_bAutoClear;
}

void ToolManager::setSceneView(ISceneViewer* pViewer)
{
	m_pSceneViewer = pViewer;
}

