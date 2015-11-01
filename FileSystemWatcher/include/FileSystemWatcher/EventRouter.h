#ifndef WINDOWS_CLR_EVENT_ROUTER_H__
#define WINDOWS_CLR_EVENT_ROUTER_H__

#include "IFileSystemWatcher.h"
//#include "FileSystemWatcher.h"

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

        virtual void OnFileModified(const File::IFileSystemWatcher::FileSystemString & strFileName)const;
    private:
        gcroot<FileWatcherBase^> _managedFileSystemWatcher;
    };
} // namespace Clr
} // namespace Windows

#endif  //#ifndef WINDOWS_CLR_EVENT_ROUTER_H__