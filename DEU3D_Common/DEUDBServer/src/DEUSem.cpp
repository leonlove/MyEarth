#include "DEUSem.h"


DEUSem::DEUSem(void)
{
}


DEUSem::~DEUSem(void)
{
}

//create semaphore
HANDLE DEUSem::CreateSem(const std::string& strSemName,unsigned int nInitValue)
{
	HANDLE hnd = NULL;
#ifdef WIN32
	hnd = CreateSemaphore(NULL,nInitValue,1,strSemName.c_str());
#else
	int nMask = umask(0);
	hnd = sem_open(strSemName.c_str(), O_RDWR|O_CREAT, 0666, nInitValue);
	umask(nMask);
#endif
	return hnd;
}

//open semaphore
HANDLE DEUSem::OpenSem(const std::string& strSemName)
{
	HANDLE hnd = NULL;
#ifdef WIN32
	hnd = OpenSemaphore(SEMAPHORE_ALL_ACCESS,FALSE,strSemName.c_str());
#else
	hnd = sem_open(strSemName.c_str(), O_RDWR|O_CREAT);
#endif
	return hnd;
}

//release semaphore
void DEUSem::ReleaseSem(HANDLE hnd)
{
#ifdef WIN32
	ReleaseSemaphore(hnd,1,NULL);
#else
	sem_post(hnd);
#endif
}

//wait semaphore
void DEUSem::WaitSem(HANDLE hnd)
{
#ifdef WIN32
	WaitForSingleObject(hnd,INFINITE);
#else
	sem_wait(hnd);
#endif
}

//close semaphore
void DEUSem::CloseSem(HANDLE hnd,const std::string& strSemName)
{
#ifdef WIN32
	CloseHandle(hnd);
#else
	sem_close(hnd);
	if(strSemName != "")
		sem_unlink(strSemName.c_str());
#endif
}
