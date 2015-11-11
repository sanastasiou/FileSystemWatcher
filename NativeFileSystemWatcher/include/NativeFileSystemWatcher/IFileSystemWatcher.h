#ifndef WINDOWS_FILE_IFILESYSTEMWATCHER_H__
#define WINDOWS_FILE_IFILESYSTEMWATCHER_H__

#if defined(WINDOWS_FILE_IFILESYSTEMWATCHER_DLL_EXPORTS)
#define WINDOWS_FILE_IFILESYSTEMWATCHER_API __declspec (dllexport)
#else
#define WINDOWS_FILE_IFILESYSTEMWATCHER_API __declspec (dllimport)
#endif

#include "Windows.h"
#include<TCHAR.H> // Implicit or explicit include
#include <string>
#include <vector>
#include "ConqurrentQueue.h"
#include "WindowsBase/Thread.h"
#include <algorithm>
#include "WindowsBase/File.h"

namespace Windows
{
namespace File
{
    struct IFileSystemWatcher
    {
        typedef std::basic_string< ::TCHAR > FileSystemString;

        /**
        * \brief   Occurs when a file was modified.
        *
        * \param   strFileName Filename of the file which was modified.
        */
        WINDOWS_FILE_IFILESYSTEMWATCHER_API virtual void OnFileModified(const FileSystemString & strFileName) = 0;

        WINDOWS_FILE_IFILESYSTEMWATCHER_API virtual void OnFileRenamed(const FileSystemString & newFileName, const FileSystemString & oldFileName) = 0;

        WINDOWS_FILE_IFILESYSTEMWATCHER_API virtual void OnFileRemoved(const FileSystemString & strFileName) = 0;

        WINDOWS_FILE_IFILESYSTEMWATCHER_API virtual void OnFileAdded(const FileSystemString & strFileName) = 0;

        WINDOWS_FILE_IFILESYSTEMWATCHER_API virtual void OnError(::DWORD const errorCode, const FileSystemString & directory) = 0;

        struct DirectoryInfo
        {
            DirectoryInfo( FileSystemString const & dir, 
                           ::DWORD const changeFlags, 
                           ::BOOL const watchSubDirectories,
                           IFileSystemWatcher * eventHandler, 
                           FileSystemString const & includeFilter,
                           FileSystemString const & excludeFilter):
                _dir(dir),
                _changeFlags(changeFlags),
                _watchSubDirectories(watchSubDirectories),
                _eventHandler(eventHandler),
                _includeFilter(includeFilter),
                _excludeFilter(excludeFilter)
            {}

            FileSystemString _dir;
            ::DWORD _changeFlags;
            ::BOOL _watchSubDirectories;
            IFileSystemWatcher * _eventHandler;
            FileSystemString _includeFilter;
            FileSystemString _excludeFilter;
        };

        WINDOWS_FILE_IFILESYSTEMWATCHER_API virtual ~IFileSystemWatcher();
    };

    struct FileSystemWatcherBase : IFileSystemWatcher
    {
        typedef std::basic_string< ::TCHAR > FileSystemString;

        virtual bool IsWatching()const = 0;

        virtual ~FileSystemWatcherBase()
        {
            if (_directoryInfo._eventHandler != nullptr)
            {
                ::delete _directoryInfo._eventHandler;
                _directoryInfo._eventHandler = nullptr;
            }
        }

        // The dwSize is the actual number of bytes sent to the APC.
        void BackupBuffer(DWORD dwSize)
        {
            // We could just swap back and forth between the two
            // buffers, but this code is easier to understand and debug.
            ::memcpy(_backupBuffer.data(), _buffer.data(), dwSize);
        }

        ::HANDLE & GetDirHandle()
        {
            return _dirHandle;
        }

        ::HANDLE const & GetDirHandle()const
        {
            return _dirHandle;
        }

        virtual LPOVERLAPPED_COMPLETION_ROUTINE GetNotificationRoutine()const = 0;

        virtual void ProcessNotification() = 0;

        void BeginRead(::LPOVERLAPPED_COMPLETION_ROUTINE const notificationCallback, ::HANDLE const dirHandle);
    protected:
        DirectoryInfo _directoryInfo;                                   //!< Simple struct with all relevant information for directory changes.
        bool _terminate;                                                //!< Notify wathcer to terminate dir observation.
        ::OVERLAPPED   _overlapped;                                     //!< Required parameter for ReadDirectoryChangesW().
        ::HANDLE _dirHandle;                                            //!< Handle to the observed directory.
        Threading::Thread _eventThread;                                 //!< This thread is just used to pop event from the conqurrent queue.
        bool _popEvents;                                                //!< Indicates if pop event thread should continue publishing events.

        FileSystemWatcherBase( FileSystemString const & dir,
                               ::DWORD changeFlags,
                               ::BOOL const watchSubDir,
                               IFileSystemWatcher * const eventHandler,
                               FileSystemString includeFilter,
                               FileSystemString excludeFilter,
                               std::vector<BYTE>::size_type const bufferSize);

        static unsigned int ThreadStartProc(void * arg)
        {
            FileSystemWatcherBase* base = static_cast<FileSystemWatcherBase*>(arg);
            base->Run();
            return 0;
        }

        // Called by QueueUserAPC to start orderly shutdown.
        static void CALLBACK TerminateWatching(__in  ULONG_PTR arg)
        {
            FileSystemWatcherBase* base = reinterpret_cast<FileSystemWatcherBase*>(arg);
            base->RequestTermination();
        }

