#include "NativeFileSystemWatcher/IFileSystemWatcher.h"
#include <algorithm>
#pragma unmanaged
#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/split.hpp"
#pragma managed

namespace Windows
{
namespace File
{
    IFileSystemWatcher::~IFileSystemWatcher()
    {
    }

    FileSystemWatcherBase::FileSystemWatcherBase(FileSystemString const & dir,
        ::DWORD changeFlags,
        ::BOOL const watchSubDir,
        IFileSystemWatcher * const eventHandler,
        FileSystemString includeFilter,
        FileSystemString excludeFilter,
        std::vector<BYTE>::size_type const bufferSize) :
        _directoryInfo(dir, changeFlags, watchSubDir, eventHandler, includeFilter, excludeFilter),
        _terminate(false),
        BUFFER_SIZE(bufferSize > MAX_BUFFER_SIZE ? MAX_BUFFER_SIZE : bufferSize),
        _dirHandle(INVALID_HANDLE_VALUE),
        _eventThread(FileSystemWatcherBase::PopEventsProc),
        _popEvents(true),
        _isIncludeFilterActive(false),
        _isExcludeFilterActive(false),
        _includeFilters(),
        _excludeFilters(),
        _tmpOldFileName()
    {
        ResetOverlappedStructure();
        _buffer.resize(bufferSize);
        _backupBuffer.resize(bufferSize);
        //initialize filters
        InitializeFilters();
    }

    void FileSystemWatcherBase::BeginRead(::LPOVERLAPPED_COMPLETION_ROUTINE const notificationCallback, ::HANDLE const dirHandle)
    {
        ::DWORD dwBytes = 0U;
        // This call needs to be reissued after every APC.
        (void)::ReadDirectoryChangesW(  dirHandle,                           // handle to directory
                                        _buffer.data(),                      // read results buffer
                                        _buffer.size(),                      // length of buffer
                                        _directoryInfo._watchSubDirectories, // monitoring option
                                        _directoryInfo._changeFlags,         // filter conditions
                                        &dwBytes,                            // bytes returned
                                        &_overlapped,                        // overlapped buffer
                                        notificationCallback);               // completion routine
    }

    void FileSystemWatcherBase::DoPopEvents()
    {
        while (_popEvents)
        {
            ConqurrentQueue<std::pair<std::wstring, ::DWORD> >::value_type e;
            _eventQueue.wait_and_pop(e);
            ClassifyAndPostEvent(e);
        }
    }

    void FileSystemWatcherBase::InitializeFilters()
    {
        if(!_directoryInfo._includeFilter.empty())
        {
            _includeFilters.clear();
            _isIncludeFilterActive = ParseFilter(_directoryInfo._includeFilter, _includeFilters);
        }
        else
        {
            _isIncludeFilterActive = false;
        }
        if (!_directoryInfo._excludeFilter.empty())
        {
            _excludeFilters.clear();
            _isExcludeFilterActive = ParseFilter(_directoryInfo._excludeFilter, _excludeFilters);
        }
        else
        {
            _isExcludeFilterActive = false;
        }
    }

    bool FileSystemWatcherBase::ParseFilter(std::wstring const & filter, std::vector<std::wstring> & filters)const
    {
        return !boost::split(filters, filter, boost::is_any_of(L";"), boost::token_compress_on).empty();
    }

    void FileSystemWatcherBase::SetIncludeFilter(IFileSystemWatcher::FileSystemString const & newInclusionFilter)
    {
        if (_directoryInfo._includeFilter != newInclusionFilter)
        {
            _directoryInfo._includeFilter = newInclusionFilter;
            InitializeFilters();
        }
    }

    void FileSystemWatcherBase::SetExcludeFilter(IFileSystemWatcher::FileSystemString const & newExclusionFilter)
    {
        if (_directoryInfo._excludeFilter != newExclusionFilter)
        {
            _directoryInfo._excludeFilter = newExclusionFilter;
            InitializeFilters();
        }
    }
    
    void FileSystemWatcherBase::ResetOverlappedStructure()
    {
        ::ZeroMemory(&_overlapped, sizeof(::OVERLAPPED));
        _overlapped.hEvent = this;
    }
} // namespace File
} // namespace Windows