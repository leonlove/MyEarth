#ifndef DEUNETWORK_EXPORTS_
#define DEUNETWORK_EXPORTS_ 1


#ifdef DEUNETWORK_EXPORTS
#define DEUNW_EXPORT    __declspec(dllexport)
#else
#define DEUNW_EXPORT    __declspec(dllimport)
#endif

#endif