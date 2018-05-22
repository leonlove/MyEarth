#include "LineParameterNode.h"
#include <osg/CullFace>

#include <ParameterSys/ILineParameter.h>
#include <ParameterSys/IDetail.h>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osgDB/Registry>
#include <osg/PolygonOffset>
#include <osg/ShapeDrawable>
#include <osgDB/ReadFile>
#include <IDProvider/Definer.h>

#include "FileReadInterceptor.h"
#include "Registry.h"
#include "ParmRectifyThreadPool.h"


LineParameterNode::LineParameterNode(param::IParameter *pParameter) : 
    ParameterNode(pParameter)
{
}


LineParameterNode::~LineParameterNode(void)
{
}


bool LineParameterNode::initFromParameter(void)
{
    if(!ParameterNode::initFromParameter())
    {
        return false;
    }

    param::ILineParameter *pLineParamter = dynamic_cast<param::ILineParameter *>(m_pParameter.get());

    if(pLineParamter == NULL)
    {
        return false;
    }

    m_bMagnet = pLineParamter->getTerrainMagnet();

    m_vecPoints.clear();

    std::vector<cmm::math::Point3d> vecCoords = pLineParamter->getCoordinates();

    osg::ref_ptr<osg::Vec3dArray> pVertex = new osg::Vec3dArray;
    for(unsigned int i = 0; i < vecCoords.size(); i++)
    {
        pVertex->push_back(osg::Vec3d(vecCoords[i].x(), vecCoords[i].y(), vecCoords[i].z()));
    }

    std::vector<std::pair<unsigned int, unsigned int> > vecPart;
    for(unsigned int i = 0; i < pLineParamter->getPartCount(); ++i)
    {
        unsigned int nOffset, nCount;
        pLineParamter->getPart(i, nOffset, nCount);
        vecPart.push_back(std::make_pair(nOffset, nCount));
    }

    if(vecPart.empty())
    {
        m_vecPoints.push_back(pVertex);
    }
    else
    {
        for(unsigned int i = 0; i < vecPart.size(); i++)
        {
            osg::ref_ptr<osg::Vec3dArray> pTempVertex = new osg::Vec3dArray(pVertex->begin() + vecPart[i].first, pVertex->begin() + vecPart[i].first + vecPart[i].second);
            m_vecPoints.push_back(pTempVertex);
        }
    }

    removeChildren(0, getNumChildren());
    addChild(createNodeByParameter(m_vecPoints));
    return true;
}


osg::Node *LineParameterNode::createNodeByParameter(const std::vector<osg::ref_ptr<osg::Vec3dArray> > &vecPoints) const
{
    param::ISymbol *pSymbol = m_pParameter->getSymbol();

    osg::ref_ptr<osg::LOD> pLOD = new osg::LOD;

    for(unsigned int i = 0; i < pSymbol->getNumDetail(); i++)
    {
        OpenSP::sp<param::IDetail> pDetail = pSymbol->getDetail(i);

        addChildByDetail(pLOD, pDetail, vecPoints);
    }
    
    return pLOD.release();
}


void LineParameterNode::addChildByDetail(osg::LOD *pLOD, const param::IDetail *pDetail, const std::vector<osg::ref_ptr<osg::Vec3dArray> > &vecPoints) const
{
    if(pDetail == NULL)
    {
        return;
    }

    double dblMin, dblMax;
    pDetail->getRange(dblMin, dblMax);

    const std::string &strType = pDetail->getStyle();

    osg::ref_ptr<osg::Node> pDetailNode = NULL;
    if(strType.compare(param::LINE_DETAIL) == 0)
    {
        pDetailNode = createLineDetail(pDetail, vecPoints);
    }
    else if(strType.compare(param::CYLINDER_DETIAL) == 0)
    {
        pDetailNode = createCylinderDetail(pDetail, vecPoints);
    }
    else if(strType.compare(param::CUBE_DETAIL) == 0)
    {
        pDetailNode = createCubeDetail(pDetail, vecPoints);
    }
    else if(strType.compare(param::Arrow_DETAIL) == 0)
    {
        pDetailNode = createArrowDetail(pDetail, vecPoints);
    }

    if(pDetailNode.valid())
    {
        pLOD->addChild(pDetailNode, dblMin, dblMax);
    }
}


