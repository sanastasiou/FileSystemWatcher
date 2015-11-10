#include "FileSystemWatcher/EventRouter.h"
#include "FileSystemWatcher/FileWatcher.h"

namespace Windows
{
namespace Clr
{
    EventRouter::EventRouter(FileWatcherBase^ managedFileSystemWatcher) :
        _managedFileSystemWatcher(managedFileSystemWatcher)
    {
    }

    void EventRouter::OnFileModified(const File::IFileSystemWatcher::FileSystemString & strFileName)
    {
        _managedFileSystemWatcher->OnFileModified(strFileName);
    }

    void EventRouter::OnFileRenamed(const FileSystemString & newFileName, const FileSystemString & oldFileName)
    {
        _managedFileSystemWatcher->OnFileRenamed(newFileName, oldFileName);
    }

    void EventRouter::OnFileRemoved(const FileSystemString & strFileName)
    {
        _managedFileSystemWatcher->OnFileRemoved(strFileName);
    }

    void EventRouter::OnFileAdded(const FileSystemString & strFileName)
    {
        _managedFileSystemWatcher->OnFileAdded(strFileName);
    }

    void EventRouter::OnError(::DWORD const errorCode, const FileSystemString & directory)
    {
        _managedFileSystemWatcher->OnError(errorCode, directory);
    }

    EventRouter::~EventRouter()
    {

    }
} // namespace Clr
} // namespace Windows