#include <osgUtil/Radial.h>
#include <assert.h>
#include <osgUtil/CommonOSG.h>
#include <Common/deuMath.h>

namespace osgUtil
{

///////////////////////////////////////////////////////////////////////////////
Radial3::Radial3(void)
        : m_ptOrigin(0.0f, 0.0f, 0.0f),
          m_vecDirection(0.0f, 0.0f, 1.0f)
{
}


Radial3::Radial3(const osg::Vec3d &ptOrigin, const osg::Vec3d &vecDirection)
{
    m_ptOrigin = ptOrigin;
    m_vecDirection = vecDirection;
    const osg::Vec3d::value_type fltLen = m_vecDirection.normalize();
    assert(fltLen > FLT_EPSILON);
    fltLen;
}

Radial3::Radial3(const osg::Camera *pCamera, const osg::Vec2d &ptPosNormalize)
{
    const osg::Matrixd &mtxVirtualView = pCamera->getViewMatrix();
    const osg::Matrixd &mtxProj = pCamera->getProjectionMatrix();
    const osg::Matrixd  mtxInverseVP = osg::Matrix::inverse(mtxVirtualView * mtxProj);
    const osg::Vec3 ptNear(ptPosNormalize.x(), ptPosNormalize.y(), -1.0f);
    const osg::Vec3 ptMid(ptPosNormalize.x(), ptPosNormalize.y(), 0.0f);

    const osg::Vec3 ptRayBegin = ptNear * mtxInverseVP;
    const osg::Vec3 ptRayEnd = ptMid * mtxInverseVP;

    m_ptOrigin = ptRayBegin;
    m_vecDirection = ptRayEnd - ptRayBegin;
    const osg::Vec3d::value_type fltLen = m_vecDirection.normalize();
    assert(fltLen > FLT_EPSILON);
    fltLen;
}


Radial3::~Radial3(void)
{
}


void Radial3::setOrigin(const osg::Vec3d &ptOrigin)
{
    m_ptOrigin = ptOrigin;
}


const osg::Vec3d &Radial3::getOrigin(void) const
{
    return m_ptOrigin;
}


void Radial3::setDirection(const osg::Vec3d &vecDir)
{
    m_vecDirection = vecDir;
    const osg::Vec3d::value_type fltLen = m_vecDirection.normalize();
    assert(fltLen > FLT_EPSILON);
    fltLen;
}


const osg::Vec3d &Radial3::getDirection(void) const
{
    return m_vecDirection;
}


osg::Vec3d Radial3::getPoint(osg::Vec3d::value_type t) const 
{
    return osg::Vec3d(m_ptOrigin + (m_vecDirection * t));
}


osg::Vec3d Radial3::operator*(osg::Vec3d::value_type t) const
{ 
    return getPoint(t);
}


osg::Vec3d::value_type Radial3::intersectPlane(const osg::Plane &plane) const
{
    const osg::Vec3d vecPlaneNormal = plane.getNormal();
    const osg::Vec3d::value_type fltCosTheta = vecPlaneNormal * m_vecDirection;
    if(cmm::math::floatEqual(fltCosTheta, 0.0))
    {
        return FLT_MAX;
    }

    const osg::Vec3d::value_type fltDistance = plane.distance(m_ptOrigin);
    const osg::Vec3d::value_type fltRatio = -(fltDistance / fltCosTheta);
    return fltRatio;
}


bool Radial3::hitPlane(const osg::Plane &plane, osg::Vec3d &point) const
{
    const osg::Vec3d::value_type fltCosTheta = plane.getNormal() * getDirection();
    if(fabs(fltCosTheta) <= FLT_EPSILON)
    {
        // ���ߺ�ƽ�����ƽ�У�������
        return false;
    }

    const osg::Vec3d::value_type fltRatio = intersectPlane(plane);
    //osg::notify(osg::NOTICE) << "�������:" << fltRatio << std::endl;
    if(fltRatio > FLT_MAX * 0.5f)
    {
        // �ཻ��ǳ�Զ��Զ����������󣩣��������ߺ�ƽ�������ƽ��
        return false;
    }

    if(fltRatio < FLT_EPSILON)
    {
        // �ཻ�㴦�����ߵķ���������޷��ཻ���޷�ʵ����ק
        return false;
    }

    point = getPoint(fabs(fltRatio));
    return true;
}


unsigned Radial3::hitSphere(const osg::BoundingSphere &sphere, osg::Vec3d &point1, osg::Vec3d &point2) const
{
    const static unsigned nSize = sizeof(osg::Vec3d::value_type) * 3u;
    const static std::vector<unsigned char> vecNaN(0xFF, nSize);
    memcpy(point1.ptr(), vecNaN.data(), nSize);
    memcpy(point2.ptr(), vecNaN.data(), nSize);
    const osg::Vec3d::value_type dblDis2Center = (m_ptOrigin - sphere.center()).length();

    osg::Vec3d vecOrg2Center = sphere.center() - m_ptOrigin;
    vecOrg2Center.normalize();

    const osg::Vec3d::value_type dblCosTheta      = vecOrg2Center * m_vecDirection;
    const osg::Vec3d::value_type dblDis2Foot      = dblDis2Center * dblCosTheta;
    const osg::Vec3d::value_type dblFoot2Center_2 = (1.0 - dblCosTheta * dblCosTheta) * dblDis2Center * dblDis2Center;
    const osg::Vec3d::value_type dblFoot2Center   = sqrt(dblFoot2Center_2);
    if(dblFoot2Center > sphere.radius())
    {
        // �������ڵ�ֱ�ߣ�������Ĺ�ϵ�����룬��˾��޿��ܴ��ڽ���
        return 0u;
    }


    const osg::Vec3d::value_type dblBias = sqrt(sphere.radius2() - dblFoot2Center_2);
    if(dblDis2Center < sphere.radius())
    {
        // ���ߵ����������֮�ڣ��������ֻ�����ߵ���������ڽ��㣬�����򽻵�Ӧ�ú���
        point2 = getPoint(dblDis2Foot + dblBias);
        return 1u;
    }

    if(dblDis2Foot < 0.0)
    {
        // ���ߵ����������֮�⣬�������ߵķ���Զ�����壬��ʱû�н���
        return 0u;
    }

    if(cmm::math::floatEqual(dblBias, 0.0))
    {
        // ���ߺ��������У���ʱֻ��һ������
        point1 = getPoint(dblDis2Foot);
        return 1;
    }

    point1 = getPoint(dblDis2Foot - dblBias);
    point2 = getPoint(dblDis2Foot + dblBias);
    return 2u;
}


unsigned Radial3::hitEllipsoid(const osg::EllipsoidModel &ellipsoid, osg::Vec3d &point1, osg::Vec3d &point2) const
{
    return 0u;
}


}