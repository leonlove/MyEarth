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
/*ͨ�����괮����һ��polygon���󣬵�����polygon��
 *���ڶ໷��polygon�ײ���Ȼ����֧�֣�������Ŀǰ��Ӧ�ó����в���Ҫ��������ʱ����¶������
 *�������
 *			coords:	�����ά���괮������Ϊ����˳��ĵ��x��y���꣬�磬x1,y1,x2,y2.....xn,yn��
 *			ptnum:	������������������coords��һ���м����㣬coords�ĳ���Ӧ����ptnum����ά�ȡ�
 *����
 *			�ɹ�����һ��polygon����ľ����ʧ�ܷ���NULL
 */
MATH2D_EXPORTS_LIB 
POLYGON_HANDLE createpolygon(double *coords, int ptnum);

/*
 *�ͷ���createpolygon����������polygon����
 */
MATH2D_EXPORTS_LIB 
void freepolygon(POLYGON_HANDLE pg);

/*�ж�һ��box��polygon�Ĺ�ϵ
 *�������
 *			pg:		����polygon����
 *			xmin, xmax, ymin, ymax:	����һ��box
 *����
 *			0��ʾbox��polygon�ڲ���1��ʾbox��polygon�����ཻ��2��ʾbox��polygon�ⲿ��-1��ʾʧ��
 */
MATH2D_EXPORTS_LIB 
int boxinpolygon(POLYGON_HANDLE pg, double xmin, double ymin, double xmax, double ymax);

}

#ifndef	MATH2D_EXPORTS
//#pragma comment(lib, "math2d.lib")
#endif



#endif	//__MATH_2D_H__
