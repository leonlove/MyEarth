#include "GeodeVisitor.h"
#include "ModelGroup.h"
#include <osgDB/ReadFile>
#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>
#include <osg/Texture2D>
#include <osg/Drawable>
#include <windows.h>
#include <io.h>
#include <direct.h>


GeodeVisitor::GeodeVisitor(ModelBuilder* pModelBuilder)
: osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
, m_nGeodesCount(0)
, m_bGetNum(false)
, m_nDataSetCode(101)
, m_bShareGeode(false)
, m_bShareModel(true)
, m_bMultiRefPoint(false)
, m_pModelBuilder(pModelBuilder)
, m_bIsMultiPathList(false)
{
    m_LodSegment = pModelBuilder->getLodSegment();
    m_dMaxScale = 0.03;
}


GeodeVisitor::~GeodeVisitor(void)
{
    m_ShareGeodes.clear();
    m_ShareImgMap.clear();
    m_setFinishGeode.clear();
}


void GeodeVisitor::setUseMultiRefPoint(bool bUse)
{
    m_bMultiRefPoint = bUse;
}


void GeodeVisitor::setTargetDB(deudbProxy::IDEUDBProxy *pDB)
{
    m_pTargetDB = pDB;
}


void GeodeVisitor::setDataSetCode(int nDataSetCode)
{
    m_nDataSetCode = nDataSetCode;
}


int GeodeVisitor::getDataSetCode(void) const
{
    return m_nDataSetCode;
}


void GeodeVisitor::setOffset(const osg::Vec3d& offset)
{
    m_offset = offset;
}


void GeodeVisitor::getOffset(osg::Vec3d& offset)
{
    offset = m_offset;
}


void GeodeVisitor::setSpatialRefInfo(const SpatialRefInfo& sri)
{
    m_sriInfo.m_dEastOffset      = sri.m_dEastOffset;
    m_sriInfo.m_dNorthOffset     = sri.m_dNorthOffset;
    m_sriInfo.m_dCentreLongitude = sri.m_dCentreLongitude;
    m_sriInfo.m_strCoordination  = sri.m_strCoordination;
    m_sriInfo.m_strProjection    = sri.m_strProjection;
}


void GeodeVisitor::getSpatialRefInfo(SpatialRefInfo& sri)
{
    sri.m_dEastOffset      = m_sriInfo.m_dEastOffset;
    sri.m_dNorthOffset     = m_sriInfo.m_dNorthOffset;
    sri.m_dCentreLongitude = m_sriInfo.m_dCentreLongitude;
    sri.m_strCoordination  = m_sriInfo.m_strCoordination;
    sri.m_strProjection    = m_sriInfo.m_strProjection;
}


void GeodeVisitor::setShareModel(bool bShareModel)
{
    m_bShareModel = bShareModel;
}


bool GeodeVisitor::getShareModel() const
{
    return m_bShareModel;
}


void GeodeVisitor::setGetNum(bool bGetNum)
{
    m_bGetNum = bGetNum;
    m_nGeodesCount = 0;

    m_models.clear();
    m_SphereVec.clear();
    m_ShareGeodes.clear();
    m_ShareImgMap.clear();
    m_setFinishGeode.clear();
}


int GeodeVisitor::getGeodesCount()
{
    return m_nGeodesCount;
}


void GeodeVisitor::setLODSegment(LODSEGMENT& LODSeg)
{
    m_LodSegment.dLOD1 = LODSeg.dLOD1;
    m_LodSegment.dLOD2 = LODSeg.dLOD2;
    m_LodSegment.dLOD3 = LODSeg.dLOD3;
    m_LodSegment.dLOD4 = LODSeg.dLOD4;
    m_LodSegment.dLOD5 = LODSeg.dLOD5;

    m_LodSegment.bOptimize          = LODSeg.bOptimize;
    m_LodSegment.nValidLodLevels    = LODSeg.nValidLodLevels;

    if (!(m_LodSegment.dLOD2 > 0))
    {
        m_LodSegment.nValidLodLevels = 1;
    }
	else if (!(m_LodSegment.dLOD3 > 0))
	{
		m_LodSegment.nValidLodLevels = 2;
	}
	else if (!(m_LodSegment.dLOD4 > 0))
	{
		m_LodSegment.nValidLodLevels = 3;
	}
    else if (!(m_LodSegment.dLOD5 > 0))
    {
        m_LodSegment.nValidLodLevels = 4;
    }
    else
    {
        m_LodSegment.nValidLodLevels = 5;
    }
}


