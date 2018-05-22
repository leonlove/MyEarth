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


// 函数功能：设置计算所需要的环境
// 传入参数：
//      double dblFovy：指定透视投影矩阵的张角
//      double dblWidth：指定渲染的视口宽度
//      double dblHeight：指定渲染的视口高度
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


// 函数功能：根据指定的圆盘状物体半径，和指定的视点距离，计算该圆盘在屏幕上渲染后得到光斑的像素半径
// 传入参数：
//      double dblObjectRadius：指定圆盘状物体的半径
//      double dblDistance：指定视距
// 返 回 值：返回光斑的像素半径
// 注    意：计算所需要的环境，需要事先用函数setEnviroment配置好
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


// 函数功能：已知圆盘状物体的半径，和该圆盘渲染后得到的光斑像素的半径，反推算此圆盘应该在多远的视距上渲染才能得到如此大小的光斑
// 传入参数：
//      double dblObjectRadius：指定圆盘状物体的半径
//      double dblPixelSize：指定圆盘渲染后得到的光斑像素的半径
// 返 回 值：返回圆盘被渲染的视距
// 注    意：计算所需要的环境，需要事先用函数setEnviroment配置好
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