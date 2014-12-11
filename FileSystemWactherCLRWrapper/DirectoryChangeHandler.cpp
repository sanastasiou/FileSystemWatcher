#include "stdafx.h"
#include "DirectoryChangeHandler.h"
#include "CDirectoryChangeWatcher.h"

//
//
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CDirectoryChangeHandler::CDirectoryChangeHandler()
: m_nRefCnt( 1 ),
  m_pDirChangeWatcher( NULL ),
  m_nWatcherRefCnt( 0L )
{
}

CDirectoryChangeHandler::~CDirectoryChangeHandler()
{
	UnwatchDirectory();
}

long CDirectoryChangeHandler::AddRef()
{ 
	return InterlockedIncrement(&m_nRefCnt);	
}

long CDirectoryChangeHandler::Release()
{  
	long nRefCnt = -1;
	if( (nRefCnt = InterlockedDecrement(&m_nRefCnt)) == 0 )
		delete this;
	return nRefCnt;
}
long CDirectoryChangeHandler::CurRefCnt()const 
{ 
	return m_nRefCnt;
}

BOOL CDirectoryChangeHandler::UnwatchDirectory()
{
	CSingleLock lock(&m_csWatcher, TRUE);	
	ASSERT( lock.IsLocked() );
	
	if( m_pDirChangeWatcher )
		return m_pDirChangeWatcher->UnwatchDirectory( this );
	return TRUE;
}

long  CDirectoryChangeHandler::ReferencesWatcher(CDirectoryChangeWatcher * pDirChangeWatcher)
{
	ASSERT( pDirChangeWatcher );
	CSingleLock lock(&m_csWatcher, TRUE);
	if( m_pDirChangeWatcher 
	&&  m_pDirChangeWatcher != pDirChangeWatcher )
	{
		TRACE(_T("CDirectoryChangeHandler...is becoming used by a different CDirectoryChangeWatcher!\n"));
		TRACE(_T("Directories being handled by this object will now be unwatched.\nThis object is now being used to ")
			  _T("handle changes to a directory watched by different CDirectoryChangeWatcher object, probably on a different directory"));
		
		if( UnwatchDirectory() )
		{
			m_pDirChangeWatcher = pDirChangeWatcher;
			m_nWatcherRefCnt = 1; //when this reaches 0, set m_pDirChangeWatcher to NULL
			return m_nWatcherRefCnt;
		}
		else
		{
			ASSERT( FALSE );//shouldn't get here!
		}
	}
	else
	{
		ASSERT( !m_pDirChangeWatcher || m_pDirChangeWatcher == pDirChangeWatcher );
		
		m_pDirChangeWatcher = pDirChangeWatcher;	
		
		if( m_pDirChangeWatcher )
			return InterlockedIncrement(&m_nWatcherRefCnt);
		
	}
	return m_nWatcherRefCnt;
}

long CDirectoryChangeHandler::ReleaseReferenceToWatcher(CDirectoryChangeWatcher * pDirChangeWatcher)
{
	ASSERT( m_pDirChangeWatcher == pDirChangeWatcher );
	CSingleLock lock(&m_csWatcher, TRUE);
	long nRef;
	if( (nRef = InterlockedDecrement(&m_nWatcherRefCnt)) <= 0L )
	{
		m_pDirChangeWatcher = NULL; //Setting this to NULL so that this->UnwatchDirectory() which is called in the dtor
									//won't call m_pDirChangeWatcher->UnwatchDirecotry(this).
									//m_pDirChangeWatcher may point to a destructed object depending on how
									//these classes are being used.
		m_nWatcherRefCnt = 0L;
	}
	return nRef;
}

//
//
//	Default implmentations for CDirectoryChangeHandler's virtual functions.
//
//
void CDirectoryChangeHandler::On_FileAdded(const CString & strFileName)
{ 
	TRACE(_T("The following file was added: %s\n"), strFileName);
}

void CDirectoryChangeHandler::On_FileRemoved(const CString & strFileName)
{
	TRACE(_T("The following file was removed: %s\n"), strFileName);
}

void CDirectoryChangeHandler::On_FileModified(const CString & strFileName)
{
	TRACE(_T("The following file was modified: %s\n"), strFileName);
}

void CDirectoryChangeHandler::On_FileNameChanged(const CString & strOldFileName, const CString & strNewFileName)
{
	TRACE(_T("The file %s was RENAMED to %s\n"), strOldFileName, strNewFileName);
}
void CDirectoryChangeHandler::On_ReadDirectoryChangesError(DWORD dwError, const CString & strDirectoryName)
{
	TRACE(_T("WARNING!!!!!\n") );
	TRACE(_T("An error has occurred on a watched directory!\n"));
	TRACE(_T("This directory has become unwatched! -- %s \n"), strDirectoryName);
	TRACE(_T("ReadDirectoryChangesW has failed! %d"), dwError);
	ASSERT( FALSE );//you should override this function no matter what. an error will occur someday.
}

void CDirectoryChangeHandler::On_WatchStarted(DWORD dwError, const CString & strDirectoryName)
{	
	if( dwError == 0 )
		TRACE(_T("A watch has begun on the following directory: %s\n"), strDirectoryName);
	else
		TRACE(_T("A watch failed to start on the following directory: (Error: %d) %s\n"),dwError, strDirectoryName);
}

void CDirectoryChangeHandler::On_WatchStopped(const CString & strDirectoryName)
{
	TRACE(_T("The watch on the following directory has stopped: %s\n"), strDirectoryName);
}

bool CDirectoryChangeHandler::On_FilterNotification(DWORD /*dwNotifyAction*/, LPCTSTR /*szFileName*/, LPCTSTR /*szNewFileName*/)
//
//	bool On_FilterNotification(DWORD dwNotifyAction, LPCTSTR szFileName, LPCTSTR szNewFileName);
//
//	This function gives your class a chance to filter unwanted notifications.
//
//	PARAMETERS: 
//			DWORD	dwNotifyAction	-- specifies the event to filter
//			LPCTSTR szFileName		-- specifies the name of the file for the event.
//			LPCTSTR szNewFileName	-- specifies the new file name of a file that has been renamed.
//
//	RETURN VALUE:
//			return true from this function, and you will receive the notification.
//			return false from this function, and your class will NOT receive the notification.
//
//	Valid values of dwNotifyAction:
//		FILE_ACTION_ADDED			-- On_FileAdded() is about to be called.
//		FILE_ACTION_REMOVED			-- On_FileRemoved() is about to be called.
//		FILE_ACTION_MODIFIED		-- On_FileModified() is about to be called.
//		FILE_ACTION_RENAMED_OLD_NAME-- On_FileNameChanged() is about to be call.
//
//	  
//	NOTE:  When the value of dwNotifyAction is FILE_ACTION_RENAMED_OLD_NAME,
//			szFileName will be the old name of the file, and szNewFileName will
//			be the new name of the renamed file.
//
//  The default implementation always returns true, indicating that all notifications will 
//	be sent.
//	
{
	return true;
}

void CDirectoryChangeHandler::SetChangedDirectoryName(const CString & strChangedDirName)
{
	m_strChangedDirectoryName = strChangedDirName;
}
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
