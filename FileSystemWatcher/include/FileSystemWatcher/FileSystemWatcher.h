#ifndef WINDOWS_FILE_FILESYSTEMWATCHER_H__
#define WINDOWS_FILE_FILESYSTEMWATCHER_H__

#include "Windows.h"
#include "FileSystemWatcher/IFileSystemWatcher.h"
#include "FileSystemWatcher/NativeSystemWatcher.h"
#include "FileSystemWatcher/DelayedFileSystemWatcher.h"
#include "FileSystemWatcher/EventRouter.h"

using namespace System;
using namespace System::IO;

namespace Windows
{
namespace Clr
{
    public ref class FileWatcherBase
    {
    public:
        event System::EventHandler<FileSystemEventArgs^>^ Changed;
        event System::EventHandler<FileSystemEventArgs^>^ Created;
        event System::EventHandler<FileSystemEventArgs^>^ Deleted;
        event System::EventHandler<RenamedEventArgs^>^    Renamed;
        event System::EventHandler<ErrorEventArgs^>^      Error;

        void OnFileModified(const File::IFileSystemWatcher::FileSystemString & strFileName);

        FileWatcherBase(EventRouter * eventRouter) :
            _pEventRouter(eventRouter)
        {
        }

        ~FileWatcherBase()
        {
            ::delete _pEventRouter;
        }
    protected:
        EventRouter * _pEventRouter;
    };

    public ref class FileWatcher : public FileWatcherBase
    {
    public:
        FileWatcher(String^ dir, ::DWORD filterFlags, bool includeSubDir, String^ include, String^ exclude);
    private:
        File::NativeFileSystemWatcher * _pDirectoryWatcher;
    };

    public ref class DelayedFileWatcher : public FileWatcherBase
    {
    public:
        DelayedFileWatcher(String^ dir, String^ include, String^ exclude, ::DWORD filterFlags, bool includeSubDir, ::DWORD const delay);
    private:
        File::DelayedFileSystemWatcher * _pDirectoryWatcher;
    };

    //typedef ref class FileSystemWatcher<File::DelayedFileSystemWatcher> DelayedFileWatcher;
} // namespace File
} // namespace Windows

#endif //#ifndef WINDOWS_FILE_FILESYSTEMWATCHER_H__
