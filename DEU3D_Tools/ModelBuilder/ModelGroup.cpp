#include "ModelGroup.h"
#include "GeodeVisitor.h"
#include <osg/ComputeBoundsVisitor>
#include <osg/CoordinateSystemNode>
#include <osg/Texture2D>
#include <osgUtil/DistancePixelComputer.h>
#include <osgDB/ReadFile>
#include <osgDB/ReaderWriter>
#include <ParameterSys/Detail.h>
#include <ParameterSys/PointParameter.h>
#include <Common/CoordinateTransform.h>
#include <strstream>
#include "SimplifyTexture.h"
#include <osgDB/WriteFile>
#include "GeodeUnitGenerator.h"
#include <DDSConverter.h>
#include "DYFaceList.h"
#include "DYModel.h"
#include "DYModelList.h"
#include "DYIntList.h"
#include "DYVert.h"
#include "DYFace.h"
#include "DYVertMap.h"

ModelGroup::ModelGroup(GeodeVisitor* pGeodeVisitor, deudbProxy::IDEUDBProxy *pDB, bool bMultiRefCenter)
: m_pMatrixTransform(NULL)
, m_pGeodeVisitor(pGeodeVisitor)
, m_pTargetDB(pDB)
, m_bMultiRefCenter(bMultiRefCenter)
,m_resolutionH(1280)
,m_resolutionV(768)
,m_nCurrentLODIndex(0)
{
}


ModelGroup::~ModelGroup(void)
{
    m_vecDetail.clear();
    m_pMatrixTransform = NULL;
}


void ModelGroup::setModel(osg::ref_ptr<osg::MatrixTransform> pMatrixTransform)
{
    m_pMatrixTransform = pMatrixTransform;
}

bool ModelGroup::saveModel()
{
    if (m_pMatrixTransform == NULL || m_pGeodeVisitor == NULL)
    {
        return false;
    }

    m_vecDetail.clear();

    saveGeodes();

    return true;
}


bool ModelGroup::saveGeodes()
{
    //计算等价包围球
    osg::ref_ptr<osg::Geode>  pGeode = m_pMatrixTransform->getChild(0)->asGeode();
    const osg::BoundingSphere& bound = pGeode->getBound();
	//const osg::BoundingSphere& bound = m_pMatrixTransform->getBound();
    if (bound.radius() < FLT_EPSILON)
    {
        return false;
    }

    double dRadius = getBoundRadius(pGeode);
	//double dRadius = getBoundRadius();
    if (dRadius < FLT_EPSILON)
    {
        return false;
    }

    //计算最大显示距离
    osgUtil::DistancePixelComputer dpc;
    dpc.setEnviroment(45.0, m_resolutionH, m_resolutionV);
    double dMaxDis = dpc.calcDistanceByPixelSize(dRadius, 4);			//- 正常生产数据用dRadius-小
	//double dMaxDis = dpc.calcDistanceByPixelSize(bound.radius(), 4);	//- dwg杆塔数据用bound.radius()-大

    //计算LOD分级，暂时不考虑缩放
    LODSEGMENT LODSeg;
    m_pGeodeVisitor->getLODSegment(LODSeg);

    std::vector<cmm::math::Point3d>     vecBubbleCenters;
    if(m_bMultiRefCenter)
    {
        generateReferencedCenter(pGeode, LODSeg.dLOD1, vecBubbleCenters);
    }

    if (LODSeg.bOptimize)
    {
        switch (LODSeg.nValidLodLevels)
        {
		case 1:
			saveGeodes_1level_optimize(pGeode, dMaxDis, LODSeg);
			break;
		case 2:
			saveGeodes_2level_optimize(pGeode, dMaxDis, LODSeg);
			break;
        case 3:
            saveGeodes_3level_optimize(pGeode, dMaxDis, LODSeg);
            break;
        case 4:
            saveGeodes_4level_optimize(pGeode, dMaxDis, LODSeg);
            break;
//         case 5:
//             saveGeodes_5level_optimize(pGeode, dMaxDis, LODSeg);
//             break;
        default:
            break;
        }
    }
    else
    {
        saveGeodes_5level_source(pGeode, dMaxDis, LODSeg);
    }

    savePointParameter(vecBubbleCenters);
    return true;
}


void ModelGroup::generateReferencedCenter(const osg::Geode *pGeode, double dblMinRange, std::vector<cmm::math::Point3d> &vecBubbleCenters) const
{
    const osg::CopyOp copyOp = osg::CopyOp::DEEP_COPY_OBJECTS    |
        osg::CopyOp::DEEP_COPY_NODES      |
        osg::CopyOp::DEEP_COPY_DRAWABLES  |
        osg::CopyOp::DEEP_COPY_ARRAYS     |
        osg::CopyOp::DEEP_COPY_PRIMITIVES |
        osg::CopyOp::DEEP_COPY_SHAPES;
    osg::ref_ptr<osg::Geode> pNewGeode = dynamic_cast<osg::Geode*>(pGeode->clone(copyOp));
    if (pNewGeode == NULL)
    {
        return;
    }

    const bool bShareModel = m_pGeodeVisitor->getShareModel();
    osg::Vec3d centerPoint(0.0, 0.0, 0.0);
    osg::Vec3d centerVec3d(0.0, 0.0, 0.0);
    if (!bShareModel)
    {
        osg::Vec3d src, dst;
        for (unsigned i = 0; i < pNewGeode->getNumDrawables(); i++)
        {
            osg::Geometry* pGeometry = pNewGeode->getDrawable(i)->asGeometry();
            osg::Vec3Array* pVecters = dynamic_cast<osg::Vec3Array*>(pGeometry->getVertexArray());
            if (pVecters == NULL)
            {
                continue;
            }

            for(size_t n = 0; n < pVecters->getNumElements(); n++)
            {
                (*pVecters)[n] = m_pMatrixTransform->getMatrix().preMult((*pVecters)[n]);
            }

            osg::ref_ptr<osg::Vec3dArray> pVectersDouble = new osg::Vec3dArray(pVecters->begin(), pVecters->end());
            for (unsigned j = 0; j < pVectersDouble->getNumElements(); j++)
            {
                osg::Vec3d &src = pVectersDouble->at(j);
                coordinateTransform(src, src);
            }

            pGeometry->setVertexArray(pVectersDouble);
        }
    }

    GeodeUnitGenerator    unitGenerator;
    unitGenerator.setBubbleRadius(dblMinRange * 0.5);
    const osg::Vec3dArray *pBubbleCenters = unitGenerator.findBubbleCenters(*pNewGeode);
    if(!pBubbleCenters) return;

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    vecBubbleCenters.resize(pBubbleCenters->size());
    for(unsigned n = 0u; n < pBubbleCenters->size(); n++)
    {
        const osg::Vec3d &vtx = pBubbleCenters->at(n);
        cmm::math::Point3d &point = vecBubbleCenters.at(n);

        if(bShareModel)
        {
            const osg::Vec3d src = vtx * m_pMatrixTransform->getMatrix();
            osg::Vec3d dst;
            coordinateTransform(src, dst);
            pEllipsoidModel->convertXYZToLatLongHeight(dst.x(), dst.y(), dst.z(), point.y(), point.x(), point.z());
        }
        else
        {
            pEllipsoidModel->convertXYZToLatLongHeight(vtx.x(), vtx.y(), vtx.z(), point.y(), point.x(), point.z());
        }
    }
}


bool ModelGroup::saveGeode(int nLODIndex, ID& geodeID, osg::Geode* pGeode, ID& idDetail)
{
    if (m_pGeodeVisitor->getModelBuilder()->getStopFlag())
    {
        return false;
    }

    SHAREGEODE shareGeode;
    bool bShareImage = isShareImage(pGeode);

    LODSEGMENT LODSeg;
    m_pGeodeVisitor->getLODSegment(LODSeg);
    osg::ref_ptr<osg::StateSet> pState = pGeode->getOrCreateStateSet();
    if ((LODSeg.nValidLodLevels==4 || LODSeg.nValidLodLevels==5) && nLODIndex==LODSeg.nValidLodLevels)
    {
        osg::ref_ptr<osg::Material> pMaterial = dynamic_cast<osg::Material *>(pState->getAttribute(osg::StateAttribute::MATERIAL));
        if (pMaterial == NULL)
        {
            pMaterial = new osg::Material;
        }

        static const osg::Vec4 clrDiffuse(0.75f, 0.75f, 0.75f, 1.0f);
        static const osg::Vec4 clrAmbient(0.6f, 0.6f, 0.6f, 1.0f);
        static const osg::Vec4 clrSpecular(0.6f, 0.6f, 0.6f, 1.0f);
        pMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, clrDiffuse);
        pMaterial->setAmbient(osg::Material::FRONT_AND_BACK, clrAmbient);
        pMaterial->setSpecular(osg::Material::FRONT_AND_BACK, clrSpecular);
        pMaterial->setTransparency(osg::Material::FRONT_AND_BACK, 0.5);
        pState->setMode(GL_BLEND, osg::StateAttribute::ON);
        pState->setAttributeAndModes(pMaterial, osg::StateAttribute::OVERRIDE|osg::StateAttribute::PROTECTED|osg::StateAttribute::ON);
        pState->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    }

    unsigned int numDrawable = pGeode->getNumDrawables();
    for (unsigned int i = 0; i < numDrawable; i++)
    {
        if (m_pGeodeVisitor->getModelBuilder()->getStopFlag())  return false;

        osg::ref_ptr<osg::Drawable> pDrawable = pGeode->getDrawable(i);
        osg::Geometry *pGeometry = pDrawable->asGeometry();
        if(!pGeometry)  continue;

        osg::ref_ptr<osg::StateSet> pStateSet = pDrawable->getStateSet();
        if (pStateSet == NULL)
        {
            continue;
        }

        if ((LODSeg.nValidLodLevels==4 || LODSeg.nValidLodLevels==5) && nLODIndex==LODSeg.nValidLodLevels)
        {
            osg::Vec4Array *pColorArray = new osg::Vec4Array;
            pColorArray->push_back(osg::Vec4(0.75f, 0.75f, 0.75f, 0.5f));
            pGeometry->setColorArray(pColorArray);
            pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

            pStateSet->getTextureAttributeList().clear();
            pDrawable->asGeometry()->getTexCoordArrayList().clear();
        }

        unsigned int nTexCount = pStateSet->getNumTextureAttributeLists();
        for(unsigned int j = 0; j < nTexCount; j++)
        {
            if (m_pGeodeVisitor->getModelBuilder()->getStopFlag())  return false;

            osg::ref_ptr<osg::StateAttribute> pAttribute = pStateSet->getTextureAttribute(j, osg::StateAttribute::TEXTURE);
            osg::Texture2D* pTexture2D = dynamic_cast<osg::Texture2D*>(pAttribute.get());
            if(pTexture2D == NULL)
            {
                continue;
            }

            osg::ref_ptr<osg::Image> pImage = pTexture2D->getImage();
            if (pImage == NULL)
            {
                continue;
            }

            osg::ref_ptr<osg::Image> pNewImage = NULL;

            ID imgID;
            if (bShareImage && m_pGeodeVisitor->getShareImg(nLODIndex, pImage->getFileName(), imgID))
            {
                pNewImage = new osg::Image();
                pNewImage->setFileName(imgID.toString());
            }
            else
            {
                pNewImage = saveImage(nLODIndex, bShareImage, pImage);
            }

            pTexture2D->setImage(pNewImage);
            pTexture2D->setInternalFormatMode(osg::Texture::USE_S3TC_DXT5_COMPRESSION);

            const GLuint nFormat = pNewImage->getInternalTextureFormat();
            if(nFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT || GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
            {
                pTexture2D->setInternalFormatMode(osg::Texture::USE_S3TC_DXT1_COMPRESSION);
            }
        }
    }

    //保存几何
    osg::ref_ptr<osgDB::ReaderWriter::Options> pOptionsPtr = new osgDB::ReaderWriter::Options;
    if (bShareImage)
    {
        pOptionsPtr->setOptionString("noTexturesInIVEFile");
    }

    if (m_pGeodeVisitor->getShareModel())
    {
        if (m_pGeodeVisitor->getModelBuilder()->getStopFlag())  return false;

        std::ostrstream ss;
        osgDB::ReaderWriter* pRW = osgDB::Registry::instance()->getReaderWriterForExtension("ive");
        pRW->writeNode(*pGeode, ss, pOptionsPtr);
        m_pTargetDB->addBlock(geodeID, ss.str(), ss.pcount());
        ss.freeze(false);

        saveDetail(geodeID, idDetail);

        shareGeode.idGeode  = geodeID;
        shareGeode.idDetail = idDetail;
        m_pGeodeVisitor->addShareGeode(nLODIndex, shareGeode);
    }
    else
    {
        //将中心点拉到原点后，做逐点上球
        osg::Vec3d src;
        osg::Vec3d dst;
        osg::Vec3d worldCenter;

        coordinateTransform(osg::Vec3d(0.0, 0.0, 0.0), worldCenter);

        if (nLODIndex == 1)
        {
            m_ModelRadius = pGeode->getBound().radius();
        }

        for (unsigned i = 0; i < pGeode->getNumDrawables(); i++)
        {
            if (m_pGeodeVisitor->getModelBuilder()->getStopFlag())  return false;

            osg::Geometry* pGeometry = pGeode->getDrawable(i)->asGeometry();
            osg::Vec3Array* pVecters = dynamic_cast<osg::Vec3Array*>(pGeometry->getVertexArray());

            if (pVecters == NULL)
            {
                continue;
            }

            osg::ref_ptr<osg::Vec3dArray> pVectersDouble = new osg::Vec3dArray(pVecters->begin(), pVecters->end());

            for (unsigned j = 0; j < pVectersDouble->getNumElements(); j++)
            {
                src = (*pVectersDouble)[j];
                coordinateTransform(src, dst);
                (*pVectersDouble)[j] = dst;
            }

            pGeometry->setVertexArray(pVectersDouble);

            pGeometry->dirtyBound();
        }

        //中心点上球
        const osg::BoundingSphere& bound = pGeode->getBound();
        osg::Vec3d center = bound.center();

        for (unsigned i = 0; i < pGeode->getNumDrawables(); i++)
        {
            if (m_pGeodeVisitor->getModelBuilder()->getStopFlag())  return false;

            osg::Geometry* pGeometry = pGeode->getDrawable(i)->asGeometry();
            osg::Vec3dArray* pVectersDouble = dynamic_cast<osg::Vec3dArray*>(pGeometry->getVertexArray());

            if (pVectersDouble == NULL)
            {
                continue;
            }

            if (nLODIndex == 1)
            {
                for (unsigned j = 0; j < pVectersDouble->getNumElements(); j++)
                {
                    (*pVectersDouble)[j] -= center;
                }
            }
            else
            {
                for (unsigned j = 0; j < pVectersDouble->getNumElements(); j++)
                {
                    (*pVectersDouble)[j] -= m_ModelCenter;
                }
            }

            osg::ref_ptr<osg::Vec3Array> pVertexs = new osg::Vec3Array(pVectersDouble->begin(), pVectersDouble->end());

            pGeometry->setVertexArray(pVertexs);

            osg::Vec3Array *pNormals  = dynamic_cast<osg::Vec3Array*>(pGeometry->getNormalArray());
            for (unsigned k = 0; k < pNormals->getNumElements(); k++)
            {
                coordinateTransform((*pNormals)[k], dst);

                (*pNormals)[k] = dst - worldCenter;
                (*pNormals)[k].normalize();
            }

            pGeometry->dirtyBound();
        }

        if (nLODIndex == 1)
        {
            m_ModelCenter = center;
            osg::EllipsoidModel::instance()->convertXYZToLatLongHeight(center.x(), center.y(), center.z(),
                m_centerPoint.y(), m_centerPoint.x(), m_centerPoint.z());
        }

        std::ostrstream ss;
        osgDB::ReaderWriter* pRW = osgDB::Registry::instance()->getReaderWriterForExtension("ive");
        pRW->writeNode(*pGeode, ss, pOptionsPtr);
        m_pTargetDB->addBlock(geodeID, ss.str(), ss.pcount());
        ss.freeze(false);

        saveDetail(geodeID, idDetail);
    }

    return true;
}


