#ifndef WINDOWS_COMMON_BASE_H__
#define WINDOWS_COMMON_BASE_H__

#if defined(WINDOWS_COMMON_BASE_DLL_EXPORTS)
#define WINDOWS_COMMON_BASE_API __declspec (dllexport)
#else
#define WINDOWS_COMMON_BASE_API __declspec (dllimport)
#endif

namespace Windows
{
namespace Common
{
    class WINDOWS_COMMON_BASE_API Base
    {
    public:
        static bool DeleteFiles(const char* dir, bool force = false);

        static bool DeleteFiles(const wchar_t * dir, bool force = false);

        static bool DeleteDirRecursive(const char* startdir, bool removeRoot = true, bool force = false);

        static bool DeleteDirRecursive(const wchar_t * startdir, bool removeRoot = true, bool force = false);

        static bool DirExist(const char* dir);

        static bool DirExist(const wchar_t * dir);

        static bool FileExist(const char *file);

        static bool FileExist(const wchar_t * file);
    };

} // namespace Common
} // namespace Windows

#endif // #ifndef WINDOWS_COMMON_BASE_H__
