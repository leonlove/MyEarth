
#include "OutlineLayouter.h"
#include <osgUtil/CullVisitor>

const static unsigned int Override_On  = osg::StateAttribute::ON  | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED;
const static unsigned int Override_Off = osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED;

OutlineLayouter::OutlineLayouter(void)
{
    m_fltWidth = 8.0f;
    m_clrColor = osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f);
    define_techniques();
}


OutlineLayouter::OutlineLayouter(float fltWidth, const osg::Vec4& color)
{
    m_fltWidth = fltWidth;
    m_clrColor = color;
    define_techniques();
}


OutlineLayouter::~OutlineLayouter(void)
{
}


void OutlineLayouter::setWidth(float width)
{
    m_fltWidth = width;
    if (m_pLineWidth.valid())
    {
        m_pLineWidth->setWidth(width);
    }
}


void OutlineLayouter::setColor(const osg::Vec4 &vColor)
{
    m_clrColor = vColor;
    if (m_pMaterial.valid())
    {
        const osg::Material::Face face = osg::Material::FRONT_AND_BACK;
        //m_pMaterial->setAmbient(face, osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        //m_pMaterial->setDiffuse(face, osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        //m_pMaterial->setSpecular(face, osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        m_pMaterial->setAmbient(face, m_clrColor);
        m_pMaterial->setDiffuse(face, m_clrColor);
        m_pMaterial->setSpecular(face, m_clrColor);
        m_pMaterial->setEmission(face, m_clrColor);
    }
}


void OutlineLayouter::define_passes() 
{
    /*
        * draw
        * - set stencil buffer to ref=1 where draw occurs
        * - clear stencil buffer to 0 where test fails
        */
    {
        osg::StateSet* state = new osg::StateSet;

        // stencil op
        osg::Stencil* stencil  = new osg::Stencil;
        stencil->setFunction(osg::Stencil::ALWAYS, 1, ~0u);
        stencil->setOperation(osg::Stencil::KEEP,
                                osg::Stencil::KEEP,
                                osg::Stencil::REPLACE);
        state->setAttributeAndModes(stencil, Override_On);

        addPass(state);
    }

    /*
        * post-draw
        * - only draw where draw didn't set the stencil buffer
        * - draw only back-facing polygons
        * - draw back-facing polys as lines
        * - disable depth-test, lighting & texture
        */
    {
        osg::StateSet* state = new osg::StateSet;

        // stencil op
        osg::Stencil* stencil  = new osg::Stencil;
        stencil->setFunction(osg::Stencil::NOTEQUAL, 1, ~0u);
        stencil->setOperation(osg::Stencil::KEEP,
                                osg::Stencil::KEEP,
                                osg::Stencil::KEEP);
        state->setAttributeAndModes(stencil, Override_On);

        // cull front-facing polys
        osg::CullFace* cullFace = new osg::CullFace;
        cullFace->setMode(osg::CullFace::FRONT);
        state->setAttributeAndModes(cullFace, Override_On);

        // draw back-facing polygon lines
        osg::PolygonMode* polyMode = new osg::PolygonMode;
        polyMode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
        state->setAttributeAndModes(polyMode, Override_On);

        // outline width
        m_pLineWidth = new osg::LineWidth;
        setWidth(m_fltWidth);
        state->setAttributeAndModes(m_pLineWidth.get(), Override_On);

        // outline color/material
        m_pMaterial = new osg::Material;
        m_pMaterial->setColorMode(osg::Material::OFF);
        setColor(m_clrColor);
        state->setAttributeAndModes(m_pMaterial.get(), Override_On);

        // disable modes
        state->setMode(GL_BLEND, Override_Off);
        //state->setMode(GL_DEPTH_TEST, Override_Off);
        state->setTextureMode(0, GL_TEXTURE_1D, Override_Off);
        state->setTextureMode(0, GL_TEXTURE_2D, Override_Off);
        state->setTextureMode(0, GL_TEXTURE_3D, Override_Off);

        osg::ref_ptr<osg::CullFace> pCullFace = new osg::CullFace(osg::CullFace::FRONT_AND_BACK);
        state->setAttributeAndModes(pCullFace.get(), osg::StateAttribute::OFF);

        addPass(state);
    }
}

void OutlineLayouter::addPass(osg::StateSet* ss)
{
    if (ss) 
    {
        _passes.push_back(ss);
        ss->setRenderBinDetails(static_cast<int>(_passes.size()), "RenderBin");
    }
}

bool OutlineLayouter::define_techniques()
{
    setWidth(m_fltWidth);
    setColor(m_clrColor);
    define_passes();
    return true;
}


void OutlineLayouter::operator()(osg::Node *pNode, osg::NodeVisitor *pNodeVisitor)
{
    osgUtil::CullVisitor *pCullVisitor = dynamic_cast<osgUtil::CullVisitor *>(pNodeVisitor);
    if (pCullVisitor == NULL)
    {
        traverse(pNode, pNodeVisitor);
        return;
    }

    for(unsigned i = 0; i < _passes.size(); i++)
    {
        pCullVisitor->pushStateSet(_passes.at(i));
        traverse(pNode, pNodeVisitor);
        pCullVisitor->popStateSet();
    }
}

