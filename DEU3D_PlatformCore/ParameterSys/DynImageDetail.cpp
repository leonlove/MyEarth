#include "DynImageDetail.h"

#include <IDProvider/Definer.h>
#include <osgDB/ReadFile>
#include <osgText/Text>
#include <osg/MatrixTransform>
#include <osg/CoordinateSystemNode>
#include <osg/AutoTransform>
#include <osg/SharedObjectPool>
#include <osg/Depth>

namespace param
{

DynImageDetail::DynImageDetail()
{
    init();
}

DynImageDetail::DynImageDetail(unsigned int nDataSetCode) : Detail(ID(nDataSetCode, DETAIL_DYN_IMAGE_ID))
{
    init();
}

void DynImageDetail::init(void)
{
    m_fltImageWidth  = 1.0f;
    m_fltImageHeight = 1.0f;
    m_bOrientateEye = true;
    m_bLockSize = true;
    m_strTextFont = "SIMSUN.TTC";
    m_strText = "";
    m_fltTextSize = 16.0f;
}

DynImageDetail::~DynImageDetail(void)
{

}

bool DynImageDetail::fromBson(bson::bsonDocument &bsonDoc)
{
    if(!Detail::fromBson(bsonDoc))
    {
        return false;
    }

    //imageID
    {
        bson::bsonElement *pEle = bsonDoc.GetElement("ImageID");
        if(pEle != NULL)
        {
            bson::bsonElementType eType = pEle->GetType();
            if(eType == bson::bsonStringType)
            {
                bson::bsonStringEle *pStringEle = dynamic_cast<bson::bsonStringEle *>(pEle);
                m_ImgID = ID::genIDfromString(pStringEle->StrValue());
            }
            else if(eType == bson::bsonBinType)
            {
                bson::bsonBinaryEle *pBinEle = dynamic_cast<bson::bsonBinaryEle *>(pEle);
                m_ImgID = ID::genIDfromBinary(pBinEle->BinData(), pBinEle->BinDataLen());
            }
        }
    }

    //imagePath
    {
        bson::bsonStringEle *pStringEle = dynamic_cast<bson::bsonStringEle *>(bsonDoc.GetElement("ImageFilePath"));
        if(pStringEle != NULL)
        {
            m_strImagePath = pStringEle->StrValue();
        }

        pStringEle = dynamic_cast<bson::bsonStringEle *>(bsonDoc.GetElement("Text"));
        if(pStringEle != NULL)
        {
            m_strText = pStringEle->StrValue();
        }

        pStringEle = dynamic_cast<bson::bsonStringEle *>(bsonDoc.GetElement("TextFont"));
        m_strTextFont = pStringEle->StrValue();
    }

    
    {
        bson::bsonDoubleEle *pDoubleEle = NULL;
        pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("ImageWidth"));
        if(pDoubleEle != NULL)
        {
            m_fltImageWidth = pDoubleEle->DblValue();
        }

        pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("ImageHeight"));
        if(pDoubleEle != NULL)
        {
            m_fltImageHeight = pDoubleEle->DblValue();
        }

        pDoubleEle = dynamic_cast<bson::bsonDoubleEle *>(bsonDoc.GetElement("TextSize"));
        if(pDoubleEle != NULL)
        {
            m_fltTextSize = pDoubleEle->DblValue();
        }
    }

    {
        bson::bsonBoolEle *pBoolEle = NULL;
        pBoolEle = dynamic_cast<bson::bsonBoolEle *>(bsonDoc.GetElement("OrientateEye"));
        if(pBoolEle != NULL)
        {
            m_bOrientateEye = pBoolEle->BoolValue();
        }

        pBoolEle = dynamic_cast<bson::bsonBoolEle *>(bsonDoc.GetElement("LockSize"));
        if(pBoolEle != NULL)
        {
            m_bLockSize = pBoolEle->BoolValue();
        }
    }

    return true;
}

bool DynImageDetail::toBson(bson::bsonDocument &bsonDoc) const
{
    if(!Detail::toBson(bsonDoc))
    {
        return false;
    }

    if(!bsonDoc.AddBinElement("ImageID", (void *)&m_ImgID, sizeof(ID)) ||
        !bsonDoc.AddStringElement("ImageFilePath", m_strImagePath.c_str()) ||
        !bsonDoc.AddStringElement("Text", m_strText.c_str()) ||
        !bsonDoc.AddStringElement("TextFont", m_strTextFont.c_str()) ||
        !bsonDoc.AddDblElement("ImageWidth", m_fltImageWidth) ||
        !bsonDoc.AddDblElement("ImageHeight", m_fltImageHeight) ||
        !bsonDoc.AddInt32Element("TextSize", m_fltTextSize) ||
        !bsonDoc.AddBoolElement("OrientateEye", m_bOrientateEye) ||
        !bsonDoc.AddBoolElement("LockSize", m_bLockSize))
    {
        return false;
    }

    return true;
}

double DynImageDetail::getBoundingSphereRadius(void) const
{
    const double dblRadius = sqrt(m_fltImageWidth * m_fltImageWidth + m_fltImageHeight * m_fltImageHeight);
    return dblRadius;
}

