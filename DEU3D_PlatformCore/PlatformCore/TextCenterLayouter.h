#ifndef TEXT_CENTER_LAYOUTER_H_37C52995_0F98_4BEE_9FB2_F727A3F5A31E_INCLUDE
#define TEXT_CENTER_LAYOUTER_H_37C52995_0F98_4BEE_9FB2_F727A3F5A31E_INCLUDE

#include <osg/NodeVisitor>
#include <osgText/Text>
#include <osgGA/GUIEventHandler>
#include <vector>

class TextCenterLayouter : public osgGA::GUIEventHandler
{
public:
    explicit TextCenterLayouter(void);
    virtual ~TextCenterLayouter(void);

protected:
    struct VertexArrayInfo
    {
        const osg::Array       *m_pVertexArray;
        osg::NodePath           m_NodePath;
        const VertexArrayInfo &operator=(const VertexArrayInfo &info)
        {
            if(this == &info)   return *this;
            m_pVertexArray = info.m_pVertexArray;
            m_NodePath = info.m_NodePath;
            return *this;
        }
    };

    class ElementFinder : public osg::NodeVisitor
    {
    public:
        explicit ElementFinder(void) : osg::NodeVisitor(osg::NodeVisitor::NODE_VISITOR, osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)    {}
        virtual ~ElementFinder(void)    {}

    public:

        void    clear(void)                     {   m_vecVertexInfo.clear();  m_vecTextObjects.clear();   }
        const   std::vector<VertexArrayInfo>   &getVertexInfo(void) const   {   return m_vecVertexInfo;   }
        std::vector<osgText::Text *>           &getTextObjects(void)        {   return m_vecTextObjects;  }

    protected:
        virtual void apply(osg::Geode &node);

    protected:
        std::vector<VertexArrayInfo>        m_vecVertexInfo;
        std::vector<osgText::Text *>        m_vecTextObjects;
    };

public:
    void setUpdateSpeed(unsigned nSpeed)
    {
        m_nUpdateSpeed = osg::clampAbove(nSpeed, 1u);
    }

protected:
    virtual void operator()(osg::Node *pNode, osg::NodeVisitor *pNodeVisitor);

protected:
    void    doUpdateTraverse(osg::Node *pNode, osg::NodeVisitor *pNodeVisitor);

    osg::Vec3d findMostSuitable(const osg::Matrixd &mtxTransform, const std::vector<VertexArrayInfo> &vecVertexInfo) const;

    double  findMostSuitable(const osg::Vec3Array *pVtxArray, const osg::Matrixd &mtx, osg::Vec3d &ptSuitable) const;
    double  findMostSuitable(const osg::Vec3dArray *pVtxArray, const osg::Matrixd &mtx, osg::Vec3d &ptSuitable) const;

    void    gatherMatricesOfNodePath(const osg::NodePath &nodePath, osg::Matrixd &mtxTransform) const;
    double  getVertexScore(const osg::Vec3d &vertex, const osg::Matrixd &mtx) const;

protected:
    osg::ref_ptr<ElementFinder>     m_pElementFinder;

public:
    unsigned        m_nUpdateSpeed;
};

#endif
