#ifndef WINDOWS_FILE_FILEWATCHER_H__
#define WINDOWS_FILE_FILEWATCHER_H__

#include "Windows.h"
#include "NativeFileSystemWatcher/IFileSystemWatcher.h"
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

        virtual void OnFileModified(const File::IFileSystemWatcher::FileSystemString & strFileName);

        virtual void OnFileRenamed(const File::IFileSystemWatcher::FileSystemString & newFileName, const File::IFileSystemWatcher::FileSystemString & oldFileName);

        virtual void OnFileRemoved(const File::IFileSystemWatcher::FileSystemString & strFileName);

        virtual void OnFileAdded(const File::IFileSystemWatcher::FileSystemString & strFileName);

        virtual void OnError(const ::DWORD errorCode, const File::IFileSystemWatcher::FileSystemString & directory);

        FileWatcherBase(EventRouter * eventRouter) :
            _pEventRouter(eventRouter)
        {
        }

        ~FileWatcherBase()
        {
            if (_pEventRouter != nullptr)
            {
                ::delete _pEventRouter;
                _pEventRouter = nullptr;
            }
        }

        static const std::vector< ::BYTE >::size_type STANDARD_BUFFER_SIZE = 65535U;
    protected:
        EventRouter * _pEventRouter;
    };

    public ref class FileWatcher : public FileWatcherBase
    {
    public:
        FileWatcher(String^ dir, ::DWORD filterFlags, bool includeSubDir, String^ include, String^ exclude, bool restartOnError, std::vector<::BYTE>::size_type bufferSize);

        bool IsWatching();

        ~FileWatcher();

    private:
        File::FileSystemWatcherBase * _pDirectoryWatcher;
    };

    public ref class DelayedFileWatcher : public FileWatcherBase
    {
    public:
        DelayedFileWatcher(String^ dir, String^ include, String^ exclude, bool restartOnError, ::DWORD filterFlags, bool includeSubDir, ::DWORD const delay);

        ~DelayedFileWatcher();

    private:
        File::FileSystemWatcherBase * _pDirectoryWatcher;
    };
} // namespace File
} // namespace Windows

#endif //#ifndef WINDOWS_FILE_FILESYSTEMWATCHER_H__
