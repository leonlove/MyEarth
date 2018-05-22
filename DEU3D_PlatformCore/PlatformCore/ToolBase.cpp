#include "ToolBase.h"
#include <assert.h>
#include <osgViewer/View>

#include <osgUtil/IntersectionVisitor.h>
#include <EventAdapter/IEventObject.h>

#include "HudLayouter.h"
#include "Utility.h"
#include "AddOrRemove_Operation.h"

ToolBase::ToolBase(const std::string &strName)
{
    assert(!strName.empty());

    m_strName        = strName;
    m_pEventAdapter  = NULL;
    m_bLButtonDown   = false;

    m_pToolNode      = new osg::Group;

    m_pCurrentArtifactNode    = NULL;
    m_pArtifactOnScreenNode   = createScreenCamera();
    m_pArtifactOnTerrainNode  = new osg::Group;
	m_pCurrentArtifactNode	  = m_pArtifactOnTerrainNode.get();

    m_pShadowOfViewCamera     = new osg::Camera;

    m_nViewIndex = -1;

    m_bAutoClear = true;
	m_bMap2Screen = false;

    osg::StateSet *pStateSet  = m_pToolNode->getOrCreateStateSet();
    pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    setArtifactTopmost(false);
}


ToolBase::~ToolBase(void)
{
    m_pToolNode = NULL;
}

const std::string &ToolBase::getName(void) const
{
    return m_strName;
}


bool ToolBase::getMap2Screen(void) const
{
    return m_bMap2Screen;
}


void ToolBase::setMap2Screen(bool bMap)
{
    if(!m_bMap2Screen == !bMap) return;

    m_bMap2Screen = bMap;

	OpenSP::sp<RemoveChildrenFormParent_Operation> pRemoveOperation1 = new RemoveChildrenFormParent_Operation();
	pRemoveOperation1->setRemoveObj(m_pToolNode, 0, m_pToolNode->getNumChildren());
	m_pSceneGraphOperator->pushOperation(pRemoveOperation1);

    //m_pToolNode->removeChildren(0u, m_pToolNode->getNumChildren());
    if(m_bMap2Screen)
    {
        m_pCurrentArtifactNode = m_pArtifactOnScreenNode.get();
    }
    else
    {
        m_pCurrentArtifactNode = m_pArtifactOnTerrainNode.get();
    }

	OpenSP::sp<RemoveChildrenFormParent_Operation> pRemoveOperation2 = new RemoveChildrenFormParent_Operation();
	pRemoveOperation2->setRemoveObj(m_pCurrentArtifactNode, 0, m_pCurrentArtifactNode->getNumChildren());
	m_pSceneGraphOperator->pushOperation(pRemoveOperation2);

	OpenSP::sp<AddChildToParent_Operation> pAddOperation = new AddChildToParent_Operation();
	pAddOperation->setAddPair(m_pToolNode, m_pCurrentArtifactNode);
	m_pSceneGraphOperator->pushOperation(pAddOperation);

    //m_pCurrentArtifactNode->removeChildren(0u, m_pCurrentArtifactNode->getNumChildren());
    //m_pToolNode->addChild(m_pCurrentArtifactNode);
}


bool ToolBase::getArtifactTopmost(void) const
{
    return m_bArtifactTopmost;
}


void ToolBase::setArtifactTopmost(bool bTopmost)
{
    m_bArtifactTopmost = bTopmost;

    osg::StateSet *pStateSet = m_pToolNode->getOrCreateStateSet();
    if(bTopmost)
    {
        pStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    }
    else
    {
        pStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
    }
}


void ToolBase::clearArtifact(void)
{
    if(!m_pToolNode.valid())
    {
        return;
    }

    //m_pToolNode->removeChildren(0, m_pToolNode->getNumChildren());
    m_pCurrentArtifactNode->removeChildren(0, m_pCurrentArtifactNode->getNumChildren());
}


const osg::Group *ToolBase::getToolNode(void) const
{
    return m_pToolNode.get();
}


osg::Group *ToolBase::getToolNode(void)
{
    return m_pToolNode.get();
}


void ToolBase::setOperationTargetNode(osg::Node *pNode)
{
    m_pOperationTargetNode = pNode;
    m_pShadowOfViewCamera->removeChildren(0u, m_pShadowOfViewCamera->getNumChildren());
    m_pShadowOfViewCamera->addChild(pNode);
}


