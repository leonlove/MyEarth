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
#include <osgUtil/IncrementalCompileOperation>

#include <osg/Drawable>
#include <osg/Notify>
#include <osg/Timer>
#include <osg/GLObjects>
#include <osg/Depth>
#include <osg/ColorMask>

#include <OpenThreads/ScopedLock>

#include <algorithm>
#include <iterator>
#include <stdlib.h>
#include <string.h>

namespace osgUtil 
{


// TODO
// priority of CompileSets
// isCompiled
// time estimation
// early completion
// needs compile given time slot
// custom CompileData elements
// pruneOldRequestsAndCheckIfEmpty()
// Use? :
//                     #if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
//                        GLint p;
//                        glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_RESIDENT, &p);
//                    #endif

/////////////////////////////////////////////////////////////////
//
// CollectStateToCompile
//
StateToCompile::StateToCompile(GLObjectsVisitor::Mode mode):
    osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
    _mode(mode),
    _assignPBOToImages(false)
{
}

void StateToCompile::apply(osg::Node& node)
{
    if (node.getStateSet())
    {
        apply(*(node.getStateSet()));
    }

    traverse(node);
}

void StateToCompile::apply(osg::Geode& node)
{
    if (node.getStateSet())
    {
        apply(*(node.getStateSet()));
    }

    for(unsigned int i=0;i<node.getNumDrawables();++i)
    {
        osg::Drawable* drawable = node.getDrawable(i);
        if (drawable)
        {
            apply(*drawable);
            if (drawable->getStateSet())
            {
                apply(*(drawable->getStateSet()));
            }
        }
    }
}

void StateToCompile::apply(osg::Drawable& drawable)
{
    if (_drawablesHandled.count(&drawable)!=0) return;

    _drawablesHandled.insert(&drawable);

    if (_mode&GLObjectsVisitor::SWITCH_OFF_DISPLAY_LISTS)
    {
        drawable.setUseDisplayList(false);
    }

    if (_mode&GLObjectsVisitor::SWITCH_ON_DISPLAY_LISTS)
    {
        drawable.setUseDisplayList(true);
    }

    if (_mode&GLObjectsVisitor::SWITCH_ON_VERTEX_BUFFER_OBJECTS)
    {
        drawable.setUseVertexBufferObjects(true);
    }

    if (_mode&GLObjectsVisitor::SWITCH_OFF_VERTEX_BUFFER_OBJECTS)
    {
        drawable.setUseVertexBufferObjects(false);
    }

    if (_mode&GLObjectsVisitor::COMPILE_DISPLAY_LISTS &&
        (drawable.getUseDisplayList() || drawable.getUseVertexBufferObjects()))
    {
        _drawables.insert(&drawable);
    }
}

void StateToCompile::apply(osg::StateSet& stateset)
{
    if (_statesetsHandled.count(&stateset)!=0) return;

    _statesetsHandled.insert(&stateset);

    if (_mode & GLObjectsVisitor::COMPILE_STATE_ATTRIBUTES)
    {
        osg::Program* program = dynamic_cast<osg::Program*>(stateset.getAttribute(osg::StateAttribute::PROGRAM));
        if (program)
        {
            _programs.insert(program);
        }

        const osg::StateSet::TextureAttributeList& tal = stateset.getTextureAttributeList();

#if 0
        if (tal.size()>1)
        {
            tal.erase(tal.begin()+1,tal.end());
        }
#endif
        for(osg::StateSet::TextureAttributeList::const_iterator itr = tal.begin();
            itr != tal.end();
            ++itr)
        {
            const osg::StateSet::AttributeList& al = *itr;
            osg::StateAttribute::TypeMemberPair tmp(osg::StateAttribute::TEXTURE,0);
            osg::StateSet::AttributeList::const_iterator texItr = al.find(tmp);
            if (texItr != al.end())
            {
                osg::Texture* texture = dynamic_cast<osg::Texture*>(texItr->second.first.get());
                if (texture)
                {
                    if (_textures.count(texture)==0)
                    {
                        apply(*texture);
                    }
                }
            }
        }
    }
}

void StateToCompile::apply(osg::Texture& texture)
{
    if (_assignPBOToImages)
    {
        unsigned int numRequringPBO = 0;
        osg::ref_ptr<osg::PixelBufferObject> pbo = 0;
        for(unsigned int i=0; i<texture.getNumImages(); ++i)
        {
            osg::Image* image = texture.getImage(i);
            if (image)
            {
                if (image->getPixelBufferObject())
                {
                    pbo = image->getPixelBufferObject();
                }
                else
                {
                    ++numRequringPBO;
                }
            }
        }
        if (numRequringPBO>0)
        {
            // assign pbo
            if (!pbo)
            {
                if (!_pbo) _pbo = new osg::PixelBufferObject;
                pbo = _pbo;
            }

            for(unsigned int i=0; i<texture.getNumImages(); ++i)
            {
                osg::Image* image = texture.getImage(i);
                if (image)
                {
                    if (!image->getPixelBufferObject())
                    {
                        //OSG_NOTICE<<"Assigning PBO"<<std::endl;
                        pbo->setCopyDataAndReleaseGLBufferObject(true);
                        pbo->setUsage(GL_DYNAMIC_DRAW_ARB);
                        image->setPixelBufferObject(pbo.get());
                    }
                }
            }
        }
    }

    _textures.insert(&texture);
}

/////////////////////////////////////////////////////////////////
//
// CompileOps
//
IncrementalCompileOperation::CompileDrawableOp::CompileDrawableOp(osg::Drawable* drawable):
    _drawable(drawable)
{
}

bool IncrementalCompileOperation::CompileDrawableOp::compile(CompileInfo& compileInfo, OpenThreads::Block &blockWait)
{
    //OSG_NOTICE<<"CompileDrawableOp::compile(..)"<<std::endl;
    _drawable->compileGLObjects(compileInfo);
    return true;
}

IncrementalCompileOperation::CompileTextureOp::CompileTextureOp(osg::Texture* texture):
    _texture(texture)
{
}

bool IncrementalCompileOperation::CompileTextureOp::compile(CompileInfo& compileInfo, OpenThreads::Block &blockWait)
{
    blockWait.block();
    _texture->apply(*compileInfo.getState());
    return true;
}

IncrementalCompileOperation::CompileProgramOp::CompileProgramOp(osg::Program* program):
    _program(program)
{
}

bool IncrementalCompileOperation::CompileProgramOp::compile(CompileInfo& compileInfo, OpenThreads::Block &blockWait)
{
    //OSG_NOTICE<<"CompileProgramOp::compile(..)"<<std::endl;
    _program->apply(*compileInfo.getState());
    return true;
}

IncrementalCompileOperation::CompileInfo::CompileInfo(osg::GraphicsContext* context, IncrementalCompileOperation* ico)
{
    setState(context->getState());
    incrementalCompileOperation = ico;
}


/////////////////////////////////////////////////////////////////
//
// CompileList
//
IncrementalCompileOperation::CompileList::CompileList()
{
}

IncrementalCompileOperation::CompileList::~CompileList()
{
}

void IncrementalCompileOperation::CompileList::add(CompileOp* compileOp)
{
    _compileOps.push_back(compileOp);
}

bool IncrementalCompileOperation::CompileList::compile(CompileInfo& compileInfo, OpenThreads::Block &blockWait)
{
//#define USE_TIME_ESTIMATES
    
    for(CompileOps::iterator itr = _compileOps.begin();
        itr != _compileOps.end(); )
    {
        #ifdef USE_TIME_ESTIMATES
        osg::ElapsedTime timer;
        #endif

        CompileOps::iterator saved_itr(itr);
        ++itr;
        if ((*saved_itr)->compile(compileInfo, blockWait))
        {
            glFlush();
            _compileOps.erase(saved_itr);
        }
    }
    return empty();
}

/////////////////////////////////////////////////////////////////
//
// CompileSet
//
void IncrementalCompileOperation::CompileSet::buildCompileMap(ContextSet& contexts, StateToCompile& stc)
{
    if (contexts.empty() || stc.empty()) return;

    if (stc.empty()) return;

    for(ContextSet::iterator itr = contexts.begin();
        itr != contexts.end();
        ++itr)
    {
        CompileList& cl = _compileMap[*itr];
        for(StateToCompile::DrawableSet::iterator ditr = stc._drawables.begin();
            ditr != stc._drawables.end();
            ++ditr)
        {
            cl.add(*ditr);
        }

        for(StateToCompile::TextureSet::iterator titr = stc._textures.begin();
            titr != stc._textures.end();
            ++titr)
        {
            cl.add(*titr);
        }

        for(StateToCompile::ProgramSet::iterator pitr = stc._programs.begin();
            pitr != stc._programs.end();
            ++pitr)
        {
            cl.add(*pitr);
        }
    }
}

void IncrementalCompileOperation::CompileSet::buildCompileMap(ContextSet& contexts, GLObjectsVisitor::Mode mode)
{
    if (contexts.empty() || !_subgraphToCompile) return;

    StateToCompile stc(mode);
    _subgraphToCompile->accept(stc);

    buildCompileMap(contexts, stc);
}

bool IncrementalCompileOperation::CompileSet::compile(CompileInfo& compileInfo, OpenThreads::Block &blockWait)
{
    CompileList& compileList = _compileMap[compileInfo.getState()->getGraphicsContext()];
    if (compileList.empty())
    {
        return true;
    }

    return compileList.compile(compileInfo, blockWait);
}

/////////////////////////////////////////////////////////////////
//
// IncrementalCompileOperation
//
IncrementalCompileOperation::IncrementalCompileOperation():
    osg::GraphicsOperation("IncrementalCompileOperation",true)
{
    _blockAllowCompiling.set(true);
}

IncrementalCompileOperation::~IncrementalCompileOperation()
{
}


void IncrementalCompileOperation::assignContexts(Contexts& contexts)
{
    for(Contexts::iterator itr = contexts.begin();
        itr != contexts.end();
        ++itr)
    {
        osg::GraphicsContext* gc = *itr;
        addGraphicsContext(gc);
    }
}

void IncrementalCompileOperation::removeContexts(Contexts& contexts)
{
    for(Contexts::iterator itr = contexts.begin();
        itr != contexts.end();
        ++itr)
    {
        osg::GraphicsContext* gc = *itr;
        removeGraphicsContext(gc);
    }
}


void IncrementalCompileOperation::addGraphicsContext(osg::GraphicsContext* gc)
{
    if (_contexts.count(gc)==0)
    {
        gc->add(this);
        _contexts.insert(gc);
    }
}

void IncrementalCompileOperation::removeGraphicsContext(osg::GraphicsContext* gc)
{
    if (_contexts.count(gc)!=0)
    {
        gc->remove(this);
        _contexts.erase(gc);
    }
}

bool IncrementalCompileOperation::requiresCompile(StateToCompile& stateToCompile)
{
    return isActive() && !stateToCompile.empty();
}

void IncrementalCompileOperation::add(osg::Node* subgraphToCompile)
{
    add(new CompileSet(subgraphToCompile));
}

void IncrementalCompileOperation::add(osg::Group* attachmentPoint, osg::Node* subgraphToCompile)
{
    add(new CompileSet(attachmentPoint, subgraphToCompile));
}


void IncrementalCompileOperation::add(CompileSet* compileSet, bool callBuildCompileMap)
{
    if (!compileSet) return;

    if (compileSet->_subgraphToCompile.valid())
    {
        // force a compute of the bound of the subgraph to avoid the update traversal from having to do this work
        // and reducing the change of frame drop.
        compileSet->_subgraphToCompile->getBound();
    }
    
    if (callBuildCompileMap) compileSet->buildCompileMap(_contexts);

    OpenThreads::ScopedLock<OpenThreads::Mutex>  lock(_toCompileMutex);
    _toCompile.push_back(compileSet);
}

void IncrementalCompileOperation::remove(CompileSet* compileSet)
{
    if (!compileSet) return;

    // remove CompileSet from _toCompile list if it's present.
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex>  lock(_toCompileMutex);
        for(CompileSets::iterator itr = _toCompile.begin();
            itr != _toCompile.end();
            ++itr)
        {
            if (*itr == compileSet)
            {
                _toCompile.erase(itr);
                return;
            }
        }
    }
}


