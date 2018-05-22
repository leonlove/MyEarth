#include <Windows.h>
#include "Model.h"
#include "ModelBuilder.h"
#include <osg\CoordinateSystemNode>
#include <Common\CoordinateTransform.h>
#include <osg\Geometry>
#include <osg\ComputeBoundsVisitor>
#include <osgUtil\DistancePixelComputer.h>
#include <osg\Texture2D>
#include <osg\PagedLOD>
#include <osg\Material>
#include <osgDB\ReadFile>
#include <strstream>
#include "md2.h"
#include "SimplifyTexture.h"
#include <ParameterSys\Detail.h>
#include <IDProvider/Definer.h>
#include <osgDB/WriteFile>
#include <assert.h>

    //最大纹理图片像素
	unsigned int   _max_image_size = 512;
	//最小纹理图片像素
	unsigned int   _min_image_size = 32;

Model::Model(unsigned short dataset_code, osg::MatrixTransform *mt)
{
    _dataset_code = dataset_code;
    _mt = mt;
}

Model::~Model(void)
{

}

osg::MatrixTransform* Model::mt()
{
    return _mt;
}

void Model::addGeode(osg::Geode* g, ID &id, double range)
{
    if (g == NULL)
    {
        throw std::string("错误：几何体节点不能为空");
    }

    _geodes.push_back(g);
    _ids.push_back(id);
    
    if (_ranges.empty()) _ranges.push_back(0);
    _ranges.push_back(range);
}

void Model::saveDetail(deudbProxy::IDEUDBProxy *db)
{
    if (db == NULL)
    {
        throw std::string("错误：db不能为空");
    }

    //写符号信息
    for(int j = (int)_ids.size() - 1; j >= 0; j--)
    {
        OpenSP::sp<param::IStaticModelDetail> sm   = dynamic_cast<param::IStaticModelDetail *>(param::createDetail(param::STATIC_DETAIL, _dataset_code));
        sm->setModelID(_ids[j]);
        sm->setAsOnGlobe(true);

        bson::bsonDocument doc;
        bson::bsonStream bstream;

        if (!sm->toBson(doc)) goto WRITE_ERROR;

        if (!doc.Write(&bstream)) goto WRITE_ERROR;

        if (!db->addBlock(sm->getID(), bstream.Data(), bstream.DataLen())) 
        {
            WRITE_ERROR:
            std::string warning = "警告: ";
            warning += "存储IStaticModelDetail时出错，无法写入DB";
            ModelBuilder::writeLog(warning.c_str());
        }

        _detail_ids.push_back(sm->getID());
        //_param->addDetail(sm->getID(), _ranges[j], _ranges[j + 1]);

    }
    //_param->addProperty("Name", name);
}

void Model::getDetail(param::IPointParameter *p)
{
    if (p == NULL)
    {
        throw std::string("错误：点参数不能为空");
    }

    for(int i = 0, j = (int)_ids.size() - 1; j >= 0; i++, j--)
    {
        p->addDetail(_detail_ids[i], _ranges[j], _ranges[j + 1]);
    }

    p->addProperty("Name", _mt->getName());
}

bool Vec3_Transform(osg::Vec3d src, osg::Vec3d &dst, const SpatialRefInfo&  sri)
{
    cmm::CoordinateTransform  ct;
    bool transed_ok = true;
    osg::Vec3d vLonLat(0.0, 0.0, src.z());

    if (sri._coordinate_sys == "WGS84")
	{
		transed_ok = ct.Gauss_wgsxyToBl( src.x(),
							src.y(),
							&vLonLat[1],
							&vLonLat[0],
							sri._band_no,
							sri._band_type);
	}
	else if(sri._coordinate_sys == "BJ54")
	{
		transed_ok = ct.Gauss_54xyToBl(  src.x(),
							src.y(),
							&vLonLat[1],
							&vLonLat[0],
							sri._band_no,
							sri._band_type);

	}else if(sri._coordinate_sys == "XA80")
	{
        transed_ok = ct.Gauss_80xyToBl( src.x(),
							src.y(),
							&vLonLat[1],
							&vLonLat[0],
							sri._band_no,
							sri._band_type);
	}
	else
	{
        dst = src;
        return true;
	}

    if (transed_ok == false)
    {
        char tmp[128];
        sprintf_s(tmp, 128, "错误: 变换上球失败,输入数据坐标不是数字(x=%f, y=%f)\r\n", src.x(), src.y());
        Builder::writeLog(tmp);
        return false;
    }

    osg::EllipsoidModel::instance()->convertLatLongHeightToXYZ(
        osg::DegreesToRadians(vLonLat[1]), 
        osg::DegreesToRadians(vLonLat[0]),
        vLonLat[2],
        dst.x(), dst.y(), dst.z());

    return transed_ok;
}

