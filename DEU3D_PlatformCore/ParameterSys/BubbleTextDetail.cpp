#include "BubbleTextDetail.h"

#include <IDProvider/Definer.h>

#include <osgUtil/CommonModelCreater.h>
#include <osg/MatrixTransform>
#include <osg/CoordinateSystemNode>

#include <osgText/Text>
#include <osg/ShapeDrawable>
#include <osg/PolygonOffset>

namespace param
{

class BubbleText: public osgText::Text
{
public:
    virtual void drawImplementation(osg::RenderInfo& renderInfo) const
    {
        drawMyImplementation(*renderInfo.getState(), osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    }

    virtual void computeGlyphRepresentation()
    {
        osgText::Text::computeGlyphRepresentation();
        
        _textBB._min -= _pixel_offset;
        _textBB._max -= _pixel_offset;
    }

    void    setBkColor(osg::Vec4 c){_bkColor = c;}
    void    setArrowPos(osg::Vec3 pt){_ptArrow = pt;}
    void    setOffset(osg::Vec3 offset){_pixel_offset = offset;}

protected:
    osg::Vec3 _ptArrow;
    osg::Vec4 _bkColor;
    osg::Vec3 _pixel_offset;

    void drawMyImplementation(osg::State& state, const osg::Vec4& colorMultiplier) const
    {
        unsigned int contextID = state.getContextID();

        state.applyMode(GL_BLEND,true);
    #if 1
        state.applyTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::ON);
    #else
        state.applyTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::OFF);
    #endif
    #if defined(OSG_GL_FIXED_FUNCTION_AVAILABLE)
        state.applyTextureAttribute(0,getActiveFont()->getTexEnv());
    #endif
                                                                                                                                                                                        if (_characterSizeMode!=OBJECT_COORDS || _autoRotateToScreen)
    {
        unsigned int frameNumber = state.getFrameStamp()?state.getFrameStamp()->getFrameNumber():0;
        AutoTransformCache& atc = _autoTransformCache[contextID];
        const osg::Matrix& modelview = state.getModelViewMatrix();
        const osg::Matrix& projection = state.getProjectionMatrix();

        osg::Vec3 newTransformedPosition = _position*modelview;

        int width = atc._width;
        int height = atc._height;

        const osg::Viewport* viewport = state.getCurrentViewport();
        if (viewport)
        {
            width = static_cast<int>(viewport->width());
            height = static_cast<int>(viewport->height());
        }

        bool doUpdate = atc._traversalNumber==-1;
        if (atc._traversalNumber>=0)
        {
            if (atc._modelview!=modelview)
            {
                doUpdate = true;
            }
            else if (width!=atc._width || height!=atc._height)
            {
                doUpdate = true;
            }
            else if (atc._projection!=projection)
            {
                doUpdate = true;
            }
        }
        
        atc._traversalNumber = frameNumber;
        atc._width = width;
        atc._height = height;
        
        if (doUpdate)
        {    
            atc._transformedPosition = newTransformedPosition;
            atc._projection = projection;
            atc._modelview = modelview;

            computePositions(contextID);
        }
        
    }
    
    
        // Ensure that the glyph coordinates have been transformed for
        // this context id.
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> scope(_mtxTextureGlyphQuadMap);

            if ( !_textureGlyphQuadMap.empty() )
            {
                const GlyphQuads& glyphquad = (_textureGlyphQuadMap.begin())->second;
                if ( glyphquad._transformedCoords[contextID].empty() )
                {
                    computePositions(contextID);
                }
            }
        }

        osg::GLBeginEndAdapter& gl = (state.getGLBeginEndAdapter());

        state.Normal(_normal.x(), _normal.y(), _normal.z());

