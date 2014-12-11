#include "stdafx.h"
#include "NotificationClass.h"
#include "FileSystemWactherCLRWrapper.h"

namespace FileSystemWactherCLRWrapper
{
    NotificationClass::NotificationClass( FileSystemWatcher^ fileSystemWatcherPointer, bool hasApplicationGUI, CString const & include, CString const & exclude, DWORD const dwFilterFlags )
        : CDelayedDirectoryChangeHandler( this, hasApplicationGUI, include, exclude, dwFilterFlags )
    {
        this->_pFileSystemWatcher = fileSystemWatcherPointer; 
    }

    void NotificationClass::On_FileNameChanged(const CString & strOldFileName, const CString & strNewFileName)
    {
        this->_pFileSystemWatcher->OnFileNameChanged(strOldFileName, strNewFileName);
    }

    void NotificationClass::On_ReadDirectoryChangesError( DWORD dwError, const CString & strDirectoryName )
    {
        this->_pFileSystemWatcher->OnReadDirectoryChangesError(dwError, strDirectoryName);
    }

    void NotificationClass::On_FileAdded( const CString & strFileName)
    {
        this->_pFileSystemWatcher->OnFileAdded( strFileName );
    }

    void NotificationClass::On_FileRemoved( const CString & strFileName)
    {
        this->_pFileSystemWatcher->OnFileRemoved( strFileName );
    }

    void NotificationClass::On_FileModified( const CString & strFileName)
    {
        this->_pFileSystemWatcher->OnFileModified( strFileName );
    }

    void NotificationClass::On_WatchStarted( DWORD dwError, const CString & strDirectoryName )
    {
        if( dwError != 0 )
        {
            this->_pFileSystemWatcher->OnReadDirectoryChangesError( dwError, strDirectoryName );
        }
    }

    void NotificationClass::On_WatchStopped( const CString & strDirectoryName)
    {
        this->_pFileSystemWatcher->OnReadDirectoryChangesError( 0, strDirectoryName );
    }
}