        // Called by QueueUserAPC to add another directory.
        static void CALLBACK AddDirectoryProc(__in  ULONG_PTR arg)
        {
            FileSystemWatcherBase * pWatcher = reinterpret_cast<FileSystemWatcherBase*>(arg);
            pWatcher->BeginRead(pWatcher->GetNotificationRoutine(), pWatcher->GetDirHandle());
        }

        static unsigned int PopEventsProc(void * arg)
        {
            FileSystemWatcherBase * pWatcher = static_cast<FileSystemWatcherBase*>(arg);
            pWatcher->DoPopEvents();
            return 0;
        }

        void Run()
        {
            while (!_terminate)
            {
                (void)::SleepEx(INFINITE, true);
            }
        }

        // Call this from dervied classes, else thread won't terminate.
        virtual void RequestTermination() = 0
        {
            _terminate = true;
        }

        ConqurrentQueue<std::pair<std::wstring, ::DWORD> > & GetEventQueue()
        {
            return _eventQueue;
        }

        std::vector<::BYTE> & GetBackUpBuffer()
        {
            return _backupBuffer;
        }

        void StartPoppingEvents()
        {
            _popEvents = true;
            _eventThread.Start(this);
        }

        void StopPopping()
        {
            _popEvents = false;
            //send "dummy" event that watching has been terminated, it's up to the user to restart watching if needed
            _eventQueue.push(std::make_pair(L"", ERROR_OPERATION_ABORTED));
            _eventThread.Stop();
        }

        bool IsPopping()const
        {
            return _eventThread.IsStarted();
        }

        bool IsExcludeFilterActive()const;

        bool IsIncludeFilterActive()const;

        bool IsFileIncluded(std::wstring const & file)const;

        bool IsFileExcluded(std::wstring const & file)const;

        void SetIncludeFilter(IFileSystemWatcher::FileSystemString const & newInclusionFilter);

        void SetExcludeFilter(IFileSystemWatcher::FileSystemString const & newExclusionFilter);

        void ResetOverlappedStructure();
    private:
        static const std::size_t MAX_BUFFER_SIZE = 65535U;

        std::vector<::BYTE> _buffer;                                    //!< Data buffer for the request. Since the memory is allocated by malloc, it will always be aligned as required by ReadDirectoryChangesW().
        std::vector<::BYTE> _backupBuffer;                              //!< Double buffer strategy so that we can issue a new read request before we process the current buffer.
        std::vector<BYTE>::size_type const BUFFER_SIZE;                 //!< Max notification buffer size.
        ConqurrentQueue<std::pair<std::wstring, ::DWORD> > _eventQueue; //!< Holds file system events.
        bool _isIncludeFilterActive;                                    //!< Indicates if include filter is active.
        bool _isExcludeFilterActive;                                    //!< Indicates if exclude filter is active.
        std::vector<std::wstring> _includeFilters;                      //!< Stores include filters.
        std::vector<std::wstring> _excludeFilters;                      //!< Stores exclude filters.
        std::wstring _tmpOldFileName;                                   //!< Store old file name is case of file renaming event.

        void DoPopEvents();

        void ClassifyAndPostEvent(std::pair<std::wstring, ::DWORD> const & e);

        void InitializeFilters();

        /*
        *   If true, indicates that at least one filter is present. Filters are stored in "filters" parameter.
        */
        bool ParseFilter(std::wstring const & filter, std::vector<std::wstring> & filters)const;
    };

    inline void FileSystemWatcherBase::ClassifyAndPostEvent(std::pair<std::wstring, ::DWORD> const & e)
    {
        switch (e.second)
        {
        case FILE_ACTION_ADDED:
            _directoryInfo._eventHandler->OnFileAdded(e.first);
            break;
        case FILE_ACTION_REMOVED:
            _directoryInfo._eventHandler->OnFileRemoved(e.first);
            break;
        case FILE_ACTION_MODIFIED:
            _directoryInfo._eventHandler->OnFileModified(e.first);
            break;
        case FILE_ACTION_RENAMED_OLD_NAME:
            _tmpOldFileName = e.first;
            //this has to come always first, in case a file name is changed
            break;
        case FILE_ACTION_RENAMED_NEW_NAME:
            //after a FILE_ACTION_RENAMED_OLD_NAME this has to come next, only then we have the complete event to dispatch
            _directoryInfo._eventHandler->OnFileRenamed(e.first, _tmpOldFileName);
            break;
        default:
            //error
            _directoryInfo._eventHandler->OnError(e.second, _directoryInfo._dir);
            OnError(e.second, _directoryInfo._dir);
            break;
        }
    }

    inline bool FileSystemWatcherBase::IsIncludeFilterActive()const
    {
        return _isIncludeFilterActive;
    }

    inline bool FileSystemWatcherBase::IsExcludeFilterActive()const
    {
        return _isExcludeFilterActive;
    }

    inline bool FileSystemWatcherBase::IsFileIncluded(std::wstring const & file)const
    {
        return _isIncludeFilterActive && std::find_if(_includeFilters.begin(), _includeFilters.end(), [&file](std::wstring const & wildcard) { 
            return Windows::Utilities::String::wildcard_compare(wildcard, file);
        }) != _includeFilters.end();
    }

    inline bool FileSystemWatcherBase::IsFileExcluded(std::wstring const & file)const
    {
        return _isExcludeFilterActive && std::find_if(_excludeFilters.begin(), _excludeFilters.end(), [&file](std::wstring const & wildcard) {
            return Windows::Utilities::String::wildcard_compare(wildcard, file);
        }) != _excludeFilters.end();
    }
} // namespace File
} // namespace Windows

#endif //#ifndef WINDOWS_FILE_IFILESYSTEMWATCHER_H__