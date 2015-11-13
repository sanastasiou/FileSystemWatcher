#ifndef WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_H__
#define WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_H__

#if defined(WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_DLL_EXPORTS)
#define WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_API __declspec (dllexport)
#else
#define WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_API __declspec (dllimport)
#endif

#pragma managed(push, off)

#include "WindowsUtilities/File.h"
#include "WindowsUtilities/Thread.h"
#include "NativeFileSystemWatcher.h"

namespace Windows
{
namespace File
{
    class DelayedFileSystemWatcher : public NativeFileSystemWatcher
    {
    public:
        WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_API DelayedFileSystemWatcher( IFileSystemWatcher::FileSystemString const & dir,
                                                                            ::DWORD changeFlags,
                                                                            ::BOOL const watchSubDir,
                                                                            IFileSystemWatcher * const eventHandler,
                                                                            IFileSystemWatcher::FileSystemString includeFilter,
                                                                            IFileSystemWatcher::FileSystemString excludeFilter,
                                                                            bool restartOnError,
                                                                            unsigned int delay,
                                                                            std::vector<::BYTE>::size_type const bufferSize
                                                                          );

        WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_API virtual ~DelayedFileSystemWatcher();
    private:
        unsigned int _delay;

        DelayedFileSystemWatcher           (const DelayedFileSystemWatcher&) = delete;
        DelayedFileSystemWatcher& operator=(const DelayedFileSystemWatcher&) = delete;
    }; // class DelayedFileSystemWatcher
} // namespace File
} // namespace Windows

#pragma managed(pop)

#endif //#ifndef WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_H__