#pragma once
#include "ParamPoint.h"
#include "Line.h"
#include "Face.h"
#include "Shape.h"

class Source2D
{
public:
    Source2D(void);
    ~Source2D(void);

    void addShape(Shape2D *s);
    bool GenParamPoints(ParamPointList &points, unsigned short dataset_code);
    bool GenParamLines(const list<LineStyle>& lod_styles, ParamLineList &lines, unsigned short dataset_code);
    bool GenParamFaces(const list<FaceStyle> &lod_styles, ParamFaceList &faces, unsigned short dataset_code);

protected:
    list<sp<Shape2D>> _shapes;
    void*    _shpHandle;
	void*    _dbfHandle;
    int      _shpCount;
	int      _fiedCount;
	int      _shpType;
};

