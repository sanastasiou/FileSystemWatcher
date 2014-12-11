#pragma once

#include "CDelayedDirectoryChangeHandler.h"
#include <vcclr.h>

namespace FileSystemWactherCLRWrapper
{
    ref class FileSystemWatcher;
    class NotificationClass : public CDelayedDirectoryChangeHandler
    {
    public:
        NotificationClass( FileSystemWatcher^ fileSystemWatcherPointer, bool hasApplicationGUI, CString const & include, CString const & exclude, DWORD const dwFilterFlags );
        virtual ~NotificationClass(void){};

        void On_FileNameChanged(const CString & strOldFileName, const CString & strNewFileName);

        void On_FileAdded(const CString & strFileName);

        void On_FileRemoved(const CString & strFileName);

        void On_FileModified(const CString & strFileName);

        void On_ReadDirectoryChangesError(DWORD dwError, const CString & strDirectoryName);

        void On_WatchStarted(DWORD dwError, const CString & strDirectoryName);

        void On_WatchStopped(const CString & strDirectoryName);

        /**
         * \fn  bool NotificationClass::On_FilterNotification(DWORD dwNotifyAction,
         *      LPCTSTR szFileName, LPCTSTR szNewFileName)
         *
         * \brief   Executes the filter notification action.
         * \note    Override if you want to filter out some modifications.
         * \author  Stefanos Anastasiou
         * \date    10.08.2013
         *
         * \param   dwNotifyAction  The notify action.
         * \param   szFileName      Filename of the file.
         * \param   szNewFileName   Filename of the new file.
         *
         * \return  true if it succeeds, false if it fails.
         */
        bool On_FilterNotification(DWORD dwNotifyAction, LPCTSTR szFileName, LPCTSTR szNewFileName)
        {
            return CDirectoryChangeHandler::On_FilterNotification( dwNotifyAction, szFileName, szNewFileName );
        }

    private:
        gcroot<FileSystemWatcher^> _pFileSystemWatcher;
    };
}

