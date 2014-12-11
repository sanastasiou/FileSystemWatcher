#pragma once

#include <afxmt.h>
#include <afxtempl.h>

// Helper functions
static BOOL	EnablePrivilege(LPCTSTR pszPrivName, BOOL fEnable = TRUE)
{    
    BOOL fOk = FALSE;    
    // Assume function fails    
    HANDLE hToken;    
    // Try to open this process's access token    
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
    {        
        // privilege        
        TOKEN_PRIVILEGES tp = { 1 };        

        if( LookupPrivilegeValue(NULL, pszPrivName,  &tp.Privileges[0].Luid) )
        {
            tp.Privileges[0].Attributes = fEnable ?  SE_PRIVILEGE_ENABLED : 0;

            AdjustTokenPrivileges(hToken, FALSE, &tp, 			      
                sizeof(tp), NULL, NULL);

            fOk = (GetLastError() == ERROR_SUCCESS);		
        }
        CloseHandle(hToken);	
    }	
    return(fOk);
}

static inline bool HasTrailingBackslash(const CString & str )
{
	if( str.GetLength() > 0 
	&&	str[ str.GetLength() - 1 ] == _T('\\') )
		return true;
	return false;
}

static bool IsDirectory(const CString & strPath)
{
    DWORD dwAttrib	= GetFileAttributes( strPath );
    return static_cast<bool>( ( dwAttrib != 0xffffffff 
        &&	(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) );
}

static inline bool IsEmptyString(LPCTSTR sz)
{
	return (bool)(sz==NULL || *sz == 0);
}