bool ModelGroup::saveDetail(const ID& id, ID& idDetail)
{
    if (m_pTargetDB == NULL || m_pGeodeVisitor->getModelBuilder()->getStopFlag())
    {
        return false;
    }

    OpenSP::sp<param::IStaticModelDetail> pDetail = dynamic_cast<param::IStaticModelDetail *>(param::createDetail(param::STATIC_DETAIL, m_pGeodeVisitor->getDataSetCode()));
    pDetail->setModelID(id);
    if (m_pGeodeVisitor->getShareModel())
    {
        pDetail->setAsOnGlobe(false);
    }
    else
    {
        pDetail->setAsOnGlobe(true);
    }

    bson::bsonDocument doc;
    bson::bsonStream   bstream;

    if (!pDetail->toBson(doc))
    {
        return false;
    }

    if (!doc.Write(&bstream))
    {
        return false;
    }

    idDetail = pDetail->getID();
    m_pTargetDB->addBlock(idDetail, bstream.Data(), bstream.DataLen());

    return true;
}


bool ModelGroup::savePointParameter(const std::vector<cmm::math::Point3d> &vecBubbleCenters)
{
    ID modelID = ID::genNewID();
    modelID.ObjectID.m_nDataSetCode = m_pGeodeVisitor->getDataSetCode();
    modelID.ObjectID.m_nType        = PARAM_POINT_ID;

    OpenSP::sp<param::IPointParameter> pParameter = dynamic_cast<param::PointParameter*>(param::createParameter(modelID));
    for (size_t i = 0; i < m_vecDetail.size(); i++)
    {
        if (m_pGeodeVisitor->getModelBuilder()->getStopFlag())  return false;

        pParameter->addDetail(m_vecDetail[i].idDetail, m_vecDetail[i].dMinRange, m_vecDetail[i].dMaxRange);
    }

    osg::Vec3d centerVec3d(0.0, 0.0, 0.0);
    cmm::math::Point3d centerPoint(0.0, 0.0, 0.0);
    cmm::math::Point3d scale(1.0, 1.0, 1.0);

    double dblPitchAngle	= 0.0;
	double dblRollAngle		= 0.0;
    double dblAzimuthAngle	= 0.0;

    if (m_pGeodeVisitor->getModelBuilder()->getStopFlag())  return false;

    if (m_pGeodeVisitor->getShareModel())
    {
        //中心点上球
        osg::Vec3d center = m_pMatrixTransform->getBound().center();
        coordinateTransform(center, centerVec3d);

        osg::EllipsoidModel::instance()->convertXYZToLatLongHeight(centerVec3d.x(), centerVec3d.y(), centerVec3d.z(),
            centerPoint.y(), centerPoint.x(), centerPoint.z());

		osg::Quat qtRotate, qtTemp;
        osg::Vec3 vScale, vTrans;
        m_pMatrixTransform->getMatrix().decompose(vTrans, qtRotate, vScale, qtTemp);

		//旋转
		qtRotate.getEnlerAngle(dblPitchAngle, dblRollAngle, dblAzimuthAngle);

        //缩放
        scale.x() = vScale.x();
        scale.y() = vScale.y();
        scale.z() = vScale.z();
    }
    else
    {
        centerPoint = m_centerPoint;
    }

    pParameter->setCoordinate(centerPoint);
	pParameter->setPitchAngle(dblPitchAngle);
	pParameter->setRollAngle(dblRollAngle);
    pParameter->setAzimuthAngle(dblAzimuthAngle);
    pParameter->setScale(scale);

    if(!vecBubbleCenters.empty())
    {
        for(unsigned n = 0u; n < vecBubbleCenters.size(); n++)
        {
            if (m_pGeodeVisitor->getModelBuilder()->getStopFlag())  return false;

            const cmm::math::Point3d &ref = vecBubbleCenters.at(n);
            pParameter->addRefPoint(ref);
        }
    }
    pParameter->addProperty("Name", m_pMatrixTransform->getChild(0)->asGeode()->getName());

    param::PointParameter *p = dynamic_cast<param::PointParameter*>(pParameter.get());
    p->setRadius(m_pMatrixTransform->getBound().radius());

    bson::bsonDocument bsonDoc;
    bson::bsonStream   bstream;
    param::Parameter *p1 = dynamic_cast<param::Parameter*>(pParameter.get());
    p1->toBson(bsonDoc);
    bsonDoc.Write(&bstream);

    m_pTargetDB->addBlock(modelID, bstream.Data(), bstream.DataLen());

    centerVec3d.x() = centerPoint.x();
    centerVec3d.y() = centerPoint.y();
    centerVec3d.z() = centerPoint.z();

    osg::BoundingSphere sphere;
    sphere._center = centerVec3d;
    if (m_pGeodeVisitor->getShareModel())
    {
        sphere._radius = m_pMatrixTransform->getChild(0)->asGeode()->getBound().radius();
    }
    else
    {
        sphere._radius = m_ModelRadius;
    }

    m_pGeodeVisitor->addModel(modelID, sphere);
    return true;
}


osg::ref_ptr<osg::Image> ModelGroup::saveImage(int nLODIndex, bool bShare, osg::ref_ptr<osg::Image>& pSrcImage)
{
    int nScaleS = getImageScaleS(nLODIndex, pSrcImage);
    int nScaleT = getImageScaleT(nLODIndex, pSrcImage);

    osg::ref_ptr<osg::Image> pNewImage = dynamic_cast<osg::Image*>(pSrcImage->clone(osg::CopyOp::DEEP_COPY_ALL));

    //如果纹理不是2的幂，则把数据归为2的幂
    const unsigned int nSizeS = m_pGeodeVisitor->getModelBuilder()->findLowerNumber(pSrcImage->s()/nScaleS);
    const unsigned int nSizeT = m_pGeodeVisitor->getModelBuilder()->findLowerNumber(pSrcImage->t()/nScaleT);

    pNewImage->scaleImage(nSizeS, nSizeT, pSrcImage->r());
    const GLenum ePixelFormat = pNewImage->getPixelFormat();
    unsigned char *pDDSData = NULL;
    unsigned nDataLen = 0u;
    switch(ePixelFormat)
    {
    case GL_RGB:
        RGB_2_DXT1(pNewImage->data(), pNewImage->s(), pNewImage->t(), PO_RGB, &pDDSData, &nDataLen);
        break;
    case GL_BGR:
        RGB_2_DXT1(pNewImage->data(), pNewImage->s(), pNewImage->t(), PO_BGR, &pDDSData, &nDataLen);
        break;
    case GL_RGBA:
        {
            const GLenum eBest = pNewImage->findBestBitsOf16();
            if(eBest == GL_UNSIGNED_SHORT_5_5_5_1)
            {
                RGBA_2_DXT1(pNewImage->data(), pNewImage->s(), pNewImage->t(), PO_RGBA, &pDDSData, &nDataLen);
            }
            else if(eBest == GL_UNSIGNED_SHORT_4_4_4_4)
            {
                RGBA_2_DXT5(pNewImage->data(), pNewImage->s(), pNewImage->t(), PO_RGBA, &pDDSData, &nDataLen);
            }
            break;
        }
    case GL_BGRA:
        {
            const GLenum eBest = pNewImage->findBestBitsOf16();
            if(eBest == GL_UNSIGNED_SHORT_5_5_5_1)
            {
                RGBA_2_DXT1(pNewImage->data(), pNewImage->s(), pNewImage->t(), PO_BGRA, &pDDSData, &nDataLen);
            }
            else if(eBest == GL_UNSIGNED_SHORT_4_4_4_4)
            {
                RGBA_2_DXT5(pNewImage->data(), pNewImage->s(), pNewImage->t(), PO_BGRA, &pDDSData, &nDataLen);
            }
            break;
        }
    }
    if(!pDDSData || nDataLen < 1u)
    {
        return NULL;
    }

    {
        std::istrstream iss((char *)pDDSData, nDataLen);
        osgDB::ReaderWriter::Options *pOptions = osgDB::Registry::instance()->getOptions();
        osgDB::ReaderWriter* pDDSRW = osgDB::Registry::instance()->getReaderWriterForExtension("dds");
        osgDB::ReaderWriter::ReadResult rr = pDDSRW->readImage(iss, pOptions);
        if(!rr.success())
        {
            freeDXTMemory(pDDSData);
            return NULL;
        }

        pNewImage = rr.getImage();
    }

    //保存共享纹理
    if (bShare)
    {
        ID imgID = ID::genNewID();
        imgID.ObjectID.m_nDataSetCode = m_pGeodeVisitor->getDataSetCode();

        if (m_pGeodeVisitor->isShareImg(pSrcImage->getFileName()))
        {
            imgID.ObjectID.m_nType = SHARE_IMAGE_ID;
        }
        else
        {
            imgID.ObjectID.m_nType = IMAGE_ID;
        }

        m_pTargetDB->addBlock(imgID, pDDSData, nDataLen);

        pNewImage->setFileName(imgID.toString());
        m_pGeodeVisitor->addShareImg(pSrcImage->getFileName(), nLODIndex, imgID);
    }
    else
    {
        //纹理压缩
        std::string oldFileName = pNewImage->getFileName();
        std::string newFileName = oldFileName;
        unsigned nIndex = oldFileName.find_last_of(".");
        if (nIndex != -1)
        {
            newFileName = oldFileName.substr(0, nIndex) + ".dds";
        }

        pNewImage->setFileName(newFileName);
    }
    freeDXTMemory(pDDSData);
    return pNewImage.release();
}


