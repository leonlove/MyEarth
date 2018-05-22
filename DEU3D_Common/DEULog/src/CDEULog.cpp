#include "CDEULog.h"
#include "io.h"
//#include "stdio.h"
//#include "stdlib.h"
#include "json/cJSON.h"

/////////////////////////////////////////////////////////////////////////
namespace deulog
{

    // Global function
    static inline int GetRcdSize(ConfigItem *pcitem)
    {
        if (pcitem == NULL)	return 0;
        int nsize = 0;
        std::vector<FormatItem>::iterator p;
        for (p = pcitem->Format.begin(); p != pcitem->Format.end(); p++)
        {
            nsize += p->Size;
        }
        return nsize;
    }

    static inline int GetFileSize(FILE *pf)
    {
        if (NULL == pf)	return -1;
        int curpos = ftell(pf);
        fseek(pf, 0, SEEK_END);
        int len = ftell(pf);
        fseek(pf, curpos, SEEK_SET);
        return len;
    }


    bool WriteJsonFile(const char *pFileName, cJSON *pRoot)
    {
        FILE *pf = fopen(pFileName, "wb");
        if (NULL == pf)		{return false;}
        char *pbuf = cJSON_PrintUnformatted(pRoot);
        if (NULL == pbuf)	{return false;}
        int len = strlen(pbuf);
        fwrite(pbuf, 1, len, pf);
        free(pbuf);
        fclose(pf);
        return true;
    }

    cJSON * ReadJsonFile(const char *pFileName)
    {
        FILE *pf = fopen(pFileName, "rb");
        if (NULL == pf)	return NULL;
        fseek(pf, 0, SEEK_END);
        int len = ftell(pf);
        fseek(pf, 0, SEEK_SET);
        char *pbuf = new char[len+1];
        if (NULL == pbuf)	return NULL;
        memset (pbuf, 0, len+1);
        fread(pbuf, 1, len, pf);
        fclose(pf);

        cJSON *pRoot = cJSON_Parse(pbuf);
        delete []pbuf;
        if (NULL == pRoot)	return NULL;

        return pRoot;
    }

    /////////////////////////////////////////////////////////////////////////////////
    // DEULogLock
    DEULogLock::DEULogLock()
    {
        init();
    }

    DEULogLock::~DEULogLock()
    {
        DeleteCriticalSection(&m_cs);
    }

    void DEULogLock::init()
    {
        InitializeCriticalSection(&m_cs);
    }

    void DEULogLock::lock()
    {
        EnterCriticalSection(&m_cs);
    }

    void DEULogLock::unlock()
    {
        LeaveCriticalSection(&m_cs);
    }


    ////////////////////////////////////////////////////////////////////////////
    // DEULogConfig
    DEULogConfig::DEULogConfig(const char *pRootFileName)
    {
        if (NULL != pRootFileName)
            m_RootFileName = pRootFileName;
    }
    DEULogConfig::~DEULogConfig()
    {
    }

    unsigned int DEULogConfig::Count(void) const
    {
        return m_TypeList.size();
    }

    ConfigItem & DEULogConfig::operator[](unsigned int index)
    {
        static ConfigItem tmpItem;
        if (index < 0 || index >= m_TypeList.size())	return tmpItem;
        return m_TypeList[index];
    }

    ConfigItem & DEULogConfig::GetConfigItem(const char *ptype)
    {
        //static ConfigItem tmpItem;
        if (NULL == ptype)	return *((ConfigItem *)NULL);

        std::vector<ConfigItem>::iterator p = m_TypeList.begin();
        for (; p != m_TypeList.end(); p++)
        {
            if (p->TypeID == ptype)	return (*p);
        }
        return *((ConfigItem *)NULL);
    }

    void DEULogConfig::AddItem(const ConfigItem &item)
    {
        if (item.TypeID.length() > 0)	m_TypeList.push_back(item);
    }

