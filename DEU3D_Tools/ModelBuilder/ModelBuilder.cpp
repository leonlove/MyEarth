#include "ModelBuilder.h"
#include <strstream>
#include <Common\CoordinateTransform.h>
#include <Windows.h>
#include "ErrorCode.h"
#include <ParameterSys\IDetail.h>
#include <IDProvider/Definer.h>
#include <EventAdapter\IEventObject.h>
#include <ParameterSys\IPointParameter.h>
#include <ParameterSys\ILineParameter.h>
#include <ParameterSys\IFaceParameter.h>
#include <ParameterSys\PointParameter.h>
#include <ParameterSys\LineParameter.h>
#include <ParameterSys\FaceParameter.h>
#include "GeodeVisitor.h"
#include <DDSConverter.h>
#include <io.h>
#include <sys\stat.h>

#include "DEUCheck.h"
DEUCheck checker(18, 5);

void trim(string& str)
{
    static const string delims = " \t\r";
    str.erase(str.find_last_not_of(delims)+1);
    str.erase(0, str.find_first_not_of(delims));
}

IModelBuilder* createModelBuilder(void)
{
    return (IModelBuilder*)new ModelBuilder();
}

void ModelBuilder::initLodConfigParam()
{
    std::string strFileName = cmm::GetAppPath(false) + "LODConfig.ini";
    if (access(strFileName.c_str(), 0) == 0)
    {
        readLodConfigFile(strFileName);
    }

    //- 设置默认值
    m_LodSegment.dLOD1 = 200.0;
    m_LodSegment.dLOD2 = 600.0;
    m_LodSegment.dLOD3 = 2500.0;
    m_LodSegment.dLOD4 = 3000.0;
    m_LodSegment.dLOD5 = 0.0;

    m_LodSegment.bOptimize          = true;
    m_LodSegment.nValidLodLevels    = 4;

    return;
}

ModelBuilder::ModelBuilder()
{
	m_strTempFilePath = cmm::GetAppPath(false) + "TempFile\\";

	srand(time(NULL));
	int nDirVal = rand();
	char chTmp[100];
	itoa(nDirVal, chTmp, 10);
	string strSpliceFolder(chTmp);
	strSpliceFolder = strSpliceFolder + "_";
	m_strTempFilePath += strSpliceFolder;

	string strTmpLog = cmm::GetAppPath(false) + "TempFile\\" + "modelpath_log.txt";
    m_of_file.open(strTmpLog.c_str(), ios::out | ios_base::app);
	SetFileAttributes(strTmpLog.c_str(), FILE_ATTRIBUTE_HIDDEN);

    m_bShareModel  = true;
    m_bInitialized = false;
    m_bStop        = false;
    m_strSelectedLodName = "";

    initLodConfigParam();
}

ModelBuilder::~ModelBuilder()
{
    std::string strFileName = cmm::GetAppPath(false) + "LODConfig.ini";
    writeLodConfigFile(strFileName);

    m_of_file.close();
}

/**********************************************************************************
*函数说明：
    将图层里每个对象的ID写到DB里，同时写入图层ID
*输入参数：
*输出参数：
*返回值:
*   IDEUException: 异常对象
**********************************************************************************/
void ModelBuilder::writeConfigFile(OpenSP::sp<cmm::IDEUException> e)
{
    //*
    ID id = ID::genNewID();
    id.ObjectID.m_nDataSetCode = 6;
    id.ObjectID.m_nType        = CULTURE_LAYER_ID;

    std::string strLayerName = osgDB::getSimpleFileName(m_strTargetDB);
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

    m_pTargetDB->closeDB();

    std::ofstream file;
    file.open(m_strTargetDB+".dscfg", std::ios::out | std::ios::trunc);
    file<<"{\"IDList\":[\""<<id.toString()<<"\"]}";
    file.close();

    m_models.clear();
    m_SphereVec.clear();

	string strTemp = m_strTempFilePath + "images";
	RemoveDirectory(strTemp.c_str());
	strTemp = m_strTempFilePath + "multi";
	RemoveDirectory(strTemp.c_str());
	strTemp = m_strTempFilePath + "output";
	RemoveDirectory(strTemp.c_str());

    return;
}

