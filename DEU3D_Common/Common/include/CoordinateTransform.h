#ifndef COORDINATE_TRANSFORM_H_78C1A2AE_D292_45B1_A6EB_FB005EDB6252_INCLUDE
#define COORDINATE_TRANSFORM_H_78C1A2AE_D292_45B1_A6EB_FB005EDB6252_INCLUDE

#include <math.h>
#include <string>

#include "Export.h"

namespace cmm
{

typedef struct tagProjectPara
{
    double a;
    double f;
    double offset;
}PROJECTPARA;


class CM_EXPORT CoordinateTransform
{
public:
    CoordinateTransform();
    virtual ~CoordinateTransform();

    /////////////////////////////////////////////////////////////////
    //wgs
    bool BlToGauss_wgsxy(double b,double l,double *x,double *y, double dEastOffset, double dNorthOffset, double dCentreLongitude);
    bool Gauss_wgsxyToBl(double x,double y,double *b,double *l, double dEastOffset, double dNorthOffset, double dCentreLongitude);

    //UTM
    bool BlToUTM_wgsxy(double b,double l,double *x,double *y, double dEastOffset, double dNorthOffset, double dCentreLongitude);
    bool UTM_wgsxyToBl(double x,double y,double *b,double *l, double dEastOffset, double dNorthOffset, double dCentreLongitude);

    //80
    bool BlToGauss_80xy(double b,double l,double *x,double *y, double dEastOffset, double dNorthOffset, double dCentreLongitude);
    bool Gauss_80xyToBl(double x,double y,double *b,double *l, double dEastOffset, double dNorthOffset, double dCentreLongitude);
    bool BlToUTM_80xy(double b,double l,double *x,double *y, double dEastOffset, double dNorthOffset, double dCentreLongitude);
    bool UTM_80xyToBl(double x,double y,double *b,double *l, double dEastOffset, double dNorthOffset, double dCentreLongitude);

    //54
    bool BlToGauss_54xy(double b,double l,double *x,double *y, double dEastOffset, double dNorthOffset, double dCentreLongitude);
    bool Gauss_54xyToBl(double x,double y,double *b,double *l, double dEastOffset, double dNorthOffset, double dCentreLongitude);
    bool BlToUTM_54xy(double b,double l,double *x,double *y, double dEastOffset, double dNorthOffset, double dCentreLongitude);
    bool UTM_54xyToBl(double x,double y,double *b,double *l, double dEastOffset, double dNorthOffset, double dCentreLongitude);
   
    //haerbin
    bool haerbinPMto84BL(double dx, double dy,double* db,double* dl);
    bool BLtohaerbinPM  (double db, double dl,double* dx,double* dy);
    ////////////////////////////////////////////////////////////////////////
    //x 南北方向 y 东西方向
private:
    bool GaussxyToBl(double x,double y,double *b,double *l);
    bool BlToGaussxy(double b,double l,double *x,double *y);
    double * gcc(double a,double f);
    void GetBandAndCentreLongitudeByBl(double b,double l,int nGaussType,int BandNo);
    void GetBandAndCentreLongitudeByXy(double x,double y,int nGaussType);
    bool GuassXyToBl_126(double dx, double dy,double* db,double* dl);
    bool GuassBlToXy_126(double B, double L,double* dx,double* dy);
    bool D2P4(double dx0, double dy0,double* dx,double* dy);
    double invD2P4(double dx0, double dy0, bool is_26,double* dx,double* dy);
    double CalcuBf_126(double m);
    // Attributes
    // Operations
private:
    int  m_BandNo;
    double m_CentreLongitude;
    PROJECTPARA m_ProjectPara;

};

}

#endif