#include <iostream>
#include "BB_3DTile.h"
using namespace std;
using namespace BANBAN;


int main()
{
	//1. 通过Assimp读取通用模型数据
	//2. Assimp导出Model
	BB_3DTile bb3dtile;
	bb3dtile.Export("../../temp/tileset.json");
	return 0;
}