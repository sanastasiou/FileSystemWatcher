#ifndef WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_H__
#define WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_H__

#include "WindowsBase/File.h"
#include "WindowsBase/Thread.h"
#include "IFileSystemWatcher.h"

namespace Windows
{
namespace File
{
    class DelayedFileSystemWatcher : public FileSystemWatcherBase
    {
        virtual ~DelayedFileSystemWatcher();
    }; // class DelayedFileSystemWatcher
} // namespace File
} // namespace Windows
#endif //#ifndef WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_H__