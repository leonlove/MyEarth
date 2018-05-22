#include "DYTriangleIndex.h"


DYTriangleIndex::DYTriangleIndex(void)
{
}

DYTriangleIndex::DYTriangleIndex(int _n1, int _n2, int _n3)
{
	n1 = _n1;
	n2 = _n2;
	n3 = _n3;
}

DYTriangleIndex::~DYTriangleIndex(void)
{
}

bool DYTriangleIndex::isStrip(DYTriangleIndex& Ex)
{
	//- method1
// 	bool b12 = (n1==Ex.n1||n1==Ex.n2||n1==Ex.n3) && (n2==Ex.n1||n2==Ex.n2||n2==Ex.n3);
// 	bool b13 = (n1==Ex.n1||n1==Ex.n2||n1==Ex.n3) && (n3==Ex.n1||n3==Ex.n2||n3==Ex.n3);
// 	bool b23 = (n2==Ex.n1||n2==Ex.n2||n2==Ex.n3) && (n3==Ex.n1||n3==Ex.n2||n3==Ex.n3);
// 
// 	if (b12 || b13 || b23)
// 	{
// 		return true;
// 	}

	//- method2
	bool b1 = (n2==Ex.n1) && (n3==Ex.n3);
	bool b2 = (n2==Ex.n2) && (n3==Ex.n1);

	if (b1 || b2)
	{
		return true;
	}

	//- method3
// 	bool b12 = (n1==Ex.n1||n1==Ex.n2||n1==Ex.n3) && (n2==Ex.n1||n2==Ex.n2||n2==Ex.n3);
// 	bool b13 = (n1==Ex.n1||n1==Ex.n2||n1==Ex.n3) && (n3==Ex.n1||n3==Ex.n2||n3==Ex.n3);
// 	bool b23 = (n2==Ex.n1||n2==Ex.n2||n2==Ex.n3) && (n3==Ex.n1||n3==Ex.n2||n3==Ex.n3);
// 
// 	if (b12 || b13 || b23)
// 	{
// 		bool b1 = (n2==Ex.n1) && (n3==Ex.n3);
// 		bool b2 = (n2==Ex.n2) && (n3==Ex.n1);
// 
// 		if (b1 || b2)
// 		{
// 			return true;
// 		}
// 		else
// 		{
// 			if (b12)
// 			{
// 				int tmp1 = n1;
// 				int tmp2 = n2;
// 				int tmp_ex_noequal = getStripIndex(Ex);
// 
// 				n1 = n3;
// 				n2 = tmp1;
// 				n3 =  tmp2;
// 
// 				Ex.n1 = n2;
// 				Ex.n2 = tmp_ex_noequal;
// 				Ex.n3 = n3;
// 			}
// 
// 			if (b13)
// 			{
// 				int tmp1 = n1;
// 				int tmp_ex_noequal = getStripIndex(Ex);
// 
// 				n1 = n2;
// 				n2 = tmp1;
// 
// 				Ex.n1 = n2;
// 				Ex.n2 = tmp_ex_noequal;
// 				Ex.n3 = n3;
// 			}
// 
// 			if (b23)
// 			{
// 				int tmp_ex_noequal = getStripIndex(Ex);
// 				Ex.n1 = n2;
// 				Ex.n2 = tmp_ex_noequal;
// 				Ex.n3 = n3;
// 			}
// 			return true;
// 		}
// 	}

	return false;
}

int DYTriangleIndex::getStripIndex(const DYTriangleIndex& Ex)
{
	if (Ex.n1!=n1 &&Ex.n1!=n2 &&Ex.n1!=n3)
	{
		return Ex.n1;
	}
	if (Ex.n2!=n1 &&Ex.n2!=n2 &&Ex.n2!=n3)
	{
		return Ex.n2;
	}
	if (Ex.n3!=n1 &&Ex.n3!=n2 &&Ex.n3!=n3)
	{
		return Ex.n3;
	}

	return -1;
}
