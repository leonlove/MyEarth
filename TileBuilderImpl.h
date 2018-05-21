#ifndef _TileBuilderImpl_H
#define _TileBuilderImpl_H

#include "ITileBuilder.h"
#include "Source.h"
#include "TaskQueue.h"
#include "TilingThreadPool.h"
#include "OpenSP/sp.h"
#include "EventAdapter/IEventAdapter.h"
#include <DEUDBProxy/IDEUDBProxy.h>
#include <osg\CoordinateSystemNode>



class TileBuilderImpl : public ITileBuilder
{
public:
    TileBuilderImpl(void);
    ~TileBuilderImpl(void);

public:
    virtual void     setAutoComputeLevel(const bool bCompute);
    virtual bool     getAutoComputeLevel();
    virtual void     setDEM(const bool isDem);
    virtual bool     getDEM();
    virtual bool     setMinLevel(const unsigned nLevel);
    virtual unsigned getMinLevel();
    virtual bool     setMaxLevel(const unsigned nLevel);
    virtual unsigned getMaxLevel();
    virtual unsigned getMinInterval();
    virtual unsigned getMaxInterval();
    virtual bool     setTileSize(const unsigned nTileSize);
    virtual unsigned getTileSize();
    virtual void     setDataSetCode(const unsigned nDataSetCode);
    virtual unsigned getDataSetCode();
    virtual void     setTargetDB(const std::string& strDB);
    virtual void     getTargetDB(std::string& strDB);
    virtual bool     addSourceFile(const std::string &strFileName);
    virtual bool     getFileCoordinates(const std::string &strFileName, double& dMinX, double& dMinY, double& dMaxX, double& dMaxY);
    virtual bool     setFileCoordinates(const std::string &strFileName, double dMinX, double dMinY, double dMaxX, double dMaxY);
    virtual bool     getSourceFile(const unsigned nIndex, std::string& strFileName);
    virtual bool     removeSourceFile(const unsigned nIndex);
    virtual unsigned getSourceFileCount();
    virtual void     clearSourceFile();
    virtual void     setEventAdapter(ea::IEventAdapter* pEventAdapter);
    virtual bool     process();
    virtual void     stop();
    virtual bool     addInvalidColor(const std::string &strInvalidColor);
    virtual bool     delInvalidColor(const std::string &strInvalidColor);
    virtual std::vector<INVALIDCOLOR>&  getAllInvalidColor();
    std::vector<INVALIDCOLOR>&  getVecInvalidColor();

private:
    //生产一层瓦片
    bool tileBuildLevel(unsigned int nLevel);
    //生产一层中的一行瓦片
    bool tileBuildRow(unsigned int nLevel, unsigned int nRow);
    //生产一层中的单个瓦片
    bool tileBuild(unsigned int nLevel, unsigned int nRow, unsigned int nCol);

    void writeConfigurationFile();
    void getInvalidColor();

    bool setLevel(const unsigned int nMinLevel, const unsigned int nMaxLevel);
    void getLevel(unsigned int &nMinLevel, unsigned int &nMaxLevel);


    bool computePyramidInfo();
    unsigned int computeMinInterval();
    unsigned int computeMaxInterval();
    unsigned int computeMinMaxLevel();

private:
    bool         m_bStop;
    int          m_nDataSetCode;
    bool         m_bHeightField;
    bool         m_bAutoComputeLevel;
    unsigned int m_nTileSize;
    unsigned int m_nMinLevel;   //用户选择最小层级
    unsigned int m_nMaxLevel;   //用户选择最大层级
    unsigned int m_nMinInterval;//自动计算可达到最小层级
    unsigned int m_nMaxInterval;//自动计算可达到最大层级

    std::string  m_strTargetDB;
    OpenSP::sp<deudbProxy::IDEUDBProxy> m_pTargetDB;

    cmm::math::Box2d                m_TotalAreaBound;
    OpenSP::sp<TaskQueue>           m_pTaskQueue;
    OpenSP::sp<TilingThreadPool>    m_pTilingThreadPool;
    OpenSP::sp<ea::IEventAdapter>   m_pEventAdapter;
    std::vector<OpenSP::sp<Source>> m_vecSourceFile;
    std::vector<INVALIDCOLOR>       m_vecInvalidColor;
};

#endif
