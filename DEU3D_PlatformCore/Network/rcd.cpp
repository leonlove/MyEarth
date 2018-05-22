#include "define.h"
#include "string.h"

namespace deunw
{
    rcd::rcd(UINT32 beginNum, UINT32 endNum, const char *pURL)
        : m_beginNum(beginNum)
        , m_endNum(endNum)
    {
        if (NULL != pURL)
        {
            m_UrlList.push_back(pURL);
        }
    }

    rcd::rcd(const rcd &obj)
        : m_beginNum(0)
        , m_endNum(0)
    {
        *this = obj;
    }

    rcd::~rcd()
    {
    }

    unsigned rcd::GetURLNum() const
    {
        return m_UrlList.size();
    }

    std::string rcd::GetURL(unsigned int index) const
    {
        if (index >= m_UrlList.size())
            return NULL;
        return m_UrlList[index];
    }

    void rcd::DelURL(unsigned int index)
    {
        if (index < 0 || index >= m_UrlList.size())
            return ;
        m_UrlList.erase(m_UrlList.begin() + index);
    }

    void rcd::SetURL(unsigned int index, const char *pURL)
    {
        if (index < 0 || index >= m_UrlList.size() || NULL == pURL)
            return ;
        if (pURL[0] == 0)
            return ;

        m_UrlList[index] = pURL;
    }

    UINT32 rcd::GetBegin()
    {
        return m_beginNum;
    }

    void rcd::SetBegin(UINT32 beginNum)
    {
        m_beginNum = beginNum;
    }

    UINT32 rcd::GetEnd()
    {
        return m_endNum;
    }

    void rcd::SetEnd(UINT32 endNum)
    {
        m_endNum = endNum;
    }

    int rcd::NumInRcd(UINT32 num)
    {
        if (num < m_beginNum)
            return -1;
        else if (num <= m_endNum)
            return 0;
        else
            return 1;
    }

    bool rcd::IsEmpty()
    {
        return m_beginNum > m_endNum;
    }

    const rcd & rcd::operator = (const rcd &obj)
    {
        m_beginNum = obj.m_beginNum;
        m_endNum = obj.m_endNum;
        m_UrlList = obj.m_UrlList;

        return *this;
    }

    bool rcd::operator > (const rcd &obj)
    {
        return m_endNum > obj.m_endNum;
    }

    bool rcd::operator < (const rcd &obj)
    {
        return !(*this > obj);
    }


    void rcd::Append(const char *pURL)
    {
        m_UrlList.push_back(pURL);
    }
}