bool Model::transformOntoGlobe(const osg::Vec3d &offset, const SpatialRefInfo&  sri)
{
    osg::ref_ptr<osg::Geode> pGeode = _mt->getChild(0)->asGeode();
    
    if (pGeode == NULL)
    {
        throw std::string("错误：几何体节点不能为空");
    }

    if(pGeode->getParents().size() > 1)
    {
        pGeode = new osg::Geode(*_mt->getChild(0)->asGeode(), osg::CopyOp::DEEP_COPY_ALL);
        _mt->replaceChild(_mt->getChild(0)->asGeode(), pGeode);
    }

    unsigned     n       = pGeode->getNumDrawables();
    osg::Matrixd mOrigin = _mt->getMatrix();

    for (unsigned i = 0; i < n; i++)
    {
        osg::Drawable   *drawable		= pGeode->getDrawable(i);
        osg::Geometry   *geo			= drawable->asGeometry();
        osg::Vec3Array  *verticies		= dynamic_cast<osg::Vec3Array*>(geo->getVertexArray());

		if (verticies == NULL) continue;

		osg::Vec3dArray *new_vertices	= new osg::Vec3dArray;

        unsigned m = verticies->getNumElements();

        for (unsigned j = 0; j < m; j++)
        {
            osg::Vec3d src = osg::Vec3d((*verticies)[j]) * mOrigin + offset;
            osg::Vec3d dst;

            if (Vec3_Transform(src, dst, sri) == false) continue;

			new_vertices->push_back(dst);
        }
        
		geo->setVertexArray(new_vertices);
        geo->dirtyBound();
    }

	osg::BoundingSphere bs = pGeode->getBound();

    //将中心点的偏移拿到矩阵里
    unsigned k = 0;
	const osg::Vec3d center = bs.center();
    osg::Vec3d vWorldCenter;

    if (Vec3_Transform(osg::Vec3d(0,0,0), vWorldCenter, sri) == false)
    {
        return false;
    }

    for (unsigned i = 0; i < n; i++)
    {
        osg::Drawable  *drawable = pGeode->getDrawable(i);
        osg::Geometry  *geo      = drawable->asGeometry();
        osg::Vec3dArray *verticies= dynamic_cast<osg::Vec3dArray*>(geo->getVertexArray());

		if (verticies == NULL) continue;

		osg::Vec3Array *float_vertices = new osg::Vec3Array;

        unsigned m = verticies->getNumElements();
        for (unsigned j = 0; j < m; j++)
        {
			float_vertices->push_back((*verticies)[j] - center);
        }

		geo->setVertexArray(float_vertices);

        osg::Vec3Array *normals  = dynamic_cast<osg::Vec3Array*>(geo->getNormalArray());
        for (unsigned j = 0; j < normals->getNumElements(); j++)
        {
            osg::Vec3d dst;
            if (Vec3_Transform((*normals)[j], dst, sri) == false) continue;

            (*normals)[j] = dst - vWorldCenter;
            (*normals)[j].normalize();
        }

        geo->dirtyBound();
    }
    
    const osg::Matrixd mtx(osg::Matrixd::translate(center));
    _mt->setMatrix(mtx);

    return true;
}

