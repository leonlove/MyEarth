#ifndef _DEULOG_EXPORT_H_
#define _DEULOG_EXPORT_H_ 1

#ifdef WIN32
#ifdef DEULOG_EXPORTS
    #define DEULOG_EXPORT    __declspec(dllexport)
#else
    #define DEULOG_EXPORT    __declspec(dllimport)
#endif
#else
    #define DEULOG_EXPORT
#endif

#endif //_DEULOG_EXPORT_H_
