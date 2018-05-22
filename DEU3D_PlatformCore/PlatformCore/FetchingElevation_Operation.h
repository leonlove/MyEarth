#ifndef FETCHING_ELEVATION_OPERATION_H_E7E260A3_BA0D_47BE_B954_A34084EF767C_INCLUDE
#define FETCHING_ELEVATION_OPERATION_H_E7E260A3_BA0D_47BE_B954_A34084EF767C_INCLUDE

#include "SceneGraphOperationBase.h"
#include <OpenThreads/Block>
#include <osgViewer/View>
#include <common/deuMath.h>
#include <osgUtil/Radial.h>

class FetchingElevation_Operation : public SceneGraphOperationBase
{
public:
    explicit FetchingElevation_Operation(void) : m_dblElevation(0.0){}
    virtual ~FetchingElevation_Operation(void) {}

public:
    void    fetchElevation(osgViewer::View *pView, const cmm::math::Point2d &position);
    double  waitForFinishing(void);

protected:
    virtual bool doAction(SceneGraphOperator *pOperator);

protected:
    bool hitScene(const osgUtil::Radial3 &ray, osg::Node *pTerrainNode, osg::Vec3d &ptHitTest) const;

protected:
    OpenThreads::Block              m_blockFinished;
    osg::ref_ptr<osgViewer::View>   m_pTargetView;

    cmm::math::Point2d              m_ptCoordinate;
    double                          m_dblElevation;
};


#endif
