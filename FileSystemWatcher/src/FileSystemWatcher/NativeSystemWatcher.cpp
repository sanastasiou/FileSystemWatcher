#include "FileSystemWatcher/NativeSystemWatcher.h"
#include "WindowsBase/PrivilegeEnabler.h"
#include <vector>

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
        _watcherThread(DirectoryChangedCallback)
    {
        std::vector<::LPCTSTR> _privileges(3);
        _privileges.push_back(SE_BACKUP_NAME);
        _privileges.push_back(SE_RESTORE_NAME);
        _privileges.push_back(SE_CHANGE_NOTIFY_NAME);

        //enable privileges required for directory watching
        Utilities::PrivilegeEnabler::Initialize(_privileges);
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