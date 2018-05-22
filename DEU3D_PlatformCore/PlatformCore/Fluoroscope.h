#ifndef FLUORO_SCOPE_H_CEA9D920_9FAF_472E_B216_F3957BFF6573_INCLUDE
#define FLUORO_SCOPE_H_CEA9D920_9FAF_472E_B216_F3957BFF6573_INCLUDE

//#include <osg/Switch>
#include <osg/Vec4>
#include <osg/Geode>
#include <osg/Image>
#include <osg/MatrixTransform>
#include <osg/Stencil>
#include <osg/NodeVisitor>
#include <set>
#include "IFluoroScope.h"
#include "HudLayouter.h"

//class CoordinateParam;
#pragma warning (disable : 4250)

class StatusBar;

class FluoroScope : public osg::Group, public IFluoroScope
{
protected:
    class Layouter : public HudLayouter
    {
    public:
        explicit Layouter(void);
    protected:
        virtual ~Layouter(void);

    public:
        static    osg::Stencil *getUnPenetrableAttribute(void);
        void    setFlatMode(bool bFlat) {   m_bFlatMode = bFlat;    }
        bool    isFlatMode(void) const  {   return m_bFlatMode;     }

        void    removeAllCameras(void);

    protected:
        void    removeExpiredCameras(void);

    protected:
        virtual void operator()(osg::Node *pNode, osg::NodeVisitor *pNodeVisitor);
        //virtual void    CalcViewport(const osg::Viewport *pViewport, osg::Viewport *pHudViewport);

    protected:
        static osg::ref_ptr<osg::Stencil>        m_pUnPenetrableAttribute;
        std::map<osg::Camera *, GLbitfield>        m_mapViewCameras;
        bool    m_bFlatMode;
    };

    friend class FluoroScopeEventHandler;
public:
    explicit FluoroScope(void);
protected:
    virtual ~FluoroScope(void);

public:
    virtual void        setEnable(bool bEnable);
    virtual bool        isEnable(void) const        {   return m_bEnable;   }
    virtual void        setPolygonSideCount(unsigned nCount);
    virtual unsigned    getPolygonSideCount(void) const;
    virtual void        setFluoroScopeSize(double dblSize);
    virtual double      getFluoroScopeSize(void) const;
    virtual void        setFlatMode(bool bFlat);
    virtual bool        isFlatMode(void) const {  return m_bFlatMode;   };

public:
    bool                init(void);
    void                unInit(void);

    void                setStatusBar(StatusBar *pStatusBar)   {   m_pStatusBar = pStatusBar;  }

    void                setFrameColor(const osg::Vec4 &color);
    const osg::Vec4     &getFrameColor(void) const;

    void                setFrameWidth(double dblWidth);
    double              getFrameWidth(void) const;

    void                addToWhiteList(osg::Node *pNode);
    void                removeFromBlackList(osg::Node *pNode);
    void                clearPenetrableNode(void);

    bool                isNodeInFluoroScope(osg::Node *pNode) const;

    void                setPosition(const osg::Vec3 &vecPos);

    void                createUnPenetrableAttribute(void);
    void                createPenetrableAttribute(void);

protected:
    void                createHudCamera(void);
    void                createRtsCamera(void);

    void                createFluoroScopeLens(void);
    void                createFluoroScopeFrame(void);
    osg::Matrix         calcFluoroScopeMatrix(void) const;

    virtual void        traverse(osg::NodeVisitor& nv);

protected:
    osg::ref_ptr<osg::Stencil>                  m_pPenetrableAttribute;
    osg::ref_ptr<osg::Stencil>                  m_pUnPenetrableAttribute;

    std::set< osg::observer_ptr<osg::Node> >    m_setBlackListNodes;

    osg::Vec4                                   m_clrFrameColor;
    double                                      m_dblFluoroScopeSize;
    double                                      m_dblFrameWidth;
    unsigned                                    m_nPolygonSideCount;

    osg::Vec3                                   m_vecTranslate;

    StatusBar                                   *m_pStatusBar;

    bool                                        m_bDirty;
    bool                                        m_bFlatMode;
    bool                                        m_bEnable;
    osg::ref_ptr<osg::Camera>                   m_pHUDCamera;
    osg::ref_ptr<osg::Camera>                   m_pRTSCamera;
    osg::ref_ptr<osg::MatrixTransform>          m_pFluoroScopeLensNode;
    osg::ref_ptr<osg::MatrixTransform>          m_pFluoroScopeFrameNode;
    osg::ref_ptr<FluoroScopeEventHandler>       m_pFluoroScopeEventHandler;
};

#endif
