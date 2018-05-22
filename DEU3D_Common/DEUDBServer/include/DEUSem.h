#ifndef _DEUSEM_H_
#define _DEUSEM_H_

#include "DEUDefine.h"

class DEUSem
{
public:
	DEUSem(void);
	~DEUSem(void);
public:
	//create semaphore
	static HANDLE CreateSem(const std::string& strSemName,unsigned int nInitValue);
	//open semaphore
	static HANDLE OpenSem(const std::string& strSemName);
	//release semaphore
	static void ReleaseSem(HANDLE hnd);
	//wait semaphore
	static void WaitSem(HANDLE hnd);
	//close semaphore
	static void CloseSem(HANDLE hnd,const std::string& strSemName);

};
#endif //_DEUSEM_H_

