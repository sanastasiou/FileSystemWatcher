﻿using System;
using System.Reflection;
using Windows.Clr;


namespace TestFileSystemWatcher
{
    using NUnit.Framework;

    [TestFixture]
    public class FileSystemWatcherTests
    {
        string _testFile = string.Empty;

        private byte[] GetBytes(string str)
        {
            byte[] bytes = new byte[str.Length * sizeof(char)];
            Buffer.BlockCopy(str.ToCharArray(), 0, bytes, 0, bytes.Length);
            return bytes;
        }

        [SetUp]
        public void Init()
        {
            string aBinDir = System.IO.Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            //empty folder from .txt files and then just create the original file
            _testFile = aBinDir + "\\" + Properties.Resources.OriginalFileName;

            System.IO.DirectoryInfo downloadedMessageInfo = new System.IO.DirectoryInfo(aBinDir);

            foreach (System.IO.FileInfo file in downloadedMessageInfo.GetFiles())
            {
                if (file.Extension == ".txt")
                {
                    file.Delete();
                }
            }
            using (var aOriginalFile = System.IO.File.CreateText(_testFile))
            {
                aOriginalFile.Write(Properties.Resources.OriginalFileContent);
            }
        }

        [Test]
        public void SmokeTest()
        {
            FileWatcher myWatcher = new FileWatcher(@"C:\FooDirectoryLALALALALALALAWLWWLWLWL",
                                                 (uint)(System.IO.NotifyFilters.FileName | System.IO.NotifyFilters.LastWrite | System.IO.NotifyFilters.CreationTime | System.IO.NotifyFilters.Size | System.IO.NotifyFilters.LastAccess | System.IO.NotifyFilters.Attributes),
                                                 true,
                                                 @"*.atm",
                                                 string.Empty,
                                                 false,
                                                 FileWatcherBase.STANDARD_BUFFER_SIZE);

            myWatcher.Dispose();
        }

        [Test]
        public void EnsureNonExistantDirIsNotWatched()
        {
            FileWatcher myWatcher = new FileWatcher(@"",
                                                 (uint)(System.IO.NotifyFilters.FileName | System.IO.NotifyFilters.LastWrite | System.IO.NotifyFilters.CreationTime | System.IO.NotifyFilters.Size | System.IO.NotifyFilters.LastAccess | System.IO.NotifyFilters.Attributes),
                                                 true,
                                                 @"*.atm",
                                                 string.Empty,
                                                 false,
                                                 FileWatcherBase.STANDARD_BUFFER_SIZE);

            Assert.IsFalse(myWatcher.IsWatching());
            myWatcher.Dispose();
        }

        [Test]
        public void EnsureExistantDirIsWatched()
        {
            string aBinDir = System.IO.Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            FileWatcher myWatcher = new FileWatcher(aBinDir,
                                                 (uint)(System.IO.NotifyFilters.FileName | System.IO.NotifyFilters.LastWrite | System.IO.NotifyFilters.CreationTime | System.IO.NotifyFilters.Size | System.IO.NotifyFilters.LastAccess | System.IO.NotifyFilters.Attributes),
                                                 true,
                                                 @"*.atm",
                                                 string.Empty,
                                                 false,
                                                 FileWatcherBase.STANDARD_BUFFER_SIZE);

            Assert.IsTrue(myWatcher.IsWatching());
            myWatcher.Dispose();
        }

        [Test]
        public void FileModificationTest()
        {
            FileWatcher myWatcher = new FileWatcher(System.IO.Path.GetDirectoryName(_testFile),
                                                    (uint)(System.IO.NotifyFilters.FileName | System.IO.NotifyFilters.LastWrite | System.IO.NotifyFilters.CreationTime | System.IO.NotifyFilters.Size | System.IO.NotifyFilters.LastAccess | System.IO.NotifyFilters.Attributes),
                                                    true,
                                                    @"*.atm",
                                                    string.Empty,
                                                    false,
                                                    FileWatcherBase.STANDARD_BUFFER_SIZE);
            try
            {
                bool notificationFired = false;
                int count = 0;
                EventHandler<System.IO.FileSystemEventArgs> handler = (s, e) => 
                {
                    notificationFired = true;
                    ++count;
                };
                myWatcher.Changed += handler;

                for (int i = 0; i < 5; ++i)
                {

                    //modify file
                    using (var aFile = System.IO.File.OpenWrite(_testFile))
                    {
                        var aBytesToWrite = GetBytes("Some extra stuff");
                        aFile.Write(aBytesToWrite, 0, aBytesToWrite.Length);
                    }

                    System.Threading.Thread.Sleep(1000);
                }
        
                Assert.True(notificationFired);
                Assert.True(count == 5);
                myWatcher.Changed -= handler;
                myWatcher.Dispose();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }
    }
}
