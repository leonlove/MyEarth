#ifndef	__DEU_CHECK_H__
#define	__DEU_CHECK_H__

#ifdef	WIN32
#ifdef	DEULIC_EXPORTS
#define	_DEULIC_EXPORTS_  __declspec(dllexport)
#else
#define	_DEULIC_EXPORTS_  __declspec(dllimport)
#endif
#else
#define	_DEULIC_EXPORTS_
#endif

class _DEULIC_EXPORTS_ DEUCheck
{
public:
	DEUCheck(int PID);
	DEUCheck(int PID, int FID);

public:
	const unsigned char * PData(int PID, int &len);
};


#endif
