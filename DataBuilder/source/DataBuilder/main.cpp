#include <iostream>
#include "BB_3DTile.h"
using namespace std;
using namespace BANBAN;


int main()
{
	BB_3DTile _bb3dtile;
	_bb3dtile.Export("../../temp/tileset.json");
	return 0;
}