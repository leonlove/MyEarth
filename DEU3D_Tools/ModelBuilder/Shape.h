#pragma once
#include <vector>
#include <Common\deuMath.h>
#include <OpenSP\ref.h>
#include "ParamPoint.h"

using namespace std;
using namespace cmm;
using namespace OpenSP;

class Shape2D:public Ref
{
public:
    Shape2D(void);
    ~Shape2D(void);

    void            addPoint(double x, double y);
    size_t          getNumPoints();
    bool            getPoint(size_t i, math::Point2d &pt);

    void            genParamPoints(ParamPointList &points, unsigned short dataset_code);

    void            setName(string name);
    string          getName();

    void            addPart(unsigned int begin, unsigned int count);
    bool            getPart(size_t i, unsigned int &begin, unsigned int &count);
    size_t          getPartCount();

    void            setType(DeuObjectIDType type);
    DeuObjectIDType getType();
    
protected:
    vector<math::Point2d> _pts;
    vector<pair<unsigned int, unsigned int>>  _parts;

    string                _name;
    DeuObjectIDType       _type;
};

