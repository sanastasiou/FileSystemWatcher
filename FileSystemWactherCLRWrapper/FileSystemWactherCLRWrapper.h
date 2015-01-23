// FileSystemWactherCLRWrapper.h

#pragma once
#include "FileSystemWactherCLRWrapper.h"
#include "DirectoryChangeHandler.h"
#include "CDirectoryChangeWatcher.h"
#include "NotificationClass.h"

using namespace System;
using namespace System::IO;

namespace FileSystemWactherCLRWrapper {

	public ref class FileSystemWatcher
	{
    public:
        event System::EventHandler<FileSystemEventArgs^>^ Changed;
        event System::EventHandler<FileSystemEventArgs^>^ Created;
        event System::EventHandler<FileSystemEventArgs^>^ Deleted;
        event System::EventHandler<RenamedEventArgs^>^    Renamed;
        event System::EventHandler<ErrorEventArgs^>^      Error;

        /**
         * \fn  FileSystemWatcher::FileSystemWatcher( String^ dir, bool hasGUI, String^ include,
         *      String^ exclude, DWORD filterFlags, bool includeSubDir );
         *
         * \brief   Constructor.
         *
         * \author  Stefanos Anastasiou
         * \date    10.08.2013
         *
         * \param   dir             The dir.
         * \param   hasGUI          true if this object has graphical user interface.
         * \param   include         the include filter.
         * \param   exclude         the exclude filter.
         * \param   filterFlags     the filter flags.
         * \param   includeSubDir   the include sub dir flag.
         */
        FileSystemWatcher( String^ dir, bool hasGUI, String^ include, String^ exclude, DWORD filterFlags, bool includeSubDir );

        /**
         * \fn  FileSystemWatcher::~FileSystemWatcher();
         *
         * \brief   Destructor.
         *
         * \author  Stefanos Anastasiou
         * \date    10.08.2013
         */
        ~FileSystemWatcher();

        /**
         * \property    property String^ Directory
         *
         * \brief   Gets the pathname of the directory being monitored.
         *
         * \return  The pathname of the directory.
         */
        property String^ Directory;

        /**
         * \property    property String^ Include
         *
         * \brief   Gets the include filter.
         *
         * \return  The include.
         */
        property String^ Include;

        /**
         * \property    property String^ Exclude
         *
         * \brief   Gets the exclude filter.
         *
         * \return  The exclude.
         */
        property String^ Exclude;

        /**
         * \property    property DWORD FilterFlags
         *
         * \brief   Gets the filter flags.
         *
         * \return  The filter flags.
         */
        property DWORD FilterFlags;

        /**
         * \property    property bool HasGUI
         *
         * \brief   Gets a value indicating whether this object has graphical user interface.
         *
         * \return  true if this object has graphical user interface, false if not.
         */
        property bool HasGUI;

        /**
         * \property    property bool MonitorSubDirectories
         *
         * \brief   Gets a value indicating whether to monitor sub directories.
         *
         * \return  true if monitor sub directories, false if not.
         */
        property bool MonitorSubDirectories;

        void StopWatching()
        {
            if(_isWatching)
            {
                _isWatching = false;
            }
        }

        void RestartWatching();
    internal:
        /**
         * \fn  void FileSystemWatcher::OnFileNameChanged();
         *
         * \brief   Executes the file name changed action.
         *
         * \author  Stefanos Anastasiou
         * \date    10.08.2013
         */
        void OnFileNameChanged(const CString & strOldFileName, const CString & strNewFileName);

        /**
         * \fn  void FileSystemWatcher::RaisedOnFileNameChanged( FileSystemEventArgs^ e)
         *
         * \brief   Raises on file name changed managed event.
         *
         * \author  Stefanos Anastasiou
         * \date    10.08.2013
         *
         * \param [in,out]  e   If non-null, the FileSystemEventArgs^ to process.
         */
        void RaiseOnFileNameChanged( RenamedEventArgs^ e) { Renamed( this, e ); }

        /**
         * \fn  void FileSystemWatcher::OnReadDirectoryChangesError( DWORD dwError,
         *      const CString & strDirectoryName );
         *
         * \brief   Executes the read directory changes error action.
         *
         * \author  Stefanos Anastasiou
         * \date    10.08.2013
         *
         * \param   dwError             The error.
         * \param   strDirectoryName    Pathname of the directory.
         */
        void OnReadDirectoryChangesError( DWORD dwError, const CString & strDirectoryName );

        /**
         * \fn  void FileSystemWatcher::RaiseOnReadDirectoryChangesError( ErrorEventArgs^ e )
         *
         * \brief   Raises the on read directory changes error event.
         *
         * \author  Stefanos Anastasiou
         * \date    10.08.2013
         *
         * \param [in,out]  e   If non-null, the ErrorEventArgs^ to process.
         */
        void RaiseOnReadDirectoryChangesError( ErrorEventArgs^ e ) { Error( this, e ); }

        /**
         * \fn  void FileSystemWatcher::OnFileAdded( const CString & strFileName );
         *
         * \brief   Executes the file added action.
         *
         * \author  Stefanos Anastasiou
         * \date    10.08.2013
         *
         * \param   strFileName Filename of the file.
         */
        void OnFileAdded( const CString & strFileName );

        /**
         * \fn  void FileSystemWatcher::RaiseOnFileAdded( FileSystemEventArgs^ e )
         *
         * \brief   Raises the on file added event.
         *
         * \author  Stefanos Anastasiou
         * \date    10.08.2013
         *
         * \param [in,out]  e   If non-null, the FileSystemEventArgs^ to process.
         */
        void RaiseOnFileAdded( FileSystemEventArgs^ e ) { Created( this, e ); }

                /**
         * \fn  void FileSystemWatcher::OnFileRemoved( const CString & strFileName );
         *
         * \brief   Executes the file removed action.
         *
         * \author  Stefanos Anastasiou
         * \date    10.08.2013
         *
         * \param   strFileName Filename of the file.
         */
        void OnFileRemoved( const CString & strFileName );

        /**
         * \fn  void FileSystemWatcher::RaiseOnFileRemoved( FileSystemEventArgs^ e)
         *
         * \brief   Raises the on file removed event.
         *
         * \author  Stefanos Anastasiou
         * \date    10.08.2013
         *
         * \param [in,out]  e   If non-null, the FileSystemEventArgs^ to process.
         */
        void RaiseOnFileRemoved( FileSystemEventArgs^ e) { Deleted( this, e ); }

        /**
         * \fn  void FileSystemWatcher::OnFileModified( const CString & strFileName );
         *
         * \brief   Executes the file modified action.
         *
         * \author  Stefanos Anastasiou
         * \date    10.08.2013
         *
         * \param   strFileName Filename of the file.
         */
        void OnFileModified( const CString & strFileName );

        /**
         * \fn  void FileSystemWatcher::RaiseOnFileModified( FileSystemEventArgs^ e)
         *
         * \brief   Raises the on file modified event.
         *
         * \author  Stefanos Anastasiou
         * \date    10.08.2013
         *
         * \param [in,out]  e   If non-null, the FileSystemEventArgs^ to process.
         */
        void RaiseOnFileModified( FileSystemEventArgs^ e) { Changed( this, e ); }
    private:
        CDirectoryChangeWatcher * _pDirectoryWatcher;       //!< The directory watcher.
        CDirectoryChangeHandler * _pDirectoryChangeHandler; //!< The directory notifications handler.
        char                    * _pWatchedDirectory;       //!< The directory being watched.
        bool                      _hasGUI;                  
        char*                     _include;
        char*                     _exclude;
        DWORD                     _filterFlags;
        bool                      _includeSubDir;
        bool                      _isWatching;

        /**
         * \fn  CString FileSystemWatcher::getCString( String^ systemString );
         *
         * \brief   Converts a System::String to a CString.
         *
         * \author  Stefanos Anastasiou
         * \date    11.08.2013
         *
         * \param [in,out]  systemString    If non-null, the system string.
         *
         * \return  The c string.
         */
        CString getCString( String^ systemString );
	};      
}
