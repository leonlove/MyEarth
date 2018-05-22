#ifndef	__CONV_2_PY_H__
#define	__CONV_2_PY_H__

#include <vector>
#include <string>
#include "Export.h"

namespace cmm
{
//类conv2py，实现将汉字转换成拼音的功能
//目前只支持ansi字符集的汉字编码
//转换结果中，每个汉字拼音间会自动加上空格
class CM_EXPORT Conv2PyS
{
public:
	Conv2PyS(void);
	~Conv2PyS(void);

public:
	//指定拼音索引的位置，初始化类
	//该位置必须存在pytbl.dat和pyidx.dat文件
	bool init(const char *pPath);

public:
	//转换汉字到拼音，bpunctuation表示是否处理中文标点
	void convert(const std::string &instr, std::string &outstr, bool bpunctuation = true);
	std::string convert(const std::string &instr, bool bpunctuation = true);

private:
	bool			m_binit;
	short			*m_ptbl;
	std::vector<std::string>	m_pyidx;
};

}
#endif
