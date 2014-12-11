#include "Stdafx.h"
#include "CDirectoryChangeWatcher.h"
#include "Utilities.h"
#include "PrivilegeEnabler.h"
#include <afxmt.h>
#include <afxtempl.h>
#include <afxwin.h>
#include "CDelayedDirectoryChangeHandler.h"
#include "FileNotifyInformation.h"

CDirectoryChangeWatcher::CDirWatchInfo::CDirWatchInfo(HANDLE hDir, 
													  const CString & strDirectoryName, 
													  CDirectoryChangeHandler * pChangeHandler,
													  DWORD dwChangeFilter, 
													  BOOL bWatchSubDir,
													  bool bAppHasGUI,
													  LPCTSTR szIncludeFilter,
													  LPCTSTR szExcludeFilter,
													  DWORD dwFilterFlags)
 :	m_pChangeHandler( NULL ), 
	m_hDir(hDir),
	m_dwChangeFilter( dwChangeFilter ),
	m_bWatchSubDir( bWatchSubDir ),
	m_strDirName( strDirectoryName ),
	m_dwBufLength(0),
	m_dwReadDirError(ERROR_SUCCESS),
	m_StartStopEvent(FALSE, TRUE), //NOT SIGNALLED, MANUAL RESET
	m_RunningState( RUNNING_STATE_NOT_SET )
{ 
	
	ASSERT( hDir != INVALID_HANDLE_VALUE 
		&& !strDirectoryName.IsEmpty() );
	
	//
	//	This object 'decorates' the pChangeHandler passed in
	//	so that notifications fire in the context a thread other than
	//	CDirectoryChangeWatcher::MonitorDirectoryChanges()
	//
	//	Supports the include and exclude filters
	//
	//
	m_pChangeHandler = new CDelayedDirectoryChangeHandler( pChangeHandler, bAppHasGUI, szIncludeFilter, szExcludeFilter, dwFilterFlags );
	if( m_pChangeHandler ) 
		m_pChangeHandler->SetPartialPathOffset( m_strDirName );//to support FILTERS_CHECK_PARTIAL_PATH..this won't change for the duration of the watch, so set it once... HERE!
	ASSERT( m_pChangeHandler );

	ASSERT( GetChangeHandler() );
	ASSERT( GetRealChangeHandler() );
	if( GetRealChangeHandler() )
		GetRealChangeHandler()->AddRef();
	
	memset(&m_Overlapped, 0, sizeof(m_Overlapped));
	//memset(m_Buffer, 0, sizeof(m_Buffer));
}

CDirectoryChangeWatcher::CDirWatchInfo::~CDirWatchInfo()
{
	if( GetChangeHandler() )
	{//If this call to CDirectoryChangeHandler::Release() causes m_pChangeHandler to delete itself,
		//the dtor for CDirectoryChangeHandler will call CDirectoryChangeWatcher::UnwatchDirectory( CDirectoryChangeHandler * ),
		//which will make try to delete this object again.
		//if m_pChangeHandler is NULL, it won't try to delete this object again...
		CDirectoryChangeHandler * pTmp = SetRealDirectoryChangeHandler( NULL );
		if( pTmp )
			pTmp->Release();
		else{
			ASSERT( FALSE );
		}
	}
	
	CloseDirectoryHandle();

	delete m_pChangeHandler;
	m_pChangeHandler = NULL;
	
}

void CDirectoryChangeWatcher::CDirWatchInfo::DeleteSelf(CDirectoryChangeWatcher * pWatcher)
//
//	There's a reason for this function!
//
//	the dtor is private to enforce that it is used.
//
//
//	pWatcher can be NULL only if CDirecotryChangeHandler::ReferencesWatcher() has NOT been called.
//	ie: in certain sections of WatchDirectory() it's ok to pass this w/ NULL, but no where else.
//
{
	//ASSERT( pWatcher != NULL );


	ASSERT( GetRealChangeHandler() );
	if( pWatcher )
	{
	//
	//
	//	Before this object is deleted, the CDirectoryChangeHandler object
	//	needs to release it's reference count to the CDirectoryChangeWatcher object.
	//	I might forget to do this since I getting rid of CDirWatchInfo objects
	//	in more than one place...hence the reason for this function.
	//
		pWatcher->ReleaseReferenceToWatcher( GetRealChangeHandler() );
	}
	
	delete this;
}

CDelayedDirectoryChangeHandler* CDirectoryChangeWatcher::CDirWatchInfo::GetChangeHandler() const 
{ 
	return m_pChangeHandler; 
}

CDirectoryChangeHandler * CDirectoryChangeWatcher::CDirWatchInfo::GetRealChangeHandler() const
//
//	The 'real' change handler is the CDirectoryChangeHandler object 
//	passed to CDirectoryChangeWatcher::WatchDirectory() -- this is the object
//	that really handles the changes.
//
{	
	ASSERT( m_pChangeHandler ); 
	return m_pChangeHandler->GetRealChangeHandler(); 
}

CDirectoryChangeHandler * CDirectoryChangeWatcher::CDirWatchInfo::SetRealDirectoryChangeHandler(CDirectoryChangeHandler * pChangeHandler)
//
//	Allows you to switch out, at run time, which object really handles the change notifications.
//
{
	CDirectoryChangeHandler * pOld = GetRealChangeHandler();
	m_pChangeHandler->GetRealChangeHandler() = pChangeHandler; 
	return pOld;
}

BOOL CDirectoryChangeWatcher::CDirWatchInfo::CloseDirectoryHandle()
//
//	Closes the directory handle that was opened in CDirectoryChangeWatcher::WatchDirecotry()
//
//
{
	BOOL b = TRUE;
	if( m_hDir != INVALID_HANDLE_VALUE )
	{
		b = CloseHandle(m_hDir);
		m_hDir = INVALID_HANDLE_VALUE;
	}
	return b;
}

DWORD CDirectoryChangeWatcher::CDirWatchInfo::StartMonitor(HANDLE hCompPort)
/*********************************************
  Sets the running state of the object to perform the initial call to ReadDirectoryChangesW()
  , wakes up the thread waiting on GetQueuedCompletionStatus()
  and waits for an event to be set before returning....

  The return value is either ERROR_SUCCESS if ReadDirectoryChangesW is successfull,
  or is the value of GetLastError() for when ReadDirectoryChangesW() failed.
**********************************************/
{
	ASSERT( hCompPort );

	//Guard the properties of this object 
	VERIFY( LockProperties() );
	

		
	m_RunningState = RUNNING_STATE_START_MONITORING;//set the state member to indicate that the object is to START monitoring the specified directory
	PostQueuedCompletionStatus(hCompPort, sizeof(this), (DWORD)this, &m_Overlapped);//make the thread waiting on GetQueuedCompletionStatus() wake up

	VERIFY( UnlockProperties() );//unlock this object so that the thread can get at them...

	//wait for signal that the initial call 
	//to ReadDirectoryChanges has been made
	DWORD dwWait = 0;
	do{
		dwWait = WaitForSingleObject(m_StartStopEvent, 10 * 1000);
		if( dwWait != WAIT_OBJECT_0 )
		{
			//
			//	shouldn't ever see this one....but just in case you do, notify me of the problem wesj@hotmail.com.
			//
			TRACE(_T("WARNING! Possible lockup detected. FILE: %s Line: %d\n"), _T( __FILE__ ), __LINE__);
		}
	} while( dwWait != WAIT_OBJECT_0 );

	ASSERT( dwWait == WAIT_OBJECT_0 );
	m_StartStopEvent.ResetEvent();
	
	return m_dwReadDirError;//This value is set in the worker thread when it first calls ReadDirectoryChangesW().
}