    void DEULogConfig::InsItem(unsigned int Index, const ConfigItem &item)
    {
        if (Index < 0) {
            std::vector<ConfigItem>::iterator p = m_TypeList.begin();
            m_TypeList.insert(p, item);
        } else if (Index >= m_TypeList.size()) {
            if (item.TypeID.length() > 0)	m_TypeList.push_back(item);
        } else {
            std::vector<ConfigItem>::iterator p = m_TypeList.begin();
            p+=Index;
            m_TypeList.insert(p, item);
        }
    }

    const char * DEULogConfig::GetRoot()
    {
        return m_RootFileName.c_str();
    }

    void DEULogConfig::SetRoot(const char *pRoot)
    {
        if (NULL != pRoot)
            m_RootFileName = pRoot;
    }

    bool DEULogConfig::Save(const char *pConfigFileName)
    {
        if ((NULL == pConfigFileName) && (m_RootFileName.length() < 1))
            return false;

        cJSON *pRoot = cJSON_CreateObject();
        if (NULL == pRoot)	return false;
        cJSON_AddStringToObject(pRoot, "root", m_RootFileName.c_str()); 
        cJSON *pArr = cJSON_CreateArray();
        if (NULL == pArr)	{cJSON_Delete(pRoot); return false;}
        cJSON_AddItemToObject(pRoot, "types", pArr);

        std::vector<ConfigItem>::iterator p;
        for (p = m_TypeList.begin(); p != m_TypeList.end(); p++)
        {
            cJSON *pType = cJSON_CreateObject();
            if (NULL == pType)	{cJSON_Delete(pRoot); return false;}
            cJSON_AddItemToArray(pArr, pType);
            cJSON_AddStringToObject(pType, "typeid", p->TypeID.c_str());
            cJSON_AddStringToObject(pType, "desc", p->TypeDesc.c_str());
            cJSON *pFormatArr = cJSON_CreateArray();
            if (NULL == pFormatArr)	{cJSON_Delete(pRoot); return false;}
            cJSON_AddItemToObject(pType, "format", pFormatArr);
            std::vector<FormatItem>::iterator pformat;
            for (pformat = p->Format.begin(); pformat != p->Format.end(); pformat++)
            {
                cJSON *pFormatItem = cJSON_CreateObject();
                if (NULL == pFormatItem)	{cJSON_Delete(pRoot); return false;}
                cJSON_AddItemToArray(pFormatArr, pFormatItem);
                cJSON_AddStringToObject(pFormatItem, "name", pformat->Name.c_str());
                cJSON_AddNumberToObject(pFormatItem, "type", pformat->Type);
                cJSON_AddNumberToObject(pFormatItem, "size", pformat->Size);
            }
        }
        return WriteJsonFile(pConfigFileName, pRoot);
    }
    //{"root":"rootfile", "types":[{"typeid":"1","desc":"aaaaa","format":[{"name":"f1","len":8,"type":1},...]},{},{}...]}