ID ModelGroup::getGeodID()
{
    ID geodeID = ID::genNewID();
    geodeID.ObjectID.m_nDataSetCode = m_pGeodeVisitor->getDataSetCode();

    if (m_pGeodeVisitor->isShareGeode())
    {
        geodeID.ObjectID.m_nType = SHARE_MODEL_ID;
    }
    else
    {
        geodeID.ObjectID.m_nType = MODEL_ID;
    }

    return geodeID;
}


int ModelGroup::getImageScaleS(int nLODIndex, osg::ref_ptr<osg::Image>& pSrcImage)
{
    if (nLODIndex == 1)
    {
        return 1;
    }

    int nScale = 1;
    switch (nLODIndex)
    {
    case 1:
        nScale = 1;
        break;
    case 2:
        nScale = 16;
        break;
    case 3:
        nScale = 128;
        break;
    case 4:
        nScale = 512;
        break;
    case 5:
        nScale = 1024;
        break;
    default:
        break;
    }

    if (pSrcImage->s()/nScale < 2)
    {
        return pSrcImage->s() / 2;
    }

    return nScale;
}


int ModelGroup::getImageScaleT(int nLODIndex, osg::ref_ptr<osg::Image>& pSrcImage)
{
    if (nLODIndex == 1)
    {
        return 1;
    }

    int nScale = 1;
    switch (nLODIndex)
    {
    case 1:
        nScale = 1;
        break;
    case 2:
        nScale = 16;
        break;
    case 3:
        nScale = 128;
        break;
    case 4:
        nScale = 512;
        break;
    case 5:
        nScale = 1024;
        break;
    default:
        break;
    }

    if (pSrcImage->t()/nScale < 2)
    {
        return pSrcImage->t() / 2;
    }

    return nScale;
}


bool ModelGroup::isShareImage(osg::Geode* pGeode)
{
    if (pGeode == NULL)
    {
        return false;
    }

    unsigned int numDrawable = pGeode->getNumDrawables();
    for (unsigned int i = 0; i < numDrawable; i++)
    {
        if (m_pGeodeVisitor->getModelBuilder()->getStopFlag())  return false;

        osg::ref_ptr<osg::Drawable> pDrawable = pGeode->getDrawable(i);
        osg::ref_ptr<osg::StateSet> pStateSet = pDrawable->getStateSet();
        if (pStateSet == NULL)
        {
            continue;
        }

        unsigned int nTexCount = pStateSet->getNumTextureAttributeLists();
        for(unsigned int j = 0; j < nTexCount; j++)
        {
            if (m_pGeodeVisitor->getModelBuilder()->getStopFlag())  return false;

            osg::ref_ptr<osg::StateAttribute> pAttribute = pStateSet->getTextureAttribute(j, osg::StateAttribute::TEXTURE);
            osg::Texture2D* pTexture2D = dynamic_cast<osg::Texture2D*>(pAttribute.get());
            if(pTexture2D == NULL)
            {
                continue;
            }

            osg::Image* pImage = pTexture2D->getImage();
            if (pImage == NULL)
            {
                continue;
            }

            if (m_pGeodeVisitor->isShareImg(pImage->getFileName()))
            {
                return true;
            }
        }
    }

    return false;
}


void ModelGroup::coordinateTransform(osg::Vec3d src, osg::Vec3d &dst) const
{
    osg::Vec3d offset;
    SpatialRefInfo sri;
    m_pGeodeVisitor->getOffset(offset);
    m_pGeodeVisitor->getSpatialRefInfo(sri);

    src += offset;

    osg::Vec3d vLonLat(0.0, 0.0, src.z());
    cmm::CoordinateTransform trans;
    if (sri.m_strCoordination == "WGS84" && sri.m_strProjection == "高斯")
    {
        trans.Gauss_wgsxyToBl(src.x(), src.y(), &vLonLat[1], &vLonLat[0], sri.m_dEastOffset, sri.m_dNorthOffset, sri.m_dCentreLongitude);
    }
    else if (sri.m_strCoordination == "WGS84" && sri.m_strProjection == "UTM")
    {
        trans.UTM_wgsxyToBl(src.x(), src.y(), &vLonLat[1], &vLonLat[0], sri.m_dEastOffset, sri.m_dNorthOffset, sri.m_dCentreLongitude);
    }
    else if(sri.m_strCoordination == /*"BJ54"*/"北京54" && sri.m_strProjection == "高斯")
    {
        trans.Gauss_54xyToBl(src.x(), src.y(), &vLonLat[1], &vLonLat[0], sri.m_dEastOffset, sri.m_dNorthOffset, sri.m_dCentreLongitude);
    }
    else if(sri.m_strCoordination == /*"BJ54"*/"北京54" && sri.m_strProjection == "UTM")
    {
        trans.UTM_54xyToBl(src.x(), src.y(), &vLonLat[1], &vLonLat[0], sri.m_dEastOffset, sri.m_dNorthOffset, sri.m_dCentreLongitude);
    }
    else if(sri.m_strCoordination == /*"XA80"*/"西安80" && sri.m_strProjection == "高斯")
    {
        trans.Gauss_80xyToBl(src.x(), src.y(), &vLonLat[1], &vLonLat[0], sri.m_dEastOffset, sri.m_dNorthOffset, sri.m_dCentreLongitude);
    }
    else if(sri.m_strCoordination == /*"XA80"*/"西安80" && sri.m_strProjection == "UTM")
    {
        trans.UTM_80xyToBl(src.x(), src.y(), &vLonLat[1], &vLonLat[0], sri.m_dEastOffset, sri.m_dNorthOffset, sri.m_dCentreLongitude);
    }
    else if (sri.m_strCoordination == "哈尔滨")
    {
        trans.haerbinPMto84BL(src.y(), src.x(), &vLonLat[1], &vLonLat[0]);
    }
    else
    {
        dst = src;
        return;
    }

    osg::EllipsoidModel::instance()->convertLatLongHeightToXYZ(osg::DegreesToRadians(vLonLat[1]),
        osg::DegreesToRadians(vLonLat[0]),
        vLonLat[2],
        dst.x(), dst.y(), dst.z());

    return;
}


double ModelGroup::getBoundRadius(osg::Geode* pGeode)
{
    osg::ref_ptr<osg::ComputeBoundsVisitor> pComputer = new osg::ComputeBoundsVisitor;
    pGeode->accept(*pComputer);

    const osg::BoundingBox &bb = pComputer->getBoundingBox();
    double dblWidth   = osg::clampAbove(double(bb.xMax() - bb.xMin()), 0.1);
    double dblHeight  = osg::clampAbove(double(bb.yMax() - bb.yMin()), 0.1);
    double dblDepth   = osg::clampAbove(double(bb.zMax() - bb.zMin()), 0.1);

    const double dMax = (std::max)((std::max)(dblWidth, dblDepth), dblHeight);
    dblWidth  = osg::clampAbove(dMax * m_pGeodeVisitor->getMaxScale(), dblWidth);
    dblHeight = osg::clampAbove(dMax * m_pGeodeVisitor->getMaxScale(), dblHeight);
    dblDepth  = osg::clampAbove(dMax * m_pGeodeVisitor->getMaxScale(), dblDepth);

    const double dblVolume  = dblWidth * dblHeight * dblDepth;

    const double dblRadius3 = dblVolume * 0.75 / osg::PI;//volume of sphere = (r ^ 3) * pi * (4 / 3);
    const double dblRadius  = pow(dblRadius3, 1.0 / 3.0);

    return dblRadius;
}

double ModelGroup::getBoundRadius()
{
    osg::ref_ptr<osg::ComputeBoundsVisitor> pComputer = new osg::ComputeBoundsVisitor;
    m_pMatrixTransform->accept(*pComputer);

    const osg::BoundingBox &bb = pComputer->getBoundingBox();
    double dblWidth   = osg::clampAbove(double(bb.xMax() - bb.xMin()), 0.1);
    double dblHeight  = osg::clampAbove(double(bb.yMax() - bb.yMin()), 0.1);
    double dblDepth   = osg::clampAbove(double(bb.zMax() - bb.zMin()), 0.1);

    const double dMax = (std::max)((std::max)(dblWidth, dblDepth), dblHeight);
    dblWidth  = osg::clampAbove(dMax * m_pGeodeVisitor->getMaxScale(), dblWidth);
    dblHeight = osg::clampAbove(dMax * m_pGeodeVisitor->getMaxScale(), dblHeight);
    dblDepth  = osg::clampAbove(dMax * m_pGeodeVisitor->getMaxScale(), dblDepth);

    const double dblVolume  = dblWidth * dblHeight * dblDepth;

    const double dblRadius3 = dblVolume * 0.75 / osg::PI;//volume of sphere = (r ^ 3) * pi * (4 / 3);
    const double dblRadius  = pow(dblRadius3, 1.0 / 3.0);

    return dblRadius;
}

bool ModelGroup::UseSpaliceTextureForLevel1(osg::ref_ptr<osg::Geode>& pGeode)
{
    m_mapSpliceImage.clear();

    int nDrawNum = pGeode->getNumDrawables();
    if (nDrawNum == 0)
    {
        return false;
    }

    for (int i=0; i<nDrawNum; i++)
    {
        osg::ref_ptr<osg::Drawable> drawAble = pGeode->getDrawable(i);
        if (drawAble == NULL)
        {
            continue;
        }
        osg::ref_ptr<osg::Geometry> geometry = drawAble->asGeometry();
        if (geometry == NULL)
        {
            continue;
        }

        UseSpaliceTexture(geometry);
		
		//osg::Array* vertexArray = geometry->getVertexArray();
		//if (NULL != vertexArray)
		//{
		//	osg::Vec3Array* verts = dynamic_cast<osg::Vec3Array*>(vertexArray);
		//	if (NULL!=verts && verts->size()<50000) getUniqueVertexs(geometry);
		//}
		
		getUniqueVertexs(geometry);
    }
    MergeDrawable(pGeode);

    return true;
}