osg::Node *LineParameterNode::createLineDetail(const param::IDetail *pDetail, const std::vector<osg::ref_ptr<osg::Vec3dArray> > &vecPoints) const
{
    const param::IDynLineDetail *pLineDetail = dynamic_cast<const param::IDynLineDetail *>(pDetail);
    if(pLineDetail == NULL)
    {
        return NULL;
    }

    const cmm::FloatColor &clr = pLineDetail->getLineColor();
    const double dblLineWidth = pLineDetail->getLineWidth();

    osg::ref_ptr<osg::Group> pGroup = new osg::Group;

    osg::ref_ptr<osg::Vec4Array> pColor = new osg::Vec4Array;
    pColor->push_back(osg::Vec4(clr.m_fltR, clr.m_fltG, clr.m_fltB, clr.m_fltA));

    osg::ref_ptr<osg::LineWidth> pLineWidth = new osg::LineWidth;
    pLineWidth->setWidth(dblLineWidth);

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    std::vector<osg::ref_ptr<osg::Vec3dArray> >::const_iterator itor = vecPoints.begin();
    for( ; itor != vecPoints.end(); ++itor)
    {
        osg::ref_ptr<osg::Vec3Array> pVertex = new osg::Vec3Array;
        osg::Vec3d vCenter;
        for(unsigned int i = 0; i < (*itor)->size(); i++)
        {
            osg::Vec3d &pt1 = (*itor)->at(i);
            osg::Vec3d pt2;
            pEllipsoidModel->convertLatLongHeightToXYZ(pt1._v[1], pt1._v[0], pt1._v[2] + m_dblHeight, pt2._v[0], pt2._v[1], pt2._v[2]);
            vCenter += pt2;
            pVertex->push_back(pt2);
        }

        vCenter /= (double)(pVertex->size());

        for(unsigned int i = 0; i < pVertex->size(); i++)
        {
            (*pVertex)[i] -= vCenter;
        }

        //计算每一段的长度
        std::vector<double> vecLen;
        double dblAllLen = 0.0;
        for(unsigned int i = 0; i < pVertex->size() - 1; i++)
        {
            osg::Vec3d vDis = (*pVertex)[i + 1] - (*pVertex)[i];
            dblAllLen += vDis.length();
            vecLen.push_back(dblAllLen);
        }

        osg::ref_ptr<osg::Vec2Array> pCoordArray = new osg::Vec2Array;
        pCoordArray->push_back(osg::Vec2(0.0f, 0.0f));
        for(unsigned int i = 0; i < vecLen.size(); i++)
        {
            double dblRatio = vecLen[i] / dblAllLen;
            pCoordArray->push_back(osg::Vec2(dblRatio, 0.0));
        }


        osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform;
        osg::Matrix matrix;
        matrix.setTrans(vCenter);
        pMatrixTransform->setMatrix(matrix);

        osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
        osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
        pGeometry->setVertexArray(pVertex);
        pGeometry->setColorArray(pColor);
        pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        pGeometry->getOrCreateStateSet()->setAttribute(pLineWidth.get());
        pGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        pGeometry->getOrCreateStateSet()->setAttributeAndModes(new osg::PolygonOffset(-1.0, -1000.0), osg::StateAttribute::ON);
        pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, pVertex->size()));
        pGeometry->setTexCoordArray(0, pCoordArray.get());
        pGeode->addDrawable(pGeometry);
        pMatrixTransform->addChild(pGeode);
        pGroup->addChild(pMatrixTransform);
    }
    return pGroup.release();
}

