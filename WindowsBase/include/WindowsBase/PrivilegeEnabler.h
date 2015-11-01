#ifndef WINDOWS_UTILITIES_PRIVILEGE_ENABLER_H__
#define WINDOWS_UTILITIES_PRIVILEGE_ENABLER_H__

#if defined(WINDOWS_UTILITIES_PRIVILEGE_ENABLER_DLL_EXPORTS)
#define WINDOWS_UTILITIES_PRIVILEGE_ENABLER_API __declspec (dllexport)
#else
#define WINDOWS_UTILITIES_PRIVILEGE_ENABLER_API __declspec (dllimport)
#endif

#include "Windows.h"
#include <vector>
#include "Mutex.h"


namespace Windows
{
namespace Utilities
{
    class WINDOWS_UTILITIES_PRIVILEGE_ENABLER_API PrivilegeEnabler
    {
    public:
        static void Initialize(std::vector<::LPCTSTR> const & privileges);

        ~PrivilegeEnabler();
    private:
        static PrivilegeEnabler * _pInstance;
        static Windows::Threading::Mutex _mutex;

        PrivilegeEnabler(std::vector<::LPCTSTR> const & privileges);

        ::BOOL EnablePrivilege(::LPCTSTR pszPrivName, ::BOOL fEnable = TRUE);
    };

} // namespace Common
} // namespace Windows

#endif // #ifndef WINDOWS_UTILITIES_PRIVILEGE_ENABLER_H__
