#include "FileSystemWatcher/NativeSystemWatcher.h"
#include "WindowsBase/PrivilegeEnabler.h"
#include <vector>
#include "WindowsBase/File.h"
#include <msclr/marshal_cppstd.h>

namespace Windows
{
namespace File
{
    NativeFileSystemWatcher::NativeFileSystemWatcher( IFileSystemWatcher::FileSystemString const & dir,
                                                      ::DWORD changeFlags,
                                                      ::BOOL const watchSubDir,
                                                      IFileSystemWatcher const * const eventHandler,
                                                      IFileSystemWatcher::FileSystemString includeFilter,
                                                      IFileSystemWatcher::FileSystemString excludeFilter,
                                                      std::vector<::BYTE>::size_type const bufferSize  ) :
        FileSystemWatcherBase(dir, changeFlags, watchSubDir, eventHandler, includeFilter, excludeFilter, bufferSize),
        _isWatching(false),
        _watcherThread(FileSystemWatcherBase::ThreadStartProc)
    {
        std::vector<::LPCTSTR> _privileges(3);
        _privileges.push_back(SE_BACKUP_NAME);
        _privileges.push_back(SE_RESTORE_NAME);
        _privileges.push_back(SE_CHANGE_NOTIFY_NAME);
        //enable privileges required for directory watching
        Utilities::PrivilegeEnabler::Initialize(_privileges);

        //check parameters
        if (Common::Base::DirExist(dir.c_str()) && changeFlags != NO_CHANGES)
        {
            _isWatching = StartDirectoryWatching();
            if (_isWatching)
            {
                //listen for modifications
                ::QueueUserAPC(FileSystemWatcherBase::AddDirectoryProc, _watcherThread, reinterpret_cast<ULONG_PTR>(this));
            }
        }
    }

    bool NativeFileSystemWatcher::StartDirectoryWatching()
    {
        //open the directory to watch....
        FileSystemWatcherBase::GetDirHandle() = ::CreateFileW( FileSystemWatcherBase::_directoryInfo._dir.c_str(),
                                                               FILE_LIST_DIRECTORY,
                                                               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, //<-- removing FILE_SHARE_DELETE prevents the user or someone else from renaming or deleting the watched directory.
                                                               NULL, //security attributes
                                                               OPEN_EXISTING,
                                                               FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,//<- the required priviliges for this flag are: SE_BACKUP_NAME and SE_RESTORE_NAME.
                                                               NULL
                                                             );
        if (_dirHandle == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        //start background thread
        return _watcherThread.Start(this);
    }

    void WINAPI NativeFileSystemWatcher::DirectoryNotification(::DWORD dwErrorCode, ::DWORD dwNumberOfBytesTransfered, ::LPOVERLAPPED lpOverlapped)
    {
        FileSystemWatcherBase* pBlock = reinterpret_cast<FileSystemWatcherBase*>(lpOverlapped->hEvent);

        if (dwErrorCode == ERROR_OPERATION_ABORTED)
        {
            System::Diagnostics::Trace::WriteLine("omfg!!!");
            //send error notification to shut down this watcher
            return;
        }
    }

    LPOVERLAPPED_COMPLETION_ROUTINE NativeFileSystemWatcher::GetNotificationRoutine()const
    {
        return &NativeFileSystemWatcher::DirectoryNotification;
    }

    NativeFileSystemWatcher::~NativeFileSystemWatcher()
    {
        StopWatching();
    }

    void NativeFileSystemWatcher::RequestTermination()
    {
        ::CancelIo(_dirHandle);
        ::CloseHandle(_dirHandle);
        _dirHandle = nullptr;
        FileSystemWatcherBase::RequestTermination();
    }

    void NativeFileSystemWatcher::StartWatching()
    {
        if (!IsWatching())
        {
            _isWatching = StartDirectoryWatching();
        }
    }

    void NativeFileSystemWatcher::StopWatching()
    {
        ::QueueUserAPC(FileSystemWatcherBase::TerminateProc, _watcherThread, reinterpret_cast<ULONG_PTR>(this));
        _watcherThread.Stop();
        _isWatching = false;
    }

    void NativeFileSystemWatcher::SetDir(IFileSystemWatcher::FileSystemString const & newDir)
    {

    }

    void NativeFileSystemWatcher::SetFlags(::DWORD const newFlags)
    {

    }

    void NativeFileSystemWatcher::SetIncludeFilter(IFileSystemWatcher::FileSystemString const & newInclusionFilter)
    {

    }

    void NativeFileSystemWatcher::SetExcludeFilter(IFileSystemWatcher::FileSystemString const & newExclusionFilter)
    {

    }

    void NativeFileSystemWatcher::OnFileModified(const FileSystemString & strFileName)const
    {
        _directoryInfo._eventHandler->OnFileModified(strFileName);
    }
} // namespace File
} // namespace Windows