        if (_drawMode & FILLEDBOUNDINGBOX)
        {
        if (_textBB.valid())
        {
        #if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
            state.applyTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::OFF);

            const osg::Matrix& matrix = _autoTransformCache[contextID]._matrix;

            osg::BoundingBox bb = _textBB;
            bb._min += _pixel_offset;
            bb._max += _pixel_offset;

            osg::Vec3 c00(osg::Vec3(bb.xMin(),bb.yMin(),bb.zMin())*matrix);
            osg::Vec3 c10(osg::Vec3(bb.xMax(),bb.yMin(),bb.zMin())*matrix);
            osg::Vec3 c11(osg::Vec3(bb.xMax(),bb.yMax(),bb.zMin())*matrix);
            osg::Vec3 c01(osg::Vec3(bb.xMin(),bb.yMax(),bb.zMin())*matrix);
            
            double middle = (bb.xMin() + bb.xMax()) / 2;
            double width  = (bb.xMax() - bb.xMin()) / 10;
            osg::Vec3 arrow_left(osg::Vec3(middle - width,bb.yMin(),bb.zMin())*matrix);
            osg::Vec3 arrow_right(osg::Vec3(middle + width,bb.yMin(),bb.zMin())*matrix);

            switch(_backdropImplementation)
            {
                case NO_DEPTH_BUFFER:
                    // Do nothing.  The bounding box will be rendered before the text and that's all that matters.
                    break;
                case DEPTH_RANGE:
                    glPushAttrib(GL_DEPTH_BUFFER_BIT);
                    //unsigned int backdrop_index = 0;
                    //unsigned int max_backdrop_index = 8;
                    //const double offset = double(max_backdrop_index - backdrop_index) * 0.003;
                    glDepthRange(0.001, 1.001);
                    break;
                /*case STENCIL_BUFFER:
                    break;*/
                default:
                    glPushAttrib(GL_POLYGON_OFFSET_FILL);
                    glEnable(GL_POLYGON_OFFSET_FILL);
                    glPolygonOffset(0.1f * osg::PolygonOffset::getFactorMultiplier(), 10.0f * osg::PolygonOffset::getUnitsMultiplier() );
            }

            gl.Color4f(_bkColor.r(), _bkColor.g(), _bkColor.b(), _bkColor.a());
            gl.Begin(GL_POLYGON);
                gl.Vertex3fv(arrow_left.ptr());
                gl.Vertex3fv(_ptArrow.ptr());
                gl.Vertex3fv(arrow_right.ptr());
                gl.Vertex3fv(c10.ptr());
                gl.Vertex3fv(c11.ptr());
                gl.Vertex3fv(c01.ptr());
                gl.Vertex3fv(c00.ptr());
            gl.End();

            switch(_backdropImplementation)
            {
                case NO_DEPTH_BUFFER:
                    // Do nothing.
                    break;
                case DEPTH_RANGE:
                    glDepthRange(0.0, 1.0);
                    glPopAttrib();
                    break;
                /*case STENCIL_BUFFER:
                    break;*/
                default:
                    glDisable(GL_POLYGON_OFFSET_FILL);
                    glPopAttrib();
            }
        #else
            OSG_NOTICE<<"Warning: Text::drawImplementation() fillMode FILLEDBOUNDINGBOX not supported"<<std::endl;
        #endif
        }
        }    

        #if 1
        state.applyTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::ON);
            #else
        state.applyTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::OFF);
        #endif
            #if defined(OSG_GL_FIXED_FUNCTION_AVAILABLE)
        state.applyTextureAttribute(0,getActiveFont()->getTexEnv());
        #endif

    if (_drawMode & TEXT)
    {

        state.disableAllVertexArrays();

        // Okay, since ATI's cards/drivers are not working correctly,
        // we need alternative solutions to glPolygonOffset.
        // So this is a pick your poison approach. Each alternative
        // backend has trade-offs associated with it, but with luck,
        // the user may find that works for them.
        if(_backdropType != NONE && _backdropImplementation != DELAYED_DEPTH_WRITES)
        {
            switch(_backdropImplementation)
            {
                case POLYGON_OFFSET:
                    renderWithPolygonOffset(state,colorMultiplier);
                    break;
                case NO_DEPTH_BUFFER:
                    renderWithNoDepthBuffer(state,colorMultiplier);
                    break;
                case DEPTH_RANGE:
                    renderWithDepthRange(state,colorMultiplier);
                    break;
                case STENCIL_BUFFER:
                    renderWithStencilBuffer(state,colorMultiplier);
                    break;
                default:
                    renderWithPolygonOffset(state,colorMultiplier);
            }
        }
        else
        {
            renderWithDelayedDepthWrites(state,colorMultiplier);
        }
    }

        if (_drawMode & BOUNDINGBOX)
        {
            if (_textBB.valid())
            {
                state.applyTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::OFF);

                const osg::Matrix& matrix = _autoTransformCache[contextID]._matrix;

                osg::BoundingBox bb = _textBB;
                bb._min += _pixel_offset;
                bb._max += _pixel_offset;

                osg::Vec3 c00(osg::Vec3(bb.xMin(),bb.yMin(),bb.zMin())*matrix);
                osg::Vec3 c10(osg::Vec3(bb.xMax(),bb.yMin(),bb.zMin())*matrix);
                osg::Vec3 c11(osg::Vec3(bb.xMax(),bb.yMax(),bb.zMin())*matrix);
                osg::Vec3 c01(osg::Vec3(bb.xMin(),bb.yMax(),bb.zMin())*matrix);
            
                double middle = (bb.xMin() + bb.xMax()) / 2;
                double width  = (bb.xMax() - bb.xMin()) / 10;
                osg::Vec3 arrow_left(osg::Vec3(middle - width,bb.yMin(),bb.zMin())*matrix);
                osg::Vec3 arrow_right(osg::Vec3(middle + width,bb.yMin(),bb.zMin())*matrix);
            
                gl.Color4f(colorMultiplier.r()*_textBBColor.r(),colorMultiplier.g()*_textBBColor.g(),colorMultiplier.b()*_textBBColor.b(),colorMultiplier.a()*_textBBColor.a());
                gl.Begin(GL_LINE_LOOP);
                    gl.Vertex3fv(c00.ptr());
                    gl.Vertex3fv(arrow_left.ptr());
                    gl.Vertex3fv(_ptArrow.ptr());
                    gl.Vertex3fv(arrow_right.ptr());
                    gl.Vertex3fv(c10.ptr());
                    gl.Vertex3fv(c11.ptr());
                    gl.Vertex3fv(c01.ptr());
                gl.End();
            }
        }

        if (_drawMode & ALIGNMENT)
        {
            gl.Color4fv(colorMultiplier.ptr());

            float cursorsize = _characterHeight*0.5f;

            const osg::Matrix& matrix = _autoTransformCache[contextID]._matrix;

            osg::Vec3 hl(osg::Vec3(_offset.x()-cursorsize,_offset.y(),_offset.z())*matrix);
            osg::Vec3 hr(osg::Vec3(_offset.x()+cursorsize,_offset.y(),_offset.z())*matrix);
            osg::Vec3 vt(osg::Vec3(_offset.x(),_offset.y()-cursorsize,_offset.z())*matrix);
            osg::Vec3 vb(osg::Vec3(_offset.x(),_offset.y()+cursorsize,_offset.z())*matrix);

            state.applyTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::OFF);
        
            gl.Begin(GL_LINES);
                gl.Vertex3fv(hl.ptr());
                gl.Vertex3fv(hr.ptr());
                gl.Vertex3fv(vt.ptr());
                gl.Vertex3fv(vb.ptr());
            gl.End();
        }    
    }
};

