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
#include <osg/GLObjects>

#include <osg/VertexProgram>
#include <osg/FragmentProgram>
#include <osg/Shader>
#include <osg/BufferObject>
#include <osg/FrameBufferObject>
#include <osg/OcclusionQueryNode>

namespace osg
{
void flushDeletedGLObjects(unsigned int contextID, double currentTime, double& availableTime)
{
#ifdef OSG_GL_DISPLAYLISTS_AVAILABLE
    //Drawable::flushDeletedDisplayLists(contextID,availableTime);
#endif

#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
    FragmentProgram::flushDeletedFragmentProgramObjects(contextID,currentTime,availableTime);
    VertexProgram::flushDeletedVertexProgramObjects(contextID,currentTime,availableTime);
#endif

    GLBufferObject::flushDeletedBufferObjects(contextID,currentTime,availableTime);
    FrameBufferObject::flushDeletedFrameBufferObjects(contextID,currentTime,availableTime);
    RenderBuffer::flushDeletedRenderBuffers(contextID,currentTime,availableTime);
    Program::flushDeletedGlPrograms(contextID,currentTime,availableTime);
    Shader::flushDeletedGlShaders(contextID,currentTime,availableTime);
    //Texture::flushDeletedTextureObjects(contextID,currentTime,availableTime);
    OcclusionQueryNode::flushDeletedQueryObjects(contextID,currentTime,availableTime);
}

void flushAllDeletedGLObjects(unsigned int contextID)
{
    double currentTime = DBL_MAX;
    double availableTime = DBL_MAX;

#ifdef OSG_GL_DISPLAYLISTS_AVAILABLE
    //Drawable::flushAllDeletedDisplayLists(contextID);
#endif

#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
    FragmentProgram::flushDeletedFragmentProgramObjects(contextID,currentTime,availableTime);
    VertexProgram::flushDeletedVertexProgramObjects(contextID,currentTime,availableTime);
#endif

    GLBufferObject::flushAllDeletedBufferObjects(contextID);
    //Texture::flushAllDeletedTextureObjects(contextID);

    FrameBufferObject::flushDeletedFrameBufferObjects(contextID,currentTime,availableTime);
    Program::flushDeletedGlPrograms(contextID,currentTime,availableTime);
    RenderBuffer::flushDeletedRenderBuffers(contextID,currentTime,availableTime);
    Shader::flushDeletedGlShaders(contextID,currentTime,availableTime);
    OcclusionQueryNode::flushDeletedQueryObjects(contextID,currentTime,availableTime);
}


void deleteAllGLObjects(unsigned int contextID)
{
    double currentTime = DBL_MAX;
    double availableTime = DBL_MAX;

#ifdef OSG_GL_DISPLAYLISTS_AVAILABLE
    //Drawable::flushAllDeletedDisplayLists(contextID);
#endif

#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
    FragmentProgram::flushDeletedFragmentProgramObjects(contextID,currentTime,availableTime);
    VertexProgram::flushDeletedVertexProgramObjects(contextID,currentTime,availableTime);
#endif

    GLBufferObject::deleteAllBufferObjects(contextID);
    //Texture::deleteAllTextureObjects(contextID);

    FrameBufferObject::flushDeletedFrameBufferObjects(contextID,currentTime,availableTime);
    Program::flushDeletedGlPrograms(contextID,currentTime,availableTime);
    RenderBuffer::flushDeletedRenderBuffers(contextID,currentTime,availableTime);
    Shader::flushDeletedGlShaders(contextID,currentTime,availableTime);
    OcclusionQueryNode::flushDeletedQueryObjects(contextID,currentTime,availableTime);
}


void discardAllGLObjects(unsigned int contextID)
{
#ifdef OSG_GL_DISPLAYLISTS_AVAILABLE
    //Drawable::discardAllDeletedDisplayLists(contextID);
#endif

#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
    FragmentProgram::discardDeletedFragmentProgramObjects(contextID);
    VertexProgram::discardDeletedVertexProgramObjects(contextID);
#endif

    GLBufferObject::discardAllBufferObjects(contextID);
    //Texture::discardAllTextureObjects(contextID);

    FrameBufferObject::discardDeletedFrameBufferObjects(contextID);
    Program::discardDeletedGlPrograms(contextID);
    RenderBuffer::discardDeletedRenderBuffers(contextID);
    Shader::discardDeletedGlShaders(contextID);
    OcclusionQueryNode::discardDeletedQueryObjects(contextID);
}


void flushDeletedDisplayList(unsigned int contextID)
{
#ifdef OSG_GL_DISPLAYLISTS_AVAILABLE
    osg::GLObjectPool::instance()->flushDeletedDisplayList(contextID);
#endif
}


void flushDeletedTexture(unsigned int contextID)
{
    osg::GLObjectPool::instance()->flushDeletedTexture(contextID);
}


void flushDeletedBufferObject(unsigned int contextID)
{
    osg::GLObjectPool::instance()->flushDeletedBufferObject(contextID);
}


class PoolImpl
{
public:
    explicit PoolImpl(void)
    {
        GLObjectPool::instance();
    }
};
static PoolImpl __impl;


GLObjectPool *GLObjectPool::instance(void)
{
    static osg::ref_ptr<GLObjectPool>  spObjectPool = new GLObjectPool;
    return spObjectPool.get();
}


GLObjectPool::GLObjectPool(void)
{
}


GLObjectPool::~GLObjectPool(void)
{
}


GLuint GLObjectPool::genDisplayList(void)
{
    const GLuint nList = glGenLists(1);

    return nList;
}


void GLObjectPool::deleteDisplayList(unsigned nContextID, GLuint nList)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxDeletedDisplayLists);
    ObjectList &listObjects = m_mapDeletedDisplayLists[nContextID];
    listObjects.push_back(nList);
}