bool ToolBase::handleEvent(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    //过滤除鼠标左键和鼠标移动的事件
    const osgGA::GUIEventAdapter::EventType eEventType = ea.getEventType();
    const int nButton = ea.getButton();
    if((nButton != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
        && (nButton != osgGA::GUIEventAdapter::DOUBLECLICK)
        && (eEventType != osgGA::GUIEventAdapter::PUSH)
        && (eEventType != osgGA::GUIEventAdapter::DRAG)
        && (eEventType != osgGA::GUIEventAdapter::MOVE)
        && (eEventType != osgGA::GUIEventAdapter::RELEASE)
        && (eEventType != osgGA::GUIEventAdapter::KEYDOWN)
        && (eEventType != osgGA::GUIEventAdapter::SCROLL))
    {
        return false;
    }

    if(m_bMap2Screen)
    {
        osgViewer::View &view = dynamic_cast<osgViewer::View &>(aa);
        osg::Camera *pCamera = view.getCamera();
        osg::Viewport *pViewport = pCamera->getViewport();
        m_pArtifactOnScreenNode->setProjectionMatrixAsOrtho2D(0.0, pViewport->width(), 0.0, pViewport->height());

        return operateOnScreen(ea, aa);
    }
    else
    {
        return operateOnCulture(ea, aa);
    }

    return false;
}


void ToolBase::onActive(void)
{
	OpenSP::sp<RemoveChildrenFormParent_Operation> pRemoveOperation1 = new RemoveChildrenFormParent_Operation();
	pRemoveOperation1->setRemoveObj(m_pToolNode, 0, m_pToolNode->getNumChildren());
	m_pSceneGraphOperator->pushOperation(pRemoveOperation1);

	OpenSP::sp<RemoveChildrenFormParent_Operation> pRemoveOperation2 = new RemoveChildrenFormParent_Operation();
	pRemoveOperation2->setRemoveObj(m_pCurrentArtifactNode, 0, m_pCurrentArtifactNode->getNumChildren());
	m_pSceneGraphOperator->pushOperation(pRemoveOperation2);

	OpenSP::sp<AddChildToParent_Operation> pAddOperation = new AddChildToParent_Operation();
	pAddOperation->setAddPair(m_pToolNode, m_pCurrentArtifactNode);
	m_pSceneGraphOperator->pushOperation(pAddOperation);

    //m_pToolNode->removeChildren(0u, m_pToolNode->getNumChildren());
    //m_pCurrentArtifactNode->removeChildren(0u, m_pCurrentArtifactNode->getNumChildren());
    //m_pToolNode->addChild(m_pCurrentArtifactNode);
}


void ToolBase::onDeactive(void)
{
	m_nViewIndex = -1;

	OpenSP::sp<RemoveChildrenFormParent_Operation> pRemoveOperation1 = new RemoveChildrenFormParent_Operation();
	pRemoveOperation1->setRemoveObj(m_pToolNode, 0, m_pToolNode->getNumChildren());
	m_pSceneGraphOperator->pushOperation(pRemoveOperation1);

	OpenSP::sp<RemoveChildrenFormParent_Operation> pRemoveOperation2 = new RemoveChildrenFormParent_Operation();
	pRemoveOperation2->setRemoveObj(m_pCurrentArtifactNode, 0, m_pCurrentArtifactNode->getNumChildren());
	m_pSceneGraphOperator->pushOperation(pRemoveOperation2);

    //m_pToolNode->removeChildren(0u, m_pToolNode->getNumChildren());
    //m_pCurrentArtifactNode->removeChildren(0u, m_pCurrentArtifactNode->getNumChildren());
}


void ToolBase::setEventAdapter(ea::IEventAdapter *pEventAdapter)
{
    m_pEventAdapter = pEventAdapter;
}


osg::Camera *ToolBase::createScreenCamera(void)
{
    osg::ref_ptr<osg::Camera>   pHudCamera = new osg::Camera;
    pHudCamera->setName("ScreenCamera_Tool");

    pHudCamera->setProjectionMatrix(osg::Matrix::ortho2D(0.0, 1.0, 0.0, 1.0));
    pHudCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    pHudCamera->setViewMatrix(osg::Matrix::identity());
    pHudCamera->setClearMask(GL_DEPTH_BUFFER_BIT);
    pHudCamera->setRenderOrder(osg::Camera::POST_RENDER);
    pHudCamera->setAllowEventFocus(false);
    pHudCamera->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    return pHudCamera.release();
}

