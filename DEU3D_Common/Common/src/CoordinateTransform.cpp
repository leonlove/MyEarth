#include <CoordinateTransform.h>
#include <float.h>

const double DEGTORAD = 0.017453292519722;
const double PI       = 3.1415926535897932;
const int    A        = 6378137; 
const double e1_2     = 0.0066943799013;
const double e2_2     = 0.00673949674227;


namespace cmm
{
    CoordinateTransform::CoordinateTransform()
    {
    }

    CoordinateTransform::~CoordinateTransform()
    {

    }

    bool CoordinateTransform::haerbinPMto84BL(double dx, double dy,double* db,double* dl)
    {
        double pm2000dx = 0.0;
        double pm2000dy = 0.0;

        D2P4(dx, dy,&pm2000dx,&pm2000dy);

        return GuassXyToBl_126(pm2000dx,pm2000dy,db,dl);
    }
    bool CoordinateTransform::BLtohaerbinPM(double db, double dl,double* dx,double* dy)
    {
        double pm2000dx = 0;
        double pm2000dy = 0;
        bool is26 = false;
        if (db > 45.440319 && db < 45.44274034 && dl > 126.40492221 && dl < 126.378013534)
        {
            //26区范围
            is26 = true;
        }
        else
        {
            is26 = false;
        }

        GuassBlToXy_126(db, dl,&pm2000dx,&pm2000dy);
        invD2P4(pm2000dx, pm2000dy, is26,dx,dy);

        return true;
    }

    double CoordinateTransform::invD2P4(double dx0, double dy0, bool is_26,double* dx,double* dy)
    {
        double dx1 = -4869643.7344038114;
        double dy1 = -438433.56453097053;
        double dC = 1.0014090893408589;
        double dD = 0.036760240493361351;
        dy0 = dy0 - 500000;


        *dx = dx1 + dC * dx0 - dD * dy0;
        *dy = dy1 + dD * dx0 + dC * dy0;

        *dy += 500000;
        
        return true;
    }


    bool CoordinateTransform::GuassBlToXy_126(double B, double L,double* dx,double* dy)
    {
        double x, y;
        double X;
        int L0;
        double dB, dL;

        L0 = 126;

        double l = L - L0;				//求经差
        l = l / 180 * PI;			//经差用弧度表示

        dB = B / 180 * PI;				//弧度表示
        dL = L / 180 * PI;				//弧度表示

        double m0, m2, m4, m6, m8, a0, a2, a4, a6, a8;

        m0 = A * (1 - e1_2);
        m2 = 3 * m0 * e1_2 / 2;
        m4 = 5 * m2 * e1_2 / 4;
        m6 = 7 * m4 * e1_2 / 6;
        m8 = 9 * m6 * e1_2 / 8;

        a0 = m0 + m2 / 2 + 3 * m4 / 8 + 5 * m6 / 16 + 35 * m8 / 128;
        a2 = m2 / 2 + m4 / 2 + 15 * m6 / 32 + 7 * m8 / 16;
        a4 = m4 / 8 + 3 * m6 / 16 + 7 * m8 / 32;
        a6 = m6 / 32 + m8 / 16;
        a8 = m8 / 128;

        X = a0 * dB - a2 * sin(2 * dB) / 2 + a4 * sin(4 * dB) / 4 - a6 * sin(6 * dB) / 6 + a8 * sin(8 * dB) / 8;

        double t, eta_2, N;
        t = tan(dB);
        eta_2 = e2_2 * cos(dB) * cos(dB);
        N = A / sqrt(1 - e1_2 * sin(dB) * sin(dB));		//计算卯酉圈半径

        double cosbl_2, cosbl_3, cosbl_4, cosbl_5, cosbl_6;
        cosbl_2 = cos(dB) * cos(dB) * l * l;
        cosbl_3 = cos(dB) * cos(dB) * cos(dB) * l * l * l;
        cosbl_4 = cos(dB) * cos(dB) * cos(dB) * cos(dB) * l * l * l * l;
        cosbl_5 = cos(dB) * cos(dB) * cos(dB) * cos(dB) * cos(dB) * l * l * l * l * l;
        cosbl_6 = cos(dB) * cos(dB) * cos(dB) * cos(dB) * cos(dB) * cos(dB) * l * l * l * l * l * l;
        x = X + N * t * cosbl_2 / 2 + N * t * (5 - t * t + 9 * eta_2 + 4 * eta_2 * eta_2) * cosbl_4 / 24 + N * t * (61 - 58 * t * t + t * t * t * t) * cosbl_6 / 720;
        y = N * l * cos(dB) + N * (1 - t * t + eta_2) * cosbl_3 / 6 + N * (5 - 18 * t * t + t * t * t * t + 14 * eta_2 - 58 * t * t * eta_2) * cosbl_5 / 120;

        *dx = x;
        *dy = y + 500000;
        return true;
    }

