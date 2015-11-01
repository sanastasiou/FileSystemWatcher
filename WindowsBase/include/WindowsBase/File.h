#ifndef WINDOWS_UTILITIES_FILE_H__
#define WINDOWS_UTILITIES_FILE_H__

#if defined(WINDOWS_UTILITIES_FILE_DLL_EXPORTS)
#define WINDOWS_UTILITIES_FILE_API __declspec (dllexport)
#else
#define WINDOWS_UTILITIES_FILE_API __declspec (dllimport)
#endif

#include <Windows.h>
#include <string>
#include <vector>

namespace Windows
{
namespace Utilities
{
    namespace String
    {
        namespace internal__
        {
            template <class StringType>
            struct string_traits;

            template <>
            struct string_traits<std::string>
            {
                typedef std::string::value_type char_trait;
                static const UINT CODEPAGE = 65001U; // UTF-8

                inline static std::string convert(std::wstring const & src)
                {
                    std::wstring::size_type const aSrcLength = src.length();
                    if (aSrcLength == 0)
                    {
                        return std::string();
                    }
                    int const aStrLength = ::WideCharToMultiByte(CODEPAGE, 0, src.data(), src.length(), NULL, 0, NULL, NULL);

                    std::vector<char> aBuffer(aStrLength);
                    ::WideCharToMultiByte(CODEPAGE, 0, src.data(), src.length(), &aBuffer[0], aStrLength, NULL, NULL);

                    return std::string(aBuffer.begin(), aBuffer.end());
                }
            };

            template <>
            struct string_traits<std::wstring>
            {
                typedef std::wstring::value_type char_trait;
                static const UINT CODEPAGE = CP_ACP; // ANSI

                inline static std::wstring convert(std::string const & src)
                {
                    std::string::size_type const aSrcLength = src.length();
                    if (aSrcLength == 0)
                    {
                        return std::wstring();
                    }

                    int const aWideStrLength = ::MultiByteToWideChar(CODEPAGE, 0, src.data(), src.length(), NULL, 0);

                    std::vector<wchar_t> aWideBuffer(aWideStrLength);
                    ::MultiByteToWideChar(CODEPAGE, 0, src.data(), src.length(), &aWideBuffer[0], aWideStrLength);

                    return std::wstring(aWideBuffer.begin(), aWideBuffer.end());
                }
            };

            template<class To, class From>
            struct string_cast_impl
            {
                inline static To cast(From const & src)
                {
                    return string_traits<To>::convert(src);
                }
            };

            template<class To>
            struct string_cast_impl<To, To>
            {
                inline static To cast(To const & src)
                {
                    return src;
                }
            };

            template <class T>
            struct string_type_of;

            template<>
            struct string_type_of<const char *>
            {
                typedef std::string wrapper;
            };

            template<>
            struct string_type_of<const wchar_t *>
            {
                typedef std::wstring wrapper;
            };
        }

        template<class To, class From>
        To string_cast(From const & src)
        {
            return internal__::string_cast_impl<To, From>::cast(src);
        }

        template<class To, class From>
        To string_cast(From * src)
        {
            return internal__::string_cast_impl<To, typename internal__::string_type_of<const From *>::wrapper >::cast(src);
        }
    } //namespace String

    class WINDOWS_UTILITIES_FILE_API File
    {
    public:

        inline static bool const GetFileSize64(::LPCTSTR path, ::PLARGE_INTEGER size)
        {
            ::BOOL aSuccessState = FALSE;
            if (path && size)
            {
                size->QuadPart = 0;
                ::HANDLE aFileHandle = NULL;

                aFileHandle = CreateFile(path, READ_CONTROL, 0, NULL, OPEN_EXISTING, 0, NULL);

                if (aFileHandle != INVALID_HANDLE_VALUE)
                {
                    aSuccessState = ::GetFileSizeEx(aFileHandle, size);
                    ::CloseHandle(aFileHandle);
                }
            }
            return aSuccessState != FALSE;
        }

        static bool const CreateNewFile(char const * path, ::FILE *& fileStream, char const * mode = "a+t");

        static bool const CreateNewFile(wchar_t const * path, ::FILE *& fileStream, wchar_t const * mode = L"a+t");

        static std::string GetFullPath(char const * relativePath);

        static std::wstring GetFullPath(wchar_t const * relativePath);

        static std::string GetDirectoryFromFilePath(char const * path);

        static std::wstring GetDirectoryFromFilePath(wchar_t const * path);

        static std::string GetFileFromFilePath(char const * path);

        static std::wstring GetFileFromFilePath(wchar_t const * path);
    };

} // namespace Utilities
} // namespace Windows

#endif //#ifndef WINDOWS_UTILITIES_FILE_H__