#ifndef _ITILEBUILDER_H
#define _ITILEBUILDER_H

#ifdef WIN32
#ifdef TILEBUILDER_EXPORTS
    #define TILEBUILDER_EXPORT  __declspec(dllexport)
#else
    #define TILEBUILDER_EXPORT  __declspec(dllimport)
#endif
#else
    #define TILEBUILDER_EXPORT   
    #define TILEBUILDER_EXPORT   
#endif

#include <vector>
#include <openSP/Ref.h>

namespace ea
{
    class IEventAdapter;
}

typedef struct InvalidColor
{
    char r;
    char g;
    char b;
}INVALIDCOLOR;

class ITileBuilder : public OpenSP::Ref
{
public:
    virtual void     setAutoComputeLevel(const bool bCompute)           = 0;
    virtual void     setDEM(const bool isDem)                           = 0;
    virtual bool     getDEM()                                           = 0;
    virtual bool     setMinLevel(const unsigned nLevel)                 = 0;
    virtual unsigned getMinLevel()                                      = 0;
    virtual bool     setMaxLevel(const unsigned nLevel)                 = 0;
    virtual unsigned getMaxLevel()                                      = 0;
    virtual unsigned getMinInterval()                                   = 0;
    virtual unsigned getMaxInterval()                                   = 0;
    virtual bool     setTileSize(const unsigned nTileSize)              = 0;
    virtual unsigned getTileSize()                                      = 0;
    virtual void     setDataSetCode(const unsigned nDataSetCode)        = 0;
    virtual unsigned getDataSetCode()                                   = 0;
    virtual void     setTargetDB(const std::string& strDB)              = 0;
    virtual void     getTargetDB(std::string& strDB)                    = 0;
    virtual bool     addSourceFile(const std::string &strFileName)      = 0;
    virtual bool     getFileCoordinates(const std::string &strFileName, double& dMinX, double& dMinY, double& dMaxX, double& dMaxY) = 0;
    virtual bool     setFileCoordinates(const std::string &strFileName, double dMinX, double dMinY, double dMaxX, double dMaxY) = 0;
    virtual bool     getSourceFile(const unsigned nIndex, std::string& strFileName) = 0;
    virtual bool     removeSourceFile(const unsigned nIndex)            = 0;
    virtual unsigned getSourceFileCount()                               = 0;
    virtual void     clearSourceFile()                                  = 0;
    virtual void     setEventAdapter(ea::IEventAdapter* pEventAdapter)  = 0;
    virtual bool     process()                                          = 0;
    virtual void     stop()                                             = 0;
    virtual bool     addInvalidColor(const std::string &strInvalidColor)= 0;
    virtual bool     delInvalidColor(const std::string &strInvalidColor)= 0;
    virtual std::vector<INVALIDCOLOR>&  getAllInvalidColor()            = 0;
};

TILEBUILDER_EXPORT ITileBuilder* createTileBuilder();

#endif
