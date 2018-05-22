#ifndef SERVICE_MANAGER_H_EBD271D3_C14A_4B59_8132_6A9FCE9102B4_INCLUDE
#define SERVICE_MANAGER_H_EBD271D3_C14A_4B59_8132_6A9FCE9102B4_INCLUDE

#pragma warning(disable:4018)

#include "export.h"
#include "IServiceManager.h"
#include "IService.h"
#include "Service.h"
#include "OpenSP/sp.h"
#include <Common/variant.h>
#include "DEUUrlServer.h"
#include <string>
#include <set>
#include <map>



namespace dk
{
    class IService;
    class ServiceManager : public IServiceManager
    {
    
    public:
        ServiceManager();
        ~ServiceManager();

    private:
        OpenSP::sp<IDataKeeper>                             m_pDataKeeper;
        std::set<OpenSP::sp<dk::IService>>                  m_SetServices;
        std::map<std::string, std::string>                  m_mapApacheHost;
        std::map<std::string, std::vector<std::string>>     m_mapHost;

    protected:
        //- ��ʼ��������������
        void                            init();

        //- ���ݷ�����IP�Ͷ˿����ӷ������ڵ�
        bool                            addService(const std::string &strIP, unsigned nPort, OpenSP::sp<dk::IService>& pServiceObj);

        //- ���ӷ������ڵ�
        bool                            addService(OpenSP::sp<dk::IService> pServiceObj);

        //- ɾ���������ڵ�
        bool                            delService(const std::string &strIP, unsigned nPort);

        //- ��ȡ�������ڵ����
        unsigned                        getServiceCount(void) const;

        //- ����������ȡ�������ڵ�
        OpenSP::sp<IService>            getService(unsigned nIndex);

        //- ���ݷ�����IP�Ͷ˿ڲ��ҷ������ڵ����
        OpenSP::sp<IService>            findService(const std::string &strIP, unsigned nPort);

        //- ����ָ�������ݼ�������ҷ������б�
        bool                            findServiceByCode(unsigned nCode, cmm::variant_data &varServiceList);

        //- �������з���
        bool                            start(void);

        //- �ر����з���
        bool                            stop(void);

        //- ���·�����������
        bool                            update(void);

        //- �ύ���з������ڵ�
        bool                            commit(void);

        //- ��ȡ������״̬��Ϣ
        void                            stateInfo(std::vector<std::string>& vecServiceState);

        //- ���ӷ������ڵ�
        bool                            addServer(std::string& szHost, std::string& ApachePort, std::string& port);

        //- ɾ���������ڵ�
        bool                            delServer(std::string& szHost, std::string& ApachePort, std::string& port);

        //- ��ȡ�������ڵ�
        void                            getServer(std::vector<std::string>& vecSrvHost);

        //- ��������������
        OpenSP::sp<dk::IService>        createService(void);

        //- ���õ�ǰ����������������DataKeeperʵ������
        void                            setDataKeeper(OpenSP::sp<IDataKeeper> pDataKeeper);

        //- ��ȡ��ǰ��������������DataKeeperʵ������
        OpenSP::sp<IDataKeeper>         getDataKeeper();

        //- ����IP����Apache�˿�
        std::string                     findApache(const std::string& strIP);
    };
}

#endif