#include "ColorState.h"

#include <osg/PolygonOffset>
#include <osgUtil/CullVisitor>

ColorState::ColorCallBack::ColorCallBack(const osg::Vec4 &color)
{
    m_pStateSet = new osg::StateSet;
    m_pStateSet->setName("ColorState");
    osg::PolygonOffset* polyoffset = new osg::PolygonOffset;
    polyoffset->setFactor(-1.0f);
    polyoffset->setUnits(-1.0f);
    m_pStateSet->setAttributeAndModes(polyoffset,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

    m_pMaterial = new osg::Material;
    m_pMaterial->setColorMode(osg::Material::OFF);
    // turn all lighting off 
    m_pMaterial->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.0, 0.0f, 0.0f, 1.0f));
    m_pMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0.0, 0.0f, 0.0f, 1.0f));
    m_pMaterial->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0.0, 0.0f, 0.0f, 1.0f));
    // except emission... in which we set the color we desire
    m_pMaterial->setEmission(osg::Material::FRONT_AND_BACK, color);

    m_pStateSet->setAttributeAndModes(m_pMaterial, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
    m_pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
}


void ColorState::ColorCallBack::setColor(const osg::Vec4 &color)
{
    m_pMaterial->setEmission(osg::Material::FRONT_AND_BACK, color);

    if(color[3] < 1.0)
    {
        m_pMaterial->setTransparency(osg::Material::FRONT_AND_BACK, color[3]);
        m_pStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
        m_pStateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    }
    else
    {
        m_pStateSet->setMode(GL_BLEND, osg::StateAttribute::OFF);
    }
}


void ColorState::ColorCallBack::operator()(osg::Node *pNode, osg::NodeVisitor *pNodeVisitor)
{
    osgUtil::CullVisitor *cv = dynamic_cast<osgUtil::CullVisitor *>(pNodeVisitor);
    if (cv == NULL)
    {
        traverse(pNode, pNodeVisitor);
        return;
    }

    osgUtil::CullVisitor *pCullVisitor = dynamic_cast<osgUtil::CullVisitor *>(pNodeVisitor);
    pCullVisitor->pushStateSet(m_pStateSet.get());
    traverse(pNode, pNodeVisitor);
    pCullVisitor->popStateSet();

    return;
}


ColorState::ColorState(const std::string &strName) : StateBase(strName)
{
    m_color.set(1.0, 0.0, 0.0, 0.5);

    m_pColorCallBack = new ColorCallBack(m_color);
}


ColorState::~ColorState(void)
{
}

const cmm::FloatColor ColorState::getColor() const
{
    cmm::FloatColor clr;
    clr.m_fltR = m_color[0];
    clr.m_fltG = m_color[1];
    clr.m_fltB = m_color[2];
    clr.m_fltA = m_color[3];
    return clr;
}

void ColorState::setColor(const cmm::FloatColor &clr)
{
    m_color.set(clr.m_fltR, clr.m_fltG, clr.m_fltB, clr.m_fltA);

    m_pColorCallBack->setColor(m_color);
}

bool ColorState::applyState(osg::Node *pNode, bool bApply)
{
    //对应参数化做修改
    if(bApply)
    {
        pNode->addCullCallback(m_pColorCallBack);
    }
    else
    {
        pNode->removeCullCallback(m_pColorCallBack);
    }

    return true;
}