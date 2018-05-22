#ifndef	__MATH_2D_H__
#define	__MATH_2D_H__

#ifndef	MATH2D_EXPORTS
#define	MATH2D_EXPORTS_LIB __declspec(dllimport)
#else
#define	MATH2D_EXPORTS_LIB __declspec(dllexport)
#endif

#define	POLYGON_HANDLE void *

extern "C"
{
/*通过坐标串构建一个polygon对象，单环的polygon。
 *对于多环的polygon底层虽然可以支持，但是在目前的应用场景中不需要，所以暂时不暴露出来。
 *传入参数
 *			coords:	传入二维坐标串，内容为按照顺序的点的x和y坐标，如，x1,y1,x2,y2.....xn,yn。
 *			ptnum:	传入坐标点个数，描述coords中一共有几个点，coords的长度应该是ptnum乘以维度。
 *返回
 *			成功返回一个polygon对象的句柄，失败返回NULL
 */
MATH2D_EXPORTS_LIB 
POLYGON_HANDLE createpolygon(double *coords, int ptnum);

/*
 *释放由createpolygon方法创建的polygon对象
 */
MATH2D_EXPORTS_LIB 
void freepolygon(POLYGON_HANDLE pg);

/*判断一个box和polygon的关系
 *传入参数
 *			pg:		传入polygon对象
 *			xmin, xmax, ymin, ymax:	传入一个box
 *返回
 *			0表示box在polygon内部，1表示box和polygon边线相交，2表示box在polygon外部，-1表示失败
 */
MATH2D_EXPORTS_LIB 
int boxinpolygon(POLYGON_HANDLE pg, double xmin, double ymin, double xmax, double ymax);

}

#ifndef	MATH2D_EXPORTS
//#pragma comment(lib, "math2d.lib")
#endif



#endif	//__MATH_2D_H__