    bool CoordinateTransform::GuassXyToBl_126(double dx, double dy,double* db,double* dl)
    {
        double x = dx;
        double y = dy;
        double L0 = 126;              //中央子午线经度 固定在126
        y = y - 500000;               //y的真实坐标
        double Bf = CalcuBf_126(x);   //求底点纬度
        double Mf = (A * (1 - e1_2)) / sqrt((1 - e1_2 * sin(Bf) * sin(Bf)) * (1 - e1_2 * sin(Bf) * sin(Bf)) * (1 - e1_2 * sin(Bf) * sin(Bf)));
        double Nf = A / sqrt(1 - e1_2 * sin(Bf) * sin(Bf));
        double tf = tan(Bf);
        double etaf_2 = e2_2 * cos(Bf) * cos(Bf);

        double tf_2 = tf * tf;
        double tf_4 = tf_2 * tf_2;
        double y_2 = y * y;
        double y_3 = y_2 * y;
        double y_4 = y_3 * y;
        double y_5 = y_4 * y;
        double y_6 = y_5 * y;

        double Nf_3 = pow(Nf, 3);
        double Nf_5 = pow(Nf, 5);

        double B = Bf - tf * y_2 / (2 * Mf * Nf) + tf * (5 + 3 * tf_2 + etaf_2 - 9 * etaf_2 * tf_2) * y_4 / (24 * Mf * Nf_3) - tf * (61 + 90 * tf_2 + 45 * tf_4) * y_6 / (720 * Mf * Nf_5);
        double L = y / (Nf * cos(Bf)) - y_3 * (1 + 2 * tf_2 + etaf_2) / (6 * Nf_3 * cos(Bf)) + (5 + 28 * tf_2 + 24 * tf_4 + 6 * etaf_2 + 8 * etaf_2 * tf_2) * y_5 / (120 * Nf_5 * cos(Bf));

        B = B / PI * 180;		//转换为角度小数
        L = L / PI * 180;		//转换为角度小数
        L += L0;

        *db = B;
        *dl = L;

        return true;
    }

    double CoordinateTransform::CalcuBf_126(double m)
    {
        double b0, c0, e1_4, e1_6, e1_8, e1_10, e1_12, e1_14, e1_16;

        e1_4 = pow(e1_2, 2);
        e1_6 = pow(e1_2, 3);
        e1_8 = pow(e1_2, 4);
        e1_10 = pow(e1_2, 5);
        e1_12 = pow(e1_2, 6);
        e1_14 = pow(e1_2, 7);
        e1_16 = pow(e1_2, 8);
        c0 = 1 + e1_2 / 4 + 7 * e1_4 / 64 + 15 * e1_6 / 256 + 579 * e1_8 / 16384 + 1515 * e1_10 / 65536 + 16837 * e1_12 / 1048576 + 48997 * e1_14 / 4194304 + 9467419 * e1_16 / 1073741824;
        c0 = A / c0;

        b0 = m / c0;
        double d1, d2, d3, d4, d5, d6, d7;
        d1 = 3 * e1_2 / 8 + 45 * e1_4 / 128 + 175 * e1_6 / 512 + 11025 * e1_8 / 32768 + 43659 * e1_10 / 131072 + 693693 * e1_12 / 2097152 + 10863435 * e1_14 / 33554432;
        d2 = -21 * e1_4 / 64 - 277 * e1_6 / 384 - 19413 * e1_8 / 16384 - 56331 * e1_10 / 32768 - 2436477 * e1_12 / 1048576 - 196473 * e1_14 / 65536;
        d3 = 151 * e1_6 / 384 + 5707 * e1_8 / 4096 + 53189 * e1_10 / 163840 + 4599609 * e1_12 / 655360 + 15842375 * e1_14 / 1048576;
        d4 = -1097 * e1_8 / 2048 - 1687 * e1_10 / 640 - 3650333 * e1_12 / 327680 - 114459079 * e1_14 / 27525120;
        d5 = 8011 * e1_10 / 1024 + 874457 * e1_12 / 98304 + 216344925 * e1_14 / 3670016;
        d6 = -682193 * e1_12 / 245760 - 46492223 * e1_14 / 1146880;
        d7 = 36941521 * e1_14 / 3440640;

        double bf;
        bf = b0 + sin(2 * b0) * (d1 + sin(b0) * sin(b0) * (d2 + sin(b0) * sin(b0) * (d3 + sin(b0) * sin(b0) * (d4 + sin(b0) * sin(b0) * (d5 + sin(b0) * sin(b0) * (d6 + d7 * sin(b0) * sin(b0)))))));
        return bf;
    }

