#ifndef WINDOWS_FILE_NATIVESYSTEMWATCHER_H__
#define WINDOWS_FILE_NATIVESYSTEMWATCHER_H__

#include "WindowsBase/File.h"
#include "WindowsBase/Thread.h"
#include "IFileSystemWatcher.h"

namespace Windows
{
namespace File
{
    class NativeFileSystemWatcher : public FileSystemWatcherBase
    {
    public:
        NativeFileSystemWatcher(IFileSystemWatcher::FileSystemString const & dir, 
                                ::DWORD changeFlags,
                                ::BOOL const watchSubDir,
                                IFileSystemWatcher const * const eventHandler,
                                IFileSystemWatcher::FileSystemString includeFilter,
                                IFileSystemWatcher::FileSystemString excludeFilter
                               );

        virtual ~NativeFileSystemWatcher();

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

        virtual void OnFileModified(const FileSystemString & strFileName)const;
    private:
        static const ::DWORD NO_CHANGES = 0UL;

        Threading::Thread _watcherThread; //!< Thread where dir changes are observed.
        bool _isWatching;                 //!< Indicates whether the specified dir is being observed.
        ::HANDLE _dirHandle;              //!< Handle to the observed directory.

        bool StartDirectoryWatching();

        void StopDirectoryWatching();

        static unsigned int DirectoryChangedCallback(void * data); //!< Called back when a change has been detected in the watched directory.
    }; //class NativeFileSystemWatcher

    inline bool NativeFileSystemWatcher::IsWatching()const
    {
        return _isWatching;
    }

    inline IFileSystemWatcher::FileSystemString NativeFileSystemWatcher::GetExcludeFilter()const
    {
        return _excludeFilter;
    }

    inline IFileSystemWatcher::FileSystemString NativeFileSystemWatcher::GetIncludeFilter()const
    {
        return _includeFilter;
    }

    inline ::DWORD  NativeFileSystemWatcher::GetFlags()const
    {
        return _changeFlags;
    }

    inline IFileSystemWatcher::FileSystemString NativeFileSystemWatcher::GetDir()const
    {
        return _dir;
    }


} // namespace File
} // namespace Windows

#endif //#ifndef WINDOWS_FILE_NATIVESYSTEMWATCHER_H__