void ModelGroup::UseSpaliceTexture(osg::ref_ptr<osg::Geometry>& geometry)
{
    if (geometry == NULL)
    {
        return;
    }

    std::string strTextureName;
    osg::ref_ptr<osg::StateSet> stateSet = geometry->getStateSet();
    unsigned int uTextureAttributeIndex = 0;
    if (stateSet != NULL)
    {
        osg::StateSet::TextureAttributeList texattriList = stateSet->getTextureAttributeList();
        for (size_t i=0; i<texattriList.size(); i++)
        {
            osg::StateSet::AttributeList attriList = texattriList[i];
            osg::StateSet::AttributeList::iterator itrattri = attriList.begin();
            for (; itrattri!=attriList.end(); itrattri++,uTextureAttributeIndex++)
            {
                osg::StateAttribute::TypeMemberPair typePair = itrattri->first;
                if (typePair.first != osg::StateAttribute::TEXTURE) continue;

                osg::StateSet::RefAttributePair attriPair = itrattri->second;
                osg::ref_ptr<osg::StateAttribute> stateAttri = attriPair.first;
                osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(stateAttri.get());
                if (texture == NULL) continue;
                strTextureName = texture->getImage()->getFileName();
                break;
            }
            break;
        }
        //- end if
    }

    const DYFileToID mapFileToID = m_pGeodeVisitor->getModelBuilder()->getFileToID();
    DYFileToID::const_iterator itr_map2 = mapFileToID.find(strTextureName);
    if (itr_map2 != mapFileToID.end())
    {
        string strOutTexturePath(m_pGeodeVisitor->getModelBuilder()->getTempFilePath());
        strOutTexturePath += "output\\";

        string strIndex1;
        stringstream ss1;
        ss1.imbue(std::locale("C"));
        ss1<<itr_map2->second;ss1>>strIndex1;
        strOutTexturePath = strOutTexturePath + strIndex1 + ".bmp";

        osg::ref_ptr<osg::StateAttribute> pAttribute = stateSet->getTextureAttribute(uTextureAttributeIndex, osg::StateAttribute::TEXTURE);
        osg::ref_ptr<osg::Texture2D> pTexture2D = dynamic_cast<osg::Texture2D*>(pAttribute.get());
        if(pTexture2D != NULL)
        {
            osg::ref_ptr<osg::Image> pImage = pTexture2D->getImage();
            if (m_mapSpliceImage.find(strOutTexturePath) != m_mapSpliceImage.end())
            {
                pTexture2D->setImage(m_mapSpliceImage[strOutTexturePath]);
            }
            else
            {
                pImage = osgDB::readImageFile(strOutTexturePath);
                pImage->setFileName(strOutTexturePath);
                pTexture2D->setImage(pImage.get());

                m_mapSpliceImage[strOutTexturePath] = pImage;
            }
        }
    }

    double derx=1.0, dery=1.0, dofx=0.0, dofy=0.0;
    const DYFileToMat3x3 mapFileToMat3x3 = m_pGeodeVisitor->getModelBuilder()->getFileToMat3x3();
    DYFileToMat3x3::const_iterator itr_map1 = mapFileToMat3x3.find(strTextureName);
    if (itr_map1 != mapFileToMat3x3.end())
    {
        derx = itr_map1->second.M00;
        dery = itr_map1->second.M11;
        dofx = itr_map1->second.M20;
        dofy = itr_map1->second.M21;
    }

    osg::Array* texcoordArray = geometry->getTexCoordArray(0);
    if (texcoordArray != NULL)
    {
        osg::Vec2Array* texcoords = dynamic_cast<osg::Vec2Array*>(texcoordArray);
        std::vector<osg::Vec2 >::iterator iter_tex = texcoords->begin();

        for (; iter_tex!=texcoords->end(); iter_tex++)
        {
            iter_tex->_v[0] = derx * iter_tex->x() + dofx;
            iter_tex->_v[1] = dery * iter_tex->y() + dofy;
        }
    }

    return;
}

bool ModelGroup::CreateLodFromGeode(osg::ref_ptr<osg::Geode>& pGeode, float fAlpha)
{
    m_mapSpliceImage.clear();

    int nDrawNum = pGeode->getNumDrawables();
    if (nDrawNum == 0)
    {
        return false;
    }

    for (int i=0; i<nDrawNum; i++)
    {
        osg::ref_ptr<osg::Drawable> drawAble = pGeode->getDrawable(i);
        if (drawAble == NULL)
        {
            continue;
        }
        osg::ref_ptr<osg::Geometry> geometry = drawAble->asGeometry();
        if (geometry == NULL)
        {
            continue;
        }

        vector<DYVert> vecDYVert, vecDYVertOut;
        AnalysisVertex(geometry, vecDYVert);

        DYIntList indexList;
        AnalysisVertIndex(geometry, indexList);

        DYFaceList pFaceList;
        for (int j=0; j<indexList.GetCount(); j++)
        {
            DYFace aFace(vecDYVert[indexList[j]], vecDYVert[indexList[j+1]], vecDYVert[indexList[j+2]]);j += 2;
            pFaceList.Append(aFace);
        }

        vector<pair<int, DYIntList> > vecModeAndIndexList;
        DYFaceList::FromSimplify(pFaceList,vecDYVertOut, vecModeAndIndexList, fAlpha);

        if (vecDYVertOut.size()==0 || vecModeAndIndexList.size()==0)
        {
            pGeode->removeDrawable(drawAble);
            nDrawNum--;i--;
            continue;
        }

        ModifyGeometry(geometry, vecDYVertOut, vecModeAndIndexList);
    }

    MergeDrawable(pGeode);

    return true;
}

bool ModelGroup::MergeDrawable(osg::ref_ptr<osg::Geode>& pGeode)
{
    int nDrawNum = pGeode->getNumDrawables();
    if (nDrawNum == 0)
    {
        return false;
    }

    LODSEGMENT LODSeg;
    m_pGeodeVisitor->getLODSegment(LODSeg);

	if (	(LODSeg.nValidLodLevels==2 && m_nCurrentLODIndex==2)
		 || (LODSeg.nValidLodLevels==3 && m_nCurrentLODIndex==3)
		 || (LODSeg.nValidLodLevels==4 && m_nCurrentLODIndex==3)
		 || (LODSeg.nValidLodLevels==4 && m_nCurrentLODIndex==4) )
	{
		MergeDrawableForLastLevel(pGeode);
	}
    else
    {
        map<string, vector<osg::ref_ptr<osg::Drawable>> > mapTexNameAndVecDrawable;
        
        for (int i=0; i<nDrawNum; i++)
        {
            osg::ref_ptr<osg::Drawable> drawAble = pGeode->getDrawable(i);
            if (drawAble == NULL)   continue;

            osg::ref_ptr<osg::Geometry> geometry = drawAble->asGeometry();
            if (geometry == NULL)   continue;

			mapTexNameAndVecDrawable[getTextureName(geometry)].push_back(drawAble);
        }

        map<string, vector<osg::ref_ptr<osg::Drawable>> >::iterator itr = mapTexNameAndVecDrawable.begin();
        for (; itr!=mapTexNameAndVecDrawable.end(); itr++)
        {
            if (itr->second.size() <= 1)
			{
				continue;
			}

            Merge(pGeode, (osg::ref_ptr<osg::Geometry>)(itr->second[0]->asGeometry()), itr->second);
        }
    }

    return true;
}

// 根据材质合并
// map<osg::ref_ptr<osg::Material>, vector<osg::ref_ptr<osg::Drawable>>> mapMaterialAndDrawable;
// 
// osg::ref_ptr<osg::StateSet> pState = geometry->getOrCreateStateSet();
// osg::ref_ptr<osg::Material> pMaterial = dynamic_cast<osg::Material *>(pState->getAttribute(osg::StateAttribute::MATERIAL));
// 
// if (mapMaterialAndDrawable.find(pMaterial) != mapMaterialAndDrawable.end())
// {
// 	mapTexNameAndVecDrawable[getTextureName(geometry)].push_back(drawAble);
// }
// else
// {
// 	mapMaterialAndDrawable[pMaterial].push_back(drawAble);
// }

bool ModelGroup::MergeDrawableForLastLevel(osg::ref_ptr<osg::Geode>& pGeode)
{
    int nDrawNum = pGeode->getNumDrawables();
    if (nDrawNum == 0)
    {
        return false;
    }

	LODSEGMENT LODSeg;
	m_pGeodeVisitor->getLODSegment(LODSeg);

    vector<osg::ref_ptr<osg::Drawable>> vecDrawable;
    for (int i=0; i<nDrawNum; i++)
    {
        osg::ref_ptr<osg::Drawable> drawAble = pGeode->getDrawable(i);
        if (drawAble == NULL)
        {
            continue;
        }

		osg::ref_ptr<osg::Geometry> geometry = drawAble->asGeometry();
		if (geometry == NULL)
		{
			continue;
		}

		if (LODSeg.nValidLodLevels==2 || LODSeg.nValidLodLevels == 3)
		{
			if (!VertexRender(drawAble)) continue;
		}
		else if (LODSeg.nValidLodLevels==4 && m_nCurrentLODIndex==LODSeg.nValidLodLevels-1)
		{
			if (!VertexRender(drawAble)) continue;
		}

        vecDrawable.push_back(drawAble);
    }

	if (vecDrawable.size() > 0)
	{
		Merge(pGeode, (osg::ref_ptr<osg::Geometry>)(vecDrawable[0]->asGeometry()), vecDrawable);
	}

    return true;
}

void ModelGroup::Merge(osg::ref_ptr<osg::Geode>& pGeode, osg::ref_ptr<osg::Geometry>& geometry, vector<osg::ref_ptr<osg::Drawable>>& vecDrawable)
{
    if (geometry == NULL)
    {
        return;
    }

    osg::Vec3Array* coords = (osg::Vec3Array*)(geometry->getVertexArray());
    osg::Vec3Array* ncoords = (osg::Vec3Array*)(geometry->getNormalArray());
    osg::Vec2Array* tcoords = (osg::Vec2Array*)(geometry->getTexCoordArray(0));
    osg::Vec4Array* colors = (osg::Vec4Array*)(geometry->getColorArray());

    unsigned int uTotalInex = 0;
    for (size_t i=1; i<vecDrawable.size(); i++)
    {
        if (vecDrawable[i-1]->asGeometry()==NULL || vecDrawable[i]->asGeometry()==NULL)
        {
            continue;
        }

        osg::Vec3Array* tmp = (osg::Vec3Array*)(vecDrawable[i-1]->asGeometry()->getVertexArray());
        if (tmp != NULL)
        {
            unsigned int utmpcnt = tmp->getNumElements();
            uTotalInex += utmpcnt;
        }

        osg::Vec3Array* coordstmp = (osg::Vec3Array*)(vecDrawable[i]->asGeometry()->getVertexArray());
        if (coords!=NULL && coordstmp!=NULL)
        {
            std::vector<osg::Vec3 >::iterator iter_ver = coordstmp->begin () ;
            for (; iter_ver!=coordstmp->end(); iter_ver++)
            {
                osg::Vec3 v(*iter_ver);
                (*coords).push_back( osg::Vec3(v.x(), v.y(), v.z()) );
            }
        }

        osg::Vec3Array* ncoordstmp = (osg::Vec3Array*)(vecDrawable[i]->asGeometry()->getNormalArray());
        if (ncoords!=NULL && ncoordstmp!=NULL)
        {
            std::vector<osg::Vec3 >::iterator iter_ver_nor = ncoordstmp->begin () ;
            for (; iter_ver_nor!=ncoordstmp->end(); iter_ver_nor++)
            {
                osg::Vec3 v(*iter_ver_nor);
                (*ncoords).push_back( osg::Vec3(v.x(), v.y(), v.z()) );
            }
        }

        osg::Vec2Array* tcoordstmp = (osg::Vec2Array*)(vecDrawable[i]->asGeometry()->getTexCoordArray(0));
        if (tcoords!=NULL && tcoordstmp!=NULL)
        {
            std::vector<osg::Vec2 >::iterator iter_ver_tex = tcoordstmp->begin () ;
            for (; iter_ver_tex!=tcoordstmp->end(); iter_ver_tex++)
            {
                osg::Vec2 v(*iter_ver_tex);
                (*tcoords).push_back( osg::Vec2(v.x(), v.y()) );
            }
        }

        osg::Vec4Array* colorstmp = (osg::Vec4Array*)(vecDrawable[i]->asGeometry()->getColorArray());
        if (colors!=NULL && colorstmp!=NULL)
        {
            std::vector<osg::Vec4 >::iterator iter_ver = colorstmp->begin () ;
            for (; iter_ver!=colorstmp->end(); iter_ver++)
            {
                osg::Vec4 v(*iter_ver);
                (*colors).push_back( osg::Vec4(v.r(), v.g(), v.b(),v.a()) );
            }
        }

        for (unsigned int ipr=0; ipr<vecDrawable[i]->asGeometry()->getNumPrimitiveSets(); ipr++)
        {
            osg::PrimitiveSet* prset=vecDrawable[i]->asGeometry()->getPrimitiveSet(ipr);
            if (prset == NULL)
            {
                continue;
            }

            osg::ref_ptr<osg::DrawElementsUInt> elements = new osg::DrawElementsUInt(prset->getMode());
            unsigned int nCnt = prset->getNumIndices();
            elements->reserveElements(nCnt);
            for (unsigned int k=0; k<nCnt; k++) 
            {
                elements->addElement(prset->index(k) + uTotalInex);
            }
            geometry->addPrimitiveSet(elements.get());
        }

        pGeode->removeDrawable(vecDrawable[i]);
    }

    geometry->dirtyBound();
}