void GeodeVisitor::getLODSegment(LODSEGMENT& LODSeg)
{
    LODSeg.dLOD1 = m_LodSegment.dLOD1;
    LODSeg.dLOD2 = m_LodSegment.dLOD2;
    LODSeg.dLOD3 = m_LodSegment.dLOD3;
    LODSeg.dLOD4 = m_LodSegment.dLOD4;
    LODSeg.dLOD5 = m_LodSegment.dLOD5;

    LODSeg.bOptimize        = m_LodSegment.bOptimize;
    LODSeg.nValidLodLevels  = m_LodSegment.nValidLodLevels;
}


bool GeodeVisitor::getShareGeode(const int nLODIndex, SHAREGEODE &node)
{
    if (static_cast<int>(m_ShareGeodes.size()) < nLODIndex)
    {
        return false;
    }

    node = m_ShareGeodes[nLODIndex - 1];

    return true;
}


void GeodeVisitor::addShareGeode(const int nLODIndex, const SHAREGEODE &node)
{
    if (static_cast<int>(m_ShareGeodes.size()) < nLODIndex)
    {
        m_ShareGeodes.push_back(node);
    }

    return;
}


bool GeodeVisitor::getShareImg(const int nLODIndex, const std::string& strName, ID& idLOD)
{
    if (nLODIndex > 5)
    {
        return false;
    }

    ShareImgMap::iterator it = m_ShareImgMap.find(strName);
    if (it != m_ShareImgMap.end())
    {
        if (it->second.setLODIndex.find(nLODIndex) == it->second.setLODIndex.end())
        {
            return false;
        }
        else
        {
            switch (nLODIndex)
            {
            case 1:
                idLOD = it->second.idLOD1;
                break;
            case 2:
                idLOD = it->second.idLOD2;
                break;
            case 3:
                idLOD = it->second.idLOD3;
                break;
            case 4:
                idLOD = it->second.idLOD4;
                break;
            case 5:
                idLOD = it->second.idLOD5;
                break;
            default:
                break;
            }

            return true;
        }
    }

    return false;
}


void GeodeVisitor::addShareImg(const std::string& strName, const int nLODIndex, const ID& idLOD)
{
    if (nLODIndex > 5)
    {
        return;
    }

    ShareImgMap::iterator it = m_ShareImgMap.find(strName);
    if (it != m_ShareImgMap.end())
    {
        if (it->second.setLODIndex.find(nLODIndex) != it->second.setLODIndex.end())
        {
            return;
        }

        it->second.setLODIndex.insert(nLODIndex);
        switch (nLODIndex)
        {
        case 1:
            it->second.idLOD1 = idLOD;
            break;
        case 2:
            it->second.idLOD2 = idLOD;
            break;
        case 3:
            it->second.idLOD3 = idLOD;
            break;
        case 4:
            it->second.idLOD4 = idLOD;
            break;
        case 5:
            it->second.idLOD5 = idLOD;
            break;
        default:
            break;
        }
    }
    else
    {
        SHAREIMG img;
        img.setLODIndex.insert(nLODIndex);
        switch (nLODIndex)
        {
        case 1:
            img.idLOD1 = idLOD;
            break;
        case 2:
            img.idLOD2 = idLOD;
            break;
        case 3:
            img.idLOD3 = idLOD;
            break;
        case 4:
            img.idLOD4 = idLOD;
            break;
        case 5:
            img.idLOD5 = idLOD;
            break;
        default:
            break;
        }

        m_ShareImgMap.insert(std::pair<std::string, SHAREIMG>(strName, img));
    }

    return;
}


void GeodeVisitor::addModel(const ID& modelId, const osg::BoundingSphere& sphere)
{
    m_models.push_back(modelId);
    m_SphereVec.push_back(sphere);

    getModelBuilder()->getVecModelID().push_back(modelId);
    getModelBuilder()->getVecBoundingSphere().push_back(sphere);
}


void GeodeVisitor::writeConfigurationFile(const std::string& fileName)
{
    ID id = ID::genNewID();
    id.ObjectID.m_nDataSetCode = 6;
    id.ObjectID.m_nType        = CULTURE_LAYER_ID;

    std::string strLayerName = osgDB::getSimpleFileName(fileName);
    strLayerName = osgDB::getNameLessExtension(strLayerName);

    bson::bsonDocument bsonDoc;
    bsonDoc.AddStringElement("Name", strLayerName.c_str());

    osg::BoundingSphere box;
    for (size_t i = 0; i < m_SphereVec.size(); i++)
    {
        box.expandBy(m_SphereVec[i]);
    }

    osg::Vec3d center = box.center();
    bson::bsonArrayEle *pBoundingSphere = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.AddArrayElement("BoundingSphere"));
    pBoundingSphere->AddDblElement(center.x());
    pBoundingSphere->AddDblElement(center.y());
    pBoundingSphere->AddDblElement(center.z());
    pBoundingSphere->AddDblElement(box.radius());

    bson::bsonArrayEle *pChildrenID = dynamic_cast<bson::bsonArrayEle *>(bsonDoc.AddArrayElement("ChildrenID"));
    for (size_t j = 0; j < m_models.size(); j++)
    {
        pChildrenID->AddBinElement(&(m_models[j]), sizeof(m_models[j]));
    }

    bson::bsonStream bstream;
    bsonDoc.Write(&bstream);

    m_pTargetDB->addBlock(id, bstream.Data(), bstream.DataLen());

    std::ofstream file;
    file.open(fileName+".dscfg", std::ios::out | std::ios::trunc);
    file<<"{\"IDList\":[\""<<id.toString()<<"\"]}";
    file.close();
}