double computeRadiusOfBoundChecking(osg::Node *pNode)
{
    if (pNode == NULL)
    {
        throw std::string("错误：节点不能为空");
    }

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

int Model::changeTextureIntoLOD(osg::Geode *pGeode, osg::Geode *pTempGeode, unsigned int nLevel, bool texture_shared, SharedTexs &shared_textures)
{
    if (pGeode == NULL || pTempGeode == NULL)
    {
        throw std::string("错误：几何体节点不能为空");
    }

    int nLastLevel = 0;
    unsigned int numDrawable = pTempGeode->getNumDrawables();

    for(unsigned int j = 0; j < numDrawable; j++)
    {
        //对这个Geode下的所有纹理进行压缩
        osg::ref_ptr<osg::Drawable> pDrawable = pTempGeode->getDrawable(j);
        osg::ref_ptr<osg::Geometry> pGeometry = pDrawable->asGeometry();
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
            if(pTexture2D == NULL) continue;

            osg::ref_ptr<osg::Image> pImage = pTexture2D->getImage();
            if (!pImage) continue;

            if((unsigned int)pImage->s() >= _min_image_size * 2 || (unsigned int)pImage->t() >= _min_image_size * 2)
            {
                //如果不是第一级则需要进行纹理压缩，并把纹理转成dds
                unsigned int iscale = 1;
                if(nLevel > 0) iscale = 4;
                OpenSP::sp<osg::Image> image;

                //如果原始纹理有共享
                osg::Texture2D *pOriginalTex = dynamic_cast<osg::Texture2D *>(pGeode->getDrawable(j)->getStateSet()->getTextureAttribute(k, osg::StateAttribute::TEXTURE));

                if (texture_shared)
                {
                    ID id;
                    std::string img_name = pOriginalTex->getImage()->getFileName();

                    //共享纹理已经存在
                    SharedTexs::iterator res = shared_textures.find(img_name);
                    if (res != shared_textures.end())
                    {
                        //查当前level的缩小纹理在不在
                        SharedTexsValue::iterator val = res->second.find(nLevel);

                        if (val == res->second.end())
                        {
                            //不存在，则新建
                            id = ID::genNewID();
                            id.ObjectID.m_nDataSetCode = _dataset_code;
                            id.ObjectID.m_nType = IMAGE_ID;

                            image = CompressImage2DDS(pImage.get(), iscale);
                            _id_imgs.push_back(id);
                            _imgs.push_back(image.get());

                            res->second[nLevel] = SharedImage(id, image.get());

                            image->setFileName(id.toString());
                            pTexture2D->setImage(image.get());
					        pTexture2D->setInternalFormatMode(osg::Texture::USE_S3TC_DXT5_COMPRESSION);
                        }
                        else
                        {
                            id    = val->second.first;
                            image = val->second.second;

                            image->setFileName(id.toString());
                            pTexture2D->setImage(image.get());
					        pTexture2D->setInternalFormatMode(osg::Texture::USE_S3TC_DXT5_COMPRESSION);
                        }
                    }
                    else
                    {
                        id = ID::genNewID();
                        id.ObjectID.m_nDataSetCode = _dataset_code;
                        id.ObjectID.m_nType = IMAGE_ID;

                        image = CompressImage2DDS(pImage.get(), iscale);
                        if (image.get() == NULL)
                        {
                            std::string tmp = pImage->getFileName();
                            tmp += " 被压缩过，无法处理";

                            ModelBuilder::writeLog(tmp.c_str());
                            return -1;
                        }

                        _id_imgs.push_back(id);
                        _imgs.push_back(image.get());

                        SharedTexsValue val;
                        val.insert(std::pair<unsigned int, SharedImage>(nLevel, SharedImage(id, image.get())));
                        shared_textures.insert(std::pair<const std::string, SharedTexsValue>(pOriginalTex->getImage()->getFileName(), val));
                        
                        image->setFileName(id.toString());
                        pTexture2D->setImage(image.get());
					    pTexture2D->setInternalFormatMode(osg::Texture::USE_S3TC_DXT5_COMPRESSION);
                    }
                }
                else
                {
                    image = CompressImage2DDS(pImage.get(), iscale);
    			    pTexture2D->setImage(image.get());
				    pTexture2D->setInternalFormatMode(osg::Texture::USE_S3TC_DXT5_COMPRESSION);
                }

                nLastLevel++;
			}
			else
			{
				OpenSP::sp<osg::Image> image = CompressImage2DDS(pImage.get(), 1);
    			pTexture2D->setImage(image.get());
				pTexture2D->setInternalFormatMode(osg::Texture::USE_S3TC_DXT5_COMPRESSION);

				if (texture_shared)
				{
					ID id = ID::genNewID();
                    id.ObjectID.m_nDataSetCode = _dataset_code;
                    id.ObjectID.m_nType = IMAGE_ID;

					image->setFileName(id.toString());
					_id_imgs.push_back(id);
                    _imgs.push_back(image.get());
				}

			}
		}
	}

    return nLastLevel;
}

