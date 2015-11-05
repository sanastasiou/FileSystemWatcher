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

        struct DirectoryInfo
        {
            FileSystemString _dir;
            ::DWORD _changeFlags;
            ::BOOL _watchSubDirectories;
            IFileSystemWatcher const * const _eventHandler;
            FileSystemString _includeFilter;
            FileSystemString _excludeFilter;
        };
    };

    struct FileSystemWatcherBase : IFileSystemWatcher
    {
        typedef std::basic_string< ::TCHAR > FileSystemString;

        virtual bool IsWatching()const = 0;

        virtual ~FileSystemWatcherBase()
        {
        }
    protected:
        FileSystemWatcherBase( FileSystemString const & dir,
                               ::DWORD changeFlags,
                               ::BOOL const watchSubDir,
                               IFileSystemWatcher const * const eventHandler,
                               FileSystemString includeFilter = _T(""),
                               FileSystemString excludeFilter = _T(""));

        static unsigned int ThreadStartProc(void * arg)
        {
            FileSystemWatcherBase* pServer = static_cast<FileSystemWatcherBase*>(arg);
            pServer->Run();
            return 0;
        }

        // Called by QueueUserAPC to start orderly shutdown.
        static void CALLBACK TerminateProc(__in  ULONG_PTR arg)
        {
            FileSystemWatcherBase* pServer = reinterpret_cast<FileSystemWatcherBase*>(arg);
            pServer->RequestTermination();
        }

        void Run()
        {
            while (!_terminate)
            {
                (void)::SleepEx(INFINITE, true);
            }
        }

        // Call this from dervied classes, else thread won't terminate.
        virtual void RequestTermination() = 0
        {
            _terminate = true;
        }

        DirectoryInfo _directoryInfo;
        bool _terminate;                  //!< Notify wathcer to terminate dir observation.
    };

    inline FileSystemWatcherBase::FileSystemWatcherBase( FileSystemString const & dir,
                                                         ::DWORD changeFlags,
                                                         ::BOOL const watchSubDir,
                                                         IFileSystemWatcher const * const eventHandler,
                                                         FileSystemString includeFilter /*_T("")*/,
                                                         FileSystemString excludeFilter /*_T("")*/) :
        _directoryInfo(DirectoryInfo{ dir, changeFlags, watchSubDir, eventHandler, includeFilter, excludeFilter}),
        _terminate(false)
    {
    }
} // namespace File
} // namespace Windows

#endif //#ifndef WINDOWS_FILE_IFILESYSTEMWATCHER_H__