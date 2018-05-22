#ifndef DEUDB_SEM_H_E4D02EFC_CA25_4734_AD3B_286D694D6379_INCLUDE
#define DEUDB_SEM_H_E4D02EFC_CA25_4734_AD3B_286D694D6379_INCLUDE

#include "DEUDefine.h"

namespace deudbProxy
{
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
}
#endif //_DEUSEM_H_

