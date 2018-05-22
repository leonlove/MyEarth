#ifndef FRAME_IMAGE_FETCHER_H_60741E6C_E2E9_45C1_9158_5EEFA60D3C32_INCLUDE
#define FRAME_IMAGE_FETCHER_H_60741E6C_E2E9_45C1_9158_5EEFA60D3C32_INCLUDE

#include <osg/Camera>
#include <osg/Image>
#include <osgDB/WriteFile>
#include <OpenThreads/Atomic>
#include <Common/DeuImage.h>

class FrameImageFetcher : public osg::Camera::DrawCallback
{
public:
    explicit FrameImageFetcher(void);
    virtual ~FrameImageFetcher(void);

public:
    const osg::Image *getImage(void) const
    {
        return m_pFrameImage.get();
    }

    void pulseSnapshot(void);
    void waitForFrame(void);

protected:
    virtual void operator()(const osg::Camera &camera) const;

protected:
    mutable OpenThreads::Block          m_blockImageReady;
    osg::ref_ptr<osg::Image>            m_pFrameImage;
};


#endif

