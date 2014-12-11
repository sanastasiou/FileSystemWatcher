#pragma once

#include <afxmt.h>
#include <afxtempl.h>

class CDirChangeNotification;

class CDelayedNotifier
//
//	Abstract base class for ensuring notifications are fired in a thread 
//
//
{
public:
	virtual ~CDelayedNotifier(){}
	virtual void PostNotification(CDirChangeNotification * pNotification) = 0;

};

class CDelayedNotificationWindow : public CDelayedNotifier
//
//	A class that implements a
//	there will always be only one of the actual windows 
//	in existance. 
//
{
public:
		CDelayedNotificationWindow(){  AddRef(); }
		virtual ~CDelayedNotificationWindow(){ Release(); }
		

		void PostNotification(CDirChangeNotification * pNotification);
private:
		long AddRef();		//	the window handle is reference counted
		long Release();		//

		static long s_nRefCnt;
		static HWND s_hWnd; //there's only one window no matter how many instances of this class there are.... this means that all notifications are handled by the same thread.
		static BOOL s_bRegisterWindow;
		BOOL RegisterWindowClass(LPCTSTR szClassName);
		BOOL CreateNotificationWindow();
};

class CDelayedNotificationThread : public CDelayedNotifier
//
//	Class that implements a worker thread w/ a message pump.
//	CDirectoryChangeWatcher posts notifications to this thread, where they are dispatched.
//	This thread executes CDirectoryChangeHandler notifications.
//
{
public:
	CDelayedNotificationThread()
		:m_hThreadStartEvent(NULL)
	{ 
		m_hThreadStartEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
		ASSERT( m_hThreadStartEvent );
		AddRef(); 
	}
	virtual ~CDelayedNotificationThread()
	{ 
		Release(); 
		if( m_hThreadStartEvent ) 
			CloseHandle(m_hThreadStartEvent), m_hThreadStartEvent = NULL;
	}

	void PostNotification(CDirChangeNotification * pNotification);

private:
	long AddRef();					// The thread handle is reference
	long Release();					// counted so that only one thread is used
									// so that there's only one worker thread(performing this functino)
	static long		s_nRefCnt;		// no matter how many directories are being watched
	static HANDLE	s_hThread;		//	
	static DWORD	s_dwThreadID;	//  
										
	static UINT __stdcall ThreadFunc(LPVOID lpvThis);

	bool StartThread();
	bool StopThread();

	BOOL WaitForThreadStartup(){ return WaitForSingleObject(m_hThreadStartEvent, INFINITE) == WAIT_OBJECT_0; };
	BOOL SignalThreadStartup(){ return SetEvent( m_hThreadStartEvent ) ; }

	HANDLE m_hThreadStartEvent;//signals that the worker thread has started. this fixes a bug condition.
		
};