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

    void EventRouter::OnFileModified(const File::IFileSystemWatcher::FileSystemString & strFileName)const
    {
        _managedFileSystemWatcher->OnFileModified(strFileName);
    }
} // namespace Clr
} // namespace Windows