    bool DEULogConfig::Load(const char *pConfigFileName)
    {
        if ((NULL == pConfigFileName) && (m_RootFileName.length() < 1))
            return false;

        cJSON *pRoot = ReadJsonFile(pConfigFileName);
        if (NULL == pRoot)	return false;
        if (pRoot->type != cJSON_Object)	{cJSON_Delete(pRoot); return false;}

        m_TypeList.clear();
        m_RootFileName = "";

        cJSON *ptmp = pRoot->child;
        while (NULL != ptmp)
        {
            if (strcmp(ptmp->string, "root") == 0) {
                m_RootFileName = ptmp->valuestring;
            } else if (strcmp(ptmp->string, "types") == 0) {
                cJSON *pArr = ptmp->child;
                while (NULL != pArr)
                {
                    ConfigItem item;
                    cJSON *pitem = pArr->child;
                    while (NULL != pitem)
                    {
                        if (strcmp(pitem->string, "typeid") == 0) {
                            item.TypeID = pitem->valuestring;
                        } else if (strcmp(pitem->string, "desc") == 0) {
                            item.TypeDesc = pitem->valuestring;
                        } else if (strcmp(pitem->string, "format") == 0) {
                            cJSON *pFormatItem = pitem->child;
                            while (NULL != pFormatItem)
                            {
                                FormatItem fitem;
                                cJSON *pfitem = pFormatItem->child;
                                while (NULL != pfitem){
                                    if (strcmp(pfitem->string, "name") == 0) {
                                        fitem.Name = pfitem->valuestring;
                                    } else if (strcmp(pfitem->string, "type") == 0) {
                                        fitem.Type = pfitem->valueint;
                                    } else if (strcmp(pfitem->string, "size") == 0) {
                                        fitem.Size = pfitem->valueint;
                                    } else {
                                    }
                                    pfitem = pfitem->next;
                                }
                                if (fitem.Size > 0)	item.Format.push_back(fitem);
                                pFormatItem = pFormatItem->next;
                            }
                        } else {
                        }
                        pitem = pitem->next;
                    }
                    if (item.TypeID.length() > 0)	m_TypeList.push_back(item);
                    pArr = pArr->next;
                }
            } else {
            }
            ptmp = ptmp->next;
        }
        cJSON_Delete(pRoot); 
        return true;
    }

    void DEULogConfig::clear()
    {
        m_RootFileName = "";
        m_TypeList.clear();
    }

    //{"root":"rootfile", "types":[{"typeid":"1","desc":"aaaaa","format":[{"name":"f1","len":8,"type":1},...]},{},{}...]}


    ////////////////////////////////////////////////////////////////////
    // DEULogRoot
    DEULogRoot::DEULogRoot()
    {
    }

    DEULogRoot::~DEULogRoot()
    {
    }

    unsigned int DEULogRoot::Count() const
    {
        return m_CategoryItems.size();
    }

    CategoryItem & DEULogRoot::operator[](unsigned int index)
    {
        static CategoryItem tmpItem;
        if (index >= m_CategoryItems.size())	return tmpItem;
        return m_CategoryItems[index];
    }

    CategoryItem & DEULogRoot::GetItem(const char *ptype)
    {
        static CategoryItem tmpItem;
        if (NULL == ptype)	return tmpItem;

        std::vector<CategoryItem>::iterator p = m_CategoryItems.begin();
        for (; p != m_CategoryItems.end(); p++)
        {
            if (p->Type == ptype)	return (*p);
        }
        return tmpItem;
    }

    void DEULogRoot::AddItem(const CategoryItem &item)
    {
        m_CategoryItems.push_back(item);
    }

    void DEULogRoot::InsItem(unsigned int Index, const CategoryItem &item)
    {
        if (Index < 0) {
            std::vector<CategoryItem>::iterator p = m_CategoryItems.begin();
            m_CategoryItems.insert(p, item);
        } else if (Index >= m_CategoryItems.size()) {
            if (item.Type.length() > 0)	m_CategoryItems.push_back(item);
        } else {
            std::vector<CategoryItem>::iterator p = m_CategoryItems.begin();
            p+=Index;
            m_CategoryItems.insert(p, item);
        }
    }

