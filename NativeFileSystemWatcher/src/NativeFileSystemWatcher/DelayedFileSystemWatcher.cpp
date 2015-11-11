#include "NativeFileSystemWatcher/DelayedFileSystemWatcher.h"

namespace Windows
{
namespace File
{
    DelayedFileSystemWatcher::DelayedFileSystemWatcher(IFileSystemWatcher::FileSystemString const & dir,
                                                       ::DWORD changeFlags,
                                                       ::BOOL const watchSubDir,
                                                       IFileSystemWatcher * const eventHandler,
                                                       IFileSystemWatcher::FileSystemString includeFilter,
                                                       IFileSystemWatcher::FileSystemString excludeFilter,
                                                       bool restartOnError,
                                                       unsigned int delay,
                                                       std::vector<::BYTE>::size_type const bufferSize
                                                       ) :
        NativeFileSystemWatcher(dir, changeFlags, watchSubDir, eventHandler, includeFilter, excludeFilter, restartOnError, bufferSize),
        _delay(delay)
    {
    }


    DelayedFileSystemWatcher::~DelayedFileSystemWatcher()
    {
    }

} //namespace File
} //namespace Windows
