#ifndef INTERSECT_OPERATOR_H_48D3B5A2_A01B_4875_B253_5C977B186478_INCLUDE
#define INTERSECT_OPERATOR_H_48D3B5A2_A01B_4875_B253_5C977B186478_INCLUDE

#include "SceneGraphOperationBase.h"
#include <OpenThreads/Block>
#include <osgViewer/View>

class Intersect_Operation : public SceneGraphOperationBase
{
public:
    explicit Intersect_Operation(void) {}
    virtual ~Intersect_Operation(void) {}

public:
    void    selectByScreenPoint(osgViewer::View *pView, const osg::Vec2s &point);

    struct SelectingResulet
    {
        ID id;
        osg::Vec3d point;
    };
    void    waitForFinishing(std::vector<osg::Vec3d> &vecResults);

protected:
    void    doScreenPointSelecting(osg::Node *pTargetNode);

protected:
    virtual bool doAction(SceneGraphOperator *pOperator);

protected:
    std::vector<osg::Vec2s>         m_vecScreenSelectingCoord;
    osg::ref_ptr<osgViewer::View>   m_pTargetView;

    std::vector<osg::Vec3d>   m_vecResults;
    OpenThreads::Block      m_blockFinished;
};

#endif