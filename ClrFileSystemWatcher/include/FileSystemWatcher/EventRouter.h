#ifndef WINDOWS_CLR_EVENT_ROUTER_H__
#define WINDOWS_CLR_EVENT_ROUTER_H__

#include "NativeFileSystemWatcher/IFileSystemWatcher.h"

#include <vcclr.h>

namespace Windows
{
namespace Clr
{
    ref class FileWatcherBase;

    class EventRouter : public File::IFileSystemWatcher
    {
    public:
        EventRouter(FileWatcherBase^ managedFileSystemWatcher);

        virtual void OnFileModified(const File::IFileSystemWatcher::FileSystemString & strFileName);

        virtual void OnFileRenamed(const File::IFileSystemWatcher::FileSystemString & newFileName, const FileSystemString & oldFileName);

        virtual void OnFileRemoved(const File::IFileSystemWatcher::FileSystemString & strFileName);

        virtual void OnFileAdded(const File::IFileSystemWatcher::FileSystemString & strFileName);

        virtual void OnError(::DWORD const errorCode, const FileSystemString & directory);

        virtual ~EventRouter();

    private:
        gcroot<FileWatcherBase^> _managedFileSystemWatcher;
    };
} // namespace Clr
} // namespace Windows

#endif  //#ifndef WINDOWS_CLR_EVENT_ROUTER_H__