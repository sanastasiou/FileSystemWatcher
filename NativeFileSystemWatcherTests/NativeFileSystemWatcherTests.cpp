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
    unsigned _deletionCount;
    unsigned _additionCount;
    unsigned _renamingCount;
    unsigned _errorCount;

    FileNotificationReceiver() : 
        _lastNotificationFile(L""),
        _notificationCount(0),
        _oldFileName(L""),
        _deletionCount(0),
        _additionCount(0),
        _renamingCount(0),
        _errorCount(0)
    {
    }

    virtual void OnError(::DWORD const, const FileSystemString &)
    {
        ++_notificationCount;
        ++_errorCount;
    }

    virtual void OnFileModified(const FileSystemString & strFileName)
    {
        _lastNotificationFile = strFileName;
        ++_notificationCount;
    }

    virtual ~FileNotificationReceiver()
    {
        _oldFileName = _lastNotificationFile = L"";
        _notificationCount = _deletionCount = _additionCount = _renamingCount = 0;
    }

    virtual void OnFileRenamed(const FileSystemString & newFileName, const FileSystemString & oldFileName)
    {
        _oldFileName = oldFileName;
        _lastNotificationFile = newFileName;
        ++_notificationCount;
        ++_renamingCount;
    }

    virtual void OnFileRemoved(const FileSystemString & strFileName)
    {
        _lastNotificationFile = strFileName;
        ++_notificationCount;
        ++_deletionCount;
    }

    virtual void OnFileAdded(const FileSystemString & strFileName)
    {
        _lastNotificationFile = strFileName;
        ++_notificationCount;
        ++_additionCount;
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

        _testFilePath = _testDir + L"\\" + _testFile;

        CreateNewFile(_testFilePath);
    }

    virtual void TearDown()
    {
    }

protected:
    void CreateNewFile(std::wstring const & filePath)
    {
        FILE * _testFileContent;

        if (Windows::Utilities::File::CreateNewFile(filePath.c_str(), _testFileContent) && _testFileContent)
        {
            ::fclose(_testFileContent);
        }
    }

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
        false,
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
        false,
        16384);
    ASSERT_FALSE(myWatcher->IsWatching());
    ASSERT_EQ(_fileNotificationReceiver->_notificationCount, 0U);
    ASSERT_EQ(_fileNotificationReceiver->_deletionCount, 0U);
    ASSERT_EQ(_fileNotificationReceiver->_additionCount, 0U);
    ASSERT_EQ(_fileNotificationReceiver->_renamingCount, 0U);
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
        false,
        16384);
    ASSERT_TRUE(myWatcher->IsWatching());
    ASSERT_EQ(_fileNotificationReceiver->_notificationCount, 0U);
    ASSERT_EQ(_fileNotificationReceiver->_deletionCount, 0U);
    ASSERT_EQ(_fileNotificationReceiver->_additionCount, 0U);
    ASSERT_EQ(_fileNotificationReceiver->_renamingCount, 0U);
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
        false,
        16384);
    ASSERT_TRUE(myWatcher->IsWatching());

    std::ofstream myfile(_testFilePath);
    myfile << "Writing this to a file.\n";

    ::Sleep(1);

    ASSERT_FALSE(_fileNotificationReceiver->_lastNotificationFile == _testFilePath);
    ASSERT_EQ(_fileNotificationReceiver->_notificationCount, 0U );
    ASSERT_EQ(_fileNotificationReceiver->_deletionCount, 0U);
    ASSERT_EQ(_fileNotificationReceiver->_additionCount, 0U);
    ASSERT_EQ(_fileNotificationReceiver->_renamingCount, 0U);

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
        false,
        16384);
    ASSERT_TRUE(myWatcher->IsWatching());

    std::ofstream myfile(_testFilePath);
    for (auto i = 0; i < 100; ++i)
    {
        myfile << "Writing this to a file.\n";
    }
    myfile.flush();

    myfile.close();

    ::Sleep(1);

    ASSERT_TRUE(_fileNotificationReceiver->_lastNotificationFile == _testFilePath);
    ASSERT_GE(_fileNotificationReceiver->_notificationCount, 1U);
    ASSERT_EQ(_fileNotificationReceiver->_deletionCount, 0U);
    ASSERT_EQ(_fileNotificationReceiver->_additionCount, 0U);
    ASSERT_EQ(_fileNotificationReceiver->_renamingCount, 0U);

    ::delete myWatcher;
}

