#ifndef _BUBBLETEXT_H_
#define _BUBBLETEXT_H_

#include <osg/Node>
#include <osg/Drawable>
#include <ParameterSys/IDetail.h>

class BubbleTextBuilder
{
public:
               BubbleTextBuilder(const param::IBubbleTextDetail *bt):m_pBubbleText(bt){}
    osg::Node* Build(const osg::Vec3& center);

protected:
    osg::Drawable* createRect(const osg::Vec3& center, int w, int h)const;

protected:
    OpenSP::sp<const param::IBubbleTextDetail> m_pBubbleText;
};

#endif