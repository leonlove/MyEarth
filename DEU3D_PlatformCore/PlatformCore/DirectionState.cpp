#include "DirectionState.h"

#include <assert.h>
#include <osg/Image>
#include <osgUtil/CullVisitor>
#include <osg/TexEnv>
#include <osg/Group>
#include <common/deuMath.h>
#include <IDProvider/Definer.h>

static char s_DVertexSource[] =
    "varying vec2 gv_ptTexcoord0;\n"
    "varying vec4 gv_Color;\n"
    "varying vec4 gv_vecLightIndensity;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    gv_ptTexcoord0   = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;\n"
    "    vec3 vecNormal   = normalize(gl_NormalMatrix * gl_Normal).xyz;\n"
    "    vec3 vecLightDir = normalize(gl_LightSource[0].position.xyz);\n"
    "    float fltDot     = dot(vecLightDir, vecNormal);\n"
    "    float fltLight   = max(fltDot, 0.0);\n"
    "    gv_vecLightIndensity = gl_LightSource[0].diffuse * fltLight\n"
    "                         + gl_LightSource[0].ambient;\n"
    "    gv_Color = gl_Color;\n"
    "    gl_Position    = ftransform();\n"
    "}\n";

static char s_DFragmentSource[] =
    "uniform sampler2D  texture;\n"
    "uniform bool       btexture;\n"
    "uniform float      time;\n"
    "uniform float      speed;\n"
    "uniform float      step;\n"
    "uniform vec4       color;\n"
    "\n"
    "varying vec2       gv_ptTexcoord0;\n"
    "varying vec4       gv_Color;\n"
    "varying vec4       gv_vecLightIndensity;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    vec4 fcolor    = color;\n"
    "    float fs       = floor((gv_ptTexcoord0.x + time * speed) / step);\n"
    "    float mod      = mod(fs, 2.0);\n"
    "    if(mod == 0.0)"
    "    {\n"
    "       if(btexture)\n"
    "       {\n"
    "           fcolor = texture2D(texture, gv_ptTexcoord0);\n"
    "       }\n"
    "       else\n"
    "       {\n"
    "           fcolor = gv_Color;\n"
    "       }\n"
    "    }\n"
    "    //gl_FragColor = fcolor;\n"
    "    gl_FragColor.x = fcolor.x * gv_vecLightIndensity.x;\n"
    "    gl_FragColor.y = fcolor.y * gv_vecLightIndensity.y;\n"
    "    gl_FragColor.z = fcolor.z * gv_vecLightIndensity.z;\n"
    "    gl_FragColor.w = 1.0;\n"
    "}\n";

DirectionState::DirectionCallBack::DirectionCallBack(void) : m_nFrameNum(0)
{
    m_pProgram = new osg::Program();
    osg::ref_ptr<osg::Shader> pVS = new osg::Shader(osg::Shader::VERTEX, s_DVertexSource);
    m_pProgram->addShader(pVS);
    osg::ref_ptr<osg::Shader> pFS = new osg::Shader(osg::Shader::FRAGMENT, s_DFragmentSource);
    m_pProgram->addShader(pFS);


    m_pColorUniform = new osg::Uniform("color", osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f));
    m_pSpeedUniform = new osg::Uniform("speed", 1.0f);
    m_pStepUniform = new osg::Uniform("step", 1.0f);
    m_pTimeUniform = new osg::Uniform("time", 0.0f);
}


void DirectionState::DirectionCallBack::operator()(osg::Node *pNode, osg::NodeVisitor *pNodeVisitor)
{
    osgUtil::CullVisitor *pCullVisitor = dynamic_cast<osgUtil::CullVisitor *>(pNodeVisitor);
    if (pCullVisitor == NULL)
    {
        traverse(pNode, pNodeVisitor);
        return;
    }

    osg::Group *pGroup = dynamic_cast<osg::Group *>(pNode);
    const osg::FrameStamp *pFrameStamp = pNodeVisitor->getFrameStamp();

    if(m_nFrameNum <= pFrameStamp->getFrameNumber())
    {
        float fTime = (float)(pFrameStamp->getReferenceTime());
        m_pTimeUniform->set(fTime);
    }

    traverse(pNode, pNodeVisitor);
}