TEST_F(NativeFileSystemWatcherTestFixture, FileModificationTestFileDeletion)
{
    _fileNotificationReceiver = new FileNotificationReceiver();

    Windows::File::NativeFileSystemWatcher * myWatcher = new Windows::File::NativeFileSystemWatcher(_cwd,
        (FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_ATTRIBUTES),
        true,
        _fileNotificationReceiver,
        L"*.txt",
        L"",
        false,
        16384);
    ASSERT_TRUE(myWatcher->IsWatching());

    Windows::Common::Base::FileDelete(_testFilePath.c_str());

    ::Sleep(1);

    ASSERT_TRUE(_fileNotificationReceiver->_lastNotificationFile == _testFilePath);
    ASSERT_GE(_fileNotificationReceiver->_notificationCount, 1U);
    ASSERT_GE(_fileNotificationReceiver->_deletionCount, 1U);
    ASSERT_EQ(_fileNotificationReceiver->_additionCount, 0U);
    ASSERT_EQ(_fileNotificationReceiver->_renamingCount, 0U);

    ::delete myWatcher;
}

TEST_F(NativeFileSystemWatcherTestFixture, FileModificationTestFileAddition)
{
    _fileNotificationReceiver = new FileNotificationReceiver();

    Windows::File::NativeFileSystemWatcher * myWatcher = new Windows::File::NativeFileSystemWatcher(_cwd,
        (FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_ATTRIBUTES),
        true,
        _fileNotificationReceiver,
        L"*.txt",
        L"",
        false,
        16384);
    ASSERT_TRUE(myWatcher->IsWatching());

    auto aTestDir = Windows::Utilities::File::GetDirectoryFromFilePath(_testFilePath.c_str());
    auto aTestFile (aTestDir += L"\\secondTestFile.txt");
    CreateNewFile(aTestFile);

    ::Sleep(1);

    ASSERT_TRUE(_fileNotificationReceiver->_lastNotificationFile == aTestFile);
    ASSERT_GE(_fileNotificationReceiver->_notificationCount, 1U);
    ASSERT_EQ(_fileNotificationReceiver->_deletionCount, 0U);
    ASSERT_EQ(_fileNotificationReceiver->_additionCount, 1U);
    ASSERT_EQ(_fileNotificationReceiver->_renamingCount, 0U);

    ::delete myWatcher;
}

TEST_F(NativeFileSystemWatcherTestFixture, FileModificationTestFileRenaming)
{
    _fileNotificationReceiver = new FileNotificationReceiver();

    Windows::File::NativeFileSystemWatcher * myWatcher = new Windows::File::NativeFileSystemWatcher(_cwd,
        (FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_ATTRIBUTES),
        true,
        _fileNotificationReceiver,
        L"*.txt",
        L"",
        false,
        16384);
    ASSERT_TRUE(myWatcher->IsWatching());

    auto aTestDir = Windows::Utilities::File::GetDirectoryFromFilePath(_testFilePath.c_str());
    auto aTestFile(aTestDir += L"\\secondTestFile.txt");
    Windows::Common::Base::FileRename(_testFilePath.c_str(), aTestFile.c_str());

    ::Sleep(1);

    ASSERT_TRUE(_fileNotificationReceiver->_lastNotificationFile == aTestFile);
    ASSERT_TRUE(_fileNotificationReceiver->_oldFileName == _testFilePath);
    ASSERT_GE(_fileNotificationReceiver->_notificationCount, 2U);
    ASSERT_EQ(_fileNotificationReceiver->_deletionCount, 0U);
    ASSERT_EQ(_fileNotificationReceiver->_additionCount, 0U);
    ASSERT_EQ(_fileNotificationReceiver->_renamingCount, 1U);

    ::delete myWatcher;
}

