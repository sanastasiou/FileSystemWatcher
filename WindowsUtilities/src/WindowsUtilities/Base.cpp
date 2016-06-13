#include "Windows.h"

#include "WindowsUtilities/Base.h"
#include "WindowsUtilities/File.h"

#include <string>
#include <exception>
#include <sstream>

namespace Windows
{
namespace Common
{
namespace internal__
{
    template<class T>
    struct windows_traits;

    template<>
    struct windows_traits<std::string>
    {
        typedef ::LPSTR char_pointer;
        typedef ::LPCSTR const_char_pointer;
        typedef std::string type;
        using FormatMessageType = decltype(&::FormatMessageA);
        static const FormatMessageType Format;
        typedef LPSTR ErrorBufferType;
    };

    const windows_traits<std::string>::FormatMessageType windows_traits<std::string>::Format = &::FormatMessageA;

    template<>
    struct windows_traits<std::wstring>
    {
        typedef ::LPWSTR char_pointer;
        typedef ::LPCWSTR const_char_pointer;
        typedef std::wstring type;
        using FormatMessageType = decltype(&::FormatMessageW);
        static const FormatMessageType Format;
        typedef LPWSTR ErrorBufferType;
    };

    const windows_traits<std::wstring>::FormatMessageType windows_traits<std::wstring>::Format = &::FormatMessageW;

    template<class StringType>
    typename StringType::type GetLastErrorImpl()
    {
       ::DWORD error = GetLastError();
        if (error)
        {
            typename StringType::char_pointer lpMsgBuf = nullptr;
            ::DWORD bufLen = StringType::Format (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                                 NULL,
                                                 error,
                                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                                 (typename StringType::ErrorBufferType)&lpMsgBuf,
                                                 0, NULL);
            if (bufLen)
            {
                typename StringType::const_char_pointer lpMsgStr = static_cast<typename StringType::const_char_pointer>(lpMsgBuf);
                typename StringType::type result(lpMsgStr, lpMsgStr + bufLen);

                LocalFree(lpMsgBuf);

                return result;
            }
        }
        return typename StringType::type();
    }
}

    void Base::GetLastErrorStr(std::string & error)
    {
        error = internal__::GetLastErrorImpl< internal__::windows_traits<std::string> >();
    }

    void Base::GetLastErrorStr(std::wstring & error)
    {
        error = internal__::GetLastErrorImpl< internal__::windows_traits<std::wstring> >();
    }

    bool Base::FlushFileBuffers(const wchar_t * drive)
    {
        const auto volume = CreateFileW( drive,
                                         GENERIC_READ,
                                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                                         NULL,
                                         OPEN_EXISTING,
                                         FILE_FLAG_NO_BUFFERING | FILE_FLAG_RANDOM_ACCESS,
                                         NULL);

        if (volume == INVALID_HANDLE_VALUE)
        {
            std::string aError;
            GetLastErrorStr(aError);
            std::stringstream iStream;
            iStream << "Could not open handle to " << Utilities::String::string_cast<std::string>(drive) << ", error: " << aError;
            throw std::exception(iStream.str().c_str());
        }

        //The documented way to flush an entire volume
        if (!::FlushFileBuffers(volume))
        {
            std::string aError;
            GetLastErrorStr(aError);
            std::stringstream iStream;
            iStream << "Could not flush " << Utilities::String::string_cast<std::string>(drive) << ", error: " << aError;
            throw std::exception(iStream.str().c_str());
        }
        
        if (!CloseHandle(volume))
        {
            std::string aError;
            GetLastErrorStr(aError);
            std::stringstream iStream;
            iStream << "Could not close handle " << Utilities::String::string_cast<std::string>(drive) << ", error: " << aError;
            throw std::exception(iStream.str().c_str());
        }

        return true;
    }

    bool Base::RemoveFileAttribute(const char * fileName, ::DWORD const attribute, bool force /*= false */)
    {
        if (force)
        {
            if (::GetFileAttributesA(fileName) & FILE_ATTRIBUTE_READONLY)
            {
                ::SetFileAttributesA(fileName, (::GetFileAttributesA(fileName) & ~FILE_ATTRIBUTE_READONLY));
            }
            if (attribute == FILE_ATTRIBUTE_READONLY)
            {
                return true;
            }
            else
            {
                return ::SetFileAttributesA(fileName, (::GetFileAttributesA(fileName) & ~attribute)) != 0;
            }
        }
        else
        {
            return ::SetFileAttributesA(fileName, (::GetFileAttributesA(fileName) & ~attribute)) != 0;
        }
    }

    bool Base::RemoveFileAttribute(const wchar_t * fileName, ::DWORD const attribute, bool force /*= false */)
    {
        if (force)
        {
            if (::GetFileAttributesW(fileName) & FILE_ATTRIBUTE_READONLY)
            {
                ::SetFileAttributesW(fileName, (::GetFileAttributesW(fileName) & ~FILE_ATTRIBUTE_READONLY));
            }
            if (attribute == FILE_ATTRIBUTE_READONLY)
            {
                return true;
            }
            else
            {
                return ::SetFileAttributesW(fileName, (::GetFileAttributesW(fileName) & ~attribute)) != 0;
            }
        }
        else
        {
            return ::SetFileAttributesW(fileName, (::GetFileAttributesW(fileName) & ~attribute)) != 0;
        }
    }

    bool Base::FileRename(const char * oldFileName, const char * newFileName, bool force /*= false */)
    {
        if (force)
        {
            RemoveFileAttribute(oldFileName, FILE_ATTRIBUTE_READONLY);
        }
        return ::MoveFileA(oldFileName, newFileName) != 0;
    }

    bool Base::FileRename(const wchar_t * oldFileName, const wchar_t * newFileName, bool force /*= false */)
    {
        if (force)
        {
            RemoveFileAttribute(oldFileName, FILE_ATTRIBUTE_READONLY);
        }
        return ::MoveFileW(oldFileName, newFileName) != 0;
    }

    bool Base::FileDelete(const char* filePath, bool force /*= false */)
    {
        if (FileExist(filePath))
        {
            if (force)
            {
                if (::GetFileAttributesA(filePath) & FILE_ATTRIBUTE_READONLY)
                {
                    ::SetFileAttributesA(filePath, (::GetFileAttributesA(filePath) & ~FILE_ATTRIBUTE_READONLY));
                }
            }
            ::DeleteFileA(filePath);
            return true;
        }
        return false;
    }

    bool Base::FileDelete(const wchar_t* filePath, bool force /*= false */)
    {
        if (FileExist(filePath))
        {
            if (force)
            {
                if (::GetFileAttributesW(filePath) & FILE_ATTRIBUTE_READONLY)
                {
                    ::SetFileAttributesW(filePath, (::GetFileAttributesW(filePath) & ~FILE_ATTRIBUTE_READONLY));
                }
            }
            ::DeleteFileW(filePath);
            return true;
        }
        return false;
    }

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
        return MakeDir(Utilities::String::string_cast<std::wstring>(dir).c_str(), attributes);
    }

    bool Base::MakeDir(const wchar_t * dir, LPSECURITY_ATTRIBUTES attributes /*= nullptr*/)
    {
        return (::CreateDirectoryW(dir, attributes) != FALSE);
    }

} // namespace Common
} // namespace Windows
