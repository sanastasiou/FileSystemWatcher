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
#include <algorithm>

namespace Windows
{
namespace Utilities
{
    namespace String
    {
        namespace internal__
        {
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

            template<class StringType>
            struct wildcards;

            template<>
            struct wildcards<std::string>
            {
                static const char STAR = '*';
                static const char QUESTION_MARK = '?';
            };

            template<>
            struct wildcards<std::wstring>
            {
                static const wchar_t STAR = L'*';
                static const wchar_t QUESTION_MARK = L'?';
            };

            template <class StringType>
            struct string_traits;

            template <>
            struct WINDOWS_UTILITIES_FILE_API string_traits<std::string>
            {
                typedef std::string::value_type char_trait;
                static const UINT CODEPAGE = 65001U; // UTF-8
                static decltype(::tolower) * tolower;

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
            struct WINDOWS_UTILITIES_FILE_API string_traits<std::wstring>
            {
                typedef std::wstring::value_type char_trait;
                static const UINT CODEPAGE = CP_ACP; // ANSI
                static decltype(::towlower) * tolower;

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

            template<class StringType>
            struct string_wildcard_compare_impl
            {
                inline static bool compare(StringType const & wildcardString, StringType const & string, bool const ignoreCase)
                {
                    if (ignoreCase)
                    {
                        StringType aTmpWildChardStr;
                        aTmpWildChardStr.resize(wildcardString.length());
                        std::transform(wildcardString.begin(), wildcardString.end(), aTmpWildChardStr.begin(), string_traits<StringType>::tolower);

                        StringType aTmpStr;
                        aTmpStr.resize(string.length());
                        std::transform(string.begin(), string.end(), aTmpStr.begin(), string_traits<StringType>::tolower);

                        do_compare(aTmpWildChardStr, aTmpStr, wildcards<StringType>());
                    }
                    return do_compare(wildcardString, string, wildcards<StringType>());
                }

            private:
                template<class Wildcards>
                inline static bool do_compare(StringType const & wildcardString, StringType const & string, Wildcards const)
                {
                    if (wildcardString.empty())
                    {
                        return string.empty();
                    }

                    int pS = 0;
                    int pW = 0;
                    int lS = string.length();
                    int lW = wildcardString.length();

                    while (pS < lS && pW < lW && wildcardString[pW] != Wildcards::STAR)
                    {
                        auto wild = wildcardString[pW];
                        if (wild != Wildcards::QUESTION_MARK && wild != string[pS])
                        {
                            return false;
                        }
                        pW++;
                        pS++;
                    }

                    int pSm = 0;
                    int pWm = 0;
                    while (pS < lS && pW < lW)
                    {
                        auto wild = wildcardString[pW];
                        if (wild == Wildcards::STAR)
                        {
                            pW++;
                            if (pW == lW)
                            {
                                return true;
                            }
                            pWm = pW;
                            pSm = pS + 1;
                        }
                        else if (wild == Wildcards::QUESTION_MARK || wild == string[pS])
                        {
                            pW++;
                            pS++;
                        }
                        else
                        {
                            pW = pWm;
                            pS = pSm;
                            pSm++;
                        }
                    }
                    while (pW < lW && wildcardString[pW] == Wildcards::STAR)
                    {
                        pW++;
                    }
                    return pW == lW && pS == lS;
                }
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

        /*
        * base on example from http://www.codeproject.com/Articles/1088/Wildcard-string-compare-globbing
        * can be used with std::string, std::wstring
        */
        template<class StringType>
        inline bool wildcard_compare(StringType const & wildcardString, StringType const & string, bool const ignoreCase = true)
        {
            return internal__::string_wildcard_compare_impl<StringType>::compare(wildcardString, string, ignoreCase);
        }

        /*
        * base on example from http://www.codeproject.com/Articles/1088/Wildcard-string-compare-globbing
        * can be used with char*, wchat_t*
        */
        template<class StringType>
        inline bool wildcard_compare(StringType * wildcardString, StringType * string, bool const ignoreCase = true)
        {
            return internal__::string_wildcard_compare_impl<typename internal__::string_type_of<const StringType *>::wrapper >::compare(wildcardString, string, ignoreCase);
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