osg::Node *LineParameterNode::createArrowDetail(const param::IDetail *pDetail, const std::vector<osg::ref_ptr<osg::Vec3dArray> > &vecPoints) const
{
    const param::IArrowDetail *pArrowDetail = dynamic_cast<const param::IArrowDetail *>(pDetail);
    if(pArrowDetail == NULL)
    {
        return NULL;
    }

    const cmm::FloatColor &rgbArrow = pArrowDetail->getArrowColor();
    const cmm::FloatColor &rgbLine = pArrowDetail->getLineColor();

    const double dblArrowLength = pArrowDetail->getArrowLength();
    const double dblIntervalLength = pArrowDetail->getIntervalLength();
    const double dblHeadLength = dblArrowLength * 0.1;
    const double dblBodyLength = dblArrowLength - dblHeadLength;

    const osg::Vec4 vArrowColor(rgbArrow.m_fltR, rgbArrow.m_fltG, rgbArrow.m_fltB, rgbArrow.m_fltA);
    const osg::Vec4 vLineColor(rgbLine.m_fltR, rgbLine.m_fltG, rgbLine.m_fltB, rgbLine.m_fltA);

    osg::ref_ptr<osg::Group> pGroup = new osg::Group;
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    std::vector<osg::ref_ptr<osg::Vec3dArray> >::const_iterator itor = vecPoints.begin();
    for( ; itor != vecPoints.end(); ++itor)
    {
        const osg::Vec3dArray   *pVec3dArray = itor->get();
        for(unsigned int i = 0; i < pVec3dArray->size() - 1; i++)
        {
            const osg::Vec3d &pt1 = pVec3dArray->at(i);
            const osg::Vec3d &pt2 = pVec3dArray->at(i + 1);

            osg::Vec3d ptTemp1, ptTemp2;
            pEllipsoidModel->convertLatLongHeightToXYZ(pt1.y(), pt1.x(), pt1.z() + m_dblHeight, ptTemp1.x(), ptTemp1.y(), ptTemp1.z());
            pEllipsoidModel->convertLatLongHeightToXYZ(pt2.y(), pt2.x(), pt2.z() + m_dblHeight, ptTemp2.x(), ptTemp2.y(), ptTemp2.z());

            //中心点
            const osg::Vec3d ptCenter = (ptTemp1 + ptTemp2) * 0.5;

            //管线的长度
            osg::Vec3d vecDir = ptTemp2 - ptTemp1;
            const double dblLength = vecDir.normalize();

            //管线Z轴的夹角
            const osg::Vec3d vecAxisZ(0.0, 0.0, 1.0);

            //把管线转到正确的方向
            osg::Quat qtRotation;
            qtRotation.makeRotate(vecAxisZ, vecDir);

            osg::ref_ptr<osg::TessellationHints> pHints = new osg::TessellationHints;
            pHints->setTessellationMode(osg::TessellationHints::USE_TARGET_NUM_FACES);

            pHints->setTargetNumFaces((unsigned) (16 / dblArrowLength));
            if (pHints->getTargetNumFaces() < 3) pHints->setTargetNumFaces(3);

            double dblRadius = dblArrowLength / 20;

            double cur_len = 0.0;
            osg::Vec3d tail = ptTemp1;

            while (cur_len < dblLength)
            {
                osg::Matrixd m;
                osg::Vec3d head = tail + vecDir * dblBodyLength;
                m.setTrans((tail + head) * 0.5);

                osg::Cylinder *pCylinder = new osg::Cylinder(osg::Vec3d(0, 0, 0), dblRadius, dblBodyLength);
                pCylinder->setRotation(qtRotation);

                osg::ShapeDrawable *pSD = new osg::ShapeDrawable(pCylinder, pHints);
                pSD->setColor(vLineColor);

                osg::ref_ptr<osg::Geode> pCylinderGeode = new osg::Geode;
                pCylinderGeode->addDrawable(pSD);
                pCylinderGeode->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT_AND_BACK), osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

                osg::ref_ptr<osg::MatrixTransform> pMTCylinder = new osg::MatrixTransform;
                pMTCylinder->addChild(pCylinderGeode);
                pMTCylinder->setMatrix(m);
                pGroup->addChild(pMTCylinder);

                osg::Cone *Cone = new osg::Cone(osg::Vec3d(0, 0, 0), dblRadius * 1.5, dblHeadLength);
                Cone->setRotation(qtRotation);
                
                osg::ShapeDrawable *pArrow = new osg::ShapeDrawable(Cone, pHints);
                pArrow->setColor(vArrowColor);

                osg::ref_ptr<osg::Geode> pArrowGeode = new osg::Geode;
                pArrowGeode->addDrawable(pArrow);
                pArrowGeode->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT_AND_BACK), osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
                
                m.setTrans(head);
                osg::ref_ptr<osg::MatrixTransform> pMTArrow = new osg::MatrixTransform;
                pMTArrow->addChild(pArrowGeode);
                pMTArrow->setMatrix(m);
                pGroup->addChild(pMTArrow);

                cur_len += dblArrowLength + dblIntervalLength;
                tail = ptTemp1 + vecDir * cur_len;
            }
        }
    }

    return pGroup.release();
}

