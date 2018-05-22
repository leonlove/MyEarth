#include "DataSetSegment.h"
#include "Service.h"

namespace dk
{

    dk::IDataSetSegment* CreateDataSetSegment(void)
    {
         OpenSP::sp<IDataSetSegment> pDataSetSegment = new DataSetSegment();
         return pDataSetSegment.release();
    }

    DataSetSegment::DataSetSegment(void) : m_pParent(NULL)
    {
        m_Min = 0;
        m_Max = 0;
    }


    DataSetSegment::~DataSetSegment(void)
    {
    }


    bool DataSetSegment::addService(OpenSP::sp<IService> pServiceObj)
    {
        m_SetIService.insert(pServiceObj);

        return true;
    }


    unsigned DataSetSegment::getServicesCount(void) const
    {
        return (unsigned)m_SetIService.size();
    }


    bool DataSetSegment::deleteService(unsigned nIndex)
    {
        if(nIndex > m_SetIService.size()) return false;

        unsigned nCounter=0;
        std::set<OpenSP::sp<IService>>::iterator it(m_SetIService.begin());
        for(; it != m_SetIService.end(); it++)
        {
            if(nCounter == nIndex)
            {
                m_SetIService.erase(it);
                break;
            }
            nCounter++;
        }

        return true;
    }


    bool DataSetSegment::deleteAllService()
    {
        m_SetIService.clear();

        return true;
    }


    OpenSP::sp<IService> DataSetSegment::getService(unsigned nIndex)
    {
        if(nIndex < m_SetIService.size())
        {
            size_t i = 0;
            std::set<OpenSP::sp<dk::IService>>::iterator it = m_SetIService.begin();
            for (; it!=m_SetIService.end(); ++it)
            {
                if (nIndex == i)
                {
                    return *it;
                }

                i++;
            }
        }

        return NULL;
    }


    void DataSetSegment::setParent(OpenSP::sp<IDataSet> pParent)
    {
        if (pParent != NULL)
        {
            m_pParent = pParent;
        }
    }


    OpenSP::sp<IDataSet> DataSetSegment::getParent()
    {
        return m_pParent;
    }

    unsigned DataSetSegment::getDataItemCount(void)
    {


        return 0;
    }

    bool DataSetSegment::getIndicesData(const std::string& strHost, 
                                        const std::string& strPort, 
                                        unsigned nDSCode, 
                                        unsigned nOffset,
                                        unsigned nCount, 
                                        std::vector<ID>& vecIds)
    {


        return true;
    }

}