bool ModelGroup::AnalysisVertex(osg::ref_ptr<osg::Geometry> geometry, std::vector<DYVert>& vecDYVert)
{
    if (geometry == NULL)
    {
        return false;
    }

    osg::Array* vertexArray = geometry->getVertexArray();
    if (vertexArray != NULL)
    {
        osg::Vec3Array* verts = dynamic_cast<osg::Vec3Array*>(vertexArray);
        long lVertNum = verts->size();
        if (lVertNum <= 0)
        {
            return false;
        }
        vecDYVert.resize(lVertNum);

        std::vector<osg::Vec3 >::iterator iter_ver = verts->begin () ;
        long nVertIndex = 0;
        for (; iter_ver!=verts->end(); iter_ver++, nVertIndex++)
        {
            osg::Vec3 v(*iter_ver);

            vecDYVert[nVertIndex].Point.X = v.x();
            vecDYVert[nVertIndex].Point.Y = v.y();
            vecDYVert[nVertIndex].Point.Z = v.z();
        }
    }

    osg::Array* normalArray = geometry->getNormalArray();
    if (normalArray != NULL)
    {
        osg::Vec3Array* normals = dynamic_cast<osg::Vec3Array*>(normalArray);
        std::vector<osg::Vec3 >::iterator iter_nor = normals->begin () ;
        long nVertIndex = 0;
        for (; iter_nor!=normals->end(); iter_nor++, nVertIndex++)
        {
            osg::Vec3 _origin;
            osg::Vec3 v(*iter_nor);

            vecDYVert[nVertIndex].Normal.X = v.x();
            vecDYVert[nVertIndex].Normal.Y = v.y();
            vecDYVert[nVertIndex].Normal.Z = v.z();
        }
    }

    osg::Array* texcoordArray = geometry->getTexCoordArray(0);
    if (texcoordArray != NULL)
    {
        osg::Vec2Array* texcoords = dynamic_cast<osg::Vec2Array*>(texcoordArray);
        std::vector<osg::Vec2 >::iterator iter_tex = texcoords->begin () ;
        long nVertIndex = 0;
        for (; iter_tex!=texcoords->end(); iter_tex++, nVertIndex++)
        {
            vecDYVert[nVertIndex].Texel.X = iter_tex->x();
            vecDYVert[nVertIndex].Texel.Y = iter_tex->y();
        }
    }

    return true;
}

bool ModelGroup::AnalysisVertIndex(osg::ref_ptr<osg::Geometry> geometry, DYIntList& indexList)
{
    unsigned int nSetcnt = geometry->getNumPrimitiveSets();
    for (unsigned int ipr=0; ipr<geometry->getNumPrimitiveSets(); ipr++)
    {
        osg::PrimitiveSet* prset=geometry->getPrimitiveSet(ipr);
        if (prset == NULL)
        {
            continue;
        }

        std::vector<unsigned int> vecVertIndx;
        unsigned int ncnt = prset->getNumIndices();
        for (unsigned int ic=0; ic<ncnt; ic++) 
        {
            unsigned int iIndex = prset->index(ic);
            vecVertIndx.push_back(iIndex);
        }

        int nMode = prset->getMode();
        if (nMode == 5 || nMode == 3 || nMode == 2)
        {
            std::vector<unsigned int> vecNewVertIndx;
            for (unsigned int ic=2; ic<ncnt; ic++) 
            {
                if (ic%2 == 0)
                {
                    vecNewVertIndx.push_back(vecVertIndx[ic-2]);
                    vecNewVertIndx.push_back(vecVertIndx[ic+1 -2]);
                    vecNewVertIndx.push_back(vecVertIndx[ic+2 -2]);
                }
                else
                {
                    vecNewVertIndx.push_back(vecVertIndx[ic-2]);
                    vecNewVertIndx.push_back(vecVertIndx[ic+2 -2]);
                    vecNewVertIndx.push_back(vecVertIndx[ic+1 -2]);
                }
            }
            vecVertIndx = vecNewVertIndx;
        }

        for (size_t i=0; i<vecVertIndx.size(); i++)
        {
            indexList.Append(vecVertIndx[i]);
        }
    }

    return true;
}

bool ModelGroup::ModifyGeometry(osg::ref_ptr<osg::Geometry>& geometry, const std::vector<DYVert>& vecDYVertOut, const std::vector<std::pair<int, DYIntList> >& vecModeAndIndexList)
{
    if (geometry == NULL)
    {
        return false;
    }

    long lVertNum = vecDYVertOut.size();
    if (lVertNum==0 || vecModeAndIndexList.size()==0)
    {
        return false;
    }

    osg::Vec3Array* coords = (osg::Vec3Array*)(geometry->getVertexArray());
    if (coords != NULL)
    {
        (*coords).clear();
        for (long j=0; j<lVertNum; j++)
        {
            (*coords).push_back(osg::Vec3(vecDYVertOut[j].Point.X, vecDYVertOut[j].Point.Y,vecDYVertOut[j].Point.Z));
        }
    }

    osg::Vec3Array* ncoords = (osg::Vec3Array*)(geometry->getNormalArray());
    if (ncoords != NULL)
    {
        (*ncoords).clear();
        for (long j=0; j<lVertNum; j++)
        {
            (*ncoords).push_back(osg::Vec3(vecDYVertOut[j].Normal.X, vecDYVertOut[j].Normal.Y, vecDYVertOut[j].Normal.Z));
        }
    }

    ModifyTextureInfo(geometry, vecDYVertOut);

	for (int i=0; i<geometry->getNumPrimitiveSets(); i=0)
	{
		geometry->removePrimitiveSet(i);
	}
	for (size_t i=0; i<vecModeAndIndexList.size(); i++)
	{
		osg::ref_ptr<osg::DrawElementsUInt> elements = new osg::DrawElementsUInt(vecModeAndIndexList[i].first);

		int nCnt = vecModeAndIndexList[i].second.GetCount();
		elements->reserveElements(nCnt);
		for (unsigned int k=0; k<nCnt; k++) 
		{
			elements->addElement(vecModeAndIndexList[i].second[k]);
		}
		geometry->addPrimitiveSet(elements.get());
	}

    geometry->dirtyBound();
    return true;
}

void ModelGroup::ModifyTextureInfo(osg::ref_ptr<osg::Geometry>& geometry, const std::vector<DYVert> &vecDYVertOut)
{
    long lVertNum = vecDYVertOut.size();
    if (lVertNum == 0)
    {
        return;
    }

    std::string strTextureName;
    osg::ref_ptr<osg::StateSet> stateSet = geometry->getStateSet();
    unsigned int uTextureAttributeIndex = 0;
    if (stateSet != NULL)
    {
        osg::StateSet::TextureAttributeList texattriList = stateSet->getTextureAttributeList();
        for (size_t i=0; i<texattriList.size(); i++)
        {
            osg::StateSet::AttributeList attriList = texattriList[i];
            osg::StateSet::AttributeList::iterator itrattri = attriList.begin();
            for (; itrattri!=attriList.end(); itrattri++,uTextureAttributeIndex++)
            {
                osg::StateAttribute::TypeMemberPair typePair = itrattri->first;
                if (typePair.first != osg::StateAttribute::TEXTURE) continue;

                osg::StateSet::RefAttributePair attriPair = itrattri->second;
                osg::ref_ptr<osg::StateAttribute> stateAttri = attriPair.first;
                osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(stateAttri.get());
                if (texture == NULL) continue;
                strTextureName = texture->getImage()->getFileName();
                break;
            }
            break;
        }
        //- end if
    }

    const DYFileToID mapFileToID = m_pGeodeVisitor->getModelBuilder()->getFileToID();
    DYFileToID::const_iterator itr_map2 = mapFileToID.find(strTextureName);
    if (itr_map2 != mapFileToID.end())
    {
        string strOutTexturePath(m_pGeodeVisitor->getModelBuilder()->getTempFilePath());
        strOutTexturePath += "output\\";

        string strIndex1;
        stringstream ss1;
        ss1.imbue(std::locale("C"));
        ss1<<itr_map2->second;ss1>>strIndex1;
        strOutTexturePath = strOutTexturePath + strIndex1 + ".bmp";

        osg::ref_ptr<osg::StateAttribute> pAttribute = stateSet->getTextureAttribute(uTextureAttributeIndex, osg::StateAttribute::TEXTURE);
        osg::ref_ptr<osg::Texture2D> pTexture2D = dynamic_cast<osg::Texture2D*>(pAttribute.get());
        if(pTexture2D != NULL)
        {
            osg::ref_ptr<osg::Image> pImage = pTexture2D->getImage();
            if (m_mapSpliceImage.find(strOutTexturePath) != m_mapSpliceImage.end())
            {
                pTexture2D->setImage(m_mapSpliceImage[strOutTexturePath]);
            }
            else
            {
                pImage = osgDB::readImageFile(strOutTexturePath);
                pImage->setFileName(strOutTexturePath);
                pTexture2D->setImage(pImage.get());

                m_mapSpliceImage[strOutTexturePath] = pImage;
            }
        }
    }

    double derx=1.0, dery=1.0, dofx=0.0, dofy=0.0;
    const DYFileToMat3x3 mapFileToMat3x3 = m_pGeodeVisitor->getModelBuilder()->getFileToMat3x3();
    DYFileToMat3x3::const_iterator itr_map1 = mapFileToMat3x3.find(strTextureName);
    if (itr_map1 != mapFileToMat3x3.end())
    {
        derx = itr_map1->second.M00;
        dery = itr_map1->second.M11;
        dofx = itr_map1->second.M20;
        dofy = itr_map1->second.M21;
    }

    osg::Vec2Array* tcoords = (osg::Vec2Array*)(geometry->getTexCoordArray(0));
    if (tcoords != NULL)
    {
        (*tcoords).clear();
        for (long j=0; j<lVertNum; j++)
        {
            (*tcoords).push_back(osg::Vec2(derx * vecDYVertOut[j].Texel.X + dofx , dery * vecDYVertOut[j].Texel.Y + dofy));
        }
    }

    return;
}

void ModelGroup::RemoveTextureAttribute(osg::ref_ptr<osg::Geode> pGeode)
{
    unsigned int numDrawable = pGeode->getNumDrawables();
    for (unsigned int i = 0; i < numDrawable; i++)
    {
        osg::Drawable* pDrawable = pGeode->getDrawable(i);
        osg::StateSet* pStateSet = pDrawable->getStateSet();
        if (pStateSet==NULL || m_pGeodeVisitor->getMultiPathList() || pStateSet->getNumParents()>1)
        {
            continue;
        }

        unsigned int nTexCount = pStateSet->getNumTextureAttributeLists();
        for(unsigned int j = 0; j < nTexCount; j++)
        {
            osg::StateAttribute* pAttribute = (osg::StateAttribute*)pStateSet->getTextureAttribute(j, osg::StateAttribute::TEXTURE);
            pStateSet->removeTextureAttribute(j, pAttribute);
        }
    }

    return;
}