    bool DEULogRoot::Save(const char *pRootFileName)
    {
        if (NULL == pRootFileName)	return false;

        cJSON *pRoot = cJSON_CreateObject();
        if (NULL == pRoot)	return false;
        cJSON *pArr = cJSON_CreateArray();
        if (NULL == pArr)	{cJSON_Delete(pRoot); return false;}
        cJSON_AddItemToObject(pRoot, "category", pArr);

        std::vector<CategoryItem>::iterator p;
        for (p = m_CategoryItems.begin(); p != m_CategoryItems.end(); p++)
        {
            cJSON *pItem = cJSON_CreateObject();
            if (NULL == pItem)	{cJSON_Delete(pRoot); return false;}
            cJSON_AddItemToArray(pArr, pItem);
            cJSON_AddStringToObject(pItem, "type", p->Type.c_str());
            cJSON_AddStringToObject(pItem, "dir", p->Dir.c_str());
            cJSON *pFiles = cJSON_CreateArray();
            if (NULL == pFiles)	{cJSON_Delete(pRoot); return false;}
            cJSON_AddItemToObject(pItem, "files", pFiles);

            std::vector<std::string>::iterator pfiles;
            for (pfiles = p->Files.begin(); pfiles != p->Files.end(); pfiles++)
            {
                cJSON_AddItemToArray(pFiles, cJSON_CreateString(pfiles->c_str()));
            }
        }
        return WriteJsonFile(pRootFileName, pRoot);
    }

    bool DEULogRoot::Load(const char *pRootFileName)
    { 
        if (NULL == pRootFileName)
            return false;

        cJSON *pRoot = ReadJsonFile(pRootFileName);
        if (NULL == pRoot)	return false;
        if (pRoot->type != cJSON_Object)	{cJSON_Delete(pRoot); return false;}

        m_CategoryItems.clear();
        cJSON *ptmp = pRoot->child;
        while (NULL != ptmp)
        {
            if (strcmp(ptmp->string, "category") == 0) {
                cJSON *pArr = ptmp->child;
                while (NULL != pArr)
                {
                    CategoryItem item;
                    cJSON *pitem = pArr->child;
                    while (NULL != pitem)
                    {
                        if (strcmp(pitem->string, "type") == 0) {
                            item.Type = pitem->valuestring;
                        } else if (strcmp(pitem->string, "dir") == 0) {
                            item.Dir = pitem->valuestring;
                        } else if (strcmp(pitem->string, "files") == 0) {
                            cJSON *pArrFiles = pitem->child;
                            std::string filename;
                            while (NULL != pArrFiles)
                            {
                                filename = pArrFiles->valuestring;
                                item.Files.push_back(filename);
                                pArrFiles = pArrFiles->next;
                            }
                        } else {
                        }
                        pitem = pitem->next;
                    }
                    m_CategoryItems.push_back(item);
                    pArr = pArr->next;
                }
            } else {
            }
            ptmp = ptmp->next;
        }
        cJSON_Delete(pRoot); 
        return true;
    }

    void DEULogRoot::clear()
    {
        m_CategoryItems.clear();
    }



    //////////////////////////////////////////////////////////////////////////////
    // DEULogRecord
    DEULogRecord::DEULogRecord(ConfigItem *pcitem)
        : m_pcitem(pcitem)
        , m_bRef(false)
        , m_pbuf(NULL)
        , m_rcdtime(0)
    {
        init(pcitem);
    }

    DEULogRecord::~DEULogRecord()
    {
        if (!m_bRef && NULL != m_pbuf)
        {
            delete []m_pbuf;
        }
    }

    const std::string &	DEULogRecord::TypeID(void) const
    {
        return m_pcitem->TypeID;
    }

    const std::string &	DEULogRecord::TypeDesc(void) const
    {
        return m_pcitem->TypeDesc;
    }

    unsigned int DEULogRecord::FldCount(void) const
    {
        return m_pcitem->Format.size();
    }

    DEULogFld *	DEULogRecord::GetFld(unsigned int index)
    {
        if (index >= m_pcitem->Format.size())
            return NULL;
        return &(m_pcitem->Format[index]);
    }

    DEULogFld * DEULogRecord::GetFld(const char *pName)
    {
        std::vector<FormatItem>::iterator p;
        for (p = m_pcitem->Format.begin(); p != m_pcitem->Format.end(); p++)
        {
            if (p->Name == pName)	return p._Ptr;
        }
        return NULL;
    }