BOOL CDirectoryChangeWatcher::CDirWatchInfo::UnwatchDirectory(HANDLE hCompPort)
/*******************************************

    Sets the running state of the object to stop monitoring a directory,
	Causes the worker thread to wake up and to stop monitoring the dierctory
	
********************************************/
{
	ASSERT( hCompPort );
	//
	// Signal that the worker thread is to stop watching the directory
	//
	if( SignalShutdown(hCompPort) )
	{
		//and wait for the thread to signal that it has shutdown
		return WaitForShutdown();

	}
	return FALSE;
}

BOOL CDirectoryChangeWatcher::CDirWatchInfo::SignalShutdown( HANDLE hCompPort )
//added to fix a bug -- this will be called normally by UnwatchDirectory(HANDLE)
//						and abnormally by the worker thread in the case that ReadDirectoryChangesW fails -- see code.
//
//	Signals the worker thread(via the I/O completion port) that it is to stop watching the 
//	directory for this object, and then returns.
//
{
	BOOL bRetVal = FALSE;
	ASSERT( hCompPort );
	ASSERT( m_hDir != INVALID_HANDLE_VALUE );
	//Lock the properties so that they aren't modified by another thread
	VERIFY( LockProperties() ); //unlikey to fail...
		
	//set the state member to indicate that the object is to stop monitoring the 
	//directory that this CDirWatchInfo is responsible for...
	m_RunningState = CDirectoryChangeWatcher::CDirWatchInfo::RUNNING_STATE_STOP;
	//put this object in the I/O completion port... GetQueuedCompletionStatus() will return it inside the worker thread.
	bRetVal = PostQueuedCompletionStatus(hCompPort, sizeof(CDirWatchInfo*), (DWORD)this, &m_Overlapped);

	if( !bRetVal )
	{
		TRACE(_T("PostQueuedCompletionStatus() failed! GetLastError(): %d\n"), GetLastError());
	}
	VERIFY( UnlockProperties() );
	
	return bRetVal;
}

BOOL CDirectoryChangeWatcher::CDirWatchInfo::WaitForShutdown()
//
//	This is to be called some time after SignalShutdown().
//	
//
{
	ASSERT_VALID(&m_StartStopEvent);
	
	//Wait for the Worker thread to indicate that the watch has been stopped
	DWORD dwWait;
	bool bWMQuitReceived = false;
	do{
		dwWait	= MsgWaitForMultipleObjects(1, &m_StartStopEvent.m_hObject, FALSE, 5000, QS_ALLINPUT);//wait five seconds
		switch( dwWait )
		{
		case WAIT_OBJECT_0:
			//handle became signalled!
			break;
		case WAIT_OBJECT_0 + 1:
			{
				//This thread awoke due to sent/posted message
				//process the message Q
				//
				//	There is a message in this thread's queue, so 
				//	MsgWaitForMultipleObjects returned.
				//	Process those messages, and wait again.

				MSG msg;
				while( PeekMessage(&msg, NULL, 0,0, PM_REMOVE ) ) 
				{
					if( msg.message != WM_QUIT)
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
					else
					{
						/****
						This appears to be causing quite a lot of pain, to quote Mustafa.

						//it's the WM_QUIT message, put it back in the Q and
						// exit this function
						PostMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam );
						bWMQuitReceived = true;

						****/
						break;
					}
				}
			}break;
		case WAIT_TIMEOUT:
			{
				TRACE(_T("WARNING: Possible Deadlock detected! ThreadID: %d File: %s Line: %d\n"), GetCurrentThreadId(), _T(__FILE__), __LINE__);
			}break;
		}//end switch(dwWait)
	}while( dwWait != WAIT_OBJECT_0 && !bWMQuitReceived );
		
	
	
	ASSERT( dwWait == WAIT_OBJECT_0 || bWMQuitReceived);

	m_StartStopEvent.ResetEvent();
	
	return (BOOL) (dwWait == WAIT_OBJECT_0 || bWMQuitReceived);
}

CDirectoryChangeWatcher::CDirectoryChangeWatcher(bool bAppHasGUI /*= true*/, DWORD dwFilterFlags/*=FILTERS_CHECK_FILE_NAME_ONLY*/)
: m_hCompPort( NULL )
 ,m_hThread( NULL )
 ,m_dwThreadID( 0UL )
 ,m_bAppHasGUI( bAppHasGUI )
 ,m_dwFilterFlags( dwFilterFlags == 0? FILTERS_DEFAULT_BEHAVIOR : dwFilterFlags)
{
	//NOTE:  
	//	The bAppHasGUI variable indicates that you have a message pump associated
	//	with the main thread(or the thread that first calls CDirectoryChangeWatcher::WatchDirectory() ).
	//	Directory change notifications are dispatched to your main thread.
	//	
	//	If your app doesn't have a gui, then pass false.  Doing so causes a worker thread
	//	to be created that implements a message pump where it dispatches/executes the notifications.
	//  It's ok to pass false even if your app does have a GUI.
	//	Passing false is required for Console applications, or applications without a message pump.
	//	Note that notifications are fired in a worker thread.
	//

	//NOTE:
	//
	//
}

CDirectoryChangeWatcher::~CDirectoryChangeWatcher()
{

	UnwatchAllDirectories();

	if( m_hCompPort )
	{
		CloseHandle( m_hCompPort );
		m_hCompPort = NULL;
	}
}

DWORD CDirectoryChangeWatcher::SetFilterFlags(DWORD dwFilterFlags)
//
//	SetFilterFlags()
//	
//	sets filter behavior for directories watched AFTER this function has been called.
//
//
//
{
	DWORD dwOld = m_dwFilterFlags;
	m_dwFilterFlags = dwFilterFlags;
	if( m_dwFilterFlags == 0 )
		m_dwFilterFlags = FILTERS_DEFAULT_BEHAVIOR;//the default.
	return dwOld;
}

