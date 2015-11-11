#include "WindowsUtilities/PrivilegeEnabler.h"

//workaround for visual studio bug, atltrace.h doesn't know about _T -> fail
#ifdef _UNICODE
#define _T(x)      L ## x
#else /* _UNICODE */
#define _T(x)      x
#endif /* _UNICODE */

#include "atltrace.h"

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
