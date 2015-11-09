#ifndef WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_H__
#define WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_H__

#if defined(WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_DLL_EXPORTS)
#define WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_API __declspec (dllexport)
#else
#define WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_API __declspec (dllimport)
#endif

#include "WindowsBase/File.h"
#include "WindowsBase/Thread.h"
#include "IFileSystemWatcher.h"

namespace Windows
{
namespace File
{
    class DelayedFileSystemWatcher : public FileSystemWatcherBase
    {
        WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_API virtual ~DelayedFileSystemWatcher();
    }; // class DelayedFileSystemWatcher
} // namespace File
} // namespace Windows
#endif //#ifndef WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_H__