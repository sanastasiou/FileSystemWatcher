// ***********************************************************************
// Copyright (c) 2009 Charlie Poole
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// ***********************************************************************

using NUnit.Framework.Internal;
using NUnitLite.Runner;
using System;
using System.Reflection;

namespace TestClrFileSystemWatcher
{
    using NUnit.Framework;
    using System.Threading;

    public class Program
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

#if !DEBUG_TEST
        [Test]
        public void SmokeTest()
        {
            try
            {
                var myWatcher = new Windows.Clr.FileWatcher(@"C:\FooDirectoryLALALALALALALAWLWWLWLWL",
                                                            (uint)(System.IO.NotifyFilters.FileName | System.IO.NotifyFilters.LastWrite | System.IO.NotifyFilters.CreationTime | System.IO.NotifyFilters.Size | System.IO.NotifyFilters.LastAccess | System.IO.NotifyFilters.Attributes),
                                                            true,
                                                            @"*.atm",
                                                            string.Empty,
                                                            false,
                                                            Windows.Clr.FileWatcherBase.STANDARD_BUFFER_SIZE);
            
                myWatcher.Dispose();
            }
            catch
            {
                Console.WriteLine("Exception..");
            }
        }

        [Test]
        public void EnsureNonExistantDirIsNotWatched()
        {
            Windows.Clr.FileWatcher myWatcher = new Windows.Clr.FileWatcher(@"",
                                                                            (uint)(System.IO.NotifyFilters.FileName | System.IO.NotifyFilters.LastWrite | System.IO.NotifyFilters.CreationTime | System.IO.NotifyFilters.Size | System.IO.NotifyFilters.LastAccess | System.IO.NotifyFilters.Attributes),
                                                                            true,
                                                                            @"*.atm",
                                                                            string.Empty,
                                                                            false,
                                                                            Windows.Clr.FileWatcherBase.STANDARD_BUFFER_SIZE);

            Assert.IsFalse(myWatcher.IsWatching());
            myWatcher.Dispose();
        }

        [Test]
        public void EnsureExistantDirIsWatched()
        {
            string aBinDir = System.IO.Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            Windows.Clr.FileWatcher myWatcher = new Windows.Clr.FileWatcher(aBinDir,
                                                                            (uint)(System.IO.NotifyFilters.FileName | System.IO.NotifyFilters.LastWrite | System.IO.NotifyFilters.CreationTime | System.IO.NotifyFilters.Size | System.IO.NotifyFilters.LastAccess | System.IO.NotifyFilters.Attributes),
                                                                            true,
                                                                            @"*.atm",
                                                                            string.Empty,
                                                                            false,
                                                                            Windows.Clr.FileWatcherBase.STANDARD_BUFFER_SIZE);

            Assert.IsTrue(myWatcher.IsWatching());
            myWatcher.Dispose();
        }

