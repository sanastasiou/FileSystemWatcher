#ifndef WINDOWS_FILE_NATIVESYSTEMWATCHER_H__
#define WINDOWS_FILE_NATIVESYSTEMWATCHER_H__

#if defined(WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_DLL_EXPORTS)
#define WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_API __declspec (dllexport)
#else
#define WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_API __declspec (dllimport)
#endif

#include "WindowsBase/File.h"
#include "WindowsBase/Thread.h"
#include "WindowsBase/Mutex.h"
#include "IFileSystemWatcher.h"
#include <vector>

namespace Windows
{
namespace File
{
    class NativeFileSystemWatcher : public FileSystemWatcherBase
    {
    public:
        WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_API NativeFileSystemWatcher(IFileSystemWatcher::FileSystemString const & dir,
                                                                         ::DWORD changeFlags,
                                                                         ::BOOL const watchSubDir,
                                                                         IFileSystemWatcher * const eventHandler,
                                                                         IFileSystemWatcher::FileSystemString includeFilter,
                                                                         IFileSystemWatcher::FileSystemString excludeFilter,
                                                                         bool restartOnError,
                                                                         std::vector<::BYTE>::size_type const bufferSize
                                                                        );

        WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_API virtual ~NativeFileSystemWatcher();

        WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_API void StartWatching();
                                                 
        WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_API void StopWatching();
                                                 
        WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_API void SetDir(IFileSystemWatcher::FileSystemString const & newDir);
                                                 
        WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_API void SetFlags(::DWORD const newFlags);
                                                 
        WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_API void SetIncludeFilter(IFileSystemWatcher::FileSystemString const & newInclusionFilter);
                                                 
        WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_API void SetExcludeFilter(IFileSystemWatcher::FileSystemString const & newExclusionFilter);
                                                 
        WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_API void SetRestartOnError(bool const restart);

        WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_API virtual void OnFileModified(const FileSystemString & strFileName);

        WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_API virtual void OnFileRenamed(const FileSystemString & newFileName, const FileSystemString & oldFileName);

        WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_API virtual void OnFileRemoved(const FileSystemString & strFileName);

        WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_API virtual void OnFileAdded(const FileSystemString & strFileName);

        WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_API virtual void OnError(::DWORD const, const FileSystemString &);

        bool IsAutomaticRestartingEnabled()const;

        IFileSystemWatcher::FileSystemString GetDir()const;

        ::DWORD GetFlags()const;

        IFileSystemWatcher::FileSystemString GetIncludeFilter()const;

        IFileSystemWatcher::FileSystemString GetExcludeFilter()const;

        bool IsWatching()const;
    protected:
        virtual void RequestTermination();

        virtual LPOVERLAPPED_COMPLETION_ROUTINE GetNotificationRoutine()const;

        virtual void ProcessNotification();
    private:
        static const ::DWORD NO_CHANGES = 0UL;

        Threading::Thread _watcherThread; //!< Thread where dir changes are observed.
        bool _isWatching;                 //!< Indicates whether the specified dir is being observed.
        Threading::Mutex _addDirLock;     //!< Mutex when adding directories to watch.
        bool _restartOnError;             //!< Restart wathing on error automatically.

        bool StartDirectoryWatching();

        static void WINAPI DirectoryNotification(::DWORD dwErrorCode, ::DWORD dwNumberOfBytesTransfered, ::LPOVERLAPPED lpOverlapped );
    }; //class NativeFileSystemWatcher

    inline bool NativeFileSystemWatcher::IsAutomaticRestartingEnabled()const
    {
        return _restartOnError;
    }

    inline IFileSystemWatcher::FileSystemString NativeFileSystemWatcher::GetDir()const
    {
        return _directoryInfo._dir;
    }

    inline ::DWORD  NativeFileSystemWatcher::GetFlags()const
    {
        return _directoryInfo._changeFlags;
    }

    inline IFileSystemWatcher::FileSystemString NativeFileSystemWatcher::GetIncludeFilter()const
    {
        return _directoryInfo._includeFilter;
    }

    inline IFileSystemWatcher::FileSystemString NativeFileSystemWatcher::GetExcludeFilter()const
    {
        return _directoryInfo._excludeFilter;
    }

    inline bool NativeFileSystemWatcher::IsWatching()const
    {
        return _isWatching;
    }
} // namespace File
} // namespace Windows

#endif //#ifndef WINDOWS_FILE_NATIVESYSTEMWATCHER_H__