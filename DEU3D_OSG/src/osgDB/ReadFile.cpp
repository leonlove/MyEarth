/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/
#include <osg/Notify>
#include <osg/Object>
#include <osg/Image>
#include <osg/Shader>
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TextureRectangle>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <Common/Common.h>
#include <Common/deuImage.h>
#include <strstream>

#include <osg/GL>
#include <osg/GLU>

using namespace osg;
using namespace osgDB;

Object* osgDB::readObjectFile(const std::string& filename,const Options* options, const osg::Referenced *creationInfo)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readObject(filename, options, true, creationInfo);
    if (rr.validObject()) return rr.takeObject();
    if (rr.error()) OSG_WARN << rr.message() << std::endl;
    return NULL;
}
Object* osgDB::readObjectFile(const ID& id,const Options* options, const osg::Referenced *creationInfo)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readObject(id, options, true, creationInfo);
    if (rr.validObject()) return rr.takeObject();
    if (rr.error()) OSG_WARN << rr.message() << std::endl;
    return NULL;
}

Image* osgDB::readImageFile(const std::string& filename,const Options* options, const osg::Referenced *creationInfo)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readImage(filename,options, creationInfo);
    if (rr.validImage()) return rr.takeImage();
    if (rr.error()) OSG_WARN << rr.message() << std::endl;
    return NULL;
}

Image* osgDB::readImageFile(const ID& id,const Options* options, const osg::Referenced *creationInfo)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readImage(id,options, creationInfo);
    if (rr.validImage()) return rr.takeImage();
    if (rr.error()) OSG_WARN << rr.message() << std::endl;
    return NULL;
}

Shader* osgDB::readShaderFile(const std::string& filename,const Options* options, const osg::Referenced *creationInfo)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readShader(filename,options, creationInfo);
    if (rr.validShader()) return rr.takeShader();
    if (rr.error()) OSG_WARN << rr.message() << std::endl;
    return NULL;
}

Shader* osgDB::readShaderFile(const ID& id,const Options* options, const osg::Referenced *creationInfo)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readShader(id,options, creationInfo);
    if (rr.validShader()) return rr.takeShader();
    if (rr.error()) OSG_WARN << rr.message() << std::endl;
    return NULL;
}

HeightField* osgDB::readHeightFieldFile(const std::string& filename,const Options* options, const osg::Referenced *creationInfo)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readHeightField(filename,options, creationInfo);
    if (rr.validHeightField()) return rr.takeHeightField();
    if (rr.error()) OSG_WARN << rr.message() << std::endl;
    return NULL;
}

HeightField* osgDB::readHeightFieldFile(const ID& id,const Options* options, const osg::Referenced *creationInfo)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readHeightField(id,options, creationInfo);
    if (rr.validHeightField()) return rr.takeHeightField();
    if (rr.error()) OSG_WARN << rr.message() << std::endl;
    return NULL;
}

Node* osgDB::readNodeFile(const std::string& filename,const Options* options, const osg::Referenced *creationInfo)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readNode(filename, options, true, creationInfo);
    if (rr.validNode()) return rr.takeNode();
    if (rr.error()) OSG_WARN << rr.message() << std::endl;
    if (rr.notEnoughMemory()) OSG_INFO << "Not enought memory to load file "<<filename << std::endl;
    return NULL;
}

Node* osgDB::readNodeFile(const ID& id,const Options* options, const osg::Referenced *creationInfo)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readNode(id, options, true, creationInfo);
    if (rr.validNode()) return rr.takeNode();
    if (rr.error()) OSG_WARN << rr.message() << std::endl;
    if (rr.notEnoughMemory()) OSG_INFO << "Not enought memory to load file "<< (id.toString()) << std::endl;
    return NULL;
}


osg::ref_ptr<osg::Object> osgDB::readRefObjectFile(const std::string& filename,const Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readObject(filename,options);
    if (rr.validObject()) return osg::ref_ptr<osg::Object>(rr.getObject());
    if (rr.error()) OSG_WARN << rr.message() << std::endl;
    return NULL;
}

osg::ref_ptr<osg::Object> osgDB::readRefObjectFile(const ID& id,const Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readObject(id,options);
    if (rr.validObject()) return osg::ref_ptr<osg::Object>(rr.getObject());
    if (rr.error()) OSG_WARN << rr.message() << std::endl;
    return NULL;
}

osg::ref_ptr<osg::Image> osgDB::readRefImageFile(const std::string& filename,const Options* options, const osg::Referenced *creationInfo)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readImage(filename,options, creationInfo);
    if (rr.validImage()) return osg::ref_ptr<osg::Image>(rr.getImage());
    if (rr.error()) OSG_WARN << rr.message() << std::endl;
    return NULL;
}

osg::ref_ptr<osg::Image> osgDB::readRefImageFile(const ID& id,const Options* options, const osg::Referenced *creationInfo)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readImage(id, options, creationInfo);
    if (rr.validImage()) return osg::ref_ptr<osg::Image>(rr.getImage());
    if (rr.error()) OSG_WARN << rr.message() << std::endl;
    return NULL;
}