BubbleTextDetail::BubbleTextDetail()
{
    m_bBorderVisible = false;
    m_bBkVisible = false;
    m_bOrientateEye = false;
    m_dBorderWidth = 1.0;
    m_dSize = 20.0;
    m_clrBorder.m_fltA = 1.0;
    m_clrBorder.m_fltR = 1.0;
    m_clrBorder.m_fltG = 1.0;
    m_clrBorder.m_fltB = 1.0;

    m_Color.m_fltA = 1.0;
    m_Color.m_fltR = 1.0;
    m_Color.m_fltG = 1.0;
    m_Color.m_fltB = 1.0;

    m_vOffset.set(0.0, 0.0, 0.0);
    m_strFont = "SIMSUN.ttc";
}

BubbleTextDetail::BubbleTextDetail(unsigned int nDataSetCode) : DynModelDetail(nDataSetCode, DETAIL_BUBBLE_TEXT_ID)
{
    m_bBorderVisible = false;
    m_bBkVisible = false;
    m_bOrientateEye = false;
    m_dBorderWidth = 1.0;
    m_dSize = 20.0;
    m_clrBorder.m_fltA = 1.0;
    m_clrBorder.m_fltR = 1.0;
    m_clrBorder.m_fltG = 1.0;
    m_clrBorder.m_fltB = 1.0;
    m_Color.m_fltA = 1.0;
    m_Color.m_fltR = 1.0;
    m_Color.m_fltG = 1.0;
    m_Color.m_fltB = 1.0;

    m_vOffset.set(0.0, 0.0, 0.0);
    m_strFont = "SIMSUN.ttc";
}

