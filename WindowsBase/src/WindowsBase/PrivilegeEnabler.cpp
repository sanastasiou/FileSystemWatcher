#include "WindowsBase/PrivilegeEnabler.h"
#include "atltrace.h"
#include <tchar.h>

namespace Windows
{
namespace Utilities
{
    PrivilegeEnabler * PrivilegeEnabler::_pInstance = nullptr;
    Threading::Mutex PrivilegeEnabler::_mutex;

    void PrivilegeEnabler::Initialize(std::vector<std::wstring> const & privileges)
    {
        if (_pInstance == nullptr)
        {
            _mutex.Lock();
            if (_pInstance == nullptr)
            {
                _pInstance = new PrivilegeEnabler(privileges);
            }
        }
    }
    
    PrivilegeEnabler::PrivilegeEnabler(std::vector<std::wstring> const & privileges)
    {
        for (auto i = privileges.begin(); i != privileges.end(); ++i)
        {
            if (EnablePrivilege((*i).c_str(), TRUE) == FALSE)
            {
                ATLTRACE(_T("Unable to enable privilege: %s    --    GetLastError(): %d\n"), (*i).c_str(), ::GetLastError());
                ATLTRACE(_T("File: %s Line: %d\n"), _T(__FILE__), __LINE__);
            }
        }
    }
    
    ::BOOL PrivilegeEnabler::EnablePrivilege(::LPCTSTR pszPrivName, ::BOOL fEnable /* = TRUE*/)
    {
        ::BOOL aResult = FALSE;
        // Assume function fails    
        ::HANDLE aProcessToken;
        // Try to open this process's access token    
        if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &aProcessToken))
        {
            // privilege
            ::TOKEN_PRIVILEGES aTokenPrivilege = { 1 };
            if (LookupPrivilegeValue(NULL, pszPrivName, &aTokenPrivilege.Privileges[0].Luid))
            {
                aTokenPrivilege.Privileges[0].Attributes = fEnable ? SE_PRIVILEGE_ENABLED : 0;
                AdjustTokenPrivileges(aProcessToken, FALSE, &aTokenPrivilege, sizeof(aTokenPrivilege), NULL, NULL);
                aResult = (::GetLastError() == ERROR_SUCCESS);
            }
            ::CloseHandle(aProcessToken);
        }
        return aResult;
    }

    PrivilegeEnabler::~PrivilegeEnabler()
    {
    }

} // namespace Common
} // namespace Windows
