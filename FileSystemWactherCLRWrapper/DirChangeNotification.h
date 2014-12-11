#pragma once

#include <afxmt.h>
#include <afxtempl.h>

class CDelayedDirectoryChangeHandler;

class CDirChangeNotification
//
//	 A class to help dispatch the change notifications to the main thread.
//
//	 This class holds the data in memory until the notification can be dispatched.(ie: this is the time between when the notification is posted, and the clients notification code is called).
//
//
{
private:
	CDirChangeNotification();//not implemented
public:
	explicit CDirChangeNotification(CDelayedDirectoryChangeHandler * pDelayedHandler, DWORD dwPartialPathOffset);
	~CDirChangeNotification();

	//
	//
	void PostOn_FileAdded(LPCTSTR szFileName);
	void PostOn_FileRemoved(LPCTSTR szFileName);
	void PostOn_FileNameChanged(LPCTSTR szOldName, LPCTSTR szNewName);
	void PostOn_FileModified(LPCTSTR szFileName);
	void PostOn_ReadDirectoryChangesError(DWORD dwError, LPCTSTR szDirectoryName);
	void PostOn_WatchStarted(DWORD dwError, LPCTSTR szDirectoryName);
	void PostOn_WatchStopped(LPCTSTR szDirectoryName);

	void DispatchNotificationFunction();


	enum eFunctionToDispatch{	eFunctionNotDefined = -1,
								eOn_FileAdded		= FILE_ACTION_ADDED, 
								eOn_FileRemoved		= FILE_ACTION_REMOVED, 
								eOn_FileModified	= FILE_ACTION_MODIFIED,
								eOn_FileNameChanged	= FILE_ACTION_RENAMED_OLD_NAME,
								eOn_ReadDirectoryChangesError,
								eOn_WatchStarted,
								eOn_WatchStopped
	};	
protected:
	void PostNotification();
	
private:
	friend class CDelayedDirectoryChangeHandler;
	CDelayedDirectoryChangeHandler * m_pDelayedHandler;

	//
	//	Members to help implement DispatchNotificationFunction
	//
	//

	eFunctionToDispatch m_eFunctionToDispatch;
	//Notification Data:
	TCHAR *	m_szFileName1;//<-- is the szFileName parameter to On_FileAdded(),On_FileRemoved,On_FileModified(), and is szOldFileName to On_FileNameChanged(). Is also strDirectoryName to On_ReadDirectoryChangesError(), On_WatchStarted(), and On_WatchStopped()
	TCHAR *	m_szFileName2;//<-- is the szNewFileName parameter to On_FileNameChanged()
	DWORD m_dwError;	  //<-- is the dwError parameter to On_WatchStarted(), and On_ReadDirectoryChangesError()
	//

	DWORD m_dwPartialPathOffset;//helps support FILTERS_CHECK_PARTIAL_PATH...not passed to any functions other than may be used during tests in CDelayedDirectoryChangeHandler::NotifyClientOfFileChange()


	friend class CDirChangeNotification;
	friend class CDirectoryChangeWatcher;
	friend DWORD GetPathOffsetBasedOnFilterFlags(CDirChangeNotification*,DWORD);//a friend function
};