osg::Node *LineParameterNode::createCylinderDetail(const param::IDetail *pDetail, const std::vector<osg::ref_ptr<osg::Vec3dArray> > &vecPoints) const
{
    const param::ICylinderDetail *pCylinderDetail = dynamic_cast<const param::ICylinderDetail *>(pDetail);
    if(pCylinderDetail == NULL)
    {
        return NULL;
    }

    const cmm::FloatColor &clr = pCylinderDetail->getDynModelColor();
    const bool bTopVisible = pCylinderDetail->getTopVisible();
    const bool bBottomVisible = pCylinderDetail->getBottomVisible();
    const double dblRadius = pCylinderDetail->getRadiusBottom();

    osg::ref_ptr<osg::Group> pGroup = new osg::Group;

    const osg::Vec4 vColor(clr.m_fltR, clr.m_fltG, clr.m_fltB, clr.m_fltA);

    const ID &imgID = pCylinderDetail->getImageID();

    osg::ref_ptr<osg::Texture> pTexture;
    if(imgID.isValid() && imgID.ModelID.m_nType == IMAGE_ID)
    {
        pTexture = bindTexture(imgID, NULL);
    }

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    std::vector<osg::ref_ptr<osg::Vec3dArray> >::const_iterator itor = vecPoints.begin();
    for( ; itor != vecPoints.end(); ++itor)
    {
        const osg::Vec3dArray   *pVec3dArray = itor->get();
        for(unsigned int i = 0; i < pVec3dArray->size() - 1; i++)
        {
            osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform;
            osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
            const osg::Vec3d &pt1 = pVec3dArray->at(i);
            const osg::Vec3d &pt2 = pVec3dArray->at(i + 1);

            osg::Vec3d ptTemp1, ptTemp2;
            pEllipsoidModel->convertLatLongHeightToXYZ(pt1.y(), pt1.x(), pt1.z() + m_dblHeight, ptTemp1.x(), ptTemp1.y(), ptTemp1.z());
            pEllipsoidModel->convertLatLongHeightToXYZ(pt2.y(), pt2.x(), pt2.z() + m_dblHeight, ptTemp2.x(), ptTemp2.y(), ptTemp2.z());

            //中心点
            const osg::Vec3d ptCenter = (ptTemp1 + ptTemp2) * 0.5;

            //管线的长度
            osg::Vec3d vecLen = ptTemp2 - ptTemp1;
            const double dblLength = vecLen.normalize();

            //管线Z轴的夹角
            const osg::Vec3d vecAxisZ(0.0, 0.0, 1.0);

            //把管线转到正确的方向
            osg::Quat qtRotation;
            qtRotation.makeRotate(vecAxisZ, vecLen);

            osg::Cylinder *pCylinder = new osg::Cylinder(osg::Vec3d(0, 0, 0), dblRadius, dblLength);
            pCylinder->setRotation(qtRotation);

            osg::ref_ptr<osg::TessellationHints> pHints = new osg::TessellationHints;
            pHints->setCreateTop(bTopVisible);
            pHints->setCreateBottom(bBottomVisible);
            pHints->setDetailRatio(0.5);
            pHints->setTextureRatio(dblLength);
            osg::ShapeDrawable *pSD = new osg::ShapeDrawable(pCylinder, pHints);
            pSD->getOrCreateStateSet()->setTextureAttributeAndModes(0, pTexture.get(), osg::StateAttribute::ON);
            pSD->setColor(vColor);
            pGeode->addDrawable(pSD);
            pGeode->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT_AND_BACK), osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
            pMatrixTransform->addChild(pGeode);

            osg::Matrixd m;
            m.setTrans(ptCenter);
            pMatrixTransform->setMatrix(m);
            pGroup->addChild(pMatrixTransform);
        }
    }

    return pGroup.release();
}