/**********************************************************************************
*函数说明：      
    初始化模型构建器
*输入参数：
*   strTargetDB :   DB路径
*   nDataSetCode :  数据集编号
*   ea:             事件适配器，用于发事件
*返回值:
*   IDEUException: 异常对象
**********************************************************************************/
bool ModelBuilder::initialize(const std::string &strTargetDB, unsigned nDataSetCode, ea::IEventAdapter *ea, OpenSP::sp<cmm::IDEUException> e)
{
    if (m_bInitialized)
    {
        return true;
    }

    m_pEventAdapter = ea;

    unsigned nReadBufferSize = 128u * 1024u * 1024u;
    unsigned nWriteBufferSize = 32u * 1024u * 1024u;

    if(strTargetDB.empty())
    {
        m_strTargetDB = cmm::genLocalTempDB();
        nReadBufferSize = cmm::getLocalTempDBBufferSize();
        nWriteBufferSize = cmm::getLocalTempDBBufferSize();
    }
    else
    {
        m_strTargetDB = strTargetDB;
    }

    m_datasetCode = nDataSetCode;

    m_pTargetDB = deudbProxy::createDEUDBProxy();

    m_models.clear();
    m_SphereVec.clear();


    if (!m_pTargetDB->openDB(m_strTargetDB, nReadBufferSize, nWriteBufferSize) && e.valid())
    {
        e->setReturnCode(MAKE_ERR_CODE(3));
        std::string tmp = m_strTargetDB + " 打开失败";
        e->setMessage(tmp);
        e->setTargetSite("ModelBuilder::initialize()");
        return false;
    }

    m_bInitialized = true;

    return true;
}

bool ModelBuilder::setProjectionInfo(const std::string &strCoordSys, const std::string &strProj, double dEastOffset, double dNorthOffset, double dCentreLongitude)
{
    m_strCoordination  = strCoordSys;
    m_strProjection    = strProj;
    m_dEastOffset      = dEastOffset;
    m_dNorthOffset     = dNorthOffset;
    m_dCentreLongitude = dCentreLongitude;

    return true;
}


bool ModelBuilder::setOffset(double dblEast, double dblNorth, double dblElevation)
{
    m_offset.x() = dblEast;
    m_offset.y() = dblNorth;
    m_offset.z() = dblElevation;

    return true;
}

unsigned ModelBuilder::findLowerNumber(unsigned n) const
{
    const static unsigned nNumbers[12] = {1u, 2u, 4u, 8u, 16u, 32u, 64u, 128u, 256u, 512u, 1024u, 2048u};
    const unsigned *pos = std::upper_bound(nNumbers, nNumbers + 12, n);
    if(pos == nNumbers)
    {
        return *pos;
    }
    return *--pos;
}

void ModelBuilder::writeLog(const char *pInfo)
{
    if (m_pEventAdapter)
    {
        std::string strInfo(pInfo);
        cmm::variant_data data(strInfo);

        OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
        pEventObject->setAction(ea::ACTION_MODEL_BUILDER);
        pEventObject->putExtra("LogInfo", data);
        m_pEventAdapter->sendBroadcast(pEventObject.get());
    }
}

void ModelBuilder::updateProgress()
{
    if (m_pEventAdapter)
    {
        m_nProgress++;
        cmm::variant_data p(m_nProgress);
        cmm::variant_data t(m_nModeCount);

        OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
        pEventObject->setAction(ea::ACTION_MODEL_BUILDER);
        pEventObject->putExtra("Progress_Current", p);
        pEventObject->putExtra("Progress_Total", t);
        m_pEventAdapter->sendBroadcast(pEventObject.get());
    }
}

