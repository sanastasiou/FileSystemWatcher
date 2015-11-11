#ifndef WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_H__
#define WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_H__

#if defined(WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_DLL_EXPORTS)
#define WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_API __declspec (dllexport)
#else
#define WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_API __declspec (dllimport)
#endif

#include "WindowsBase/File.h"
#include "WindowsBase/Thread.h"
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
    }; // class DelayedFileSystemWatcher
} // namespace File
} // namespace Windows
#endif //#ifndef WINDOWS_FILE_DELAYEDFILESYSTEMWATCHER_H__