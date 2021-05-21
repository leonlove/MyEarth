#pragma once

#include <string>
using namespace std;

namespace BANBAN
{
	class WY_3DTile
	{
	public:
		WY_3DTile();
		~WY_3DTile();

	public:
		bool Import(const std::string& inPath);
		bool Export(const std::string& outPath);
	};
}


