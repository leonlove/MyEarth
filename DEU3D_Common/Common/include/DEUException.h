#ifndef CDUEEXCEPTION_H_16232D0C_9D85_404C_B879_9CE2E047FA1F_INCLUDE
#define CDUEEXCEPTION_H_16232D0C_9D85_404C_B879_9CE2E047FA1F_INCLUDE

#include "IDEUException.h"
namespace cmm
{
///////////////////////////////////////////////////////////
//
//  �����ߣ�  ���
//  ����ʱ�䣺2013-11-20
//  ���ܼ�飺������󣬷��ش����뼰�����ַ���       
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

	//�����룬0Ϊ�ɹ�������0Ϊʧ��
	unsigned long getReturnCode() const; 
	void setReturnCode(unsigned long ulReturnCode);

	//�����ַ���
	const std::string &getMessage() const;
    void setMessage(const std::string &strMessage);

	//���³���Ķ��󣬷Ǳ���
	const std::string &getSource() const;
    void setSource(const std::string &strSource);

	//���³���ĺ������Ǳ���
	const std::string &getTargetSite() const;
    void setTargetSite(const std::string &strTargetSite);

	//���ö���m_ulReturnCode����Ϊ0��������Ա����Ϊ��
	void Reset();
private:
	unsigned long m_ulReturnCode;
	std::string   m_strMessage;
	std::string   m_strSource;
	std::string   m_strTargetSite;
};
}
#endif