#ifndef _COMMON_DEUDEFINE_H_
#define _COMMON_DEUDEFINE_H_
#include <map>
#include <vector>

struct DEUMetaData
{
	double    m_dOriginX;   //起始点经度坐标
	double    m_dOriginY;   //起始点纬度坐标
	double    m_dMinX;      //数据左下角点经度坐标
	double    m_dMinY;      //数据左下角点纬度坐标
	double    m_dMaxX;      //数据右上角点经度坐标
	double    m_dMaxY;      //数据右上角点纬度坐标
	unsigned  m_nRow;       //瓦片高度
	unsigned  m_nCol;       //瓦片宽度
	unsigned  m_nDPI;       //DPI
	//std::map<int,double> m_dScaleMap;//每一级的比例尺
	std::vector<int> m_nLayerVec;      //级数
	std::vector<double> m_dSceleVec;   //比例尺
};

struct DEUDataSvrInfo
{
	std::string m_strType;         //数据服务类型(影像/高程/属性等)
	std::string m_strName;         //数据服务名称
	std::string m_strDataSetName;  //数据集名称
	std::string m_strMetaUrl;      //元数据链接
	std::string m_strTileUrl;      //瓦片链接
};

struct DEUTileInfo
{
	double      m_dMinX;   //瓦片左下角点经度坐标
	double      m_dMinY;   //瓦片左下角点纬度坐标
	double      m_dMaxX;   //瓦片右上角点经度坐标
	double      m_dMaxY;   //瓦片右上角点纬度坐标
	unsigned    m_nLevel;
	unsigned    m_nCurRow;
	unsigned    m_nCurCol;
	unsigned    m_nRow;    //瓦片高度
	unsigned    m_nCol;    //瓦片宽度
	void*       m_pData;   //二进制数据
	unsigned    m_nLength; //数据长度
	
};

#endif //_COMMON_DEUDEFINE_H_