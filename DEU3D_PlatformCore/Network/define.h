#ifndef __DEFINE_H__
#define    __DEFINE_H__

#include <string>
#include <vector>
#include <list>
#include <limits.h>

#if defined(_MSC_VER)
#pragma warning( disable : 4244 )
#pragma warning( disable : 4251 )
#pragma warning( disable : 4275 )
#pragma warning( disable : 4512 )
#pragma warning( disable : 4267 )
#pragma warning( disable : 4702 )
#pragma warning( disable : 4511 )
#endif


#define    UINT64 unsigned long long
#define    UINT32 unsigned int
#define    BYTE unsigned char

//#define EXPORT_INTERFACE    __declspec(dllexport)
#define EXPORT_INTERFACE

namespace deunw
{
    class rcd
    {
    public:
        rcd(UINT32 beginNum = 0, UINT32 endNum = UINT_MAX, const char *pURL = NULL);
        rcd(const rcd &obj);
        ~rcd();

    public:
        //�õ����rcd��Ӧ��url����
        unsigned GetURLNum() const;
        //ͨ��index�õ�һ��url��index��ȡֵ��Χ��[0, URLNum)
        std::string GetURL(unsigned int index) const;
        //ͨ��indexɾ��һ��url�� indexȡֵ��Χ��[0, URLNum)
        void DelURL(unsigned int index);
        //ͨ��index����һ��url��indexȡֵ��Χ��[0, URLNum)����pURLΪ�յ�ʱ��ʲô��������
        void SetURL(unsigned int index, const char *pURL);
        //����url���б�
        void Append(const char *pURL);

    public:
        UINT32 GetBegin();
        void SetBegin(UINT32 beginNum);
        UINT32 GetEnd();
        void SetEnd(UINT32 endNum);

        //�ж�num�Ƿ������rcd��Χ��
        //���� -1 ��ʾnum�������Χ֮ǰ
        //���� 0 ��ʾ�������Χ��
        //���� 1 ��ʾ�������Χ��
        int NumInRcd(UINT32 num);

        bool IsEmpty();

    public:
        const rcd & operator = (const rcd &obj);
        bool operator > (const rcd &obj);
        bool operator < (const rcd &obj);

    private:
        UINT32 m_beginNum;
        UINT32 m_endNum;
        std::vector<std::string> m_UrlList;
    };

    class ID2Url
    {
    public:
        ID2Url();
        ID2Url(int nRcdNum);
        ~ID2Url();

    public:
        //ͨ��ID�õ�NUM����NUM���Լ������Ǹ�rcd
        bool GetNum(BYTE *pID, int len, UINT32 &num) const;
        //��GetNum�����õ���num������ͨ����������õ���Ӧ��rcd
        rcd * GetRcdByNum(UINT32 num);

    public:
        //�õ�rcd�ĸ���
        UINT32 GetRcdNum() const;
        //ͨ�������ŵõ�rcd��������ȡֵ��Χ��[0, rcdnum)
        rcd * GetRcdByIndex(UINT32 index);
        //ָ��������ɾ��һ��rcd��ID2Url���Զ���ʣ���ID��Χ�������С������Ƽ�ʹ�����������
        bool DelRcd(UINT32 index);
        //����һ��rcd��ID2Url��ID2Url�����ʵ�������ID��Χ������ȷ�����Ƽ���������������Լ��Ĺ���
        bool SetRcd(rcd &r);

        void SetInit(int nRcdNum);
        void AddRcd(unsigned nIndex,unsigned nBegin,unsigned nEnd);
        void SetRcd(int index, const char* szUrl);

    private:
        void init();

    private:
        UINT32  m_rcdNum;
        std::vector<rcd> m_rcds;
    };
}

#endif