void GeodeVisitor::countShareImg(const osg::Geode& node)
{
    unsigned int numDrawable = node.getNumDrawables();
    for (unsigned int i = 0; i < numDrawable; i++)
    {
        if (m_pModelBuilder->getStopFlag()) return;

        const osg::Drawable* pDrawable = node.getDrawable(i);
        const osg::StateSet* pStateSet = pDrawable->getStateSet();
        if (pStateSet == NULL)
        {
            continue;
        }

        unsigned int nTexCount = pStateSet->getNumTextureAttributeLists();
        for(unsigned int j = 0; j < nTexCount; j++)
        {
            if (m_pModelBuilder->getStopFlag()) return;

            osg::StateAttribute* pAttribute = (osg::StateAttribute*)pStateSet->getTextureAttribute(j, osg::StateAttribute::TEXTURE);
            osg::Texture2D* pTexture2D = dynamic_cast<osg::Texture2D*>(pAttribute);
            if(pTexture2D == NULL)
            {
                continue;
            }

            osg::Image* pImage = pTexture2D->getImage();
            if (pImage == NULL)
            {
                continue;
            }

            //============================================================
            if (m_LodSegment.bOptimize)
            {
                const GLenum ePixelFormat = pImage->getPixelFormat();
                if (ePixelFormat==GL_RGBA || ePixelFormat==GL_BGRA)
                {
                    ExportOsgImage(pImage, true);
                }
                else
                {
                    bool bMultiTextureFlag = false;
                    const osg::Geometry* geometry = pDrawable->asGeometry();
                    if (geometry != NULL)
                    {
                        const osg::Array* texcoordArray = geometry->getTexCoordArray(0);
                        if (texcoordArray != NULL)
                        {
                            const osg::Vec2Array* texcoords = dynamic_cast<const osg::Vec2Array*>(texcoordArray);

                            for (int j=0; j<texcoords->size(); j++)
                            {
                                double dx = (*texcoords)[j].x();
                                double dy = (*texcoords)[j].y();
                                if ( !(dx>=0.0 && dx<=1.0) || !(dy>=0.0 && dy<=1.0) )
                                {
                                    bMultiTextureFlag = true;
                                    break;
                                }
                            }
                        }

                        ExportOsgImage(pImage, bMultiTextureFlag);
                    }
                }
                //- end if
            }
			//============================================================

            if (m_ImgRefCount.find(pImage->getFileName()) == m_ImgRefCount.end())
            {
                m_ImgRefCount.insert(std::pair<const std::string, int>(pImage->getFileName(), 1));
            }
            else
            {
                m_ImgRefCount[pImage->getFileName()]++;
            }
        }
    }
}


bool GeodeVisitor::isShareImg(const std::string& fileName)
{
    if (m_ImgRefCount.find(fileName) == m_ImgRefCount.end())
    {
        return false;
    }

    if (m_ImgRefCount[fileName] == 1)
    {
        return false;
    }

    return true;
}


bool GeodeVisitor::isShareGeode()
{
    return m_bShareGeode;
}