BOOL CDirectoryChangeWatcher::IsWatchingDirectory(const CString & strDirName)const
/*********************************************
  Determines whether or not a directory is being watched

  be carefull that you have the same name path name, including the backslash
  as was used in the call to WatchDirectory().

	  ie:	
			"C:\\Temp"
		is different than
			"C:\\Temp\\"
**********************************************/
{
	CSingleLock lock( const_cast<CCriticalSection*>(&m_csDirWatchInfo), TRUE);
	ASSERT( lock.IsLocked() );
	int i;
	if( GetDirWatchInfo(strDirName, i) )
		return TRUE;
	return FALSE;
}

int	CDirectoryChangeWatcher::NumWatchedDirectories()const
{
	CSingleLock lock(const_cast<CCriticalSection*>(&m_csDirWatchInfo), TRUE);
	ASSERT( lock.IsLocked() );
	int nCnt(0),max = m_DirectoriesToWatch.GetSize();
	for(int i(0); i < max; ++i)
	{
		if( m_DirectoriesToWatch[i] != NULL )//array may contain NULL elements.
			nCnt++;
	}

	return nCnt;
}

DWORD CDirectoryChangeWatcher::WatchDirectory(const CString & strDirToWatch, 
									   DWORD dwChangesToWatchFor, 
									   CDirectoryChangeHandler * pChangeHandler,
									   BOOL bWatchSubDirs /*=FALSE*/,
									   LPCTSTR szIncludeFilter /*=NULL*/,
									   LPCTSTR szExcludeFilter /*=NULL*/
									   )
/*************************************************************
FUNCTION:	WatchDirectory(const CString & strDirToWatch,   --the name of the directory to watch
						   DWORD dwChangesToWatchFor, --the changes to watch for see dsk docs..for ReadDirectoryChangesW
						   CDirectoryChangeHandler * pChangeHandler -- handles changes in specified directory
						   BOOL bWatchSubDirs      --specified to watch sub directories of the directory that you want to watch
						   )

PARAMETERS:
		const CString & strDirToWatch -- specifies the path of the directory to watch.
		DWORD dwChangesToWatchFor	-- specifies flags to be passed to ReadDirectoryChangesW()
		CDirectoryChangeHandler *	-- ptr to an object which will handle notifications of file changes.
		BOOL bWatchSubDirs			-- specifies to watch subdirectories.
		LPCTSTR szIncludeFilter		-- A file pattern string for files that you wish to receive notifications
									   for. See Remarks.
		LPCTSTR szExcludeFilter		-- A file pattern string for files that you do not wish to receive notifications for. See Remarks

	Starts watching the specified directory(and optionally subdirectories) for the specified changes

	When specified changes take place the appropriate CDirectoryChangeHandler::On_Filexxx() function is called.

	dwChangesToWatchFor can be a combination of the following flags, and changes map out to the 
	following functions:
	FILE_NOTIFY_CHANGE_FILE_NAME    -- CDirectoryChangeHandler::On_FileAdded()
									   CDirectoryChangeHandler::On_FileNameChanged, 
									   CDirectoryChangeHandler::On_FileRemoved
	FILE_NOTIFY_CHANGE_DIR_NAME     -- CDirectoryChangeHandler::On_FileNameAdded(), 
									   CDirectoryChangeHandler::On_FileRemoved
	FILE_NOTIFY_CHANGE_ATTRIBUTES   -- CDirectoryChangeHandler::On_FileModified
	FILE_NOTIFY_CHANGE_SIZE         -- CDirectoryChangeHandler::On_FileModified
	FILE_NOTIFY_CHANGE_LAST_WRITE   -- CDirectoryChangeHandler::On_FileModified
	FILE_NOTIFY_CHANGE_LAST_ACCESS  -- CDirectoryChangeHandler::On_FileModified
	FILE_NOTIFY_CHANGE_CREATION     -- CDirectoryChangeHandler::On_FileModified
	FILE_NOTIFY_CHANGE_SECURITY     -- CDirectoryChangeHandler::On_FileModified?
	

	Returns ERROR_SUCCESS if the directory will be watched, 
	or a windows error code if the directory couldn't be watched.
	The error code will most likely be related to a call to CreateFile(), or 
	from the initial call to ReadDirectoryChangesW().  It's also possible to get an
	error code related to being unable to create an io completion port or being unable 
	to start the worker thread.

	This function will fail if the directory to be watched resides on a 
	computer that is not a Windows NT/2000/XP machine.


	You can only have one watch specified at a time for any particular directory.
	Calling this function with the same directory name will cause the directory to be 
	unwatched, and then watched again(w/ the new parameters that have been passed in).  

**************************************************************/
{
	ASSERT( dwChangesToWatchFor != 0);

	if( strDirToWatch.IsEmpty()
	||  dwChangesToWatchFor == 0 
	||  pChangeHandler == NULL )
	{
		TRACE(_T("ERROR: You've passed invalid parameters to CDirectoryChangeWatcher::WatchDirectory()\n"));
		::SetLastError(ERROR_INVALID_PARAMETER);
		return ERROR_INVALID_PARAMETER;
	}

	
	//double check that it's really a directory
	if( !IsDirectory( strDirToWatch ) )
	{
		TRACE(_T("ERROR: CDirectoryChangeWatcher::WatchDirectory() -- %s is not a directory!\n"), strDirToWatch);
		::SetLastError(ERROR_BAD_PATHNAME);
		return ERROR_BAD_PATHNAME;
	}

	//double check that this directory is not already being watched....
	//if it is, then unwatch it before restarting it...
	if( IsWatchingDirectory( strDirToWatch) )
	{
		VERIFY( 
			UnwatchDirectory( strDirToWatch ) 
			);
	}
	//
	//
	//	Reference this singleton so that privileges for this process are enabled 
	//	so that it has required permissions to use the ReadDirectoryChangesW API, etc.
	//
	CPrivilegeEnabler::Instance();
	//
	//open the directory to watch....
	HANDLE hDir = CreateFile(strDirToWatch, 
								FILE_LIST_DIRECTORY, 
								FILE_SHARE_READ | FILE_SHARE_WRITE ,//| FILE_SHARE_DELETE, <-- removing FILE_SHARE_DELETE prevents the user or someone else from renaming or deleting the watched directory. This is a good thing to prevent.
								NULL, //security attributes
								OPEN_EXISTING,
								FILE_FLAG_BACKUP_SEMANTICS | //<- the required priviliges for this flag are: SE_BACKUP_NAME and SE_RESTORE_NAME.  CPrivilegeEnabler takes care of that.
                                FILE_FLAG_OVERLAPPED, //OVERLAPPED!
								NULL);
	if( hDir == INVALID_HANDLE_VALUE )
	{
		DWORD dwError = GetLastError();
		TRACE(_T("CDirectoryChangeWatcher::WatchDirectory() -- Couldn't open directory for monitoring. %d\n"), dwError);
		::SetLastError(dwError);//who knows if TRACE will cause GetLastError() to return success...probably won't, but set it manually just for fun.
		return dwError;
	}
	//opened the dir!
	
	CDirWatchInfo * pDirInfo = new CDirWatchInfo( hDir, strDirToWatch, pChangeHandler, dwChangesToWatchFor, bWatchSubDirs, m_bAppHasGUI, szIncludeFilter, szExcludeFilter, m_dwFilterFlags);
	if( !pDirInfo )
	{
		TRACE(_T("WARNING: Couldn't allocate a new CDirWatchInfo() object --- File: %s Line: %d\n"), _T( __FILE__ ), __LINE__);
		CloseHandle( hDir );
		::SetLastError(ERROR_OUTOFMEMORY);
		return ERROR_OUTOFMEMORY;
	}
	
	//create a IO completion port/or associate this key with
	//the existing IO completion port
	m_hCompPort = CreateIoCompletionPort(hDir, 
										m_hCompPort, //if m_hCompPort is NULL, hDir is associated with a NEW completion port,
													 //if m_hCompPort is NON-NULL, hDir is associated with the existing completion port that the handle m_hCompPort references
										(DWORD)pDirInfo, //the completion 'key'... this ptr is returned from GetQueuedCompletionStatus() when one of the events in the dwChangesToWatchFor filter takes place
										0);
	if( m_hCompPort == NULL )
	{
		TRACE(_T("ERROR -- Unable to create I/O Completion port! GetLastError(): %d File: %s Line: %d"), GetLastError(), _T( __FILE__ ), __LINE__ );
		DWORD dwError = GetLastError();
		pDirInfo->DeleteSelf( NULL );
		::SetLastError(dwError);//who knows what the last error will be after i call pDirInfo->DeleteSelf(), so set it just to make sure
		return dwError;
	}
	else
	{//completion port created/directory associated w/ it successfully

		//if the thread isn't running start it....
		//when the thread starts, it will call ReadDirectoryChangesW and wait 
		//for changes to take place
		if( m_hThread == NULL )
		{
			//start the thread
			CWinThread * pThread = AfxBeginThread(MonitorDirectoryChanges, this);
			if( !pThread )
			{//couldn't create the thread!
				TRACE(_T("CDirectoryChangeWatcher::WatchDirectory()-- AfxBeginThread failed!\n"));
				pDirInfo->DeleteSelf( NULL );
				return (GetLastError() == ERROR_SUCCESS)? ERROR_MAX_THRDS_REACHED : GetLastError();
			}
			else
			{
				m_hThread	 = pThread->m_hThread;
				m_dwThreadID = pThread->m_nThreadID;
				pThread->m_bAutoDelete = TRUE;//pThread is deleted when thread ends....it's TRUE by default(for CWinThread ptrs returned by AfxBeginThread(threadproc, void*)), but just makin sure.
				
			}
		}
		if( m_hThread != NULL )
		{//thread is running, 
			//signal the thread to issue the initial call to
			//ReadDirectoryChangesW()
		   DWORD dwStarted = pDirInfo->StartMonitor( m_hCompPort );

		   if( dwStarted != ERROR_SUCCESS )
		   {//there was a problem!
			   TRACE(_T("Unable to watch directory: %s -- GetLastError(): %d\n"), dwStarted);
			   pDirInfo->DeleteSelf( NULL );
				::SetLastError(dwStarted);//I think this'll set the Err object in a VB app.....
			   return dwStarted;
		   }
		   else
		   {//ReadDirectoryChangesW was successfull!
				//add the directory info to the first empty slot in the array

				//associate the pChangeHandler with this object
				pChangeHandler->ReferencesWatcher( this );//reference is removed when directory is unwatched.
				//CDirWatchInfo::DeleteSelf() must now be called w/ this CDirectoryChangeWatcher pointer becuse
				//of a reference count

				//save the CDirWatchInfo* so I'll be able to use it later.
				VERIFY( AddToWatchInfo( pDirInfo ) );
				SetLastError(dwStarted);
				return dwStarted;
		   }

		}
		else
		{
			ASSERT(FALSE);//control path shouldn't get here
			::SetLastError(ERROR_MAX_THRDS_REACHED);
			return ERROR_MAX_THRDS_REACHED;
		}
		
	}
	ASSERT( FALSE );//shouldn't get here.
}