    bool CoordinateTransform::D2P4(double dx0, double dy0,double* dx,double* dy)
    {
        double dx1 = 4866331.887181;
        double dy1 = 254533.399546;
        double dC  = 0.999689;
        double dD  = -0.008263;
        dy0 = dy0 - 500000;

        *dx = dx1 + dC * dx0 - dD * dy0;
        *dy = dy1 + dD * dx0 + dC * dy0;

        *dy += 500000;
        return true;
    }

    bool CoordinateTransform::Gauss_54xyToBl(double x,double y,double *b,double *l, double dEastOffset, double dNorthOffset, double dCentreLongitude)
    {
        if (_isnan(x) || _isnan(y)) 
        {
            *b = 0.0;
            *l = 0.0;
            return false;
        }

        m_ProjectPara.a = 6378245.000;
        m_ProjectPara.f = 298.300;
        m_ProjectPara.offset = dEastOffset/*500000.000*/;

        //m_BandNo = nBandNo;
        //GetBandAndCentreLongitudeByXy(x,y,nGaussType);
        m_CentreLongitude = dCentreLongitude;

        //x -= m_BandNo * 1.0E6;
        x -= m_ProjectPara.offset;

        GaussxyToBl(y,x,b,l);

        *l += m_CentreLongitude;
        if (*l > 180.) *l -= 360.;
        return true;
    }

    bool CoordinateTransform::UTM_54xyToBl(double x,double y,double *b,double *l, double dEastOffset, double dNorthOffset, double dCentreLongitude)
    {
        if (_isnan(x) || _isnan(y)) 
        {
            *b = 0.0;
            *l = 0.0;
            return false;
        }

        m_ProjectPara.a = 6378245.000*0.9996;
        m_ProjectPara.f = 298.300;
        m_ProjectPara.offset = dEastOffset/*500000.000*/;

        //m_BandNo = nBandNo;
        //GetBandAndCentreLongitudeByXy(x,y,nGaussType);
        m_CentreLongitude = dCentreLongitude;

        //x -= m_BandNo * 1.0E6;
        x -= m_ProjectPara.offset;

        GaussxyToBl(y,x,b,l);

        *l += m_CentreLongitude;
        if (*l > 180.) *l -= 360.;
        return true;
    }

    bool CoordinateTransform::Gauss_80xyToBl(double x,double y,double *b,double *l, double dEastOffset, double dNorthOffset, double dCentreLongitude)
    {
        if (_isnan(x) || _isnan(y)) 
        {
            *b = 0.0;
            *l = 0.0;
            return false;
        }

        m_ProjectPara.a = 6378140.000;
        m_ProjectPara.f = 298.257;
        m_ProjectPara.offset = dEastOffset/*500000.000*/;

       //m_BandNo = nBandNo;
       //GetBandAndCentreLongitudeByXy(x,y,nGaussType);
        m_CentreLongitude = dCentreLongitude;

        //x -= m_BandNo * 1.0E6;
        x -= m_ProjectPara.offset;

        GaussxyToBl(y,x,b,l);

        *l += m_CentreLongitude;
        if (*l > 180.) *l -= 360.;
        return true;
    }

