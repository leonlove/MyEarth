#ifndef CDUEEXCEPTION_H_16232D0C_9D85_404C_B879_9CE2E047FA1F_INCLUDE
#define CDUEEXCEPTION_H_16232D0C_9D85_404C_B879_9CE2E047FA1F_INCLUDE

#include "IDEUException.h"
namespace cmm
{
///////////////////////////////////////////////////////////
//
//  创建者：  朱辉
//  创建时间：2013-11-20
//  功能简介：错误对象，返回错误码及错误字符串       
//
///////////////////////////////////////////////////////////
class CDEUException: public IDEUException
{
public:
	CDEUException(void);
        
    CDEUException(const CDEUException &param);

    const CDEUException & operator= (const CDEUException &param);


	~CDEUException(void);
public:
    virtual const IDEUException & operator= (const IDEUException &param);

	//返回码，0为成功，大于0为失败
	unsigned long getReturnCode() const; 
	void setReturnCode(unsigned long ulReturnCode);

	//错误字符串
	const std::string &getMessage() const;
    void setMessage(const std::string &strMessage);

	//导致出错的对象，非必填
	const std::string &getSource() const;
    void setSource(const std::string &strSource);

	//导致出错的函数，非必填
	const std::string &getTargetSite() const;
    void setTargetSite(const std::string &strTargetSite);

	//重置对象，m_ulReturnCode设置为0，其他成员设置为空
	void Reset();
private:
	unsigned long m_ulReturnCode;
	std::string   m_strMessage;
	std::string   m_strSource;
	std::string   m_strTargetSite;
};
}
#endif