BOOL CDirectoryChangeWatcher::UnwatchAllDirectories()
{
	
	//unwatch all of the watched directories
	//delete all of the CDirWatchInfo objects,
	//kill off the worker thread
	if( m_hThread != NULL )
	{
		ASSERT( m_hCompPort != NULL );
		
		CSingleLock lock( &m_csDirWatchInfo, TRUE);
		ASSERT( lock.IsLocked() );

		CDirWatchInfo * pDirInfo;
		//Unwatch each of the watched directories
		//and delete the CDirWatchInfo associated w/ that directory...
		int max = m_DirectoriesToWatch.GetSize();
		for(int i = 0; i < max; ++i)
		{
			if( (pDirInfo = m_DirectoriesToWatch[i]) != NULL )
			{
				VERIFY( pDirInfo->UnwatchDirectory( m_hCompPort ) );

				m_DirectoriesToWatch.SetAt(i, NULL)	;
				pDirInfo->DeleteSelf(this);
			}
			
		}
		m_DirectoriesToWatch.RemoveAll();
		//kill off the thread
		PostQueuedCompletionStatus(m_hCompPort, 0, 0, NULL);//The thread will catch this and exit the thread
		//wait for it to exit
		WaitForSingleObject(m_hThread, INFINITE);
		//CloseHandle( m_hThread );//Since thread was started w/ AfxBeginThread() this handle is closed automatically, closing it again will raise an exception
		m_hThread = NULL;
		m_dwThreadID = 0UL;		

		//close the completion port...
		CloseHandle( m_hCompPort );
		m_hCompPort = NULL;


		return TRUE;
	}
	else
	{
#ifdef _DEBUG
		//make sure that there aren't any 
		//CDirWatchInfo objects laying around... they should have all been destroyed 
		//and removed from the array m_DirectoriesToWatch
		if( m_DirectoriesToWatch.GetSize() > 0 )
		{
			for(int i = 0; i < m_DirectoriesToWatch.GetSize(); ++i)
			{
				ASSERT( m_DirectoriesToWatch[i] == NULL );
			}
		}
#endif
	}
	return FALSE;
}