    bool CoordinateTransform::UTM_80xyToBl(double x,double y,double *b,double *l, double dEastOffset, double dNorthOffset, double dCentreLongitude)
    {
        if (_isnan(x) || _isnan(y)) 
        {
            *b = 0.0;
            *l = 0.0;
            return false;
        }

        m_ProjectPara.a = 6378140.000*0.9996;
        m_ProjectPara.f = 298.257;
        m_ProjectPara.offset = dEastOffset/*500000.000*/;

        //m_BandNo = nBandNo;
        //GetBandAndCentreLongitudeByXy(x,y,nGaussType);
        m_CentreLongitude = dCentreLongitude;

        //x -= m_BandNo * 1.0E6;
        x -= m_ProjectPara.offset;

        GaussxyToBl(y,x,b,l);

        *l += m_CentreLongitude;
        if (*l > 180.) *l -= 360.;
        return true;
    }

    bool CoordinateTransform::Gauss_wgsxyToBl(double x,double y,double *b,double *l, double dEastOffset, double dNorthOffset, double dCentreLongitude)
    {
        if (_isnan(x) || _isnan(y)) 
        {
            *b = 0.0;
            *l = 0.0;
            return false;
        }

        m_ProjectPara.a = 6378137.000;
        m_ProjectPara.f = 298.7580625430662;
        //m_ProjectPara.f = 298.257223563;
        m_ProjectPara.offset = dEastOffset/*500000.000*/;

        //m_BandNo = nBandNo;
        //GetBandAndCentreLongitudeByXy(x,y,nGaussType);
        m_CentreLongitude = dCentreLongitude;

        //x -= m_BandNo * 1.0E6;
        x -= m_ProjectPara.offset;

        GaussxyToBl(y,x,b,l);

        *l += m_CentreLongitude;
        if (*l > 180.) *l -= 360.;
        return true;
    }

    bool CoordinateTransform::UTM_wgsxyToBl(double x,double y,double *b,double *l, double dEastOffset, double dNorthOffset, double dCentreLongitude)
    {
        if (_isnan(x) || _isnan(y)) 
        {
            *b = 0.0;
            *l = 0.0;
            return false;
        }

        m_ProjectPara.a = 6378137.000*0.9996;
        m_ProjectPara.f = 298.7580625430662;
        //m_ProjectPara.f = 298.257223563;
        m_ProjectPara.offset = dEastOffset/*500000.000*/;

        //m_BandNo = nBandNo;
        //GetBandAndCentreLongitudeByXy(x,y,nGaussType);
        m_CentreLongitude = dCentreLongitude;

        //x -= m_BandNo * 1.0E6;
        x -= m_ProjectPara.offset;

        GaussxyToBl(y,x,b,l);

        *l += m_CentreLongitude;
        if (*l > 180.) *l -= 360.;
        return true;
    }

