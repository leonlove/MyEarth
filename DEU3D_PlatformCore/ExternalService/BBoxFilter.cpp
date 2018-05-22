#include "BBoxFilter.h"


BBoxFilter::BBoxFilter(void)
{
    m_dMinX = m_dMinY = m_dMaxX = m_dMaxY = 0.0;
}


BBoxFilter::~BBoxFilter(void)
{
}

void BBoxFilter::setBBox(double dMinX,double dMinY,double dMaxX,double dMaxY)
{
    m_strPropertyName = "";
    m_dMinX = dMinX;
    m_dMinY = dMinY;
    m_dMaxX = dMaxX;
    m_dMaxY = dMaxY;
}

void BBoxFilter::getBBox(double& dMinX,double& dMinY,double& dMaxX,double& dMaxY)
{
    dMinX = m_dMinX;
    dMinY = m_dMinY;
    dMaxX = m_dMaxX;
    dMaxY = m_dMaxY;
}