#include <string>
#include <vector>

namespace deunw
{
    class DEUServerConf
    {
    public:
        DEUServerConf(void);
        ~DEUServerConf(void);

        //创建服务
        bool createService(const std::string& strHost,const std::string& strPath,const std::vector<std::string>& strPortVec,
                           std::string& strErr = std::string(),bool bIsMainSvr = false,const std::string& strMainPort = "");
    private:
        bool postFun(const std::string& strUrl, const std::vector<char> &vecBuffer, std::string& strErr);
    private:
        unsigned m_nRetry;
                            

    };
}


