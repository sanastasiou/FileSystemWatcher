#include "WindowsBase/Base.h"
#include "Windows.h"
#include <string>
#include "WindowsBase/File.h"

namespace Windows
{
namespace Common
{
    bool Base::DeleteFiles(const char* dir, bool force /* = false */)
    {
        if (!DirExist(dir)) return false;

        ::WIN32_FIND_DATAA fileinfo;
        ::HANDLE aFileHandle = NULL;
        std::string aSourcePath(dir);
        aSourcePath += "/*.*";
        if ((aFileHandle = ::FindFirstFileA(aSourcePath.c_str(), &fileinfo)) == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        do
        {
            if (fileinfo.cFileName[0] != '.')
            {
                if (!(fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    aSourcePath = std::string(dir) + std::string("/") + std::string(fileinfo.cFileName);

                    if (force)
                    {
                        if (fileinfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                        {
                            ::SetFileAttributesA(aSourcePath.c_str(), (::GetFileAttributesA(aSourcePath.c_str()) & ~FILE_ATTRIBUTE_READONLY));
                        }
                    }
                    ::DeleteFileA(aSourcePath.c_str());
                }
            }
        } while (::FindNextFileA(aFileHandle, &fileinfo));

        ::FindClose(aFileHandle);

        return true;
    }

    bool Base::DeleteFiles(const wchar_t * dir, bool force /* = false */)
    {
        if (!DirExist(dir)) return false;

        ::WIN32_FIND_DATAW fileinfo;
        ::HANDLE aFileHandle = NULL;
        std::wstring aSourcePath(dir);
        aSourcePath += L"/*.*";

        if ((aFileHandle = ::FindFirstFileW(aSourcePath.c_str(), &fileinfo)) == INVALID_HANDLE_VALUE) return false;
        do
        {
            if (fileinfo.cFileName[0] != '.')
            {
                if (!(fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))    // It's a file and not a directory
                {
                    aSourcePath = std::wstring(dir) + std::wstring(L"/") + std::wstring(fileinfo.cFileName);
                    if (force)
                    {
                        if (fileinfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                        {
                            ::SetFileAttributesW(aSourcePath.c_str(), (::GetFileAttributesW(aSourcePath.c_str()) & ~FILE_ATTRIBUTE_READONLY));
                        }
                    }
                    ::DeleteFileW(aSourcePath.c_str());
                }
            }
        } while (::FindNextFileW(aFileHandle, &fileinfo));

        ::FindClose(aFileHandle);

        return true;
    }

    bool Base::DeleteDirRecursive(const char* dir, bool root /* = true */, bool force /* = false */)
    {
        if (FileExist(dir))
        {
            return (::DeleteFileA(dir) ? true : false);
        }

        if (!DirExist(dir)) return false;

        ::HANDLE aFileHandle = NULL;
        ::WIN32_FIND_DATAA fileinfo;
        bool aSuccess = true;
        std::string aRoot(dir);
        aRoot += "/*.*";
        if ((aFileHandle = ::FindFirstFileA(aRoot.c_str(), &fileinfo)) == INVALID_HANDLE_VALUE) return false;

        do
        {
            if (fileinfo.cFileName[0] != '.')
            {
                aRoot = std::string(dir) + std::string("/") + std::string(fileinfo.cFileName);
                if (fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (!DeleteDirRecursive(aRoot.c_str(), true, force))
                        aSuccess = false;
                }
                else
                {
                    if (force)
                    {
                        if (fileinfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                        {
                            ::SetFileAttributesA(aRoot.c_str(), (::GetFileAttributesA(aRoot.c_str()) & ~FILE_ATTRIBUTE_READONLY));
                        }
                    }
                    if (!::DeleteFileA(aRoot.c_str()))
                    {
                        aSuccess = false;
                    }
                }
            }
        } while (::FindNextFileA(aFileHandle, &fileinfo));
        ::FindClose(aFileHandle);

        if (root && aSuccess)
        {
            if (!RemoveDirectoryA(dir))
            {
                return false;
            }
        }

        return aSuccess;
    }

    bool Base::DeleteDirRecursive(const wchar_t * dir, bool root /* = true */, bool force /* = false */)
    {
        if (FileExist(dir))
        {
            return (::DeleteFileW(dir) ? true : false);
        }

        if (!DirExist(dir))
        {
            return false;
        }

        ::HANDLE aFileHandle = NULL;
        ::WIN32_FIND_DATAW fileinfo;
        bool aSuccess = true;
        std::wstring aRoot(dir);
        aRoot += L"/*.*";

        if ((aFileHandle = FindFirstFileW(aRoot.c_str(), &fileinfo)) == INVALID_HANDLE_VALUE) return false;
        do
        {
            if (fileinfo.cFileName[0] != '.')
            {
                aRoot = std::wstring(dir) + std::wstring(L"/") + std::wstring(fileinfo.cFileName);
                if (fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (!DeleteDirRecursive(aRoot.c_str(), true, force))
                    {
                        aSuccess = false;
                    }
                }
                else
                {
                    if (force)
                    {
                        if (fileinfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                        {
                            ::SetFileAttributesW(aRoot.c_str(), (::GetFileAttributesW(aRoot.c_str()) & ~FILE_ATTRIBUTE_READONLY));
                        }
                    }
                    if (!::DeleteFileW(aRoot.c_str()))
                        aSuccess = false;
                }
            }
        } while (::FindNextFileW(aFileHandle, &fileinfo));
        ::FindClose(aFileHandle);

        if (root && aSuccess)
        {
            if (!RemoveDirectoryW(dir))
            {
                return false;
            }
        }

        return aSuccess;
    }

    bool Base::DirExist(const char* dir)
    {
        ::DWORD aFileAttributes = GetFileAttributesA(dir);
        if (aFileAttributes == INVALID_FILE_ATTRIBUTES) return false;
        if (aFileAttributes & FILE_ATTRIBUTE_DIRECTORY) return true;
        return false;
    }

    bool Base::DirExist(const wchar_t * dir)
    {
        ::DWORD aFileAttributes = GetFileAttributesW(dir);
        if (aFileAttributes == INVALID_FILE_ATTRIBUTES) return false;
        if (aFileAttributes & FILE_ATTRIBUTE_DIRECTORY) return true;
        return false;
    }

    bool Base::FileExist(const char *file)
    {
        ::DWORD aFileAttributes = GetFileAttributesA(file);
        if (aFileAttributes == INVALID_FILE_ATTRIBUTES) return false;
        if (aFileAttributes & FILE_ATTRIBUTE_DIRECTORY) return false;
        return true;
    }

    bool Base::FileExist(const wchar_t * file)
    {
        ::DWORD aFileAttributes = GetFileAttributesW(file);
        if (aFileAttributes == INVALID_FILE_ATTRIBUTES) return false;
        if (aFileAttributes & FILE_ATTRIBUTE_DIRECTORY) return false;
        return true;
    }

    bool Base::MakeDir(const char* dir, LPSECURITY_ATTRIBUTES attributes /*= nullptr*/)
    {
        return MakeDir(Utilities::String::string_cast<std::wstring>(dir).c_str());
    }

    bool Base::MakeDir(const wchar_t * dir, LPSECURITY_ATTRIBUTES attributes /*= nullptr*/)
    {
        return (::CreateDirectoryW(dir, attributes) != FALSE);
    }

} // namespace Common
} // namespace Windows
