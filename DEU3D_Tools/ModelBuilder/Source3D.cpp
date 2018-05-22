#include "Source3D.h"
#include "ModelBuilder.h"
#include <osg\Texture2D>
#include <osgDB\ReadFile>
#include "ErrorCode.h"
#include <exception>
#include <math.h>
#include <osgDB\WriteFile>

class SharedGeodeSplitter : public osg::NodeVisitor
{
public:
    explicit SharedGeodeSplitter(std::vector<osg::ref_ptr<osg::MatrixTransform>> &pMTs): 
    osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
    _mts(pMTs)
    {
    }

protected:
    virtual void apply(osg::Geode &node)
    {
        //if (_mts.size() > 0) return;

        if (node.getNumDrawables()  == 0)
        {
            return;
        }

        osg::NodePathList path_list = node.getParentalNodePaths();
        if(path_list.empty())   return;

        for( size_t i = 1; i < path_list.size(); i++)
        {
            convert2MatrixTransform(path_list[i], (osg::Geode*)node.clone(osg::CopyOp::DEEP_COPY_ALL));
        }

        convert2MatrixTransform(path_list[0], &node);
    }

    void convert2MatrixTransform(osg::NodePath &np, osg::Geode* geode)
    {
        if (geode == NULL)
        {
            throw std::string("错误：几何体节点不能为空");
        }
        
        osg::MatrixTransform *mt = new osg::MatrixTransform();

        osg::Matrixd l2w = osg::computeLocalToWorld(np);

        for (size_t k = 0; k < geode->getNumDrawables(); k++)
        {
            osg::Geometry *geo = geode->getDrawable(k)->asGeometry();

            if (geo)
            {
                osg::Vec3Array *va = (osg::Vec3Array*)geo->getVertexArray();

                if (va == NULL) 
                {
                    char buf[256];
                    sprintf_s(buf, 256, "警告: 模型(%s)里第%d/%d个元素没有顶点数据", geode->getName().c_str(), k + 1, geode->getNumDrawables());
                    Builder::writeLog(buf);
                    continue;
                }

                for( size_t l = 0; l < va->getNumElements(); l++)
                {
                    (*va)[l] = l2w.preMult((*va)[l]);
                }

                va = (osg::Vec3Array*)geo->getNormalArray();

                if (va == NULL) 
                {
                    char buf[256];
                    sprintf_s(buf, 256, "警告: 模型(%s)里第%d/%d个元素没有法向量数据", geode->getName().c_str(), k + 1, geode->getNumDrawables());
                    Builder::writeLog(buf);
                    continue;
                }

                for( size_t l = 0; l < va->getNumElements(); l++)
                {
                    (*va)[l] = l2w.preMult((*va)[l]) - l2w.preMult(osg::Vec3d(0.0, 0.0, 0.0));
                    (*va)[l].normalize();
                }

                geo->dirtyBound();
            }
        }

        for(size_t k = 0; k < geode->getNumDrawables(); k++)
        {
            osg::Geometry *geo = geode->getDrawable(k)->asGeometry();
            osg::Vec3Array *va = (osg::Vec3Array*)geo->getNormalArray();

            if (va == NULL) 
            {
                char buf[256];
                sprintf_s(buf, 256, "警告: 模型(%s)里第%d/%d个元素没有法向量数据", geode->getName().c_str(), k + 1, geode->getNumDrawables());
                Builder::writeLog(buf);
                continue;
            }

            for( size_t l = 0; l < va->getNumElements(); l++)
            {
                (*va)[l] = l2w.preMult((*va)[l]) - l2w.preMult(osg::Vec3d(0.0, 0.0, 0.0));
                (*va)[l].normalize();
            }
        }

        //osg::Matrixd l2w = osg::computeLocalToWorld(np);
        //l2w.preMult(osg::Matrixd::translate(bs.center()));

        osg::Matrixd m;
        mt->setMatrix(m);
        mt->addChild(geode);
        mt->setName(geode->getName());

        _mts.push_back(mt);
    }


    std::vector<osg::ref_ptr<osg::MatrixTransform>> &_mts;
};

