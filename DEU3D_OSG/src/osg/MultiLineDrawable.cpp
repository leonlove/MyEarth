#include <osg/MultiLineDrawable>
#include <osg/CoordinateSystemNode>
#include <common/Common.h>
#include <common/Pyramid.h>

namespace osg {

class Creator
{
public:
    Creator(void)
    {
        MultiLineDrawable::instance();
    }
}__creator;

const unsigned MultiLineDrawable::ms_nCubeLevel = 13u;

MultiLineDrawable *MultiLineDrawable::instance(void)
{
    static osg::ref_ptr<MultiLineDrawable>  spMultiLineDrawable = new MultiLineDrawable;
    return spMultiLineDrawable.get();
}


MultiLineDrawable::MultiLineDrawable(void)
{
    _useDisplayList = false;
    _useVertexBufferObjects = false;
}


MultiLineDrawable::~MultiLineDrawable(void)
{
}


unsigned __int64 MultiLineDrawable::addLine(const osg::Vec3d &point0, const osg::Vec3d &point1, const osg::Vec4 &color, float fltLineWidth)
{
    const osg::Vec3d ptSegCenter((point0 + point1) * 0.5);
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    cmm::math::Point3d ptCenterCoord;
    pEllipsoidModel->convertXYZToLatLongHeight(ptSegCenter.x(), ptSegCenter.y(), ptSegCenter.z(), ptCenterCoord.y(), ptCenterCoord.x(), ptCenterCoord.z());

    const cmm::Pyramid3 *pPyramid3 = cmm::Pyramid3::instance();
    CubeID cubeID;
    pPyramid3->getCube(ms_nCubeLevel, ptCenterCoord, cubeID.m_nRow, cubeID.m_nCol, cubeID.m_nHeight);

    cmm::math::Point3d ptCubePosMinCoord, ptCubePosMaxCoord;
    pPyramid3->getCubePos(ms_nCubeLevel, cubeID.m_nRow, cubeID.m_nCol, cubeID.m_nHeight, ptCubePosMinCoord, ptCubePosMaxCoord);

    const cmm::math::Point3d  ptCubeCenterCoord((ptCubePosMinCoord + ptCubePosMaxCoord) * 0.5);
    osg::Vec3d ptCubeCenter;
    pEllipsoidModel->convertLatLongHeightToXYZ(ptCubeCenterCoord.y(), ptCubeCenterCoord.x(), ptCubeCenterCoord.z(), ptCubeCenter.x(), ptCubeCenter.y(), ptCubeCenter.z());

    ptCubeCenter.x() = floor(ptCubeCenter.x());
    ptCubeCenter.y() = floor(ptCubeCenter.y());
    ptCubeCenter.z() = floor(ptCubeCenter.z());

    const osg::Vec3d pt0(point0 - ptCubeCenter);
    const osg::Vec3d pt1(point1 - ptCubeCenter);

    OutterKey nOutterKey = 0u;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxRenderInfo);
        const LineAttr attr(color, fltLineWidth);

        LineSegCube &cubeLineSeg = m_mapLineSegsContainer[attr];
        LineSegList &listLineSeg = cubeLineSeg[cubeID];

        InnerKey nInnerKey = 0u;
        cmm::genUniqueValue64(nInnerKey);
        listLineSeg[nInnerKey] = LineSeg(pt0, pt1);
        listLineSeg.m_vecOffset = ptCubeCenter;

        cmm::genUniqueValue64(nOutterKey);
        AttributeIndex &index = m_mapAttributeIndices[nOutterKey];
        index.m_CubeID    = cubeID;
        index.m_nInnerKey = nInnerKey;
        index.m_pLineAttr = &m_mapLineSegsContainer.find(attr)->first;
        dirtyBound();
    }

    return nOutterKey;
}


