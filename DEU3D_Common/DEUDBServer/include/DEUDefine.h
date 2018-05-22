#ifndef _DEUDEFINE_H_
#define _DEUDEFINE_H_

//////////////////////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include <map>

#include <IDProvider/ID.h>
#if defined (WIN32) || defined (WIN64)
#include <Windows.h>
#include <process.h>
#else

#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>

typedef sem_t* HANDLE;

#endif
//////////////////////////////////////////////////////////////////////////////

#define DEUSHMNAME  "DEUDBSVRSHM"
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
#define DEU_END                      -2
#define DEU_REG_SHM                  1
#define DEU_UNREG_SHM                0
#define DEU_READ_BUF                 134217728u
#define DEU_WRITE_BUF                10u


#define DEU_REG_SHM                  1
#define DEU_UNREG_SHM                0
#define DEU_ALREADY_EXIST            2
#define DEU_REG_SUCCESS              3
#define DEU_UNREG_SUCCESS            4
#define DEU_ERROR_TYPE               5
//////////////////////////////////////////////////////////////////////////////

#endif //_DEUDEFINE_H_