    void DEULogRecord::SetData(char *pBuf)
    {
        if (m_bRef)	m_pbuf = NULL;
        if (NULL == m_pbuf)
        {
            m_pbuf = new char[m_size];
            if (NULL == m_pbuf)	return;
        }
        memcpy(m_pbuf, pBuf, m_size);
        m_bRef = false;
    }

    void		DEULogRecord::SetDataRef(char *pBuf)
    {
        if (!m_bRef && NULL != m_pbuf)
        {
            delete []m_pbuf;
            m_pbuf = NULL;
        }
        m_pbuf = pBuf;
        m_bRef = true;
    }

    void *		DEULogRecord::GetData()
    {
        return m_pbuf;
    }

    bool		DEULogRecord::init(ConfigItem *pcitem)
    {
        if (NULL == pcitem)	return false;
        if (!m_bRef && NULL != m_pbuf)
        {
            delete []m_pbuf;
            m_pbuf = NULL;
        }
        m_pcitem = pcitem;
        m_size = GetRcdSize(pcitem) + sizeof(time_t);;
        m_pbuf = new char[m_size];
        m_bRef = false;
        return true;
    }

    time_t &	DEULogRecord::RcdTime()
    {
        return *((time_t *)m_pbuf);
    }

    tm *		DEULogRecord::RcdTime() const
    {
        return localtime((time_t*)m_pbuf);
    }

    void *		DEULogRecord::GetVal(unsigned int Index)
    {
        if (Index >= m_pcitem->Format.size())	return NULL;
        if (NULL == m_pbuf)	return NULL;

        char *ptmp = m_pbuf + sizeof(time_t);
        for (unsigned i = 0; i < Index; i++)
        {
            ptmp += m_pcitem->Format[i].Size;
        }
        return ptmp;
    }
    void *		DEULogRecord::GetVal(const char *pName)
    {
        if (NULL == m_pbuf)	return NULL;

        char *ptmp = m_pbuf + sizeof(time_t);
        std::vector<FormatItem>::iterator p;
        for (p = m_pcitem->Format.begin(); p != m_pcitem->Format.end(); p++)
        {
            if (p->Name == pName)	return ptmp;
            ptmp += p->Size;
        }
        return NULL;
    }

    char *		DEULogRecord::StrVal(unsigned int Index)
    {
        return (char *)GetVal(Index);
    }
    char *		DEULogRecord::StrVal(const char *pName)
    {
        return (char *)GetVal(pName);
    }
    int &		DEULogRecord::IntVal(unsigned int Index)
    {
        return *((int *)GetVal(Index));
    }
    int &		DEULogRecord::IntVal(const char *pName)
    {
        return *((int *)GetVal(pName));
    }
    double &	DEULogRecord::DblVal(unsigned int Index)
    {
        return *((double *)GetVal(Index));
    }
    double &	DEULogRecord::DblVal(const char *pName)
    {
        return *((double *)GetVal(pName));
    }
    time_t &	DEULogRecord::TimeVal(unsigned int Index)
    {
        return *((time_t *)GetVal(Index));
    }
    time_t &	DEULogRecord::TimeVal(const char *pName)
    {
        return *((time_t *)GetVal(pName));
    }
    bool &		DEULogRecord::BoolVal(unsigned int Index)
    {
        return *((bool *)GetVal(Index));
    }
    bool &		DEULogRecord::BoolVal(const char *pName)
    {
        return *((bool *)GetVal(pName));
    }


    //////////////////////////////////////////////////////////////////////////////
    // DEULogFile
    DEULogFile::DEULogFile(const char *pfilename, ConfigItem *pcitem)
        : m_pf(NULL)
        , m_pcitem(NULL)
        , m_rcdsize(0)
        , m_count(0)
    {
        init(pfilename, pcitem);
    }

    DEULogFile::~DEULogFile()
    {
        clear();
    }

