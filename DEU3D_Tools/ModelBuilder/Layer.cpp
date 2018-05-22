#include "Layer.h"
#include "ModelBuilder.h"
#include <osgDB\ReadFile>
#include <osgDB\ReaderWriter>
#include <osg\PagedLOD>
#include "SimplifyTexture.h"
#include <strstream>
#include "md2.h"
#include "ErrorCode.h"
#include <IDProvider/Definer.h>
#include <osg\coordinateSystemNode>

Layer::Layer()
{
    reset();
}

void Layer::reset()
{
    _id = ID::genNewID();
    _id.ObjectID.m_nDataSetCode = 6;
    _id.ObjectID.m_nType    = CULTURE_LAYER_ID;

    _bs.setCenter(cmm::math::Vector3d(0.0,0,0));
    _bs.setRadius(-1.0f);
    _model_ids.clear();
}

Layer::~Layer(void)
{
}

ID Layer::getID()
{
    return _id;
}

cmm::IDEUException *Layer::createPointFromModelSymbol(Model &m, deudbProxy::IDEUDBProxy *db)
{
    if (db == NULL)
    {
        throw std::string("错误：db不能为空");
    }

    osg::ref_ptr<ParamPoint> point = new ParamPoint(m.getDatasetCode());
    
    m.writeToDB(db);

    point->fromModelSymbol(m);

    return addParam(point, db);
}

//IDEUException Layer::addParamPointsAndImages(ParamPointList& points, list<string>& image_files, IDEUDB *db)
//{
//    //将已创建好的参数点存进layer
//    ParamPointList::iterator k = points.begin();
//    for (; k != points.end(); ++k)
//    {
//        _params.push_back(dynamic_cast<Parameter*>(k->get()));
//    }
//    
//    if (points.size() != image_files.size())
//    {
//        string info = "参数点和图片个数不一样(";
//        char tmp[255];
//        sprintf_s(tmp, 255, "%d", points.size());
//        info += tmp;
//        info += ":";
//        sprintf_s(tmp, 255, "%d", image_files.size());
//        info += tmp;
//        info += "),输出以个数少为准";
//        
//        ModelBuilder::writeLog(info.c_str());
//    }
//
//    //将每个图片放入每个参数点，以两者个数少的为准
//    list<string>::iterator   i = image_files.begin();
//    ParamPointList::iterator j = points.begin();
//    ParameterList::iterator  l = _params.begin();
//
//    for(; i != image_files.end() && j != points.end(); ++i, ++j, ++l)
//    {
//        //从文件创建osgImage
//        osg::Image *image = osgDB::readImageFile(*i);
//        if (image == NULL)
//        {
//            return false;
//        }
//
//        //将osgImage存入ImageSymbol
//        ostrstream os;
//        string ext = osgDB::getFileExtension(*i);
//        ID                              id = ID::genNewID();
//        id.ObjectID.m_nDataSetCode          = _id.ObjectID.m_nDataSetCode;
//        id.ObjectID.m_nType                 = IMAGE_ID;
//        osgDB::ReaderWriter             *w = osgDB::Registry::instance()->getReaderWriterForExtension(ext);
//        osgDB::ReaderWriter::WriteResult r = w->writeImage(*image, os);
//
//        if (!db->addBlock(id, os.str(), os.pcount()))
//        {
//            return false;
//        }
//
//        //设置ImageSymbol相关参数
//        ID idSymbol = id;
//        idSymbol.ObjectID.m_nType = SYMBOL_ID;
//        sp<ISymbol>     symbol = ::createSymbol(idSymbol);
//        sp<IDynImageDetail>  img = dynamic_cast<IDynImageDetail*>(createDetail(IMAGE_DETAIL));
//        img->setImageSize(image->r(), image->s());
//        img->setImageID(id);
//
//        //将ImageSymbol放入ParamPoint
//        if (!j->get()->setImageSymbol(symbol))
//        {
//            return false;
//        }
//    }
//
//    image_files.erase(i, image_files.end());
//    _params.erase(l, _params.end());
//
//    return true;
//}

//IDEUException *Layer::addParamLines(ParamLineList& lines)
//{
//    ParamLineList::iterator k = lines.begin();
//
//    for (; k != lines.end(); ++k)
//    {
//        _params.push_back(dynamic_cast<::Parameter*>(k->get()));
//    }
//
//    sp<IDEUException> e = cmm::createDEUException();
//    return e.release();
//}
//
//IDEUException *Layer::addParamFaces(ParamFaceList& faces)
//{
//    ParamFaceList::iterator k = faces.begin();
//    for (; k != faces.end(); ++k)
//    {
//        _params.push_back(dynamic_cast<::Parameter*>(k->get()));
//    }
//
//    sp<IDEUException> e = cmm::createDEUException();
//    return e.release();
//}