BOOL CDirectoryChangeWatcher::UnwatchDirectory(const CString & strDirToStopWatching)
/***************************************************************
FUNCTION:	UnwatchDirectory(const CString & strDirToStopWatching -- if this directory is being watched, the watch is stopped

****************************************************************/
{
	BOOL bRetVal = FALSE;



	if( m_hCompPort != NULL )//The io completion port must be open
	{
		ASSERT( !strDirToStopWatching.IsEmpty() );
		
		CSingleLock lock(&m_csDirWatchInfo, TRUE);
		ASSERT( lock.IsLocked() );	
		int nIdx = -1;
		CDirWatchInfo * pDirInfo = GetDirWatchInfo(strDirToStopWatching, nIdx);
		if( pDirInfo != NULL
		&&	nIdx != -1 )
		{

			//stop watching this directory
			VERIFY( pDirInfo->UnwatchDirectory( m_hCompPort ) );

			//cleanup the object used to watch the directory
			m_DirectoriesToWatch.SetAt(nIdx, NULL);
			pDirInfo->DeleteSelf(this);
			bRetVal = TRUE;
		}
	}

	return bRetVal;
}

BOOL CDirectoryChangeWatcher::UnwatchDirectory(CDirectoryChangeHandler * pChangeHandler)
/************************************

  This function is called from the dtor of CDirectoryChangeHandler automatically,
  but may also be called by a programmer because it's public...
  
  A single CDirectoryChangeHandler may be used for any number of watched directories.

  Unwatch any directories that may be using this 
  CDirectoryChangeHandler * pChangeHandler to handle changes to a watched directory...
  
  The CDirWatchInfo::m_pChangeHandler member of objects in the m_DirectoriesToWatch
  array will == pChangeHandler if that handler is being used to handle changes to a directory....
************************************/
{
	ASSERT( pChangeHandler );

	CSingleLock lock(&m_csDirWatchInfo, TRUE);
	
	ASSERT( lock.IsLocked() );
	
	int nUnwatched = 0;
	int nIdx = -1;
	CDirWatchInfo * pDirInfo;
	//
	//	go through and unwatch any directory that is 
	//	that is using this pChangeHandler as it's file change notification handler.
	//
	while( (pDirInfo = GetDirWatchInfo( pChangeHandler, nIdx )) != NULL )
	{
		VERIFY( pDirInfo->UnwatchDirectory( m_hCompPort ) );

		nUnwatched++;
		m_DirectoriesToWatch.SetAt(nIdx, NULL);
		pDirInfo->DeleteSelf(this);	
	}
	return (BOOL)(nUnwatched != 0);
}

BOOL CDirectoryChangeWatcher::UnwatchDirectoryBecauseOfError(CDirWatchInfo * pWatchInfo)
//
//	Called in the worker thread in the case that ReadDirectoryChangesW() fails
//	during normal operation. One way to force this to happen is to watch a folder
//	using a UNC path and changing that computer's IP address.
//	
{
	ASSERT( pWatchInfo );
	ASSERT( m_dwThreadID == GetCurrentThreadId() );//this should be called from the worker thread only.
	BOOL bRetVal = FALSE;
	if( pWatchInfo )
	{
		CSingleLock lock(&m_csDirWatchInfo, TRUE);
		
		ASSERT( lock.IsLocked() );
		int nIdx = -1;
		if( GetDirWatchInfo(pWatchInfo, nIdx) == pWatchInfo )
		{
			// we are actually watching this....

			//
			//	Remove this CDirWatchInfo object from the list of watched directories.
			//
			m_DirectoriesToWatch.SetAt(nIdx, NULL);//mark the space as free for the next watch...

			//
			//	and delete it...
			//

			pWatchInfo->DeleteSelf(this);
		
		}

	}
	return bRetVal;
}

int	CDirectoryChangeWatcher::AddToWatchInfo(CDirectoryChangeWatcher::CDirWatchInfo * pWatchInfo)
//
//	
//	To add the CDirWatchInfo  * to an array.
//	The array is used to keep track of which directories 
//	are being watched.
//
//	Add the ptr to the first non-null slot in the array.
{
	CSingleLock lock( &m_csDirWatchInfo, TRUE);
	ASSERT( lock.IsLocked() );
	
	//first try to add it to the first empty slot in m_DirectoriesToWatch
	int max = m_DirectoriesToWatch.GetSize();
    int i = 0;
	for(; i < max; ++i)
	{
		if( m_DirectoriesToWatch[i] == NULL )
		{
			m_DirectoriesToWatch[i] = pWatchInfo;
			break;
		}
	}
	if( i == max )
	{
		// there where no empty slots, add it to the end of the array
		try{
			i = m_DirectoriesToWatch.Add( pWatchInfo );
		}
		catch(CMemoryException * e){
			e->ReportError();
			e->Delete();//??? delete this? I thought CMemoryException objects where pre allocated in mfc? -- sample code in msdn does, so will i
			i = -1;
		}
	}

	return (BOOL)(i != -1);
}

//
//	functions for retrieving the directory watch info based on different parameters
//
CDirectoryChangeWatcher::CDirWatchInfo * CDirectoryChangeWatcher::GetDirWatchInfo(const CString & strDirName, int & ref_nIdx)const
{
	if( strDirName.IsEmpty() )// can't be watching a directory if it's you don't pass in the name of it...
		return FALSE;		  //
	
	CSingleLock lock(const_cast<CCriticalSection*>(&m_csDirWatchInfo), TRUE);

	int max = m_DirectoriesToWatch.GetSize();
	CDirWatchInfo * p = NULL;
	for(int i = 0; i < max; ++i )
	{
		if( (p = m_DirectoriesToWatch[i]) != NULL
		&&	p->m_strDirName.CompareNoCase( strDirName ) == 0 )
		{
			ref_nIdx = i;
			return p;
		}
	}
			
	return NULL;//NOT FOUND
}

CDirectoryChangeWatcher::CDirWatchInfo * CDirectoryChangeWatcher::GetDirWatchInfo(CDirectoryChangeWatcher::CDirWatchInfo * pWatchInfo, int & ref_nIdx)const
{
	ASSERT( pWatchInfo != NULL );

	CSingleLock lock( const_cast<CCriticalSection*>(&m_csDirWatchInfo), TRUE);
	int i(0), max = m_DirectoriesToWatch.GetSize();
	CDirWatchInfo * p;
	for(; i < max; ++i)
	{
		if( (p = m_DirectoriesToWatch[i]) != NULL
		&&	 p == pWatchInfo )
		{
			ref_nIdx = i;
			return p;
		}
	}
	return NULL;//NOT FOUND
}

