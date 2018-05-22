#ifndef __DEU_BSON_H__
#define __DEU_BSON_H__
#include "Export.h"
#include <vector>
#include <string>
#include "membuf.h"
#include <MemPool.h>

#pragma warning(disable: 4251)

namespace bson
{
typedef unsigned char bsonByte;
typedef int bsonInt32;
typedef long long bsonInt64;
typedef std::string bsonCString;

typedef std::string bsonCString;
typedef bsonCString bsonEName;


enum bsonElementType
{
    bsonDoubleType  = 1,
    bsonStringType  = 2,
    bsonDocType     = 3,
    bsonArrayType   = 4,
    bsonBinType     = 5,
    bsonUndefType   = 6,
    bsonOIDType     = 7,
    bsonBoolType    = 8,
    bsonUTCTimeType = 9,
    bsonNULLType    = 10,
    bsonRegExp      = 11,
    bsonDBPointType = 12,
    bsonJSCodeType  = 13,
    bsonSymbolType  = 14,
    bsonJSCodeWType = 15,
    bsonInt32Type   = 16,
    bsonTimestampType = 17,
    bsonInt64Type   = 18,
    bsonMinKey      = 0xff,
    bsonMaxKey      = 0x7f
};


class membuf
{
public:
    explicit membuf(void);
             membuf(const membuf &obj);
    explicit membuf(const void *pbuf, long buflen);
    virtual ~membuf(void);

public:
    bool setdata(const void *pbuf, long buflen);
    bool adddata(const void *pbuf, long buflen);
    bool clear(void);
    long datalen(void) const;
    void * getdata(void);
    const void * getdata(void) const;

public:
    const membuf & operator = (const membuf &obj);

private:
    bool reallocbuf(long len, bool copydata = false);

private:
    char   *m_pbuf;
    long    m_buflen;
    long    m_datalen;
};


class CM_EXPORT bsonStream
{
public:
    explicit bsonStream(void);
    virtual ~bsonStream(void);

public:
    unsigned int Pos(void) const;
    bool SetPos(unsigned int pos);
    bool Reset(void);
    bool Clear(void);

    bool Write(const void *pdata, unsigned int len);
    bool Read(void *pdatabuf, unsigned int len);

    unsigned int DataLen(void) const;
    void *Data(void);
    const void *Data(void) const;

private:
    membuf         *m_buf;
    unsigned int    m_pos;
};

class CM_EXPORT bsonElement
{
public:
    explicit bsonElement(bsonElementType etype = bsonNULLType)
        : m_etype(etype)
    {}
    virtual ~bsonElement(void)
    {}

public:
    bsonElementType GetType(void) const
    {
        return m_etype;
    }
    void SetType(bsonElementType type)
    {
        m_etype = type;
    }
    const char * EName(void) const
    {
        if (m_ename.length() < 1)    return NULL;
        return m_ename.c_str();
    }
    void SetEName(const char * pName)
    {
        if (NULL != pName)  m_ename = pName;
        else                m_ename = "";
    }

    virtual bool Write(bsonStream *pStream) const = 0;
    virtual bool Read(bsonStream *pStream) = 0;

    virtual bool JsonString(std::string &jsonstr) const= 0;
    virtual bool String(std::string &str) const = 0;
    virtual bool ValueString(std::string &str, bool bMarks = true) const = 0;

    virtual double DblValue(void) const         { return 0.0;   }
    virtual bool SetDblValue(double dval)       { return false; }
    virtual const char * StrValue(void) const   { return NULL;  }
    virtual bool SetStrValue(const char *pStr)  { return false; }
    virtual bool BoolValue(void) const          { return false; }
    virtual bool SetBoolValue(bool bval)        { return false; }
    virtual bsonInt32 Int32Value(void) const    { return 0;     }
    virtual bool SetInt32Value(bsonInt32 val)   { return false; }
    virtual bsonInt64 Int64Value(void) const    { return 0;     }
    virtual bool SetInt64Value(bsonInt64 val)   { return false; }
    virtual const void *BinData(void) const     { return NULL;  }
    virtual unsigned int BinDataLen(void) const { return 0;     }
    virtual bool SetBinData(const void *pdata, unsigned int len){ return false; }

public:
    virtual unsigned int CalcSize(void) const { return 0; }

private:
    bsonElementType    m_etype;
    bsonEName m_ename;
};

class CM_EXPORT bsonNullEle : public bsonElement
{
public:
    explicit bsonNullEle(void);
    virtual ~bsonNullEle(void);

public:
    virtual bool Write(bsonStream *pStream) const;
    virtual bool Read(bsonStream *pStream);