void ModelGroup::saveGeodes_5level_source(osg::ref_ptr<osg::Geode>& pGeode, double dMaxDis, const LODSEGMENT& LODSeg)
{
    ID idDetail;
    DETAILINFO detailInfo;
    bool bStopFlag = m_pGeodeVisitor->getModelBuilder()->getStopFlag();

    if (dMaxDis > 0.0 && !bStopFlag)//一级LOD
    {
        m_nCurrentLODIndex = 1;
        osg::ref_ptr<osg::Geode> pNewGeode = dynamic_cast<osg::Geode*>(pGeode->clone(osg::CopyOp::DEEP_COPY_ALL));
        if (pNewGeode == NULL)
        {
            return;
        }

        SHAREGEODE shareGeode;
        if (m_pGeodeVisitor->getShareGeode(m_nCurrentLODIndex, shareGeode))
        {
            idDetail = shareGeode.idDetail;
        }
        else
        {
            changeGeode(pNewGeode);
            saveGeode(m_nCurrentLODIndex, getGeodID(), pNewGeode, idDetail);
        }

        detailInfo.idDetail  = idDetail;
        detailInfo.dMinRange = 0.0;
        detailInfo.dMaxRange = dMaxDis < LODSeg.dLOD1 ? dMaxDis : LODSeg.dLOD1;
        m_vecDetail.push_back(detailInfo);
    }

    if (dMaxDis > LODSeg.dLOD1 && !bStopFlag)//二级LOD
    {
        m_nCurrentLODIndex = 2;
        osg::ref_ptr<osg::Geode> pNewGeode = dynamic_cast<osg::Geode*>(pGeode->clone(osg::CopyOp::DEEP_COPY_ALL));
        if (pNewGeode == NULL)
        {
            return;
        }

        SHAREGEODE shareGeode;
        if (m_pGeodeVisitor->getShareGeode(m_nCurrentLODIndex, shareGeode))
        {
            idDetail = shareGeode.idDetail;
        }
        else
        {
            changeGeode(pNewGeode);
            saveGeode(m_nCurrentLODIndex, getGeodID(), pNewGeode, idDetail);
        }

        detailInfo.idDetail  = idDetail;
        detailInfo.dMinRange = LODSeg.dLOD1;
        detailInfo.dMaxRange = dMaxDis < LODSeg.dLOD2 ? dMaxDis : LODSeg.dLOD2;
        m_vecDetail.push_back(detailInfo);
    }

    if (dMaxDis > LODSeg.dLOD2 && !bStopFlag)//三级LOD
    {
        m_nCurrentLODIndex = 3;
        osg::ref_ptr<osg::Geode> pNewGeode = dynamic_cast<osg::Geode*>(pGeode->clone(osg::CopyOp::DEEP_COPY_ALL));
        if (pNewGeode == NULL)
        {
            return;
        }

        SHAREGEODE shareGeode;
        if (m_pGeodeVisitor->getShareGeode(m_nCurrentLODIndex, shareGeode))
        {
            idDetail = shareGeode.idDetail;
        }
        else
        {
            changeGeode(pNewGeode);
            saveGeode(m_nCurrentLODIndex, getGeodID(), pNewGeode, idDetail);
        }

        detailInfo.idDetail  = idDetail;
        detailInfo.dMinRange = LODSeg.dLOD2;
        detailInfo.dMaxRange = dMaxDis < LODSeg.dLOD3 ? dMaxDis : LODSeg.dLOD3;;
        m_vecDetail.push_back(detailInfo);
    }

    if (dMaxDis > LODSeg.dLOD3 && !bStopFlag)//四级LOD
    {
        m_nCurrentLODIndex = 4;
        osg::ref_ptr<osg::Geode> pNewGeode = dynamic_cast<osg::Geode*>(pGeode->clone(osg::CopyOp::DEEP_COPY_ALL));
        if (pNewGeode == NULL)
        {
            return;
        }

        SHAREGEODE shareGeode;
        if (m_pGeodeVisitor->getShareGeode(m_nCurrentLODIndex, shareGeode))
        {
            idDetail = shareGeode.idDetail;
        }
        else
        {
            changeGeode(pNewGeode);
            saveGeode(m_nCurrentLODIndex, getGeodID(), pNewGeode, idDetail);
        }

        detailInfo.idDetail  = idDetail;
        detailInfo.dMinRange = LODSeg.dLOD3;
        detailInfo.dMaxRange = dMaxDis < LODSeg.dLOD4 ? dMaxDis : LODSeg.dLOD4;
        m_vecDetail.push_back(detailInfo);
    }

    if (dMaxDis > LODSeg.dLOD4 && !bStopFlag)//五级LOD
    {
        m_nCurrentLODIndex = 5;
        osg::ref_ptr<osg::Geode> pNewGeode = dynamic_cast<osg::Geode*>(pGeode->clone(osg::CopyOp::DEEP_COPY_ALL));
        if (pNewGeode == NULL)
        {
            return;
        }

        SHAREGEODE shareGeode;
        if (m_pGeodeVisitor->getShareGeode(m_nCurrentLODIndex, shareGeode))
        {
            idDetail = shareGeode.idDetail;
        }
        else
        {
            changeGeode(pNewGeode);
            saveGeode(m_nCurrentLODIndex, getGeodID(), pNewGeode, idDetail);
        }

        detailInfo.idDetail  = idDetail;
        detailInfo.dMinRange = LODSeg.dLOD4;
        detailInfo.dMaxRange = dMaxDis < LODSeg.dLOD5 ? dMaxDis : LODSeg.dLOD5;
        m_vecDetail.push_back(detailInfo);
    }

    RemoveTextureAttribute(pGeode);
}

void ModelGroup::saveGeodes_1level_optimize(osg::ref_ptr<osg::Geode>& pGeode, double dMaxDis, const LODSEGMENT& LODSeg)
{
	ID idDetail;
	DETAILINFO detailInfo;
	bool bStopFlag = m_pGeodeVisitor->getModelBuilder()->getStopFlag();

	if (dMaxDis > 0.0 && !bStopFlag)//一级LOD
	{
		m_nCurrentLODIndex = 1;
		osg::ref_ptr<osg::Geode> pNewGeode = dynamic_cast<osg::Geode*>(pGeode->clone(osg::CopyOp::DEEP_COPY_ALL));
		if (pNewGeode == NULL)
		{
			return;
		}

		SHAREGEODE shareGeode;
		if (m_pGeodeVisitor->getShareGeode(m_nCurrentLODIndex, shareGeode))
		{
			idDetail = shareGeode.idDetail;
		}
		else
		{
			changeGeode(pNewGeode);
			UseSpaliceTextureForLevel1(pNewGeode);
			saveGeode(m_nCurrentLODIndex, getGeodID(), pNewGeode, idDetail);
		}

		detailInfo.idDetail  = idDetail;
		detailInfo.dMinRange = 0.0;
		detailInfo.dMaxRange = dMaxDis < LODSeg.dLOD1 ? dMaxDis : LODSeg.dLOD1;
		m_vecDetail.push_back(detailInfo);
	}

	RemoveTextureAttribute(pGeode);
}

void ModelGroup::saveGeodes_2level_optimize(osg::ref_ptr<osg::Geode>& pGeode, double dMaxDis, const LODSEGMENT& LODSeg)
{
	ID idDetail;
	DETAILINFO detailInfo;
	bool bStopFlag = m_pGeodeVisitor->getModelBuilder()->getStopFlag();

	if (dMaxDis > 0.0 && !bStopFlag)//一级LOD
	{
		m_nCurrentLODIndex = 1;
		osg::ref_ptr<osg::Geode> pNewGeode = dynamic_cast<osg::Geode*>(pGeode->clone(osg::CopyOp::DEEP_COPY_ALL));
		if (pNewGeode == NULL)
		{
			return;
		}

		SHAREGEODE shareGeode;
		if (m_pGeodeVisitor->getShareGeode(m_nCurrentLODIndex, shareGeode))
		{
			idDetail = shareGeode.idDetail;
		}
		else
		{
			changeGeode(pNewGeode);
			UseSpaliceTextureForLevel1(pNewGeode);
			saveGeode(m_nCurrentLODIndex, getGeodID(), pNewGeode, idDetail);
		}

		detailInfo.idDetail  = idDetail;
		detailInfo.dMinRange = 0.0;
		detailInfo.dMaxRange = dMaxDis < LODSeg.dLOD1 ? dMaxDis : LODSeg.dLOD1;
		m_vecDetail.push_back(detailInfo);
	}

	if (dMaxDis > LODSeg.dLOD1 && !bStopFlag)//二级LOD
	{
		m_nCurrentLODIndex = 2;
		osg::ref_ptr<osg::Geode> pNewGeode = dynamic_cast<osg::Geode*>(pGeode->clone(osg::CopyOp::DEEP_COPY_ALL));
		if (pNewGeode == NULL)
		{
			return;
		}

		SHAREGEODE shareGeode;
		if (m_pGeodeVisitor->getShareGeode(m_nCurrentLODIndex, shareGeode))
		{
			idDetail = shareGeode.idDetail;
		}
		else
		{
			changeGeode(pNewGeode);
			UseSpaliceTextureForLevel1(pNewGeode);
			saveGeode(m_nCurrentLODIndex, getGeodID(), pNewGeode, idDetail);
		}

		detailInfo.idDetail  = idDetail;
		detailInfo.dMinRange = LODSeg.dLOD1;
		detailInfo.dMaxRange = dMaxDis < LODSeg.dLOD2 ? dMaxDis : LODSeg.dLOD2;
		m_vecDetail.push_back(detailInfo);
	}

	RemoveTextureAttribute(pGeode);
}

void ModelGroup::saveGeodes_3level_optimize(osg::ref_ptr<osg::Geode>& pGeode, double dMaxDis, const LODSEGMENT& LODSeg)
{
    ID idDetail;
    DETAILINFO detailInfo;
    bool bStopFlag = m_pGeodeVisitor->getModelBuilder()->getStopFlag();

    if (dMaxDis > 0.0 && !bStopFlag)//一级LOD
    {
        m_nCurrentLODIndex = 1;
        osg::ref_ptr<osg::Geode> pNewGeode = dynamic_cast<osg::Geode*>(pGeode->clone(osg::CopyOp::DEEP_COPY_ALL));
        if (pNewGeode == NULL)
        {
            return;
        }

        SHAREGEODE shareGeode;
        if (m_pGeodeVisitor->getShareGeode(m_nCurrentLODIndex, shareGeode))
        {
            idDetail = shareGeode.idDetail;
        }
        else
        {
            changeGeode(pNewGeode);
            UseSpaliceTextureForLevel1(pNewGeode);
            saveGeode(m_nCurrentLODIndex, getGeodID(), pNewGeode, idDetail);
        }

        detailInfo.idDetail  = idDetail;
        detailInfo.dMinRange = 0.0;
        detailInfo.dMaxRange = dMaxDis < LODSeg.dLOD1 ? dMaxDis : LODSeg.dLOD1;
        m_vecDetail.push_back(detailInfo);
    }

    if (dMaxDis > LODSeg.dLOD1 && !bStopFlag)//二级LOD
    {
        m_nCurrentLODIndex = 2;
        osg::ref_ptr<osg::Geode> pNewGeode = dynamic_cast<osg::Geode*>(pGeode->clone(osg::CopyOp::DEEP_COPY_ALL));
        if (pNewGeode == NULL)
        {
            return;
        }

        SHAREGEODE shareGeode;
        if (m_pGeodeVisitor->getShareGeode(m_nCurrentLODIndex, shareGeode))
        {
            idDetail = shareGeode.idDetail;
        }
        else
        {
            changeGeode(pNewGeode);
            //CreateLodFromGeode(pNewGeode, 1.0);
			UseSpaliceTextureForLevel1(pNewGeode);	//- 生产梅岭数据
            saveGeode(m_nCurrentLODIndex, getGeodID(), pNewGeode, idDetail);
        }

        detailInfo.idDetail  = idDetail;
        detailInfo.dMinRange = LODSeg.dLOD1;
        detailInfo.dMaxRange = dMaxDis < LODSeg.dLOD2 ? dMaxDis : LODSeg.dLOD2;
        m_vecDetail.push_back(detailInfo);
    }

    if (dMaxDis > LODSeg.dLOD2 && !bStopFlag)//三级LOD
    {
        m_nCurrentLODIndex = 3;
        osg::ref_ptr<osg::Geode> pNewGeode = dynamic_cast<osg::Geode*>(pGeode->clone(osg::CopyOp::DEEP_COPY_ALL));
        if (pNewGeode == NULL)
        {
            return;
        }

        SHAREGEODE shareGeode;
        if (m_pGeodeVisitor->getShareGeode(m_nCurrentLODIndex, shareGeode))
        {
            idDetail = shareGeode.idDetail;
        }
        else
        {
            changeGeode(pNewGeode);
            //CreateLodFromGeode(pNewGeode, 16.0);
			CreateLodFromGeode(pNewGeode, 1.0);	//- 生产梅岭数据
            saveGeode(m_nCurrentLODIndex, getGeodID(), pNewGeode, idDetail);
        }

        detailInfo.idDetail  = idDetail;
        detailInfo.dMinRange = LODSeg.dLOD2;
        detailInfo.dMaxRange = dMaxDis < LODSeg.dLOD3 ? dMaxDis : LODSeg.dLOD3;;
        m_vecDetail.push_back(detailInfo);
    }

    RemoveTextureAttribute(pGeode);
}

