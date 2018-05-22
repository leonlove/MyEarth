//////////////////////////////////////////////////////////////////////
//
// DYCommon.h: global definition for the tidu classes.
//
//////////////////////////////////////////////////////////////////////

#ifndef DY_COMMON_H
#define DY_COMMON_H

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include <windows.h>
#include <mmsystem.h>

#include <iostream>

#include <io.h>
#include <stdarg.h>
#include <stdio.h>
#include <tchar.h>
#include <assert.h>
#include <math.h>
#include <sys/timeb.h>
#include <time.h>

#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <map>

using namespace std;

typedef string                                       DYString;

#define DY_NO_IMPLEMENTATION                         NULL
#define DY_TEXT                                      _T

#define DY_MAX_PATH                                  0x00001000

#define DY_INT_INFINITY_PLUS                         1000000000
#define DY_INT_INFINITY_MINUS                       -1000000000

#define DY_FLOAT_INFINITY_PLUS                       1000000000.0f
#define DY_FLOAT_INFINITY_MINUS                     -1000000000.0f

#define DY_DOUBLE_INFINITY_PLUS                      1000000000000000000.0
#define DY_DOUBLE_INFINITY_MINUS                    -1000000000000000000.0

#define DY_FLOAT_PRECISION0                          1e+000f
#define DY_FLOAT_PRECISION1                          1e-001f
#define DY_FLOAT_PRECISION2                          1e-002f
#define DY_FLOAT_PRECISION3                          1e-003f
#define DY_FLOAT_PRECISION4                          1e-004f
#define DY_FLOAT_PRECISION5                          1e-005f
#define DY_FLOAT_PRECISION6                          1e-006f
#define DY_FLOAT_PRECISION7                          1e-007f
#define DY_FLOAT_PRECISION8                          1e-008f
#define DY_FLOAT_PRECISION9                          1e-009f

#define DY_PLANE_THICKNESS                           DY_FLOAT_PRECISION5
#define DY_NORMAL_PRECISION                          DY_FLOAT_PRECISION5
#define DY_RADIAN_PRECISION                          DY_FLOAT_PRECISION5
#define DY_COSINE_PRECISION                          DY_FLOAT_PRECISION5
#define DY_AREA_PRECISION                            DY_FLOAT_PRECISION5

#ifndef DY_LENGTH_PRECISION
#define DY_LENGTH_PRECISION                          DY_FLOAT_PRECISION5
#endif

#define DY_SQRT_2                                    1.41421356237309504880168872420970f
#define DY_1_SQRT_2                                  0.70710678118654752440084436210485f
#define DY_PI                                        3.14159265358979323846264338327950f
#define DY_2PI                                       6.28318530717958647692528676655900f
#define DY_1_DIV_PI                                  0.31830988618379067153776752674503f
#define DY_1_DIV_2PI                                 0.15915494309189533576888376337251f
#define DY_PI_DIV_2                                  1.57079632679489661923132169163980f
#define DY_PI_DIV_3                                  1.04719755119659774615421446109320f
#define DY_PI_DIV_4                                  0.78539816339744830961566084581988f
#define DY_PI_DIV_6                                  0.52359877559829887307710723054658f
#define DY_3PI_DIV_2                                 4.71238898038468985769396507491930f
#define DY_5PI_DIV_2                                 7.85398163397448309615660845819880f
#define DY_2PI_DIV_3                                 2.09439510239319549230842892218630f
#define DY_4PI_DIV_3                                 4.18879020478639098461685784437270f

#define DY_ANGLE_TO_RADIAN(fAngle)                   ((fAngle)*(0.017453292519943295769236907684886f))
#define DY_RADIAN_TO_ANGLE(fRadian)                  ((fRadian)*(57.2957795130823208767981548141050f))

#define DY_SWAP(a,b,type)                            {type t=(a); (a)=(b); (b)=(t);}

#define DY_DELETE_PTR(p)                             {if((p   )!=NULL) {delete      (p);            (p   )=NULL;}}
#define DY_DELETE_ARR(p)                             {if((p   )!=NULL) {delete[]    (p);            (p   )=NULL;}}
#define DY_RELEASE_COM(p)                            {if((p   )!=NULL) {(p)->Release();             (p   )=NULL;}}
#define DY_CLOSE_FILE(fp)                            {if((fp  )!=NULL) {fclose      (fp);           (fp  )=NULL;}}
#define DY_CLOSE_HANDLE(h)                           {if((h   )!=NULL) {CloseHandle (h);            (h   )=NULL;}}
#define DY_CLOSE_FIND(h)                             {if((h   )!=NULL) {FindClose   (h);            (h   )=NULL;}}
#define DY_RELEASE_DC(hWnd,hDC)                      {if((hDC )!=NULL) {::ReleaseDC ((hWnd),(hDC)); (hDC )=NULL;}}
#define DY_DELETE_DC(hDC)                            {if((hDC )!=NULL) {DeleteDC    (hDC);          (hDC )=NULL;}}
#define DY_DELETE_OBJECT(hObj)                       {if((hObj)!=NULL) {DeleteObject(hObj);         (hObj)=NULL;}}
#define DY_DELETE_ARR2(pp,n)                         {if((pp)!=NULL) {INT i=0; for(i=0;i<(n);i++) {DY_DELETE_ARR((pp)[i]);} DY_DELETE_ARR(pp);}}
#define DY_DELETE_ARR3(pp,n,m)                       {if((pp)!=NULL) {INT i=0; for(i=0;i<(n);i++) {DY_DELETE_ARR2(((pp)[i]),m);} DY_DELETE_ARR(pp);}}
#define DY_DELETE_PTR2(pp,n)                         {if((pp)!=NULL) {INT i=0; for(i=0;i<(n);i++) {DY_DELETE_PTR((pp)[i]);} DY_DELETE_ARR(pp);}}

