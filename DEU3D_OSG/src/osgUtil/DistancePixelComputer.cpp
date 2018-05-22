#include <osgUtil/DistancePixelComputer.h>

namespace osgUtil
{
DistancePixelComputer::DistancePixelComputer(void)
{
    setEnviroment(45.0, 1024.0, 1024.0);
}


DistancePixelComputer::~DistancePixelComputer(void)
{
}


// �������ܣ����ü�������Ҫ�Ļ���
// ���������
//      double dblFovy��ָ��͸��ͶӰ������Ž�
//      double dblWidth��ָ����Ⱦ���ӿڿ��
//      double dblHeight��ָ����Ⱦ���ӿڸ߶�
void DistancePixelComputer::setEnviroment(double dblFovy, double dblWidth, double dblHeight)
{
    const double dblRatioHV = dblWidth / dblHeight;
    m_mtxProjection.makePerspective(dblFovy, dblRatioHV, 1.0, 10000.0);

    if(!m_pViewport.valid())
    {
        m_pViewport = new osg::Viewport;
    }
    m_pViewport->setViewport(0.0, 0.0, dblWidth, dblHeight);
    m_mtxWindow = m_pViewport->computeWindowMatrix();
}


// �������ܣ�����ָ����Բ��״����뾶����ָ�����ӵ���룬�����Բ������Ļ����Ⱦ��õ���ߵ����ذ뾶
// ���������
//      double dblObjectRadius��ָ��Բ��״����İ뾶
//      double dblDistance��ָ���Ӿ�
// �� �� ֵ�����ع�ߵ����ذ뾶
// ע    �⣺��������Ҫ�Ļ�������Ҫ�����ú���setEnviroment���ú�
double DistancePixelComputer::calcPixelSizeByDistance(double dblObjectRadius, double dblDistance) const
{
    if(dblObjectRadius < FLT_EPSILON)
    {
        return 0.0;
    }
    if(dblDistance < FLT_EPSILON)
    {
        return FLT_MAX;
    }

    const osg::Vec3d ptEye(0.0, 0.0, -dblDistance);
    const osg::Vec3d ptCenter(0.0, 0.0, 0.0);
    const osg::Vec3d vecUp(0.0, 1.0, 0.0);

    osg::Matrixd mtxModelView;
    mtxModelView.makeLookAt(ptEye, ptCenter, vecUp);

    const osg::Vec3d point(dblObjectRadius * 0.5, dblObjectRadius * 0.5, 0.0);
    const osg::Vec3d ptPixel = point * mtxModelView * m_mtxProjection * m_mtxWindow;
    const osg::Vec2d ptViewCenter(m_pViewport->x() + m_pViewport->width() * 0.5, m_pViewport->y() + m_pViewport->height() * 0.5);
    const osg::Vec2d pixel(ptPixel.x() - ptViewCenter.x(), ptPixel.y() - ptViewCenter.y());

    const double dblRadiusPixel = sqrt(pixel.length2() * 0.5);
    return dblRadiusPixel;
}


// �������ܣ���֪Բ��״����İ뾶���͸�Բ����Ⱦ��õ��Ĺ�����صİ뾶���������Բ��Ӧ���ڶ�Զ���Ӿ�����Ⱦ���ܵõ���˴�С�Ĺ��
// ���������
//      double dblObjectRadius��ָ��Բ��״����İ뾶
//      double dblPixelSize��ָ��Բ����Ⱦ��õ��Ĺ�����صİ뾶
// �� �� ֵ������Բ�̱���Ⱦ���Ӿ�
// ע    �⣺��������Ҫ�Ļ�������Ҫ�����ú���setEnviroment���ú�
double DistancePixelComputer::calcDistanceByPixelSize(double dblObjectRadius, double dblPixelSize) const
{
    if(dblObjectRadius < FLT_EPSILON)
    {
        return 0.0;
    }
    if(dblPixelSize < FLT_EPSILON)
    {
        return FLT_MAX;
    }

    const double dblRefDistance  = 100.0;
    const double dblRefPixelSize = calcPixelSizeByDistance(dblObjectRadius, dblRefDistance);
    const double dblRatio        = dblRefPixelSize / dblPixelSize;
    const double dblDistance     = dblRefDistance  * dblRatio;
    return dblDistance;
}

}