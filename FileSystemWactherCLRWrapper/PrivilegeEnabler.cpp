#include "stdafx.h"
#include "PrivilegeEnabler.h"
#include <afxmt.h>
#include <afxtempl.h>
#include "Utilities.h"

CPrivilegeEnabler::CPrivilegeEnabler()
{
	LPCTSTR arPrivelegeNames[]	=	{
										SE_BACKUP_NAME, //	these two are required for the FILE_FLAG_BACKUP_SEMANTICS flag used in the call to 
										SE_RESTORE_NAME,//  CreateFile() to open the directory handle for ReadDirectoryChangesW

										SE_CHANGE_NOTIFY_NAME //just to make sure...it's on by default for all users.
										//<others here as needed>
									};
	for(int i = 0; i < sizeof(arPrivelegeNames) / sizeof(arPrivelegeNames[0]); ++i)
	{
		if( !EnablePrivilege(arPrivelegeNames[i], TRUE) )
		{
			TRACE(_T("Unable to enable privilege: %s	--	GetLastError(): %d\n"), arPrivelegeNames[i], GetLastError());
			TRACE(_T("CDirectoryChangeWatcher notifications may not work as intended due to insufficient access rights/process privileges.\n"));
			TRACE(_T("File: %s Line: %d\n"), _T(__FILE__), __LINE__);
		}
	}
}

CPrivilegeEnabler & CPrivilegeEnabler::Instance()
{
	static CPrivilegeEnabler theInstance;//constructs this first time it's called.
	return theInstance;
}