BubbleTextDetail::~BubbleTextDetail()
{

}

bool BubbleTextDetail::fromBson(bson::bsonDocument &bsonDoc)
{
    if(!DynModelDetail::fromBson(bsonDoc))
    {
        return false;
    }

    bson::bsonStringEle *text = dynamic_cast<bson::bsonStringEle*>(bsonDoc.GetElement("Text"));
    if (!text) return false;
    m_strText = text->StrValue();

    bson::bsonStringEle *font = dynamic_cast<bson::bsonStringEle*>(bsonDoc.GetElement("Font"));
    if (!font) return false;
    m_strFont = font->StrValue();

    bson::bsonDoubleEle *size = dynamic_cast<bson::bsonDoubleEle*>(bsonDoc.GetElement("TextSize"));
    if (!size) return false;
    m_dSize = size->DblValue();

    bson::bsonBinaryEle *text_color = dynamic_cast<bson::bsonBinaryEle*>(bsonDoc.GetElement("TextColor"));
    if (!text_color) return false;
    memcpy(&m_clrText, text_color->BinData(), text_color->BinDataLen());

    bson::bsonBinaryEle *color = dynamic_cast<bson::bsonBinaryEle*>(bsonDoc.GetElement("BorderColor"));
    if (!color) return false;
    memcpy(&m_clrBorder, color->BinData(), color->BinDataLen());

    bson::bsonDoubleEle *width = dynamic_cast<bson::bsonDoubleEle*>(bsonDoc.GetElement("BorderWidth"));
    if (!width) return false;
    m_dBorderWidth = width->DblValue();

    bson::bsonBoolEle *visible        = dynamic_cast<bson::bsonBoolEle*>(bsonDoc.GetElement("BorderVisible"));
    if (!visible) return false;
    m_bBorderVisible = visible->BoolValue();

    bson::bsonBoolEle *bk_visible      = dynamic_cast<bson::bsonBoolEle*>(bsonDoc.GetElement("BkVisible"));
    if (!bk_visible) return false;
    m_bBkVisible = bk_visible->BoolValue();

    bson::bsonBoolEle *OrientateEye   = dynamic_cast<bson::bsonBoolEle*>(bsonDoc.GetElement("OrientateEye"));
    if (!OrientateEye) return false;
    m_bOrientateEye = OrientateEye->BoolValue();

    bson::bsonArrayEle *Offset   = dynamic_cast<bson::bsonArrayEle*>(bsonDoc.GetElement("Offset"));
    if (!Offset || Offset->ChildCount() != 3) return true;

    bson::bsonDoubleEle * offset_x = dynamic_cast<bson::bsonDoubleEle*>(Offset->GetElement(0u));
    bson::bsonDoubleEle * offset_y = dynamic_cast<bson::bsonDoubleEle*>(Offset->GetElement(1u));
    bson::bsonDoubleEle * offset_z = dynamic_cast<bson::bsonDoubleEle*>(Offset->GetElement(2u));
    if (offset_x && offset_y && offset_z)
    {
        m_vOffset.x() = offset_x->DblValue();
        m_vOffset.y() = offset_y->DblValue();
        m_vOffset.z() = offset_z->DblValue();
    }

    return true;
}

bool BubbleTextDetail::toBson(bson::bsonDocument &bsonDoc) const
{
    if(!DynModelDetail::toBson(bsonDoc))
    {
        return false;
    }

    if( !bsonDoc.AddStringElement("Text", m_strText.c_str()) ||
        !bsonDoc.AddStringElement("Font", m_strFont.c_str()) ||
        !bsonDoc.AddDblElement("TextSize", m_dSize) ||
        !bsonDoc.AddBinElement("BorderColor", (void*)&m_clrBorder, sizeof(m_clrBorder)) ||
        !bsonDoc.AddBinElement("TextColor", (void*)&m_clrText, sizeof(m_clrText)) ||
        !bsonDoc.AddDblElement("BorderWidth", m_dBorderWidth) ||
        !bsonDoc.AddBoolElement("BorderVisible", m_bBorderVisible) ||
        !bsonDoc.AddBoolElement("BkVisible", m_bBkVisible) ||
        !bsonDoc.AddBoolElement("OrientateEye", m_bOrientateEye))
    {
        return false;
    }

    bson::bsonArrayEle * Offset = dynamic_cast<bson::bsonArrayEle*>(bsonDoc.AddArrayElement("Offset"));
    if (Offset == NULL) return false;

    if (!Offset->AddDblElement(m_vOffset.x()) || !Offset->AddDblElement(m_vOffset.y()) || !Offset->AddDblElement(m_vOffset.z()))
    {
        return false;
    }

    return true;
}