        [Test]
        public void FileModificationTest()
        {
            Windows.Clr.FileWatcher myWatcher = new Windows.Clr.FileWatcher(System.IO.Path.GetDirectoryName(_testFile),
                                                                            (uint)(System.IO.NotifyFilters.FileName | System.IO.NotifyFilters.LastWrite | System.IO.NotifyFilters.CreationTime | System.IO.NotifyFilters.Size | System.IO.NotifyFilters.LastAccess | System.IO.NotifyFilters.Attributes),
                                                                            true,
                                                                            @"*.txt",
                                                                            string.Empty,
                                                                            false,
                                                                            Windows.Clr.FileWatcherBase.STANDARD_BUFFER_SIZE);
            myWatcher.Renamed += myWatcher_Renamed;
            try
            {
                bool notificationFired = false;
                int count = 0;
                EventHandler<System.IO.FileSystemEventArgs> handler = (s, e) =>
                {
                    notificationFired = true;
                    ++count;
                    Console.WriteLine(String.Format("Full path : {0}, name : {1}, change : {2}", e.FullPath, e.Name, e.ChangeType));
                };
                myWatcher.Changed += handler;

                //modify file
                using (var aFile = System.IO.File.OpenWrite(_testFile))
                {
                    var aBytesToWrite = GetBytes("Some extra stuff");
                    aFile.Write(aBytesToWrite, 0, aBytesToWrite.Length);
                }

                System.Threading.Thread.Sleep(1000);

                Assert.True(notificationFired);
                Assert.AreEqual(count, 1);
                myWatcher.Changed -= handler;
                myWatcher.Dispose();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        [Test]
        public void EnduranceTestCreationDeletion()
        {
            for (uint j = 0; j < 5; ++j)
            {
                try
                {
                    Windows.Clr.FileWatcher myWatcher = new Windows.Clr.FileWatcher(System.IO.Path.GetDirectoryName(_testFile),
                                                                                (uint)(System.IO.NotifyFilters.FileName | System.IO.NotifyFilters.LastWrite | System.IO.NotifyFilters.CreationTime | System.IO.NotifyFilters.Size | System.IO.NotifyFilters.LastAccess | System.IO.NotifyFilters.Attributes),
                                                                                true,
                                                                                @"*.txt",
                                                                                string.Empty,
                                                                                false,
                                                                                Windows.Clr.FileWatcherBase.STANDARD_BUFFER_SIZE);

                    bool notificationFired = false;
                    int count = 0;
                    EventHandler<System.IO.FileSystemEventArgs> handler = (s, e) =>
                    {
                        notificationFired = true;
                        ++count;
                    };
                    myWatcher.Changed += handler;

                    for (uint i = 0; i < 10; ++i)
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
                    Assert.AreEqual(count, 10);
                    myWatcher.Changed -= handler;
                    myWatcher.Dispose();
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.Message);
                }
            }
        }

#else
        [Test]
        public void FileModificationFromExternalSourceTest()
        {
            try
            {
                if (RunWithTimeout(LongRunningOperation, TimeSpan.FromMilliseconds(3000000)))
                {
                    Console.WriteLine("Worker thread finished.");
                }
                else
                {
                    Console.WriteLine("Worker thread was aborted.");
                }
            }
            catch (Exception)
            {
            }
        }
#endif 
        bool RunWithTimeout(ThreadStart threadStart, TimeSpan timeout)
        {
            Thread workerThread = new Thread(threadStart);

            workerThread.Start();

            bool finished = workerThread.Join(timeout);
            if (!finished)
                workerThread.Abort();

            return finished;
        }

        void LongRunningOperation()
        {
            Windows.Clr.FileWatcher myWatcher = new Windows.Clr.FileWatcher("C:\\Work\\RTextNpp\\TestFiles",
                                                                            (uint)(System.IO.NotifyFilters.FileName | System.IO.NotifyFilters.LastWrite | System.IO.NotifyFilters.CreationTime | System.IO.NotifyFilters.Size | System.IO.NotifyFilters.LastAccess | System.IO.NotifyFilters.Attributes),
                                                                            true,
                                                                            @"*.atm",
                                                                            string.Empty,
                                                                            false,
                                                                            Windows.Clr.FileWatcherBase.STANDARD_BUFFER_SIZE);
            myWatcher.Renamed += myWatcher_Renamed;
            myWatcher.Changed += myWatcher_Changed;
            myWatcher.Deleted += myWatcher_Deleted;
            Assert.True(myWatcher.IsWatching());
            while (true)
            {
                Thread.Sleep(100000);
            }

        }

        void myWatcher_Deleted(object sender, System.IO.FileSystemEventArgs e)
        {
            Console.WriteLine(String.Format("myWatcher_Deleted : {0}, changed : {1}, name : {2}", e.FullPath, e.ChangeType, e.Name));
        }

        void myWatcher_Changed(object sender, System.IO.FileSystemEventArgs e)
        {
            Console.WriteLine(String.Format("myWatcher_Changed : {0}, changed : {1}, name : {2}", e.FullPath, e.ChangeType, e.Name));
        }

        void myWatcher_Renamed(object sender, System.IO.RenamedEventArgs e)
        {
            Console.WriteLine(String.Format("myWatcher_Renamed : {0}, new full path : {1}, changed : {2}, name : {3}", e.OldFullPath, e.FullPath, e.ChangeType, e.Name));
        }

        // The main program executes the tests. Output may be routed to
        // various locations, depending on the arguments passed.
        //
        // Arguments:
        //
        //  Arguments may be names of assemblies or options prefixed with '/'
        //  or '-'. Normally, no assemblies are passed and the calling
        //  assembly (the one containing this Main) is used. The following
        //  options are accepted:
        //
        //    -test:<testname>  Provides the name of a test to be exected.
        //                      May be repeated. If this option is not used,
        //                      all tests are run.
        //
        //    -out:PATH         Path to a file to which output is written.
        //                      If omitted, Console is used, which means the
        //                      output is lost on a platform with no Console.
        //
        //    -full             Print full report of all tests.
        //
        //    -result:PATH      Path to a file to which the XML test result is written.
        //
        //    -explore[:Path]   If specified, list tests rather than executing them. If a
        //                      path is given, an XML file representing the tests is written
        //                      to that location. If not, output is written to tests.xml.
        //
        //    -noheader,noh     Suppress display of the initial message.
        //
        //    -wait             Wait for a keypress before exiting.
        //
        //    -include:categorylist 
        //             If specified, nunitlite will only run the tests with a category 
        //             that is in the comma separated list of category names. 
        //             Example usage: -include:category1,category2 this command can be used
        //             in combination with the -exclude option also note that exlude takes priority
        //             over all includes.
        //
        //    -exclude:categorylist 
        //             If specified, nunitlite will not run any of the tests with a category 
        //             that is in the comma separated list of category names. 
        //             Example usage: -exclude:category1,category2 this command can be used
        //             in combination with the -include option also note that exclude takes priority
        //             over all includes
        public static void Main(string[] args)
        {
            new TextUI().Execute(args);
        }
    }
}