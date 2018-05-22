#include "WFSDefine.h"
#include <vector>
#include "..\ModelBuilder\IModelBuilder.h"
#include <LogicalManager/ILayer.h>

namespace wfsb
{
    class BuildGML : public OpenSP::Ref
    {
    public:
        BuildGML(void);
        ~BuildGML(void);
        void initialize(IModelBuilder* pModelBuilder,unsigned nDSCode,const std::string& strUrl,bool bReverse);
        bool convtParameter(logical::ILayer* pLayer,const std::string& strID,const std::vector<double>& ptVec,GMType gmType,const std::map<std::string,std::string>& attrMap);
    protected:
        OpenSP::sp<IModelBuilder> m_pBuilder;
        unsigned                  m_nDSCode;
        std::string               m_strUrl;
        bool                      m_bReverse;
        
    };
}


