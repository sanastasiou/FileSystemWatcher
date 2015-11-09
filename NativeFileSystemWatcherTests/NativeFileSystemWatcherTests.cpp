#include "gtest/gtest.h"
#include "NativeFileSystemWatcher/NativeFileSystemWatcher.h"
#include "NativeFileSystemWatcher/IFileSystemWatcher.h"
#include "Windows.h"
#include "Direct.h"
#include <fstream>
#include <iostream>

struct FileNotificationReceiver : Windows::File::IFileSystemWatcher
{
    FileSystemString _lastNotificationFile;
    unsigned _notificationCount;
    FileSystemString _oldFileName;

    FileNotificationReceiver() : _lastNotificationFile(L""), _notificationCount(0), _oldFileName(L"")
    {
    }

    virtual void OnFileModified(const FileSystemString & strFileName)
    {
        _lastNotificationFile = strFileName;
        ++_notificationCount;
        _oldFileName = L"";
    }

    virtual ~FileNotificationReceiver()
    {
        _lastNotificationFile = L"";;
        _notificationCount = 0;
    }

    virtual void OnFileRenamed(const FileSystemString & newFileName, const FileSystemString & oldFileName)
    {
        _oldFileName = oldFileName;
        _lastNotificationFile = newFileName;
        ++_notificationCount;
    }

    virtual void OnFileRemoved(const FileSystemString & strFileName)
    {
        _lastNotificationFile = strFileName;
        _oldFileName = L"";
        ++_notificationCount;
    }

    virtual void OnFileAdded(const FileSystemString & strFileName)
    {
        _lastNotificationFile = strFileName;
        _oldFileName = L"";
        ++_notificationCount;
    }
};

class NativeFileSystemWatcherTestFixture : public ::testing::Test
{
    virtual void SetUp()
    {
        _cwd.reserve(MAX_PATH);
        _cwd = _wgetcwd(&_cwd[0], MAX_PATH);
        _testFile = L"TestFile.txt";

        _testDir = _cwd + L"\\TestFiles";

        if (!Windows::Common::Base::DirExist(_testDir.c_str()))
        {
            Windows::Common::Base::MakeDir(_testDir.c_str());
        }
        //delete previous files
        Windows::Common::Base::DeleteDirRecursive(_testDir.c_str(), false, true);

        FILE * _testFileContent;

        _testFilePath = _testDir + L"\\" + _testFile;

        Windows::Utilities::File::CreateNewFile(_testFilePath.c_str(), _testFileContent);

        if (_testFileContent)
        {
            ::fclose(_testFileContent);
        }
    }

    virtual void TearDown()
    {
    }

protected:
    FileNotificationReceiver * _fileNotificationReceiver = nullptr;
    std::wstring _cwd;
    std::wstring _testDir;
    std::wstring _testFile;
    std::wstring _testFilePath;
};

TEST_F(NativeFileSystemWatcherTestFixture, SmokeTest)
{
    _fileNotificationReceiver = new FileNotificationReceiver();
    Windows::File::NativeFileSystemWatcher * myWatcher = new Windows::File::NativeFileSystemWatcher(L"C:\\FooDirectoryLALALALALALALAWLWWLWLWL",
        (FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_ATTRIBUTES),
        true,
        _fileNotificationReceiver,
        L"*.atm",
        L"",
        16384);

    ::delete myWatcher;
}

TEST_F(NativeFileSystemWatcherTestFixture, EnsureNonExistantDirIsNotWatched)
{
    _fileNotificationReceiver = new FileNotificationReceiver();
    Windows::File::NativeFileSystemWatcher * myWatcher = new Windows::File::NativeFileSystemWatcher(L"",
        (FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_ATTRIBUTES),
        true,
        _fileNotificationReceiver,
        L"*.atm",
        L"",
        16384);
    ASSERT_FALSE(myWatcher->IsWatching());
    ::delete myWatcher;
}

TEST_F(NativeFileSystemWatcherTestFixture, EnsureExistantDirIsWatched)
{
    _fileNotificationReceiver = new FileNotificationReceiver();
    Windows::File::NativeFileSystemWatcher * myWatcher = new Windows::File::NativeFileSystemWatcher(_cwd,
        (FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_ATTRIBUTES),
        true,
        _fileNotificationReceiver,
        L"*.atm",
        L"",
        16384);
    ASSERT_TRUE(myWatcher->IsWatching());
    ::delete myWatcher;
}

TEST_F(NativeFileSystemWatcherTestFixture, FileModificationTestInclusionFilterFailed)
{
    _fileNotificationReceiver = new FileNotificationReceiver();

    Windows::File::NativeFileSystemWatcher * myWatcher = new Windows::File::NativeFileSystemWatcher(_cwd,
        (FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_ATTRIBUTES),
        true,
        _fileNotificationReceiver,
        L"*.atm",
        L"",
        16384);
    ASSERT_TRUE(myWatcher->IsWatching());

    std::ofstream myfile(_testFilePath);
    myfile << "Writing this to a file.\n";

    ::Sleep(1);

    ASSERT_FALSE(_fileNotificationReceiver->_lastNotificationFile == _testFilePath);
    ASSERT_EQ(_fileNotificationReceiver->_notificationCount, 0U );

    ::delete myWatcher;
    myfile.close();
}

TEST_F(NativeFileSystemWatcherTestFixture, FileModificationTestInclusionFilterSucceeded)
{
    _fileNotificationReceiver = new FileNotificationReceiver();

    Windows::File::NativeFileSystemWatcher * myWatcher = new Windows::File::NativeFileSystemWatcher(_cwd,
        (FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_ATTRIBUTES),
        true,
        _fileNotificationReceiver,
        L"*.txt",
        L"",
        16384);
    ASSERT_TRUE(myWatcher->IsWatching());

    std::ofstream myfile(_testFilePath);
    myfile << "Writing this to a file.\n";

    ::Sleep(1);

    ASSERT_TRUE(_fileNotificationReceiver->_lastNotificationFile == _testFilePath);
    ASSERT_GE(_fileNotificationReceiver->_notificationCount, 1U);

    ::delete myWatcher;
    myfile.close();
}

int main(int argc, wchar_t ** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}