void ModelGroup::saveGeodes_4level_optimize(osg::ref_ptr<osg::Geode>& pGeode, double dMaxDis, const LODSEGMENT& LODSeg)
{
    ID idDetail;
    DETAILINFO detailInfo;
    bool bStopFlag = m_pGeodeVisitor->getModelBuilder()->getStopFlag();

    if (dMaxDis > 0.0 && !bStopFlag)//一级LOD
    {
        m_nCurrentLODIndex = 1;
        osg::ref_ptr<osg::Geode> pNewGeode = dynamic_cast<osg::Geode*>(pGeode->clone(osg::CopyOp::DEEP_COPY_ALL));
        if (pNewGeode == NULL)
        {
            return;
        }

        SHAREGEODE shareGeode;
        if (m_pGeodeVisitor->getShareGeode(m_nCurrentLODIndex, shareGeode))
        {
            idDetail = shareGeode.idDetail;
        }
        else
        {
            changeGeode(pNewGeode);
            UseSpaliceTextureForLevel1(pNewGeode);
            saveGeode(m_nCurrentLODIndex, getGeodID(), pNewGeode, idDetail);
        }

        detailInfo.idDetail  = idDetail;
        detailInfo.dMinRange = 0.0;
        detailInfo.dMaxRange = dMaxDis < LODSeg.dLOD1 ? dMaxDis : LODSeg.dLOD1;
        m_vecDetail.push_back(detailInfo);
    }

    if (dMaxDis > LODSeg.dLOD1 && !bStopFlag)//二级LOD
    {
        m_nCurrentLODIndex = 2;
        osg::ref_ptr<osg::Geode> pNewGeode = dynamic_cast<osg::Geode*>(pGeode->clone(osg::CopyOp::DEEP_COPY_ALL));
        if (pNewGeode == NULL)
        {
            return;
        }

        SHAREGEODE shareGeode;
        if (m_pGeodeVisitor->getShareGeode(m_nCurrentLODIndex, shareGeode))
        {
            idDetail = shareGeode.idDetail;
        }
        else
        {
            changeGeode(pNewGeode);
            //CreateLodFromGeode(pNewGeode, 1.0);
			UseSpaliceTextureForLevel1(pNewGeode);	//- 厦门数据第二级不简化
            saveGeode(m_nCurrentLODIndex, getGeodID(), pNewGeode, idDetail);
        }

        detailInfo.idDetail  = idDetail;
        detailInfo.dMinRange = LODSeg.dLOD1;
        detailInfo.dMaxRange = dMaxDis < LODSeg.dLOD2 ? dMaxDis : LODSeg.dLOD2;
        m_vecDetail.push_back(detailInfo);
    }

    if (dMaxDis > LODSeg.dLOD2 && !bStopFlag)//三级LOD
    {
        m_nCurrentLODIndex = 3;
        osg::ref_ptr<osg::Geode> pNewGeode = dynamic_cast<osg::Geode*>(pGeode->clone(osg::CopyOp::DEEP_COPY_ALL));
        if (pNewGeode == NULL)
        {
            return;
        }

        SHAREGEODE shareGeode;
        if (m_pGeodeVisitor->getShareGeode(m_nCurrentLODIndex, shareGeode))
        {
            idDetail = shareGeode.idDetail;
        }
        else
        {
            changeGeode(pNewGeode);
            //CreateLodFromGeode(pNewGeode, 4.0);
			CreateLodFromGeode(pNewGeode, 0.5);	//- 厦门数据第三级简化0.5
            saveGeode(m_nCurrentLODIndex, getGeodID(), pNewGeode, idDetail);
        }

        detailInfo.idDetail  = idDetail;
        detailInfo.dMinRange = LODSeg.dLOD2;
        detailInfo.dMaxRange = dMaxDis < LODSeg.dLOD3 ? dMaxDis : LODSeg.dLOD3;
        m_vecDetail.push_back(detailInfo);
    }

    if (dMaxDis > LODSeg.dLOD3 && !bStopFlag)//四级LOD
    {
        m_nCurrentLODIndex = 4;
        osg::ref_ptr<osg::Geode> pNewGeode = dynamic_cast<osg::Geode*>(pGeode->clone(osg::CopyOp::DEEP_COPY_ALL));
        if (pNewGeode == NULL)
        {
            return;
        }

        SHAREGEODE shareGeode;
        if (m_pGeodeVisitor->getShareGeode(m_nCurrentLODIndex, shareGeode))
        {
            idDetail = shareGeode.idDetail;
        }
        else
        {
            changeGeode(pNewGeode);
            CreateLodFromGeode(pNewGeode, 16.0);
            saveGeode(m_nCurrentLODIndex, getGeodID(), pNewGeode, idDetail);
        }

        detailInfo.idDetail  = idDetail;
        detailInfo.dMinRange = LODSeg.dLOD3;
        detailInfo.dMaxRange = dMaxDis < LODSeg.dLOD4 ? dMaxDis : LODSeg.dLOD4;
        m_vecDetail.push_back(detailInfo);
    }

    RemoveTextureAttribute(pGeode);
}

// void ModelGroup::saveGeodes_5level_optimize(osg::ref_ptr<osg::Geode>& pGeode, double dMaxDis, const LODSEGMENT& LODSeg)
// {
//     ID idDetail;
//     DETAILINFO detailInfo;
//     bool bStopFlag = m_pGeodeVisitor->getModelBuilder()->getStopFlag();
// 
//     if (dMaxDis > 0.0 && !bStopFlag)//一级LOD
//     {
//         m_nCurrentLODIndex = 1;
//         osg::ref_ptr<osg::Geode> pNewGeode = dynamic_cast<osg::Geode*>(pGeode->clone(osg::CopyOp::DEEP_COPY_ALL));
//         if (pNewGeode == NULL)
//         {
//             return;
//         }
// 
//         SHAREGEODE shareGeode;
//         if (m_pGeodeVisitor->getShareGeode(m_nCurrentLODIndex, shareGeode))
//         {
//             idDetail = shareGeode.idDetail;
//         }
//         else
//         {
//             changeGeode(pNewGeode);
//             UseSpaliceTextureForLevel1(pNewGeode);
//             saveGeode(m_nCurrentLODIndex, getGeodID(), pNewGeode, idDetail);
//         }
// 
//         detailInfo.idDetail  = idDetail;
//         detailInfo.dMinRange = 0.0;
//         detailInfo.dMaxRange = dMaxDis < LODSeg.dLOD1 ? dMaxDis : LODSeg.dLOD1;
//         m_vecDetail.push_back(detailInfo);
//     }
// 
//     if (dMaxDis > LODSeg.dLOD1 && !bStopFlag)//二级LOD
//     {
//         m_nCurrentLODIndex = 2;
//         osg::ref_ptr<osg::Geode> pNewGeode = dynamic_cast<osg::Geode*>(pGeode->clone(osg::CopyOp::DEEP_COPY_ALL));
//         if (pNewGeode == NULL)
//         {
//             return;
//         }
//         SHAREGEODE shareGeode;
//         if (m_pGeodeVisitor->getShareGeode(m_nCurrentLODIndex, shareGeode))
//         {
//             idDetail = shareGeode.idDetail;
//         }
//         else
//         {
//             changeGeode(pNewGeode);
//             CreateLodFromGeode(pNewGeode, 1.0);
//             saveGeode(m_nCurrentLODIndex, getGeodID(), pNewGeode, idDetail);
//         }
// 
//         detailInfo.idDetail  = idDetail;
//         detailInfo.dMinRange = LODSeg.dLOD1;
//         detailInfo.dMaxRange = dMaxDis < LODSeg.dLOD2 ? dMaxDis : LODSeg.dLOD2;
//         m_vecDetail.push_back(detailInfo);
//     }
// 
//     if (dMaxDis > LODSeg.dLOD2 && !bStopFlag)//三级LOD
//     {
//         m_nCurrentLODIndex = 3;
//         osg::ref_ptr<osg::Geode> pNewGeode = dynamic_cast<osg::Geode*>(pGeode->clone(osg::CopyOp::DEEP_COPY_ALL));
//         if (pNewGeode == NULL)
//         {
//             return;
//         }
//         SHAREGEODE shareGeode;
//         if (m_pGeodeVisitor->getShareGeode(m_nCurrentLODIndex, shareGeode))
//         {
//             idDetail = shareGeode.idDetail;
//         }
//         else
//         {
//             changeGeode(pNewGeode);
//             CreateLodFromGeode(pNewGeode, 1.0);
//             saveGeode(m_nCurrentLODIndex, getGeodID(), pNewGeode, idDetail);
//         }
// 
//         detailInfo.idDetail  = idDetail;
//         detailInfo.dMinRange = LODSeg.dLOD2;
//         detailInfo.dMaxRange = dMaxDis < LODSeg.dLOD3 ? dMaxDis : LODSeg.dLOD3;;
//         m_vecDetail.push_back(detailInfo);
//     }
// 
//     if (dMaxDis > LODSeg.dLOD3 && !bStopFlag)//四级LOD
//     {
//         m_nCurrentLODIndex = 4;
//         osg::ref_ptr<osg::Geode> pNewGeode = dynamic_cast<osg::Geode*>(pGeode->clone(osg::CopyOp::DEEP_COPY_ALL));
//         if (pNewGeode == NULL)
//         {
//             return;
//         }
//         
//         SHAREGEODE shareGeode;
//         if (m_pGeodeVisitor->getShareGeode(m_nCurrentLODIndex, shareGeode))
//         {
//             idDetail = shareGeode.idDetail;
//         }
//         else
//         {
//             changeGeode(pNewGeode);
//             CreateLodFromGeode(pNewGeode, 4.0);
//             saveGeode(m_nCurrentLODIndex, getGeodID(), pNewGeode, idDetail);
//         }
// 
//         detailInfo.idDetail  = idDetail;
//         detailInfo.dMinRange = LODSeg.dLOD3;
//         detailInfo.dMaxRange = dMaxDis < LODSeg.dLOD4 ? dMaxDis : LODSeg.dLOD4;
//         m_vecDetail.push_back(detailInfo);
//     }
// 
//     if (dMaxDis > LODSeg.dLOD4 && !bStopFlag)//五级LOD
//     {
//         m_nCurrentLODIndex = 5;
//         osg::ref_ptr<osg::Geode> pNewGeode = dynamic_cast<osg::Geode*>(pGeode->clone(osg::CopyOp::DEEP_COPY_ALL));
//         if (pNewGeode == NULL)
//         {
//             return;
//         }
//         
//         SHAREGEODE shareGeode;
//         if (m_pGeodeVisitor->getShareGeode(m_nCurrentLODIndex, shareGeode))
//         {
//             idDetail = shareGeode.idDetail;
//         }
//         else
//         {
//             changeGeode(pNewGeode);
//             CreateLodFromGeode(pNewGeode, 16.0);
//             saveGeode(m_nCurrentLODIndex, getGeodID(), pNewGeode, idDetail);
//         }
// 
//         detailInfo.idDetail  = idDetail;
//         detailInfo.dMinRange = LODSeg.dLOD4;
//         detailInfo.dMaxRange = dMaxDis < LODSeg.dLOD5 ? dMaxDis : LODSeg.dLOD5;
//         m_vecDetail.push_back(detailInfo);
//     }
// 
//     RemoveTextureAttribute(pGeode);
// }

std::string ModelGroup::getTextureName(const osg::ref_ptr<osg::Geometry>& geometry)
{
    std::string strTextureName("");

    osg::StateSet* stateSet = geometry->getStateSet();
    if (stateSet != NULL)
    {
        osg::StateSet::TextureAttributeList texattriList = stateSet->getTextureAttributeList();
        for (size_t i=0; i<texattriList.size(); i++)
        {
            osg::StateSet::AttributeList attriList = texattriList[i];
            osg::StateSet::AttributeList::iterator itrattri = attriList.begin();
            for (; itrattri!=attriList.end(); itrattri++)
            {
                osg::StateAttribute::TypeMemberPair typePair = itrattri->first;
                if (typePair.first != osg::StateAttribute::TEXTURE) continue;

                osg::StateSet::RefAttributePair attriPair = itrattri->second;
                osg::ref_ptr<osg::StateAttribute> stateAttri = attriPair.first;
                osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(stateAttri.get());
                if (texture == NULL) continue;
                strTextureName = texture->getImage()->getFileName();
                break;
            }
            break;
        }
    } 

    return strTextureName;
}

