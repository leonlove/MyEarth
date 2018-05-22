#include <DEUException.h>

namespace cmm
{

IDEUException *createDEUException(void)
{
    return new CDEUException;
}

CDEUException::CDEUException(void)
{
    Reset();
}

CDEUException::CDEUException(const CDEUException &param)
{
    m_ulReturnCode = param.m_ulReturnCode;
    m_strMessage = param.m_strMessage;
    m_strSource = param.m_strSource;
    m_strTargetSite = param.m_strTargetSite;
}


const CDEUException & CDEUException::operator=(const CDEUException &param)
{
    m_ulReturnCode = param.m_ulReturnCode;
    m_strMessage = param.m_strMessage;
    m_strSource = param.m_strSource;
    m_strTargetSite = param.m_strTargetSite;

    return *this;
}

CDEUException::~CDEUException(void)
{
}

const IDEUException & CDEUException::operator= (const IDEUException &param)
{
    if (this == &param) return *this;

    const CDEUException &p = dynamic_cast<const CDEUException &>(param);
    m_ulReturnCode  = p.m_ulReturnCode;
    m_strMessage    = p.m_strMessage;
    m_strSource     = p.m_strSource;
    m_strTargetSite = p.m_strTargetSite;

    return *this;
}

unsigned long CDEUException::getReturnCode() const
{
	return m_ulReturnCode;
}

void CDEUException::setReturnCode(unsigned long ulReturnCode)
{
	m_ulReturnCode = ulReturnCode;
}

const std::string &CDEUException::getMessage() const
{
	return m_strMessage;
}

void CDEUException::setMessage(const std::string &strMessage)
{
	m_strMessage = strMessage;
}

const std::string &CDEUException::getSource() const
{
	return m_strSource;
}

void CDEUException::setSource(const std::string &strSource)
{
	m_strSource = strSource;
}

const std::string &CDEUException::getTargetSite() const
{
	return m_strTargetSite;
}

void CDEUException::setTargetSite(const std::string &strTargetSite)
{
	m_strTargetSite = strTargetSite;
}

void CDEUException::Reset()
{
	m_ulReturnCode = 0;
	m_strMessage.clear();
	m_strSource.clear();
	m_strTargetSite.clear();
}
}