osg::Node *LineParameterNode::createCubeDetail(const param::IDetail *pDetail, const std::vector<osg::ref_ptr<osg::Vec3dArray> > &vecPoints) const
{
    const param::ICubeDetail *pCubeDetail = dynamic_cast<const param::ICubeDetail *>(pDetail);
    if(pCubeDetail == NULL)
    {
        return NULL;
    }

    double dblCubeWidth, dblCubeHeight, dblCubeLength;
    pCubeDetail->getCubeSize(dblCubeLength, dblCubeWidth, dblCubeHeight);

    const cmm::FloatColor &clr = pCubeDetail->getDynModelColor();
    const bool bTopVisible = pCubeDetail->getTopVisible();
    const bool bBottomVisible = pCubeDetail->getBottomVisible();

    osg::ref_ptr<osg::Group> pGroup = new osg::Group;
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    const osg::Vec4 vColor(clr.m_fltR, clr.m_fltG, clr.m_fltB, clr.m_fltA);

    const ID &imgID = pCubeDetail->getImageID();

    osg::ref_ptr<osg::Texture> pTexture;
    if(imgID.isValid() && imgID.ModelID.m_nType == IMAGE_ID)
    {
        pTexture = bindTexture(imgID, NULL);
    }

    std::vector<osg::ref_ptr<osg::Vec3dArray> >::const_iterator itor = vecPoints.begin();
    for( ; itor != vecPoints.end(); ++itor)
    {
        const osg::Vec3dArray   *pVec3dArray = itor->get();
        for(unsigned int i = 0; i < pVec3dArray->size() - 1u; i++)
        {
            osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform;
            osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
            const osg::Vec3d &pt1 = pVec3dArray->at(i);
            const osg::Vec3d &pt2 = pVec3dArray->at(i + 1u);

            osg::Vec3d ptTemp1, ptTemp2;
            pEllipsoidModel->convertLatLongHeightToXYZ(pt1.y(), pt1.x(), pt1.z() + m_dblHeight, ptTemp1.x(), ptTemp1.y(), ptTemp1.z());
            pEllipsoidModel->convertLatLongHeightToXYZ(pt2.y(), pt2.x(), pt2.z() + m_dblHeight, ptTemp2.x(), ptTemp2.y(), ptTemp2.z());

            // 本截管道的中心点
            const osg::Vec3d ptCenter = (ptTemp1 + ptTemp2) * 0.5;

            const double dblLength = (ptTemp2 - ptTemp1).length();

            // 把管线旋转到正确的方向
#if 0
            osg::Quat   qtRotation;

            osg::Vec3d vecPositionUp = ptCenter;
            vecPositionUp.normalize();

            osg::Vec3d vecLineDir = ptTemp2 - ptTemp1;
            vecLineDir.normalize();

            // 将管道放倒
            osg::Quat   qt0;
            const osg::Vec3d vecAxisZ(0.0, 0.0, 1.0);
            qt0.makeRotate(vecAxisZ, vecLineDir);
            qtRotation *= qt0; 

            osg::Quat   qt1;
            // 将管道沿管道自身的轴向打滚，将它放平
            // 严重警告：这里若管道是正好垂直于当地海平面的，那么将无法打滚！！！！
            // 理由如下：
            //      虽然根据整段管道搜索，也许能够搜索到一段不垂直的，继而进行计算，但是这仍然不能解决所有问题，因为若整条管道都是垂直于地表的，那么将搜索失败
            //      简单来说，若管道是垂直于地表的，那么将无所谓它的朝向，缺少必要参量导致无法计算管道的朝向
            // 在此，我们只能忽略这个问题
            osg::Vec3d vecAxisY(0.0, -1.0, 0.0);
            vecAxisY = qt0 * vecAxisY;

            const double dblCos = vecLineDir * vecPositionUp;
            const double dblAngle = acos(dblCos);
            const osg::Quat   qt3(osg::PI_2 - dblAngle, vecLineDir ^ vecPositionUp);

            const osg::Vec3d vecPipeUp = qt3 * vecPositionUp;
            qt1.makeRotate(vecAxisY, vecPipeUp);
            qtRotation *= qt1;      // 顺着自己的轴向打滚，将它放平
#else
            const osg::Quat qtRotation = findCubePipeRotation(pVec3dArray, i);
#endif

            osg::Box *pBox = new osg::Box(osg::Vec3d(0.0, 0.0, 0.0), dblCubeWidth, dblCubeHeight, dblLength);
            pBox->setRotation(qtRotation);

            osg::ref_ptr<osg::TessellationHints> pHints = new osg::TessellationHints;
            pHints->setCreateTop(bTopVisible);
            pHints->setCreateBottom(bTopVisible);
            pHints->setDetailRatio(0.5);
            pHints->setTextureRatio(dblLength);

            osg::ShapeDrawable *pShapeDrawable = new osg::ShapeDrawable(pBox, pHints);
            pShapeDrawable->getOrCreateStateSet()->setTextureAttributeAndModes(0, pTexture.get(), osg::StateAttribute::ON);
            pShapeDrawable->setColor(vColor);

            pGeode->addDrawable(pShapeDrawable);
            pGeode->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT_AND_BACK), osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
            pMatrixTransform->addChild(pGeode);

            osg::Matrixd m;
            m.setTrans(ptCenter);
            pMatrixTransform->setMatrix(m);
            pGroup->addChild(pMatrixTransform);
        }
    }

    return pGroup.release();
}


