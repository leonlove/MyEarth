#include <iostream>
#include "BB_3DTile.h"
using namespace std;
using namespace BANBAN;


int main()
{
	//1. ͨ��Assimp��ȡͨ��ģ������
	//2. Assimp����Model
	BB_3DTile bb3dtile;
	bb3dtile.Export("../../temp/tileset.json");
	return 0;
}