/**********************************************************************************
*函数说明：
    由IVE文件构建模型
*输入参数：
*   strFile :       IVE文件路径
*返回值:
*   IDEUException: 异常对象
**********************************************************************************/
void ModelBuilder::buildIveModel(const std::string &strFile, OpenSP::sp<cmm::IDEUException> e)
{
    try
    {
        m_of_file << strFile << endl;
        std::string ext = osgDB::getFileExtension(strFile);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext != "ive" && ext != "osg") 
        {
            writeLog("错误：无法处理的文件类型(只接受ive或osg)");
            return;
        }

        if(m_bStop) 
        {
            writeLog("停止模型数据处理!\r\n");
            return;
        }

        char buf[1024] = {0};
        sprintf_s(buf, 1024, "开始处理文件：%s\r\n", strFile.c_str());
        writeLog(buf);
        osg::Vec3d     offset(m_offset.x(), m_offset.y(), m_offset.z());
        SpatialRefInfo sri;
        sri.m_strCoordination  = m_strCoordination;
        sri.m_strProjection    = m_strProjection;
        sri.m_dEastOffset      = m_dEastOffset;
        sri.m_dNorthOffset     = m_dNorthOffset;
        sri.m_dCentreLongitude = m_dCentreLongitude;

        //- 判断文件是否为空
        struct _stat file;
        int nRes = _stat(strFile.c_str(), &file);
        if (file.st_size == 0)
        {
            string strInfo = "读取" + strFile + "失败！\r\n";
            writeLog(strInfo.c_str());
            return;
        }

        osg::ref_ptr<osg::Node> pNode = osgDB::readNodeFile(strFile);
        if (pNode == NULL)
        {
            string strInfo = "读取" + strFile + "失败！\r\n";
            writeLog(strInfo.c_str());
            return;
        }
        if(m_bStop) 
        {
            writeLog("停止模型数据处理!\r\n");
            return;
        }

        osg::ref_ptr<GeodeVisitor> pGeodeVisitor = new GeodeVisitor(this);
        pGeodeVisitor->setUseMultiRefPoint(m_bUseMultiRefPoint);
        pGeodeVisitor->setGetNum(true);
        pNode->accept(*pGeodeVisitor);
        m_nModeCount = pGeodeVisitor->getGeodesCount();
        m_nProgress = 0;

        if(m_bStop) 
        {
            RemoveTempTexture(true);
            writeLog("停止模型数据处理!\r\n");
            return;
        }
		
        if (m_LodSegment.bOptimize)
        {
            DYCanvasList::UnionDIR(DY_TEXT(m_strTempFilePath + "images"), DY_TEXT(m_strTempFilePath + "output"), m_mapFileToMat3x3, m_mapFileToID, 1024);
            CountSpliceShareImg(m_strTempFilePath + "output", pGeodeVisitor);
        }

        if(m_bStop) 
        {
            RemoveTempTexture(true);
            writeLog("停止模型数据处理!\r\n");
            return;
        }
        sprintf_s(buf, 1024, "模型个数：%d，处理中...\r\n", m_nModeCount);
        writeLog(buf);

        pGeodeVisitor->setDataSetCode(m_datasetCode);
        pGeodeVisitor->setGetNum(false);
        pGeodeVisitor->setOffset(offset);
        pGeodeVisitor->setSpatialRefInfo(sri);
        pGeodeVisitor->setShareModel(m_bShareModel);
        pGeodeVisitor->setTargetDB(m_pTargetDB);
        pNode->accept(*pGeodeVisitor);

        RemoveTempTexture(false);

        if (m_bStop)
        {
            RemoveTempTexture(true);
            writeLog("停止模型数据处理!\r\n");
        }
        else
            writeLog("成功!\r\n");
    }
    catch(std::string err)
    {
        writeLog(err.c_str());
        writeLog("处理无法进行！");
    }
}

/**********************************************************************************
*函数说明：
    由I参数构建模型
*输入参数：
*   pParameter :       参数对象
*返回值:
*   IDEUException: 异常对象
**********************************************************************************/
void ModelBuilder::buildParamModel(param::IParameter *pParameter, OpenSP::sp<cmm::IDEUException> e,const std::map<std::string,std::string>& attrMap)
{
    try
    {
        OpenSP::sp<cmm::IDEUException> eInternal = cmm::createDEUException();

        if (pParameter->getID().ObjectID.m_nType == PARAM_POINT_ID)
        {
            param::IPointParameter *pIPointParameter = dynamic_cast<param::IPointParameter *>(pParameter);
            param::PointParameter  *pPointParameter  = dynamic_cast<param::PointParameter *>(pParameter);

            const cmm::math::Point3d &pt = pIPointParameter->getCoordinate();
            for(size_t i = 0; i < pIPointParameter->getNumDetail(); i++)
            {
                ID idDetail; 
                double min, max;
                if (!pIPointParameter->getDetail(i, idDetail, min, max))
                {
                    return;
                }

                void *buffer = NULL;
                unsigned int len = 0;

                if (!m_pTargetDB->readBlock(idDetail, buffer, len))
                {
                    return;
                }

                bson::bsonStream bs;
                bs.Write(buffer, len);
                bs.Reset();
                bson::bsonDocument doc;
                doc.Read(&bs);

                OpenSP::sp<param::IDetail> detail = param::createDetail(idDetail);
                detail->fromBson(doc);

                deudbProxy::freeMemory(buffer);

                if(idDetail.ObjectID.m_nType == DETAIL_STATIC_MODEL_ID)
                {
                    param::IStaticModelDetail *sd = dynamic_cast<param::IStaticModelDetail*>(detail.get());

                    //osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(sd->getModelID());
                    void *pBuffer = NULL;
                    unsigned nLength = 0u;
                    if (!m_pTargetDB->readBlock(sd->getModelID(), pBuffer, nLength)) return;

                    osg::ref_ptr<osgDB::ReaderWriter> pIveRW = osgDB::Registry::instance()->getReaderWriterForExtension("ive");
                    if(pIveRW.valid() == false)
                    {
                        deudbProxy::freeMemory(pBuffer);
                        return;
                    }
                    std::strstream ss((char *)pBuffer, nLength);
                    osgDB::ReaderWriter::ReadResult wr = pIveRW->readNode(ss);
                    if (wr.success() == false)
                    {
                        deudbProxy::freeMemory(pBuffer);
                        return;
                    }

                    osg::ref_ptr<osg::Node> model = wr.getNode();
                    if (!model.valid())
                    {
                        deudbProxy::freeMemory(pBuffer);
                        return;
                    }
                    deudbProxy::freeMemory(pBuffer);

                    pPointParameter->setRadius(model->getBound().radius());
                }
                else
                {
                    pPointParameter->setRadius(detail->getBoundingSphereRadius());
                }
            }

            writeToDB(pParameter, m_pTargetDB,attrMap);
        }
        else if(pParameter->getID().ObjectID.m_nType == PARAM_LINE_ID)
        {
           writeToDB(pParameter, m_pTargetDB,attrMap);
        }
        else if (pParameter->getID().ObjectID.m_nType == PARAM_FACE_ID)
        {
            writeToDB(pParameter, m_pTargetDB,attrMap);
        }

        if (e.valid())
        {
            *e.get() = *eInternal.get();
        }
    }
    catch (std::string err)
    {
        writeLog(err.c_str());
        writeLog("处理无法进行！");
    }
}


