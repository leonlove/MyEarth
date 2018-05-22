#ifndef TOOL_BASE_H_5BD70915_BC69_4C9B_9EE3_6E9BCA93506F_INCLUDE
#define TOOL_BASE_H_5BD70915_BC69_4C9B_9EE3_6E9BCA93506F_INCLUDE

#if defined(_MSC_VER) && defined(DISABLE_MSVC_WARNINGS)
#pragma warning( disable : 4250 )
#endif

#include <string>
#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIActionAdapter>
#include <osg/Group>
#include <osgUtil/Radial.h>

#include <EventAdapter/IEventAdapter.h>
#include <EventAdapter/IEventObject.h>

#include "IToolBase.h"
#include "SceneGraphOperator.h"

class ToolBase : virtual public IToolBase
{
public:
    explicit ToolBase(const std::string &strName);
    virtual ~ToolBase(void) = 0;

protected: // virtual methods from IToolBase
    virtual const   std::string &getName(void) const;
    virtual const   std::string &getType(void) const = 0;
    virtual bool    getMap2Screen(void) const;
    virtual void    setMap2Screen(bool bMap);
    virtual bool    getArtifactTopmost(void) const;
    virtual void    setArtifactTopmost(bool bTopmost);
    virtual void    clearArtifact(void);

public:     // virtual methods by itself
    virtual bool    handleEvent(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);
    virtual void    onActive(void);
    virtual void    onDeactive(void);

    virtual bool    operateOnScreen(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa) = 0;
    virtual bool    operateOnCulture(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa) = 0;

protected:
    osg::Camera    *createScreenCamera(void);

public:
    const osg::Group   *getToolNode(void) const;
    osg::Group         *getToolNode(void);
    void                setOperationTargetNode(osg::Node *pNode);

    void                setAutoClear(bool bAutoClear)   {   m_bAutoClear = bAutoClear;  }

public:
    void                setEventAdapter(ea::IEventAdapter *pEventAdapter);
	void				setSceneGraphOperator(SceneGraphOperator *pSceneGraphOperator)	{	m_pSceneGraphOperator = pSceneGraphOperator;	}

protected:
    std::string                     m_strName;
    osg::ref_ptr<osg::Group>        m_pToolNode;
    osg::Group*                     m_pCurrentArtifactNode;
    osg::ref_ptr<osg::Group>        m_pArtifactOnTerrainNode;
    osg::ref_ptr<osg::Camera>       m_pArtifactOnScreenNode;

    OpenSP::sp<ea::IEventAdapter>   m_pEventAdapter;

    osg::ref_ptr<osg::Camera>       m_pShadowOfViewCamera;
    osg::ref_ptr<osg::Node>         m_pOperationTargetNode;
	SceneGraphOperator				*m_pSceneGraphOperator;
	
    bool            m_bLButtonDown;

    bool            m_bMap2Screen;
    bool            m_bArtifactTopmost;
    int             m_nViewIndex;
    bool            m_bAutoClear;
};


#endif
