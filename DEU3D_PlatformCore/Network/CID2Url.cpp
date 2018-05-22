#include "define.h"
#include "Common/md2.h"
#include "Common/crc.h"

namespace deunw
{

    ID2Url::ID2Url()
    {
        //     m_rcdNum = 1024;
        //     init();
    }

    ID2Url::ID2Url(int nRcdNum)
    {
        m_rcdNum = nRcdNum;
        init();
    }


    ID2Url::~ID2Url()
    {

    }


    void ID2Url::SetInit(int nRcdNum)
    {
        m_rcdNum = nRcdNum;
        init();
    }


    bool ID2Url::GetNum(BYTE *pID, int len, UINT32 &Num) const
    {
        if (NULL == pID || len <= 0)
        {
            Num = 0;
            return false;
        }
#ifdef    __MD2_HASH__
        Num = cmm:createHashMD2(pID, len);
#else
        Num = cmm::createHashCRC32(pID, len);
#endif

        return true;
    }

    std::vector<rcd>::iterator FindRcd(std::vector<rcd>::iterator begin, std::vector<rcd>::iterator end, UINT32 num)
    {
        const int n = end - begin;
        std::vector<rcd>::iterator p = begin + n/2;
        const int ret = p->NumInRcd(num);
        if (ret < 0)
        {
            return FindRcd(begin, p-1, num);
        }
        else if (ret == 0)
        {
            return p;
        }
        else
        {
            return FindRcd(p+1, end, num);
        }
    }

    rcd* ID2Url::GetRcdByNum(UINT32 num)
    {
        //std::vector<rcd>::const_iterator p = FindRcd(m_rcds.begin(), m_rcds.end(), num);
        std::vector<rcd>::iterator it = m_rcds.begin();
        for (; it != m_rcds.end(); ++it)
        {
            if (it->NumInRcd(num) == 0)
            {
                return it._Ptr;
            }
        }

        return NULL;
    }

    UINT32 ID2Url::GetRcdNum() const
    {
        return m_rcds.size();
    }

    rcd * ID2Url::GetRcdByIndex(UINT32 index)
    {
        if (index < 0 || index >= m_rcds.size())
        {
            return NULL;
        }
        return &m_rcds[index];
    }

    bool ID2Url::DelRcd(UINT32 index)
    {
        if (index < 0 || index >= m_rcds.size())
        {
            return false;
        }

        std::vector<rcd>::iterator p = m_rcds.begin() + index;
        std::vector<rcd>::iterator p1;
        if (index == 0)
        {
            p1 = p+1;
            p1->SetBegin(p->GetBegin());
        }
        else
        {
            p1 = p-1;
            p1->SetEnd(p->GetEnd());
        }

        m_rcds.erase(p);

        return true;
    }

    bool ID2Url::SetRcd(rcd &r)
    {
        UINT32 begin = r.GetBegin();
        UINT32 end = r.GetEnd();
        std::vector<rcd>::iterator pbegin = FindRcd(m_rcds.begin(), m_rcds.end(), begin);
        std::vector<rcd>::iterator pend = FindRcd(m_rcds.begin(), m_rcds.end(), end);
        int beginindex = pbegin - m_rcds.begin();

        if (pbegin == pend)
        {
            rcd r1(*pbegin);
            rcd r2(r);
            rcd r3(*pbegin);

            r1.SetEnd(r.GetBegin() - 1);
            r3.SetEnd(r.GetEnd() + 1);

            pend = m_rcds.begin() + beginindex + 1;
            if ((r.GetBegin() != 0) && (!r1.IsEmpty()))
            {
                m_rcds.insert(pend, r1);
                pend = m_rcds.begin() + beginindex + 2;
            }
            if (!r2.IsEmpty())
            {
                m_rcds.insert(pend, r2);
                pend = m_rcds.begin() + beginindex + 3;
            }
            if ((r.GetEnd() != UINT_MAX) && (!r3.IsEmpty()))
            {
                m_rcds.insert(pend, r3);
            }
            pbegin = m_rcds.begin() + beginindex;
            m_rcds.erase(pbegin);

            return true;
        }
        else
        {
            pbegin++;
            m_rcds.erase(pbegin, pend);
            pend = m_rcds.begin() + (beginindex + 1);

            m_rcds.insert(pend, r);
            pend = m_rcds.begin() + beginindex+2;
            pend->SetBegin(r.GetEnd() + 1);
            if ((r.GetEnd() == UINT_MAX) || (pend->IsEmpty()))
                m_rcds.erase(pend);

            pbegin = m_rcds.begin() + beginindex;
            pbegin->SetEnd(r.GetBegin() - 1);
            if ((r.GetBegin() == 0) || (pbegin->IsEmpty()))
                m_rcds.erase(pbegin);

            return true;
        }
    }


    void ID2Url::init()
    {
        m_rcds.clear(); // 没有使用 DelRcd，是为了直接清除所有的散列对象，直接填充新对象。
        m_rcds.resize(m_rcdNum);
    }

    void ID2Url::AddRcd(unsigned nIndex,unsigned nBegin,unsigned nEnd)
    {
        if(nBegin < 0 || nBegin > 100 || nEnd < 0 || nEnd > 100 || nBegin > nEnd || nIndex < 0 || nIndex >= m_rcdNum)
        {
            return;
        }

        unsigned nStep = UINT_MAX / 100;
        unsigned nRcdBegin = 0,nRcdEnd = 0;

        nRcdBegin =  nBegin*nStep;

        if(nEnd == 100)
        {
            nRcdEnd = UINT_MAX;
        }
        else
        {
            nRcdEnd = nEnd*nStep;
        }  
        //m_rcds.push_back(rcd(nRcdBegin,nRcdEnd));      
        m_rcds[nIndex] = rcd(nRcdBegin,nRcdEnd);
    }


    void ID2Url::SetRcd(int index, const char* szUrl)
    {
        m_rcds[index].Append(szUrl);
    }
}