    virtual bool JsonString(std::string &jsonstr) const;
    virtual bool String(std::string &str) const;
    virtual bool ValueString(std::string &str, bool bMarks = true) const;

    virtual unsigned int CalcSize(void) const;
};


class CM_EXPORT bsonDoubleEle : public bsonElement
{
public:
    explicit bsonDoubleEle(void);
    virtual ~bsonDoubleEle(void);

public:
    virtual double DblValue(void) const;
    virtual bool SetDblValue(double dval);

public:
    virtual bool Write(bsonStream *pStream) const;
    virtual bool Read(bsonStream *pStream);

    virtual bool JsonString(std::string &jsonstr) const;
    virtual bool String(std::string &str) const;
    virtual bool ValueString(std::string &str,bool bMarks = true) const;

    virtual unsigned int CalcSize(void) const;

private:
    double m_val;
};

class CM_EXPORT bsonStringEle : public bsonElement
{
public:
    explicit bsonStringEle(void);
    virtual ~bsonStringEle(void);

public:
    virtual const char *StrValue(void) const;
    virtual bool SetStrValue(const char *pStr);

public:
    virtual bool Write(bsonStream *pStream) const;
    virtual bool Read(bsonStream *pStream);

    virtual bool JsonString(std::string &jsonstr) const;
    virtual bool String(std::string &str) const;
    virtual bool ValueString(std::string &str, bool bMarks = true) const;

    virtual unsigned int CalcSize(void) const;

private:
    std::string m_str;
};

class CM_EXPORT bsonBoolEle : public bsonElement
{
public:
    explicit bsonBoolEle(void);
    virtual ~bsonBoolEle(void);

public:
    virtual bool BoolValue(void) const;
    virtual bool SetBoolValue(bool bval);

public:
    virtual bool Write(bsonStream *pStream) const;
    virtual bool Read(bsonStream *pStream);

    virtual bool JsonString(std::string &jsonstr) const;
    virtual bool String(std::string &str) const;
    virtual bool ValueString(std::string &str, bool bMarks = true) const;

    virtual unsigned int CalcSize(void) const;

private:
    bool    m_bval;
};

class CM_EXPORT bsonInt32Ele : public bsonElement
{
public:
    explicit bsonInt32Ele(void);
    virtual ~bsonInt32Ele(void);

public:
    virtual bsonInt32 Int32Value(void) const;
    virtual bool SetInt32Value(bsonInt32 val);

public:
    virtual bool Write(bsonStream *pStream) const;
    virtual bool Read(bsonStream *pStream);

    virtual bool JsonString(std::string &jsonstr) const;
    virtual bool String(std::string &str) const;
    virtual bool ValueString(std::string &str, bool bMarks = true) const;

    virtual unsigned int CalcSize(void) const;

private:
    bsonInt32    m_val;
};

class CM_EXPORT bsonInt64Ele : public bsonElement
{
public:
    explicit bsonInt64Ele(void);
    virtual ~bsonInt64Ele(void);

public:
    virtual bsonInt64 Int64Value(void) const;
    virtual bool SetInt64Value(bsonInt64 val);

public:
    virtual bool Write(bsonStream *pStream) const;
    virtual bool Read(bsonStream *pStream);

    virtual bool JsonString(std::string &jsonstr) const;
    virtual bool String(std::string &str) const;
    virtual bool ValueString(std::string &str, bool bMarks = true) const;

    virtual unsigned int CalcSize(void) const;

private:
    bsonInt64    m_val;
};


class CM_EXPORT bsonBinaryEle : public bsonElement
{
public:
    explicit bsonBinaryEle(void);
    virtual ~bsonBinaryEle(void);

public:
    virtual const void *BinData(void) const;
    virtual unsigned int BinDataLen(void) const;
    virtual bool SetBinData(const void *pdata, unsigned int len);

public:
    virtual bool Write(bsonStream *pStream) const;
    virtual bool Read(bsonStream *pStream);

    virtual bool JsonString(std::string &jsonstr) const;
    virtual bool String(std::string &str) const;
    virtual bool ValueString(std::string &str, bool bMarks = true) const;

