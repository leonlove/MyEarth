#include "Shape.h"
#include "ModelBuilder.h"

Shape2D::Shape2D(void)
{
}


Shape2D::~Shape2D(void)
{
}

void Shape2D::addPoint(double x, double y)
{
    _pts.push_back(math::Point2d(x, y));
}

void Shape2D::setName(string name)
{
    _name = name;
}

string Shape2D::getName()
{
    return _name;
}

void Shape2D::genParamPoints(ParamPointList &points, unsigned short dataset_code)
{
    for (size_t i = 0; i < _pts.size(); i++)
    {
        math::Point3d pt(_pts[i].x(), _pts[i].y(), 0.0);
        points.push_back(new ParamPoint(pt, dataset_code));
    }
}

size_t Shape2D::getNumPoints()
{
    return _pts.size();
}
 
bool Shape2D::getPoint(size_t i, math::Point2d &pt)
{
    if (i >= _pts.size())
    {
        return false;
    }

    pt = _pts[i];
    return true;
}

void Shape2D::setType(DeuObjectIDType type)
{
    _type = type;
}

void Shape2D::addPart(unsigned int begin, unsigned int count)
{
    _parts.push_back(pair<unsigned int, unsigned int>(begin, count));
}

DeuObjectIDType Shape2D::getType()
{
    return _type;
}

bool Shape2D::getPart(size_t i, unsigned int &begin, unsigned int &count)
{
    if(i >= _parts.size()) return false;

    begin = _parts[i].first;
    count = _parts[i].second;

    return true;
}

size_t Shape2D::getPartCount()
{
    return _parts.size();
}