CDirectoryChangeWatcher::CDirWatchInfo * CDirectoryChangeWatcher::GetDirWatchInfo(CDirectoryChangeHandler * pChangeHandler, int & ref_nIdx)const
{
	ASSERT( pChangeHandler != NULL );
	CSingleLock lock( const_cast<CCriticalSection*>(&m_csDirWatchInfo), TRUE);
	int i(0),max = m_DirectoriesToWatch.GetSize();
	CDirWatchInfo * p;
	for( ; i < max; ++i)
	{
		if( (p = m_DirectoriesToWatch[i]) != NULL
		&&	p->GetRealChangeHandler() == pChangeHandler )
		{
			ref_nIdx = i;
			return p;
		}
	}
	return NULL;//NOT FOUND
}

long CDirectoryChangeWatcher::ReleaseReferenceToWatcher(CDirectoryChangeHandler * pChangeHandler)
{
	ASSERT( pChangeHandler );
	return pChangeHandler->ReleaseReferenceToWatcher(this);
}

UINT CDirectoryChangeWatcher::MonitorDirectoryChanges(LPVOID lpvThis)
/********************************************
   The worker thread function which monitors directory changes....
********************************************/
{
    DWORD numBytes;

    CDirWatchInfo * pdi;
    LPOVERLAPPED lpOverlapped;
    
	CDirectoryChangeWatcher * pThis = reinterpret_cast<CDirectoryChangeWatcher*>(lpvThis);
	ASSERT( pThis );

	pThis->On_ThreadInitialize();


    do
    {
        // Retrieve the directory info for this directory
        // through the io port's completion key
        if( !GetQueuedCompletionStatus( pThis->m_hCompPort,
                                   &numBytes,
                                   (LPDWORD) &pdi,//<-- completion Key
                                   &lpOverlapped,
                                   INFINITE) )
		{//The io completion request failed...
		 //probably because the handle to the directory that
		 //was used in a call to ReadDirectoryChangesW has been closed.

			//
			//	calling pdi->CloseDirectoryHandle() will cause GetQueuedCompletionStatus() to return false.
			//  
			//
			if( !pdi 
			|| ( pdi && AfxIsValidAddress(pdi, sizeof(CDirectoryChangeWatcher::CDirWatchInfo)))
					 &&  pdi->m_hDir != INVALID_HANDLE_VALUE //the directory handle is still open! (we expect this when after we close the directory handle )
			  )
			{
#ifdef _DEBUG
			TRACE(_T("GetQueuedCompletionStatus() returned FALSE\nGetLastError(): %d Completion Key: %p lpOverlapped: %p\n"), GetLastError(), pdi, lpOverlapped);
			MessageBeep( static_cast<UINT>(-1) );
#endif
			}
		}
		
		if ( pdi )//pdi will be null if I call PostQueuedCompletionStatus(m_hCompPort, 0,0,NULL);
        {
			//
			//	The following check to AfxIsValidAddress() should go off in the case
			//	that I have deleted this CDirWatchInfo object, but it was still in 
			//	"in the Queue" of the i/o completion port from a previous overlapped operation.
			//
			ASSERT( AfxIsValidAddress(pdi, 
					sizeof(CDirectoryChangeWatcher::CDirWatchInfo)) );
			/***********************************
			The CDirWatchInfo::m_RunningState is pretty much the only member
			of CDirWatchInfo that can be modified from the other thread.
			The functions StartMonitor() and UnwatchDirecotry() are the functions that 
			can modify that variable.

			So that I'm sure that I'm getting the right value, 
			I'm using a critical section to guard against another thread modyfying it when I want
			to read it...
			
			************************************/
			bool bObjectShouldBeOk = true;
			try{
			    VERIFY( pdi->LockProperties() );//don't give the main thread a chance to change this object
			}
			catch(...){
				//any sort of exception here indicates I've
				//got a hosed object.
				TRACE(_T("CDirectoryChangeWatcher::MonitorDirectoryChanges() -- pdi->LockProperties() raised an exception!\n"));
				bObjectShouldBeOk = false;
			}
			if( bObjectShouldBeOk )
			{
										    //while we're working with this object...

				CDirWatchInfo::eRunningState Run_State = pdi->m_RunningState ;
				
				VERIFY( pdi->UnlockProperties() );//let another thread back at the properties...
				/***********************************
				 Unlock it so that there isn't a DEADLOCK if 
				 somebody tries to call a function which will 
				 cause CDirWatchInfo::UnwatchDirectory() to be called
				 from within the context of this thread (eg: a function called because of
				 the handler for one of the CDirectoryChangeHandler::On_Filexxx() functions)
		
				************************************/
				
				ASSERT( pdi->GetChangeHandler() );
				switch( Run_State )
				{
				case CDirWatchInfo::RUNNING_STATE_START_MONITORING:
					{
						//Issue the initial call to ReadDirectoryChangesW()
						
						if( !ReadDirectoryChangesW( pdi->m_hDir,
											pdi->m_Buffer,//<--FILE_NOTIFY_INFORMATION records are put into this buffer
											READ_DIR_CHANGE_BUFFER_SIZE,
											pdi->m_bWatchSubDir,
											pdi->m_dwChangeFilter,
											&pdi->m_dwBufLength,//this var not set when using asynchronous mechanisms...
											&pdi->m_Overlapped,
											NULL) )//no completion routine!
						{
							pdi->m_dwReadDirError = GetLastError();
							if( pdi->GetChangeHandler() )
								pdi->GetChangeHandler()->On_WatchStarted(pdi->m_dwReadDirError, pdi->m_strDirName);
						}
						else
						{//read directory changes was successful!
						 //allow it to run normally
							pdi->m_RunningState = CDirWatchInfo::RUNNING_STATE_NORMAL;
							pdi->m_dwReadDirError = ERROR_SUCCESS;
							if( pdi->GetChangeHandler() )
								pdi->GetChangeHandler()->On_WatchStarted(ERROR_SUCCESS, pdi->m_strDirName );
						}
						pdi->m_StartStopEvent.SetEvent();//signall that the ReadDirectoryChangesW has been called
														 //check CDirWatchInfo::m_dwReadDirError to see whether or not ReadDirectoryChangesW succeeded...

						//
						//	note that pdi->m_dwReadDirError is the value returned by WatchDirectory()
						//
						
		
					}break;
				case CDirWatchInfo::RUNNING_STATE_STOP:
					{
						//We want to shut down the monitoring of the directory
						//that pdi is managing...
						
						if( pdi->m_hDir != INVALID_HANDLE_VALUE )
						{
						 //Since I've previously called ReadDirectoryChangesW() asynchronously, I am waiting
						 //for it to return via GetQueuedCompletionStatus().  When I close the
						 //handle that ReadDirectoryChangesW() is waiting on, it will
						 //cause GetQueuedCompletionStatus() to return again with this pdi object....
						 // Close the handle, and then wait for the call to GetQueuedCompletionStatus()
						 //to return again by breaking out of the switch, and letting GetQueuedCompletionStatus()
						 //get called again
							pdi->CloseDirectoryHandle();
							pdi->m_RunningState = CDirWatchInfo::RUNNING_STATE_STOP_STEP2;//back up step...GetQueuedCompletionStatus() will still need to return from the last time that ReadDirectoryChangesW() was called.....

						 //
						 //	The watch has been stopped, tell the client about it
						 //						if( pdi->GetChangeHandler() )
							pdi->GetChangeHandler()->On_WatchStopped( pdi->m_strDirName );
						}
						else
						{
							//either we weren't watching this direcotry in the first place,
							//or we've already stopped monitoring it....
							pdi->m_StartStopEvent.SetEvent();//set the event that ReadDirectoryChangesW has returned and no further calls to it will be made...
						}
						
					
					}break;
				case CDirWatchInfo::RUNNING_STATE_STOP_STEP2:
					{

						//GetQueuedCompletionStatus() has returned from the last
						//time that ReadDirectoryChangesW was called...
						//Using CloseHandle() on the directory handle used by
						//ReadDirectoryChangesW will cause it to return via GetQueuedCompletionStatus()....
						if( pdi->m_hDir == INVALID_HANDLE_VALUE )
							pdi->m_StartStopEvent.SetEvent();//signal that no further calls to ReadDirectoryChangesW will be made
															 //and this pdi can be deleted 
						else
						{//for some reason, the handle is still open..
												
							pdi->CloseDirectoryHandle();

							//wait for GetQueuedCompletionStatus() to return this pdi object again


						}
		
					}break;
															
				case CDirWatchInfo::RUNNING_STATE_NORMAL:
					{
						
						if( pdi->GetChangeHandler() )
							pdi->GetChangeHandler()->SetChangedDirectoryName( pdi->m_strDirName );
		
						DWORD dwReadBuffer_Offset = 0UL;

						//process the FILE_NOTIFY_INFORMATION records:
						CFileNotifyInformation notify_info( (LPBYTE)pdi->m_Buffer, READ_DIR_CHANGE_BUFFER_SIZE);

						pThis->ProcessChangeNotifications(notify_info, pdi, dwReadBuffer_Offset);
		

						//	Changes have been processed,
						//	Reissue the watch command
						//
						if( !ReadDirectoryChangesW( pdi->m_hDir,
											  pdi->m_Buffer + dwReadBuffer_Offset,//<--FILE_NOTIFY_INFORMATION records are put into this buffer 
								              READ_DIR_CHANGE_BUFFER_SIZE - dwReadBuffer_Offset,
											  pdi->m_bWatchSubDir,
										      pdi->m_dwChangeFilter,
											  &pdi->m_dwBufLength,//this var not set when using asynchronous mechanisms...
											&pdi->m_Overlapped,
											NULL) )//no completion routine!
						{
							//
							//	NOTE:  
							//		In this case the thread will not wake up for 
							//		this pdi object because it is no longer associated w/
							//		the I/O completion port...there will be no more outstanding calls to ReadDirectoryChangesW
							//		so I have to skip the normal shutdown routines(normal being what happens when CDirectoryChangeWatcher::UnwatchDirectory() is called.
							//		and close this up, & cause it to be freed.
							//
							TRACE(_T("WARNING: ReadDirectoryChangesW has failed during normal operations...failed on directory: %s\n"), pdi->m_strDirName);

							ASSERT( pThis );
							//
							//	To help insure that this has been unwatched by the time
							//	the main thread processes the On_ReadDirectoryChangesError() notification
							//	bump the thread priority up temporarily.  The reason this works is because the notification
							//	is really posted to another thread's message queue,...by setting this thread's priority
							//	to highest, this thread will get to shutdown the watch by the time the other thread has a chance
							//	to handle it. *note* not technically guaranteed 100% to be the case, but in practice it'll work.
							int nOldThreadPriority = GetThreadPriority( GetCurrentThread() );
							SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);							

							//
							//	Notify the client object....(a CDirectoryChangeHandler derived class)
							//
							try{
								pdi->m_dwReadDirError = GetLastError();
								pdi->GetChangeHandler()->On_ReadDirectoryChangesError( pdi->m_dwReadDirError, pdi->m_strDirName );


								//Do the shutdown
								pThis->UnwatchDirectoryBecauseOfError( pdi );
								//pdi = NULL; <-- DO NOT set this to NULL, it will cause this worker thread to exit.
								//pdi is INVALID at this point!!
							}
							catch(...)
							{
								//just in case of exception, this thread will be set back to 
								//normal priority.
							}
							//
							//	Set the thread priority back to normal.
							//
							SetThreadPriority(GetCurrentThread(), nOldThreadPriority);
													
						}
						else
						{//success, continue as normal
							pdi->m_dwReadDirError = ERROR_SUCCESS;
						}
					}break;
				default:
					TRACE(_T("MonitorDirectoryChanges() -- how did I get here?\n"));
					break;//how did I get here?
				}//end switch( pdi->m_RunningState )
		
		
		
			}//end if( bObjectShouldBeOk )
        }//end if( pdi )
    } while( pdi );

	pThis->On_ThreadExit();
	return 0; //thread is ending
}

