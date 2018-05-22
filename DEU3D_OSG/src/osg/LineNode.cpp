#include <osg/LineNode>
#include <osg/SingleLineGeode>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/LineWidth>
#include <osg/SharedStateAttributes>
#include <osg/MultiLineDrawable>

namespace osg {

LineNode::LineNode(void)
    : m_color(1.0f, 1.0f, 1.0f, 1.0f),
      m_fltLineWidth(1.0f),
      m_bCreateAsIntegration(true),
      m_bDirtyRenderInfo(true)
{
}


LineNode::LineNode(const LineNode &node,const CopyOp& copyop/*=CopyOp::SHALLOW_COPY*/)
    : Group(node, copyop)
{
    m_vecVertices = node.m_vecVertices;
    m_color = node.m_color;
    m_fltLineWidth = node.m_fltLineWidth;
    m_bCreateAsIntegration = node.m_bCreateAsIntegration;
    m_bDirtyRenderInfo = node.m_bDirtyRenderInfo;
}


LineNode::~LineNode(void)
{
}


void LineNode::traverse(NodeVisitor& nv)
{
    if(nv.getVisitorType() == NodeVisitor::CULL_VISITOR)
    {
        if(m_bDirtyRenderInfo)
        {
            removeChildren(0u, getNumChildren());

            ref_ptr<Node> pNode = NULL;
            if(m_bCreateAsIntegration)
            {
                pNode = createAsIntegration();
            }
            else
            {
                pNode = createAsNonIntegration();
            }

            if(pNode.valid())
            {
                addChild(pNode.get());
            }

            m_bDirtyRenderInfo = false;
        }
    }

    if(!m_bDirtyRenderInfo)
    {
        Group::traverse(nv);
    }
}


Node *LineNode::createAsIntegration(void)
{
    if(m_vecVertices.size() < 2u)   return NULL;

    if(m_vecVertices.size() == 2u)
    {
        const osg::Vec3d &point0 = m_vecVertices[0];
        const osg::Vec3d &point1 = m_vecVertices[1];
        osg::ref_ptr<SingleLineGeode>  pLineGeode = new SingleLineGeode(point0, point1, m_color, (float)m_fltLineWidth);
        return pLineGeode.release();
    }

    osg::ref_ptr<osg::Group>    pGroup = new osg::Group;
    for(unsigned i = 0u; i < m_vecVertices.size() - 1u; i++)
    {
        const Vec3d &point0 = m_vecVertices[i];
        const Vec3d &point1 = m_vecVertices[i + 1u];
        osg::ref_ptr<osg::SingleLineGeode>  pLineGeode = new SingleLineGeode(point0, point1, m_color, (float)m_fltLineWidth);
        pGroup->addChild(pLineGeode.get());
    }
    return pGroup.release();
}


Node *LineNode::createAsNonIntegration(void)
{
    if(m_vecVertices.size() < 2u)   return NULL;

    unsigned int i = 0u;
    Vec3d ptCenter(0.0, 0.0, 0.0);
    for(i = 0u; i < m_vecVertices.size(); i++)
    {
        ptCenter += m_vecVertices[i];
    }
    ptCenter /= double(m_vecVertices.size());

    ref_ptr<Vec3Array>   pCoordArray = new Vec3Array;
    for(i = 0u; i < m_vecVertices.size(); i++)
    {
        const Vec3d &vtx = m_vecVertices[i];
        pCoordArray->push_back(vtx - ptCenter);
    }

    osg::ref_ptr<Vec2Array>    pTexCoord = new osg::Vec2Array;
    float fTotalLen = 0.0;
    for(i = 0u; i < pCoordArray->size(); i++)
    {
        const Vec3 &point = (*pCoordArray)[i];

        if(i == 0u)
        {
            pTexCoord->push_back(Vec2(0.0f, 0.0f));
        }
        else
        {
            const Vec3d &ptPrev = (*pCoordArray)[i - 1u];
            const float fltLen = (ptPrev - point).length();
            fTotalLen += fltLen;
            pTexCoord->push_back(Vec2(fltLen, 1.0f));
        }
    }

    for(i = 0u; i < pTexCoord->size(); i++)
    {
        (*pTexCoord)[i][0u] /= fTotalLen;
    }

    ref_ptr<MatrixTransform> pMatrixTransform = new MatrixTransform(Matrixd::translate(ptCenter));

    ref_ptr<Geode> pGeode = new Geode;
    pMatrixTransform->addChild(pGeode);

    ref_ptr<Geometry> pGeometry = new Geometry;
    pGeode->addDrawable(pGeometry);

    pGeometry->setVertexArray(pCoordArray.get());
    pGeometry->setTexCoordArray(0u, pTexCoord.get());
    pGeometry->addPrimitiveSet(new DrawArrays(PrimitiveSet::LINE_STRIP, 0, pCoordArray->size()));


    //ÉèÖÃÊôÐÔ
    StateSet *pStateSet = pMatrixTransform->getOrCreateStateSet();
    pStateSet->setMode(GL_LIGHTING, StateAttribute::OFF | StateAttribute::PROTECTED);

    ref_ptr<Vec4Array> pColorArray = SharedStateAttributes::instance()->getColorArrayByColor(m_color);
    pGeometry->setColorArray(pColorArray.get());
    pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    if(m_fltLineWidth > 1.0f)
    {
        LineWidth *pLineWidth = SharedStateAttributes::instance()->getLineWidth(m_fltLineWidth);
        pStateSet->setAttribute(pLineWidth);
    }

    pMatrixTransform->getOrCreateStateSet()->getOrCreateUniform("btexture", osg::Uniform::BOOL)->set(false);
    return pMatrixTransform.release();
}


void LineNode::setIntegration(bool bIntegration)
{
    if(!m_bCreateAsIntegration == !bIntegration)
    {
        return;
    }
    m_bCreateAsIntegration = bIntegration;
    m_bDirtyRenderInfo = true;
}


void LineNode::addVertex(const Vec3d &vtx)
{
    m_vecVertices.push_back(vtx);
    m_bDirtyRenderInfo = true;
}


bool LineNode::insertVertex(unsigned pos, const Vec3d &vtx)
{
    if(pos > m_vecVertices.size())
    {
        return false;
    }
    m_vecVertices.insert(m_vecVertices.begin() + pos, vtx);
    m_bDirtyRenderInfo = true;
    return true;
}


bool LineNode::removeVertex(unsigned pos)
{
    if(pos >= m_vecVertices.size())
    {
        return false;
    }

    m_vecVertices.erase(m_vecVertices.begin() + pos);
    m_bDirtyRenderInfo = true;
    return true;
}


void LineNode::OnPagedLODExpired(void)
{
    NodeList::iterator itor = _children.begin();
    for( ; itor != _children.end(); ++itor)
    {
        osg::SingleLineGeode *pLineGeode = dynamic_cast<osg::SingleLineGeode *>(itor->get());
        if(!pLineGeode) continue;

        const unsigned __int64 nLineKey = pLineGeode->getLineKey();
        MultiLineDrawable::instance()->removeLine(nLineKey);
    }
    removeChildren(0u, _children.size());
    m_bDirtyRenderInfo = true;
}


}