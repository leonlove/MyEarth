#ifndef	__C_DEU_LOG_H__
#define	__C_DEU_LOG_H__

#include "windows.h"

#include "vector"
#include "string"
#include "time.h"
#include "Export.h"

#pragma warning( disable : 4251 )
namespace deulog
{
    //config file format
    //{"root":"rootfile", "types":[{"typeid":"1","desc":"aaaaa","format":[{"name":"f1","len":8,"type":1},...]},{},{}...]}
    //log root format
    //{"category":[{"type":"1","files":["file1","file2"...]},{},{}....]}
    //log file format
    //log item list, each item has same size;
    //one log file maxcount is 500'000
    //category file format
    //

#define	MAX_LOGNUM_IN_A_FILE 200000

#define	STRING_TYPE 1	//user define size
#define	INTEGER_TYPE 2	//size is 4
#define	FLOAT_TYPE 3	//size is 8
#define	BOOL_TYPE 4		//size is 1
#define	TIME_TYPE 5		//size is 8
#define	UNKNOWN_TYPE 0	//size is 0

    struct  FormatItem{
        std::string Name;
        int Type;
        int Size;
    } ;

    typedef FormatItem DEULogFld;

    struct  ConfigItem{
        std::string TypeID;
        std::string TypeDesc;
        std::vector<FormatItem> Format;
    };

    class DEULOG_EXPORT DEULogLock
    {
    public:
        DEULogLock();
        ~DEULogLock();

    public:
        void init();

    public:
        void lock();
        void unlock();

    private:
        CRITICAL_SECTION	m_cs;
    };

    class DEULOG_EXPORT DEULobLockUtil
    {
    public:
        DEULobLockUtil(DEULogLock *plock)
        {
            m_plock = plock;
            if (NULL != m_plock) m_plock->lock();
        }
        ~DEULobLockUtil()
        {
            if (NULL != m_plock)	m_plock->unlock();
        }
    private:
        DEULogLock *m_plock;
    };

    class DEULOG_EXPORT DEULogConfig
    {
    public:
        DEULogConfig(const char *pRootFileName = NULL);
        ~DEULogConfig();

    public:
        unsigned int Count(void) const;
        ConfigItem & operator[](unsigned int index);
        ConfigItem & GetConfigItem(const char *ptype);
        void AddItem(const ConfigItem &item);
        void InsItem(unsigned int Index, const ConfigItem &item);
        const char * GetRoot();
        void SetRoot(const char *pRoot);

    public:
        bool Save(const char *pConfigFileName);
        bool Load(const char *pConfigFileName);

        void clear();

    private:
        std::vector<ConfigItem> m_TypeList;
        std::string m_RootFileName;
    };

    struct CategoryItem{
        std::string Type;
        std::string Dir;
        std::vector<std::string> Files;
    };

    class DEULOG_EXPORT DEULogRoot
    {
    public:
        DEULogRoot();
        ~DEULogRoot();

    public:
        unsigned int Count(void) const;
        CategoryItem & operator[](unsigned int index);
        CategoryItem & GetItem(const char *ptype);
        void AddItem(const CategoryItem &item);
        void InsItem(unsigned int Index, const CategoryItem &item);

    public:
        bool Save(const char *pRootFileName);
        bool Load(const char *pRootFileName);

        void clear();

    private:
        std::vector<CategoryItem> m_CategoryItems;
    };

    class DEULOG_EXPORT DEULogRecord
    {
    public:
        DEULogRecord(ConfigItem *pcitem);
        ~DEULogRecord();

    public:
        const std::string &TypeID(void) const;
        const std::string &TypeDesc(void) const;
        unsigned		FldCount(void) const;
        DEULogFld *	GetFld(unsigned index);
        DEULogFld * GetFld(const char *pName);

    public:
        void		SetData(char *pBuf);
        void		SetDataRef(char *pBuf);
        void *		GetData();
        long		DataSize() const {return m_size;}
        bool		IsRef() const {return m_bRef;}

    public:
        bool		init(ConfigItem *pcitem);

    public:
        time_t &	RcdTime();
        tm *		RcdTime() const;

        void *		GetVal(unsigned int Index);
        void *		GetVal(const char *pName);

        char *		StrVal(unsigned int Index);
        char *		StrVal(const char *pName);
        int &		IntVal(unsigned int Index);
        int &		IntVal(const char *pName);
        double &	DblVal(unsigned int Index);
        double &	DblVal(const char *pName);
        time_t &	TimeVal(unsigned int Index);
        time_t &	TimeVal(const char *pName);
        bool &		BoolVal(unsigned int Index);
        bool &		BoolVal(const char *pName);

    private:
        ConfigItem	*m_pcitem;
        time_t		m_rcdtime;
        unsigned	m_size;
        char		*m_pbuf;
        bool		m_bRef;
    };

    //log file format
    //log item list, each item has same size;
    class DEULOG_EXPORT DEULogFile
    {
    public:
        DEULogFile(const char *pfilename, ConfigItem *pcitem);
        ~DEULogFile(void);

    public:
        unsigned GetCount(void) const;
        bool ReadLog(unsigned int index, DEULogRecord &logrcd);
        bool WriteLog(DEULogRecord &logrcd);

    public:
        bool init(const char *pfilename, ConfigItem *pcitem);

    private:
        void clear();

    private:
        std::string	m_FileName;
        ConfigItem	*m_pcitem;
        FILE		*m_pf;
        unsigned	m_rcdsize;
        unsigned	m_count;
        DEULogLock	m_lock;
    };

    //{"category":[{"type":"1","files":["file1","file2"...]},{},{}....]}
    class DEULOG_EXPORT DEULogCategory
    {
    public:
        DEULogCategory(DEULogConfig *pcfg, DEULogRoot *proot, const char *ptype);
        ~DEULogCategory();

    public:
        unsigned GetCount(void) const;
        ConfigItem * GetCfgItem(void);
        CategoryItem * GetCatItem(void);
        bool ReadLog(unsigned int index, DEULogRecord &logrcd);
        bool WriteLog(DEULogRecord &logrcd);

    public:
        //get index range by time range
        void GetRange(tm &btime, tm &etime, int &bindex, int &eindex);

    public:
        bool init(DEULogConfig *pcfg, DEULogRoot *proot, const char *ptype);
        void clear();

    private:
        DEULogConfig	*m_plogcfg;
        DEULogRoot		*m_plogroot;
        ConfigItem		m_cfgitem;
        CategoryItem	m_catitem;
        std::vector<DEULogFile *>	m_logfiles;
    };

    class DEULOG_EXPORT DEULog
    {
    public:
        DEULog();
        ~DEULog();

    public:
        bool init(const char *pcfgfile);
        void clear();

        unsigned int Count(void) const;
        DEULogConfig *	GetLogCfg()		{return &m_logcfg; }
        DEULogRoot *	GetLogRoot()	{return &m_logroot;}

    public:
        bool AddCategory(ConfigItem &item, const char *pDir);
        DEULogCategory * GetCategory(unsigned int index);
        DEULogCategory * GetCategory(const char *ptype);

    private:
        std::string		    m_cfgfile;
        DEULogConfig	    m_logcfg;
        DEULogRoot		    m_logroot;
        std::vector<DEULogCategory *>	m_cats;
        DEULogLock		    m_lock;

    };
}

#endif //__C_DEU_LOG_H__
