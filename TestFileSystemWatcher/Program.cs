using System;
using FileSystemWactherCLRWrapper;

namespace TestFileSystemWatcher
{
    using NUnit.Framework;

    [TestFixture]
    public class FileSystemWatcherTests
    {
        [Test]
        public void SmokeTest()
        {
            FileSystemWatcher myWatcher = new FileSystemWatcher(@"C:\Work\body_trunk",
                                                                false,
                                                                @"*.atm",
                                                                String.Empty,
                                                                (uint)(System.IO.NotifyFilters.FileName | System.IO.NotifyFilters.LastWrite | System.IO.NotifyFilters.CreationTime | System.IO.NotifyFilters.Size | System.IO.NotifyFilters.LastAccess | System.IO.NotifyFilters.Attributes),
                                                                true);
            try
            {
                myWatcher.Dispose();
            }
            catch(Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }
    }
}
