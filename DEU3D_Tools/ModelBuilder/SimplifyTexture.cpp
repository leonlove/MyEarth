
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>

#include <Windows.h>
#include "Common\Common.h"
#include <IL\ilu.h>
#include "ModelBuilder.h"
#include "SimplifyTexture.h"

double computeScaleRatio(const osg::Viewport& W, const osg::Matrixd& P, const osg::Matrixd& M)
{
    const double P00 = P(0,0)*W.width()*0.5;
    const double P20_00 = P(2,0)*W.width()*0.5 + P(2,3)*W.width()*0.5;
    osg::Vec3 scale_00(M(0,0)*P00 + M(0,2)*P20_00,
        M(1,0)*P00 + M(1,2)*P20_00,
        M(2,0)*P00 + M(2,2)*P20_00);

    // scaling for vertical pixels
    const double P10 = P(1,1)*W.height()*0.5;
    const double P20_10 = P(2,1)*W.height()*0.5 + P(2,3)*W.height()*0.5;
    osg::Vec3 scale_10(M(0,1)*P10 + M(0,2)*P20_10,
        M(1,1)*P10 + M(1,2)*P20_10,
        M(2,1)*P10 + M(2,2)*P20_10);

    const double scaleRatio  = 0.7071067811 / sqrtf(scale_00.length2()+scale_10.length2());

    return scaleRatio;
}

double computeDistanceByPixelSize(double dblRadius, double fPixelSize, unsigned int nResolutionH, unsigned int nResolutionV)
{
    osg::ref_ptr<osg::Viewport> pViewport = new osg::Viewport(0, 0, nResolutionH, nResolutionV);
    const osg::Matrixd mtxProjection(osg::Matrixd::perspective(45.0, 1.0, 0.1, 10000.0));


    const osg::Matrixd mtxView(osg::Matrixd::lookAt(osg::Vec3d(0.0, 0.0, -10000.0), osg::Vec3d(0.0, 0.0, 0.0), osg::Vec3d(0.0, 1.0, 0.0)));

    const double fScaleRatio = computeScaleRatio(*pViewport, mtxProjection, mtxView);

    const double ft = dblRadius / fPixelSize;
    const double fDis = ft / fScaleRatio;

    return fDis;
}

bool SimplifyTexture2Color(osg::Image *pImage, osg::Vec4 &color)
{
    if (pImage == NULL) return false;

    switch (pImage->getDataType())
    {
        case(GL_BYTE):          
        case(GL_UNSIGNED_BYTE): 
        case(GL_SHORT):         
        case(GL_UNSIGNED_SHORT):
        case(GL_INT):           
        case(GL_UNSIGNED_INT):
        case(GL_FLOAT):
            break;
        default:
            return false;
    }

    switch(pImage->getPixelFormat())
    {
        case(GL_DEPTH_COMPONENT):
        case(GL_LUMINANCE):      
        case(GL_ALPHA):          
        case(GL_LUMINANCE_ALPHA):
        case(GL_RGB):            
        case(GL_RGBA):           
        case(GL_BGR):            
        case(GL_BGRA):    
            break;
        default:
            return false;
    }

    color.set(0.0f, 0.0f, 0.0f, 0.0f);

    for(int i = 0; i < pImage->t(); i++)
    {
        for(int j = 0; j < pImage->s(); j++)
        {
            color += pImage->getColor(j, i);
        }
    }

    size_t pixels = pImage->t() * pImage->s();   

    color._v[0] /= pixels;
    color._v[1] /= pixels;
    color._v[2] /= pixels;
    color._v[3] /= pixels;

    return true;
}

int Do_2_Log(int iValue)
{
    int index = 0;
    while (iValue > 0)
    {
        index++;
        iValue = iValue / 2;
    }
    return index;
}

GLboolean legalFormat(GLenum format)
{
    switch(format) {
      case GL_COLOR_INDEX:
      case GL_STENCIL_INDEX:
      case GL_DEPTH_COMPONENT:
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_RGB:
      case GL_RGBA:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
      case GL_BGR:
      case GL_BGRA:
        return GL_TRUE;
      default:
        return GL_FALSE;
    }
}

GLboolean legalType(GLenum type)
{
    switch(type) {
      case GL_BITMAP:
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
      case GL_UNSIGNED_BYTE_3_3_2:
      case GL_UNSIGNED_BYTE_2_3_3_REV:  
      case GL_UNSIGNED_SHORT_5_6_5:
      case GL_UNSIGNED_SHORT_5_6_5_REV:
      case GL_UNSIGNED_SHORT_4_4_4_4:
      case GL_UNSIGNED_SHORT_4_4_4_4_REV:
      case GL_UNSIGNED_SHORT_5_5_5_1:
      case GL_UNSIGNED_SHORT_1_5_5_5_REV:
      case GL_UNSIGNED_INT_8_8_8_8:
      case GL_UNSIGNED_INT_8_8_8_8_REV:
      case GL_UNSIGNED_INT_10_10_10_2:
      case GL_UNSIGNED_INT_2_10_10_10_REV:
         return GL_TRUE;
      default:
        return GL_FALSE;
    }
}

