//////////////////////////////////////////////////////////////////////
//
// DYCommon.cpp: implementation of global definition for the tidu classes.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DYCommon.h"
#include "DYStringList.h"

VOID CreateFolder(const CHAR *szDirectory)
{
	assert(szDirectory!=NULL);

	WIN32_FIND_DATA dataFind;

	HANDLE hFind=FindFirstFile(szDirectory,&dataFind);

	if(hFind==INVALID_HANDLE_VALUE)
	{
		CreateDirectory(szDirectory,NULL);
		SetFileAttributes(szDirectory, FILE_ATTRIBUTE_HIDDEN);
	}

	DY_CLOSE_FIND(hFind);
}

BOOL EnumFiles(const CHAR *szRootDir,DYStringList &pstrFullPaths)
{
	assert(szRootDir!=NULL && (INT)(strlen(szRootDir))>0);

	CHAR szRootDir2[DY_MAX_PATH];
	CHAR szFullPath[DY_MAX_PATH];

	if(szRootDir[(INT)(strlen(szRootDir))-1]!='\\')
	{
		sprintf(szRootDir2,DY_TEXT("%s\\"),szRootDir);
	}
	else
	{
		strcpy(szRootDir2,szRootDir);
	}

	sprintf(szFullPath,DY_TEXT("%s*.*"),szRootDir2);

	WIN32_FIND_DATA aFindData;

	HANDLE hFind=FindFirstFile(szFullPath,&aFindData);

	if(hFind==INVALID_HANDLE_VALUE)
	{
		pstrFullPaths.Append(DYString(szRootDir));

		DY_CLOSE_FIND(hFind);

		return TRUE;
	}

	while(hFind!=INVALID_HANDLE_VALUE)
	{
		if(!FindNextFile(hFind,&aFindData))
		{
			break;
		}

		if(strcmp(aFindData.cFileName,DY_TEXT(".."))!=0)
		{
			if(aFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				sprintf(szFullPath,DY_TEXT("%s%s\\"),szRootDir2,aFindData.cFileName);

				if(!EnumFiles(szFullPath,pstrFullPaths))
				{
					DY_CLOSE_FIND(hFind);

					return FALSE;
				}
			}
			else
			{
				sprintf(szFullPath,DY_TEXT("%s%s"),szRootDir2,aFindData.cFileName);

				pstrFullPaths.Append(DYString(szFullPath));
			}
		}
	}

	DY_CLOSE_FIND(hFind);

	return TRUE;
}

INT Shift(INT nNumber)
{
	INT nShift=0;

	BOOL bShift=FALSE;

	while(nNumber>=2)
	{
		if(nNumber & 0x1)
		{
			bShift=TRUE;
		}

		nNumber=(nNumber>>1);

		nShift++;
	}

	if(bShift)
	{
		nShift++;
	}

	return nShift;
}

INT Normalize(const INT nNumber)
{
	return (1<<Shift(nNumber));
}