osg::Quat LineParameterNode::findCubePipeRotation(const osg::Vec3dArray *pPipeVertices, unsigned nPos) const
{
    osg::Quat   qtRotation;

    const unsigned nVerticesCount = pPipeVertices->size();
    if(!pPipeVertices || nVerticesCount < 1u || nPos >= nVerticesCount - 1u)
    {
        return qtRotation;
    }

    const osg::Vec3d &ptBeginCoord = pPipeVertices->at(nPos);
    const osg::Vec3d &ptEndCoord   = pPipeVertices->at(nPos + 1u);

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    osg::Vec3d ptBegin, ptEnd;
    pEllipsoidModel->convertLatLongHeightToXYZ(ptBeginCoord.y(), ptBeginCoord.x(), ptBeginCoord.z() + m_dblHeight, ptBegin.x(), ptBegin.y(), ptBegin.z());
    pEllipsoidModel->convertLatLongHeightToXYZ(ptEndCoord.y(), ptEndCoord.x(), ptEndCoord.z() + m_dblHeight, ptEnd.x(), ptEnd.y(), ptEnd.z());

    const osg::Vec3d ptCenter = (ptBegin + ptEnd) * 0.5;
    osg::Vec3d vecCenterUp = ptCenter;
    vecCenterUp.normalize();

    osg::Vec3d vecLineDir = ptEnd - ptBegin;
    vecLineDir.normalize();


    // 将管道放倒
    osg::Quat   qt0;
    const osg::Vec3d vecAxisZ(0.0, 0.0, 1.0);
    qt0.makeRotate(vecAxisZ, vecLineDir);
    qtRotation *= qt0;

    osg::Vec3d vecAxisY(0.0, -1.0, 0.0);
    vecAxisY = qt0 * vecAxisY;

    const double dblCos = vecLineDir * vecCenterUp;
    if(fabs(dblCos) < 0.999)
    {
        // 非常好，本段管道和当地海平面不垂直，那么直接可以计算管道的打滚量
        const double dblCos = vecLineDir * vecCenterUp;
        const double dblAngle = acos(dblCos);
        const osg::Quat   qt2(osg::PI_2 - dblAngle, vecLineDir ^ vecCenterUp);

        const osg::Vec3d vecPipeUp = qt2 * vecCenterUp;

        osg::Quat   qt1;
        qt1.makeRotate(vecAxisY, vecPipeUp);
        qtRotation *= qt1;      // 顺着自己的轴向打滚，将它放平
        return qtRotation;
    }

    // 接下来搜索整段管道，找到和海平面不垂直的那一截
    bool        bFound = false;
    osg::Vec3d  vecFindLineDir;
    unsigned nFindBack = nPos;
    unsigned nFindForward = nPos + 1u;
    while(nFindBack >= 1u || nFindForward < nVerticesCount - 1u)
    {
        osg::Vec3d  ptCoord0, ptCoord1;

        // 后向搜索
        if(nFindBack >= 1u)
        {
            ptCoord0 = pPipeVertices->at(nFindBack - 1u);
            ptCoord1 = pPipeVertices->at(nFindBack);
            nFindBack--;
        }

        // 前向搜索
        else if(nFindForward < nVerticesCount - 1u)
        {
            ptCoord0 = pPipeVertices->at(nFindForward);
            ptCoord1 = pPipeVertices->at(nFindForward + 1u);
            nFindForward++;
        }

        osg::Vec3d ptTemp0, ptTemp1;
        pEllipsoidModel->convertLatLongHeightToXYZ(ptCoord0.y(), ptCoord0.x(), ptCoord0.z() + m_dblHeight, ptTemp0.x(), ptTemp0.y(), ptTemp0.z());
        pEllipsoidModel->convertLatLongHeightToXYZ(ptCoord1.y(), ptCoord1.x(), ptCoord1.z() + m_dblHeight, ptTemp1.x(), ptTemp1.y(), ptTemp1.z());

        osg::Vec3d vecDir = ptTemp1 - ptTemp0;
        vecDir.normalize();

        osg::Vec3d vecFindCenterUp = (ptTemp1 + ptTemp0) * 0.5;
        vecFindCenterUp.normalize();

        const double dbl = vecDir * vecFindCenterUp;
        if(fabs(dbl) < 0.999)
        {
            vecFindLineDir = vecDir;
            bFound = true;
            break;
        }
    }

    if(!bFound)
    {
        // 很遗憾，整条管道里头的每一截都和海平面垂直
        return qtRotation;
    }

    osg::Vec3d vecRightDir = vecLineDir ^ vecFindLineDir;
    vecRightDir.normalize();
    osg::Vec3d vecPipeUp = vecRightDir ^ vecLineDir;
    vecPipeUp.normalize();

    osg::Quat   qt1;
    qt1.makeRotate(vecAxisY, vecPipeUp);
    qtRotation *= qt1;      // 顺着自己的轴向打滚，将它放平

    return qtRotation;
}


