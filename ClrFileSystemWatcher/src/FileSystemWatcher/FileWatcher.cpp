#include "FileSystemWatcher/FileWatcher.h"
#include "NativeFileSystemWatcher/NativeFileSystemWatcher.h"
#include "NativeFileSystemWatcher/DelayedFileSystemWatcher.h"
#include "FileSystemWatcher/EventRouter.h"
#include <msclr/marshal_cppstd.h>
#include "WindowsUtilities/Base.h"

namespace Windows
{
namespace Clr
{
    FileWatcher::FileWatcher(String^ dir, ::DWORD filterFlags, bool includeSubDir, String^ include, String^ exclude, bool restartOnError, std::vector<::BYTE>::size_type bufferSize) :
        FileWatcherBase(nullptr),
        _pDirectoryWatcher(nullptr)
    {
        _pEventRouter = new EventRouter(this);
        _pDirectoryWatcher = new File::NativeFileSystemWatcher( msclr::interop::marshal_as<File::IFileSystemWatcher::FileSystemString>(dir),
                                                                filterFlags, 
                                                                includeSubDir,
                                                                _pEventRouter, 
                                                                msclr::interop::marshal_as<File::IFileSystemWatcher::FileSystemString>(include),
                                                                msclr::interop::marshal_as<File::IFileSystemWatcher::FileSystemString>(exclude),
                                                                restartOnError,
                                                                bufferSize
                                                              );
    }

    bool FileWatcher::IsWatching()
    {
        return _pDirectoryWatcher->IsWatching();
    }

    void FileWatcherBase::OnFileModified(const File::IFileSystemWatcher::FileSystemString & strFileName)
    {
        System::String^ aClrStrFileName(msclr::interop::marshal_as<System::String^>(Utilities::File::GetFileFromFilePath(strFileName.c_str())));
        System::String^ aClrDir(msclr::interop::marshal_as<System::String^>(Utilities::File::GetDirectoryFromFilePath(strFileName.c_str())));
        Changed(this, gcnew FileSystemEventArgs(System::IO::WatcherChangeTypes::Changed, aClrDir, aClrStrFileName));
    }

    void FileWatcherBase::OnFileRenamed(const File::IFileSystemWatcher::FileSystemString & newFileName, const File::IFileSystemWatcher::FileSystemString & oldFileName)
    {
        System::String^ aClrNewFileName(msclr::interop::marshal_as<System::String^>(Utilities::File::GetFileFromFilePath(newFileName.c_str())));
        System::String^ aClrOldFileName(msclr::interop::marshal_as<System::String^>(Utilities::File::GetFileFromFilePath(oldFileName.c_str())));
        System::String^ aClrDir(msclr::interop::marshal_as<System::String^>(Utilities::File::GetDirectoryFromFilePath(oldFileName.c_str())));
        Renamed(this, gcnew RenamedEventArgs(System::IO::WatcherChangeTypes::Renamed,  aClrDir, aClrNewFileName, aClrOldFileName));
    }

    void FileWatcherBase::OnFileRemoved(const File::IFileSystemWatcher::FileSystemString & strFileName)
    {
        System::String^ aClrStrFileName(msclr::interop::marshal_as<System::String^>(Utilities::File::GetFileFromFilePath(strFileName.c_str())));
        System::String^ aClrDir(msclr::interop::marshal_as<System::String^>(Utilities::File::GetDirectoryFromFilePath(strFileName.c_str())));
        Deleted(this, gcnew FileSystemEventArgs(System::IO::WatcherChangeTypes::Deleted, aClrDir, aClrStrFileName));
    }

    void FileWatcherBase::OnFileAdded(const File::IFileSystemWatcher::FileSystemString & strFileName)
    {
        System::String^ aClrStrFileName(msclr::interop::marshal_as<System::String^>(Utilities::File::GetFileFromFilePath(strFileName.c_str())));
        System::String^ aClrDir(msclr::interop::marshal_as<System::String^>(Utilities::File::GetDirectoryFromFilePath(strFileName.c_str())));
        Created(this, gcnew FileSystemEventArgs(System::IO::WatcherChangeTypes::Created, aClrDir, aClrStrFileName));
    }

    void FileWatcherBase::OnError(const ::DWORD errorCode, const File::IFileSystemWatcher::FileSystemString & directory)
    {
        System::String^ aClrDir(msclr::interop::marshal_as<System::String^>(directory));
        Error(this, gcnew ErrorEventArgs(gcnew System::Exception(String::Format("Error while observing directory : {0}. Error code : {1}", aClrDir, errorCode))));
    }

    FileWatcher::~FileWatcher()
    {
        if (_pDirectoryWatcher != nullptr)
        {
            ::delete _pDirectoryWatcher;
            _pDirectoryWatcher = nullptr;
        }
    }

    FileWatcher::!FileWatcher()
    {

    }

    DelayedFileWatcher::DelayedFileWatcher(String^ dir, ::DWORD filterFlags, bool includeSubDir, String^ include, String^ exclude, bool restartOnError, ::DWORD const delay, std::vector<::BYTE>::size_type bufferSize) :
        FileWatcherBase(nullptr),
        _pDirectoryWatcher(nullptr)
    {
        _pEventRouter = new EventRouter(this);
        _pDirectoryWatcher = new File::DelayedFileSystemWatcher( msclr::interop::marshal_as<File::IFileSystemWatcher::FileSystemString>(dir),
                                                                filterFlags, 
                                                                includeSubDir,
                                                                _pEventRouter, 
                                                                msclr::interop::marshal_as<File::IFileSystemWatcher::FileSystemString>(include),
                                                                msclr::interop::marshal_as<File::IFileSystemWatcher::FileSystemString>(exclude),
                                                                restartOnError,
                                                                delay,
                                                                bufferSize
                                                              );
    }

    DelayedFileWatcher::~DelayedFileWatcher()
    {
        ::delete _pDirectoryWatcher;
    }

    DelayedFileWatcher::!DelayedFileWatcher()
    {
    }
} // namespace Clr
} // namespace Windows
