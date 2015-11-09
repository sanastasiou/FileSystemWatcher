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
                                                                         std::vector<::BYTE>::size_type const bufferSize
                                                                        );

        WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_API virtual ~NativeFileSystemWatcher();

        void StartWatching();

        void StopWatching();

        void SetDir(IFileSystemWatcher::FileSystemString const & newDir);

        void SetFlags(::DWORD const newFlags);

        void SetIncludeFilter(IFileSystemWatcher::FileSystemString const & newInclusionFilter);

        void SetExcludeFilter(IFileSystemWatcher::FileSystemString const & newExclusionFilter);

        IFileSystemWatcher::FileSystemString GetDir()const;

        ::DWORD GetFlags()const;

        IFileSystemWatcher::FileSystemString GetIncludeFilter()const;

        IFileSystemWatcher::FileSystemString GetExcludeFilter()const;

        bool IsWatching()const;

        WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_API virtual void OnFileModified(const FileSystemString & strFileName);

        WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_API virtual void OnFileRenamed(const FileSystemString & newFileName, const FileSystemString & oldFileName);

        WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_API virtual void OnFileRemoved(const FileSystemString & strFileName);

        WINDOWS_FILE_NATIVEFILESYSTEMWATCHER_API virtual void OnFileAdded(const FileSystemString & strFileName);
    protected:
        virtual void RequestTermination();

        virtual LPOVERLAPPED_COMPLETION_ROUTINE GetNotificationRoutine()const;

        virtual void ProcessNotification();
    private:
        static const ::DWORD NO_CHANGES = 0UL;

        Threading::Thread _watcherThread; //!< Thread where dir changes are observed.
        bool _isWatching;                 //!< Indicates whether the specified dir is being observed.
        Threading::Mutex _addDirLock;     //!< Mutex when adding directories to watch.        

        bool StartDirectoryWatching();

        static void WINAPI DirectoryNotification(::DWORD dwErrorCode, ::DWORD dwNumberOfBytesTransfered, ::LPOVERLAPPED lpOverlapped );
    }; //class NativeFileSystemWatcher

    inline bool NativeFileSystemWatcher::IsWatching()const
    {
        return _isWatching;
    }

    inline IFileSystemWatcher::FileSystemString NativeFileSystemWatcher::GetExcludeFilter()const
    {
        return _directoryInfo._excludeFilter;
    }

    inline IFileSystemWatcher::FileSystemString NativeFileSystemWatcher::GetIncludeFilter()const
    {
        return _directoryInfo._includeFilter;
    }

    inline ::DWORD  NativeFileSystemWatcher::GetFlags()const
    {
        return _directoryInfo._changeFlags;
    }

    inline IFileSystemWatcher::FileSystemString NativeFileSystemWatcher::GetDir()const
    {
        return _directoryInfo._dir;
    }


} // namespace File
} // namespace Windows

#endif //#ifndef WINDOWS_FILE_NATIVESYSTEMWATCHER_H__