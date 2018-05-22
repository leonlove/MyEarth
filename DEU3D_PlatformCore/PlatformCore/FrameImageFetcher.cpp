#include "FrameImageFetcher.h"
#include "DEUSceneViewer.h"


FrameImageFetcher::FrameImageFetcher(void)
{
    m_pFrameImage = new osg::Image;
}


FrameImageFetcher::~FrameImageFetcher(void)
{
}


void FrameImageFetcher::operator()(const osg::Camera &camera) const
{
    const osg::Viewport *pViewport = camera.getViewport();
    m_pFrameImage->readPixels(pViewport->x(), pViewport->y(), pViewport->width(), pViewport->height(), GL_RGBA, GL_UNSIGNED_BYTE);
    m_blockImageReady.release();
}


void FrameImageFetcher::pulseSnapshot(void)
{
    m_blockImageReady.set(false);
}


void FrameImageFetcher::waitForFrame(void)
{
    m_blockImageReady.block();
}


