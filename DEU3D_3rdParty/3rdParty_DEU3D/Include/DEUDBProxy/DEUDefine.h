#ifndef DEUDB_DEFINE_H_02B9B62F_832F_455D_BD18_609071E088FE_INCLUDE
#define DEUDB_DEFINE_H_02B9B62F_832F_455D_BD18_609071E088FE_INCLUDE

//////////////////////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include <map>

#include <IDProvider/ID.h>
#if defined (WIN32) || defined (WIN64)
#include <Windows.h>
#include <process.h>
#else
#include <stdlib.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <uuid/uuid.h>

#include <string.h>
typedef sem_t* HANDLE;

#endif
//////////////////////////////////////////////////////////////////////////////

#define DEU_SHAREMEM_SIZE   68
#define DEU_CONNECT_STR		"----DEUSERVER----"
#define DEU_CONNECT_LEN      17

#define DEU_READ_DATA                0
#define DEU_UPDATE_DATA              1
#define DEU_WRITE_DATA               2
#define DEU_REMOVE_DATA              3 
#define DEU_IS_EXIST                 4
#define DEU_ALL_INDEX                5
#define DEU_GET_COUNT                6
#define DEU_INDEX_INRANGE            7
#define DEU_GET_VERSION              8
#define DEU_REPLACE_DATA             9
#define DEU_SET_CLEAR_FLAG           10
#define DEU_FAIL                     -1

#define DEU_REG_SHM                  1
#define DEU_UNREG_SHM                0

#define DYSEMNAME                    "DEUSEMNAME"

#ifdef WIN32
typedef unsigned __int64    UINT_64;
#else
typedef unsigned long long    UINT_64;
#endif

#endif //_DEUCLIENTDEFINE_H_