bool ModelGroup::getUniqueVertexs( osg::ref_ptr<osg::Geometry>& geometry )
{
    vector<DYVert> vecDYVert,vecDYVertOut;
    AnalysisVertex(geometry, vecDYVert);
    if (vecDYVert.empty())
    {
        return false;
    }

    DYIntList indexList,NewIndexList;
    AnalysisVertIndex(geometry, indexList);
    if (indexList.IsEmpty())
    {
        return false;
    }

    DYFaceList pFaceList;
    for (int j=0; j<indexList.GetCount(); j++)
    {
        DYFace aFace(vecDYVert[indexList[j]], vecDYVert[indexList[j+1]], vecDYVert[indexList[j+2]]);j += 2;
        pFaceList.Append(aFace);
    }

    vector<pair<int, DYIntList> > vecModeAndIndexList;
    DYFaceList::FromDelRepeat(pFaceList, vecDYVertOut, vecModeAndIndexList);

    indexList = vecModeAndIndexList[0].second;

    osg::Vec3Array* coords = (osg::Vec3Array*)(geometry->getVertexArray());
    if (coords != NULL) (*coords).clear();

    osg::Vec2Array* tcoords = (osg::Vec2Array*)(geometry->getTexCoordArray(0));
    if (tcoords != NULL) (*tcoords).clear();

    osg::Vec3Array* ncoords = (osg::Vec3Array*)(geometry->getNormalArray());
    if (ncoords != NULL) (*ncoords).clear();

    for (int j=0; j<vecDYVertOut.size(); j++)
    {

        if (coords != NULL) (*coords).push_back(osg::Vec3(vecDYVertOut[j].Point.X, vecDYVertOut[j].Point.Y,vecDYVertOut[j].Point.Z));
        if (tcoords != NULL) (*tcoords).push_back(osg::Vec2(vecDYVertOut[j].Texel.X , vecDYVertOut[j].Texel.Y));
        if (ncoords != NULL) (*ncoords).push_back(osg::Vec3(vecDYVertOut[j].Normal.X, vecDYVertOut[j].Normal.Y,vecDYVertOut[j].Normal.Z));
    }

    for (int i=0; i<geometry->getNumPrimitiveSets(); i=0)
    {
        geometry->removePrimitiveSet(i);
    }

    osg::ref_ptr<osg::DrawElementsUInt> elements = new osg::DrawElementsUInt(4);

    int nCnt = indexList.GetCount();
    elements->reserveElements(nCnt);
    for (unsigned int k=0; k<nCnt; k++) 
    {
        elements->addElement(indexList[k]);
    }
    geometry->addPrimitiveSet(elements.get());

    return true;
}

//去重源代码
//bool ModelGroup::getUniqueVertexs( osg::ref_ptr<osg::Geometry>& geometry )
//{
//    vector<DYVert> vecDYVertOut;
//    AnalysisVertex(geometry, vecDYVertOut);
//
//    DYIntList indexList;
//    AnalysisVertIndex(geometry, indexList);
//
//    for (int i=0; i<vecDYVertOut.size(); i++)
//    {
//        //- 如果该点是无效点，表示已经比较过
//        if (!vecDYVertOut[i].valid)
//        {
//            continue;
//        }
//        for (int j=i+1; j<vecDYVertOut.size(); j++)
//        {
//            //- 首先找到相同点，将后面所有与该点相同的点设置为无效点，
//            //- 只保留最前面一个点为有效点，并将无效点的索引值指向第一个有效点的索引
//            if (vecDYVertOut[i].Point==vecDYVertOut[j].Point && vecDYVertOut[i].Texel==vecDYVertOut[j].Texel && vecDYVertOut[i].Normal == vecDYVertOut[j].Normal)
//            {
//                vecDYVertOut[j].valid = false;
//                for (int m = 0; m < indexList.GetCount(); m++)
//                {
//                    if (indexList[m] == j)
//                    {
//                        indexList[m] = i;
//                    }
//                }
//
//            }
//        }
//    }
//
//    for (int i=0; i<vecDYVertOut.size(); i++)
//    {
//        if (!vecDYVertOut[i].valid)
//        {
//            vector<DYVert>::iterator itr = vecDYVertOut.begin() + i;
//            itr = vecDYVertOut.erase(itr);
//
//            for (int j=0; j<indexList.GetCount(); j++)
//            {
//                //- 减少一个顶点，所有大于当前顶点对应索引的索引序列自减一次
//                if (indexList[j] > i)
//                {
//                    indexList[j] -= 1;
//                }
//            }
//
//            //- 如果连续的顶点都是无效点
//            if (itr!=vecDYVertOut.end() && !itr->valid)  i--;
//
//            continue;
//        }
//    }
//
//    //设置信息回去
//    osg::Vec3Array* pVectexs = (osg::Vec3Array*)(geometry->getVertexArray());
//    if (pVectexs == NULL)
//    {
//        return false;
//    }
//
//    //法线信息
//    osg::Vec3Array* pNormals = (osg::Vec3Array*)(geometry->getNormalArray());
//    if (pNormals == NULL)
//    {
//        return false;
//    }
//
//    //纹理信息
//    osg::Geometry::ArrayDataList TexCoor = geometry->getTexCoordArrayList();
//    osg::Vec2Array* pTexCoor  = (osg::Vec2Array*)(geometry->getTexCoordArray(0));
//
//    (*pVectexs).clear();
//    (*pNormals).clear();
//
//    if (pTexCoor != NULL)
//    {
//        (*pTexCoor).clear();
//    }
//
//    for (int i  = 0; i < vecDYVertOut.size();i++)
//    {
//        float xVec = vecDYVertOut[i].Point.X;
//        float yVec = vecDYVertOut[i].Point.Y;
//        float zVec = vecDYVertOut[i].Point.Z;
//        (*pVectexs).push_back(osg::Vec3(xVec,yVec,zVec));
//
//        float xNor = vecDYVertOut[i].Normal.X;
//        float yNor = vecDYVertOut[i].Normal.Y;
//        float zNor = vecDYVertOut[i].Normal.Z;
//        (*pNormals).push_back(osg::Vec3(xNor,yNor,zNor));
//
//        if (pTexCoor != NULL)
//        {
//            (*pTexCoor).push_back(osg::Vec2(vecDYVertOut[i].Texel.X,vecDYVertOut[i].Texel.Y));
//        }
//    }
//
//    //设回索引数组，要将之前的primitiveset置为空
//    for (unsigned int z = 0; z < geometry->getNumPrimitiveSets();z = 0)
//    {
//        geometry->removePrimitiveSet(z);
//    }
//
//    osg::ref_ptr<osg::DrawElementsUInt> pTriangles = new osg::DrawElementsUInt(4);
//    for (int j = 0; j < indexList.GetCount(); j++)
//    {
//        pTriangles->push_back(indexList[j]);
//    }
//
//    geometry->addPrimitiveSet(pTriangles);
//    
//    return true;
//}

bool ModelGroup::changeGeode( osg::ref_ptr<osg::Geode>& pNewGeode)
{
    if (!m_pGeodeVisitor->getShareModel())
    {
        for(size_t m = 0; m < pNewGeode->getNumDrawables(); m++)
        {
            osg::Vec3Array* pVectexs = dynamic_cast<osg::Vec3Array*>( pNewGeode->getDrawable(m)->asGeometry()->getVertexArray());
            if (pVectexs == NULL)
            {
                continue;
            }

            for(size_t n = 0; n < pVectexs->getNumElements(); n++)
            {
                (*pVectexs)[n] = m_pMatrixTransform->getMatrix().preMult((*pVectexs)[n]);
            }

            osg::Vec3Array* pNormals = dynamic_cast<osg::Vec3Array*>(pNewGeode->getDrawable(m)->asGeometry()->getNormalArray());

            if (pNormals == NULL)
            {
                pNewGeode->getDrawable(m)->asGeometry()->dirtyBound();

                continue;
            }

            for(size_t n = 0; n < pNormals->getNumElements(); n++)
            {
                (*pNormals)[n] = m_pMatrixTransform->getMatrix().preMult((*pNormals)[n]) - m_pMatrixTransform->getMatrix().preMult(osg::Vec3d(0.0, 0.0, 0.0));
                (*pNormals)[n].normalize();
            }

            pNewGeode->getDrawable(m)->asGeometry()->dirtyBound();
        }
    }
    return true;
}

bool ModelGroup::VertexRender(osg::ref_ptr<osg::Drawable>& drawAble)
{
	osg::ref_ptr<osg::Geometry> geometry = drawAble->asGeometry();
	if (geometry == NULL)
	{
		return false;
	}

	osg::ref_ptr<osg::StateSet> pStateSet = drawAble->getStateSet();
	if (NULL == pStateSet)
	{
		return false;
	}

	//获取纹理图片颜色值，并摄入顶点颜色数组
	std::vector<osg::Vec4> vecColor;
	unsigned int nTexCount = pStateSet->getNumTextureAttributeLists();
	for(unsigned int j = 0; j < nTexCount; j++)
	{
		osg::ref_ptr<osg::StateAttribute> pAttribute = pStateSet->getTextureAttribute(j, osg::StateAttribute::TEXTURE);
		osg::Texture2D* pTexture2D = dynamic_cast<osg::Texture2D*>(pAttribute.get());
		if(pTexture2D == NULL)
		{
			continue;
		}

		osg::ref_ptr<osg::Image> pImage = pTexture2D->getImage();
		if (pImage == NULL)
		{
			continue;
		}

		osg::Array* texcoordArray = geometry->getTexCoordArray(0);
		if (texcoordArray != NULL)
		{
			osg::Vec2Array* texcoords = dynamic_cast<osg::Vec2Array*>(texcoordArray);
			std::vector<osg::Vec2>::iterator iter_tex = texcoords->begin();
			for (;iter_tex != texcoords->end();iter_tex++)
			{
				if (!pImage->valid())
				{
					continue;
				}
				iter_tex->x() = fabs(iter_tex->x());
				iter_tex->y() = fabs(iter_tex->y());

				osg::Vec4 Tempcolor  = pImage->getColor(*iter_tex);
				Tempcolor.a() = 0.75;
				vecColor.push_back(Tempcolor);
			}
		}

		pImage = NULL;
		pTexture2D->setImage(pImage);
	}

	osg::Vec4Array* colors = (osg::Vec4Array*)(geometry->getColorArray());
	if (colors != NULL)
	{
		colors->clear();
	}
	else
	{
		colors = new osg::Vec4Array;
	}

	osg::Vec4 Tempcolor ;
	std::vector<osg::Vec4>::iterator iter_color = vecColor.begin();
	for (;iter_color != vecColor.end();iter_color++)
	{
		Tempcolor.r() += iter_color->r();
		Tempcolor.g() += iter_color->g();
		Tempcolor.b() += iter_color->b();
		Tempcolor.a() += iter_color->a();
	}

	Tempcolor.r() = Tempcolor.r() / vecColor.size();
	Tempcolor.g() = Tempcolor.g() / vecColor.size();
	Tempcolor.b() = Tempcolor.b() / vecColor.size();
	Tempcolor.a() = Tempcolor.a() / vecColor.size();

	osg::Vec3Array* coordstmp = (osg::Vec3Array*)(geometry->getVertexArray());
	if (coordstmp!=NULL)
	{
		std::vector<osg::Vec3 >::iterator iter_ver = coordstmp->begin () ;
		for (; iter_ver!=coordstmp->end(); iter_ver++)
		{
			(*colors).push_back(Tempcolor);
		}
	}

	//- 以点为单位进行着色
	//vecColor.clear();
	//vecColor.push_back(Tempcolor);

	//geometry->setColorArray(vecColor);
	//geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

	//std::vector<osg::Vec4>::iterator iter_ver = vecColor.begin () ;
	//for (; iter_ver!=vecColor.end(); iter_ver++)
	//{
	//    osg::Vec4 v(*iter_ver);
	//    (*colors).push_back( osg::Vec4(v.r(), v.g(), v.b(),v.a()) );
	//}

	geometry->setColorArray(colors);
	geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	//删除材质，设置混合
	osg::ref_ptr<osg::Material> pMaterial = dynamic_cast<osg::Material *>(pStateSet->getAttribute(osg::StateAttribute::MATERIAL));
	if (pMaterial != NULL)
	{
		pStateSet->removeAttribute(pMaterial);
	}

	pStateSet->getTextureAttributeList().clear();
	drawAble->asGeometry()->getTexCoordArrayList().clear();

	pStateSet->setMode(GL_BLEND,osg::StateAttribute::ON);
	pStateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	return true;
}