bool Source3D::load(const std::string &ivefile)
{
    std::string out = "提示：读取" + ivefile;
    Builder::writeLog(out.c_str());

    std::string ext = osgDB::getFileExtension(ivefile);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext != "ive" && ext != "osg") 
    {
        Builder::writeLog("错误：无法处理的文件类型(只接受ive或osg)");
        return false;
    }

    try{
        osg::ref_ptr<osg::Node> pNode = osgDB::readNodeFile(ivefile);
        if(pNode == NULL) 
        {
            out = "错误：无法打开文件: " + ivefile;
            Builder::writeLog(out.c_str());
            return false;
        }

        osg::BoundingSphere bs = pNode->getBound();
        char tmp[256];
        sprintf_s(tmp, 256, "提示: 模型中心点(%0.2f, %0.2f, %0.2f), 半径(%0.2f)", bs.center().x(), bs.center().y(), bs.center().z(), bs.radius());
        Builder::writeLog(tmp);

        SharedGeodeSplitter sgs(_mts);
        pNode->accept(sgs);

        if(_mts.size() == 0) 
        {
            out = "错误：文件" + ivefile + "里有效几何体为0";
            Builder::writeLog(out.c_str());
            return false;
        }
    }catch(...)
    {
        out = "错误：未知异常";
        Builder::writeLog(out.c_str());
        return false;
    }

    Builder::writeLog("提示: 文件读取成功");
    return true;
}

void CountTextureSharedTimes(osg::Geode *pGeode, std::map<const std::string, int> &imgRefCount, std::map<const std::string, size_t> &imgSize, std::map<const std::string, size_t> &imgPixels)
{
    if (pGeode == NULL)
    {
        throw std::string("错误：几何体节点不能为空");
    }

    int cnt = 1;
    size_t total_size = 0;

    //char buf[256];
    //sprintf_s(buf, 256, "提示: 模型(%s)里的图片:", pGeode->getName().c_str());
    //Builder::writeLog(buf);

    for(unsigned int j = 0; j < pGeode->getNumDrawables(); j++)
    {
        osg::ref_ptr<osg::Drawable> pDrawable = pGeode->getDrawable(j);
        osg::ref_ptr<osg::StateSet> pStateSet = pDrawable->getStateSet();
        if(pStateSet == NULL)
        {
            continue;
        }
        unsigned nTexCount = pStateSet->getNumTextureAttributeLists();

        for(unsigned int k = 0; k < nTexCount; k++)
        {
            osg::ref_ptr<osg::StateAttribute> pAttribute = pStateSet->getTextureAttribute(k, osg::StateAttribute::TEXTURE);
            osg::Texture2D *pTexture2D = dynamic_cast<osg::Texture2D *>(pAttribute.get());
            
            if (pTexture2D)
            {
                osg::Image *img = pTexture2D->getImage();

                if (img)
                {
                    total_size += img->getTotalSizeInBytes();

                    imgPixels[img->getFileName()] = img->s() * img->t();

                    if (imgSize.find(img->getFileName()) == imgSize.end())
                    {
                        imgSize.insert(std::pair<const std::string, size_t>(img->getFileName(), img->getTotalSizeInBytes()));
                    }

                    if (imgRefCount.find(img->getFileName()) == imgRefCount.end())
                    {
                        imgRefCount.insert(std::pair<const std::string, int>(img->getFileName(), 1));
                    }
                    else
                    {
                        imgRefCount[img->getFileName()]++;
                    }
                }
            }
        }
    }

    //sprintf_s(buf, 256, "合计: 图片数=%d, 图片总大小=%0.1fK", cnt, total_size / 1024.0f);
    //Builder::writeLog(buf);
    //Builder::writeLog(" ");
}

