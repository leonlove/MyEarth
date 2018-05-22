// CommonOSG.cpp : 定义 DLL 应用程序的导出函数。
//

#include <osgUtil/CommonOSG.h>
#include <Common/deuMath.h>

#include <osg/CoordinateSystemNode>
#include <osg/Material>

namespace osgUtil
{



bool isMatrixPerspective(const osg::Matrixf &mtxProj)
{
    //if (_mat[0][3]!=0.0 || _mat[1][3]!=0.0 || _mat[2][3]!=-1.0 || _mat[3][3]!=0.0) return false;

    if(!cmm::math::floatEqual(mtxProj(0, 3),  0.0f))    return false;
    if(!cmm::math::floatEqual(mtxProj(1, 3),  0.0f))    return false;
    if(!cmm::math::floatEqual(mtxProj(2, 3), -1.0f))    return false;
    if(!cmm::math::floatEqual(mtxProj(3, 3),  0.0f))    return false;

    return true;
}


bool isMatrixOrtho(const osg::Matrixf &mtxProj)
{
    //if (_mat[0][3]!=0.0 || _mat[1][3]!=0.0 || _mat[2][3]!=0.0 || _mat[3][3]!=1.0) return false;

    if(!cmm::math::floatEqual(mtxProj(0, 3), 0.0f))    return false;
    if(!cmm::math::floatEqual(mtxProj(1, 3), 0.0f))    return false;
    if(!cmm::math::floatEqual(mtxProj(2, 3), 0.0f))    return false;
    if(!cmm::math::floatEqual(mtxProj(3, 3), 1.0f))    return false;

    return true;
}


bool isMatrixPerspective(const osg::Matrixd &mtxProj)
{
    //if (_mat[0][3]!=0.0 || _mat[1][3]!=0.0 || _mat[2][3]!=-1.0 || _mat[3][3]!=0.0) return false;

    if(!cmm::math::floatEqual(mtxProj(0, 3),  0.0))    return false;
    if(!cmm::math::floatEqual(mtxProj(1, 3),  0.0))    return false;
    if(!cmm::math::floatEqual(mtxProj(2, 3), -1.0))    return false;
    if(!cmm::math::floatEqual(mtxProj(3, 3),  0.0))    return false;

    return true;
}


bool isMatrixOrtho(const osg::Matrixd &mtxProj)
{
    //if (_mat[0][3]!=0.0 || _mat[1][3]!=0.0 || _mat[2][3]!=0.0 || _mat[3][3]!=1.0) return false;

    if(!cmm::math::floatEqual(mtxProj(0, 3), 0.0))    return false;
    if(!cmm::math::floatEqual(mtxProj(1, 3), 0.0))    return false;
    if(!cmm::math::floatEqual(mtxProj(2, 3), 0.0))    return false;
    if(!cmm::math::floatEqual(mtxProj(3, 3), 1.0))    return false;

    return true;
}


osg::Quat calcRotationByCoordAndAngle(const osg::Vec2d &ptCoord, double dblAzimuth, double dblPitch)
{
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    const osg::Vec3d vecPlumbLine = pEllipsoidModel->computeLocalPlumbLine(ptCoord.y(), ptCoord.x());
    const osg::Quat  qtStand(osg::Vec3d(0.0, 0.0, -1.0), vecPlumbLine);

    const osg::Vec3d vecEastern = pEllipsoidModel->computeLocalEastern(ptCoord.y(), ptCoord.x());
    const osg::Quat  qtForward(osg::Vec3d(1.0, 0.0, 0.0), vecEastern);

    const osg::Quat  qtAzimuth(dblAzimuth, -vecPlumbLine);

    const osg::Vec3d vecLocalRight = qtAzimuth * vecEastern;
    const osg::Quat  qtPitch(dblPitch, vecLocalRight);

    const osg::Quat qtRotation = qtForward * qtStand * qtAzimuth * qtPitch;
    return qtRotation;
}

osg::Vec3d convertWorld2Globe(const osg::Vec3d &point)
{
    osg::Vec3d ptCoord;
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    pEllipsoidModel->convertXYZToLatLongHeight(point.x(), point.y(), point.z(), ptCoord.y(), ptCoord.x(), ptCoord.z());
    return ptCoord;
}

osg::Vec3d convertGlobe2World(const osg::Vec3d &point)
{
    osg::Vec3d ptCoord;
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    pEllipsoidModel->convertLatLongHeightToXYZ(point.y(), point.x(), point.z(), ptCoord.x(), ptCoord.y(), ptCoord.z());
    return ptCoord;
}

}