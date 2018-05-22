#ifndef CTRIANGLE_INDEX_H
#define CTRIANGLE_INDEX_H

class DYTriangleIndex
{
public:
	DYTriangleIndex(void);
	DYTriangleIndex(int _n1, int _n2, int _n3);
	~DYTriangleIndex(void);

	bool isStrip(DYTriangleIndex& Ex);
	int getStripIndex(const DYTriangleIndex& Ex);

public:
	int n1;
	int n2;
	int n3;
};

#endif