void Geode_simplifyTexture(osg::Geode *pGeode, std::map<const std::string, int> &imgRefCount, std::map<const std::string, size_t> &imgPixels)
{
    if (pGeode == NULL)
    {
        throw std::string("错误：几何体节点不能为空");
    }

    const size_t maxPixelsPerGeode = 1024 * 2048;//每个Geode允许最大的纹理像素数
    
    //计算加权后的像素数
    size_t numPixel = 0;
    std::map<const std::string, size_t>::iterator i = imgPixels.begin();
    for(; i != imgPixels.end(); ++i)
    {
        //图片实际像素数除以被引用次数
        numPixel += i->second / imgRefCount[i->first];
    }

    if (numPixel <= maxPixelsPerGeode) return;

    size_t numTarget = numPixel;
    size_t scaleRate = 0;

    while(numTarget > maxPixelsPerGeode)
    {
        numTarget >>= 2;
        scaleRate++;
    }

    for(unsigned int j = 0; j < pGeode->getNumDrawables(); j++)
    {
        osg::ref_ptr<osg::Drawable> pDrawable = pGeode->getDrawable(j);
        osg::ref_ptr<osg::StateSet> pStateSet = pDrawable->getStateSet();
        unsigned nTexCount = pStateSet->getNumTextureAttributeLists();

        for(unsigned int k = 0; k < nTexCount; k++)
        {
            osg::ref_ptr<osg::StateAttribute> pAttribute = pStateSet->getTextureAttribute(k, osg::StateAttribute::TEXTURE);
            osg::Texture2D *pTexture2D = dynamic_cast<osg::Texture2D *>(pAttribute.get());
            
            if (pTexture2D == NULL) continue;
            
            osg::Image *img = pTexture2D->getImage();
            osg::Image::MipmapDataType mdt;
            img->setMipmapLevels(mdt);

            if (img == NULL) continue;

            int newW = img->s() >> scaleRate;
            int newH = img->t() >> scaleRate;

            if (newW >= 32 && newH >= 32)
            {
                img->scaleImage(newW, newH, img->r());
            }
        }
    }

}

bool IsGeodeSharedTex(osg::Geode *pGeode, std::map<const std::string, int> &imgNames)
{
    if (pGeode == NULL)
    {
        throw std::string("错误：几何体节点不能为空");
    }

    for(unsigned int j = 0; j < pGeode->getNumDrawables(); j++)
    {
        osg::ref_ptr<osg::Drawable> pDrawable = pGeode->getDrawable(j);
        osg::ref_ptr<osg::StateSet> pStateSet = pDrawable->getStateSet();
        if(pStateSet == NULL)
        {
            continue;
        }
        unsigned nTexCount = pStateSet->getNumTextureAttributeLists();

        for(unsigned int k = 0; k < nTexCount; k++)
        {
            osg::ref_ptr<osg::StateAttribute> pAttribute = pStateSet->getTextureAttribute(k, osg::StateAttribute::TEXTURE);
            osg::Texture2D *pTexture2D = dynamic_cast<osg::Texture2D *>(pAttribute.get());
            
            if (pTexture2D)
            {
                osg::Image *img = pTexture2D->getImage();
                if (img)
                {
                    if (imgNames[img->getFileName()] > 1) 
                        return true;
                }
            }
        }
    }
    return false;
}

//bool Source3D::createSymbols(const std::string &ivefile, unsigned short dataset_code, bsonArrayEle *pSymbolIDs, deudbProxy::IDEUDBProxy *db)
//{
//    if (!load(ivefile))
//    {
//        return false;
//    }
//
//    std::map<const std::string, int> imgSharedCnt;
//    std::map<const std::string, size_t> imgSize;
//
//    for(unsigned int i=0; i < _mts.size(); i++)
//    {
//        ref_ptr<MatrixTransform> pMatrixTransform = _mts[i];
//        ref_ptr<Geode> pGeode = pMatrixTransform->getChild(0)->asGeode();
//        CountTextureSharedTimes(pGeode, imgSharedCnt, imgSize);
//    }
//
//    _shared_textures.clear();
//
//    for(unsigned int i=0; i < _mts.size(); i++)
//    {
//        ref_ptr<MatrixTransform> mt = _mts[i];
//
//        ref_ptr<Geode> geode = mt->getChild(0)->asGeode();
//        bool texture_shared = IsGeodeSharedTex(geode, imgSharedCnt);
//
//        Model m(dataset_code, mt);
//        
//        if (!m.changeIntoLOD(texture_shared, _shared_textures))
//        {
//            return false;
//        }
//
//        m.writeToDB(db);
//        pSymbolIDs->AddStringElement(m.symbol()->getID().toString().c_str());
//        
//        _mts[i] = NULL;
//    }
//
//    return true;
//}

