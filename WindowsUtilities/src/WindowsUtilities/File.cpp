#include "WindowsUtilities/File.h"
#include <exception>

namespace Windows
{
namespace Utilities
{
namespace String
{
namespace internal__
{
    WINDOWS_UTILITIES_FILE_API decltype(::tolower) * string_traits<std::string>::tolower = &::tolower;
    WINDOWS_UTILITIES_FILE_API decltype(::towlower) * string_traits<std::wstring>::tolower = &::towlower;

}   //namespace internal__
}   //namespace String
    bool const File::CreateNewFile(char const * path, ::FILE * & fileStream, char const * mode)
    {
        ::errno_t err = fopen_s( &fileStream, path, mode );
        return (err == 0);
    }

    bool const File::CreateNewFile(wchar_t const * path, ::FILE  * & fileStream, wchar_t const * mode)
    {
        ::errno_t err = _wfopen_s( &fileStream, path, mode );
        return (err == 0);
    }

    std::string File::GetFullPath( char const * relativePath)
    {
        const int aBUFSIZE = 4096;
        char  aBuffer[aBUFSIZE] = {""};
        char** aLppPart={NULL};
        const ::DWORD aRetval = ::GetFullPathNameA( relativePath, aBUFSIZE, aBuffer, aLppPart);

        if (aRetval == 0) 
        {
            throw std::exception(std::string("Could not resolve absolute path from relative path : " + std::string(relativePath)).c_str());
        }

        return std::string( aBuffer );
    }

    std::wstring File::GetFullPath( wchar_t const * relativePath)
    {
        const int aBUFSIZE = 4096;
        wchar_t aBuffer[aBUFSIZE] = {L""};
        wchar_t ** aLppPart={NULL};
        const ::DWORD aRetval = ::GetFullPathNameW( relativePath, aBUFSIZE, aBuffer, aLppPart );

        if (aRetval == 0)
        {
            throw std::exception(String::string_cast<std::string>(std::wstring(L"Could not resolve absolute path from relative path : ") + std::wstring(relativePath)).c_str());
        }

        return std::wstring(aBuffer);
    }

    std::string File::GetDirectoryFromFilePath( char const * path)
    {
        auto aFullPath(File::GetFullPath(path));
        auto aPos = aFullPath.find_last_of("/\\");
        if( aPos == std::string::npos ) 
        {
            throw std::exception(std::string("Could not find directory using file path : " + std::string(path)).c_str());
        }            
        return aFullPath.substr( 0, aPos );
    }

    std::wstring File::GetDirectoryFromFilePath( wchar_t const * path)
    {
        return String::string_cast<std::wstring>(File::GetDirectoryFromFilePath(String::string_cast<std::string>(path).c_str()));
    }

    std::string File::GetFileFromFilePath( char const * path )
    {
        std::string aPath(path);
        std::string aFilename;

        const size_t pos = aPath.find_last_of("\\/");
        if (pos != std::string::npos)
        {
            aFilename.assign(aPath.begin() + pos + 1, aPath.end());
        }
        else
        {
            aFilename = aPath;
        }
        return aFilename.c_str();
    }

    std::wstring File::GetFileFromFilePath( wchar_t const * path )
    {
        return String::string_cast<std::wstring>(File::GetFileFromFilePath(String::string_cast<std::string>(path).c_str()));
    }

} // namespace Utilities
} // namespace Windows