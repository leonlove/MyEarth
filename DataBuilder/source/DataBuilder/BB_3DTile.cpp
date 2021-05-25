#include "BB_3DTile.h"
#include <iostream>
#include <fstream>

namespace BANBAN
{
	BB_3DTile::BB_3DTile()
	{
		_asset._version = "1.0";
		_asset._tilesetVersion = "e575c6f1-a45b-420a-b172-6449fa6e0a59";
		_geometricError = 494.50961650991815;

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
		FillAsset(tileset);

		FillProperties(tileset);

		tileset["geometricError"] = _geometricError;

		FillRoot(tileset);

		std::string tilesetJson = tileset.dump(4);
		jfile << tilesetJson;

		return true;
	}

	void BB_3DTile::FillAsset(json& tileset)
	{
		json asset;
		asset["version"] = _asset._version;
		asset["tilesetVersion"] = _asset._tilesetVersion;
		tileset["asset"] = asset;
	}

	void BB_3DTile::FillProperties(json& tileset)
	{
		json properties;
		json height;
		height["minimum"] = 1;
		height["maximum"] = 241.6;
		properties["Height"] = height;
		tileset["properties"] = properties;
	}

	void BB_3DTile::FillRoot(json& tileset)
	{
		json root;

		json boundingVolume;
		json region;
		region.push_back(-0.0005682966577418737);
		region.push_back(0.8987233516605286);
		region.push_back(0.00011646582098558159);
		region.push_back(0.8990603398325034);
		region.push_back(0);
		region.push_back(241.6);
		boundingVolume["region"] = region;
		root["boundingVolume"] = boundingVolume;

		root["geometricError"] = 268.37878244706053;
		root["refine"] = "ADD";

		json content;
		content["uri"] = "ll.b3dm";
		json content_boundingVolume;
		json content_region;
		content_region.push_back(-0.0005682966577418737);
		content_region.push_back(0.8987233516605286);
		content_region.push_back(0.00011646582098558159);
		content_region.push_back(0.8990603398325034);
		content_region.push_back(0);
		content_region.push_back(241.6);
		content_boundingVolume["region"] = content_region;
		content["boundingVolume"] = content_boundingVolume;
		root["content"] = content;

		tileset["root"] = root;
	}
}

