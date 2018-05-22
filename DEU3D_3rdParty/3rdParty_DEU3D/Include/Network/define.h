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
        //得到这个rcd对应的url个数
        unsigned GetURLNum() const;
        //通过index得到一个url，index的取值范围是[0, URLNum)
        std::string GetURL(unsigned int index) const;
        //通过index删除一个url， index取值范围是[0, URLNum)
        void DelURL(unsigned int index);
        //通过index设置一个url，index取值范围是[0, URLNum)，当pURL为空的时候，什么都不做。
        void SetURL(unsigned int index, const char *pURL);
        //增加url到列表
        void Append(const char *pURL);

    public:
        UINT32 GetBegin();
        void SetBegin(UINT32 beginNum);
        UINT32 GetEnd();
        void SetEnd(UINT32 endNum);

        //判断num是否在这个rcd范围内
        //返回 -1 表示num在这个范围之前
        //返回 0 表示在这个范围内
        //返回 1 表示在这个范围后
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
        //通过ID得到NUM，该NUM可以计算在那个rcd
        bool GetNum(BYTE *pID, int len, UINT32 &num) const;
        //用GetNum方法得到的num，可以通过这个方法得到对应的rcd
        rcd * GetRcdByNum(UINT32 num);

    public:
        //得到rcd的个数
        UINT32 GetRcdNum() const;
        //通过索引号得到rcd，索引号取值范围是[0, rcdnum)
        rcd * GetRcdByIndex(UINT32 index);
        //指定索引号删除一个rcd，ID2Url会自动将剩余的ID范围进行排列。（不推荐使用这个方法）
        bool DelRcd(UINT32 index);
        //设置一个rcd到ID2Url，ID2Url会根据实际情况将ID范围排列正确。（推荐用这个方法设置自己的规则）
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