cmm::IDEUException *Source3D::createSymbolsWithOffset(const std::string &ivefile, unsigned short dataset_code, osg::Vec3d offset, const SpatialRefInfo& sri, deudbProxy::IDEUDBProxy *db, Layer &layer)
{
    if (dataset_code <= 100)
    {
        throw std::string("错误：数据集编号必须大于100");
    }
    else if (db == NULL)
    {
        throw std::string("错误：db不能为空");
    }

    OpenSP::sp<cmm::IDEUException> e = cmm::createDEUException();
    //以下代码由邱鑫添加 2014.06.22
    //if(loadIVE(ivefile, dataset_code, offset, sri, db, layer))
    //{
    //    e->Reset();
    //    return e.release();
    //}
    //以上代码由邱鑫添加 2014.06.22

    if (!load(ivefile))
    {
        std::string tmp = ivefile + "文件打开失败";
		e->setReturnCode(MAKE_ERR_CODE(3));
        e->setMessage(tmp);
        e->setTargetSite("Source3D::createSymbolsWithOffset()");
        return e.release();
    }

    std::map<const std::string, int> imgSharedCnt;
    std::map<const std::string, size_t> imgSize;

    for(unsigned int i=0; i < _mts.size(); i++)
    {
        osg::ref_ptr<osg::MatrixTransform> mt = _mts[i];
        osg::ref_ptr<osg::Geode> pGeode = mt->getChild(0)->asGeode();
        std::map<const std::string, size_t> imgPixels;

        CountTextureSharedTimes(pGeode, imgSharedCnt, imgSize, imgPixels);
        Geode_simplifyTexture(pGeode, imgSharedCnt, imgPixels);
    }

    size_t total_size = 0;
    char buf[256];

    for (std::map<const std::string, int>::iterator it = imgSharedCnt.begin(); it != imgSharedCnt.end(); it++)
    {
        total_size += imgSize[it->first];
    }
    sprintf_s(buf, 256, "提示: %s里图片总数=%d, 图片总大小=%0.3fM", ivefile.c_str(), imgSharedCnt.size(), total_size / 1024.0f / 1024.0f);
    Builder::writeLog(buf);

    _shared_textures.clear();

    Builder::writeLog("提示: 模型开始上球");
    //osg::ref_ptr<osg::Group> group = new osg::Group();

    for(unsigned int i=0; i < _mts.size(); i++)
    {
        Builder::updateProgress(i + 1, _mts.size());

        osg::ref_ptr<osg::MatrixTransform> mt = _mts[i];
        osg::ref_ptr<osg::Geode> geode = mt->getChild(0)->asGeode();
        bool texture_shared = IsGeodeSharedTex(geode, imgSharedCnt);

        Model m(dataset_code, mt.get());

        if (!m.transformOntoGlobe(offset, sri))
        {
            char tmp[128];
            sprintf_s(tmp, 128, "错误:模型(%s)变换上球失败，忽略此模型\r\n", mt->getName().c_str());
            Builder::writeLog(tmp);

            continue;
        }

        if (!m.changeIntoLOD(texture_shared, _shared_textures, db))
        {
            char tmp[128];
            sprintf_s(tmp, 128, "错误:模型(%s)变换LOD失败，忽略此模型\r\n", mt->getName().c_str());
            Builder::writeLog(tmp);
            continue;
        }

        e = layer.createPointFromModelSymbol(m, db);
        if (e->getReturnCode())
        {
            char tmp[128];
            sprintf_s(tmp, 128, "错误:创建模型(%s)点失败，忽略此模型\r\n", mt->getName().c_str());
            Builder::writeLog(tmp);
        }
        
        _mts[i] = NULL;
    }

    e->Reset();
    return e.release();
}


//以下代码邱鑫添加 2014.06.18

void getAllParentMatrixTransformByGeode(osg::Geode *pGeode, std::vector<osg::ref_ptr<osg::MatrixTransform> > &vecMatrixTransform)
{
    osg::NodePathList path_list = pGeode->getParentalNodePaths();

    if(path_list.empty())
    {
        return;
    }

    osg::Geode *pNewGeode = new osg::Geode(*pGeode, osg::CopyOp::DEEP_COPY_ALL);
    osg::NodePathList::iterator itor = path_list.begin();
    for(; itor != path_list.end(); ++itor)
    {
        osg::MatrixTransform *pMatrixTransform = new osg::MatrixTransform;
        osg::Matrixd matrix = osg::computeLocalToWorld(*itor);
        pMatrixTransform->setMatrix(matrix);
        pMatrixTransform->addChild(pNewGeode);
        vecMatrixTransform.push_back(pMatrixTransform);
    }
}

