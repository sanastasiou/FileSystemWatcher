#pragma once

#include <afxmt.h>
#include <afxtempl.h>

class CFileNotifyInformation 
/*******************************

A Class to more easily traverse the FILE_NOTIFY_INFORMATION records returned 
by ReadDirectoryChangesW().

FILE_NOTIFY_INFORMATION is defined in Winnt.h as: 

 typedef struct _FILE_NOTIFY_INFORMATION {
    DWORD NextEntryOffset;
	DWORD Action;
    DWORD FileNameLength;
    WCHAR FileName[1];
} FILE_NOTIFY_INFORMATION, *PFILE_NOTIFY_INFORMATION;	

  ReadDirectoryChangesW basically puts x amount of these records in a 
  buffer that you specify.
  The FILE_NOTIFY_INFORMATION structure is a 'dynamically sized' structure (size depends on length
  of the file name (+ sizeof the DWORDs in the struct))

  Because each structure contains an offset to the 'next' file notification
  it is basically a singly linked list.  This class treats the structure in that way.
  

  Sample Usage:
  BYTE Read_Buffer[ 4096 ];

  ...
  ReadDirectoryChangesW(...Read_Buffer, 4096,...);
  ...

  CFileNotifyInformation notify_info( Read_Buffer, 4096);
  do{
	    switch( notify_info.GetAction() )
		{
		case xx:
		    notify_info.GetFileName();
		}

  while( notify_info.GetNextNotifyInformation() );
  
********************************/
{
public:
	CFileNotifyInformation( BYTE * lpFileNotifyInfoBuffer, DWORD dwBuffSize)
	: m_pBuffer( lpFileNotifyInfoBuffer ),
	  m_dwBufferSize( dwBuffSize )
	{
		ASSERT( lpFileNotifyInfoBuffer && dwBuffSize );
		
		m_pCurrentRecord = (PFILE_NOTIFY_INFORMATION) m_pBuffer;
	}

	
	BOOL GetNextNotifyInformation();
	
	BOOL CopyCurrentRecordToBeginningOfBuffer(OUT DWORD & ref_dwSizeOfCurrentRecord);

	DWORD	GetAction() const;//gets the type of file change notifiation
	CString GetFileName()const;//gets the file name from the FILE_NOTIFY_INFORMATION record
	CString GetFileNameWithPath(const CString & strRootPath) const;//same as GetFileName() only it prefixes the strRootPath into the file name

	
protected:
	BYTE * m_pBuffer;//<--all of the FILE_NOTIFY_INFORMATION records 'live' in the buffer this points to...
	DWORD  m_dwBufferSize;
	PFILE_NOTIFY_INFORMATION m_pCurrentRecord;//this points to the current FILE_NOTIFY_INFORMATION record in m_pBuffer
	
};