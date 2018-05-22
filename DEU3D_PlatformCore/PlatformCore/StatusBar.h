#ifndef STATUS_BAR_H_A472FB20_8D31_43A0_8F5D_0076C3D98318_INCLUDE
#define STATUS_BAR_H_A472FB20_8D31_43A0_8F5D_0076C3D98318_INCLUDE

#include <osg/Group>
#include <osgText/Text>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include "IStatusBar.h"
#include <osgGA/GUIEventHandler>

#pragma warning (disable : 4250)

class HudLayouter;
class StatusBar : public IStatusBar, public osg::Group
{
    friend class    StatusBarEventHandler;
public:
    explicit StatusBar(void);
protected:
    virtual ~StatusBar(void);

public: // Methods from IStatusBar
    virtual void    setVisible(bool bShow = true);
    virtual bool    isVisible(void) const   {   return m_bVisible;  }

    virtual void    setUserString(const std::string &strUserString);

    virtual void    setTextColor(const cmm::FloatColor &color);
    virtual const   cmm::FloatColor &getTextColor(void) const;

    virtual void    setTextFont(const std::string &strTextFont);
    virtual const   std::string &getTextFont(void) const;

    virtual void    setTextSize(double dblSize);
    virtual double  getTextSize(void) const;

    virtual void    setPaneColor(const cmm::FloatColor &color);
    virtual const   cmm::FloatColor& getPaneColor() const;

    virtual void    setBarWidth(unsigned nWidth);
    virtual void    setDemHelper(IDemHelper *pDemHelper);


public:
    bool    initialize(void);
    void    setTargetNode(osg::Node *pTargetNode)   {   m_pTargetNode = pTargetNode;    }
    double  getCameraHieght();

protected:
    void    setFPS(double dblFPS);
    void    setInfo(const osg::Vec3d &ptMousePos, double dblCameraHeight);

protected:
    bool            m_bCreated;
    bool            m_bVisible;

    unsigned        m_nBarWidth;
    cmm::FloatColor m_clrTextColor;
    cmm::FloatColor m_clrPaneColor;

    double          m_dblTextSize;

    std::string     m_strTextFont;

    osg::ref_ptr<IDemHelper>        m_pDemHelper;
    osg::ref_ptr<osg::Camera>       m_pHudCamera;

    osg::ref_ptr<osgText::Text>     m_pFPSTextGeom;
    osg::ref_ptr<osgText::Text>     m_pInfoTextGeom;

    osg::ref_ptr<osg::Geometry>     m_pPaneGeom;
    osg::ref_ptr<osg::Node>         m_pTargetNode;

    double                          m_dblCameraHeight;

    //osg::ref_ptr<HudLayouter>               m_pHudLayouter;
    osg::ref_ptr<StatusBarEventHandler>     m_pEventHandler;
};


#endif