bool Model::changeIntoLOD(bool texture_shared, SharedTexs &shared_textures, deudbProxy::IDEUDBProxy *db)
{
    if (db == NULL)
    {
        throw std::string("错误：db不能为空");
    }

    _shared_texture = texture_shared;

    //判断非法数据
    osg::ref_ptr<osg::Geode> pGeode = _mt->getChild(0)->asGeode();

    const osg::BoundingSphere &bs = pGeode->getBound();
    if(bs.radius() < FLT_EPSILON)
    {
        std::string tmp = "错误:物体'" + pGeode->getName() + "'太小，无法处理(忽略此物体)\r\n";
        ModelBuilder::writeLog(tmp.c_str());
        return false;
    }

    const double dblRadiusChecking = computeRadiusOfBoundChecking(pGeode);
    if(dblRadiusChecking < FLT_EPSILON)
    {
        std::string tmp = "错误:物体'" + pGeode->getName() + "'太小，无法处理(忽略此物体)\r\n";
        ModelBuilder::writeLog(tmp.c_str());
        return false;
    }

    //创建LOD
    osgUtil::DistancePixelComputer dpc;

    cmm::math::Point2i resolution = Builder::getResolution();
    dpc.setEnviroment(45.0, resolution.x(), resolution.y());

    cmm::math::Vector2d pixel_size = Builder::getPixelSize();

    const double fMinDis = dpc.calcDistanceByPixelSize(dblRadiusChecking, pixel_size.y());
    const double fMaxDis = dpc.calcDistanceByPixelSize(dblRadiusChecking, pixel_size.x());

    unsigned int nLevel = 0;

    while(true)
    {
        osg::ref_ptr<osg::Geode> pTempGeode = new osg::Geode(*pGeode, osg::CopyOp::DEEP_COPY_ALL);

        int nLastLevel = changeTextureIntoLOD(pGeode, pTempGeode, nLevel, texture_shared, shared_textures);
        if (nLastLevel == -1)
        {
            return false;
        }

        //把一个Geode对象存起来
        ID id;
        if (pTempGeode->getName().length() == 0)
        {
            id = ID::genNewID();
        }
        else
        {
            const char *name = pTempGeode->getName().c_str();
            char *tmp = new char[pTempGeode->getName().length() + 256];
            
            sprintf_s(tmp, 256, "%s%u", name, nLevel);
            CreateHashMD2((unsigned char*)tmp, pTempGeode->getName().length() + 256, (unsigned char*)&id.ObjectID.m_UniqueID);
            delete[] tmp;
        }
        id.ObjectID.m_nDataSetCode = _dataset_code;
        id.ObjectID.m_nType = MODEL_ID;

        //判断是否需要进行多级LOD的计算
        int nMulti = 4;
        float fTemp = fMinDis * powf(nMulti, nLevel);

        if(fTemp > fMaxDis || 0 == nLastLevel)
        {
            addGeode(pTempGeode, id, fMaxDis);
            break;
        }
        else
        {
            addGeode(pTempGeode, id, fTemp);
        }
        nLevel++;
    }

    saveDetail(db);

    return true;
}

void writeNodeCache(deudbProxy::IDEUDBProxy *db, osg::Node &node, const ID &id, bool texture_shared)
{
    if (db == NULL)
    {
        throw std::string("错误：db不能为空");
    }
    else if (id.ObjectID.m_nType != MODEL_ID && id.ObjectID.m_nType != SHARE_MODEL_ID)
    {
        throw std::string("错误：向db写入节点时，ID类型不是模型");
    }

    osg::ref_ptr<osgDB::ReaderWriter::Options> opts = new osgDB::ReaderWriter::Options;
    if(texture_shared)
    {
        opts->setOptionString("noTexturesInIVEFile");
    }

    osgDB::ReaderWriter *pRW = osgDB::Registry::instance()->getReaderWriterForExtension("ive");
    std::ostrstream ss;
    osgDB::ReaderWriter::WriteResult wr = pRW->writeNode(node, ss, opts);
    if (!wr.success())
    {
        throw std::string("错误：节点写入流时失败");
    }

    if (!db->updateBlock(id, ss.str(), ss.pcount()))
    {
        ss.freeze(false);
        throw std::string("错误：节点写入DB时失败");
    }

    ss.freeze(false);
    //解冻，然后在析构函数中释放资源
}

void WriteImageCache(deudbProxy::IDEUDBProxy *db, osg::Image *pImg, const ID &id)
{
    if (pImg == NULL)
    {
        throw std::string("错误：图片不能为空");
    }
    else if (db == NULL)
    {
        throw std::string("错误：db不能为空");
    }
    else if (id.ObjectID.m_nType != IMAGE_ID && id.ObjectID.m_nType != SHARE_IMAGE_ID)
    {
        throw std::string("错误：向db写入Image时，ID类型不是图片");
    }

    //保存纹理
    osgDB::ReaderWriter::Options *pOptions = osgDB::Registry::instance()->getOptions();
    osgDB::ReaderWriter *pRW = osgDB::Registry::instance()->getReaderWriterForExtension("dds");
    std::ostrstream ss;

    osgDB::ReaderWriter::WriteResult wr = pRW->writeImage(*pImg, ss, pOptions);
    if (!wr.success())
    {
        throw std::string("错误：纹理写入流时失败");
    }

    if (!db->addBlock(id, ss.str(), ss.pcount()))
    {
        ss.freeze(false);
        throw std::string("错误：纹理写入DB时失败");
    }
    ss.freeze(false);//解冻，然后在析构函数中释放资源
}

bool Model::writeToDB(deudbProxy::IDEUDBProxy *db)
{
    if (db == NULL)
    {
        throw std::string("错误：db不能为空");
    }

    for(size_t i = 0; i < _geodes.size(); ++i)
    {
        _geodes[i]->setID(_ids[i]);
        writeNodeCache(db, *_geodes[i].get(), _ids[i], _shared_texture);
    }

    for(size_t j = 0; j < _imgs.size(); j++)
    {
        WriteImageCache(db, _imgs[j], _id_imgs[j]);
    }

    return true;
}