#include "Source2D.h"
#include "ModelBuilder.h"

Source2D::Source2D(void)
{
}


Source2D::~Source2D(void)
{
}

void Source2D::addShape(Shape2D *s)
{
    _shapes.push_back(s);
}

bool Source2D::GenParamPoints(ParamPointList &points, unsigned short dataset_code)
{
    list<sp<Shape2D>>::iterator i = _shapes.begin();
    for (; i != _shapes.end(); ++i)
    {
        if ((*i)->getType() == PARAM_POINT_ID)
        {
            (*i)->genParamPoints(points, dataset_code);
        }
        else
        {
            string tmp = "shape: " + (*i)->getName() + " 不是点类型，被忽略";
            ModelBuilder::writeLog(tmp.c_str());
        }
    }
    return true;
}

bool Source2D::GenParamLines(const list<LineStyle>&  lod_styles, ParamLineList &lines, unsigned short dataset_code)
{
    list<sp<Shape2D>>::iterator     i = _shapes.begin();
    
    for (; i != _shapes.end(); ++i)
    {
        sp<ParamLine> line = new ParamLine(dataset_code);
        line->create((*i), lod_styles);
        lines.push_back(line);
    }
    return true;
}

bool Source2D::GenParamFaces(const list<FaceStyle> &lod_styles, ParamFaceList &faces, unsigned short dataset_code)
{
    list<sp<Shape2D>>::iterator     i = _shapes.begin();
    
    for (; i != _shapes.end(); ++i)
    {
        if ((*i)->getType() == PARAM_FACE_ID)
        {
            sp<ParamFace> face = new ParamFace(dataset_code);
            face->create((*i), lod_styles);
            faces.push_back(face);
        }
        else
        {
            string tmp = "shape: " + (*i)->getName() + " 不是面类型，被忽略";
            ModelBuilder::writeLog(tmp.c_str());
        }
    }
    return true;
}