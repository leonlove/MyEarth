#ifndef _COMMON_DEUDEFINE_H_
#define _COMMON_DEUDEFINE_H_
#include <map>
#include <vector>

struct DEUMetaData
{
	double    m_dOriginX;   //��ʼ�㾭������
	double    m_dOriginY;   //��ʼ��γ������
	double    m_dMinX;      //�������½ǵ㾭������
	double    m_dMinY;      //�������½ǵ�γ������
	double    m_dMaxX;      //�������Ͻǵ㾭������
	double    m_dMaxY;      //�������Ͻǵ�γ������
	unsigned  m_nRow;       //��Ƭ�߶�
	unsigned  m_nCol;       //��Ƭ���
	unsigned  m_nDPI;       //DPI
	//std::map<int,double> m_dScaleMap;//ÿһ���ı�����
	std::vector<int> m_nLayerVec;      //����
	std::vector<double> m_dSceleVec;   //������
};

struct DEUDataSvrInfo
{
	std::string m_strType;         //���ݷ�������(Ӱ��/�߳�/���Ե�)
	std::string m_strName;         //���ݷ�������
	std::string m_strDataSetName;  //���ݼ�����
	std::string m_strMetaUrl;      //Ԫ��������
	std::string m_strTileUrl;      //��Ƭ����
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

#endif //_COMMON_DEUDEFINE_H_