#include "stdafx.h"
#include "FileNotifyInformation.h"

BOOL CFileNotifyInformation::GetNextNotifyInformation()
/***************
  Sets the m_pCurrentRecord to the next FILE_NOTIFY_INFORMATION record.

  Even if this return FALSE, (unless m_pCurrentRecord is NULL)
  m_pCurrentRecord will still point to the last record in the buffer.
****************/
{
	if( m_pCurrentRecord 
	&&	m_pCurrentRecord->NextEntryOffset != 0UL)//is there another record after this one?
	{
		//set the current record to point to the 'next' record
		PFILE_NOTIFY_INFORMATION pOld = m_pCurrentRecord;
		m_pCurrentRecord = (PFILE_NOTIFY_INFORMATION) ((LPBYTE)m_pCurrentRecord + m_pCurrentRecord->NextEntryOffset);

		ASSERT( (DWORD)((BYTE*)m_pCurrentRecord - m_pBuffer) < m_dwBufferSize);//make sure we haven't gone too far

		if( (DWORD)((BYTE*)m_pCurrentRecord - m_pBuffer) > m_dwBufferSize )
		{
			//we've gone too far.... this data is hosed.
			//
			// This sometimes happens if the watched directory becomes deleted... remove the FILE_SHARE_DELETE flag when using CreateFile() to get the handle to the directory...
			m_pCurrentRecord = pOld;
		}
					
		return (BOOL)(m_pCurrentRecord != pOld);
	}
	return FALSE;
}

BOOL CFileNotifyInformation::CopyCurrentRecordToBeginningOfBuffer(OUT DWORD & ref_dwSizeOfCurrentRecord)
/*****************************************
   Copies the FILE_NOTIFY_INFORMATION record to the beginning of the buffer
   specified in the constructor.

   The size of the current record is returned in DWORD & dwSizeOfCurrentRecord.
   
*****************************************/
{
	ASSERT( m_pBuffer && m_pCurrentRecord );
	if( !m_pCurrentRecord ) return FALSE;

	BOOL bRetVal = TRUE;

	//determine the size of the current record.
	ref_dwSizeOfCurrentRecord = sizeof( FILE_NOTIFY_INFORMATION );
	//subtract out sizeof FILE_NOTIFY_INFORMATION::FileName[1]
	WCHAR FileName[1];//same as is defined for FILE_NOTIFY_INFORMATION::FileName
	UNREFERENCED_PARAMETER(FileName);
	ref_dwSizeOfCurrentRecord -= sizeof(FileName);   
	//and replace it w/ value of FILE_NOTIFY_INFORMATION::FileNameLength
	ref_dwSizeOfCurrentRecord += m_pCurrentRecord->FileNameLength;

	ASSERT( (DWORD)((LPBYTE)m_pCurrentRecord + ref_dwSizeOfCurrentRecord) <= m_dwBufferSize );

	ASSERT( (void*)m_pBuffer != (void*)m_pCurrentRecord );//if this is the case, your buffer is way too small
	if( (void*)m_pBuffer != (void*) m_pCurrentRecord )
	{//copy the m_pCurrentRecord to the beginning of m_pBuffer
		
		ASSERT( (DWORD)m_pCurrentRecord > (DWORD)m_pBuffer + ref_dwSizeOfCurrentRecord);//will it overlap?
		__try{
			memcpy(m_pBuffer, m_pCurrentRecord, ref_dwSizeOfCurrentRecord);
			bRetVal = TRUE;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			TRACE(_T("EXCEPTION!  CFileNotifyInformation::CopyCurrentRecordToBeginningOfBuffer() -- probably because bytes overlapped in a call to memcpy()"));
			bRetVal = FALSE;
		}
	}
	//else
	//there was only one record in this buffer, and m_pCurrentRecord is already at the beginning of the buffer
	return bRetVal;
}

DWORD CFileNotifyInformation::GetAction() const
{ 
	ASSERT( m_pCurrentRecord );
	if( m_pCurrentRecord )
		return m_pCurrentRecord->Action;
	return 0UL;
}

CString CFileNotifyInformation::GetFileName() const
{
	//
	//BUG FIX:
	//		File Name's longer than 130 characters are truncated
	//
	//		Thanks Edric @ uo_edric@hotmail.com for pointing this out.
	if( m_pCurrentRecord )
	{
		WCHAR wcFileName[ MAX_PATH + 1] = {0};//L"";
		memcpy(	wcFileName, 
				m_pCurrentRecord->FileName, 
				//min( MAX_PATH, m_pCurrentRecord->FileNameLength) <-- buggy line
				min( (MAX_PATH * sizeof(WCHAR)), m_pCurrentRecord->FileNameLength));
		

		return CString( wcFileName );
	}
	return CString();
}		

static inline bool HasTrailingBackslash(const CString & str )
{
	if( str.GetLength() > 0 
	&&	str[ str.GetLength() - 1 ] == _T('\\') )
		return true;
	return false;
}
CString CFileNotifyInformation::GetFileNameWithPath(const CString & strRootPath) const
{
	CString strFileName( strRootPath );
	//if( strFileName.Right(1) != _T("\\") )
	if( !HasTrailingBackslash( strRootPath ) )
		strFileName += _T("\\");

	strFileName += GetFileName();
	return strFileName;
}