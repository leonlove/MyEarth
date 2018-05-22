#include "RippleState.h"
#include <osg/Program>
#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
#include <common/Common.h>

class PagedLODVisitor : public osg::NodeVisitor
{
public:
    explicit PagedLODVisitor(RippleState *rs, bool bApplyState): 
    osg::NodeVisitor(TRAVERSE_ALL_CHILDREN),
    m_bApplyState(bApplyState),
    m_pRS(rs){}

	virtual ~PagedLODVisitor(void){}

protected:
    virtual void apply(osg::PagedLOD &node)
	{
        m_pRS->apply((osg::Node*)&node, m_bApplyState);
		
	}
    bool m_bApplyState;
    RippleState *m_pRS;
};


char s_rippleVertexSource[] = "             \
    varying vec2  TexCoord;                 \
    void main(void)                         \
    {                                       \
       TexCoord = vec2 (gl_MultiTexCoord0); \
       gl_Position = ftransform();          \
    }                                       ";

char s_rippleFragmentSource[] = 
"varying vec2  TexCoord;"
"uniform sampler2D RippleTexture;"

"uniform vec2  hit_point;"
"uniform float t;"
"uniform float A;"
"uniform float n;"
"uniform float Freq;"

"uniform float dim_x;"
"uniform float dim_y;"


"vec4 computeAmp(vec2 hit_pt, float namda, float Amp, float f)"
"{"
"    float v = namda * f;"
"    float omega = 2 * 3.1415926 * f;"

"    vec2 left   = TexCoord; left.x   = TexCoord.x - 1.0 / dim_x; "
"    vec2 right  = TexCoord; right.x  = TexCoord.x + 1.0 / dim_x; "
"    vec2 top    = TexCoord; top.y    = TexCoord.y - 1.0 / dim_y; "
"    vec2 bottom = TexCoord; bottom.y = TexCoord.y + 1.0 / dim_y; "
    
"    vec4 dis_around;"
"    dis_around[0] = distance(hit_pt, left )  * dim_x;"
"    dis_around[1] = distance(hit_pt, right)  * dim_x;"
"    dis_around[2] = distance(hit_pt, top)    * dim_y;"
"    dis_around[3] = distance(hit_pt, bottom) * dim_y;"
    
"    vec4 amp_around;"
"    for(int i = 0; i < 4; i++)"
"    {"
"       float t0 = dis_around[i] / v;"
"       amp_around[i] = Amp * sin(omega * (t - t0));"
"    }"
    
"    return amp_around;"
"}"

"   void main()"
"   {"
"		vec2 pos = TexCoord;"

"		if (t >= 0.0f)"
"		{"
"			vec4 amp_around = computeAmp(hit_point,  n, A, Freq) + "
"			                  computeAmp(-hit_point, n, A, Freq) + "
"			                  computeAmp(vec2(-2.8,-2.8), n * 0.3, A * 0.1, Freq) +"
"			                  computeAmp(vec2(2.8,2.8), n * 0.3, A * 0.1, Freq);"
"			vec2 offset;"
"			offset.x = amp_around[1] - amp_around[0];"
"			offset.y = amp_around[3] - amp_around[2];"
"			pos += offset;"
"           if (pos.x < 0.0) pos.x = 1.0 - pos.x;"
"           if (pos.x >= 1.0) pos.x = pos.x - 1.0;"
"           if (pos.y < 0.0) pos.y = 1.0 - pos.y;"
"           if (pos.y >= 1.0) pos.y = pos.y - 1.0;"
"		 }"

"		vec3 color = vec3(texture2D(RippleTexture, pos));"
"		gl_FragColor = vec4(color, 1.0);"
"   }";

RippleState::RippleState(const std::string &strName) : StateBase(strName)
{
    m_pCallbk = new UpdateCallbk();

    m_pProgram = new osg::Program();
    osg::ref_ptr<osg::Shader> pVS = new osg::Shader(osg::Shader::VERTEX, s_rippleVertexSource);
    m_pProgram->addShader(pVS);
    osg::ref_ptr<osg::Shader> pFS = new osg::Shader(osg::Shader::FRAGMENT, s_rippleFragmentSource);
    m_pProgram->addShader(pFS);

    m_pRippleImg = osgDB::readImageFile(cmm::genResourceFileDir() + "ripple.jpg");

    if (m_pRippleImg.valid() == false)
    {
        osg::Vec2 dim(128, 128);
        m_pRippleImg = new osg::Image();
        m_pRippleImg->allocateImage(dim.x(), dim.y(), 1, GL_RGB, GL_UNSIGNED_BYTE);

        osg::Vec3 color(128,128,200);
        int max_dis = dim.x() * 0.75;
	    int center = dim.x() / 2;

        for (int i = 0; i < dim.x(); i++)
        {
            for (int j = 0; j < dim.y(); j++)
            {
                double dis  = sqrt(double(center - i) * (center - i) + (center - j) * (center - j));
                double rate = 1.0 - dis / max_dis;
			    rate /= 2;

                unsigned char *c = m_pRippleImg->data(i, j);

                *c     = (color[0] + (unsigned char)(255 * rate)) / 2;
                *(c+1) = (color[1] + (unsigned char)(255 * rate)) / 2;
                *(c+2) = (color[2] + (unsigned char)(255 * rate)) / 2;
            }
        }
    }

    m_pTexture = new osg::Texture2D(m_pRippleImg);
}

RippleState::~RippleState(void)
{
}

void RippleState::apply(osg::Node *pNode, bool bApply)
{
    osg::ref_ptr<osg::StateSet> ss = pNode->getOrCreateStateSet();
	if (!ss) return;

    if (bApply)
    {
		if (ss->getUpdateCallback()) return;

		ss->getOrCreateUniform("t", osg::Uniform::FLOAT)->set(0.0f);
        float a = 3.0f;
        ss->getOrCreateUniform("A", osg::Uniform::FLOAT)->set(3.0f);
		float n = pNode->getBound().radius() * 2;
		n = osg::clampBetween(n, 0.05f, 128.0f);
        
        ss->getOrCreateUniform("hit_pt", osg::Uniform::FLOAT_VEC2)->set(osg::Vec2(3.0 * n, 1.8 * n));
		ss->getOrCreateUniform("n", osg::Uniform::FLOAT)->set(n);
        ss->getOrCreateUniform("Freq", osg::Uniform::FLOAT)->set(0.06f);
        ss->getOrCreateUniform("dim_x", osg::Uniform::FLOAT)->set((float)m_pRippleImg->s());
		ss->getOrCreateUniform("dim_y", osg::Uniform::FLOAT)->set((float)m_pRippleImg->t());
		ss->getOrCreateUniform("RippleTexture", osg::Uniform::SAMPLER_2D)->set(0);
		
		ss->setTextureAttributeAndModes(0, m_pTexture.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        m_pTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
        m_pTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);

		m_pCallbk->m_timeBegin = clock();
        ss->setUpdateCallback(m_pCallbk);
		ss->setAttributeAndModes(m_pProgram, osg::StateAttribute::ON);
    }
    else
    {
        ss->setUpdateCallback(NULL);
		ss->getOrCreateUniform("t", osg::Uniform::FLOAT)->set(-1.0f);
        ss->removeTextureAttribute(0, m_pTexture.get());
		ss->removeAttribute(m_pProgram);
    }
}
bool RippleState::applyState(osg::Node *pNode, bool bApply)
{
    if (!pNode) return false;

    osg::ref_ptr<PagedLODVisitor> v = new PagedLODVisitor(this, bApply);
    
    pNode->accept(*v.get());
	
    return true;
}