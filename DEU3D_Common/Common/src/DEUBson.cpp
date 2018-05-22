#include <DEUBson.h>
#include <sstream>
#include "cJSON.h"
#ifndef _WINDOWS
#include <math.h>
#include <stdio.h>
#include <string.h>
#endif
#include <iostream>

#pragma warning( disable : 4996 )

namespace bson
{


#define BSON_DBL_EPSILON     2.2204460492503131e-016 /* smallest such that 1.0+DBL_EPSILON != 1.0 */
#define BSON_INT_MIN     (-2147483647 - 1) /* minimum (signed) int value */
#define BSON_INT_MAX       2147483647    /* maximum (signed) int value */


static char * print_number(double dval)
{
    char *str = NULL;
    long nval = (long)floor(dval);
    if (fabs((nval)-dval)<=BSON_DBL_EPSILON && dval<=BSON_INT_MAX && dval>=BSON_INT_MIN)
    {
        str=new char[21];    /* 2^64+1 can be represented in 21 chars. */
        if (str) sprintf(str,"%ld",nval);
    }
    else
    {
        str=new char[64];    /* This is a nice tradeoff. */
        if (str)
        {
            if (fabs(nval-dval)<=BSON_DBL_EPSILON)            sprintf(str,"%.0lf",dval);
            else if (fabs(dval)<1.0e-6 || fabs(dval)>1.0e9)    sprintf(str,"%le",dval);
            else                                        sprintf(str,"%10.10lf",dval);
        }
    }
    return str;
}

static const char *skip(const char *in) {while (in && *in && (unsigned char)*in<=32) in++; return in;}


//////////////////////////////////////////////////////////////////////////
membuf::membuf(void)
    : m_pbuf(NULL)
    , m_buflen(0)
    , m_datalen(0)
{
#if 0
    static bool b = true;
    if(b)
    {
        std::cout << "membuf = " << sizeof(membuf) << std::endl;
        std::cout << "bsonStream = " << sizeof(bsonStream) << std::endl;
        std::cout << "bsonElement = " << sizeof(bsonElement) << std::endl;
        std::cout << "bsonNullEle = " << sizeof(bsonNullEle) << std::endl;
        std::cout << "bsonDoubleEle = " << sizeof(bsonDoubleEle) << std::endl;
        std::cout << "bsonStringEle = " << sizeof(bsonStringEle) << std::endl;
        std::cout << "bsonBoolEle = " << sizeof(bsonBoolEle) << std::endl;
        std::cout << "bsonInt32Ele = " << sizeof(bsonInt32Ele) << std::endl;
        std::cout << "bsonInt64Ele = " << sizeof(bsonInt64Ele) << std::endl;
        std::cout << "bsonBinaryEle = " << sizeof(bsonBinaryEle) << std::endl;
        std::cout << "bsonDocument = " << sizeof(bsonDocument) << std::endl;
        std::cout << "bsonArrayEle = " << sizeof(bsonArrayEle) << std::endl;
        std::cout << "bsonDocumentEle = " << sizeof(bsonDocumentEle) << std::endl;
        b = false;
    }
#endif
}

membuf::membuf(const membuf &obj)
    : m_pbuf(NULL)
    , m_buflen(0)
    , m_datalen(0)
{
    setdata(obj.m_pbuf, obj.m_datalen);
}

membuf::membuf(const void *pbuf, long buflen)
    : m_pbuf(NULL)
    , m_buflen(0)
    , m_datalen(0)
{
    setdata(pbuf, buflen);
}

membuf::~membuf(void)
{
    if (NULL != m_pbuf)
    {
        free(m_pbuf);
    }
    m_pbuf = NULL;
    m_buflen = 0;
    m_datalen = 0;
}


bool membuf::setdata(const void *pbuf, long buflen)
{
    if (NULL == pbuf || buflen < 1)
        return false;
    if (buflen > m_buflen)
    {
        if (!reallocbuf(buflen*2))    return false;
        m_buflen = buflen*2;
    }
    memcpy(m_pbuf, pbuf, buflen);
    m_datalen = buflen;
    return true;
}

bool membuf::adddata(const void *pbuf, long buflen)
{
    if (NULL == pbuf || buflen < 1)
        return false;
    if (buflen+m_datalen > m_buflen)
    {
        if (!reallocbuf((m_datalen + buflen)*2, true))    return false;
        m_buflen = (m_datalen + buflen)*2;
    }
    memcpy((char *)m_pbuf+m_datalen, pbuf, buflen);
    m_datalen += buflen;
    return true;
}

bool membuf::clear(void)
{
    m_datalen = 0;
    return true;
}

long membuf::datalen(void) const
{
    return m_datalen;
}

void * membuf::getdata(void)
{
    return (m_datalen > 0 && NULL != m_pbuf) ? m_pbuf : NULL;
}

const void * membuf::getdata(void) const
{
    return (m_datalen > 0 && NULL != m_pbuf) ? m_pbuf : NULL;
}


const membuf & membuf::operator = (const membuf &obj)
{
    if(this == &obj)    return *this;
    setdata(obj.m_pbuf, obj.m_datalen);
    return *this;
}

bool membuf::reallocbuf(long len, bool copydata)
{
    void *ptmp = malloc(len);
    if (NULL == ptmp)    return false;
    
    if (NULL != m_pbuf)
    {
        if (copydata && m_datalen > 0)
        {
            if (len < m_datalen) m_datalen = len;
            memcpy(ptmp, m_pbuf, m_datalen);
        }
        else
            m_datalen = 0;

        free(m_pbuf);
        m_pbuf = NULL;
    }
    m_pbuf = (char *)ptmp;
    m_buflen = len;
    return true;
}


/////////////////////////////////////////////////////////////////////////
// bsonStream class
bsonStream::bsonStream(void)
    : m_pos(0)
{
    m_buf = new membuf;
}

bsonStream::~bsonStream(void)
{
    delete m_buf;
}


unsigned int bsonStream::Pos(void) const
{
    return m_pos;
}

bool bsonStream::SetPos(unsigned int pos)
{
    if (pos >= (unsigned)m_buf->datalen())    return false;
    m_pos = pos;
    return true;
}

bool bsonStream::Reset()
{
    m_pos = 0;
    return true;
}

bool bsonStream::Clear()
{
    m_buf->clear();
    m_pos = 0;
    return true;
}

bool bsonStream::Write(const void *pdata, unsigned int len)
{
    unsigned int dlen = len + m_pos;
    if (dlen > (unsigned)m_buf->datalen())
    {
        if (!m_buf->adddata(pdata, dlen - m_buf->datalen()))    return false;
    }
    unsigned char *pbuf = (unsigned char *)m_buf->getdata();
    memcpy(pbuf + m_pos, pdata, len);
    m_pos += len;
    return true;
}

bool bsonStream::Read(void *pdatabuf, unsigned int len)
{
    if (pdatabuf == NULL)    return false;
    if (m_buf->datalen() - m_pos < len)    return false;
    unsigned char *pbuf = (unsigned char *)m_buf->getdata();
    memcpy(pdatabuf, pbuf + m_pos, len);
    m_pos += len;
    return true;
}

unsigned int bsonStream::DataLen(void) const
{
    return m_buf->datalen();
}

void * bsonStream::Data(void)
{
    return m_buf->getdata();
}

bool WriteElementHeader(const bsonElement *pEle, bsonStream *pStream)
{
    if (NULL == pEle || NULL == pStream)    return false;
    bsonByte tmp = pEle->GetType();
    pStream->Write(&tmp, sizeof(tmp));
    const char *pname = pEle->EName();
    pStream->Write(pname, strlen(pname));
    tmp = 0;
    pStream->Write(&tmp, 1);
    return true;
}

bool ReadString(std::string &str, bsonStream *pStream)
{
    if (NULL == pStream)    return false;
    unsigned int pos = pStream->Pos();
    char *pdata = (char *)pStream->Data();
    pdata += pos;
    str = pdata;
    pos += str.length() + 1;
    pStream->SetPos(pos);
    return true;
}

bool ReadElementHeader(bsonElement *pEle, bsonStream *pStream)
{
    if (NULL == pEle || NULL == pStream)    return false;
    bsonByte tmp = bsonUndefType;
    if (!pStream->Read(&tmp, 1))    return false;
    pEle->SetType((bsonElementType)tmp);

    std::string str;
    if (!ReadString(str, pStream))    return false;
    pEle->SetEName(str.c_str());
    return true;
}


/////////////////////////////////////////////////////////////////////////
// bsonNullEle class
bsonNullEle::bsonNullEle()
    : bsonElement(bsonNULLType)
{
}

bsonNullEle::~bsonNullEle()
{
}


bool bsonNullEle::Write(bsonStream *pStream) const
{
    if (NULL == pStream)    return false;
    WriteElementHeader(this, pStream);
    return true;
}

bool bsonNullEle::Read(bsonStream *pStream)
{
    if (NULL == pStream)    return false;
    ReadElementHeader(this, pStream);
    return true;
}

bool bsonNullEle::JsonString(std::string &jsonstr) const
{
    jsonstr = "{}";
    return true;
}

bool bsonNullEle::String(std::string &str) const
{
    str.clear();
    return true;
}

bool bsonNullEle::ValueString(std::string &str, bool bMarks) const
{
    str.clear();
    return true;
}

unsigned int bsonNullEle::CalcSize(void) const
{
    const char *pszName = EName();
    if(pszName)
    {
        return 1 + strlen(pszName) + 1;
    }
    return 1 + 0 + 1;
}


/////////////////////////////////////////////////////////////////////////
// bsonDoubleEle class
bsonDoubleEle::bsonDoubleEle(void)
    : bsonElement(bsonDoubleType)
    , m_val(0.0)
{
}

bsonDoubleEle::~bsonDoubleEle(void)
{
}


double bsonDoubleEle::DblValue(void) const
{
    return m_val;
}

bool bsonDoubleEle::SetDblValue(double dval)
{
    m_val = dval;
    return true;
}

bool bsonDoubleEle::Write(bsonStream *pStream) const
{
    if (NULL == pStream)    return false;
    WriteElementHeader(this, pStream);
    pStream->Write(&m_val, sizeof(m_val));
    return true;
}

bool bsonDoubleEle::Read(bsonStream *pStream)
{
    if (NULL == pStream)    return false;
    ReadElementHeader(this, pStream);
    pStream->Read(&m_val, sizeof(m_val));
    return true;
}

bool bsonDoubleEle::JsonString(std::string &jsonstr) const
{
    if (GetType() != bsonDoubleType)    return false;

    char *ptmpstr = print_number(m_val);
    if (NULL == ptmpstr)    return false;
    jsonstr.clear();
    jsonstr += "{\"";
    jsonstr += EName();
    jsonstr += "\":";
    jsonstr += ptmpstr;
    delete []ptmpstr;
    jsonstr += "}";
    return true;
}


bool bsonDoubleEle::String(std::string &str) const
{
    if (GetType() != bsonDoubleType)    return false;

    char *ptmpstr = print_number(m_val);
    if (NULL == ptmpstr)    return false;
    str.clear();
    str += "\"";
    str += EName();
    str += "\":";
    str += ptmpstr;
    delete []ptmpstr;
    return true;
}

bool bsonDoubleEle::ValueString(std::string &str, bool bMarks) const
{
    if (GetType() != bsonDoubleType)    return false;

    char *ptmpstr = print_number(m_val);
    if (NULL == ptmpstr)    return false;
    str.clear();
    str += ptmpstr;
    delete []ptmpstr;
    return true;
}

unsigned int bsonDoubleEle::CalcSize(void) const
{
    //type + ename + double
    const char *pszName = EName();
    if(pszName)
    {
        return 1 + strlen(pszName) + 1 + sizeof(m_val);
    }
    return 1 + 0 + 1 + sizeof(m_val);
}



/////////////////////////////////////////////////////////////////////////
// bsonStream class
bsonStringEle::bsonStringEle()
    : bsonElement(bsonStringType)
{}
bsonStringEle::~bsonStringEle()
{}


const char * bsonStringEle::StrValue(void) const
{
    if (m_str.length() < 1)    return "";
    return m_str.c_str();
}

bool bsonStringEle::SetStrValue(const char *pStr)
{
    m_str = "";
    if (pStr != NULL)
    {
        m_str = pStr;
    }
    return true;
}

bool bsonStringEle::Write(bsonStream *pStream) const
{
    if (NULL == pStream)    return false;
    WriteElementHeader(this, pStream);
    bsonInt32 len = m_str.length() + 1;
    pStream->Write(&len, sizeof(len));
    pStream->Write(m_str.c_str(), len);
    return true;
}

bool bsonStringEle::Read(bsonStream *pStream)
{
    if (NULL == pStream)    return false;
    ReadElementHeader(this, pStream);
    bsonInt32 len = 0;
    pStream->Read(&len, sizeof(len));
    if (!ReadString(m_str, pStream))    return false;
    if (m_str.length() != len-1)    return false;
    return true;
}

bool bsonStringEle::JsonString(std::string &jsonstr) const
{
    if (GetType() != bsonStringType)    return false;

    jsonstr.clear();
    jsonstr += "{\"";
    jsonstr += EName();
    jsonstr += "\":\"";
    jsonstr += m_str;
    jsonstr += "\"}";
    return true;
}

bool bsonStringEle::String(std::string &str) const
{
    if (GetType() != bsonStringType)    return false;

    str.clear();
    str += "\"";
    str += EName();
    str += "\":\"";
    str += m_str;
    str += "\"";
    return true;
}

bool bsonStringEle::ValueString(std::string &str, bool bMarks) const
{
    if (GetType() != bsonStringType)    return false;

    str.clear();
    if(bMarks)
    {
        str += "\"";
        str += m_str;
        str += "\"";
    }
    else
    {
        str += m_str;
    }
    
    return true;
}

unsigned int bsonStringEle::CalcSize(void) const
{
    //type + ename + int32 + strlen
    const char *pszName = EName();
    if(pszName)
    {
        return 1 + strlen(pszName) + 1 + 4 + m_str.length() + 1;
    }
    return 1 + 0 + 1 + 4 + m_str.length() + 1;
}

/////////////////////////////////////////////////////////////////////////
// bsonBoolEle class
bsonBoolEle::bsonBoolEle(void)
    : bsonElement(bsonBoolType)
    , m_bval(false)
{
}

bsonBoolEle::~bsonBoolEle(void)
{
}


bool bsonBoolEle::BoolValue(void) const
{
    return m_bval;
}

bool bsonBoolEle::SetBoolValue(bool bval)
{
    m_bval = bval;
    return true;
}

bool bsonBoolEle::Write(bsonStream *pStream) const
{
    if (NULL == pStream)    return false;
    WriteElementHeader(this, pStream);
    bsonByte tmp = m_bval ? 1 : 0;
    pStream->Write(&tmp, sizeof(tmp));
    return true;
}

bool bsonBoolEle::Read(bsonStream *pStream)
{
    if (NULL == pStream)    return false;
    ReadElementHeader(this, pStream);
    bsonByte tmp = 0;
    pStream->Read(&tmp, sizeof(tmp));
    m_bval = (tmp != 0);
    return true;
}

bool bsonBoolEle::JsonString(std::string &jsonstr) const
{
    if (GetType() != bsonBoolType)    return false;

    jsonstr.clear();
    jsonstr += "{\"";
    jsonstr += EName();
    jsonstr += "\":";
    if (m_bval)    jsonstr += "true";
    else        jsonstr += "false";
    jsonstr += "}";
    return true;
}


bool bsonBoolEle::String(std::string &str) const
{
    if (GetType() != bsonBoolType)    return false;

    str.clear();
    str += "\"";
    str += EName();
    str += "\":";
    if (m_bval)    str += "true";
    else        str += "false";
    return true;
}

bool bsonBoolEle::ValueString(std::string &str, bool bMarks) const
{
    if (GetType() != bsonBoolType)    return false;

    str.clear();
    if (m_bval)    str += "true";
    else        str += "false";
    return true;
}


unsigned int bsonBoolEle::CalcSize(void) const
{
    //type + ename + bool
    const char *pszName = EName();
    if(pszName)
    {
        return 1 + strlen(pszName) + 1 + 1;
    }
    return 1 + 0 + 1 + 1;
}



/////////////////////////////////////////////////////////////////////////
// bsonInt32Ele class
bsonInt32Ele::bsonInt32Ele()
    : bsonElement(bsonInt32Type)
    , m_val(0)
{
}

bsonInt32Ele::~bsonInt32Ele()
{
}


bsonInt32 bsonInt32Ele::Int32Value(void) const
{
    return m_val;
}

bool bsonInt32Ele::SetInt32Value(bsonInt32 val)
{
    m_val = val;
    return true;
}

bool bsonInt32Ele::Write(bsonStream *pStream) const
{
    if (NULL == pStream)    return false;
    WriteElementHeader(this, pStream);
    pStream->Write(&m_val, sizeof(m_val));
    return true;
}

bool bsonInt32Ele::Read(bsonStream *pStream)
{
    if (NULL == pStream)    return false;
    ReadElementHeader(this, pStream);
    pStream->Read(&m_val, sizeof(m_val));
    return true;
}

bool bsonInt32Ele::JsonString(std::string &jsonstr) const
{
    if (GetType() != bsonInt32Type)    return false;

    char tmpbuf[64] = "";
    sprintf(tmpbuf, "%d", m_val);
    jsonstr.clear();
    jsonstr += "{\"";
    jsonstr += EName();
    jsonstr += "\":";
    jsonstr += tmpbuf;
    jsonstr += "}";
    return true;
}


bool bsonInt32Ele::String(std::string &str) const
{
    if (GetType() != bsonInt32Type)    return false;

    char tmpbuf[64] = "";
    sprintf(tmpbuf, "%d", m_val);
    str.clear();
    str += "\"";
    str += EName();
    str += "\":";
    str += tmpbuf;
    return true;
}

bool bsonInt32Ele::ValueString(std::string &str, bool bMarks) const
{
    if (GetType() != bsonInt32Type)    return false;

    char tmpbuf[64] = "";
    sprintf(tmpbuf, "%d", m_val);
    str.clear();
    str += tmpbuf;
    return true;
}

unsigned int bsonInt32Ele::CalcSize(void) const
{
    //type + ename + int32
    const char *pszName = EName();
    if(pszName)
    {
        return 1 + strlen(pszName) + 1 + sizeof(bsonInt32);
    }
    return 1 + 0 + 1 + sizeof(bsonInt32);
}


/////////////////////////////////////////////////////////////////////////
// bsonInt64Ele class
bsonInt64Ele::bsonInt64Ele()
    : bsonElement(bsonInt64Type)
    , m_val(0)
{
}

bsonInt64Ele::~bsonInt64Ele()
{
}


bsonInt64 bsonInt64Ele::Int64Value(void) const
{
    return m_val;
}

bool bsonInt64Ele::SetInt64Value(bsonInt64 val)
{
    m_val = val;
    return true;
}

bool bsonInt64Ele::Write(bsonStream *pStream) const
{
    if (NULL == pStream)    return false;
    WriteElementHeader(this, pStream);
    pStream->Write(&m_val, sizeof(m_val));
    return true;
}

bool bsonInt64Ele::Read(bsonStream *pStream)
{
    if (NULL == pStream)    return false;
    ReadElementHeader(this, pStream);
    pStream->Read(&m_val, sizeof(m_val));
    return true;
}


bool bsonInt64Ele::JsonString(std::string &jsonstr) const
{
    if (GetType() != bsonInt64Type)    return false;

    char tmpbuf[64] = "";
    sprintf(tmpbuf, "%lld", m_val);
    jsonstr.clear();
    jsonstr += "{\"";
    jsonstr += EName();
    jsonstr += "\":";
    jsonstr += tmpbuf;
    jsonstr += "}";
    return true;
}

bool bsonInt64Ele::String(std::string &str) const
{
    if (GetType() != bsonInt64Type)    return false;

    char tmpbuf[64] = "";
    sprintf(tmpbuf, "%lld", m_val);
    str.clear();
    str += "\"";
    str += EName();
    str += "\":";
    str += tmpbuf;
    return true;
}

bool bsonInt64Ele::ValueString(std::string &str, bool bMarks) const
{
    if (GetType() != bsonInt64Type)    return false;

    char tmpbuf[64] = "";
    sprintf(tmpbuf, "%lld", m_val);
    str.clear();
    str += tmpbuf;
    return true;
}

unsigned int bsonInt64Ele::CalcSize(void) const
{
    //type + ename + int64
    const char *pszName = EName();
    if(pszName)
    {
        return 1 + strlen(pszName) + 1 + sizeof(bsonInt64);
    }
    return 1 + 0 + 1 + sizeof(bsonInt64);
}




/////////////////////////////////////////////////////////////////////////
// bsonBinaryEle class
bsonBinaryEle::bsonBinaryEle()
    : bsonElement(bsonBinType)
    , m_subtype(0)
{
    m_buf = new membuf;
}

bsonBinaryEle::~bsonBinaryEle()
{
    delete m_buf;
}


const void * bsonBinaryEle::BinData(void) const
{
    return m_buf->getdata();
}

unsigned int bsonBinaryEle::BinDataLen(void) const
{
    return m_buf->datalen();
}

bool bsonBinaryEle::SetBinData(const void *pdata, unsigned int len)
{
    if (!m_buf->clear())    return false;
    return m_buf->adddata(pdata, len);
}

bool bsonBinaryEle::Write(bsonStream *pStream) const
{
    if (NULL == pStream)    return false;
    WriteElementHeader(this, pStream);
    unsigned int len = m_buf->datalen();
//    bsonByte subtype = 0;
    if (!pStream->Write(&len, sizeof(len)))    return false;
    if (!pStream->Write(&m_subtype, sizeof(m_subtype)))    return false;
    return pStream->Write(m_buf->getdata(), m_buf->datalen());
}

bool bsonBinaryEle::Read(bsonStream *pStream)
{
    if (NULL == pStream)    return false;
    ReadElementHeader(this, pStream);
    unsigned int len = 0;
    m_subtype = 0;
    if (!pStream->Read(&len, sizeof(len)))    return false;
    if (!pStream->Read(&m_subtype, sizeof(m_subtype)))    return false;
    char *pbuf = (char *)pStream->Data();
    unsigned int pos = pStream->Pos();
    pbuf += pos;
    m_buf->clear();
    bool bRet = m_buf->adddata(pbuf, len);
    pStream->SetPos(pos + len);
    return bRet;
}

static void StrToHex(unsigned char* S,int nl, std::string &out)
{
    out.clear();
    int i = 0,t=0;
    char* Result = new char[nl*2 + 1];
    memset(Result,'\0',nl*2);
    for(i = 0;i < nl;i++)
    {
        t = S[i] & 0xF;
        if (t < 10) 
            Result[i*2+1] = '0' + t;
        else
            Result[i*2+1] = 'A' + (t - 10);
        t = (S[i] >> 4) & 0xF;
        if (t < 10 )
            Result[i*2] = '0' + t;
        else
            Result[i*2] = 'A' + (t - 10);
    }
    Result[nl*2] = 0;
    out = Result;
    delete[] Result;
}

static void HexToBuf(const std::string &S, membuf &buf)
{
    int i = 0;
    unsigned int len = S.length();
    unsigned char t;
    int length = (int)(len/2);
    char* chRes = new char[length];
    memset(chRes,'\0',length);

    for(i = 0;i < length*2;i++)
    {
        if ((S[i] >= 'a') && (S[i] <= 'z') )
            t = S[i] - 'a' + 10;
        else if ((S[i] >= 'A') && (S[i] <= 'Z'))
            t = S[i] -'A' + 10;
        else if ((S[i] >= '0') && (S[i] <= '9'))
            t = S[i] - '0';
        else
            t = 0;
        if ((i+1) % 2 != 0)
            t = t << 4;
        chRes[int(i/2)] = chRes[int(i / 2)] | t;
    }
    buf.setdata(chRes, length);
    delete[] chRes;
    chRes = NULL;
}

bool bsonBinaryEle::JsonString(std::string &jsonstr) const
{
    if (GetType() != bsonBinType)    return false;

    jsonstr.clear();
    jsonstr += "{\"";
    jsonstr += EName();
    jsonstr += "\":\"BINARYDATA";
    std::string hexstr;
    StrToHex((unsigned char *)m_buf->getdata(), m_buf->datalen(), hexstr);
    jsonstr += hexstr;
    jsonstr += "\"}";
    return true;
}

bool bsonBinaryEle::String(std::string &str) const
{
    if (GetType() != bsonBinType)    return false;

    str.clear();
    str += "\"";
    str += EName();
    str += "\":\"BINARYDATA";
    std::string hexstr;
    StrToHex((unsigned char *)m_buf->getdata(), m_buf->datalen(), hexstr);
    str += hexstr;
    str += "\"";
    return true;
}

bool bsonBinaryEle::ValueString(std::string &str, bool bMarks) const
{
    if (GetType() != bsonBinType)    return false;

    str.clear();
    str += "\"BINARYDATA";
    std::string hexstr;
    StrToHex((unsigned char *)m_buf->getdata(), m_buf->datalen(), hexstr);
    str += hexstr;
    str += "\"";
    return true;
}

unsigned int bsonBinaryEle::CalcSize(void) const
{
    //type + ename + int32 + sybtype + datalen
    const char *pszName = EName();
    if(pszName)
    {
        return 1 + strlen(pszName) + 1 + sizeof(bsonInt32) + 1 + m_buf->datalen();
    }
    return 1 + 0 + 1 + sizeof(bsonInt32) + 1 + m_buf->datalen();
}


/////////////////////////////////////////////////////////////////////////
// bsonDocument class
bsonDocument::bsonDocument()
{
}

bsonDocument::~bsonDocument()
{
    clear();
}


bool bsonDocument::AddBoolElement(const char *pName, bool val)
{
    if (NULL == pName)    return false;
    bsonBoolEle *pEle = new bsonBoolEle();
    if (NULL == pEle)    return false;
    pEle->SetEName(pName);
    if (!pEle->SetBoolValue(val))    return false;
    m_elist.push_back(pEle);
    return true;
}

bool bsonDocument::AddInt32Element(const char *pName, bsonInt32 val)
{
    if (NULL == pName)    return false;
    bsonInt32Ele *pEle = new bsonInt32Ele();
    if (NULL == pEle)    return false;
    pEle->SetEName(pName);
    if (!pEle->SetInt32Value(val))    return false;
    m_elist.push_back(pEle);
    return true;
}

bool bsonDocument::AddInt64Element(const char *pName, bsonInt64 val)
{
    if (NULL == pName)    return false;
    bsonInt64Ele *pEle = new bsonInt64Ele();
    if (NULL == pEle)    return false;
    pEle->SetEName(pName);
    if (!pEle->SetInt64Value(val))    return false;
    m_elist.push_back(pEle);
    return true;
}

bool bsonDocument::AddDblElement(const char *pName, double val)
{
    if (NULL == pName)    return false;
    bsonDoubleEle *pEle = new bsonDoubleEle();
    if (NULL == pEle)    return false;
    pEle->SetEName(pName);
    if (!pEle->SetDblValue(val))    return false;
    m_elist.push_back(pEle);
    return true;
}

bool bsonDocument::AddBinElement(const char *pName, void *pbuf, unsigned int len)
{
    if (NULL == pName)    return false;
    bsonBinaryEle *pEle = new bsonBinaryEle();
    if (NULL == pEle)    return false;
    pEle->SetEName(pName);
    if (!pEle->SetBinData(pbuf, len))    return false;
    m_elist.push_back(pEle);
    return true;
}

bool bsonDocument::AddStringElement(const char *pName, const char *val)
{
    if (NULL == pName)    return false;
    bsonStringEle *pEle = new bsonStringEle();
    if (NULL == pEle)    return false;
    pEle->SetEName(pName);
    if (!pEle->SetStrValue(val))    return false;
    m_elist.push_back(pEle);
    return true;
}

bsonElement * bsonDocument::AddArrayElement(const char *pName)
{
    if (NULL == pName)    return NULL;
    bsonArrayEle *pEle = new bsonArrayEle();
    if (NULL == pEle)    return false;
    pEle->SetEName(pName);
    m_elist.push_back(pEle);
    return pEle;
}

bsonElement * bsonDocument::AddDocumentElement(const char *pName)
{
    if (NULL == pName)    return NULL;
    bsonDocumentEle *pEle = new bsonDocumentEle();
    if (NULL == pEle)    return NULL;
    pEle->SetEName(pName);
    m_elist.push_back(pEle);
    return pEle;
}

bool bsonDocument::AddNullElement(const char *pName)
{
    if (NULL == pName)    return false;
    bsonNullEle *pEle = new bsonNullEle();
    if (NULL == pEle)    return false;
    pEle->SetEName(pName);
    m_elist.push_back(pEle);
    return true;
}

bool bsonDocument::InsBoolElement(unsigned int Index, const char *pName, bool val)
{
    if (NULL == pName || Index >= m_elist.size())    return false;
    bsonBoolEle *pEle = new bsonBoolEle();
    if (NULL == pEle)    return false;
    pEle->SetEName(pName);
    if (!pEle->SetBoolValue(val))    return false;
    m_elist.insert(m_elist.begin() + Index, pEle);
    return true;
}

bool bsonDocument::InsInt32Element(unsigned int Index, const char *pName, bsonInt32 val)
{
    if (NULL == pName || Index >= m_elist.size())    return false;
    bsonInt32Ele *pEle = new bsonInt32Ele();
    if (NULL == pEle)    return false;
    pEle->SetEName(pName);
    if (!pEle->SetInt32Value(val))    return false;
    m_elist.insert(m_elist.begin() + Index, pEle);
    return true;
}

bool bsonDocument::InsInt64Element(unsigned int Index, const char *pName, bsonInt64 val)
{
    if (NULL == pName || Index >= m_elist.size())    return false;
    bsonInt64Ele *pEle = new bsonInt64Ele();
    if (NULL == pEle)    return false;
    pEle->SetEName(pName);
    if (!pEle->SetInt64Value(val))    return false;
    m_elist.insert(m_elist.begin() + Index, pEle);
    return true;
}

bool bsonDocument::InsDblElement(unsigned int Index, const char *pName, double val)
{
    if (NULL == pName || Index >= m_elist.size())    return false;
    bsonDoubleEle *pEle = new bsonDoubleEle();
    if (NULL == pEle)    return false;
    pEle->SetEName(pName);
    if (!pEle->SetDblValue(val))    return false;
    m_elist.insert(m_elist.begin() + Index, pEle);
    return true;
}

bool bsonDocument::InsBinElement(unsigned int Index, const char *pName, void *pbuf, unsigned int len)
{
    if (NULL == pName || Index >= m_elist.size())    return false;
    bsonBinaryEle *pEle = new bsonBinaryEle();
    if (NULL == pEle)    return false;
    pEle->SetEName(pName);
    if (!pEle->SetBinData(pbuf, len))    return false;
    m_elist.insert(m_elist.begin() + Index, pEle);
    return true;
}

bool bsonDocument::InsStringElement(unsigned int Index, const char *pName, const char *val)
{
    if (NULL == pName || Index >= m_elist.size())    return false;
    bsonStringEle *pEle = new bsonStringEle();
    if (NULL == pEle)    return false;
    pEle->SetEName(pName);
    if (!pEle->SetStrValue(val))    return false;
    m_elist.insert(m_elist.begin() + Index, pEle);
    return true;
}

bsonElement * bsonDocument::InsArrayElement(unsigned int Index, const char *pName)
{
    if (NULL == pName || Index >= m_elist.size())    return false;
    bsonArrayEle *pEle = new bsonArrayEle();
    if (NULL == pEle)    return false;
    pEle->SetEName(pName);
    m_elist.insert(m_elist.begin() + Index, pEle);
    return pEle;
}

bsonElement * bsonDocument::InsDocumentElement(unsigned int Index, const char *pName)
{
    if (NULL == pName || Index >= m_elist.size())    return false;
    bsonDocumentEle *pEle = new bsonDocumentEle();
    if (NULL == pEle)    return false;
    pEle->SetEName(pName);
    m_elist.insert(m_elist.begin() + Index, pEle);
    return pEle;
}

bool bsonDocument::InsNullElement(unsigned int Index, const char *pName)
{
    if (NULL == pName || Index >= m_elist.size())    return false;
    bsonNullEle *pEle = new bsonNullEle();
    if (NULL == pEle)    return false;
    pEle->SetEName(pName);
    m_elist.insert(m_elist.begin() + Index, pEle);
    return !!pEle;
}

unsigned int bsonDocument::ChildCount(void) const
{
    return m_elist.size();
}

bsonElement * bsonDocument::GetElement(unsigned int Index)
{
    if (Index >= m_elist.size())    return NULL;
    return m_elist[Index];
}

bsonElement * bsonDocument::GetElement(const char *pName)
{
    if (NULL == pName)    return NULL;
    std::vector<bsonElement*>::iterator it = m_elist.begin();
    for (; it != m_elist.end(); it++)
    {
        bsonElement *pEle = *it;
        if(!pEle->EName())  continue;

        if (strcmp(pName, pEle->EName()) == 0)
            return *it;
    }
    return NULL;
}

const bsonElement * bsonDocument::GetElement(unsigned int Index) const
{
    if (Index >= m_elist.size())    return NULL;
    return m_elist[Index];
}

const bsonElement * bsonDocument::GetElement(const char *pName) const
{
    if (NULL == pName)    return NULL;
    std::vector<bsonElement*>::const_iterator it = m_elist.begin();
    for (; it != m_elist.end(); it++)
    {
        if (strcmp(pName, (*it)->EName()) == 0)
            return *it;
    }
    return NULL;
}

bool bsonDocument::DelElement(unsigned int Index)
{
    if (Index >= m_elist.size())    return false;
    std::vector<bsonElement*>::iterator it = m_elist.begin() + Index;
    delete (*it);
    m_elist.erase(it);
    return true;
}

bool bsonDocument::Write(bsonStream *pStream) const
{
    if (NULL == pStream)    return false;
    unsigned int size = CalcSize();
    pStream->Write(&size, sizeof(size));

    std::vector<bsonElement*>::const_iterator it = m_elist.begin();
    for (; it != m_elist.end(); it++)
    {
        if (!(*it)->Write(pStream))    return false;
    }

    bsonByte tmp = 0;
    pStream->Write(&tmp, sizeof(tmp));

    return true;
}

bool bsonDocument::Read(bsonStream *pStream)
{
    clear();

    if(!pStream || pStream->DataLen() == 0u)
    {
        return false;
    }

    unsigned totalsize = 0;
    pStream->Read(&totalsize, sizeof(totalsize));
//    if (totalsize != pStream->DataLen())    return false;
    bsonByte type = 0;
    bsonElement *pEle = NULL;
    while(true)
    {
        type = 0;
        pEle = NULL;
        if (!pStream->Read(&type, sizeof(type)))    false;
        if (type == 0)    break;
        pStream->SetPos(pStream->Pos() - sizeof(type));
        switch(type)
        {
        case bsonDoubleType:
            pEle = new bsonDoubleEle();
            if (!pEle->Read(pStream)) { delete pEle;    return false; }
            m_elist.push_back(pEle);
            break;
        case bsonStringType:
            pEle = new bsonStringEle();
            if (!pEle->Read(pStream)) { delete pEle;    return false; }
            m_elist.push_back(pEle);
            break;
        case bsonBinType:
            pEle = new bsonBinaryEle();
            if (!pEle->Read(pStream)) { delete pEle;    return false; }
            m_elist.push_back(pEle);
            break;
        case bsonBoolType:
            pEle = new bsonBoolEle();
            if (!pEle->Read(pStream)) { delete pEle;    return false; }
            m_elist.push_back(pEle);
            break;
        case bsonInt32Type:
            pEle = new bsonInt32Ele();
            if (!pEle->Read(pStream)) { delete pEle;    return false; }
            m_elist.push_back(pEle);
            break;
        case bsonInt64Type:
            pEle = new bsonInt64Ele();
            if (!pEle->Read(pStream)) { delete pEle;    return false; }
            m_elist.push_back(pEle);
            break;
        case bsonArrayType:
            pEle = new bsonArrayEle();
            if (!pEle->Read(pStream)) { delete pEle;    return false; }
            m_elist.push_back(pEle);
            break;
        case bsonDocType:
            pEle = new bsonDocumentEle();
            if (!pEle->Read(pStream)) { delete pEle;    return false; }
            m_elist.push_back(pEle);
            break;
        case bsonNULLType:
            pEle = new bsonNullEle();
            if (!pEle->Read(pStream)) { delete pEle;    return false; }
            m_elist.push_back(pEle);
            break;
        case bsonUndefType:
        case bsonOIDType:
        case bsonUTCTimeType:
        case bsonRegExp:
        case bsonDBPointType:
        case bsonJSCodeType:
        case bsonSymbolType:
        case bsonJSCodeWType:
        case bsonTimestampType:
        case bsonMinKey:
        case bsonMaxKey:
            return false;
        default:
            return false;
        }
    }
    return true;
}

bool bsonDocument::JsonString(std::string &jsonstr) const
{
    jsonstr.clear();
    if(m_elist.size() > 0)
    {
        jsonstr += "{";
        std::string out;
        std::vector<bsonElement*>::const_iterator it = m_elist.begin();

        for (; it != m_elist.end(); it++)
        {
            if (!(*it)->String(out))    continue;
            if (jsonstr.length() > 1)    jsonstr += ",";
            jsonstr += out;
        }
        jsonstr += "}";
    }

    return true;
}

bool convert(cJSON *pnode, bsonDocument *pdoc);

bool convertArray(cJSON *pnode, bsonArrayEle *parr)
{
    if (NULL == pnode || NULL == parr)    return false;

    for (; pnode != NULL; pnode = pnode->next)
    {
        int type = pnode->type & 0xff;
        switch (type)
        {
        case cJSON_False:
            parr->AddBoolElement(false);
            break;
        case cJSON_True:
            parr->AddBoolElement(true);
            break;
        case cJSON_Number:
            parr->AddDblElement(pnode->valuedouble);
            break;
        case cJSON_String:
            {
                const char *psym = "BINARYDATA";
                if (memcmp(psym, pnode->valuestring, 10) == 0) {    //binary data
                    membuf buf;
                    HexToBuf(pnode->valuestring+10, buf);
                    parr->AddBinElement(buf.getdata(), buf.datalen());
                }
                else {
                    parr->AddStringElement(pnode->valuestring);
                }
            }
            break;
        case cJSON_NULL:
            parr->AddNullElement();
            break;
        case cJSON_Array:
            {
                bsonArrayEle *pArrEle = (bsonArrayEle *)parr->AddArrayElement();
                if (NULL != pArrEle) {
                    convertArray(pnode->child, pArrEle);
                }
            }
            break;
        case cJSON_Object:
            {
                bsonDocumentEle *pDocEle = (bsonDocumentEle *)parr->AddDocumentElement();
                if (NULL != pDocEle) {
                    bsonDocument & doc = pDocEle->GetDoc();
                    convert(pnode->child, &doc);
                }
            }
            break;
        default:
            break;
        }
    }
    return true;
}

bool convert(cJSON *pnode, bsonDocument *pdoc)
{
    if (NULL == pnode || NULL == pdoc)    return false;
    int type = pnode->type & 0xff;
    if (NULL == pnode->next && NULL == pnode->prev && type == cJSON_Object && NULL == pnode->string)
    {
        pnode = pnode->child;
    }
    if (NULL == pnode)  return false;
    do
    {
        int type = pnode->type & 0xff;
        std::string strName = NULL==pnode->string?"":pnode->string;
        switch (type)
        {
            case cJSON_False:
                pdoc->AddBoolElement(strName.c_str(), false);
                break;
            case cJSON_True:
                pdoc->AddBoolElement(strName.c_str(), true);
                break;
            case cJSON_NULL:
                pdoc->AddNullElement(strName.c_str());
                break;
            case cJSON_Number:
                pdoc->AddDblElement(strName.c_str(), pnode->valuedouble);
                break;
            case cJSON_String:
                {
                    const char *psym = "BINARYDATA";
                    if (memcmp(psym, pnode->valuestring, 10) == 0) {    //binary data
                        membuf buf;
                        HexToBuf(pnode->valuestring+10, buf);
                        pdoc->AddBinElement(strName.c_str(), buf.getdata(), buf.datalen());
                    } else {
                        pdoc->AddStringElement(strName.c_str(), pnode->valuestring);
                    }
                }
                break;
            case cJSON_Array:
                {
                    bsonArrayEle *pArrEle = (bsonArrayEle *)pdoc->AddArrayElement(strName.c_str());
                    if (NULL != pArrEle) {
                        convertArray(pnode->child, pArrEle);
                    }
                }
                break;
            case cJSON_Object:
                {
                    bsonDocumentEle *pDocEle = (bsonDocumentEle *)pdoc->AddDocumentElement(strName.c_str());
                    if (NULL != pDocEle)
                    {
                        bsonDocument & doc = pDocEle->GetDoc();
                        convert(pnode->child, &doc);
                    }
                }
                break;
            default:
                break;
        }
        pnode = pnode->next;
    } while (NULL != pnode);
    return true;
}

bool bsonDocument::FromJsonString(const std::string &jsonstr)
{
    if (jsonstr.length() < 1)    return false;
    if(strcmp(jsonstr.c_str(), "{}") == 0) return false;
    cJSON *proot = cJSON_Parse(jsonstr.c_str());
    if (NULL == proot)    return false;
    bool bret = convert(proot, this);
    cJSON_Delete(proot);
    return bret;
}


bool bsonDocument::FromBsonStream(const void *pBuffer, unsigned nBufLen)
{
    if(!pBuffer || nBufLen == 0u)
    {
        return false;
    }

    try{
        clear();

        bson::bsonStream bs;
        if(!bs.Write(pBuffer, nBufLen))
        {
            return false;
        }

        bs.Reset();
        Read(&bs);
        return true;
    }
    catch(...)
    {
        printf("%d: %s, %d\n", __LINE__, "parse bson stream failed, because of memory lack.", nBufLen);
    }
    return false;
}


unsigned int bsonDocument::CalcSize(void) const
{
    unsigned size = 0;
    std::vector<bsonElement*>::const_iterator it = m_elist.begin();
    for (; it != m_elist.end(); it++)
    {
        size += (*it)->CalcSize();
    }
    //加上头和尾
    size = size + sizeof(bsonInt32) + sizeof(bsonByte);
    return size;
}

void bsonDocument::clear()
{
    std::vector<bsonElement*>::iterator it = m_elist.begin();
    for (; it != m_elist.end(); it++)
    {
        bsonElement *pElement = *it;
        delete pElement;
    }
    m_elist.clear();
}


/////////////////////////////////////////////////////////////////////////
// bsonDocumentEle class
bsonDocumentEle::bsonDocumentEle()
    : bsonElement(bsonDocType)
{
    m_doc = new bsonDocument;
}

bsonDocumentEle::~bsonDocumentEle()
{
    delete m_doc;
}


bsonDocument &bsonDocumentEle::GetDoc(void)
{
    return *m_doc;
}


const bsonDocument &bsonDocumentEle::GetDoc(void) const
{
    return *m_doc;
}


bool bsonDocumentEle::Write(bsonStream *pStream) const
{
    if (NULL == pStream)    return false;
    WriteElementHeader(this, pStream);
    return m_doc->Write(pStream);
}

bool bsonDocumentEle::Read(bsonStream *pStream)
{
    if (NULL == pStream)    return false;
    ReadElementHeader(this, pStream);
    return m_doc->Read(pStream);
}

bool bsonDocumentEle::JsonString(std::string &jsonstr) const
{
    if (GetType() != bsonDocType)    return false;

    jsonstr.clear();
    jsonstr += "{\"";
    jsonstr += EName();
    jsonstr += "\":";
    std::string out;
    if (!m_doc->JsonString(out))    return false;
    jsonstr += out;
    return true;
}

bool bsonDocumentEle::String(std::string &str) const
{
    if (GetType() != bsonDocType)    return false;

    str.clear();
    str += "\"";
    str += EName();
    str += "\":";
    std::string out;
    if (!m_doc->JsonString(out))    return false;
    str += out;
    return true;
}

bool bsonDocumentEle::ValueString(std::string &str, bool bMarks) const
{
    if (GetType() != bsonDocType)    return false;

    str.clear();
    return m_doc->JsonString(str);
}

unsigned int bsonDocumentEle::CalcSize(void) const
{
    //type + ename + doc
    const char *pszName = EName();
    if(pszName)
    {
        return 1 + strlen(pszName) + 1 + m_doc->CalcSize();
    }
    return 1 + 0 + 1 + m_doc->CalcSize();
}



/////////////////////////////////////////////////////////////////////////
// bsonDocumentEle class
bsonArrayEle::bsonArrayEle()
    : bsonElement(bsonArrayType)
{
    m_doc = new bsonDocument;
}

bsonArrayEle::~bsonArrayEle()
{
    delete m_doc;
}

bool bsonArrayEle::AddBoolElement(bool val)
{
    char tmpstr[32] = "";
    sprintf(tmpstr, "%d", m_doc->ChildCount());
    return m_doc->AddBoolElement(tmpstr, val);
}

bool bsonArrayEle::AddInt32Element(bsonInt32 val)
{
    char tmpstr[32] = "";
    sprintf(tmpstr, "%d", m_doc->ChildCount());
    return m_doc->AddInt32Element(tmpstr, val);
}

bool bsonArrayEle::AddInt64Element(bsonInt64 val)
{
    char tmpstr[32] = "";
    sprintf(tmpstr, "%d", m_doc->ChildCount());
    return m_doc->AddInt64Element(tmpstr, val);
}

bool bsonArrayEle::AddDblElement(double val)
{
    char tmpstr[32] = "";
    sprintf(tmpstr, "%d", m_doc->ChildCount());
    return m_doc->AddDblElement(tmpstr, val);
}

bool bsonArrayEle::AddBinElement(void *pbuf, unsigned int len)
{
    char tmpstr[32] = "";
    sprintf(tmpstr, "%d", m_doc->ChildCount());
    return m_doc->AddBinElement(tmpstr, pbuf, len);
}

bool bsonArrayEle::AddStringElement(const char *val)
{
    char tmpstr[32] = "";
    sprintf(tmpstr, "%d", m_doc->ChildCount());
    return m_doc->AddStringElement(tmpstr, val);
}

bsonElement * bsonArrayEle::AddArrayElement()
{
    char tmpstr[32] = "";
    sprintf(tmpstr, "%d", m_doc->ChildCount());
    return m_doc->AddArrayElement(tmpstr);
}

bsonElement * bsonArrayEle::AddDocumentElement()
{
    char tmpstr[32] = "";
    sprintf(tmpstr, "%d", m_doc->ChildCount());
    return m_doc->AddDocumentElement(tmpstr);
}

bool bsonArrayEle::AddNullElement()
{
    char tmpstr[32] = "";
    sprintf(tmpstr, "%d", m_doc->ChildCount());
    return m_doc->AddNullElement(tmpstr);
}

bool bsonArrayEle::InsBoolElement(unsigned int Index, bool val)
{
    char tmpstr[32] = "";
    sprintf(tmpstr, "%d", m_doc->ChildCount());
    if (!m_doc->InsBoolElement(Index, tmpstr, val))    return false;
    const unsigned int elenum = m_doc->ChildCount();
    for (unsigned int i = Index+1u; i < elenum; i++)
    {
        sprintf(tmpstr, "%d", m_doc->ChildCount());
        m_doc->GetElement(i)->SetEName(tmpstr);
    }
    return true;
}

bool bsonArrayEle::InsInt32Element(unsigned int Index, bsonInt32 val)
{
    char tmpstr[32] = "";
    sprintf(tmpstr, "%d", m_doc->ChildCount());
    if (!m_doc->InsInt32Element(Index, tmpstr, val))    return false;
    const unsigned int elenum = m_doc->ChildCount();
    for (unsigned int i = Index+1u; i < elenum; i++)
    {
        sprintf(tmpstr, "%d", m_doc->ChildCount());
        m_doc->GetElement(i)->SetEName(tmpstr);
    }
    return true;
}

bool bsonArrayEle::InsInt64Element(unsigned int Index, bsonInt64 val)
{
    char tmpstr[32] = "";
    sprintf(tmpstr, "%d", m_doc->ChildCount());
    if (!m_doc->InsInt64Element(Index, tmpstr, val))    return false;
    const unsigned int elenum = m_doc->ChildCount();
    for (unsigned int i = Index+1u; i < elenum; i++)
    {
        sprintf(tmpstr, "%d", m_doc->ChildCount());
        m_doc->GetElement(i)->SetEName(tmpstr);
    }
    return true;
}

bool bsonArrayEle::InsDblElement(unsigned int Index, double val)
{
    char tmpstr[32] = "";
    sprintf(tmpstr, "%d", m_doc->ChildCount());
    if (!m_doc->InsDblElement(Index, tmpstr, val))    return false;
    const unsigned int elenum = m_doc->ChildCount();
    for (unsigned int i = Index+1u; i < elenum; i++)
    {
        sprintf(tmpstr, "%d", m_doc->ChildCount());
        m_doc->GetElement(i)->SetEName(tmpstr);
    }
    return true;
}

bool bsonArrayEle::InsBinElement(unsigned int Index, void *pbuf, unsigned int len)
{
    char tmpstr[32] = "";
    sprintf(tmpstr, "%d", m_doc->ChildCount());
    if (!m_doc->InsBinElement(Index, tmpstr, pbuf, len))    return false;
    const unsigned int elenum = m_doc->ChildCount();
    for (unsigned int i = Index+1u; i < elenum; i++)
    {
        sprintf(tmpstr, "%d", m_doc->ChildCount());
        m_doc->GetElement(i)->SetEName(tmpstr);
    }
    return true;
}

bool bsonArrayEle::InsStringElement(unsigned int Index, const char *val)
{
    char tmpstr[32] = "";
    sprintf(tmpstr, "%d", m_doc->ChildCount());
    if (!m_doc->InsStringElement(Index, tmpstr, val))    return false;
    const unsigned int elenum = m_doc->ChildCount();
    for (unsigned int i = Index+1u; i < elenum; i++)
    {
        sprintf(tmpstr, "%d", m_doc->ChildCount());
        m_doc->GetElement(i)->SetEName(tmpstr);
    }
    return true;
}

bsonElement * bsonArrayEle::InsArrayElement(unsigned int Index)
{
    char tmpstr[32] = "";
    sprintf(tmpstr, "%d", m_doc->ChildCount());
    bsonElement * pEle = m_doc->InsArrayElement(Index, tmpstr);
    if (NULL == pEle)    return NULL;
    const unsigned int elenum = m_doc->ChildCount();
    for (unsigned int i = Index+1u; i < elenum; i++)
    {
        sprintf(tmpstr, "%d", m_doc->ChildCount());
        m_doc->GetElement(i)->SetEName(tmpstr);
    }
    return pEle;
}

bsonElement * bsonArrayEle::InsDocumentElement(unsigned int Index)
{
    char tmpstr[32] = "";
    sprintf(tmpstr, "%d", m_doc->ChildCount());
    bsonElement * pEle = m_doc->InsDocumentElement(Index, tmpstr);
    if (NULL == pEle)    return NULL;
    const unsigned int elenum = m_doc->ChildCount();
    for (unsigned int i = Index+1u; i < elenum; i++)
    {
        sprintf(tmpstr, "%d", m_doc->ChildCount());
        m_doc->GetElement(i)->SetEName(tmpstr);
    }
    return pEle;
}

bool bsonArrayEle::IncNullElement(unsigned int Index)
{
    char tmpstr[32] = "";
    sprintf(tmpstr, "%d", m_doc->ChildCount());
    if (!m_doc->InsNullElement(Index, tmpstr))    return false;
    const unsigned int elenum = m_doc->ChildCount();
    for (unsigned int i = Index+1u; i < elenum; i++)
    {
        sprintf(tmpstr, "%d", m_doc->ChildCount());
        m_doc->GetElement(i)->SetEName(tmpstr);
    }
    return true;
}

unsigned int bsonArrayEle::ChildCount(void) const
{
    return m_doc->ChildCount();
}

bsonElement * bsonArrayEle::GetElement(unsigned int Index)
{
    return m_doc->GetElement(Index);
}

bsonElement * bsonArrayEle::GetElement(const char *pName)
{
    return m_doc->GetElement(pName);
}

const bsonElement * bsonArrayEle::GetElement(unsigned int Index) const
{
    return m_doc->GetElement(Index);
}


const bsonElement * bsonArrayEle::GetElement(const char *pName) const
{
    return m_doc->GetElement(pName);
}


bool bsonArrayEle::DelElement(unsigned int Index)
{
    return m_doc->DelElement(Index);
}

bool bsonArrayEle::Write(bsonStream *pStream) const
{
    if (NULL == pStream)    return false;
    WriteElementHeader(this, pStream);
    return m_doc->Write(pStream);
}

bool bsonArrayEle::Read(bsonStream *pStream)
{
    if (NULL == pStream)    return false;
    ReadElementHeader(this, pStream);
    return m_doc->Read(pStream);
}

bool bsonArrayEle::JsonString(std::string &jsonstr) const
{
    if (GetType() != bsonArrayType)    return false;

    jsonstr.clear();
    jsonstr += "{\"";
    jsonstr += EName();
    jsonstr += "\":[";
    std::string out;
    const unsigned int elenum = m_doc->ChildCount();
    for (unsigned int i = 0u; i < elenum; i++)
    {
        const bsonElement *pele = m_doc->GetElement(i);
        if (NULL == pele)    continue;
        if (!pele->ValueString(out))    continue;
        if (i > 0u)    jsonstr += ",";
        jsonstr += out;
    }
    jsonstr += "]}";
    return true;
}

bool bsonArrayEle::String(std::string &str) const
{
    if (GetType() != bsonArrayType)    return false;

    str.clear();
    str += "\"";
    str += EName();
    str += "\":[";
    std::string out;
    const unsigned int elenum = m_doc->ChildCount();
    for (unsigned int i = 0u; i < elenum; i++)
    {
        const bsonElement *pele = m_doc->GetElement(i);
        if (NULL == pele)    continue;
        if (!pele->ValueString(out))    continue;
        if (i > 0)    str += ",";
        str += out;
    }
    str += "]";
    return true;
}

bool bsonArrayEle::ValueString(std::string &str, bool bMarks) const
{
    if (GetType() != bsonArrayType)    return false;

    str.clear();
    str += "[";
    std::string out;
    const unsigned int elenum = m_doc->ChildCount();
    for (unsigned int i = 0; i < elenum; i++)
    {
        const bsonElement *pele = m_doc->GetElement(i);
        if (NULL == pele)    continue;
        if (!pele->ValueString(out))    continue;
        if (i > 0)    str += ",";
        str += out;
    }
    str += "]";
    return true;
}

unsigned int bsonArrayEle::CalcSize(void) const
{
    //type + ename + array
    const char *pszName = EName();
    if(pszName)
    {
        return 1 + strlen(pszName) + 1 + m_doc->CalcSize();
    }
    return 1 + 0 + 1 + m_doc->CalcSize();
}
}