void CDirectoryChangeWatcher::ProcessChangeNotifications(IN CFileNotifyInformation & notify_info, 
														 IN CDirectoryChangeWatcher::CDirWatchInfo * pdi,
														 OUT DWORD & ref_dwReadBuffer_Offset//used in case ...see case for FILE_ACTION_RENAMED_OLD_NAME
														 )
/////////////////////////////////////////////////////////////
//
//	Processes the change notifications and dispatches the handling of the 
//	notifications to the CDirectoryChangeHandler object passed to WatchDirectory()
//
/////////////////////////////////////////////////////////////
{
	//
	//	Sanity check...
	//	this function should only be called by the worker thread.
	//	
	ASSERT( m_dwThreadID == GetCurrentThreadId() );

	//	Validate parameters...
	//	
	ASSERT( pdi );
	ASSERT( AfxIsValidAddress(pdi, sizeof(CDirectoryChangeWatcher::CDirWatchInfo) ) );

	if( !pdi || !AfxIsValidAddress(pdi, sizeof(CDirectoryChangeWatcher::CDirWatchInfo)) )
	{
		TRACE(_T("Invalid arguments to CDirectoryChangeWatcher::ProcessChangeNotifications() -- pdi is invalid!\n"));
		TRACE(_T("File: %s Line: %d"), _T( __FILE__ ), __LINE__ );
		return;
	}



	DWORD dwLastAction = 0;
	ref_dwReadBuffer_Offset = 0UL;
	

	CDirectoryChangeHandler * pChangeHandler = pdi->GetChangeHandler();
	//CDelayedDirectoryChangeHandler * pChangeHandler = pdi->GetChangeHandler();
	ASSERT( pChangeHandler );
	ASSERT( AfxIsValidAddress(pChangeHandler, sizeof(CDirectoryChangeHandler)) );
	//ASSERT( AfxIsValidAddress(pChangeHandler, sizeof(CDelayedDirectoryChangeHandler)) );
	if( !pChangeHandler )
	{
		TRACE(_T("CDirectoryChangeWatcher::ProcessChangeNotifications() Unable to continue, pdi->GetChangeHandler() returned NULL!\n"));
		TRACE(_T("File: %s  Line: %d\n"), _T( __FILE__ ), __LINE__ );
		return;
	}


	//
	//	go through and process the notifications contained in the
	//	CFileChangeNotification object( CFileChangeNotification is a wrapper for the FILE_NOTIFY_INFORMATION structure
	//									returned by ReadDirectoryChangesW)
	//
    do
	{
		//The FileName member of the FILE_NOTIFY_INFORMATION
		//structure contains the NAME of the file RELATIVE to the 
		//directory that is being watched...
		//ie, if watching C:\Temp and the file C:\Temp\MyFile.txt is changed,
		//the file name will be "MyFile.txt"
		//If watching C:\Temp, AND you're also watching subdirectories
		//and the file C:\Temp\OtherFolder\MyOtherFile.txt is modified,
		//the file name will be "OtherFolder\MyOtherFile.txt

		//The CDirectoryChangeHandler::On_Filexxx() functions will receive the name of the file
		//which includes the full path to the directory being watched
		
		
		//	
		//	See what the change was
		//
		
		switch( notify_info.GetAction() )
		{
		case FILE_ACTION_ADDED:		// a file was added!
	
			pChangeHandler->On_FileAdded( notify_info.GetFileNameWithPath( pdi->m_strDirName ) ); break;

		case FILE_ACTION_REMOVED:	//a file was removed
		
			pChangeHandler->On_FileRemoved( notify_info.GetFileNameWithPath( pdi->m_strDirName ) ); break;

		case FILE_ACTION_MODIFIED:
			//a file was changed
			//pdi->m_pChangeHandler->On_FileModified( strLastFileName ); break;
			pChangeHandler->On_FileModified( notify_info.GetFileNameWithPath( pdi->m_strDirName ) ); break;

		case FILE_ACTION_RENAMED_OLD_NAME:
			{//a file name has changed, and this is the OLD name
			 //This record is followed by another one w/
			 //the action set to FILE_ACTION_RENAMED_NEW_NAME (contains the new name of the file

				CString strOldFileName = notify_info.GetFileNameWithPath( pdi->m_strDirName );

				
				if( notify_info.GetNextNotifyInformation() )
				{//there is another PFILE_NOTIFY_INFORMATION record following the one we're working on now...
				 //it will be the record for the FILE_ACTION_RENAMED_NEW_NAME record
			

					ASSERT( notify_info.GetAction() == FILE_ACTION_RENAMED_NEW_NAME );//making sure that the next record after the OLD_NAME record is the NEW_NAME record

					//get the new file name
					CString strNewFileName = notify_info.GetFileNameWithPath( pdi->m_strDirName );

					pChangeHandler->On_FileNameChanged( strOldFileName, strNewFileName);
				}
				else
				{
					//this OLD_NAME was the last record returned by ReadDirectoryChangesW
					//I will have to call ReadDirectoryChangesW again so that I will get 
					//the record for FILE_ACTION_RENAMED_NEW_NAME

					//Adjust an offset so that when I call ReadDirectoryChangesW again,
					//the FILE_NOTIFY_INFORMATION will be placed after 
					//the record that we are currently working on.

					/***************
					Let's say that 200 files all had their names changed at about the same time
					There will be 400 FILE_NOTIFY_INFORMATION records (one for OLD_NAME and one for NEW_NAME for EACH file which had it's name changed)
					that ReadDirectoryChangesW will have to report to
					me.   There might not be enough room in the buffer
					and the last record that we DID get was an OLD_NAME record,
					I will need to call ReadDirectoryChangesW again so that I will get the NEW_NAME 
					record.    This way I'll always have to strOldFileName and strNewFileName to pass
					to CDirectoryChangeHandler::On_FileRenamed().

				   After ReadDirecotryChangesW has filled out our buffer with
				   FILE_NOTIFY_INFORMATION records,
				   our read buffer would look something like this:
																						 End Of Buffer
																							  |
																							 \-/	
					|_________________________________________________________________________
					|																		  |
					|file1 OLD name record|file1 NEW name record|...|fileX+1 OLD_name record| |(the record we want would be here, but we've ran out of room, so we adjust an offset and call ReadDirecotryChangesW again to get it) 
					|_________________________________________________________________________|

					Since the record I need is still waiting to be returned to me,
					and I need the current 'OLD_NAME' record,
					I'm copying the current FILE_NOTIFY_INFORMATION record 
					to the beginning of the buffer used by ReadDirectoryChangesW()
					and I adjust the offset into the read buffer so the the NEW_NAME record
					will be placed into the buffer after the OLD_NAME record now at the beginning of the buffer.

					Before we call ReadDirecotryChangesW again,
					modify the buffer to contain the current OLD_NAME record...

					|_______________________________________________________
					|														|
					|fileX old name record(saved)|<this is now garbage>.....|
					|_______________________________________________________|
											 	 /-\
												  |
											 Offset for Read
					Re-issue the watch command to get the rest of the records...

					ReadDirectoryChangesW(..., pBuffer + (an Offset),

					After GetQueuedCompletionStatus() returns, 
					our buffer will look like this:

					|__________________________________________________________________________________________________________
					|																										   |
					|fileX old name record(saved)|fileX new name record(the record we've been waiting for)| <other records>... |
					|__________________________________________________________________________________________________________|

					Then I'll be able to know that a file name was changed
					and I will have the OLD and the NEW name of the file to pass to CDirectoryChangeHandler::On_FileNameChanged

					****************/
					//NOTE that this case has never happened to me in my testing
					//so I can only hope that the code works correctly.
					//It would be a good idea to set a breakpoint on this line of code:
					VERIFY( notify_info.CopyCurrentRecordToBeginningOfBuffer( ref_dwReadBuffer_Offset ) );
					

				}
				break;
			}
		case FILE_ACTION_RENAMED_NEW_NAME:
			{
				//This should have been handled in FILE_ACTION_RENAMED_OLD_NAME
				ASSERT( dwLastAction == FILE_ACTION_RENAMED_OLD_NAME );
				ASSERT( FALSE );//this shouldn't get here
			}
		
		default:
			TRACE(_T("CDirectoryChangeWatcher::ProcessChangeNotifications() -- unknown FILE_ACTION_ value! : %d\n"), notify_info.GetAction() );
			break;//unknown action
		}

		dwLastAction = notify_info.GetAction();
		
    
	} while( notify_info.GetNextNotifyInformation() );
}