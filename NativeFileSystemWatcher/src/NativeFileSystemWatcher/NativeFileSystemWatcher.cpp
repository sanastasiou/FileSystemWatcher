#pragma managed(push, off)

#include "NativeFileSystemWatcher/NativeFileSystemWatcher.h"
#include "WindowsUtilities/PrivilegeEnabler.h"
#include <vector>
#include <string>
#include "WindowsUtilities/File.h"
#include "WindowsUtilities/Base.h"

namespace Windows
{
namespace File
{
    NativeFileSystemWatcher::NativeFileSystemWatcher(IFileSystemWatcher::FileSystemString const & dir,
        ::DWORD changeFlags,
        ::BOOL const watchSubDir,
        IFileSystemWatcher * const eventHandler,
        IFileSystemWatcher::FileSystemString includeFilter,
        IFileSystemWatcher::FileSystemString excludeFilter,
        bool restartOnError,
        std::vector<::BYTE>::size_type const bufferSize) :
        FileSystemWatcherBase(dir, changeFlags, watchSubDir, eventHandler, includeFilter, excludeFilter, bufferSize),
        _isWatching(false),
        _watcherThread(FileSystemWatcherBase::ThreadStartProc),
        _restartOnError(restartOnError)
    {
        std::vector<std::wstring> _privileges;
        _privileges.push_back(SE_BACKUP_NAME);
        _privileges.push_back(SE_RESTORE_NAME);
        _privileges.push_back(SE_CHANGE_NOTIFY_NAME);
        //enable privileges required for directory watching
        Utilities::PrivilegeEnabler::Initialize(_privileges);
        StartWatching();
    }

    NativeFileSystemWatcher::~NativeFileSystemWatcher()
    {
        StopWatching();
    }

    void NativeFileSystemWatcher::StartWatching()
    {
        if (!_isWatching)
        {
            if (Common::Base::DirExist(_directoryInfo._dir.c_str()) && (_directoryInfo._changeFlags != NO_CHANGES))
            {
                _isWatching = StartDirectoryWatching();
                if (_isWatching)
                {
                    //listen for modifications
                    ::QueueUserAPC(FileSystemWatcherBase::AddDirectoryProc, _watcherThread, reinterpret_cast<ULONG_PTR>(this));
                    StartPoppingEvents();
                }
            }
        }
    }

    void NativeFileSystemWatcher::StopWatching()
    {
        _isWatching = false;
        //stop event publishing
        StopPopping();
        ::QueueUserAPC(FileSystemWatcherBase::TerminateWatching, _watcherThread, reinterpret_cast<ULONG_PTR>(this));
        _watcherThread.Stop();
        GetEventQueue().ClearSync();
        ResetOverlappedStructure();
    }

    void NativeFileSystemWatcher::SetDir(IFileSystemWatcher::FileSystemString const & newDir)
    {
        if (newDir != _directoryInfo._dir)
        {
            if (_isWatching)
            {
                StopWatching();
            }
            _directoryInfo._dir = newDir;
            if (_restartOnError)
            {
                StartWatching();
            }
        }
    }

    void NativeFileSystemWatcher::SetFlags(::DWORD const newFlags)
    {
        if (newFlags != _directoryInfo._changeFlags)
        {
            if (_isWatching)
            {
                StopWatching();
            }
            _directoryInfo._changeFlags = newFlags;
            if (_restartOnError)
            {
                StartWatching();
            }
        }
    }

    void NativeFileSystemWatcher::SetIncludeFilter(IFileSystemWatcher::FileSystemString const & newInclusionFilter)
    {
        if (newInclusionFilter != _directoryInfo._includeFilter)
        {
            if (_isWatching)
            {
                StopWatching();
            }
            FileSystemWatcherBase::SetIncludeFilter(newInclusionFilter);
            if (_restartOnError)
            {
                StartWatching();
            }
        }
    }

    void NativeFileSystemWatcher::SetExcludeFilter(IFileSystemWatcher::FileSystemString const & newExclusionFilter)
    {
        if (newExclusionFilter != _directoryInfo._excludeFilter)
        {
            if (_isWatching)
            {
                StopWatching();
            }
            FileSystemWatcherBase::SetExcludeFilter(newExclusionFilter);
            if (_restartOnError)
            {
                StartWatching();
            }
        }
    }

    void NativeFileSystemWatcher::SetRestartOnError(bool const restart)
    {
        _restartOnError = restart;
    }