double BubbleTextDetail::getBoundingSphereRadius(void) const
{
    //不确定该怎么设 by cc
    return m_dSize;
}

//创建气泡字
osg::Node *BubbleTextDetail::createDetailNode(const CreationInfo *pInfo) const
{
    const PointCreationInfo *pPointInfo = dynamic_cast<const PointCreationInfo *>(pInfo);
    if(pPointInfo == NULL)
    {
        return NULL;
    }

    osg::Vec3d &vPoint = pPointInfo->m_pPoints->front();

    osg::Vec3d vecTrans;
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    pEllipsoidModel->convertLatLongHeightToXYZ(vPoint.y(), vPoint.x(), vPoint.z(), vecTrans.x(), vecTrans.y(), vecTrans.z());

    osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform;
    pMatrixTransform->setMatrix(osg::Matrix::translate(vecTrans));

    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
    pMatrixTransform->addChild(pGeode);

    osg::ref_ptr<BubbleText> pBubbleText = new BubbleText;
    pGeode->addDrawable(pBubbleText);

    //字体
    osg::ref_ptr<osgText::Font> pFont = osgText::readFontFile(m_strFont);
    pBubbleText->setFont(pFont);

    //字体大小
    pBubbleText->setCharacterSize(m_dSize);

    //字色
    pBubbleText->setColor(osg::Vec4(m_clrText.m_fltR, m_clrText.m_fltG, m_clrText.m_fltB, m_clrText.m_fltA));

    pBubbleText->setBackdropType(osgText::Text::NONE);
    pBubbleText->setAxisAlignment(osgText::Text::SCREEN);
    pBubbleText->setCharacterSizeMode(osgText::Text::SCREEN_COORDS);

    osg::StateSet *pStateset = pBubbleText->getOrCreateStateSet();
    pStateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    pStateset->setMode(GL_BLEND, osg::StateAttribute::ON);

    //边框、背景
    pBubbleText->setDrawMode(osgText::TextBase::TEXT);
    pBubbleText->setAlignment(osgText::TextBase::CENTER_BOTTOM);

    if(m_bBorderVisible && m_bBkVisible)
    {
        pBubbleText->setDrawMode(osgText::TextBase::TEXT | osgText::TextBase::BOUNDINGBOX | osgText::TextBase::FILLEDBOUNDINGBOX);
    }
    else if(m_bBorderVisible)
    {
        pBubbleText->setDrawMode(osgText::TextBase::TEXT | osgText::TextBase::BOUNDINGBOX);
    }
    else if (m_bBkVisible)
    {
        pBubbleText->setDrawMode(osgText::TextBase::TEXT | osgText::TextBase::FILLEDBOUNDINGBOX);
    }

    pBubbleText->setBoundingBoxColor(osg::Vec4(m_clrBorder.m_fltR, m_clrBorder.m_fltG, m_clrBorder.m_fltB, m_clrBorder.m_fltA));

    pBubbleText->setBkColor(osg::Vec4(m_Color.m_fltR, m_Color.m_fltG, m_Color.m_fltB, m_Color.m_fltA));

    pBubbleText->setBoundingBoxMargin(40.0);

    //位置
    pBubbleText->setArrowPos(osg::Vec3(0.0f, 0.0f, 0.0f));
    pBubbleText->setPosition(osg::Vec3(0.0f, 0.0f, 0.0f));
    pBubbleText->setOffset(osg::Vec3(m_vOffset.x(), m_vOffset.y(), m_vOffset.z()));

    //文本
    const std::wstring strStringW = cmm::ANSIToUnicode(m_strText);
    pBubbleText->setText(osgText::String(strStringW.c_str()));
    
    return pMatrixTransform.release();
}

}