    double * CoordinateTransform::gcc(double a,double f)
    {
        static double ab[6];
        double a7,e2;
        int i;
        if(f<=0.5) return NULL;
        e2=2/f-1/f/f;
        ab[0]=pow(e2,5)*43659.0/65536.0+pow(e2,4)*11025.0/16384.0+1.0;
        ab[0]+=e2*e2*45.0/64.0+pow(e2,3)*175.0/256.0+e2*0.75;
        ab[1]=e2*0.75+e2*e2*15.0/16.0+pow(e2,3)*525.0/512.0;
        ab[1]+=pow(e2,4)*2205.0/2048.0+pow(e2,5)*72765.0/65536.0 ;
        ab[2]=e2*e2*15.0/64.0+pow(e2,3)*105.0/256.0;
        ab[2]+=pow(e2,4)*2205.0/4096.0+pow(e2,5)*10395.0/16384.0;
        ab[3]=pow(e2,3)*35.0/512.0+pow(e2,4)*315.0/2048.0;
        ab[3]+=pow(e2,5)*31185.0/131072.0;
        ab[4]=pow(e2,4)*315.0/16384.0+pow(e2,5)*3465.0/65536.0;
        ab[5]=pow(e2,5)*693.0/131072.0;
        a7=a*(-e2+1.0);
        ab[0]*=a7;
        for (i=1;i<6;i++)
        {
            double dT = -1;
            ab[i] *= pow(dT, i) * a7 / (2.0 * i);
        }
        return ab;
    }
    bool CoordinateTransform::BlToGauss_54xy(double b,double l,double *x,double *y, double dEastOffset, double dNorthOffset, double dCentreLongitude)
    {
        m_ProjectPara.a = 6378245.000;
        m_ProjectPara.f = 298.300;
        m_ProjectPara.offset = dEastOffset/*500000.00*/;
        //GetBandAndCentreLongitudeByBl(b,l,nGaussType,nBandNo);

        m_CentreLongitude = dCentreLongitude;

        l = l - m_CentreLongitude;
        BlToGaussxy(b,l,y,x);
        *x += m_ProjectPara.offset;
        //*x += m_BandNo * 1.0E6;
        return true;
    }
    bool CoordinateTransform::BlToUTM_54xy(double b,double l,double *x,double *y, double dEastOffset, double dNorthOffset, double dCentreLongitude)
    {
        m_ProjectPara.a = 6378245.000*0.9996;
        m_ProjectPara.f = 298.300;
        m_ProjectPara.offset = dEastOffset/*500000.00*/;
        //GetBandAndCentreLongitudeByBl(b,l,nGaussType,nBandNo);

        m_CentreLongitude = dCentreLongitude;

        l = l - m_CentreLongitude;
        BlToGaussxy(b,l,y,x);
        *x += m_ProjectPara.offset;
        //*x += m_BandNo * 1.0E6;
        return true;
    }
    bool CoordinateTransform::BlToGauss_80xy(double b,double l,double *x,double *y, double dEastOffset, double dNorthOffset, double dCentreLongitude)
    {
        m_ProjectPara.a = 6378140.000;
        m_ProjectPara.f = 298.257;
        m_ProjectPara.offset = dEastOffset/*500000.00*/;
        //GetBandAndCentreLongitudeByBl(b,l,nGaussType,nBandNo);

        m_CentreLongitude = dCentreLongitude;

        l = l -    m_CentreLongitude;
        BlToGaussxy(b,l,y,x);
        *x += m_ProjectPara.offset;
        //*x += m_BandNo * 1.0E6;
        return true;
    }
    bool CoordinateTransform::BlToUTM_80xy(double b,double l,double *x,double *y, double dEastOffset, double dNorthOffset, double dCentreLongitude)
    {
        m_ProjectPara.a = 6378140.000*0.9996;
        m_ProjectPara.f = 298.257;
        m_ProjectPara.offset = dEastOffset/*500000.00*/;
        //GetBandAndCentreLongitudeByBl(b,l,nGaussType,nBandNo);

        m_CentreLongitude = dCentreLongitude;

        l = l -    m_CentreLongitude;
        BlToGaussxy(b,l,y,x);
        *x += m_ProjectPara.offset;
        //*x += m_BandNo * 1.0E6;
        return true;
    }
    bool CoordinateTransform::BlToGauss_wgsxy(double b,double l,double *x,double *y, double dEastOffset, double dNorthOffset, double dCentreLongitude)
    {
        m_ProjectPara.a = 6378137.000;
        m_ProjectPara.f = 298.257223563;
        m_ProjectPara.offset = dEastOffset/*500000.00*/;
        //GetBandAndCentreLongitudeByBl(b,l,nGaussType,nBandNo);

        m_CentreLongitude = dCentreLongitude;

        l = l -    m_CentreLongitude;
        BlToGaussxy(b,l,y,x);
        *x += m_ProjectPara.offset;
        //*x += m_BandNo * 1.0E6;
        return true;
    }

    bool CoordinateTransform::BlToUTM_wgsxy(double b,double l,double *x,double *y, double dEastOffset, double dNorthOffset, double dCentreLongitude)
    {
        m_ProjectPara.a = 6378137.000*0.9996;
        m_ProjectPara.f = 298.257223563;
        m_ProjectPara.offset = dEastOffset/*500000.00*/;
        //GetBandAndCentreLongitudeByBl(b,l,nGaussType,nBandNo);

        m_CentreLongitude = dCentreLongitude;

        l = l -    m_CentreLongitude;
        BlToGaussxy(b,l,y,x);
        *x += m_ProjectPara.offset;
        //*x += m_BandNo * 1.0E6;
        return true;
    }

