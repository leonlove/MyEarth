#pragma once

#include <string>
#include "json.hpp"
using namespace std;
using json = nlohmann::json;

namespace BANBAN
{
	/*
	 * ���ݰ汾��Ϣ
	 */
	struct Asset {
		std::string _version;
		std::string _tilesetVersion;
	};

	/*
	 * ���ݼ����������Ϣ
	 */
	struct Porperties {
		double	_height[2];
		double	_longitude[2];
		double	_latitude[2];
	};

	struct BoundingVolume {
		double	_region[6];
	};

	struct Content {
		std::string		_uri;
		BoundingVolume  _boundingVolume;
	};

	struct Node {
		BoundingVolume	_boundingVolume;
		double			_geometricError;
		std::string		_refine;
		Content			_Content;
		Node*			_child;
	};

	class BB_3DTile
	{
	public:
		BB_3DTile();
		~BB_3DTile();

	public:
		bool Import(const std::string& inPath);

		/*
		 * ��3DTile���ݵ�����ָ��·��
		 */
		bool Export(const std::string& outPath);

	private:
		void FillAsset(json& tileset);

		void FillProperties(json& tileset);

		void FillRoot(json& tileset);

	private:
		Asset		_asset;
		Porperties	_properties;
		double		_geometricError;
		Node		_root;
	};
}


