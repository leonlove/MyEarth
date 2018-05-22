#ifndef TOOL_MANAGER_H_5E4BB9E9_7362_41E7_8FC7_CAA4B7FB7EFD_INCLUDE
#define TOOL_MANAGER_H_5E4BB9E9_7362_41E7_8FC7_CAA4B7FB7EFD_INCLUDE

#include <osgGA/GUIActionAdapter>
#include <osgGA/GUIEventAdapter>
#include <map>
#include <osgGA/GUIEventHandler>

#include "IToolManager.h"
#include "ToolBase.h"
#include "SceneGraphOperator.h"
#include "ISceneViewer.h"

class ToolManager : public IToolManager, public osgGA::GUIEventHandler
{
public:
    explicit ToolManager(ea::IEventAdapter *pEventAdapter = NULL);
    virtual ~ToolManager(void);

protected:  // virtual methods from the interface IToolManager
    virtual IToolBase          *createTool(const std::string &strToolType, const std::string &strName);
    virtual void                setAutoClear(bool bAutoClear);
    virtual bool                getAutoClear(void) const;
    virtual IToolBase          *getTool(const std::string &strName);
    virtual const IToolBase    *getTool(const std::string &strName) const;
    virtual void                setActiveTool(const std::string &strName);
    virtual const std::string  &getActiveTool(void) const;
    virtual void                deactiveTool(void);

public:  // virtual methods by itself
    virtual bool               handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

public:  // methods by itself
    osg::Node *getToolNode(void)                { return m_pToolNodesRoot.get();    }
    const osg::Node *getToolNode(void) const    { return m_pToolNodesRoot.get();    }
    void        setOperationTargetNode(osg::Node *pNode);
	void		setSceneGraphOperator(SceneGraphOperator *pSceneGraphOperator)	{	m_pSceneGraphOperator = pSceneGraphOperator;	}
	void		setSceneView(ISceneViewer* pViewer);

protected:
    ToolBase                  *findToolByName(const std::string &strName) const;

protected:
    typedef std::map< std::string, OpenSP::sp<ToolBase> > Tools;
    Tools                           m_mapAllTools;
    std::string                     m_strActiveTool;
    mutable OpenThreads::Mutex      m_mtxTools;

    osg::ref_ptr<osg::Group>        m_pToolNodesRoot;
    OpenSP::sp<ea::IEventAdapter>   m_pEventAdapter;
    osg::ref_ptr<osg::Node>         m_pOperationTargetNode;
	SceneGraphOperator              *m_pSceneGraphOperator;
    bool                            m_bAutoClear;
	ISceneViewer*					m_pSceneViewer;
};

#endif
