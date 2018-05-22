#include "Detail.h"

#include <IDProvider/Definer.h>
#include <common/Common.h>
#include <OpenSP/sp.h>
#include "Hole.h"

#include <osg/MatrixTransform>
#include <osgUtil/CommonModelCreater.h>
#include <osg/SharedObjectPool>
#include <osgDB/ReadFile>
#include <osg/Texture2D>

#include "BubbleTextDetail.h"
#include "CubeDetail.h"
#include "CylinderDetail.h"
#include "DynFaceDetail.h"
#include "DynImageDetail.h"
#include "DynLineDetail.h"
#include "DynModelDetail.h"
#include "DynPointDetail.h"
#include "PipeConnectorDetail.h"
#include "RectPipeConnectorDetail.h"
#include "PolygonPipeConnectorDetail.h"
#include "PolygonDetail.h"
#include "PrismDetail.h"
#include "PyramidDetail.h"
#include "SectorDetail.h"
#include "SphereDetail.h"
#include "StaticModelDetail.h"
#include "RoundTableDetail.h"
#include "DynPointCloudDetail.h"

#include "PointParameter.h"
#include "LineParameter.h"
#include "FaceParameter.h"

namespace param
{


IDetail *createDetail(const std::string &strDetail, unsigned int nDatasetCode)
{
    OpenSP::sp<IDetail> pDetail = NULL;

    if(strDetail.compare(STATIC_DETAIL) == 0)
    {
        pDetail = new StaticModelDetail(nDatasetCode);
    }
    else if(strDetail.compare(IMAGE_DETAIL) == 0)
    {
        pDetail = new DynImageDetail(nDatasetCode);
    }
    else if(strDetail.compare(POINT_DETAIL) == 0)
    {
        pDetail = new DynPointDetail(nDatasetCode);
    }
    else if(strDetail.compare(LINE_DETAIL) == 0)
    {
        pDetail = new DynLineDetail(nDatasetCode);
    }
    else if(strDetail.compare(FACE_DETAIL) == 0)
    {
        pDetail = new DynFaceDetail(nDatasetCode);
    }
    else if(strDetail.compare(CUBE_DETAIL) == 0)
    {
        pDetail = new CubeDetail(nDatasetCode);
    }
    else if(strDetail.compare(CYLINDER_DETIAL) == 0)
    {
        pDetail = new CylinderDetail(nDatasetCode);
    }
    else if(strDetail.compare(SPHERE_DETAIL) == 0)
    {
        pDetail = new SphereDetail(nDatasetCode);
    }
    else if(strDetail.compare(PRISM_DETAIL) == 0)
    {
        pDetail = new PrismDetail(nDatasetCode);
    }
    else if(strDetail.compare(PYRAMID_DETAIL) == 0)
    {
        pDetail = new PyramidDetail(nDatasetCode);
    }
    else if(strDetail.compare(SECTOR_DETAIL) == 0)
    {
        pDetail = new SectorDetail(nDatasetCode);
    }
    else if(strDetail.compare(PIPECONNECTOR_DETAIL) == 0)
    {
        pDetail = new PipeConnectorDetail(nDatasetCode);
    }
    else if(strDetail.compare(RECT_PIPECONNECTOR_DETAIL) == 0)
    {
        pDetail = new RectPipeConnectorDetail(nDatasetCode);
    }
    else if(strDetail.compare(POLYGON_PIPECONNECTOR_DETAIL) == 0)
    {
        pDetail = new PolygonPipeConnectorDetail(nDatasetCode);
    }
    else if(strDetail.compare(BUBBLETEXT_DETAIL) == 0)
    {
        pDetail = new BubbleTextDetail(nDatasetCode);
    }
    else if(strDetail.compare(POLYGON_DETAIL) == 0)
    {
        pDetail = new PolygonDetail(nDatasetCode);
    }
    else if(strDetail.compare(ROUNDTABLE_DETAIL) == 0)
    {
        pDetail = new RoundTableDetail(nDatasetCode);
    }
	else if(strDetail.compare(POINTCLOUD_DETAIL) == 0)
	{
		pDetail = new DynPointCloudDetail(nDatasetCode);
	}

    return pDetail.release();
}

IDetail *createDetail(const ID &id)
{
    OpenSP::sp<IDetail> pDetail = NULL;

#if 0
    static bool b = true;
    if(b)
    {
        b = false;
        std::cout << "BubbleTextDetail \t" << sizeof(BubbleTextDetail) << std::endl;
        std::cout << "CubeDetail \t" << sizeof(CubeDetail) << std::endl;
        std::cout << "CylinderDetail \t" << sizeof(CylinderDetail) << std::endl;
        std::cout << "DynFaceDetail \t" << sizeof(DynFaceDetail) << std::endl;
        std::cout << "DynImageDetail \t" << sizeof(DynImageDetail) << std::endl;
        std::cout << "DynLineDetail \t" << sizeof(DynLineDetail) << std::endl;
        std::cout << "DynModelDetail \t" << sizeof(DynModelDetail) << std::endl;
        std::cout << "DynPointDetail \t" << sizeof(DynPointDetail) << std::endl;
        std::cout << "PipeConnectorDetail \t" << sizeof(PipeConnectorDetail) << std::endl;
        std::cout << "RectPipeConnectorDetail \t" << sizeof(RectPipeConnectorDetail) << std::endl;
        std::cout << "PolygonPipeConnectorDetail \t" << sizeof(PolygonPipeConnectorDetail) << std::endl;
        std::cout << "PolygonDetail \t" << sizeof(PolygonDetail) << std::endl;
        std::cout << "PrismDetail \t" << sizeof(PrismDetail) << std::endl;
        std::cout << "PyramidDetail \t" << sizeof(PyramidDetail) << std::endl;
        std::cout << "RoundTableDetail \t" << sizeof(RoundTableDetail) << std::endl;
        std::cout << "SectorDetail \t" << sizeof(SectorDetail) << std::endl;
        std::cout << "SphereDetail \t" << sizeof(SphereDetail) << std::endl;
        std::cout << "StaticModelDetail \t" << sizeof(StaticModelDetail) << std::endl;

        std::cout << "PointParameter \t" << sizeof(PointParameter) << std::endl;
        std::cout << "LineParameter \t" << sizeof(LineParameter) << std::endl;
        std::cout << "FaceParameter \t" << sizeof(FaceParameter) << std::endl;
        std::cout << "Hole \t" << sizeof(Hole) << std::endl;

        std::cout << "CreationInfo \t" << sizeof(Detail::CreationInfo) << std::endl;
        std::cout << "PointCreationInfo \t" << sizeof(Detail::PointCreationInfo) << std::endl;
        std::cout << "PolyCreationInfo \t" << sizeof(Detail::PolyCreationInfo) << std::endl;
    }
#endif

    if(id.ObjectID.m_nType == DETAIL_PIPE_CONNECTOR_ID)
    {
        pDetail = new PipeConnectorDetail;
    }
    if(id.ObjectID.m_nType == DETAIL_RECT_PIPE_CONNECTOR_ID)
    {
        pDetail = new RectPipeConnectorDetail;
    }
    if(id.ObjectID.m_nType == DETAIL_POLYGON_PIPE_CONNECTOR_ID)
    {
        pDetail = new PolygonPipeConnectorDetail;
    }
    else if(id.ObjectID.m_nType == DETAIL_CUBE_ID)
    {
        pDetail = new CubeDetail;
    }
    else if(id.ObjectID.m_nType == DETAIL_CYLINDER_ID)
    {
        pDetail = new CylinderDetail;
    }
    else if(id.ObjectID.m_nType == DETAIL_PRISM_ID)
    {
        pDetail = new PrismDetail;
    }
    else if(id.ObjectID.m_nType == DETAIL_PYRAMID_ID)
    {
        pDetail = new PyramidDetail;
    }
    else if(id.ObjectID.m_nType == DETAIL_SPHERE_ID)
    {
        pDetail = new SphereDetail;
    }
    else if(id.ObjectID.m_nType == DETAIL_SECTOR_ID)
    {
        pDetail = new SectorDetail;
    }
    else if(id.ObjectID.m_nType == DETAIL_STATIC_MODEL_ID)
    {
        pDetail = new StaticModelDetail;
    }
    else if(id.ObjectID.m_nType == DETAIL_DYN_POINT_ID)
    {
        pDetail = new DynPointDetail(id);
    }
    else if(id.ObjectID.m_nType == DETAIL_DYN_LINE_ID)
    {
        pDetail = new DynLineDetail(id);
    }
    else if(id.ObjectID.m_nType == DETAIL_DYN_FACE_ID)
    {
        pDetail = new DynFaceDetail(id);
    }
    else if(id.ObjectID.m_nType == DETAIL_DYN_IMAGE_ID)
    {
        pDetail = new DynImageDetail;
    }
    else if(id.ObjectID.m_nType == DETAIL_BUBBLE_TEXT_ID)
    {
        pDetail = new BubbleTextDetail;
    }
    else if(id.ObjectID.m_nType == DETAIL_POLYGON_ID)
    {
        pDetail = new PolygonDetail;
    }
    else if(id.ObjectID.m_nType == DETAIL_ROUND_TABLE_ID)
    {
        pDetail = new RoundTableDetail;
    }
	else if(id.ObjectID.m_nType == DETAIL_DYN_POINT_CLOUD_ID)
	{
		pDetail = new DynPointCloudDetail;
	}

    return pDetail.release();
}


bool Detail::fromBson(bson::bsonDocument &bsonDoc)
{
    bson::bsonBinaryEle *pBin = dynamic_cast<bson::bsonBinaryEle *>(bsonDoc.GetElement("ID"));
    if(pBin == NULL || pBin->BinDataLen() != sizeof(UINT_64) * 3)
    {
        return false;
    }

    bson::bsonBoolEle *pIsStretch = dynamic_cast<bson::bsonBoolEle *>(bsonDoc.GetElement("IsTexStretch"));
    if(pIsStretch == NULL)
    {
        m_bImageStretched = true;
    }
    else
    {
        m_bImageStretched = pIsStretch->BoolValue();
    }

    m_id = ID::genIDfromBinary(pBin->BinData(), pBin->BinDataLen());
    return true;
}


bool Detail::toBson(bson::bsonDocument &bsonDoc) const
{
    return bsonDoc.AddBinElement("ID", (void*)&m_id, sizeof(ID)) && bsonDoc.AddBoolElement("IsTexStretch", m_bImageStretched);
}


const ID &Detail::getID(void) const
{
    return m_id;
}


const osg::Texture *Detail::bindTexture(const ID &idImage, osg::StateSet *pStateSet) const
{
    if(!idImage.isValid())  return NULL;
    if(idImage.ObjectID.m_nType != SHARE_IMAGE_ID && idImage.ObjectID.m_nType != IMAGE_ID)
    {
        return NULL;
    }

    osg::ref_ptr<osg::Image> pImage = osgDB::readImageFile(idImage);
    if(!pImage.valid())
    {
        return NULL;
    }

    osg::ref_ptr<osg::Texture> pTexture;
    if(idImage.ObjectID.m_nType == SHARE_IMAGE_ID)
    {
        osg::SharedObjectPool *pPool = osg::SharedObjectPool::instance();
        if(!pPool->findObject(pImage.get(), pTexture))
        {
            //printf("未查找到影像--%s！, 类型为--%d\n", strImgName.c_str(), idImage.ObjectID.m_nType);
            pTexture = new osg::Texture2D(pImage.get());
            pPool->addTexture(pImage.get(), pTexture);
        }
    }

    if(!pTexture.valid())
    {
        pTexture = new osg::Texture2D(pImage.get());
    }

    pTexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
    pTexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);

    if(pStateSet)
    {
        pStateSet->setTextureAttributeAndModes(0u, pTexture.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    }

    return pTexture.release();
}

}