    unsigned DEULogFile::GetCount(void) const
    {
        return m_count;
    }

    bool DEULogFile::ReadLog(unsigned int index, DEULogRecord &logrcd)
    {
        DEULobLockUtil lockobj(&m_lock);
        if (NULL == m_pf || index >= m_count)	return false;
        if (logrcd.DataSize() != m_rcdsize)	{
            if (!logrcd.init(m_pcitem))	return false;
        }

        long pos = index * m_rcdsize;
        fseek(m_pf, pos, SEEK_SET);
        if (logrcd.IsRef())
        {
            char *pbuf = new char[m_rcdsize];
            if (NULL == pbuf)	return false;
            fread(pbuf, 1, m_rcdsize, m_pf);
            logrcd.SetData(pbuf);
            delete []pbuf;
        }
        else
        {
            fread(logrcd.GetData(), 1, m_rcdsize, m_pf);
        }
        return true;
    }

    bool DEULogFile::WriteLog(DEULogRecord &logrcd)
    {
        DEULobLockUtil lockobj(&m_lock);
        if (NULL == m_pf)	return false;
        if (logrcd.DataSize() != m_rcdsize)	return false;

        fseek(m_pf, 0, SEEK_END);
        time_t t = time(NULL);
        void *pdata = logrcd.GetData();
        *((time_t*)pdata) = t;
        fwrite(pdata, 1, m_rcdsize, m_pf);
        fflush(m_pf);
        m_count++;
        return true;
    }

    bool DEULogFile::init(const char *pfilename, ConfigItem *pcitem)
    {
        if (NULL == pfilename || NULL == pcitem)	return false;
        DEULobLockUtil lockobj(&m_lock);

        if (NULL != m_pf)	clear();
        FILE *ptmpf = NULL;

        //判断文件是否存在
        if (access(pfilename, 0) != 0) {	//不存在就创建一个
            ptmpf = fopen(pfilename, "wb+");
        } else {
            ptmpf = fopen(pfilename, "rb+");
        }
        if (NULL == ptmpf)	return false;

        int fsize = GetFileSize(ptmpf);
        if (fsize < 0)	return false;

        m_rcdsize = GetRcdSize(pcitem) + sizeof(time_t);
        if ((fsize % m_rcdsize) != 0)	return false;
        m_pcitem = pcitem;
        m_count = fsize / m_rcdsize;
        m_FileName = pfilename;
        m_pcitem = pcitem;
        m_pf = ptmpf;
        return true;
    }

    void DEULogFile::clear()
    {
        DEULobLockUtil lockobj(&m_lock);

        if (NULL == m_pf)	return;
        fclose(m_pf);
        m_pf = NULL;
        m_FileName = "";
        m_pcitem = NULL;
        m_rcdsize = 0;
        m_count = 0;
    }



    //////////////////////////////////////////////////////////////////////////////////////////
    // DEULogCategory
    DEULogCategory::DEULogCategory(DEULogConfig *pcfg, DEULogRoot *proot, const char *ptype)
    {
        init(pcfg, proot, ptype);
    }

    DEULogCategory::~DEULogCategory()
    {
        clear();
    }

    unsigned DEULogCategory::GetCount(void) const
    {
        long nsize = 0;
        std::vector<DEULogFile *>::const_iterator p = m_logfiles.begin();
        for (; p != m_logfiles.end(); p++)
        {
            nsize += (*p)->GetCount();
        }
        return nsize;
    }

    ConfigItem * DEULogCategory::GetCfgItem()
    {
        return &m_cfgitem;
    }

    CategoryItem * DEULogCategory::GetCatItem()
    {
        return &m_catitem;
    }

    bool DEULogCategory::ReadLog(unsigned int index, DEULogRecord &logrcd)
    {
        std::vector<DEULogFile *>::iterator p = m_logfiles.begin();
        for (; p != m_logfiles.end(); p++) {
            if (index < (*p)->GetCount()) {
                return (*p)->ReadLog(index, logrcd);
            }
            index -= (*p)->GetCount();
        }
        return false;
    }

