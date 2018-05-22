#include <stdio.h>
#include <Conv2Py.h>

namespace cmm
{
Conv2PyS::Conv2PyS(void)
	: m_binit(false)
	, m_ptbl(NULL)
{
}


Conv2PyS::~Conv2PyS(void)
{
	if (NULL != m_ptbl)
	{
		delete []m_ptbl;
		m_ptbl = NULL;
	}
}

bool loadhashtable(const char *ppathname, short **pptbl)
{
	if (NULL == ppathname || NULL == pptbl)
		return false;
	*pptbl = NULL;
	FILE *pf = fopen(ppathname, "rb");
	if (NULL == pf)	return false;
	fseek(pf, 0, SEEK_END);
	long len = ftell(pf);
	*pptbl = new short[len / sizeof(short)];
	if (NULL == *pptbl)
	{
		fclose(pf);
		return false;
	}
	fseek(pf, 0, SEEK_SET);
	fread(*pptbl, 1, len, pf);
	fclose(pf);
	return true;
}

bool loadpyidx(const char *ppathname, std::vector<std::string> &idx)
{
	if (NULL == ppathname)
		return false;
	FILE *pf = fopen(ppathname, "rb");
	if (NULL == pf)	return false;
	char buf[16] = "";
	char *pfind = NULL;
	while (fgets(buf,16, pf) != NULL)
	{
		pfind = strstr(buf, "\r\n");
		if (pfind) *pfind = 0;
		idx.push_back(buf);
	}
	fclose(pf);
	return true;
}

bool Conv2PyS::init(const char *pPath)
{
	if (NULL == pPath)	return false;
	std::string spath = pPath;
	if (spath.length() < 1) return false;
	if (spath[spath.length()-1] != '\\') spath += '\\';
	std::string tblname = spath + "pytbl.dat";
	std::string idxname = spath + "pyidx.dat";

	if (!loadhashtable(tblname.c_str(), &m_ptbl))	return false;
	if (!loadpyidx(idxname.c_str(), m_pyidx))		return false;
	m_binit = true;
	return true;
}

unsigned short getcode(const char *p)
{
	if (NULL == p)	return 0;

	unsigned short code = 0;

	code = (unsigned char )p[0];
	code <<= 8;
	code |= (unsigned char )p[1];

	return code;
}

void convert(short *ptbl, std::vector<std::string> &idx, const std::string &instr, std::string &outstr, bool bpunctuation)
{
	outstr.clear();
	if (NULL == ptbl)	return;

	std::string::const_iterator p = instr.begin();
	unsigned short ch, code;
	bool bfind = true;
	while (p != instr.end())
	{
		if (*p > 0 && *p < 0x80)
		{
			outstr += *p;
			p++;
			bfind = false;
			continue;
		}
		ch = (unsigned char )(*p);
		if (ch < 0xa1)
		{
			p++;
			bfind = false;
			continue;
		}
		if (!bfind) outstr += " ";
		bfind = true;
		code = getcode(p._Ptr);
		if (ptbl[code] < 0)
		{
			if (bpunctuation)	outstr.append(p, p+2);
			p+=2;
			continue;
		}
		outstr += idx[ptbl[code]];
		p+=2;
		outstr += " ";
	}
}

void Conv2PyS::convert(const std::string &instr, std::string &outstr, bool bpunctuation)
{
	if (!m_binit)	return;
	cmm::convert(m_ptbl, m_pyidx, instr, outstr, bpunctuation);
}

std::string Conv2PyS::convert(const std::string &instr, bool bpunctuation)
{
	std::string outstr;
	convert(instr, outstr, bpunctuation);
	return outstr;
}
}