bool ModelBuilder::buildImage(const std::string &strImageFile, ID &idImage)
{
    osg::ref_ptr<osg::Image> pImage = osgDB::readImageFile(strImageFile);
    if (!pImage.valid())
    {
        return false;
    }

    idImage = ID::genNewID();
    idImage.ObjectID.m_nDataSetCode = m_datasetCode;
    idImage.ObjectID.m_nType = SHARE_IMAGE_ID;

    //如果纹理不是2的幂，则把数据归为2的幂
    const unsigned int nScaleS = findLowerNumber(pImage->s());
    const unsigned int nScaleT = findLowerNumber(pImage->t());
    pImage->scaleImage(nScaleS, nScaleT, pImage->r());

    const GLenum ePixelFormat = pImage->getPixelFormat();
    unsigned char *pDDSData = NULL;
    unsigned nDataLen = 0u;
    switch(ePixelFormat)
    {
        case GL_RGB:
            RGB_2_DXT1(pImage->data(), pImage->s(), pImage->t(), PO_RGB, &pDDSData, &nDataLen);
            break;
        case GL_BGR:
            RGB_2_DXT1(pImage->data(), pImage->s(), pImage->t(), PO_BGR, &pDDSData, &nDataLen);
            break;
        case GL_RGBA:
        {
            const GLenum eBest = pImage->findBestBitsOf16();
            if(eBest == GL_UNSIGNED_SHORT_5_5_5_1)
            {
                RGBA_2_DXT1(pImage->data(), pImage->s(), pImage->t(), PO_RGBA, &pDDSData, &nDataLen);
            }
            else if(eBest == GL_UNSIGNED_SHORT_4_4_4_4)
            {
                RGBA_2_DXT5(pImage->data(), pImage->s(), pImage->t(), PO_RGBA, &pDDSData, &nDataLen);
            }
            break;
        }
        case GL_BGRA:
        {
            const GLenum eBest = pImage->findBestBitsOf16();
            if(eBest == GL_UNSIGNED_SHORT_5_5_5_1)
            {
                RGBA_2_DXT1(pImage->data(), pImage->s(), pImage->t(), PO_BGRA, &pDDSData, &nDataLen);
            }
            else if(eBest == GL_UNSIGNED_SHORT_4_4_4_4)
            {
                RGBA_2_DXT5(pImage->data(), pImage->s(), pImage->t(), PO_BGRA, &pDDSData, &nDataLen);
            }
            break;
        }
        default:
            return false;
    }

    if(!pDDSData || nDataLen < 1u)
    {
        return false;
    }

    const bool bRet = m_pTargetDB->addBlock(idImage, pDDSData, nDataLen);
    freeDXTMemory(pDDSData);
    return bRet;
}


