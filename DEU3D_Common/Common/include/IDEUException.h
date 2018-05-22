#ifndef I_DEUEXCEPTION_H_A9931EB5_67FF_41AD_A22C_BA4A2931ACBA_INCLUDE
#define I_DEUEXCEPTION_H_A9931EB5_67FF_41AD_A22C_BA4A2931ACBA_INCLUDE

#include <string>
#include <OpenSP/Ref.h>
#include "Export.h"
namespace cmm
{
///////////////////////////////////////////////////////////
//
//  �����ߣ�  ���
//  ����ʱ�䣺2013-11-20
//  ���ܼ�飺DEUException����ĳ������       
//
///////////////////////////////////////////////////////////
class IDEUException : virtual public OpenSP::Ref
{

public:
	//�����룬0Ϊ�ɹ�������0Ϊʧ��
	virtual unsigned long getReturnCode() const = 0; 
	virtual void setReturnCode(unsigned long ulReturnCode) = 0;

	//�����ַ���
	virtual const std::string &getMessage() const = 0;
    virtual void setMessage(const std::string &strMessage) = 0;

	//���³���Ķ��󣬷Ǳ���
	virtual const std::string &getSource() const = 0;
    virtual void setSource(const std::string &strSource) = 0;

	//���³���ĺ������Ǳ���
	virtual const std::string &getTargetSite() const = 0;
    virtual void setTargetSite(const std::string &strTargetSite) = 0;

	//���ö���m_ulReturnCode����Ϊ0��������Ա����Ϊ��
	virtual void Reset() = 0;

    virtual const IDEUException & operator= (const IDEUException &param) = 0;
};

CM_EXPORT IDEUException *createDEUException(void);
}



#endif