#include <common/CoordinateTransform.h>
#include <osg/CoordinateSystemNode>
#include <IDProvider/Definer.h>
#include <strstream>
#include <osg/ProxyNode>
#include <osg/ComputeBoundsVisitor>
#include <osgUtil/DistancePixelComputer.h>
bool Transform(osg::Vec3d &vSrc, osg::Vec3d &vDst, const SpatialRefInfo &sri)
{
    cmm::CoordinateTransform  ct;
    bool transed_ok = true;

    osg::Vec3d vLonLat(0.0, 0.0, vSrc.z());

    if (sri._coordinate_sys == "WGS84")
    {
        transed_ok = ct.Gauss_wgsxyToBl(vSrc.x(),
                                        vSrc.y(),
                                        &vLonLat[1],
                                        &vLonLat[0],
                                        sri._band_no,
                                        sri._band_type);
    }
    else if(sri._coordinate_sys == "BJ54")
    {
        transed_ok = ct.Gauss_54xyToBl(vSrc.x(),
                                       vSrc.y(),
                                       &vLonLat[1],
                                       &vLonLat[0],
                                       sri._band_no,
                                       sri._band_type);

    }else if(sri._coordinate_sys == "XA80")
    {
        transed_ok = ct.Gauss_80xyToBl(vSrc.x(),
                                       vSrc.y(),
                                       &vLonLat[1],
                                       &vLonLat[0],
                                       sri._band_no,
                                       sri._band_type);
    }
    else
    {
        vDst.set(osg::DegreesToRadians(vLonLat[0]), osg::DegreesToRadians(vLonLat[1]), vLonLat[2]);
        return true;
    }

    if (!transed_ok)
    {
        return false;
    }

    vDst.set(osg::DegreesToRadians(vLonLat[0]), osg::DegreesToRadians(vLonLat[1]), vLonLat[2]);
    //osg::EllipsoidModel::instance()->convertLatLongHeightToXYZ(osg::DegreesToRadians(vLonLat[1]), 
    //                                                           osg::DegreesToRadians(vLonLat[0]),
    //                                                           vLonLat[2],
    //                                                           vDst.x(),
    //                                                           vDst.y(),
    //                                                           vDst.z());

    return transed_ok;
}

bool writeNode(deudbProxy::IDEUDBProxy *pProxyDB, osg::Node &node)
{
    if (pProxyDB == NULL)
    {
        return false;
    }

    ID id = node.getID();
    if (id.ObjectID.m_nType != MODEL_ID && id.ObjectID.m_nType != SHARE_MODEL_ID)
    {
        return false;
    }

    osg::ref_ptr<osgDB::ReaderWriter::Options> opts = new osgDB::ReaderWriter::Options;

    osgDB::ReaderWriter *pRW = osgDB::Registry::instance()->getReaderWriterForExtension("ive");
    std::ostrstream ss;
    osgDB::ReaderWriter::WriteResult wr = pRW->writeNode(node, ss, opts);
    if (!wr.success())
    {
        return false;
    }

    if (!pProxyDB->updateBlock(id, ss.str(), ss.pcount()))
    {
        ss.freeze(false);
        return false;
    }

    ss.freeze(false);
    return true;
    //解冻，然后在析构函数中释放资源
}

double computeRadiusOfBound(osg::Node *pNode)
{
    osg::ref_ptr<osg::ComputeBoundsVisitor> pComputer = new osg::ComputeBoundsVisitor;
    pNode->accept(*pComputer);

    const osg::BoundingBox &bb = pComputer->getBoundingBox();
    const double dblWidth  = osg::clampAbove(double(bb.xMax() - bb.xMin()), 0.1);
    const double dblHeight = osg::clampAbove(double(bb.yMax() - bb.yMin()), 0.1);
    const double dblDepth  = osg::clampAbove(double(bb.zMax() - bb.zMin()), 0.1);
    const double dblVolume = dblWidth * dblHeight * dblDepth;

    const double dblRadius3 = dblVolume * 0.75 / osg::PI;   // volume of sphere = (r ^ 3) * pi * (4 / 3);
    const double dblRadius  = pow(dblRadius3, 1.0 / 3.0);
    return dblRadius;
}