bool ModelBuilder::buildModel(const std::string &strIveFile, ID &idModel)
{
    osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(strIveFile);
    if(node.valid() == false) return false;

    idModel = ID::genNewID();
    idModel.ObjectID.m_nDataSetCode = m_datasetCode;
    idModel.ObjectID.m_nType = SHARE_MODEL_ID;
    node->setID(idModel);

    osg::ref_ptr<osgDB::ReaderWriter> pIveRW = osgDB::Registry::instance()->getReaderWriterForExtension("ive");
    if(pIveRW.valid() == false) return false;

    std::ostrstream ss;
    osgDB::ReaderWriter::WriteResult wr = pIveRW->writeNode(*node.get(), ss);
    if (wr.success() == false) return false;

    bool ok = m_pTargetDB->addBlock(idModel, ss.str(), ss.pcount());

    ss.freeze(false);

    return ok;
}

bool ModelBuilder::buildDetail(param::IDetail *detail, OpenSP::sp<cmm::IDEUException> e)
{
    try
    {
        bson::bsonDocument doc;
        if (!detail->toBson(doc)) return false;

        bson::bsonStream bs;
        if (!doc.Write(&bs)) return false;

        return m_pTargetDB->addBlock(detail->getID(), bs.Data(), bs.DataLen());
    }catch(std::string err)
    {
        writeLog(err.c_str());
        writeLog("处理无法进行！");
        return false;
    }
}

bool ModelBuilder::shortenLineSegment(cmm::math::Point3d &begin, cmm::math::Point3d &end, double dblShortenLength, bool bShortenFromBegin)
{
    osg::Vec3d ptBegin, ptEnd;
    osg::EllipsoidModel::instance()->convertLatLongHeightToXYZ(begin.x(), begin.y(), begin.z(), ptBegin.x(), ptBegin.y(), ptBegin.z());
    osg::EllipsoidModel::instance()->convertLatLongHeightToXYZ(end.x(), end.y(), end.z(), ptEnd.x(), ptEnd.y(), ptEnd.z());

    osg::Vec3d  vDir = ptEnd - ptBegin;
    double      len  = vDir.normalize() - dblShortenLength;
    if (len <= 0.000001) return false;

    osg::Vec3d  dest;

    if (bShortenFromBegin)
    {
        dest = ptEnd - vDir * len;
        osg::EllipsoidModel::instance()->convertXYZToLatLongHeight(dest.x(), dest.y(), dest.z(), begin.x(), begin.y(), begin.z());
    }
    else
    {
        dest = ptBegin + vDir * len;
        osg::EllipsoidModel::instance()->convertXYZToLatLongHeight(dest.x(), dest.y(), dest.z(), end.x(), end.y(), end.z());
    }

    return true;
}


bool ModelBuilder::moveLineSegment(cmm::math::Point3d &begin, cmm::math::Point3d &end, double dblMovingDist)
{
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    osg::Vec3d ptBegin, ptEnd;
    pEllipsoidModel->convertLatLongHeightToXYZ(begin.y(), begin.x(), begin.z(), ptBegin.x(), ptBegin.y(), ptBegin.z());
    pEllipsoidModel->convertLatLongHeightToXYZ(end.y(), end.x(), end.z(), ptEnd.x(), ptEnd.y(), ptEnd.z());

    osg::Vec3d  vecDir = ptEnd - ptBegin;
    const double dblLenDir = vecDir.normalize();
    if(cmm::math::floatEqual(dblLenDir, 0.0))
    {
        return false;
    }

    const cmm::math::Point3d center = (end + begin) * 0.5;
    const osg::Vec3d vecPlumbLine = pEllipsoidModel->computeLocalPlumbLine(center.y(), center.x());
    osg::Vec3d vecRightDir = vecPlumbLine ^ vecDir;
    const double dblLenRight = vecRightDir.normalize();
    if(cmm::math::floatEqual(dblLenRight, 0.0))
    {
        return false;
    }

    vecRightDir *= dblMovingDist;
    ptBegin += vecRightDir;
    ptEnd   += vecRightDir;

    pEllipsoidModel->convertXYZToLatLongHeight(ptBegin.x(), ptBegin.y(), ptBegin.z(), begin.y(), begin.x(), begin.z());
    pEllipsoidModel->convertXYZToLatLongHeight(ptEnd.x(),   ptEnd.y(),   ptEnd.z(),   end.y(),   end.x(),   end.z());
    return true;
}