osg::Node *DynImageDetail::createDetailNode(const CreationInfo *pInfo) const
{

    if(!m_ImgID.isValid() && m_strImagePath.empty() && m_strText.empty())
    {
        return NULL;
    }

    const PointCreationInfo *pPointInfo = dynamic_cast<const PointCreationInfo *>(pInfo);
    if(pPointInfo == NULL)
    {
        return NULL;
    }

    //ÉÏÇò¾ØÕó
    const osg::Vec3d &vPoint = pPointInfo->m_pPoints->front();

    osg::Vec3d vecTrans;
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    pEllipsoidModel->convertLatLongHeightToXYZ(vPoint.y(), vPoint.x(), vPoint.z(), vecTrans.x(), vecTrans.y(), vecTrans.z());

    osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform;
    pMatrixTransform->setMatrix(osg::Matrix::translate(vecTrans));

    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;

    osg::ref_ptr<osg::AutoTransform> pAutoTranseform = new osg::AutoTransform;
    pAutoTranseform->setAutoRotateMode(m_bOrientateEye ? osg::AutoTransform::ROTATE_TO_SCREEN : osg::AutoTransform::NO_ROTATION);
    pAutoTranseform->setAutoScaleToScreen(m_bLockSize);
    pAutoTranseform->setCullingActive(false);

    pAutoTranseform->addChild(pGeode.get());

    osg::ref_ptr<osg::Image> pImage;
    if(m_ImgID.isValid())
    {
        pImage = osgDB::readImageFile(m_ImgID);
    }
    else if(!m_strImagePath.empty())
    {
        pImage = osgDB::readImageFile(m_strImagePath);
    }

    if(pImage.valid())
    {
        osg::ref_ptr<osg::Geometry> pImgGeometry = new osg::Geometry;
        osg::ref_ptr<osg::Texture> pTexture;

        osg::SharedObjectPool *pSharedObjectPool = osg::SharedObjectPool::instance();
        if(!pSharedObjectPool->findObject(pImage, pTexture))
        {
            osg::ref_ptr<osg::Texture2D> pTexture2D = new osg::Texture2D();
            pTexture2D->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
            pTexture2D->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
            pTexture2D->setResizeNonPowerOfTwoHint(false);
            pTexture2D->setImage(pImage.get());

            pSharedObjectPool->addTexture(pImage, pTexture2D);

            pTexture = pTexture2D;
        }

        osg::StateSet *pStateSet = new osg::StateSet;
        pStateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
        pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        pStateSet->setMode(GL_BLEND, osg::StateAttribute::ON); // redundant. AnnotationNode sets blending.
        pStateSet->setTextureAttributeAndModes(0, pTexture, osg::StateAttribute::ON);

        pImgGeometry->setUseVertexBufferObjects(true);

        pImgGeometry->setStateSet(pStateSet);

        osg::ref_ptr<osg::Vec3Array> pVertex = new osg::Vec3Array(4);

        (*pVertex)[0].set(-m_fltImageWidth * 0.5f, -m_fltImageHeight * 0.5f, 0.0f);
        (*pVertex)[1].set( m_fltImageWidth * 0.5f, -m_fltImageHeight * 0.5f, 0.0f);
        (*pVertex)[2].set( m_fltImageWidth * 0.5f,  m_fltImageHeight * 0.5f, 0.0f);
        (*pVertex)[3].set(-m_fltImageWidth * 0.5f,  m_fltImageHeight * 0.5f, 0.0f);

        pImgGeometry->setVertexArray(pVertex.get());
        if(pVertex->getVertexBufferObject())
        {
            pVertex->getVertexBufferObject()->setUsage(GL_STATIC_DRAW_ARB);
        }

        osg::ref_ptr<osg::Vec2Array> pTexcoords = new osg::Vec2Array(4);
        (*pTexcoords)[0].set(0, 0);
        (*pTexcoords)[1].set(1, 0);
        (*pTexcoords)[2].set(1, 1);
        (*pTexcoords)[3].set(0, 1);
        pImgGeometry->setTexCoordArray(0, pTexcoords.get());

        osg::ref_ptr<osg::Vec4Array> pColorArray = new osg::Vec4Array(1);
        (*pColorArray)[0].set(1.0f,1.0f,1.0,1.0f);
        pImgGeometry->setColorArray(pColorArray.get());
        pImgGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

        pImgGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));
        if (pImgGeometry.valid())
        {
            pGeode->addDrawable(pImgGeometry);
        }
    }

    if(!m_strText.empty())
    {
        osg::ref_ptr<osgText::Text> pText = new osgText::Text();
        const std::wstring strStringW = cmm::ANSIToUnicode(m_strText);
        pText->setText(osgText::String(strStringW.c_str()));

        osg::ref_ptr<osgText::Font> pTextFont = osgText::readFontFile(m_strTextFont);
        pText->setFont(pTextFont);

        // osgText::Text turns on depth writing by default, even if you turned it off..
        pText->setEnableDepthWrites(false);

        pText->setLayout(osgText::TextBase::LEFT_TO_RIGHT);
        pText->setAlignment(osgText::Text::CENTER_TOP);

        pText->setPosition(osg::Vec3(0.0f, m_fltImageHeight + m_fltTextSize, 0.0f));

        pText->setAutoRotateToScreen(false);
        pText->setCharacterSizeMode(osgText::Text::OBJECT_COORDS);
        pText->setCharacterSize(m_fltTextSize);

        pText->setColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
        pText->setBackdropColor(osg::Vec4(0.3f, 0.3f, 0.3f, 1.0f));
        pText->setBackdropType(osgText::Text::OUTLINE);

        //pText->getStateSet()->setRenderBinToInherit();
        pGeode->addDrawable(pText.get());
    }
    

    //pGeode->getOrCreateStateSet()->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false), 1);
    
    pGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    pMatrixTransform->addChild(pAutoTranseform);

    return pMatrixTransform.release();
}

}