    bool DEULogCategory::WriteLog(DEULogRecord &logrcd)
    {
        DEULogFile *plogfile = m_logfiles[m_logfiles.size()-1];
        if (m_logfiles[m_logfiles.size()-1]->GetCount() >= MAX_LOGNUM_IN_A_FILE) {
            char buf[32] = "";
            std::string filename = m_catitem.Dir;
            if (filename[filename.length()-1] != '\\') filename += "\\";
            filename += m_catitem.Type;
            sprintf(buf, "%d", m_logfiles.size());
            filename += buf;

            plogfile = new DEULogFile(filename.c_str(), &m_cfgitem);
            if (NULL == plogfile)	return false;
            m_logfiles.push_back(plogfile);
        }
        m_logfiles[m_logfiles.size()-1]->WriteLog(logrcd);
        return true;
    }

    void DEULogCategory::GetRange(tm &btime, tm &etime, int &bindex, int &eindex)
    {
        bindex = -1;
        eindex = -1;

        time_t bt = mktime(&btime);
        time_t et = mktime(&etime);

        long num = GetCount();
        if (num < 1) return;

        DEULogRecord tmprcd(&m_cfgitem);
        //找到bindex
        if (!ReadLog(0, tmprcd))	return ;
        if (tmprcd.RcdTime() >= bt)	bindex = 0;
        else {
            long tmpbidx = 0, tmpeidx = num-1;
            long tmpidx;
            while (true) {
                tmpidx = tmpbidx + (tmpeidx - tmpbidx + 1) / 2;
                if (!ReadLog(tmpidx, tmprcd))	break;
                if (tmprcd.RcdTime() >= bt) {
                    tmpeidx = tmpidx;
                    continue;
                } else {
                    tmpbidx = tmpidx;
                    continue;
                }
                if (tmpidx == tmpbidx) {
                    bindex = tmpeidx;
                    break;
                }
            }
        }
        //找到eindex
        if (!ReadLog(num-1, tmprcd))	return;
        if (tmprcd.RcdTime() <= et)	eindex = num-1;
        else {
            long tmpbidx = 0, tmpeidx = num-1;
            long tmpidx;
            while (true) {
                tmpidx = tmpbidx + (tmpeidx - tmpbidx + 1) / 2;
                if (!ReadLog(tmpidx, tmprcd))	break;
                if (tmprcd.RcdTime() <= et) {
                    tmpbidx = tmpidx;
                    continue;
                } else {
                    tmpeidx = tmpidx;
                    continue;
                }
                if (tmpidx == tmpbidx) {
                    bindex = tmpbidx;
                    break;
                }
            }
        }
    }


    bool DEULogCategory::init(DEULogConfig *pcfg, DEULogRoot *proot, const char *ptype)
    {
        if (NULL == pcfg || NULL == proot || NULL == ptype)	return false;

        m_plogcfg = pcfg;
        m_plogroot = proot;

        int num = m_plogcfg->Count();
        for (int i = 0; i < num; i++) {
            if ((*m_plogcfg)[i].TypeID == ptype) {
                m_cfgitem = (*m_plogcfg)[i];
                break;
            }
        }
        num = m_plogroot->Count();
        for (int i = 0; i < num; i++) {
            if ((*m_plogroot)[i].Type == ptype) {
                m_catitem = (*m_plogroot)[i];
                break;
            }
        }
        num = m_catitem.Files.size();
        std::string filename;
        DEULogFile *plogfile = NULL;
        for (int i = 0; i < num; i++)
        {
            filename = m_catitem.Dir;
            if (filename[filename.length()-1] != '\\') filename += "\\";
            filename += m_catitem.Files[i];
            plogfile = new DEULogFile(filename.c_str(), &m_cfgitem);
            if (NULL != plogfile)	m_logfiles.push_back(plogfile);
        }
        return true;
    }

