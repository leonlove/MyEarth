#ifndef _DEUEXTERNALSERVICE_EXPORTS_
#define _DEUEXTERNALSERVICE_EXPORTS_ 1


#ifdef DEUEXTERNALSERVICE_EXPORTS
#define DEUES_EXPORT    __declspec(dllexport)
#else
#define DEUES_EXPORT    __declspec(dllimport)
#endif

#endif