#ifndef WINDOWS_FILE_IFILESYSTEMWATCHER_H__
#define WINDOWS_FILE_IFILESYSTEMWATCHER_H__

#include "Windows.h"
#include<TCHAR.H> // Implicit or explicit include
#include <string>
#include <vector>
#include "ConqurrentQueue.h"
#include "WindowsBase/Thread.h"

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
        virtual void OnFileModified(const FileSystemString & strFileName)const = 0;

        struct DirectoryInfo
        {
            FileSystemString _dir;
            ::DWORD _changeFlags;
            ::BOOL _watchSubDirectories;
            IFileSystemWatcher const * const _eventHandler;
            FileSystemString _includeFilter;
            FileSystemString _excludeFilter;
        };
    };

    struct FileSystemWatcherBase : IFileSystemWatcher
    {
        typedef std::basic_string< ::TCHAR > FileSystemString;

        virtual bool IsWatching()const = 0;

        virtual ~FileSystemWatcherBase()
        {
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
        FileSystemWatcherBase( FileSystemString const & dir,
                               ::DWORD changeFlags,
                               ::BOOL const watchSubDir,
                               IFileSystemWatcher const * const eventHandler,
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
        static void CALLBACK TerminateProc(__in  ULONG_PTR arg)
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

        void StartPoppingEvents()
        {
            _popEvents = true;
            _eventThread.Start(this);
        }

        void StopPopping()
        {
            _popEvents = false;
            _eventThread.Stop();
            _eventQueue.clear();
        }

        bool IsPopping()const
        {
            return _eventThread.IsStarted();
        }

        DirectoryInfo _directoryInfo;                                   //!< Simple struct with all relevant information for directory changes.
        bool _terminate;                                                //!< Notify wathcer to terminate dir observation.
        ::OVERLAPPED   _overlapped;                                     //!< Required parameter for ReadDirectoryChangesW().
        std::vector<::BYTE> _buffer;                                    //!< Data buffer for the request. Since the memory is allocated by malloc, it will always be aligned as required by ReadDirectoryChangesW().
        std::vector<::BYTE> _backupBuffer;                              //!< Double buffer strategy so that we can issue a new read request before we process the current buffer.
        std::vector<BYTE>::size_type const BUFFER_SIZE;                 //!< Max notification buffer size.
        ::HANDLE _dirHandle;                                            //!< Handle to the observed directory.
        ConqurrentQueue<std::pair<std::wstring, ::DWORD> > _eventQueue; //!< Holds file system events.
        Threading::Thread _eventThread;                                 //!< This thread is just used to pop event from the conqurrent queue.
        bool _popEvents;                                                //!< Indicates if pop event thread should continue publishing events.

    private:
        void DoPopEvents();

        void ClassifyAndPostEvent(std::pair<std::wstring, ::DWORD> const & e)const;
    };

    inline FileSystemWatcherBase::FileSystemWatcherBase( FileSystemString const & dir,
                                                         ::DWORD changeFlags,
                                                         ::BOOL const watchSubDir,
                                                         IFileSystemWatcher const * const eventHandler,
                                                         FileSystemString includeFilter,
                                                         FileSystemString excludeFilter,
                                                         std::vector<BYTE>::size_type const bufferSize) :
        _directoryInfo(DirectoryInfo{ dir, changeFlags, watchSubDir, eventHandler, includeFilter, excludeFilter}),
        _terminate(false),
        BUFFER_SIZE(bufferSize),
        _dirHandle(INVALID_HANDLE_VALUE),
        _eventThread(FileSystemWatcherBase::PopEventsProc),
        _popEvents(true)
    {
        ::ZeroMemory(&_overlapped, sizeof(::OVERLAPPED));
        _overlapped.hEvent = this;
        _buffer.resize(bufferSize);
        _backupBuffer.resize(bufferSize);
    }

    inline void FileSystemWatcherBase::BeginRead(::LPOVERLAPPED_COMPLETION_ROUTINE const notificationCallback, ::HANDLE const dirHandle)
    {
        ::DWORD dwBytes = 0U;
        // This call needs to be reissued after every APC.
        (void)::ReadDirectoryChangesW(
            dirHandle,                           // handle to directory
            _buffer.data(),                      // read results buffer
            _buffer.size(),                      // length of buffer
            _directoryInfo._watchSubDirectories, // monitoring option
            _directoryInfo._changeFlags,         // filter conditions
            &dwBytes,                            // bytes returned
            &_overlapped,                        // overlapped buffer
            notificationCallback);               // completion routine
    }

    inline void FileSystemWatcherBase::DoPopEvents()
    {
        while (_popEvents)
        {
            ConqurrentQueue<std::pair<std::wstring, ::DWORD> >::value_type e;
            _eventQueue.wait_and_pop(e);
            ClassifyAndPostEvent(e);
        }
    }

    inline void FileSystemWatcherBase::ClassifyAndPostEvent(std::pair<std::wstring, ::DWORD> const & e)const
    {
        switch (e.second)
        {
        case FILE_ACTION_ADDED:
            
            break;
        case FILE_ACTION_REMOVED:
            break;
        case FILE_ACTION_MODIFIED:
            _directoryInfo._eventHandler->OnFileModified(e.first);
            break;
        case FILE_ACTION_RENAMED_OLD_NAME:
            break;
        case FILE_ACTION_RENAMED_NEW_NAME:
            break;
        default:
            break;
        }
    }
} // namespace File
} // namespace Windows

#endif //#ifndef WINDOWS_FILE_IFILESYSTEMWATCHER_H__