void MultiLineDrawable::removeLine(unsigned __int64 nKeyValue)
{
    const OutterKey nOutterKey = nKeyValue;
    OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxRenderInfo);
    AttributeIndicesMap::const_iterator itorFindIndex = m_mapAttributeIndices.find(nOutterKey);
    if(itorFindIndex == m_mapAttributeIndices.end())
    {
        return;
    }

    const AttributeIndex &index = itorFindIndex->second;

    LineSegContainer::iterator itorFindCube = m_mapLineSegsContainer.find(*index.m_pLineAttr);
    if(itorFindCube == m_mapLineSegsContainer.end())
    {
        return;
    }

    LineSegCube &cubeLineSeg = itorFindCube->second;
    LineSegCube::iterator itorFindList = cubeLineSeg.find(index.m_CubeID);
    if(itorFindList == cubeLineSeg.end())
    {
        return;
    }

    LineSegList &listLineSeg = itorFindList->second;
    LineSegList::const_iterator itorFindSeg = listLineSeg.find(index.m_nInnerKey);
    if(itorFindSeg == listLineSeg.end())
    {
        return;
    }

    listLineSeg.erase(itorFindSeg);
    if(listLineSeg.empty())
    {
        cubeLineSeg.erase(itorFindList);
        if(cubeLineSeg.empty())
        {
            m_mapLineSegsContainer.erase(itorFindCube);
        }
    }
    m_mapAttributeIndices.erase(itorFindIndex);

    dirtyBound();
}


osg::BoundingBox MultiLineDrawable::computeBound(void) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxRenderInfo);
    osg::BoundingBoxd bbd;
    for(LineSegContainer::const_iterator itorCube = m_mapLineSegsContainer.begin(); itorCube != m_mapLineSegsContainer.end(); ++itorCube)
    {
        const LineSegCube &cubeLineSeg = itorCube->second;
        for(LineSegCube::const_iterator itorList = cubeLineSeg.begin(); itorList != cubeLineSeg.end(); ++itorList)
        {
            const LineSegList &listLineSeg = itorList->second;
            const osg::Vec3f &vecOffset = listLineSeg.m_vecOffset;
            for(LineSegList::const_iterator itorSeg = listLineSeg.begin(); itorSeg != listLineSeg.end(); ++itorSeg)
            {
                const LineSeg &seg = itorSeg->second;
                bbd.expandBy(seg.m_point0 + vecOffset);
                bbd.expandBy(seg.m_point1 + vecOffset);
            }
        }
    }

    return osg::BoundingBox(bbd._min, bbd._max);
}


void MultiLineDrawable::drawImplementation(osg::RenderInfo &renderInfo) const
{
    const osg::State *pState = renderInfo.getState();
    const osg::Matrixd &mtxViewMatrix = pState->getModelViewMatrix();

    GLfloat fltOldLineWidth = 1.0f;
    glGetFloatv(GL_LINE_WIDTH, &fltOldLineWidth);

    GLint nOldMatrixMode;
    glGetIntegerv(GL_MATRIX_MODE, &nOldMatrixMode);
    glMatrixMode(GL_MODELVIEW);

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> scope(m_mtxRenderInfo);
        for(LineSegContainer::const_iterator itorCube = m_mapLineSegsContainer.begin(); itorCube != m_mapLineSegsContainer.end(); ++itorCube)
        {
            const LineSegCube &cubeLineSeg = itorCube->second;
            const LineAttr &attr = itorCube->first;
            glLineWidth(attr.m_fltLineWidth);

            glColor4fv(attr.m_LineColor.ptr());
            for(LineSegCube::const_iterator itorList = cubeLineSeg.begin(); itorList != cubeLineSeg.end(); ++itorList)
            {
                const LineSegList &listLineSeg = itorList->second;
                const osg::Vec3f &vecOffset = listLineSeg.m_vecOffset;

                glPushMatrix();

                //glTranslatef(vecOffset.x(), vecOffset.y(), vecOffset.z());
                osg::Matrixd mtx = mtxViewMatrix;
                mtx.preMultTranslate(vecOffset);
                glLoadMatrix(mtx.ptr());

                glBegin(GL_LINES);
                for(LineSegList::const_iterator itorSeg = listLineSeg.begin(); itorSeg != listLineSeg.end(); ++itorSeg)
                {
                    const LineSeg &seg = itorSeg->second;
                    glVertex3fv(seg.m_point0.ptr());
                    glVertex3fv(seg.m_point1.ptr());
                }
                glEnd();

                glPopMatrix();
            }
        }
    }

    glMatrixMode(nOldMatrixMode);
    glLineWidth(fltOldLineWidth);
}


void MultiLineDrawable::accept(osg::PrimitiveFunctor &functor) const
{
}

}
