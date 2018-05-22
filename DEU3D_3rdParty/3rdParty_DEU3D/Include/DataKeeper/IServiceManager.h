#ifndef I_SERVICE_MANAGER_H_815F9E0B_9F93_44AA_AACD_551587A37D06_INCLUDE
#define I_SERVICE_MANAGER_H_815F9E0B_9F93_44AA_AACD_551587A37D06_INCLUDE

#include <OpenSP/Ref.h>
#include <string>
#include <Common/variant.h>


namespace dk
{

class IDataKeeper;
class IService;
class IServiceManager : public OpenSP::Ref
{
public:
    //- ��ʼ��������������
    virtual void                        init()                                                                                         = 0;
    
    //- ���ݷ�����IP�Ͷ˿����ӷ������ڵ�
    virtual bool                        addService(const std::string &strIP, unsigned nPort, OpenSP::sp<dk::IService>& pServiceObj)    = 0;

    //- ���ӷ������ڵ�
    virtual bool                        addService(OpenSP::sp<dk::IService> pServiceObj)                                               = 0;

    //- ɾ���������ڵ�
    virtual bool                        delService(const std::string &strIP, unsigned nPort)                                           = 0;

    //- ��ȡ�������ڵ����
    virtual unsigned                    getServiceCount(void) const                                                                    = 0;

    //- ����������ȡ�������ڵ�
    virtual OpenSP::sp<IService>        getService(unsigned nIndex)                                                                    = 0;

    //- ���ݷ�����IP�Ͷ˿ڲ��ҷ������ڵ����
    virtual OpenSP::sp<IService>        findService(const std::string &strIP, unsigned nPort)                                          = 0;

    //- ����ָ�������ݼ�������ҷ������б�
    virtual bool                        findServiceByCode(unsigned nCode, cmm::variant_data &varServiceList)                           = 0;

    //- �������з���
    virtual bool                        start(void)                                                                                    = 0;

    //- �ر����з���
    virtual bool                        stop(void)                                                                                     = 0;

    //- ���·�����������
    virtual bool                        update(void)                                                                                   = 0;

    //- �ύ���з������ڵ�
    virtual bool                        commit(void)                                                                                   = 0;

    //- ��ȡ������״̬��Ϣ
    virtual void                        stateInfo(std::vector<std::string>& vecServiceState)                                           = 0;

    //- ���ӷ������ڵ�
    virtual bool                        addServer(std::string& szHost, std::string& ApachePort, std::string& port)                     = 0;
    
    //- ��ȡ�������ڵ�
    virtual void                        getServer(std::vector<std::string>& vecSrvHost)                                                = 0;

    //- ɾ���������ڵ�
    virtual bool                        delServer(std::string& szHost, std::string& ApachePort, std::string& port)                     = 0;

    //- ��������������
    virtual OpenSP::sp<dk::IService>    createService(void)                                                                            = 0;

    //- ���õ�ǰ����������������DataKeeperʵ������
    virtual void                        setDataKeeper(OpenSP::sp<IDataKeeper> pDataKeeper)                                             = 0;

    //- ��ȡ��ǰ��������������DataKeeperʵ������
    virtual OpenSP::sp<IDataKeeper>     getDataKeeper()                                                                                = 0;

    //- ����IP����Apache�˿�
    virtual std::string                 findApache(const std::string& strIP)                                                                 = 0;

};

DATAKEEPER_API dk::IServiceManager* CreateServiceManager(void);

}

#endif