    bool CoordinateTransform::GaussxyToBl(double x,double y,double *b,double *l)
    {
        double a,f,e2,b0,bf0,t,n0,g,m,n,*p1;
        a = m_ProjectPara.a;
        f = m_ProjectPara.f;
        p1=gcc(a,f);
        if(p1==NULL) return false;
        bf0=x/(*p1);
        e2=2/f-1/f/f;
        for(;;)
        {
            b0=-*(p1+1)*sin(2.0*bf0)-*(p1+2)*sin(4.0*bf0)-*(p1+3)*sin(6.0*bf0);
            b0-=*(p1+4)*sin(8.0*bf0)-*(p1+5)*sin(10.0*bf0);
            b0=(x+b0)/(*p1);
            if(fabs(bf0-b0)<1.0e-15) break;
            bf0=b0;
        }
        t=tan(b0);
        g=e2/(1.0-e2)*cos(b0)*cos(b0);
        n=a/sqrt(1.0-e2*sin(b0)*sin(b0));
        m=n*(1.0-e2)/(1.0-e2*sin(b0)*sin(b0));
        n0=y/n;
        *b=-t*y*n0/m/2.0;
        *b+=(5.0+3.0*t*t+g-9.0*g*t*t)*t*y*pow(n0,3)/m/24.0;
        *b-=(61.0+90.0*t*t+45.0*t*t*t*t)*t*y*pow(n0,5)/720.0/m;
        *b+=b0;
        *l=n0/cos(b0);
        *l-=(1.0+2.0*t*t+g)*pow(n0,3)/cos(b0)/6.0;
        *l+=(5.0+28.0*t*t+24.0*t*t*t*t+6.0*g+8.0*g*t*t)*pow(n0,5)/cos(b0)/120.0;
        *b /= DEGTORAD;
        *l /= DEGTORAD;
        return true;
    }

    bool CoordinateTransform::BlToGaussxy(double b,double l,double *x,double *y)
    {
        double a,f,e2,e1,t,n,g,m0,x0,*p1;
        a = m_ProjectPara.a;
        f = m_ProjectPara.f;
        l *= DEGTORAD;
        b *= DEGTORAD;
        e2=2./f-1/f/f;
        if(f==0.)  return false;  //  /0
        e1=e2/(-e2+1.0);
        p1=gcc(a,f);
        x0=*p1*b+*(p1+1)*sin(2.0*b)+*(p1+2)*sin(4.0*b);
        x0+=*(p1+3)*sin(6.0*b)+*(p1+4)*sin(8.0*b)+*(p1+5)*sin(10.0*b);
        t=tan(b);
        n=a/sqrt(1.0-e2*sin(b)*sin(b));
        g=e1*cos(b)*cos(b);
        m0=l*cos(b);
        *x=x0+n*t*m0*m0/2.0;
        *x+=(5.0-t*t+9.0*g+4.0*g*g)*n*t*pow(m0,4)/24.0;
        *x+=n*pow(m0,6)*t*(61.0-58.0*t*t+t*t*t*t)/720.0;
        *y=n*m0+n*pow(m0,3)*(1.0-t*t+g)/6.0;
        *y+=(5.0-18.0*t*t+t*t*t*t+14.0*g-58.0*g*t*t)*n*pow(m0,5)/120.0;
        return true;
    }

    void CoordinateTransform::GetBandAndCentreLongitudeByBl(double b,double l,int nGaussType,int BandNo)
    {
        switch(nGaussType)
        {
        case 3:
            if(BandNo != -1)
                m_BandNo = BandNo ;
            else
                m_BandNo = (int)((l - 1.50)/3.0) + 1;
            m_CentreLongitude = m_BandNo * 3.0;
            break;
        case 6:
            if(BandNo != -1)
                m_BandNo = BandNo ;
            else
                m_BandNo = (int)(l/6.0 + 1.0);
            m_CentreLongitude = m_BandNo * 6.0 - 3.0;
            break;
        }
    }

    void CoordinateTransform::GetBandAndCentreLongitudeByXy(double x,double y,int nGaussType)
    {
        switch(nGaussType)
        {
        case 3:
            m_CentreLongitude = m_BandNo * 3.0;
            break;
        case 6:
            m_CentreLongitude = m_BandNo * 6 - 3;
            break;
        }
    }

}

