#ifndef HUD_LAYOUTER_H_53354F2D_1C20_4356_9752_E9348F85E8AC_INCLUDE
#define HUD_LAYOUTER_H_53354F2D_1C20_4356_9752_E9348F85E8AC_INCLUDE

#include <osg/NodeCallback>
#include <osg/Viewport>
#include <osgUtil/CullVisitor>

class HudLayouter : public osg::NodeCallback
{
public:
    explicit HudLayouter(void);
protected:
    virtual ~HudLayouter(void);

public:
    void    setReference(bool bHorzAbs, bool bVertAbs);
    void    setPosition(float x, float y, float width, float height);
    void    setLayoutCamera(bool bLayout);
    void    setRefreshSpeed(unsigned nRefreshSpeed);

protected:
    virtual void operator()(osg::Node *pNode, osg::NodeVisitor *pNodeVisitor);

protected:
    virtual void    calcViewport(const osg::Viewport *pViewport, osg::Viewport *pHudViewport);
    osg::Camera    *findViewportCamera(osg::Node *pNode, osgUtil::CullVisitor *pCullVisitor);

protected:
    float       m_fltX, m_fltY, m_fltWidth, m_fltHeight;
    bool        m_bHorzAbsolute;
    bool        m_bVertAbsolute;
    bool        m_bLayoutCamera;
    unsigned    m_nRefreshSpeed;
};


#endif