bool ModelBuilder::writeToDB(param::IParameter *pParameter, deudbProxy::IDEUDBProxy *db,const std::map<std::string,std::string>& attrMap)
{
    if (db == NULL)
    {
        throw std::string("错误：db不能为空");
    }

    bson::bsonDocument doc;
    param::Parameter *pParam = dynamic_cast<param::Parameter *>(pParameter);
    if (!pParam->toBson(doc))
    {
        return false;
    }

    std::map<std::string,std::string>::const_iterator pItr = attrMap.cbegin();
    while(pItr != attrMap.cend())
    {
        doc.AddStringElement(pItr->first.c_str(),pItr->second.c_str());
        pItr++;
    }

    bson::bsonStream bs;
    doc.Write(&bs);
    if (!db->addBlock(pParameter->getID(), bs.Data(), bs.DataLen()))
    {
        return false;
    }

    //包围球
    cmm::math::Sphered bound = pParameter->getBoundingSphere();
    osg::BoundingSphere sphere;
    sphere._center.x() = bound.getCenter().x();
    sphere._center.y() = bound.getCenter().y();
    sphere._center.z() = bound.getCenter().z();
    sphere._radius = bound.getRadius();

    m_models.push_back(pParameter->getID());
    m_SphereVec.push_back(sphere);

    return true;
}


param::IParameter* ModelBuilder::getParameterByID(const ID id)
{
    if (m_pTargetDB == NULL)
    {
        return NULL;
    }

    void *pBuffer = NULL;
    unsigned int len = 0;
    if (!m_pTargetDB->readBlock(id, pBuffer, len))
    {
        return NULL;
    }

    OpenSP::sp<param::IParameter> pIParameter = param::createParameter(id);
    if (pIParameter == NULL)
    {
        return NULL;
    }

    param::Parameter* pParameter = dynamic_cast<param::Parameter*>(pIParameter.get());
    if (pParameter == NULL)
    {
        return NULL;
    }

    bson::bsonDocument doc;
    bson::bsonStream   bstream;
    bstream.Write(pBuffer, len);
    bstream.Reset();
    doc.Read(&bstream);
    if (!pParameter->fromBson(doc))
    {
        return false;
    }

    return pIParameter.release();
}


bool ModelBuilder::updateParameter(param::IParameter* pIParameter)
{
    if (m_pTargetDB == NULL || pIParameter == NULL)
    {
        return false;
    }

    bson::bsonDocument doc;
    bson::bsonStream   bstream;

    param::Parameter* pParameter = dynamic_cast<param::Parameter*>(pIParameter);
    if (!pParameter->toBson(doc))
    {
        return false;
    }

    if (!doc.Write(&bstream))
    {
        return false;
    }

    if (!m_pTargetDB->updateBlock(pIParameter->getID(), bstream.Data(), bstream.DataLen()))
    {
        return false;
    }

    return true;
}

bool ModelBuilder::readLodConfigFile(const string& strLodConfigFileName)
{
    std::ifstream ifReader;
    ifReader.open(strLodConfigFileName.c_str());
    if (!ifReader.is_open())
    {
        return false;
    }
    m_mapLodNameAndDistance.clear();

    vector<double> vecDistance;
    string sLine, sSubLine, strName, strDistance;
    while(getline(ifReader, sLine))
    {
        trim(sLine);
        sSubLine = "Name:";
        int nIndex1 = sLine.find(sSubLine);
        if (nIndex1 != -1)
        {
            nIndex1 += sSubLine.length();
            strName = sLine.substr(nIndex1);
            continue;
        }

        //增加优化一行
        sSubLine = "Optimization:";
        int nIndex3 = sLine.find(sSubLine);
        if (nIndex3 != -1)
        {
            nIndex3 += sSubLine.length();
            strDistance = sLine.substr(nIndex3);

            double dDistance = 0.0;
            stringstream ss;
            ss << strDistance;
            ss >> dDistance;
            vecDistance.push_back(dDistance);
            continue;
        }

        sSubLine = "Lod"; 
        int nIndex2 = sLine.find(sSubLine);
        if (nIndex2 != -1)
        {
            nIndex2 += sSubLine.length()+2; //Lod1=:
            strDistance = sLine.substr(nIndex2);

            double dDistance = 0.0;
            stringstream ss;
            ss << strDistance;
            ss >> dDistance;
            vecDistance.push_back(dDistance);

            continue;
        }

        if (sLine == "-------------------")
        {
            m_mapLodNameAndDistance.insert(make_pair(strName, vecDistance));
            strName = "";
            vecDistance.clear();
            continue;
        }
    }
    ifReader.close();

    return true;
}

