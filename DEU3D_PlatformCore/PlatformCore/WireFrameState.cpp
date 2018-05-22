#include "WireFrameState.h"
#include <osgUtil/CullVisitor>
#include <osg/PolygonMode>
#include <osg/CullFace>

WireFrameLayout::WireFrameLayout(float fltWidth, const osg::Vec4& color)
    : m_dblLineWidth(fltWidth),
      m_color(color)
{
    const unsigned int Override_On  = osg::StateAttribute::ON  | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED;
    const unsigned int Override_Off = osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED;

    m_pWireFrameStateSet = new osg::StateSet;
    m_pWireFrameStateSet->setMode(osg::StateAttribute::TEXTURE, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

    // draw back-facing polygon lines
    osg::ref_ptr<osg::PolygonMode> pPolyMode = new osg::PolygonMode;
    pPolyMode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
    m_pWireFrameStateSet->setAttributeAndModes(pPolyMode.get(), Override_On);

    // outline width
    m_pLineWidth = new osg::LineWidth;
    m_pLineWidth->setWidth(m_dblLineWidth);
    m_pWireFrameStateSet->setAttributeAndModes(m_pLineWidth.get(), Override_On);

    // outline color/material
    m_pMaterial = new osg::Material;
    m_pMaterial->setColorMode(osg::Material::OFF);
    const osg::Material::Face face = osg::Material::FRONT_AND_BACK;
    m_pMaterial->setAmbient(face,  m_color);
    m_pMaterial->setDiffuse(face,  m_color);
    m_pMaterial->setSpecular(face, m_color);
    m_pMaterial->setEmission(face, m_color);
    m_pWireFrameStateSet->setAttributeAndModes(m_pMaterial.get(), Override_On);

    // disable modes
    m_pWireFrameStateSet->setMode(GL_BLEND, Override_Off);
    m_pWireFrameStateSet->setTextureMode(0, GL_TEXTURE_1D, Override_Off);
    m_pWireFrameStateSet->setTextureMode(0, GL_TEXTURE_2D, Override_Off);
    m_pWireFrameStateSet->setTextureMode(0, GL_TEXTURE_3D, Override_Off);

    osg::ref_ptr<osg::CullFace> pCullFace = new osg::CullFace(osg::CullFace::FRONT_AND_BACK);
    m_pWireFrameStateSet->setAttributeAndModes(pCullFace.get(), Override_Off);
}


WireFrameLayout::~WireFrameLayout(void)
{
}


void WireFrameLayout::setLineWidth(double dblWidth)
{
    m_dblLineWidth = dblWidth;
    m_pLineWidth->setWidth(m_dblLineWidth);
}


void WireFrameLayout::setLineColor(const osg::Vec4 &color)
{
    m_color = color;

    const osg::Material::Face face = osg::Material::FRONT_AND_BACK;
    m_pMaterial->setAmbient(face,  m_color);
    m_pMaterial->setDiffuse(face,  m_color);
    m_pMaterial->setSpecular(face, m_color);
    m_pMaterial->setEmission(face, m_color);
}


void WireFrameLayout::operator()(osg::Node *pNode, osg::NodeVisitor *pNodeVisitor)
{
    const osg::NodeVisitor::VisitorType eType = pNodeVisitor->getVisitorType();
    if(eType != osg::NodeVisitor::CULL_VISITOR)
    {
        traverse(pNode, pNodeVisitor);
        return;
    }

    osgUtil::CullVisitor *pCullVisitor = dynamic_cast<osgUtil::CullVisitor *>(pNodeVisitor);

    pCullVisitor->pushStateSet(m_pWireFrameStateSet);
    traverse(pNode, pNodeVisitor);
    pCullVisitor->popStateSet();
}


///////////////////////////////////////////////////////////////////////////////////////////////
WireFrameState::WireFrameState(const std::string &strName)
    : StateBase(strName)
{
    m_pWireFrameLayout = new WireFrameLayout(1.0, osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f));
}


WireFrameState::~WireFrameState(void)
{
}


bool WireFrameState::applyState(osg::Node *pNode, bool bApply)
{
    if(pNode == NULL)
    {
        return false;
    }

    if(bApply)
    {
        pNode->addCullCallback(m_pWireFrameLayout);
    }
    else
    {
        pNode->removeCullCallback(m_pWireFrameLayout);
    }
    return true;
}


void WireFrameState::setLineColor(const cmm::FloatColor &color)
{
    m_pWireFrameLayout->setLineColor(osg::Vec4(color.m_fltR, color.m_fltG, color.m_fltB, 1.0f));
}


cmm::FloatColor WireFrameState::getLineColor(void) const
{
    const osg::Vec4 &vColor = m_pWireFrameLayout->getLineColor();
    cmm::FloatColor color;
    color.m_fltR = vColor.r();
    color.m_fltG = vColor.g();
    color.m_fltB = vColor.b();
    color.m_fltA = vColor.a();
    return color;
}


void WireFrameState::setLineWidth(double dblWidth)
{
    m_pWireFrameLayout->setLineWidth(dblWidth);
}


double WireFrameState::getLineWidth(void) const
{
    return m_pWireFrameLayout->getLineWidth();
}

