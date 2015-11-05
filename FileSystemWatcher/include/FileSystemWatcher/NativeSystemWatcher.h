#ifndef WINDOWS_FILE_NATIVESYSTEMWATCHER_H__
#define WINDOWS_FILE_NATIVESYSTEMWATCHER_H__

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

        virtual bool IsWatching()const;

        virtual void OnFileModified(const FileSystemString & strFileName)const;
    protected:
        virtual void RequestTermination();
    private:
        static const ::DWORD NO_CHANGES = 0UL;

        Threading::Thread _watcherThread; //!< Thread where dir changes are observed.
        bool _isWatching;                 //!< Indicates whether the specified dir is being observed.
        ::HANDLE _dirHandle;              //!< Handle to the observed directory.
        Threading::Mutex _addDirLock;     //!< Mutex when adding directories to watch.        

        bool StartDirectoryWatching();

        void StopDirectoryWatching();

        static unsigned int StartDirectoryObservation(void * data); //!< Called back when a change has been detected in the watched directory.
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