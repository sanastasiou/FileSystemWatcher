#include "stdafx.h"
#include "DirChangeNotification.h"
#include "CDelayedDirectoryChangeHandler.h"


CDirChangeNotification::CDirChangeNotification(CDelayedDirectoryChangeHandler *	pDelayedHandler, DWORD dwPartialPathOffset)
:m_pDelayedHandler( pDelayedHandler )
,m_szFileName1(NULL)
,m_szFileName2(NULL)
,m_dwError(0UL)
,m_dwPartialPathOffset(dwPartialPathOffset)
{
	ASSERT( pDelayedHandler );
}

CDirChangeNotification::~CDirChangeNotification()
{
	if( m_szFileName1 ) free(m_szFileName1), m_szFileName1 = NULL;
	if( m_szFileName2 ) free(m_szFileName2), m_szFileName2 = NULL;
}

void CDirChangeNotification::DispatchNotificationFunction()
{
	ASSERT( m_pDelayedHandler );
	if( m_pDelayedHandler )
		m_pDelayedHandler->DispatchNotificationFunction( this );
}

void CDirChangeNotification::PostOn_FileAdded(LPCTSTR szFileName)
{
	ASSERT( szFileName );
	m_eFunctionToDispatch	= eOn_FileAdded;
	m_szFileName1			= _tcsdup( szFileName) ;
	//
	// post the message so it'll be dispatch by another thread.
	PostNotification();

}
void CDirChangeNotification::PostOn_FileRemoved(LPCTSTR szFileName)
{
	ASSERT( szFileName );
	m_eFunctionToDispatch	= eOn_FileRemoved;
	m_szFileName1			= _tcsdup( szFileName) ;
	//
	// post the message so it'll be dispatched by another thread.
	PostNotification();
	
}
void CDirChangeNotification::PostOn_FileNameChanged(LPCTSTR szOldName, LPCTSTR szNewName)
{
	ASSERT( szOldName && szNewName );

	m_eFunctionToDispatch	= eOn_FileNameChanged;
	m_szFileName1			= _tcsdup( szOldName) ;
	m_szFileName2			= _tcsdup( szNewName) ;
	//
	// post the message so it'll be dispatched by another thread.
	PostNotification();
	
}

void CDirChangeNotification::PostOn_FileModified(LPCTSTR szFileName)
{
	ASSERT( szFileName );

	m_eFunctionToDispatch	= eOn_FileModified;
	m_szFileName1			= _tcsdup( szFileName );
	//
	// post the message so it'll be dispatched by another thread.
	PostNotification();
}

void CDirChangeNotification::PostOn_ReadDirectoryChangesError(DWORD dwError, LPCTSTR szDirectoryName)
{
	ASSERT( szDirectoryName );

	m_eFunctionToDispatch = eOn_ReadDirectoryChangesError;
	m_dwError			  = dwError;
	m_szFileName1		  = _tcsdup(szDirectoryName);
	//
	// post the message so it'll be dispatched by the another thread.
	PostNotification();
	
}

void CDirChangeNotification::PostOn_WatchStarted(DWORD dwError, LPCTSTR szDirectoryName)
{
	ASSERT( szDirectoryName );

	m_eFunctionToDispatch = eOn_WatchStarted;
	m_dwError			  =	dwError;
	m_szFileName1		  = _tcsdup(szDirectoryName);

	PostNotification();
}

void CDirChangeNotification::PostOn_WatchStopped(LPCTSTR szDirectoryName)
{
	ASSERT( szDirectoryName );

	m_eFunctionToDispatch = eOn_WatchStopped;
	m_szFileName1		  = _tcsdup(szDirectoryName);

	PostNotification();
}

void CDirChangeNotification::PostNotification()
{
	ASSERT( m_pDelayedHandler );
	if( m_pDelayedHandler )
		m_pDelayedHandler->PostNotification( this );
}