TEST_F(NativeFileSystemWatcherTestFixture, FileModificationErrorCountNoRestarting)
{
    _fileNotificationReceiver = new FileNotificationReceiver();

    Windows::File::NativeFileSystemWatcher * myWatcher = new Windows::File::NativeFileSystemWatcher(_cwd,
        (FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_ATTRIBUTES),
        true,
        _fileNotificationReceiver,
        L"*.txt",
        L"",
        false,
        16384);
    ASSERT_TRUE(myWatcher->IsWatching());

    auto aTestDir = Windows::Utilities::File::GetDirectoryFromFilePath(_testFilePath.c_str());
    Windows::Common::Base::DeleteDirRecursive(aTestDir.c_str(), true, true);

    ::Sleep(1);

    ASSERT_TRUE(_fileNotificationReceiver->_lastNotificationFile == _testFilePath);
    ASSERT_TRUE(_fileNotificationReceiver->_oldFileName == L"");
    ASSERT_GE(_fileNotificationReceiver->_notificationCount, 0U);
    ASSERT_GE(_fileNotificationReceiver->_deletionCount, 1U);
    ASSERT_EQ(_fileNotificationReceiver->_additionCount, 0U);
    ASSERT_EQ(_fileNotificationReceiver->_renamingCount, 0U);
    ::delete myWatcher;
}

TEST_F(NativeFileSystemWatcherTestFixture, FileModificationErrorCountRestarting)
{
    _fileNotificationReceiver = new FileNotificationReceiver();

    Windows::File::NativeFileSystemWatcher * myWatcher = new Windows::File::NativeFileSystemWatcher(_cwd,
        (FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_ATTRIBUTES),
        true,
        _fileNotificationReceiver,
        L"*.txt",
        L"",
        true,
        16384);
    ASSERT_TRUE(myWatcher->IsWatching());

    auto aTestDir = Windows::Utilities::File::GetDirectoryFromFilePath(_testFilePath.c_str());
    Windows::Common::Base::DeleteDirRecursive(aTestDir.c_str(), true, true);

    ::Sleep(1);

    ASSERT_TRUE(_fileNotificationReceiver->_lastNotificationFile == _testFilePath);
    ASSERT_TRUE(_fileNotificationReceiver->_oldFileName == L"");
    ASSERT_GE(_fileNotificationReceiver->_notificationCount, 0U);
    ASSERT_GE(_fileNotificationReceiver->_deletionCount, 1U);
    ASSERT_EQ(_fileNotificationReceiver->_additionCount, 0U);
    ASSERT_EQ(_fileNotificationReceiver->_renamingCount, 0U);
    ::delete myWatcher;
}

TEST_F(NativeFileSystemWatcherTestFixture, FileModificationTestInclusionFilterFailedThenChanged)
{
    for (unsigned j = 0; j < 10; ++j)
    {
        _fileNotificationReceiver = new FileNotificationReceiver();

        Windows::File::NativeFileSystemWatcher * myWatcher = new Windows::File::NativeFileSystemWatcher(_cwd,
            (FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_ATTRIBUTES),
            true,
            _fileNotificationReceiver,
            L"*.atm",
            L"",
            true,
            16384);
        ASSERT_TRUE(myWatcher->IsWatching());

        for (unsigned int i = 0; i < 100; ++i)
        {
            if (i % 2 == 0)
            {
                myWatcher->SetIncludeFilter(L"*.txt");
            }
            else
            {
                myWatcher->SetIncludeFilter(L"*.atm");
            }
            _fileNotificationReceiver->_additionCount = 0U;
            _fileNotificationReceiver->_deletionCount = 0U;
            _fileNotificationReceiver->_errorCount = 0U;
            _fileNotificationReceiver->_lastNotificationFile = L"";
            _fileNotificationReceiver->_notificationCount = 0U;
            ::Sleep(10);

            std::ofstream myfile(_testFilePath);
            myfile << "Writing this to a file.\n";
            myfile << "Writing this to a file.\n";
            myfile << "Writing this to a file.\n";

            ::Sleep(10);

            myfile.close();
            if (i % 2 != 0)
            {
                ASSERT_TRUE(_fileNotificationReceiver->_lastNotificationFile == L"");
                ASSERT_EQ(_fileNotificationReceiver->_notificationCount, 0U);
            }
            else
            {
                ASSERT_TRUE(_fileNotificationReceiver->_lastNotificationFile == _testFilePath);
                ASSERT_GE(_fileNotificationReceiver->_notificationCount, 1U);
            }
            ASSERT_EQ(_fileNotificationReceiver->_errorCount, 0U);
        }
        ::delete myWatcher;
    }
}

int main(int argc, wchar_t ** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}