    virtual unsigned int CalcSize(void) const;

private:
    bsonByte    m_subtype;
    membuf     *m_buf;
};

class CM_EXPORT bsonDocument
{
public:
    explicit bsonDocument(void);
    virtual ~bsonDocument(void);

public:
    bool AddBoolElement(const char *pName, bool val);
    bool AddInt32Element(const char *pName, bsonInt32 val);
    bool AddInt64Element(const char *pName, bsonInt64 val);
    bool AddDblElement(const char *pName, double val);
    bool AddBinElement(const char *pName, void *pbuf, unsigned int len);
    bool AddStringElement(const char *pName, const char *val);
    bsonElement * AddArrayElement(const char *pName);
    bsonElement * AddDocumentElement(const char *pName);
    bool AddNullElement(const char *pName);

public:
    bool InsBoolElement(unsigned int Index, const char *pName, bool val);
    bool InsInt32Element(unsigned int Index, const char *pName, bsonInt32 val);
    bool InsInt64Element(unsigned int Index, const char *pName, bsonInt64 val);
    bool InsDblElement(unsigned int Index, const char *pName, double val);
    bool InsBinElement(unsigned int Index, const char *pName, void *pbuf, unsigned int len);
    bool InsStringElement(unsigned int Index, const char *pName, const char *val);
    bsonElement * InsArrayElement(unsigned int Index, const char *pName);
    bsonElement * InsDocumentElement(unsigned int Index, const char *pName);
    bool InsNullElement(unsigned int Index, const char *pName);

public:
    unsigned int ChildCount(void) const;
    bsonElement * GetElement(unsigned int Index);
    bsonElement * GetElement(const char *pName);
    const bsonElement * GetElement(unsigned int Index) const;
    const bsonElement * GetElement(const char *pName) const;
    bool DelElement(unsigned int Index);

public:
    bool Write(bsonStream *pStream) const;
    bool Read(bsonStream *pStream);

public:
    bool JsonString(std::string &jsonstr) const;
    bool FromJsonString(const std::string &jsonstr);
    bool FromBsonStream(const void *pBuffer, unsigned nBufLen);

    unsigned int CalcSize(void) const;

private:
    void clear(void);

private:
    std::vector<bsonElement*> m_elist;
};

class CM_EXPORT bsonArrayEle : public bsonElement
{
public:
    explicit bsonArrayEle(void);
    virtual ~bsonArrayEle(void);

public:
    bool AddBoolElement(bool val);
    bool AddInt32Element(bsonInt32 val);
    bool AddInt64Element(bsonInt64 val);
    bool AddDblElement(double val);
    bool AddBinElement(void *pbuf, unsigned int len);
    bool AddStringElement(const char *val);
    bsonElement * AddArrayElement(void);
    bsonElement * AddDocumentElement(void);
    bool AddNullElement(void);

public:
    bool InsBoolElement(unsigned int Index, bool val);
    bool InsInt32Element(unsigned int Index, bsonInt32 val);
    bool InsInt64Element(unsigned int Index, bsonInt64 val);
    bool InsDblElement(unsigned int Index, double val);
    bool InsBinElement(unsigned int Index, void *pbuf, unsigned int len);
    bool InsStringElement(unsigned int Index, const char *val);
    bsonElement * InsArrayElement(unsigned int Index);
    bsonElement * InsDocumentElement(unsigned int Index);
    bool IncNullElement(unsigned int Index);

public:
    unsigned int ChildCount(void) const;
    bsonElement * GetElement(unsigned int Index);
    bsonElement * GetElement(const char *pName);
    const bsonElement * GetElement(unsigned int Index) const;
    const bsonElement * GetElement(const char *pName) const;
    bool DelElement(unsigned int Index);

public:
    virtual bool Write(bsonStream *pStream) const;
    virtual bool Read(bsonStream *pStream);

    virtual bool JsonString(std::string &jsonstr) const;
    virtual bool String(std::string &str) const;
    virtual bool ValueString(std::string &str, bool bMarks = true) const;

    virtual unsigned int CalcSize(void) const;

private:
    bsonDocument *m_doc;
};

class CM_EXPORT bsonDocumentEle : public bsonElement
{
public:
    explicit bsonDocumentEle(void);
    virtual ~bsonDocumentEle(void);

public:
    bsonDocument &GetDoc(void);
    const bsonDocument &GetDoc(void) const;

public:
    virtual bool Write(bsonStream *pStream) const;
    virtual bool Read(bsonStream *pStream);

    virtual bool JsonString(std::string &jsonstr) const;
    virtual bool String(std::string &str) const;
    virtual bool ValueString(std::string &str, bool bMarks = true) const;

    virtual unsigned int CalcSize(void) const;

private:
    bsonDocument *m_doc;
};

}


#endif    //__DEU_BSON_H__
