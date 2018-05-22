#ifndef _EXTERNALSERVICE_DEUDEFINE_H_B93D9C5C_378F_45F6_B6E3_DB301FA7BBA3_
#define _EXTERNALSERVICE_DEUDEFINE_H_B93D9C5C_378F_45F6_B6E3_DB301FA7BBA3_

#include <string>
#include <map>
#include <vector>

#define  EXTERNAL_DATASET_CODE 99

struct DEUMatrixInfo
{
    double    m_dScale;      //比例尺
    double    m_dTopLeftX;   //起始点经度坐标
    double    m_dTopLeftY;   //起始点纬度坐标
    unsigned  m_nRow;        //瓦片高度
    unsigned  m_nCol;        //瓦片宽度
    unsigned  m_nWidth;      //横向瓦片个数
    unsigned  m_nHeight;     //纵向瓦片个数
    std::string m_strMatrix; //ID
};

struct DEUMetaData
{
    double    m_dMinX;      //数据左下角点经度坐标
    double    m_dMinY;      //数据左下角点纬度坐标
    double    m_dMaxX;      //数据右上角点经度坐标
    double    m_dMaxY;      //数据右上角点纬度坐标
    std::string  m_strLayer;
    std::string  m_strStyle;
    std::string  m_strFormat;
    std::string  m_strMatrixSet;
    std::map<double,DEUMatrixInfo>       m_matrixMap;
    std::map<std::string,DEUMatrixInfo*> m_matrixPtrMap;
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

//WMS
struct DEUStyleInfo
{
	std::string m_strStyleInfo;
	std::string m_strImageFormat;
};

struct DEULayerInfo
{
	std::string					m_strLayerName;
	std::vector<DEUStyleInfo>	m_vecStyleInfo;
	std::vector<std::string>	m_vecCRS;
	double						m_dMinX;
	double						m_dMinY;
	double						m_dMaxX;
	double						m_dMaxY;
};

//Morcator
struct DEUMorcatorInfo
{
	std::string					m_strMapService;
	std::vector<std::string>	m_vecMapStyle;
};
#endif