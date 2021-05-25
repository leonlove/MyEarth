#include "BB_3DTile.h"
#include "json.hpp"
#include <iostream>
#include <fstream>
using json = nlohmann::json;
using namespace std;

namespace BANBAN
{
	BB_3DTile::BB_3DTile()
	{
		_asset._version = "1.0";
		_asset._tilesetVersion = "e575c6f1-a45b-420a-b172-6449fa6e0a59";
	}


	BB_3DTile::~BB_3DTile()
	{
	}

	bool BB_3DTile::Import(const std::string& inPath)
	{
		return true;
	}

	bool BB_3DTile::Export(const std::string& outPath)
	{
		//1. 打开文件
		ofstream jfile(outPath);
		if (!jfile.is_open())
		{
			cout << "打开" << outPath << "文件失败!" << endl;
			return false;
		}

		json tileset;

		json asset;
		asset["version"] = _asset._version;
		asset["tilesetVersion"] = _asset._tilesetVersion;
		tileset["asset"] = asset;

		json properties;
		json height;
		height["minimum"] = 1;
		height["maximum"] = 241.6;
		properties["Height"] = height;
		tileset["properties"] = properties;

		std::string tilesetJson = tileset.dump(4);
		jfile << tilesetJson;

		return true;
	}

}

