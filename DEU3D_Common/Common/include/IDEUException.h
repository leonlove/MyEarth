#ifndef I_DEUEXCEPTION_H_A9931EB5_67FF_41AD_A22C_BA4A2931ACBA_INCLUDE
#define I_DEUEXCEPTION_H_A9931EB5_67FF_41AD_A22C_BA4A2931ACBA_INCLUDE

#include <string>
#include <OpenSP/Ref.h>
#include "Export.h"
namespace cmm
{
///////////////////////////////////////////////////////////
//
//  创建者：  朱辉
//  创建时间：2013-11-20
//  功能简介：DEUException对象的抽象基类       
//
///////////////////////////////////////////////////////////
class IDEUException : virtual public OpenSP::Ref
{

public:
	//返回码，0为成功，大于0为失败
	virtual unsigned long getReturnCode() const = 0; 
	virtual void setReturnCode(unsigned long ulReturnCode) = 0;

	//错误字符串
	virtual const std::string &getMessage() const = 0;
    virtual void setMessage(const std::string &strMessage) = 0;

	//导致出错的对象，非必填
	virtual const std::string &getSource() const = 0;
    virtual void setSource(const std::string &strSource) = 0;

	//导致出错的函数，非必填
	virtual const std::string &getTargetSite() const = 0;
    virtual void setTargetSite(const std::string &strTargetSite) = 0;

	//重置对象，m_ulReturnCode设置为0，其他成员设置为空
	virtual void Reset() = 0;

    virtual const IDEUException & operator= (const IDEUException &param) = 0;
};

CM_EXPORT IDEUException *createDEUException(void);
}



#endif