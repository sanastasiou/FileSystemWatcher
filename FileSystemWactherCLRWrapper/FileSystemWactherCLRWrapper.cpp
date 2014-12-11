// This is the main DLL file.
#include "stdafx.h"
#include "FileSystemWactherCLRWrapper.h"
#include "DirectoryChangeHandler.h"
#include "CDirectoryChangeWatcher.h"
#include "NotificationClass.h"
#include <vcclr.h>
#include <atlstr.h>
#include <stdio.h>
#using <mscorlib.dll>
using namespace System;
using namespace System::Runtime::InteropServices;

namespace FileSystemWactherCLRWrapper
{
    FileSystemWatcher::FileSystemWatcher( String^ dir, bool hasGUI, String^ include, String^ exclude, DWORD filterFlags, bool includeSubDir )
    {
        Directory             = dir;
        Include               = include;
        Exclude               = exclude;
        FilterFlags           = filterFlags;
        HasGUI                = hasGUI;
        MonitorSubDirectories = includeSubDir;
        this->_pDirectoryWatcher = new CDirectoryChangeWatcher( false );
        this->_pDirectoryChangeHandler = new NotificationClass( this, hasGUI, this->getCString( include ), this->getCString( exclude), filterFlags );
        this->_pWatchedDirectory = (char*)(void*)Marshal::StringToHGlobalAnsi(dir);
        this->_pDirectoryWatcher->WatchDirectory( CString( this->_pWatchedDirectory ),
                                                  filterFlags,
                                                  this->_pDirectoryChangeHandler,
                                                  includeSubDir,
                                                  this->getCString( include ),
                                                  this->getCString( exclude)
                                                );

    }

    CString FileSystemWatcher::getCString( String^ systemString )
    {
        char * aStr = (char*)(void*)Marshal::StringToHGlobalAnsi(systemString);
        CString aTemp(aStr);
        Marshal::FreeHGlobal( (System::IntPtr)(aStr) );
        return aTemp;
    }

    void FileSystemWatcher::OnFileNameChanged(const CString & strOldFileName, const CString & strNewFileName)
    {
        RenamedEventArgs^ aTempArgs = gcnew RenamedEventArgs( System::IO::WatcherChangeTypes::Renamed, gcnew String( System::IO::Path::GetDirectoryName( gcnew String( strNewFileName ) ) ), System::IO::Path::GetFileName( gcnew String( strNewFileName )), System::IO::Path::GetFileName( gcnew String( strOldFileName )));
        this->RaiseOnFileNameChanged( aTempArgs );
    }

    void FileSystemWatcher::OnReadDirectoryChangesError( DWORD dwError, const CString & strDirectoryName )
    {
        ErrorEventArgs^ aTempArgs = gcnew ErrorEventArgs( gcnew System::Exception( String::Format( "Error while observing directory : {0}. Error code : {1}", gcnew String( strDirectoryName ), dwError ) ) );
        this->RaiseOnReadDirectoryChangesError( aTempArgs );
    }

    void FileSystemWatcher::OnFileAdded( const CString & strFileName )
    {
        FileSystemEventArgs^ aTempArgs = gcnew FileSystemEventArgs( System::IO::WatcherChangeTypes::Created, gcnew String( System::IO::Path::GetDirectoryName( gcnew String( strFileName ) ) ), System::IO::Path::GetFileName( gcnew String( strFileName )) );
        this->RaiseOnFileAdded( aTempArgs );
    }

    void FileSystemWatcher::OnFileRemoved( const CString & strFileName )
    {
        FileSystemEventArgs^ aTempArgs = gcnew FileSystemEventArgs( System::IO::WatcherChangeTypes::Deleted, gcnew String( System::IO::Path::GetDirectoryName( gcnew String( strFileName ) ) ), System::IO::Path::GetFileName( gcnew String( strFileName )) );
        this->RaiseOnFileRemoved( aTempArgs );
    }

    void FileSystemWatcher::OnFileModified( const CString & strFileName )
    {
        FileSystemEventArgs^ aTempArgs = gcnew FileSystemEventArgs( System::IO::WatcherChangeTypes::Changed, gcnew String( System::IO::Path::GetDirectoryName( gcnew String( strFileName ) ) ), System::IO::Path::GetFileName( gcnew String( strFileName )) );
        this->RaiseOnFileModified( aTempArgs );
    }

    FileSystemWatcher::~FileSystemWatcher()
    {
        delete this->_pDirectoryWatcher;
        delete this->_pDirectoryChangeHandler;
        Marshal::FreeHGlobal( (System::IntPtr)(this->_pWatchedDirectory) );
    }
}