#ifndef _EXTERNALSERVICE_DEUDEFINE_H_B93D9C5C_378F_45F6_B6E3_DB301FA7BBA3_
#define _EXTERNALSERVICE_DEUDEFINE_H_B93D9C5C_378F_45F6_B6E3_DB301FA7BBA3_

#include <string>
#include <map>
#include <vector>

#define  EXTERNAL_DATASET_CODE 99

struct DEUMatrixInfo
{
    double    m_dScale;      //������
    double    m_dTopLeftX;   //��ʼ�㾭������
    double    m_dTopLeftY;   //��ʼ��γ������
    unsigned  m_nRow;        //��Ƭ�߶�
    unsigned  m_nCol;        //��Ƭ���
    unsigned  m_nWidth;      //������Ƭ����
    unsigned  m_nHeight;     //������Ƭ����
    std::string m_strMatrix; //ID
};

struct DEUMetaData
{
    double    m_dMinX;      //�������½ǵ㾭������
    double    m_dMinY;      //�������½ǵ�γ������
    double    m_dMaxX;      //�������Ͻǵ㾭������
    double    m_dMaxY;      //�������Ͻǵ�γ������
    std::string  m_strLayer;
    std::string  m_strStyle;
    std::string  m_strFormat;
    std::string  m_strMatrixSet;
    std::map<double,DEUMatrixInfo>       m_matrixMap;
    std::map<std::string,DEUMatrixInfo*> m_matrixPtrMap;
};

struct DEUTileInfo
{
    double      m_dMinX;   //��Ƭ���½ǵ㾭������
    double      m_dMinY;   //��Ƭ���½ǵ�γ������
    double      m_dMaxX;   //��Ƭ���Ͻǵ㾭������
    double      m_dMaxY;   //��Ƭ���Ͻǵ�γ������
    unsigned    m_nLevel;
    unsigned    m_nCurRow;
    unsigned    m_nCurCol;
    unsigned    m_nRow;    //��Ƭ�߶�
    unsigned    m_nCol;    //��Ƭ���
    void*       m_pData;   //����������
    unsigned    m_nLength; //���ݳ���
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