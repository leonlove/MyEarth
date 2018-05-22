// DEULog.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "CDEULog.h"
#include "iostream"
using namespace std;

//config file format
//{"types":[{"typeid":"1","desc":"aaaaa"},{},{}...]}
//log root format
//{"category":[{"type":"1","files":["file1","file2"...]},{},{}....]}
//log file format
//line 1 {"type":"1","count":"100"}
//line 2 3 .... {"logid":"aaa","logcontent":{.....}}
//one log file maxcount is 500'000

#include "CDEULog.h"

int main(int argc, char* argv[])
{
	/*
	DEULogConfig logconfig("c:\\root");
	struct ConfigItem item;
	FormatItem fitem;
	item.TypeID = "1";
	fitem.Name = "f1";
	fitem.Type = INTEGER_TYPE;
	fitem.Size = 4;
	item.Format.push_back(fitem);
	fitem.Name = "f2";
	fitem.Type = INTEGER_TYPE;
	fitem.Size = 4;
	item.Format.push_back(fitem);
	logconfig.AddItem(item);
	item.TypeID = "6";
	logconfig.AddItem(item);
	item.TypeID = "2";
	logconfig.AddItem(item);
	item.TypeID = "3";
	logconfig.AddItem(item);
	item.TypeID = "4";
	logconfig.AddItem(item);
	item.TypeID = "5";
	logconfig.AddItem(item);
	if (logconfig.Count() != 6)	{
		printf("Error 1\r\n");
		return 0;
	}
	logconfig.Save("c:\\config");
	logconfig.Load("c:\\config");
	logconfig.Save("c:\\config1");

	CategoryItem item1;
	DEULogRoot LogRoot;
	item1.Type = "1";
	item1.Dir = "c:\\";
	item1.Files.push_back("10");
//	item1.Files.push_back("11");
//	item1.Files.push_back("12");
//	item1.Files.push_back("13");
	LogRoot.AddItem(item1);
	item1.Files.clear();
	item1.Type = "2";
	item1.Files.push_back("c:/11");
	item1.Files.push_back("c:/12");
	item1.Files.push_back("c:/13");
	item1.Files.push_back("c:/14");
	LogRoot.AddItem(item1);
	if (LogRoot.Count() != 2)	{
		printf("Error 2\r\n");
		return 0;
	}
	LogRoot.Save("c:\\root");
	LogRoot.Load("c:\\root");
	LogRoot.Save("c:\\root1");

	DEULogRecord logrcd(&item);
	cout << "TypeID is : " << logrcd.TypeID() << endl;
	cout << "TypeDesc is : " << logrcd.TypeDesc() << endl;
	cout << "FldCount is : " << logrcd.FldCount() << endl;
	int fldcount = logrcd.FldCount();
	DEULogFld *pfld = NULL;
	for (int i = 0; i < fldcount; i++)
	{
		pfld = logrcd.GetFld(i);
		cout << "Field " << i << ", Name is " << pfld->Name << ", size is " << pfld->Size << endl;
	}
	pfld = logrcd.GetFld("f1");
	cout << "Field f1 " << ", Name is " << pfld->Name << ", size is " << pfld->Size << endl;
	pfld = logrcd.GetFld("f2");
	cout << "Field f1 " << ", Name is " << pfld->Name << ", size is " << pfld->Size << endl;
	
	logrcd.IntVal(0) = 1;
	logrcd.IntVal(1) = 100;
	cout << "Field 0, value is: " << logrcd.IntVal(0) << endl;
	cout << "Field 1, value is: " << logrcd.IntVal(1) << endl;

	DEULogFile logfile("c:/t1.log", &item);
	cout << "log number is : " << logfile.GetCount() << endl;
	DWORD dtime = ::GetTickCount();
	for (int i = 0; i < 1000; i++)
	{
		logfile.WriteLog(logrcd);
	}
	dtime = ::GetTickCount() - dtime;
	cout << "log number is : " << logfile.GetCount() << endl;
	cout << "time is : " << dtime << endl;
	logfile.ReadLog(0, logrcd);
	if (NULL != logrcd.GetVal(0))
		cout << "read rcd, f1 is : " << logrcd.IntVal(0) << endl;
	if (NULL != logrcd.GetVal(1))
		cout << "read rcd, f2 is : " << logrcd.IntVal(1) << endl;
	if (NULL != logrcd.GetVal(2))
		cout << "read rcd, f3 is : " << logrcd.IntVal(2) << endl;

	//测试logcategory
	DEULogCategory logcat(&logconfig, &LogRoot, "1");
	dtime = ::GetTickCount();
	for (int i = 0; i < 5000; i++)
	{
		logcat.WriteLog(logrcd);
	}
	dtime = ::GetTickCount() - dtime;
	cout << "log number is : " << logcat.GetCount() << endl;
	cout << "time is : " << dtime << endl;
	cout << "log number is : " << logcat.GetCount() << endl;
	logcat.ReadLog(300, logrcd);
	if (NULL != logrcd.GetVal(0))
		cout << "read rcd, f1 is : " << logrcd.IntVal(0) << endl;
	if (NULL != logrcd.GetVal(1))
		cout << "read rcd, f2 is : " << logrcd.IntVal(1) << endl;
	if (NULL != logrcd.GetVal(2))
		cout << "read rcd, f3 is : " << logrcd.IntVal(2) << endl;
	time_t t = logrcd.RcdTime();
	cout << "rcd 300 ,rcdtime is : " << ctime(&t) << endl;
	logcat.ReadLog(5300, logrcd);
	if (NULL != logrcd.GetVal(0))
		cout << "read rcd, f1 is : " << logrcd.IntVal(0) << endl;
	if (NULL != logrcd.GetVal(1))
		cout << "read rcd, f2 is : " << logrcd.IntVal(1) << endl;
	if (NULL != logrcd.GetVal(2))
		cout << "read rcd, f3 is : " << logrcd.IntVal(2) << endl;
	t = logrcd.RcdTime();
	cout << "rcd 5300 ,rcdtime is : " << ctime(&t) << endl;

	tm t1, t2;
	t = time(NULL);
	tm *ptm = localtime(&t);
	t1 = *ptm;
	t2 = *ptm;
	t1.tm_hour--;

	int bidx, eidx;
	logcat.GetRange(t1, t2, bidx, eidx);
	cout << "begin index is : " << bidx << endl;
	cout << "end index is : " << eidx << endl;
	*/

    deulog::DEULog ll;
    if(ll.initPath("c:\\ttt\\deucfg","c:\\ttt\\deuroot","c:\\ttt"))
    {
        deulog::DEULogConfig* pCfg = ll.GetLogCfg();
        deulog::ConfigItem item11 = pCfg->GetConfigItem("DEUDataLog");
        deulog::DEULogCategory* pcat11 = ll.GetCategory("DEUDataLog");

        DWORD dtime = ::GetTickCount();
        for (int i = 0; i < 5000; i++)
        {
            deulog::DEULogRecord rcd(item11);
            rcd.IntVal("")
            pcat11->WriteLog(rcd);
        }
    }


	deulog::ConfigItem item;
	deulog::FormatItem fitem;
	item.TypeID = "1";
	fitem.Name = "f1";
	fitem.Type = INTEGER_TYPE;
	fitem.Size = 4;
	item.Format.push_back(fitem);
	fitem.Name = "f2";
	fitem.Type = INTEGER_TYPE;
	fitem.Size = 4;
	item.Format.push_back(fitem);

	deulog::DEULogRecord logrcd(&item);
	cout << "TypeID is : " << logrcd.TypeID() << endl;
	cout << "TypeDesc is : " << logrcd.TypeDesc() << endl;
	cout << "FldCount is : " << logrcd.FldCount() << endl;
	int fldcount = logrcd.FldCount();
	deulog::DEULogFld *pfld = NULL;
	for (int i = 0; i < fldcount; i++)
	{
		pfld = logrcd.GetFld(i);
		cout << "Field " << i << ", Name is " << pfld->Name << ", size is " << pfld->Size << endl;
	}
	pfld = logrcd.GetFld("f1");
	cout << "Field f1 " << ", Name is " << pfld->Name << ", size is " << pfld->Size << endl;
	pfld = logrcd.GetFld("f2");
	cout << "Field f1 " << ", Name is " << pfld->Name << ", size is " << pfld->Size << endl;
	
	logrcd.IntVal(0) = 1;
	logrcd.IntVal(1) = 100;
	cout << "Field 0, value is: " << logrcd.IntVal(0) << endl;
	cout << "Field 1, value is: " << logrcd.IntVal(1) << endl;

	deulog::DEULog logobj;
	logobj.init("c:\\deulogcfg");

  /*  logobj.GetLogCfg()->SetRoot("c:\\deulogroot");
    DEULogCategory *pcat = logobj.GetCategory(0);
    long nCount = pcat->GetCount();
    for(unsigned n = 0;n < nCount;n++)
    {
        ConfigItem* pp = pcat->GetCfgItem();
        DEULogRecord logrcd1(pp);
        pcat->ReadLog(n,logrcd1);
        unsigned nFC = logrcd1.FldCount();
        unsigned nn = logrcd1.IntVal(0);
        unsigned mm = logrcd1.IntVal(1);
    }*/
    logobj.GetLogCfg()->SetRoot("c:\\deulogroot");
    logobj.AddCategory(item, "c:\\");
    deulog::DEULogCategory *pcat = logobj.GetCategory(0);

    DWORD dtime = ::GetTickCount();
    for (int i = 0; i < 5000; i++)
    {
        pcat->WriteLog(logrcd);
    }
    dtime = ::GetTickCount() - dtime;
    cout << "log number is : " << pcat->GetCount() << endl;
    cout << "time is : " << dtime << endl;

	return 0;
}

