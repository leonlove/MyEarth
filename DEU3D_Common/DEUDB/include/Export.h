#ifndef FILECACHE_EXPORT_
#define FILECACHE_EXPORT_ 1

#ifdef WIN32
#ifdef FILECACHE_EXPORTS
    #define DEUDB_EXPORT      __declspec(dllexport)
#else
    #define DEUDB_EXPORT      __declspec(dllimport)
#endif
#else
    #define DEUDB_EXPORT  
    #define IPV_EXPORT   
#endif


#ifndef WIN32
#include<string>
#include<string.h>
#include<limits.h>
#include<stdio.h>
#endif

#endif