bool ModelBuilder::writeLodConfigFile(const string& strLodConfigFileName)
{
    std::ofstream ofWriter;
    ofWriter.open(strLodConfigFileName.c_str(), ios_base::trunc);
    if (!ofWriter.is_open())
    {
        return false;
    }

    map<string, vector<double> >::iterator itr = m_mapLodNameAndDistance.begin();
    for (; itr!=m_mapLodNameAndDistance.end(); itr++)
    {
        ofWriter << "Name:" << itr->first << endl;

        ofWriter << "Optimization:"<< itr->second[0] << endl;

        for (size_t i=1; i<itr->second.size(); i++)
        {
            ofWriter << "Lod" << i << "=" << itr->second[i] << endl;
        }

        ofWriter << "-------------------" << endl;
    }
    ofWriter.close();

    return true;
}

LODSEGMENT ModelBuilder::getLodSegment()
{
    return m_LodSegment;
}

std::vector<ID>& ModelBuilder::getVecModelID()
{
    return m_models;
}

std::vector<osg::BoundingSphere>& ModelBuilder::getVecBoundingSphere()
{
    return m_SphereVec;
}

vector<double> ModelBuilder::getDistanceVec(const std::string& strLodConfigParam)
{
    vector<double> vecLodDistance;
    //string strDistance = strLodConfigParam.substr(strLodConfigParam.find_first_of(":"));
    //while((nPos=strDistance.find_first_of(",")) != -1)
    //{
    //    string strSub = strDistance.substr(0, nPos);
    //    stringstream ss;
    //    ss << strSub;

    //    double dDistance = 0.0;
    //    ss >> dDistance;

    //    vecLodDistance.push_back(dDistance);
    //    strDistance = strDistance.substr(nPos+1);
    //}

     //alert by liuronghu 2016-02-19
    string strDistance = strLodConfigParam.substr(strLodConfigParam.find_first_of(":") + 1);
    int nPos = 0;
    string strSub;
    
    double dDistance = 0.0;
    while((nPos=strDistance.find_first_of(",")) != -1)
    {
        strSub = strDistance.substr(0, nPos);
        stringstream ss;
        ss << strSub;
        ss >> dDistance;
        vecLodDistance.push_back(dDistance);
        strDistance = strDistance.substr(nPos+1);
    }

    stringstream strs;
    strs << strDistance;
    strs >> dDistance;
    vecLodDistance.push_back(dDistance);
    //alert end
    return vecLodDistance;
}

std::vector<std::string> ModelBuilder::getLodConfigParams()
{
    vector<string> vecLodConfigParams;
    
    map<string, vector<double> >::iterator itr = m_mapLodNameAndDistance.begin();
    for (; itr!=m_mapLodNameAndDistance.end(); itr++)
    {
        string strDistanceString("");
        for (size_t i=0; i<itr->second.size(); i++)
        {
            string strTemp("");
            stringstream ss;
            ss << itr->second[i];
            ss >> strTemp;

            if (i == itr->second.size()-1)
                strDistanceString = strDistanceString + strTemp;
            else
                strDistanceString = strDistanceString + strTemp + ",";
        }

        vecLodConfigParams.push_back(itr->first + ":" + strDistanceString);
    }

    return vecLodConfigParams;
}

bool ModelBuilder::addLodConfigParam(const std::string& strLodConfigParam)
{
    //string strName = strLodConfigParam.substr(strLodConfigParam.find_first_of(":"));
    //alert by liuronghu 2016-02-19
    string strName = strLodConfigParam.substr(0 ,strLodConfigParam.find_first_of(":"));
    //alert

    map<string, vector<double> >::iterator itr = m_mapLodNameAndDistance.find(strName);
    if (itr != m_mapLodNameAndDistance.end())
    {
        itr->second = getDistanceVec(strLodConfigParam);
    }
    else
    {
        m_mapLodNameAndDistance.insert(make_pair(strName, getDistanceVec(strLodConfigParam)));
    }

    return true;
}