cmm::IDEUException *Layer::addParam(::Parameter *param, deudbProxy::IDEUDBProxy *db)
{
    if (param == NULL || db == NULL)
    {
        throw std::string("错误：参数不能为空");
    }
    else if (db == NULL)
    {
        throw std::string("错误：db不能为空");
    }

    OpenSP::sp<cmm::IDEUException> e = cmm::createDEUException();
    
    //std::string val;
    //if (param->base()->findProperty("name", val) != -1)
    //{
    //    const char *name = val.c_str();

    //    ID id = param->base()->getID();
    //    CreateHashMD2((BYTE*)val.c_str(), val.length(), (BYTE*)&id.ObjectID.m_UniqueID);
    //    param->base()->setID(id);
    //}

    if (!param->writeToDB(getID(), db))
    {
        ModelBuilder::writeLog("Parameter入库失败\r\n");
        e->setReturnCode(MAKE_ERR_CODE(6));
        e->setMessage("写数据库失败");
        e->setTargetSite("Layer::addParam()");
        return e.release();
    }

    cmm::math::Sphered bs_param = param->getBoundingSphere();
    cmm::math::Vector3d center;
    osg::EllipsoidModel::instance()->convertLatLongHeightToXYZ(bs_param.getCenter().y(), bs_param.getCenter().x(), bs_param.getCenter().z(), 
                                                               center.x(), center.y(), center.z());
    bs_param.setCenter(center);

    _bs.expandBy(bs_param);
    _model_ids.push_back(param->getID());

    e->Reset();
    return e.release();
}

cmm::IDEUException *Layer::writeToDB(deudbProxy::IDEUDBProxy* db, const std::string &filename)
{
    if (filename.length() == 0)
    {
        throw std::string("错误：输出文件名不能为空");
    }
    else if (db == NULL)
    {
        throw std::string("错误：db不能为空");
    }

    OpenSP::sp<cmm::IDEUException> e = cmm::createDEUException();
    //存参数
    cmm::math::Vector3d old = _bs.getCenter();
    osg::Vec3d converted_center;

    osg::EllipsoidModel::instance()->convertXYZToLatLongHeight(old.x(), old.y(), old.z(), 
                                                             converted_center.y(), converted_center.x(), converted_center.z());

    //存图层
    
    /*图层的属性数据：
    {
        "ID":            "idSelf"
        "ParentID":        "idParent"
        "BoundingSphere":    "中心点的经纬度、高度、半径"
        "ChildrenID":        ["ID1","ID2","ID3",...,"IDn"]
    }*/
    
    std::string strLyrName = osgDB::getSimpleFileName(filename);
    strLyrName = osgDB::getNameLessExtension(strLyrName);

    bson::bsonDocument doc;
    doc.AddStringElement("Name", strLyrName.c_str());

    bson::bsonArrayEle *pBoundingSphere = dynamic_cast<bson::bsonArrayEle *>(doc.AddArrayElement("BoundingSphere"));
    pBoundingSphere->AddDblElement(converted_center.x());
    pBoundingSphere->AddDblElement(converted_center.y());
    pBoundingSphere->AddDblElement(converted_center.z());
    pBoundingSphere->AddDblElement(_bs.getRadius());

    bson::bsonArrayEle *pChildrenID = dynamic_cast<bson::bsonArrayEle *>(doc.AddArrayElement("ChildrenID"));
    std::vector<ID>::iterator l = _model_ids.begin();
    for(; l != _model_ids.end(); ++l)
    {
        pChildrenID->AddBinElement(&(*l), sizeof(*l));
    }

    bson::bsonStream bstream;
    if (!doc.Write(&bstream)) 
    {
        e->setReturnCode(MAKE_ERR_CODE(5));
        e->setMessage("bson流转换失败");
        e->setTargetSite("Layer::writeToDB()");
        return e.release();
    }

    if (!db->addBlock(_id, bstream.Data(), bstream.DataLen()))
    {
        e->setReturnCode(MAKE_ERR_CODE(6));
        e->setMessage("写数据库失败");
        e->setTargetSite("Layer::writeToDB()");
        return e.release();
    }

    e->Reset();
    return e.release();
}


//以下代码邱鑫添加 2014.06.19

void Layer::addPointParam(param::IPointParameter *pPointParamter)
{
    cmm::math::Sphered param_bs = pPointParamter->getBoundingSphere();
    double x, y, z;
    cmm::math::Vector3d param_center = param_bs.getCenter();
    osg::EllipsoidModel::instance()->convertLatLongHeightToXYZ(param_center.y(), param_center.x(), param_center.z(), 
        x, y, z);

    _bs.expandBy(cmm::math::Sphered(cmm::math::Vector3d(x, y, z), param_bs.getRadius()));
    _model_ids.push_back(pPointParamter->getID());
}


//以上代码邱鑫添加 2014.06.19