cmm::FloatColor DirectionState::DirectionCallBack::getColor(void) const
{
    cmm::FloatColor clr;

    osg::Vec4 vClr;
    m_pColorUniform->get(vClr);
    clr.m_fltR = vClr[0];
    clr.m_fltG = vClr[1];
    clr.m_fltB = vClr[2];
    clr.m_fltA = vClr[3];

    return clr;
}

void DirectionState::DirectionCallBack::setColor(const cmm::FloatColor &clr)
{
    osg::Vec4 vClr(clr.m_fltR, clr.m_fltG, clr.m_fltB, clr.m_fltA);
    m_pColorUniform->set(vClr);
}

double DirectionState::DirectionCallBack::getSpeed(void) const
{
    float fSpeed;
    m_pSpeedUniform->get(fSpeed);
    return fSpeed;
}

void DirectionState::DirectionCallBack::setSpeed(double dblSpeed)
{
    m_pSpeedUniform->set(float(dblSpeed));
}

double DirectionState::DirectionCallBack::getInterval(void) const
{
    float fStep;
    m_pStepUniform->get(fStep);
    return fStep;
}

void DirectionState::DirectionCallBack::setInterval(double dblInterval)
{
    m_pStepUniform->set(float(dblInterval));
}


void DirectionState::DirectionCallBack::applyState(osg::StateSet *pStateSet, bool bApply)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
    if(bApply)
    {
        pStateSet->setAttributeAndModes(m_pProgram, osg::StateAttribute::ON);
        pStateSet->addUniform(m_pColorUniform);
        pStateSet->addUniform(m_pSpeedUniform);
        pStateSet->addUniform(m_pStepUniform);
        pStateSet->addUniform(m_pTimeUniform);
    }
    else
    {
        pStateSet->removeAttribute(m_pProgram);
        pStateSet->removeUniform(m_pColorUniform);
        pStateSet->removeUniform(m_pSpeedUniform);
        pStateSet->removeUniform(m_pStepUniform);
        pStateSet->removeUniform(m_pTimeUniform);
    }
}


DirectionState::DirectionState(const std::string &strName) : StateBase(strName)
{
    m_pDirectionCallBack = new DirectionState::DirectionCallBack();
}


DirectionState::~DirectionState(void)
{
}

cmm::FloatColor DirectionState::getColor(void) const
{
    return m_pDirectionCallBack->getColor();
}

void DirectionState::setColor(const cmm::FloatColor &clr)
{
    m_pDirectionCallBack->setColor(clr);
}


double DirectionState::getSpeed() const
{
    return m_pDirectionCallBack->getSpeed();
}

void DirectionState::setSpeed(double dblSpeed)
{
    m_pDirectionCallBack->setSpeed(dblSpeed);
}

double DirectionState::getInterval(void) const
{
    return m_pDirectionCallBack->getInterval();
}

void DirectionState::setInterval(double dblInterval)
{
    m_pDirectionCallBack->setInterval(dblInterval);
}

bool DirectionState::applyState(osg::Node *pNode, bool bApply)
{
    //只对线参数处理
    const ID &id = pNode->getID();
    if(!id.isValid() || id.ObjectID.m_nType != PARAM_LINE_ID)
    {
        return false;
    }

    osg::StateSet *pStateSet = pNode->getOrCreateStateSet();

    if(bApply)
    {
        pNode->addCullCallback(m_pDirectionCallBack);
    }
    else
    {
        pNode->removeCullCallback(m_pDirectionCallBack);
    }
    m_pDirectionCallBack->applyState(pStateSet, bApply);

    return true;
}