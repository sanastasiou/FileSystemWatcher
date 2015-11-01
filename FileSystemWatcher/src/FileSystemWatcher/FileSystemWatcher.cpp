#include "FileSystemWatcher/FileSystemWatcher.h"
#include "FileSystemWatcher/NativeSystemWatcher.h"
#include "FileSystemWatcher/EventRouter.h"
#include <msclr/marshal_cppstd.h>

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace msclr::interop;

namespace Windows
{
namespace Clr
{
    FileWatcher::FileWatcher(String^ dir, ::DWORD filterFlags, bool includeSubDir, String^ include, String^ exclude) :
        FileWatcherBase(new EventRouter(this)),
        _pDirectoryWatcher(nullptr)
    {
        _pDirectoryWatcher = new File::NativeFileSystemWatcher( marshal_as<File::IFileSystemWatcher::FileSystemString>(dir), 
                                                                filterFlags, 
                                                                includeSubDir,
                                                                _pEventRouter, 
                                                                marshal_as<File::IFileSystemWatcher::FileSystemString>(include),
                                                                marshal_as<File::IFileSystemWatcher::FileSystemString>(exclude)
                                                              );
    }

    DelayedFileWatcher::DelayedFileWatcher(String^ dir, String^ include, String^ exclude, ::DWORD filterFlags, bool includeSubDir, ::DWORD const delay) :
        FileWatcherBase(new EventRouter(this)),
        _pDirectoryWatcher(nullptr)
    {
        //_pDirectoryWatcher = new EventRouter<File::DelayedFileSystemWatcher>(this);
    }

    void FileWatcherBase::OnFileModified(const File::IFileSystemWatcher::FileSystemString & strFileName)
    {
    }

    /*void FileSystemWatcher::RestartWatching()
    {
        if (!_isWatching)
        {
            _isWatching = true;
        }
    }

    CString FileSystemWatcher::getCString(String^ systemString)
    {
        char * aStr = (char*)(void*)Marshal::StringToHGlobalAnsi(systemString);
        CString aTemp(aStr);
        Marshal::FreeHGlobal((System::IntPtr)(aStr));
        return aTemp;
    }

    void FileSystemWatcher::OnFileNameChanged(const CString & strOldFileName, const CString & strNewFileName)
    {
        if (_isWatching)
        {
            RenamedEventArgs^ aTempArgs = gcnew RenamedEventArgs(System::IO::WatcherChangeTypes::Renamed, gcnew String(System::IO::Path::GetDirectoryName(gcnew String(strNewFileName))), System::IO::Path::GetFileName(gcnew String(strNewFileName)), System::IO::Path::GetFileName(gcnew String(strOldFileName)));
            this->RaiseOnFileNameChanged(aTempArgs);
        }
    }

    void FileSystemWatcher::OnReadDirectoryChangesError(::DWORD dwError, const CString & strDirectoryName)
    {
        if (_isWatching)
        {
            ErrorEventArgs^ aTempArgs = gcnew ErrorEventArgs(gcnew System::Exception(String::Format("Error while observing directory : {0}. Error code : {1}", gcnew String(strDirectoryName), dwError)));
            this->RaiseOnReadDirectoryChangesError(aTempArgs);
        }
    }

    void FileSystemWatcher::OnFileAdded(const CString & strFileName)
    {
        if (_isWatching)
        {
            FileSystemEventArgs^ aTempArgs = gcnew FileSystemEventArgs(System::IO::WatcherChangeTypes::Created, gcnew String(System::IO::Path::GetDirectoryName(gcnew String(strFileName))), System::IO::Path::GetFileName(gcnew String(strFileName)));
            this->RaiseOnFileAdded(aTempArgs);
        }
    }

    void FileSystemWatcher::OnFileRemoved(const CString & strFileName)
    {
        if (_isWatching)
        {
            FileSystemEventArgs^ aTempArgs = gcnew FileSystemEventArgs(System::IO::WatcherChangeTypes::Deleted, gcnew String(System::IO::Path::GetDirectoryName(gcnew String(strFileName))), System::IO::Path::GetFileName(gcnew String(strFileName)));
            this->RaiseOnFileRemoved(aTempArgs);
        }
    }

    void FileSystemWatcher::OnFileModified(const CString & strFileName)
    {
        if (_isWatching)
        {
            FileSystemEventArgs^ aTempArgs = gcnew FileSystemEventArgs(System::IO::WatcherChangeTypes::Changed, gcnew String(System::IO::Path::GetDirectoryName(gcnew String(strFileName))), System::IO::Path::GetFileName(gcnew String(strFileName)));
            this->RaiseOnFileModified(aTempArgs);
        }
    }

    FileSystemWatcher::~FileSystemWatcher()
    {
        StopWatching();
        delete this->_pDirectoryWatcher;
        delete this->_pDirectoryChangeHandler;
        Marshal::FreeHGlobal((System::IntPtr)(this->_pWatchedDirectory));
        Marshal::FreeHGlobal((System::IntPtr)(this->_exclude));
        Marshal::FreeHGlobal((System::IntPtr)(this->_include));
    }*/
} // namespace Clr
} // namespace Windows