    void DEULogCategory::clear()
    {
        m_cfgitem.TypeDesc = "";
        m_cfgitem.TypeID = "";
        m_cfgitem.Format.clear();
        m_catitem.Type = "";
        m_catitem.Dir = "";
        m_catitem.Files.clear();
        std::vector<DEULogFile *>::iterator p = m_logfiles.begin();
        for (; p != m_logfiles.end(); p++) {
            delete (*p);
        }
        m_logfiles.clear();
    }



    ////////////////////////////////////////////////////////////////////////////
    // DEULog
    DEULog::DEULog()
    {
    }

    DEULog::~DEULog()
    {
        clear();
    }
   
    bool DEULog::init(const char *pcfgfile)
    {
        if (NULL == pcfgfile)	return false;

        clear();
        m_cfgfile = pcfgfile;

        if (!m_logcfg.Load(pcfgfile))	return false;
        if (!m_logroot.Load(m_logcfg.GetRoot()))	return false;

        DEULogCategory *pcat = NULL;
        m_cats.resize(m_logcfg.Count(), pcat);
        return true;
    }

    void DEULog::clear()
    {
        if (m_cfgfile.length() > 0)
        {
            m_logcfg.Save(m_cfgfile.c_str());
            m_logroot.Save(m_logcfg.GetRoot());
        }
        m_cfgfile.clear();
        m_logcfg.clear();
        m_logroot.clear();

        std::vector<DEULogCategory *>::iterator p = m_cats.begin();
        for (; p != m_cats.end(); p++)
        {
            if (NULL != (*p))
                delete (*p);
        }
        m_cats.clear();
    }

    unsigned DEULog::Count(void) const
    {
        return m_cats.size();
    }

    bool DEULog::AddCategory(ConfigItem &item, const char *pDir)
    {
        if (NULL == pDir)	return false;
        DEULobLockUtil lockobj(&m_lock);

        //加cfg
        m_logcfg.AddItem(item);
        //加root
        CategoryItem catitem;
        std::string filepath;
        catitem.Dir = pDir;
        catitem.Type = item.TypeID;
        //	filepath = pDir;
        //	if (filepath[filepath.length() - 1] != '\\')	filepath += "\\";
        filepath += item.TypeID;
        filepath += "0";
        catitem.Files.push_back(filepath);
        m_logroot.AddItem(catitem);
        //加cat
        DEULogCategory *pcat = NULL;
        pcat = new DEULogCategory(&m_logcfg, &m_logroot, item.TypeID.c_str());
        if (NULL == pcat)	return false;
        m_cats.push_back(pcat);
        return true;
    }

    DEULogCategory * DEULog::GetCategory(unsigned int index)
    {
        DEULobLockUtil lockobj(&m_lock);

        if (index < 0 || index >= m_cats.size())
            return NULL;

        if (NULL == m_cats[index]) {
            m_cats[index] = new DEULogCategory(&m_logcfg, &m_logroot, m_logcfg[index].TypeID.c_str());
        }

        return m_cats[index];
    }

    DEULogCategory * DEULog::GetCategory(const char *ptype)
    {
        if (NULL == ptype)	return NULL;
        DEULobLockUtil lockobj(&m_lock);

        std::vector<DEULogCategory *>::iterator p = m_cats.begin();
        ConfigItem *pitem = NULL;
        for (; p != m_cats.end(); p++)
        {
            ConfigItem &citem = m_logcfg.GetConfigItem(ptype);
            //pitem = (*p)->GetCfgItem();

            if (NULL == &citem)	continue;
            if (citem.TypeID == ptype)	{
                if (NULL == *p) {
                    *p = new DEULogCategory(&m_logcfg, &m_logroot, ptype);
                }
                return *p;
            }
        }
        return NULL;
    }
}