void IncrementalCompileOperation::operator () (osg::GraphicsContext* context)
{
    CompileInfo compileInfo(context, this);

    CompileSets toCompileCopy;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex>  toCompile_lock(_toCompileMutex);
        toCompileCopy.swap(_toCompile);
    }

    if (!toCompileCopy.empty())
    {
        compileSets(toCompileCopy, compileInfo);
    }

    const unsigned int nContextID = context->getState()->getContextID();
    osg::flushDeletedDisplayList(nContextID);
    osg::flushDeletedTexture(nContextID);
    osg::flushDeletedBufferObject(nContextID);

    // Caution: some 'flush' missed.
    // ...
    // ...
    // ...
    //double currentTime = 0.0, flushTime = 0.0;
    //osg::flushDeletedGLObjects(nContextID, currentTime, flushTime);

}


void IncrementalCompileOperation::blockOperation(bool bBlock)
{
    if(bBlock)
    {
        _blockAllowCompiling.set(false);
    }
    else
    {
        _blockAllowCompiling.release();
    }
}


void IncrementalCompileOperation::compileSets(CompileSets& toCompile, CompileInfo &compileInfo)
{
    for(CompileSets::iterator itr = toCompile.begin(); itr != toCompile.end(); )
    {
        CompileSet* cs = itr->get();
        if(!cs->_compileCompletedCallback.valid())
        {
            ++itr;
            continue;
        }

        cs->compile(compileInfo, _blockAllowCompiling);

        cs->_compileCompletedCallback->compileCompleted(cs);

        // remove entry from list.
        itr = toCompile.erase(itr);
    }

}




} // end of namespace osgUtil