bool tryToWriteImageFile(const osg::Image *pImage, const std::string &imgFile)
{
    __try{
        osgDB::writeImageFile(*pImage, imgFile);
        return true;
    }__except(EXCEPTION_EXECUTE_HANDLER)
    {
        if (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)
        {
            //ModelBuilder::writeLog("压缩纹理写文件时出错，使用原始图片");
            return false;
        }
        return true;
    }
}

osg::ref_ptr<osg::Image> CompressImage2DDS(osg::Image *pImage, const unsigned int iscale, const std::string &strAppPath)
{
    if (pImage == NULL)
    {
        throw std::string("错误：图片不能为空");
    }

    //获取纹理图片的大小
    int iImgWidth = pImage->s();
    int iImgHeight = pImage->t();
    if(iImgWidth > 32 || iImgHeight > 32)
    {
        iImgWidth /= iscale;
        iImgHeight /= iscale;
    }
    //如果纹理不是2的幂，则把数据归为2的幂
    int iWidthIndex = Do_2_Log(iImgWidth);
    int iHeightIndex = Do_2_Log(iImgHeight);
    if (iWidthIndex >= 1) iWidthIndex -=1;
    if (iHeightIndex >= 1) iHeightIndex -=1;
    iImgWidth = pow(2.0f, iWidthIndex);
    iImgHeight = pow(2.0f, iHeightIndex);

    //设纹理图片的临时文件夹和临时文件路径
    std::string dirpath = strAppPath;
    if(dirpath.back() != '/' && dirpath.back() != '\\')
    {
        dirpath += "\\";
    }
    std::string strFileName = pImage->getFileName();
    std::replace(strFileName.begin(), strFileName.end(), '/', '_');
    std::replace(strFileName.begin(), strFileName.end(), '\\', '_');
    std::string imgFile = dirpath + "\\" + strFileName;
    std::string ddsFile = imgFile.substr(0, imgFile.size() - 4);
    ddsFile.append(".dds");

    //判断pImage是否为dds纹理
    bool bDDS = false;
    
    if (osgDB::getFileExtension(pImage->getFileName()) == "dds")
    {
        //如果缩放的倍数为1，且pImage为dds纹理，则不需要进行缩放了
        //pImage->s() == iImgWidth && pImage->t() == iImgHeight 
        //用原有尺寸跟归为2的幂的尺寸进来比较来判断原来数据是否为2的幂
        //if(iscale == 1 && pImage->s() == iImgWidth && pImage->t() == iImgHeight) 
  //      {
  //          osg::ref_ptr<osg::Image> image = new osg::Image(*pImage, osg::CopyOp::DEEP_COPY_ALL);
  //          return image.release();
  //      }

        imgFile = ddsFile;
        bDDS = true;
    }

    if (pImage->isCompressed())
    {
        //return NULL;
    }

    if (!tryToWriteImageFile(pImage, imgFile))
    {
        osg::ref_ptr<osg::Image> image = new osg::Image(*pImage, osg::CopyOp::DEEP_COPY_ALL);
        image->scaleImage(iImgWidth, iImgHeight, pImage->r());
        image->setFileName(imgFile);
        remove(imgFile.c_str());
        return image.release();
    }
    
    static bool ilInited = false;
    if (!ilInited)
    {
        ilInit();
        iluInit();
        ilInited = true;
    }

    ILuint    ImgId;
    ilGenImages(1, &ImgId);

    if (!ilLoadImage(imgFile.c_str()))
    {
        osg::ref_ptr<osg::Image> image = new osg::Image(*pImage, osg::CopyOp::DEEP_COPY_ALL);
        image->scaleImage(iImgWidth, iImgHeight, pImage->r());
        image->setFileName(imgFile);
        remove(imgFile.c_str());
        return image.release();
    }

    iluImageParameter(ILU_FILTER, ILU_BILINEAR);
    iluScale(iImgWidth, iImgHeight, 0);
    ilEnable(IL_FILE_OVERWRITE);

    ilSetInteger(IL_DXTC_FORMAT, IL_DXT5);
    ilSaveImage(ddsFile.c_str());
    ilDeleteImages(1, &ImgId);

    //把新的纹理读进来并返回
    osg::ref_ptr<osg::Image> image = osgDB::readImageFile(ddsFile);

    if (image.get() == NULL)
    {
        image = new osg::Image(*pImage, osg::CopyOp::DEEP_COPY_ALL);
        image->scaleImage(iImgWidth, iImgHeight, pImage->r());
        image->setFileName(imgFile);
    }
    else
    {
        //如果不是dds纹理则需要进行纹理翻转
        if(!bDDS) image->flipVertical();
    }
    
    //删除临时文件
    remove(imgFile.c_str());
    remove(ddsFile.c_str());
    //if(imgFile != ddsFile) remove(ddsFile.c_str());

    return image.release();
}

osg::ref_ptr<osg::Image> CompressImage2DDS(osg::Image *pImage, const unsigned int iscale)
{
    return CompressImage2DDS(pImage, iscale, cmm::GetAppPath(false));
}