void ModelBuilder::setSelectedLodName(const std::string& strSelectedLodName)
{
    m_strSelectedLodName = strSelectedLodName;

    map<string, vector<double> >::iterator itr = m_mapLodNameAndDistance.find(m_strSelectedLodName);
    if (itr != m_mapLodNameAndDistance.end())
    {
        if (itr->second[0] == 0)
        {
            m_LodSegment.bOptimize = false;
        }
        else
        {
            m_LodSegment.bOptimize = true;
        }
        m_LodSegment.dLOD1 = itr->second[1];
        m_LodSegment.dLOD2 = itr->second[2];
        m_LodSegment.dLOD3 = itr->second[3];
        m_LodSegment.dLOD4 = itr->second[4];
        m_LodSegment.dLOD5 = itr->second[5];

//         if (!(m_LodSegment.dLOD4 > 0))
//         {
//             m_LodSegment.nValidLodLevels = 3;
//         }
//         else if (!(m_LodSegment.dLOD5 > 0))
//         {
//             m_LodSegment.nValidLodLevels = 4;
//         }
//         else
//         {
//             m_LodSegment.nValidLodLevels = 5;
//         }

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

    return;
}

void ModelBuilder::stop()
{
    m_bStop = true;
}

std::string ModelBuilder::getLodConfigParamByName(const std::string& strLodConfigName)
{
    map<string, vector<double> >::iterator itr = m_mapLodNameAndDistance.find(strLodConfigName);
    string strDistanceString("");
    if (itr != m_mapLodNameAndDistance.end())
    {
        for (size_t i=0; i<itr->second.size(); i++)
        {
            string strTemp("");
            stringstream ss;
            ss << itr->second[i];
            ss >> strTemp;

            if (i == itr->second.size()-1)
                strDistanceString = strDistanceString + strTemp;
            else
                strDistanceString = strDistanceString + strTemp + ",";
        }
    }
    return strDistanceString;
}

bool ModelBuilder::deleteLodConfigParam(const std::string& strLodConfigName)
{
    m_mapLodNameAndDistance.erase(strLodConfigName);
    return true;
};

void ModelBuilder::DelTempTextureFolder(const string& strDirPath, bool bRemoveDirFlag)  
{  
	string strPathTmp = strDirPath + "\\*.*";  
	long lFile;  
	string sAddPath;  
	struct _finddata_t sDate;  
	lFile = _findfirst(strPathTmp.c_str(), &sDate);  
	if (lFile == -1)  
	{  
		return ;  
	}  

	while (_findnext(lFile, &sDate) == 0)  
	{  
		if(sDate.attrib == _A_SUBDIR)  
		{  
			sAddPath = strDirPath;  
			sAddPath += "\\";  
			sAddPath += sDate.name;  
			if (string(sDate.name)=="." || string(sDate.name)=="..")  
			{  
				continue;  
			}  
			DelTempTextureFolder(sAddPath, bRemoveDirFlag);  
		}  
		else  
		{  
			// 删除文件  
			string strFile = strDirPath + "\\" + string(sDate.name);   
			remove(strFile.c_str());  
		}  
	}

    if (bRemoveDirFlag)
    {
        RemoveDirectory(strDirPath.c_str());
    }

	_findclose(lFile);
} 

void ModelBuilder::CountSpliceShareImg(string strImagePath, osg::ref_ptr<GeodeVisitor>& pGeodeVisitor)
{
	string strPathTmp = strImagePath + "\\*.*";  
	long lFile;  
	string sAddPath;  
	struct _finddata_t sDate;  
	lFile = _findfirst(strPathTmp.c_str(), &sDate);  
	if (lFile == -1)  
	{  
		return ;  
	}  

	while (_findnext(lFile, &sDate) == 0)  
	{  
		if(sDate.attrib == _A_SUBDIR)  
		{  
			sAddPath = strImagePath;  
			sAddPath += "\\";  
			sAddPath += sDate.name;  
			if (string(sDate.name)=="." || string(sDate.name)=="..")  
			{  
				continue;  
			}  
			CountSpliceShareImg(sAddPath, pGeodeVisitor);  
		}  
		else  
		{  
			string strFile = strImagePath + "\\" + string(sDate.name);   
			pGeodeVisitor->addSpliceShareImg(strFile);
		}  
	}  

	_findclose(lFile);  
}

void ModelBuilder::RemoveTempTexture(bool bRemoveDirFlag)
{
    if (!m_LodSegment.bOptimize)
    {
        return;
    }

    DelTempTextureFolder(m_strTempFilePath + "images", bRemoveDirFlag);
    DelTempTextureFolder(m_strTempFilePath + "multi", bRemoveDirFlag);
    DelTempTextureFolder(m_strTempFilePath + "output", bRemoveDirFlag);
}

const string& ModelBuilder::getTempFilePath()
{
    return m_strTempFilePath;
}

const DYFileToMat3x3& ModelBuilder::getFileToMat3x3()
{
    return m_mapFileToMat3x3;
}

const DYFileToID& ModelBuilder::getFileToID()
{
    return m_mapFileToID;
}
