#include "FileSystemWatcher/NativeSystemWatcher.h"
#include "WindowsBase/PrivilegeEnabler.h"
#include <vector>
#include "WindowsBase/File.h"
#include <msclr/marshal_cppstd.h>
#include "atlpath.h"

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
        System::Diagnostics::Trace::WriteLine(dwErrorCode);
        if (dwErrorCode == ERROR_OPERATION_ABORTED)
        {
            System::Diagnostics::Trace::WriteLine("omfg!!!");
            //send error notification to shut down this watcher
            return;
        }

        // This might mean overflow? Not sure.
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
            return;

        pBlock->BackupBuffer(dwNumberOfBytesTransfered);

        // Get the new read issued as fast as possible. The documentation
        // says that the original OVERLAPPED structure will not be used
        // again once the completion routine is called.
        pBlock->BeginRead(pBlock->GetNotificationRoutine(), pBlock->GetDirHandle() );

        pBlock->ProcessNotification();
    }

    LPOVERLAPPED_COMPLETION_ROUTINE NativeFileSystemWatcher::GetNotificationRoutine()const
    {
        return &NativeFileSystemWatcher::DirectoryNotification;
    }

    void NativeFileSystemWatcher::ProcessNotification()
    {
        BYTE* pBase = _backupBuffer.data();

        for (;;)
        {
            FILE_NOTIFY_INFORMATION& fni = (FILE_NOTIFY_INFORMATION&)*pBase;

            std::wstring wstrFilename(fni.FileName, fni.FileNameLength / sizeof(wchar_t));
            // Handle a trailing backslash, such as for a root directory.
            if ( wstrFilename.substr(wstrFilename.length() - 1, 1) != std::wstring(L"\\"))
            {
                wstrFilename = FileSystemWatcherBase::_directoryInfo._dir + L"\\" + wstrFilename;
            }
            else
            {
                wstrFilename = FileSystemWatcherBase::_directoryInfo._dir + wstrFilename;
            }

            // If it could be a short filename, expand it.
            LPCWSTR wszFilename = ::PathFindFileNameW(wstrFilename.c_str());
            int len = lstrlenW(wszFilename);
            // The maximum length of an 8.3 filename is twelve, including the dot.
            if (len <= 12 && wcschr(wszFilename, L'~'))
            {
                // Convert to the long filename form. Unfortunately, this
                // does not work for deletions, so it's an imperfect fix.
                wchar_t wbuf[MAX_PATH];
                if (::GetLongPathNameW(wstrFilename.c_str(), wbuf, _countof(wbuf)) > 0)
                {
                    wstrFilename = wbuf;
                }
            }            

            //put everything into a thread safe queue and pop them out in a different thread, thus minimising runtime costs
            //m_pServer->m_pBase->Push(fni.Action, wstrFilename);

            if (!fni.NextEntryOffset)
            {
                break;
            }
            pBase += fni.NextEntryOffset;
        }
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