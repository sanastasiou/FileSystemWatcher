#include "stdafx.h"
#include "DelayedNotifier.h"
#include "DirChangeNotification.h"
#include <process.h>//for _beginthreadex

#define UWM_DELAYED_DIRECTORY_NOTIFICATION (WM_APP+1024)


HINSTANCE GetInstanceHandle()
{
	return (HINSTANCE)GetModuleHandle(NULL);
	// ASSERT( AfxGetInstanceHandle() == (HINSTANCE)GetModuleHandle(NULL) ); <-- true for building .exe's 
	//NOTE: In Dll's using shared MFC, AfxGetInstanceHandle() != (HINSTANCE)GetModuleHandle(NULL)...
	//don't know if this is the case for dll's using static MFC
}
static inline bool IsEmptyString(LPCTSTR sz)
{
	return (bool)(sz==NULL || *sz == 0);
}

static LRESULT CALLBACK DelayedNotificationWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//
//	This is the wndproc for the notification window
//
//	it's here to dispatch the notifications to the client
//
{
	if( message == UWM_DELAYED_DIRECTORY_NOTIFICATION )
	{
		CDirChangeNotification * pNotification = reinterpret_cast<CDirChangeNotification*>(lParam);
		ASSERT(  pNotification );
		if( pNotification )
		{
			DWORD dwEx(0);
			__try{
				pNotification->DispatchNotificationFunction();
			}
			__except(dwEx = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER){
				//An exception was raised:
				//
				//	Likely cause: there was a problem creating the CDelayedDirectoryChangeHandler::m_hWatchStoppedDispatchedEvent object
				//	and the change handler object was deleted before the notification could be dispatched to this function.
				//
				//  or perhaps, somebody's implementation of an overridden function caused an exception
				TRACE(_T("Following exception occurred: %d -- File: %s Line: %d\n"), dwEx, _T(__FILE__), __LINE__);
			}
		}
		
		return 0UL;
	}
	else
		return DefWindowProc(hWnd,message,wParam,lParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//
//CDelayedNotificationWindow static member vars:
//
long CDelayedNotificationWindow::s_nRefCnt = 0L;
HWND CDelayedNotificationWindow::s_hWnd = NULL;
BOOL CDelayedNotificationWindow::s_bRegisterWindow = FALSE;
//
//
long CDelayedNotificationWindow::AddRef()//creates window for first time if necessary
{
	if( InterlockedIncrement(&s_nRefCnt) == 1
		||	!::IsWindow( s_hWnd ) )
	{
		TRACE(_T("CDelayedNotificationWindow -- Creating the notification window\n"));
		VERIFY( CreateNotificationWindow() );
	}
	return s_nRefCnt;
}

long CDelayedNotificationWindow::Release()//destroys window for last time if necessary
{
	long nRefCnt = -1;
	if( (nRefCnt = InterlockedDecrement(&s_nRefCnt)) == 0 )
	{
		//no body else using the window so destroy it?
		TRACE(_T("CDelayedNotificationWindow -- Destroying the notification window\n"));
		DestroyWindow( s_hWnd );
		s_hWnd = NULL;
	}
	return nRefCnt;
}
BOOL CDelayedNotificationWindow::RegisterWindowClass(LPCTSTR szClassName)
//
//	registers our own window class to use as the hidden notification window.
//
{
	WNDCLASS wc = {0};
	
	wc.style = 0;
	wc.hInstance		= GetInstanceHandle();
	wc.lpszClassName	= szClassName;
	wc.hbrBackground	= (HBRUSH)GetStockObject( WHITE_BRUSH );
	wc.lpfnWndProc		= DelayedNotificationWndProc;
	
	ATOM ant = RegisterClass( &wc );
	if( ant == NULL )
	{
		TRACE(_T("CDirChangeNotification::RegisterWindowClass - RegisterClass failed: %d\n"), GetLastError());
	}
	return (BOOL)(ant!= NULL);
	
}

BOOL CDelayedNotificationWindow::CreateNotificationWindow()
//
//	Create the hidden notification windows.
//
{
	TCHAR szClassName[] = _T("Delayed_Message_Sender");
	if( !s_bRegisterWindow )
		s_bRegisterWindow = RegisterWindowClass(szClassName);
	s_hWnd 	= CreateWindowEx(0, szClassName, _T("DelayedWnd"),0,0,0,0,0, NULL, 0, 
							GetInstanceHandle(), NULL);
	if( s_hWnd == NULL )
	{
		TRACE(_T("Unable to create notification window! GetLastError(): %d\n"), GetLastError());
		TRACE(_T("File: %s Line: %d\n"), _T(__FILE__), __LINE__);
	}
	
	return (BOOL)(s_hWnd != NULL);
}
void CDelayedNotificationWindow::PostNotification(CDirChangeNotification * pNotification)
//
//	Posts a message to a window created in the main 
//	thread.
//	The main thread catches this message, and dispatches it in 
//	the context of the main thread.
//
{
	ASSERT( pNotification );
	ASSERT( s_hWnd );
	ASSERT( ::IsWindow( s_hWnd ) );

	PostMessage(s_hWnd, 
				UWM_DELAYED_DIRECTORY_NOTIFICATION, 
				0, 
				reinterpret_cast<LPARAM>( pNotification ));

//  if you don't want the notification delayed, 
//  
//	if( false )
//	{
//		pNotification->DispatchNotificationFunction();
//	}
}

/////////////////////////////////////////////////////////
//	CDelayedNoticationThread
//
long	CDelayedNotificationThread::s_nRefCnt = 0L;
HANDLE	CDelayedNotificationThread::s_hThread = NULL;
DWORD	CDelayedNotificationThread::s_dwThreadID = 0UL;

void CDelayedNotificationThread::PostNotification(CDirChangeNotification * pNotification)
{
	ASSERT( s_hThread != NULL );
	ASSERT( s_dwThreadID != 0 );

	if(
		!PostThreadMessage(s_dwThreadID, 
						   UWM_DELAYED_DIRECTORY_NOTIFICATION, 
						   0, 
						   reinterpret_cast<LPARAM>(pNotification))
	  )
	{
		//Note, this can sometimes fail.
		//Will fail if: s_dwThreadID references a invalid thread id(the thread has died for example)
		// OR will fail if the thread doesn't have a message queue.
		//
		//	This was failing because the thread had not been fully started by the time PostThreadMessage had been called
		//
		//Note: if this fails, it creates a memory leak because
		//the CDirChangeNotification object that was allocated and posted
		//to the thread is actually never going to be dispatched and then deleted.... it's
		//hanging in limbo.....

		//
		//	The fix for this situation was to force the thread that starts
		//	this worker thread to wait until the worker thread was fully started before
		//	continueing.  accomplished w/ an event... also.. posting a message to itself before signalling the 
		//  'spawning' thread that it was started ensured that there was a message pump
		//  associated w/ the worker thread by the time PostThreadMessage was called.
		TRACE(_T("PostThreadMessage() failed while posting to thread id: %d! GetLastError(): %d%s\n"), s_dwThreadID, GetLastError(), GetLastError() == ERROR_INVALID_THREAD_ID? _T("(ERROR_INVALID_THREAD_ID)") : _T(""));
	}
}

bool CDelayedNotificationThread::StartThread()
{
	TRACE(_T("CDelayedNotificationThread::StartThread()\n"));
	ASSERT( s_hThread == NULL 
		&&	s_dwThreadID == 0 );
	s_hThread = (HANDLE)_beginthreadex(NULL,0, 
								ThreadFunc, this, 0, (UINT*) &s_dwThreadID);
	if( s_hThread )
		WaitForThreadStartup();

	return s_hThread == NULL ? false : true;

}

bool CDelayedNotificationThread::StopThread()
{
	TRACE(_T("CDelayedNotificationThread::StopThread()\n"));
	if( s_hThread != NULL 
	&&	s_dwThreadID != 0 )
	{
		PostThreadMessage(s_dwThreadID, WM_QUIT, 0,0);

		WaitForSingleObject(s_hThread, INFINITE);
		CloseHandle(s_hThread);
		s_hThread	 = NULL;
		s_dwThreadID = 0UL;
		return true;
	}
	return true;//already shutdown
}

UINT __stdcall CDelayedNotificationThread::ThreadFunc(LPVOID lpvThis)
{
	//UNREFERENCED_PARAMETER( lpvThis );
	//
	//	Implements a simple message pump
	//
	CDelayedNotificationThread * pThis = reinterpret_cast<CDelayedNotificationThread*>(lpvThis);
	ASSERT( pThis );

	//
	//	Insure that this thread has a message queue by the time another
	//	thread gets control and tries to use PostThreadMessage
	//	problems can happen if someone tries to use PostThreadMessage
	//	in between the time pThis->SignalThreadStartup() is called,
	//	and the first call to GetMessage();

	::PostMessage(NULL, WM_NULL, 0,0);//if this thread didn't have a message queue before this, it does now.


	//
	//
	//	Signal that this thread has started so that StartThread can continue.
	//
	if( pThis ) pThis->SignalThreadStartup();

	TRACE(_T("CDelayedNotificationThread::ThreadFunc() ThreadID: %d -- Starting\n"), GetCurrentThreadId());
	MSG msg;
	do{
		while( GetMessage(&msg, NULL, 0,0) )//note GetMessage() can return -1, but only if i give it a bad HWND.(HWND for another thread for example)..i'm not giving an HWND, so no problemo here.
		{
			if( msg.message == UWM_DELAYED_DIRECTORY_NOTIFICATION )
			{
				CDirChangeNotification * pNotification = 
								reinterpret_cast<CDirChangeNotification *>( msg.lParam );
				DWORD dwEx(0UL);

				__try{
				if( pNotification )
					pNotification->DispatchNotificationFunction();
				}
				__except(dwEx = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER){
				//An exception was raised:
				//
				//	Likely causes: 
				//		* There was a problem creating the CDelayedDirectoryChangeHandler::m_hWatchStoppedDispatchedEvent object
				//			and the change handler object was deleted before the notification could be dispatched to this function.
				//
				//		* Somebody's implementation of an overridden virtual function caused an exception
				TRACE(_T("The following exception occurred: %d -- File: %s Line: %d\n"), dwEx, _T(__FILE__), __LINE__);
				}
			}
			else
			if( msg.message == WM_QUIT )
			{
				break;
			}
		}
	}while( msg.message != WM_QUIT );
	TRACE(_T("CDelayedNotificationThread::ThreadFunc() exiting. ThreadID: %d\n"), GetCurrentThreadId());
	return 0;
}

long CDelayedNotificationThread::AddRef()
{
	if( InterlockedIncrement(&s_nRefCnt) == 1 )
	{
		VERIFY( StartThread() );
	}
	return s_nRefCnt;
}
long CDelayedNotificationThread::Release()
{
	if( InterlockedDecrement(&s_nRefCnt) <= 0 )
	{
		s_nRefCnt = 0;
		VERIFY( StopThread() );
	}
	return s_nRefCnt;
}