bool Source3D::loadIVE(const std::string &strIVE, unsigned short nDatasetCode, osg::Vec3d offset, const SpatialRefInfo& sri, deudbProxy::IDEUDBProxy *pDB, Layer &layer)
{
    osg::ref_ptr<osg::Node> pNode = osgDB::readNodeFile(strIVE);
    if(pNode == NULL) 
    {
        return false;
    }

    osgUtil::DistancePixelComputer dpc;
    cmm::math::Point2i resolution = Builder::getResolution();
    dpc.setEnviroment(45.0, resolution.x(), resolution.y());
    cmm::math::Vector2d pixel_size = Builder::getPixelSize();

    m_pFindGeodeVisitor = new FindGeodeVisitor();
    pNode->accept(*m_pFindGeodeVisitor);
    if(m_pFindGeodeVisitor->hasSharedGedoe())
    {
#if 0
        std::set<osg::Geode *> &setGeodes = m_pFindGeodeVisitor->getGeodes();
        std::set<osg::Geode *>::iterator itor = setGeodes.begin();

        std::map<const std::string, int> imgSharedCnt;
        std::map<const std::string, size_t> imgSize;

        for(unsigned int i=0; i < _mts.size(); i++)
        {
            osg::ref_ptr<osg::MatrixTransform> mt = _mts[i];
            osg::ref_ptr<osg::Geode> pGeode = mt->getChild(0)->asGeode();
            std::map<const std::string, size_t> imgPixels;

            CountTextureSharedTimes(pGeode, imgSharedCnt, imgSize, imgPixels);
            Geode_simplifyTexture(pGeode, imgSharedCnt, imgPixels);
        }

        SharedTexs shared_textures;

        itor = setGeodes.begin();
        for(; itor != setGeodes.end(); ++itor)
        {
            bool texture_shared = IsGeodeSharedTex((*itor), imgSharedCnt);
            GeodeProcesser processer((*itor), nDatasetCode, offset,  &sri, pDB);
            if(!processer.process(texture_shared, shared_textures))
            {
                continue;
            }

            layer.createPointFromModelSymbol(processer, pDB);
        }
#endif

        std::set<osg::Geode *> &setGeodes = m_pFindGeodeVisitor->getGeodes();

        std::set<osg::Geode *>::iterator itor = setGeodes.begin();
        itor = setGeodes.begin();
        for(; itor != setGeodes.end(); ++itor)
        {
            osg::Geode *pGeode = (*itor);
            osg::BoundingSphere geode_bs = pGeode->getBound();
            if(geode_bs.radius() <= 0)
            {
                continue;
            }
            std::vector<osg::ref_ptr<osg::MatrixTransform> > vecMatrixTransform;
            getAllParentMatrixTransformByGeode(pGeode, vecMatrixTransform);

            unsigned int nSize = vecMatrixTransform.size();

            if(nSize == 0)
            {
                continue;
            }
            //没有被共享
            else if(nSize == 1)
            {
                //MatrixTransform将作为一个Detail
                osg::MatrixTransform *pMatrixTransform = vecMatrixTransform[0].get();
                const osg::BoundingSphere &bs = pMatrixTransform->getBound();

                if(bs.radius() <= 0)
                {
                    int a = 0;
                    continue;
                }
                ID model_id(nDatasetCode, MODEL_ID);
                pMatrixTransform->setID(model_id);
                writeNode(pDB, *pMatrixTransform);

                const double dblRadiusChecking = computeRadiusOfBound(pMatrixTransform);
                const double fMinDis = dpc.calcDistanceByPixelSize(dblRadiusChecking, pixel_size.y());
                const double fMaxDis = dpc.calcDistanceByPixelSize(dblRadiusChecking, pixel_size.x());

                OpenSP::sp<param::IStaticModelDetail> pStaticModelDetail = dynamic_cast<param::IStaticModelDetail *>(param::createDetail(param::STATIC_DETAIL, nDatasetCode));
                pStaticModelDetail->setModelID(model_id);
                pStaticModelDetail->setAsOnGlobe(false);

                {
                    bson::bsonDocument doc;
                    bson::bsonStream bstream;
                    pStaticModelDetail->toBson(doc);
                    doc.Write(&bstream);
                    pDB->addBlock(pStaticModelDetail->getID(), bstream.Data(), bstream.DataLen());
                }

                //点参数
                ID point_param_id(nDatasetCode, PARAM_POINT_ID);
                param::IPointParameter *pPointParameter = dynamic_cast<param::IPointParameter*>(param::createParameter(point_param_id));

                osg::Vec3d vDst;
                Transform(offset, vDst, sri);

                pPointParameter->setCoordinate(cmm::math::Point3d(vDst.x(), vDst.y(), vDst.z()));
                pPointParameter->setAzimuthAngle(0.0);
                pPointParameter->setPitchAngle(0.0);
                pPointParameter->addDetail(pStaticModelDetail->getID(), fMinDis, fMaxDis);

                param::PointParameter *p = dynamic_cast<param::PointParameter*>(pPointParameter);
                p->setRadius(bs.radius());

                {
                    param::Parameter *pParameter = dynamic_cast<param::Parameter *>(p);
                    bson::bsonDocument doc_param;
                    bson::bsonStream bstream_param;
                    pParameter->toBson(doc_param);
                    doc_param.Write(&bstream_param);
                    pDB->addBlock(pPointParameter->getID(), bstream_param.Data(), bstream_param.DataLen());
                }

                layer.addPointParam(pPointParameter);
            }
            //有被共享
            else
            {
                //Geode被作为一个SHARE_MODLE
                ID share_model_id(nDatasetCode, SHARE_MODEL_ID);
                pGeode->setID(share_model_id);
                writeNode(pDB, *pGeode);

                for(unsigned int i = 0; i < nSize; i++)
                {
                    //MatrixTransform将作为一个Detail
                    osg::MatrixTransform *pMatrixTransform = vecMatrixTransform[i].get();
                    const osg::BoundingSphere &bs = pMatrixTransform->getBound();
                    if(bs.radius() <= 0)
                    {
                        int a = 0;
                        continue;
                    }
                    ID model_id(nDatasetCode, MODEL_ID);
                    pMatrixTransform->setID(model_id);

                    const double dblRadiusChecking = computeRadiusOfBound(pMatrixTransform);
                    const double fMinDis = dpc.calcDistanceByPixelSize(dblRadiusChecking, pixel_size.y());
                    const double fMaxDis = dpc.calcDistanceByPixelSize(dblRadiusChecking, pixel_size.x());

                    osg::ref_ptr<osg::ProxyNode> pProxyNode = new osg::ProxyNode;
                    pProxyNode->setFileName(0, share_model_id.toString());
                    pMatrixTransform->removeChild(0u);
                    pMatrixTransform->addChild(pProxyNode.get());
                    writeNode(pDB, *pMatrixTransform);

                    OpenSP::sp<param::IStaticModelDetail> pStaticModelDetail = dynamic_cast<param::IStaticModelDetail *>(param::createDetail(param::STATIC_DETAIL, nDatasetCode));
                    pStaticModelDetail->setModelID(model_id);
                    pStaticModelDetail->setAsOnGlobe(false);

                    {
                        bson::bsonDocument doc;
                        bson::bsonStream bstream;
                        pStaticModelDetail->toBson(doc);
                        doc.Write(&bstream);
                        pDB->addBlock(pStaticModelDetail->getID(), bstream.Data(), bstream.DataLen());
                    }

                    //点参数
                    ID point_param_id(nDatasetCode, PARAM_POINT_ID);
                    param::IPointParameter *pPointParameter = dynamic_cast<param::IPointParameter*>(param::createParameter(point_param_id));

                    osg::Vec3d vDst;
                    Transform(offset, vDst, sri);

                    pPointParameter->setCoordinate(cmm::math::Point3d(vDst.x(), vDst.y(), vDst.z()));
                    pPointParameter->setAzimuthAngle(0.0);
                    pPointParameter->setPitchAngle(0.0);
                    pPointParameter->addDetail(pStaticModelDetail->getID(), fMinDis, fMaxDis);

                    param::PointParameter *p = dynamic_cast<param::PointParameter*>(pPointParameter);
                    p->setRadius(bs.radius());

                    {
                        param::Parameter *pParameter = dynamic_cast<param::Parameter *>(p);
                        bson::bsonDocument doc_param;
                        bson::bsonStream bstream_param;
                        pParameter->toBson(doc_param);
                        doc_param.Write(&bstream_param);
                        pDB->addBlock(pPointParameter->getID(), bstream_param.Data(), bstream_param.DataLen());
                    }

                    layer.addPointParam(pPointParameter);
                }
            }
        }
    }
    m_pFindGeodeVisitor = NULL;

    return true;
}

//以上代码邱鑫添加 2014.06.18