inline INT   DY_ABS  (const INT   nData)                                            {return (((nData)>=0)?(nData):(-(nData)));}
inline FLOAT DY_ACOS (const FLOAT fData)                                            {if((fData)>=1.0f) return 0.0f; if((fData)<=-1.0f) return DY_PI; const FLOAT fRadian=(FLOAT)(acos(fData)); if(fRadian<0.0f) return 0.0f; if(fRadian>DY_PI) return DY_PI; return fRadian;}
inline FLOAT DY_ACOT (const FLOAT fData)                                            {return (((fData)==0.0f)?(DY_PI_DIV_2):((FLOAT)(atan(1.0f/(fData)))));}
inline FLOAT DY_ASIN (const FLOAT fData)                                            {if((fData)>=1.0f) return DY_PI_DIV_2; if((fData)<=-1.0f) return -DY_PI_DIV_2; const FLOAT fRadian=(FLOAT)(asin(fData)); if(fRadian<-DY_PI_DIV_2) return -DY_PI_DIV_2; if(fRadian>DY_PI_DIV_2) return DY_PI_DIV_2; return fRadian;}
inline FLOAT DY_ATAN (const FLOAT fData)                                            {return (((fData)>=DY_FLOAT_INFINITY_PLUS)?(DY_PI_DIV_2):(((fData)<=DY_FLOAT_INFINITY_MINUS)?(-DY_PI_DIV_2):((FLOAT)(atan(fData)))));}
inline INT   DY_CEIL (const FLOAT fData)                                            {return ((INT)(ceil(fData)));}
inline FLOAT DY_COS  (const FLOAT fData)                                            {FLOAT fCosine=(FLOAT)(cos(fData)); if(fCosine<-1.0f) {fCosine=-1.0f;} if(fCosine>1.0f) {fCosine=1.0f;} return fCosine;}
inline FLOAT DY_COSH (const FLOAT fData)                                            {return ((FLOAT)(cosh(fData)));}
inline FLOAT DY_EXP  (const FLOAT fData)                                            {return ((FLOAT)(exp (fData)));}
inline FLOAT DY_FABS (const FLOAT fData)                                            {return (((fData)>=0.0f)?(fData):(-(fData)));}
inline INT   DY_FLOOR(const FLOAT fData)                                            {INT nData=(INT)(fData); if(nData>(fData)) {nData--;} return nData;}
inline INT   DY_INT  (const FLOAT fData)                                            {return ((INT)(fData));}
inline INT   DY_LIMIT(const INT   nData,const INT   nLower,const INT   nUpper)      {return (((nData)>=(nUpper))?((nUpper)):(((nData)<=(nLower))?((nLower)):(nData)));}
inline FLOAT DY_LIMIT(const FLOAT fData,const FLOAT fLower,const FLOAT fUpper)      {return (((fData)>=(fUpper))?((fUpper)):(((fData)<=(fLower))?((fLower)):(fData)));}
inline FLOAT DY_LOG  (const FLOAT fData)                                            {return (((fData)>0.0f)?((FLOAT)(log  (fData))):(0.0f));}
inline FLOAT DY_LOG10(const FLOAT fData)                                            {return (((fData)>0.0f)?((FLOAT)(log10(fData))):(0.0f));}
inline INT   DY_MAX  (const INT   nData1,const INT   nData2)                        {return (((nData1)>(nData2))?(nData1):(nData2));}
inline FLOAT DY_MAX  (const FLOAT fData1,const FLOAT fData2)                        {return (((fData1)>(fData2))?(fData1):(fData2));}
inline INT   DY_MIN  (const INT   nData1,const INT   nData2)                        {return (((nData1)<(nData2))?(nData1):(nData2));}
inline FLOAT DY_MIN  (const FLOAT fData1,const FLOAT fData2)                        {return (((fData1)<(fData2))?(fData1):(fData2));}
inline FLOAT DY_POW  (const FLOAT fData1,const FLOAT fData2)                        {return ((FLOAT)(pow((fData1),(fData2))));}
inline INT   DY_ROUND(const FLOAT fData)                                            {return ((INT)((fData)+0.5f));}
inline FLOAT DY_SIN  (const FLOAT fData)                                            {FLOAT fSine=(FLOAT)(sin(fData)); if(fSine<-1.0f) {fSine=-1.0f;} if(fSine>1.0f) {fSine=1.0f;} return fSine;}
inline FLOAT DY_SINH (const FLOAT fData)                                            {return ((FLOAT)(sinh(fData)));}
inline FLOAT DY_SQRT (const FLOAT fData)                                            {return (((fData)>0.0f)?((FLOAT)(sqrt(fData))):(0.0f));}
inline FLOAT DY_TAN  (const FLOAT fData)                                            {return ((FLOAT)(tan (fData)));}
inline FLOAT DY_TANH (const FLOAT fData)                                            {return ((FLOAT)(tanh(fData)));}
inline UCHAR DY_UCHAR(const FLOAT fData)                                            {return (((fData)>=255.0f)?(255):(((fData)<=0.0f)?(0):((UCHAR)(fData+0.5f))));}
inline BOOL  DY_EQUAL(const FLOAT fData1,const FLOAT fData2,const FLOAT fPrecision) {return (DY_FABS(fData1-fData2)<=fPrecision);}

class DYStringList;

extern VOID CreateFolder(const CHAR *szDirectory);
extern BOOL EnumFiles   (const CHAR *szRootDir,DYStringList &pstrFullPaths);
extern INT  Shift       (      INT nNumber);
extern INT  Normalize   (const INT nNumber);

#endif  // #ifndef DY_COMMON_H