osg::ref_ptr<osg::Shader> osgDB::readRefShaderFile(const std::string& filename,const Options* options, const osg::Referenced *creationInfo)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readShader(filename,options, creationInfo);
    if (rr.validShader()) return osg::ref_ptr<osg::Shader>(rr.getShader());
    if (rr.error()) OSG_WARN << rr.message() << std::endl;
    return NULL;
}

osg::ref_ptr<osg::Shader> osgDB::readRefShaderFile(const ID& id,const Options* options, const osg::Referenced *creationInfo)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readShader(id,options, creationInfo);
    if (rr.validShader()) return osg::ref_ptr<osg::Shader>(rr.getShader());
    if (rr.error()) OSG_WARN << rr.message() << std::endl;
    return NULL;
}

osg::ref_ptr<osg::HeightField> osgDB::readRefHeightFieldFile(const std::string& filename,const Options* options, const osg::Referenced *creationInfo)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readHeightField(filename,options, creationInfo);
    if (rr.validHeightField()) return osg::ref_ptr<osg::HeightField>(rr.getHeightField());
    if (rr.error()) OSG_WARN << rr.message() << std::endl;
    return NULL;
}

osg::ref_ptr<osg::HeightField> osgDB::readRefHeightFieldFile(const ID& id,const Options* options, const osg::Referenced *creationInfo)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readHeightField(id, options, creationInfo);
    if (rr.validHeightField()) return osg::ref_ptr<osg::HeightField>(rr.getHeightField());
    if (rr.error()) OSG_WARN << rr.message() << std::endl;
    return NULL;
}

osg::ref_ptr<osg::Node> osgDB::readRefNodeFile(const std::string& filename,const Options* options, const osg::Referenced *creationInfo)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readNode(filename, options, true, creationInfo);
    if (rr.validNode()) return osg::ref_ptr<osg::Node>(rr.getNode());
    if (rr.error()) OSG_WARN << rr.message() << std::endl;
    return NULL;
}

osg::ref_ptr<osg::Node> osgDB::readRefNodeFile(const ID &id,const Options* options, const osg::Referenced *creationInfo)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readNode(id, options, true, creationInfo);
    if (rr.validNode()) return osg::ref_ptr<osg::Node>(rr.getNode());
    if (rr.error()) OSG_WARN << rr.message() << std::endl;
    return NULL;
}

cmm::image::IDEUImage* osgDB::parseImageFromStream(const void *pBuffer, unsigned nLength)
{
    if(!pBuffer || nLength < 1u)    return NULL;

    osgDB::ReaderWriter::ReadResult rr(osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND);
    std::strstream ss((char *)pBuffer, nLength);

    static const std::string strJPG = "jpg";
    static const std::string strPNG = "png";
    static const std::string strBMP = "bmp";
    static const std::string strTIF = "tif";
    static const std::string strDDS = "dds";
    const cmm::image::ImageType eType = cmm::image::getImageStreamType(pBuffer);

    switch(eType)
    {
    case cmm::image::TYPE_JPG:
        {
            osg::ref_ptr<osgDB::ReaderWriter> pJpgRW = osgDB::Registry::instance()->getReaderWriterForExtension(strJPG);
            if(pJpgRW.valid())
            {
                rr = pJpgRW->readImage(ss, NULL);
            }
            break;
        }
    case cmm::image::TYPE_PNG:
        {
            osg::ref_ptr<osgDB::ReaderWriter> pPngRW = osgDB::Registry::instance()->getReaderWriterForExtension(strPNG);
            if(pPngRW.valid())
            {
                rr = pPngRW->readImage(ss, NULL);
            }
            break;
        }
    case cmm::image::TYPE_BMP:
        {
            osg::ref_ptr<osgDB::ReaderWriter> pBmpRW = osgDB::Registry::instance()->getReaderWriterForExtension(strBMP);
            if(pBmpRW.valid())
            {
                rr = pBmpRW->readImage(ss, NULL);
            }
        }
    case cmm::image::TYPE_TIF:
        {
            osg::ref_ptr<osgDB::ReaderWriter> pTifRW = osgDB::Registry::instance()->getReaderWriterForExtension(strTIF);
            if(pTifRW.valid())
            {
                rr = pTifRW->readImage(ss, NULL);
            }
            break;
        }
    default:    break;
    }

    if(!rr.validImage())    return NULL;
    osg::ref_ptr<osg::Image>    pImage = rr.takeImage();
    cmm::image::PixelFormat pFormat;
    GLenum  eFormat = pImage->getPixelFormat();
    switch (eFormat)
    {
    case GL_LUMINANCE:
        {
            pFormat = cmm::image::PF_LUMINANCE;
        }
        break;
    case GL_RGB:
    case(GL_BGR):
        {
            pFormat = cmm::image::PF_RGB;
        }
        break;
    case GL_RGBA:
    case(GL_BGRA):
        {
            pFormat = cmm::image::PF_RGBA;
        }
        break;
    default:
        return NULL;
    }

    osg::ref_ptr<cmm::image::IDEUImage> pCmmImage = cmm::image::createDEUImage();
    const unsigned char* pData = pImage->data();
    unsigned nOutLength = strlen((char*)pData);
    void* pOutData = malloc(nOutLength);
    memcpy(pOutData,pData,nOutLength);
    pCmmImage->attach(pOutData,pImage->s(),pImage->t(),pFormat);
    return pCmmImage.release();
}