void GeodeVisitor::apply(osg::Geode &node)
{
    if (node.getNumDrawables() == 0)
    {
        return;
    }

    if (m_setFinishGeode.find(&node) != m_setFinishGeode.end())
    {
        return;
    }
    else
    {
        m_setFinishGeode.insert(&node);
    }

    osg::NodePathList pathList = node.getParentalNodePaths();
    m_nGeodesCount += pathList.size();
    if(pathList.empty())
    {
        return;
    }

    if (m_bGetNum)
    {
        countShareImg(node);
        return;
    }

    m_ShareGeodes.clear();
    m_bIsMultiPathList = false;
    if (pathList.size() > 1)
    {
        m_bIsMultiPathList = true;
    }

    if (m_bShareModel)
    {
        if (pathList.size() > 1)
        {
            m_bShareGeode = true;
        }
        else
        {
            m_bShareGeode = false;
        }

        //将中心点拉到原点 
        const osg::BoundingSphere bound = node.getBound();
        osg::Vec3d center = osg::Vec3d(0.0, 0.0, 0.0) - bound.center();
        for(size_t m = 0; m < node.getNumDrawables(); m++)
        {
            if (m_pModelBuilder->getStopFlag()) return;

            osg::Vec3Array* pVectexs = dynamic_cast<osg::Vec3Array*>(node.getDrawable(m)->asGeometry()->getVertexArray());
            if (pVectexs == NULL)
            {
                continue;
            }

            for(size_t n = 0; n < pVectexs->getNumElements(); n++)
            {
                (*pVectexs)[n] += center;
            }

            node.getDrawable(m)->asGeometry()->dirtyBound();
        }

        for (size_t i = 0; i < pathList.size(); i++)
        {
            if (m_pModelBuilder->getStopFlag()) return;

            osg::Matrixd mt = osg::computeLocalToWorld(pathList[i]);
            mt.preMultTranslate(-center);

            osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform();
            pMatrixTransform->setMatrix(mt);
            pMatrixTransform->addChild(&node);

            ModelGroup mod(this, m_pTargetDB, m_bMultiRefPoint);
            mod.setModel(pMatrixTransform);
            mod.saveModel();

            m_pModelBuilder->updateProgress();
        }
    }
    else
    {
        m_bShareGeode = false;

        //用于逐点上球
        for (size_t i = 0; i < pathList.size(); i++)
        {
            if (m_pModelBuilder->getStopFlag()) return;

            osg::Matrixd mt1 = osg::computeLocalToWorld(pathList[i]);
// 			osg::Quat quat(osg::Vec4d(-1.0, 0.0, 0.0, 1.0));
// 			mt1.makeRotate(quat);

            osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform();
            pMatrixTransform->setMatrix(mt1);
            pMatrixTransform->addChild(&node);

            ModelGroup mod(this, m_pTargetDB, m_bMultiRefPoint);
            mod.setModel(pMatrixTransform);
            mod.saveModel();

            m_pModelBuilder->updateProgress();
        }
    }

}

string GeodeVisitor::ExportOsgImage(osg::Image* image, bool bMultiTextureFlag)
{
    string sTextureName = image->getFileName();

    string sName = sTextureName.substr(sTextureName.find_last_of("\\")+1);
    string sNameEx = sTextureName.substr(sTextureName.find_last_of("\\")+1);
    sName = sName.substr(0, sName.find_last_of("."));

    sTextureName = sTextureName.substr(sTextureName.find_last_of("\\")+1);
    string sTextureFullName(m_pModelBuilder->getTempFilePath());
    if (bMultiTextureFlag)
    {
        //如果images文件夹有该文件名，删除。
        string strTempPathName = sTextureFullName + "images\\" + sName + ".bmp";
        if (!access(strTempPathName.c_str(), 0))
        {
            remove(strTempPathName.c_str());
        }

        sTextureFullName += "multi\\";
        sTextureFullName += sTextureName;
    }
    else
    {
        string strTempPathName = sTextureFullName + "multi\\" + sNameEx;
        if (!access(strTempPathName.c_str(), 0))
        {
            return "";
        }

        sTextureFullName += "images\\";
        sTextureName = sTextureName.substr(0, sTextureName.find_last_of(".")+1) + "bmp";
        sTextureFullName += sTextureName;
    }

	string sTextureFolder = sTextureFullName.substr(0, sTextureFullName.find_last_of("\\"));
	if (_access(sTextureFolder.c_str(), 0))
	{
		_mkdir(sTextureFolder.c_str());
		SetFileAttributes(sTextureFolder.c_str(), FILE_ATTRIBUTE_HIDDEN);
	}

	string ext = osgDB::getFileExtension(sTextureFullName);
	osgDB::ReaderWriter* writer = osgDB::Registry::instance()->getReaderWriterForExtension(ext);
	if (writer && access(sTextureFullName.c_str(), 0))
	{
		writer->writeImage(*image, sTextureFullName, NULL);
	}
	image->setFileName(sTextureFullName);

	return sTextureFullName;
}

void GeodeVisitor::addSpliceShareImg(const string& strImageFileName)
{
    if (m_ImgRefCount.find(strImageFileName) == m_ImgRefCount.end())
    {
        m_ImgRefCount.insert(std::pair<const std::string, int>(strImageFileName, 2));
    }
    else
    {
        m_ImgRefCount[strImageFileName]++;
    }

    return;
}
