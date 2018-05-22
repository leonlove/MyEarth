#ifndef PROJECTION_MATRIX_FIXER_H_7F44D4DE_9D8C_4106_8759_58E29ED08662_INCLUDE
#define PROJECTION_MATRIX_FIXER_H_7F44D4DE_9D8C_4106_8759_58E29ED08662_INCLUDE

#include <osg/CullSettings>
#include <osgViewer/View>
#include "NavigationParam.h"

class ProjectionMatrixFixer : public osg::CullSettings::ClampProjectionMatrixCallback
{
public:
    explicit ProjectionMatrixFixer(void);
protected:
    virtual ~ProjectionMatrixFixer(void);

public:
    void    setPlanetNode(const osg::Node *pNode);
    void    clampNearAbove(double dblNear)          {   m_dblNearAbove = dblNear;       }

protected:
    virtual bool clampProjectionMatrixImplementation(const osg::Camera *pCamera, osg::Matrixf &mtxProjection, double &dblNearZ, double &dblFarZ) const;
    virtual bool clampProjectionMatrixImplementation(const osg::Camera *pCamera, osg::Matrixd &mtxProjection, double &dblNearZ, double &dblFarZ) const;

protected:
    template<class matrix_type>
    bool _clampProjectionMatrix(const osg::Camera *pCamera, matrix_type &mtxProjection, double &dblNearZ, double &dblFarZ) const;

    template<class matrix_type>
    double computePerspectiveFovy(matrix_type &mtxProjection) const;

    double computeCurrentCameraHeight(const osg::Camera *pCamera) const;

    template<class matrix_type>
    static  bool isPerspectiveMatrix(matrix_type &mtxProjection);

protected:
    double      m_dblClampRatio;
    double      m_dblNearAbove;
    osg::ref_ptr<const osg::Node>     m_pPlanetNode;
};


#endif
