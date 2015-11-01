#ifndef WINDOWS_FILE_IFILESYSTEMWATCHER_H__
#define WINDOWS_FILE_IFILESYSTEMWATCHER_H__

#include "Windows.h"
#include<TCHAR.H> // Implicit or explicit include
#include <string>

namespace Windows
{
namespace File
{
    struct IFileSystemWatcher
    {
        typedef std::basic_string< ::TCHAR > FileSystemString;

        /**
        * \brief   Occurs when a file was modified.
        *
        * \param   strFileName Filename of the file which was modified.
        */
        virtual void OnFileModified(const FileSystemString & strFileName)const = 0;
    };

    struct FileSystemWatcherBase : IFileSystemWatcher
    {
        typedef std::basic_string< ::TCHAR > FileSystemString;
    protected:
        FileSystemWatcherBase( FileSystemString const & dir,
                               ::DWORD changeFlags,
                               ::BOOL const watchSubDir,
                               IFileSystemWatcher const * const eventHandler,
                               FileSystemString includeFilter = _T(""),
                               FileSystemString excludeFilter = _T(""));

        FileSystemString _dir;
        ::DWORD _changeFlags;
        ::BOOL _watchSubDirectories;
        IFileSystemWatcher const * const _eventHandler;
        FileSystemString _includeFilter;
        FileSystemString _excludeFilter;
    };

    inline FileSystemWatcherBase::FileSystemWatcherBase( FileSystemString const & dir,
                                                         ::DWORD changeFlags,
                                                         ::BOOL const watchSubDir,
                                                         IFileSystemWatcher const * const eventHandler,
                                                         FileSystemString includeFilter /*_T("")*/,
                                                         FileSystemString excludeFilter /*_T("")*/) :
        _dir(dir),
        _changeFlags(changeFlags),
        _watchSubDirectories(watchSubDir),
        _eventHandler(eventHandler),
        _includeFilter(includeFilter),
        _excludeFilter(excludeFilter)
    {
    }
} // namespace File
} // namespace Windows

#endif //#ifndef WINDOWS_FILE_IFILESYSTEMWATCHER_H__