void GLObjectPool::flushDeletedDisplayList(unsigned nContextID)
{
    ObjectList  listForDelete;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxDeletedDisplayLists);
        ObjectList &listObjects = m_mapDeletedDisplayLists[nContextID];
        if(listObjects.empty()) return;
        listForDelete.swap(listObjects);
    }

    for(ObjectList::iterator itor = listForDelete.begin(); itor != listForDelete.end(); ++itor)
    {
        const GLuint nList = *itor;
        glDeleteLists(nList, 1u);
    }
}


GLuint GLObjectPool::genTexture(void)
{
    GLuint nTexture = 0u;
    glGenTextures(1, &nTexture);
    return nTexture;
}


void GLObjectPool::deleteTexture(unsigned nContextID, GLuint nTexture)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxDeletedTextures);

    ObjectList &listObjects = m_mapDeletedTextures[nContextID];
    listObjects.push_back(nTexture);
}


void GLObjectPool::flushDeletedTexture(unsigned nContextID)
{
    ObjectList listForDeleted;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxDeletedTextures);
        ObjectList &listObjects = m_mapDeletedTextures[nContextID];
        listForDeleted.swap(listObjects);
    }
    for(ObjectList::iterator itor = listForDeleted.begin(); itor != listForDeleted.end(); ++itor)
    {
        const GLuint nTexture = *itor;
        glDeleteTextures(1u, &nTexture);
    }
}


void GLObjectPool::deleteBufferObject(unsigned nContextID, GLuint nBufObj)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxDeletedBufferObjects);

    ObjectList &listObjects = m_mapDeletedBufferObjects[nContextID];
    listObjects.push_back(nBufObj);
}


void GLObjectPool::flushDeletedBufferObject(unsigned nContextID)
{
    ObjectList listForDeleted;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxDeletedBufferObjects);
        ObjectList &listObjects = m_mapDeletedBufferObjects[nContextID];
        listForDeleted.swap(listObjects);
    }

    GLBufferObject::Extensions *pExtensions = GLBufferObject::getExtensions(nContextID, true);
    if(!pExtensions)    return;
    for(ObjectList::iterator itor = listForDeleted.begin(); itor != listForDeleted.end(); ++itor)
    {
        const GLuint nBufObj = *itor;
        pExtensions->glDeleteBuffers(1, &nBufObj);
    }
}


}
