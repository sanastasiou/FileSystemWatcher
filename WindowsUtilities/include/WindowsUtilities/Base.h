#ifndef WINDOWS_COMMON_BASE_H__
#define WINDOWS_COMMON_BASE_H__

#if defined(WINDOWS_COMMON_BASE_DLL_EXPORTS)
#define WINDOWS_COMMON_BASE_API __declspec (dllexport)
#else
#define WINDOWS_COMMON_BASE_API __declspec (dllimport)
#endif

#include "Windows.h"
#include <string>

namespace Windows
{
namespace Common
{
    class WINDOWS_COMMON_BASE_API Base
    {
    public:
        static void GetLastErrorStr(std::wstring & error);

        static void GetLastErrorStr(std::string & error);

        static bool FlushFileBuffers(const wchar_t * drive);

        static bool RemoveFileAttribute(const char * fileName, ::DWORD const attribute, bool force = false);

        static bool RemoveFileAttribute(const wchar_t * fileName, ::DWORD const attribute, bool force = false);

        static bool FileRename(const char * oldFileName, const char * newFileName, bool force = false);

        static bool FileRename(const wchar_t * oldFileName, const wchar_t * newFileName, bool force = false);

        static bool FileDelete(const char* filePath, bool force = false);

        static bool FileDelete(const wchar_t* filePath, bool force = false);

        static bool DeleteFiles(const char* dir, bool force = false);

        static bool DeleteFiles(const wchar_t * dir, bool force = false);

        static bool DeleteDirRecursive(const char* startdir, bool removeRoot = true, bool force = false);

        static bool DeleteDirRecursive(const wchar_t * startdir, bool removeRoot = true, bool force = false);

        static bool DirExist(const char* dir);

        static bool DirExist(const wchar_t * dir);

        static bool FileExist(const char *file);

        static bool FileExist(const wchar_t * file);

        static bool MakeDir(const char* dir, LPSECURITY_ATTRIBUTES attributes = nullptr);

        static bool MakeDir(const wchar_t * dir, LPSECURITY_ATTRIBUTES attributes = nullptr);
    };

} // namespace Common
} // namespace Windows

#endif // #ifndef WINDOWS_COMMON_BASE_H__