void LineParameterNode::traverse(osg::NodeVisitor& nv)
{
    if(nv.getVisitorType() != osg::NodeVisitor::CULL_VISITOR)
    {
        return Group::traverse(nv);
    }

    if(m_bFollowTerrain && m_bHasIntered)
    {
        osg::Node *pChild = getChild(0);
        if(pChild != NULL && m_pRectifiedNode.valid())
        {
            replaceChild(getChild(0), m_pRectifiedNode);
        }

        m_bHasIntered = false;
        m_pRectifiedNode = NULL;
        m_pParmRectifyRequest = NULL;

        return Group::traverse(nv);
    }

    if(m_bFollowTerrain)
    {
        FileReadInterceptor *pFileReadInterceptor = NULL;
        const unsigned nCallbackCount = osgDB::Registry::instance()->getReadFileCallbackCount();
        for(unsigned n = 0u; n < nCallbackCount; n++)
        {
            pFileReadInterceptor = dynamic_cast<FileReadInterceptor *>(osgDB::Registry::instance()->getReadFileCallback(n));
            if(pFileReadInterceptor)
            {
                break;
            }
        }
        if(pFileReadInterceptor == NULL)
        {
            return Group::traverse(nv);
        }

        //if(m_nLastSendTime != pFileReadInterceptor->getLastTerrainUpdate())
        //{
        //    if(pFileReadInterceptor->getLastTerrainUpdate() + 1 < osg::Timer::instance()->time_s())
        //    {
        //        ParmRectifyThreadPool *pThreadPool = Registry::instance()->getParmRectifyThreadPool();
        //        pThreadPool->requestParmRectify(this, m_pParmRectifyRequest);

        //        m_nLastSendTime = pFileReadInterceptor->getLastTerrainUpdate();
        //    }
        //}

        const unsigned nCurTime = clock();
        //const unsigned durationTick = nCurTime - pFileReadInterceptor->getLastTerrainUpdate();
        const unsigned nLastDuration = nCurTime - m_nLastSendTime;

        if(m_nLastSendTime !=  pFileReadInterceptor->getLastTerrainUpdate() && nLastDuration > 2000)
        {
            ParmRectifyThreadPool *pThreadPool = Registry::instance()->getParmRectifyThreadPool();
            pThreadPool->requestParmRectify(this, m_pParmRectifyRequest);
            m_nLastSendTime = pFileReadInterceptor->getLastTerrainUpdate();
            printf("requestParmRectify\n");
        }
    }

    return Group::traverse(nv);
}