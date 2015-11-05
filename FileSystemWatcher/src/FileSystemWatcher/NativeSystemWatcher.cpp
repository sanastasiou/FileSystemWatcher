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
                                                      IFileSystemWatcher::FileSystemString includeFilter /*= ""*/,
                                                      IFileSystemWatcher::FileSystemString excludeFilter /*= ""*/) :
        FileSystemWatcherBase(dir, changeFlags, watchSubDir, eventHandler, includeFilter, excludeFilter),
        _isWatching(false),
        _watcherThread(DirectoryChangedCallback),
        _dirHandle(nullptr)
    {
        //check parameters
        if (Common::Base::DirExist(dir.c_str()) && changeFlags != NO_CHANGES)
        {
            //dir exists, start watcher thread...
            _isWatching = StartDirectoryWatching();
        }

        std::vector<::LPCTSTR> _privileges(3);
        _privileges.push_back(SE_BACKUP_NAME);
        _privileges.push_back(SE_RESTORE_NAME);
        _privileges.push_back(SE_CHANGE_NOTIFY_NAME);
        //enable privileges required for directory watching
        Utilities::PrivilegeEnabler::Initialize(_privileges);
    }

    bool NativeFileSystemWatcher::StartDirectoryWatching()
    {
        //open the directory to watch....
        _dirHandle = CreateFile(    FileSystemWatcherBase::_dir.c_str(),
                                    FILE_LIST_DIRECTORY,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, //<-- removing FILE_SHARE_DELETE prevents the user or someone else from renaming or deleting the watched directory.
                                    NULL, //security attributes
                                    OPEN_EXISTING,
                                    FILE_FLAG_BACKUP_SEMANTICS | //<- the required priviliges for this flag are: SE_BACKUP_NAME and SE_RESTORE_NAME.
                                    FILE_FLAG_OVERLAPPED, //OVERLAPPED!
                                    NULL
                                );
        if (_dirHandle == INVALID_HANDLE_VALUE)
        {
            return false;
        }
    }

    NativeFileSystemWatcher::~NativeFileSystemWatcher()
    {

    }

    void NativeFileSystemWatcher::StartWatching()
    {

    }

    void NativeFileSystemWatcher::StopWatching()
    {

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

    unsigned int NativeFileSystemWatcher::DirectoryChangedCallback(void * data)
    {
        return 0;
    }

    void NativeFileSystemWatcher::OnFileModified(const FileSystemString & strFileName)const
    {
        _eventHandler->OnFileModified(strFileName);
    }
} // namespace File
} // namespace Windows