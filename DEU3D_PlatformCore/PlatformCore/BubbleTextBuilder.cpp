#include "BubbleTextBuilder.h"
#include <osgText/Text>
#include <osg/ShapeDrawable>
#include <osg/PolygonOffset>

class BubbleText: public osgText::Text
{
public:
    virtual void drawImplementation(osg::RenderInfo& renderInfo) const
    {
        drawMyImplementation(*renderInfo.getState(), osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    }
    void    setBkColor(osg::Vec4 c){_bkColor = c;}
    void    setArrowPos(osg::Vec3 pt){_ptArrow = pt;}

protected:
    osg::Vec3 _ptArrow;
    osg::Vec4 _bkColor;

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

            osg::Vec3 c00(osg::Vec3(_textBB.xMin(),_textBB.yMin(),_textBB.zMin())*matrix);
            osg::Vec3 c10(osg::Vec3(_textBB.xMax(),_textBB.yMin(),_textBB.zMin())*matrix);
            osg::Vec3 c11(osg::Vec3(_textBB.xMax(),_textBB.yMax(),_textBB.zMin())*matrix);
            osg::Vec3 c01(osg::Vec3(_textBB.xMin(),_textBB.yMax(),_textBB.zMin())*matrix);

            double middle = (_textBB.xMin() + _textBB.xMax()) / 2;
            double width  = (_textBB.xMax() - _textBB.xMin()) / 10;
            osg::Vec3 arrow_left(osg::Vec3(middle - width,_textBB.yMin(),_textBB.zMin())*matrix);
            osg::Vec3 arrow_right(osg::Vec3(middle + width,_textBB.yMin(),_textBB.zMin())*matrix);

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

                osg::Vec3 c00(osg::Vec3(_textBB.xMin(),_textBB.yMin(),_textBB.zMin())*matrix);
                osg::Vec3 c10(osg::Vec3(_textBB.xMax(),_textBB.yMin(),_textBB.zMin())*matrix);
                osg::Vec3 c11(osg::Vec3(_textBB.xMax(),_textBB.yMax(),_textBB.zMin())*matrix);
                osg::Vec3 c01(osg::Vec3(_textBB.xMin(),_textBB.yMax(),_textBB.zMin())*matrix);
            
                double middle = (_textBB.xMin() + _textBB.xMax()) / 2;
                double width  = (_textBB.xMax() - _textBB.xMin()) / 10;
                osg::Vec3 arrow_left(osg::Vec3(middle - width,_textBB.yMin(),_textBB.zMin())*matrix);
                osg::Vec3 arrow_right(osg::Vec3(middle + width,_textBB.yMin(),_textBB.zMin())*matrix);
            
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

osg::Node* BubbleTextBuilder::Build(const osg::Vec3& center)
{
    if (m_pBubbleText == NULL) return NULL;

    osg::ref_ptr<BubbleText> text = new BubbleText;

    //文本
    const std::wstring strStringW = cmm::ANSIToUnicode(m_pBubbleText->getContent());
    text->setText(osgText::String(strStringW.c_str()));
    
    //字体
    osg::ref_ptr<osgText::Font> pFont = osgText::readFontFile(m_pBubbleText->getFont());
    text->setFont(pFont);

    //字体大小
    text->setCharacterSize(m_pBubbleText->getTextSize());

    //字色
    osg::Vec4 color(m_pBubbleText->getTextColor().m_fltR,
                    m_pBubbleText->getTextColor().m_fltG,
                    m_pBubbleText->getTextColor().m_fltB,
                    m_pBubbleText->getTextColor().m_fltA);
    text->setColor(color);

    text->setBackdropType(osgText::Text::OUTLINE);
    text->setAxisAlignment(osgText::Text::SCREEN);
    text->setCharacterSizeMode(osgText::Text::SCREEN_COORDS);
    text->setAlignment(osgText::Text::CENTER_CENTER);
   
    osg::StateSet* stateset = new osg::StateSet;
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
    text->setStateSet(stateset);

    //边框、背景
    text->setDrawMode(osgText::TextBase::TEXT);
    
    if (m_pBubbleText->getBorderVisible() && m_pBubbleText->getBkVisible()) 
    {
        text->setDrawMode(osgText::TextBase::TEXT | osgText::TextBase::BOUNDINGBOX | osgText::TextBase::FILLEDBOUNDINGBOX);
    }
    else if (m_pBubbleText->getBorderVisible())
    {
        text->setDrawMode(osgText::TextBase::TEXT | osgText::TextBase::BOUNDINGBOX);
    }
    else if (m_pBubbleText->getBkVisible())
    {
        text->setDrawMode(osgText::TextBase::TEXT | osgText::TextBase::FILLEDBOUNDINGBOX);
    }

    osg::Vec4 colorBB(  m_pBubbleText->getBorderColor().m_fltR,
                        m_pBubbleText->getBorderColor().m_fltG,
                        m_pBubbleText->getBorderColor().m_fltB,
                        m_pBubbleText->getBorderColor().m_fltA);
    text->setBoundingBoxColor(colorBB);
    
    osg::Vec4 colorBK(  m_pBubbleText->getDynModelColor().m_fltR,
                        m_pBubbleText->getDynModelColor().m_fltG,
                        m_pBubbleText->getDynModelColor().m_fltB,
                        m_pBubbleText->getDynModelColor().m_fltA);
    text->setBkColor(colorBK);

    text->setBoundingBoxMargin(40.0);

     //位置
    text->setArrowPos(center);
    text->setPosition(osg::Vec3(center.x(), center.y(), center.z() + 1));

    osg::ref_ptr<osg::Geode> g = new osg::Geode;
    g->addDrawable(text);
    return g.release();
}

