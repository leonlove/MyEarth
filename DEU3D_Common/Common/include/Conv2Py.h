#ifndef	__CONV_2_PY_H__
#define	__CONV_2_PY_H__

#include <vector>
#include <string>
#include "Export.h"

namespace cmm
{
//��conv2py��ʵ�ֽ�����ת����ƴ���Ĺ���
//Ŀǰֻ֧��ansi�ַ����ĺ��ֱ���
//ת������У�ÿ������ƴ������Զ����Ͽո�
class CM_EXPORT Conv2PyS
{
public:
	Conv2PyS(void);
	~Conv2PyS(void);

public:
	//ָ��ƴ��������λ�ã���ʼ����
	//��λ�ñ������pytbl.dat��pyidx.dat�ļ�
	bool init(const char *pPath);

public:
	//ת�����ֵ�ƴ����bpunctuation��ʾ�Ƿ������ı��
	void convert(const std::string &instr, std::string &outstr, bool bpunctuation = true);
	std::string convert(const std::string &instr, bool bpunctuation = true);

private:
	bool			m_binit;
	short			*m_ptbl;
	std::vector<std::string>	m_pyidx;
};

}
#endif