    void NativeFileSystemWatcher::OnError()
    {
        if (_isWatching)
        {
            //stop watching and restart watching if specified
            StopWatching();
            if (_restartOnError)
            {
                StartWatching();
            }
        }
    }

    void NativeFileSystemWatcher::RequestTermination()
    {
        ::CancelIo(_dirHandle);
        ::CloseHandle(_dirHandle);
        _dirHandle = nullptr;
        FileSystemWatcherBase::RequestTermination();
    }

    LPOVERLAPPED_COMPLETION_ROUTINE NativeFileSystemWatcher::GetNotificationRoutine()const
    {
        return &NativeFileSystemWatcher::DirectoryNotification;
    }

    void NativeFileSystemWatcher::ProcessNotification()
    {
        auto pBase = GetBackUpBuffer().data();

        for (;;)
        {
            FILE_NOTIFY_INFORMATION& fni = (FILE_NOTIFY_INFORMATION&)*pBase;

            std::wstring wstrFilename(fni.FileName, fni.FileNameLength / sizeof(wchar_t));
            
            if (IsFileIncluded(wstrFilename) && !IsFileExcluded(wstrFilename))
            {
                // The maximum length of an 8.3 filename is twelve, including the dot.
                if (wstrFilename.length() <= 12U && (wstrFilename.find(L'~') != std::wstring::npos))
                {
                    // Convert to the long filename form. Unfortunately, this
                    // does not work for deletions, so it's an imperfect fix.
                    wchar_t wbuf[MAX_PATH];
                    if (::GetLongPathNameW(wstrFilename.c_str(), wbuf, _countof(wbuf)) > 0)
                    {
                        wstrFilename = wbuf;
                    }
                }
                else
                {
                    // Handle a trailing backslash, such as for a root directory.
                    if (wstrFilename.substr(wstrFilename.length() - 1, 1) != std::wstring(L"\\"))
                    {
                        wstrFilename = FileSystemWatcherBase::_directoryInfo._dir + L"\\" + wstrFilename;
                    }
                    else
                    {
                        wstrFilename = FileSystemWatcherBase::_directoryInfo._dir + wstrFilename;
                    }
                }
                GetEventQueue().push(std::make_pair(wstrFilename, fni.Action));
            }

            if (!fni.NextEntryOffset)
            {
                break;
            }
            pBase += fni.NextEntryOffset;
        }
    }

    bool NativeFileSystemWatcher::StartDirectoryWatching()
    {
        //open the directory to watch....
        FileSystemWatcherBase::GetDirHandle() = ::CreateFileW(FileSystemWatcherBase::_directoryInfo._dir.c_str(),
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

        _terminate = false;
        //start background thread
        return _watcherThread.Start(this);
    }

    void WINAPI NativeFileSystemWatcher::DirectoryNotification(::DWORD dwErrorCode, ::DWORD dwNumberOfBytesTransfered, ::LPOVERLAPPED lpOverlapped)
    {
        NativeFileSystemWatcher* pBlock = reinterpret_cast<NativeFileSystemWatcher*>(lpOverlapped->hEvent);

        if (dwErrorCode == ERROR_OPERATION_ABORTED)
        {
            //send error notification to shut down this watcher
            pBlock->GetEventQueue().push(std::make_pair(L"", ERROR_OPERATION_ABORTED));
            return;
        }

        // The number of bytes transferred. If an error occurs, this parameter is zero.
        if (!dwNumberOfBytesTransfered)
        {
            //nothing to read...
            return;
        }

        // Can't use sizeof(FILE_NOTIFY_INFORMATION) because
        // the structure is padded to 16 bytes.
        _ASSERTE(dwNumberOfBytesTransfered >= offsetof(FILE_NOTIFY_INFORMATION, FileName) + sizeof(WCHAR));

        // This might mean overflow? Not sure.
        if (!dwNumberOfBytesTransfered)
        {
            return;
        }

        pBlock->BackupBuffer(dwNumberOfBytesTransfered);

        // Get the new read issued as fast as possible. The documentation
        // says that the original OVERLAPPED structure will not be used
        // again once the completion routine is called.
        pBlock->BeginRead(pBlock->GetNotificationRoutine(), pBlock->GetDirHandle());

        pBlock->ProcessNotification();
    }
} // namespace File
} // namespace Windows

#pragma managed(pop)