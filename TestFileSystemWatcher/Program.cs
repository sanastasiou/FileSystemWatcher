using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using FileSystemWactherCLRWrapper;

namespace TestFileSystemWatcher
{
    class Program
    {
        static void Main(string[] args)
        {
            FileSystemWatcher myWatcher = new FileSystemWatcher(@"C:\Work\body_trunk",
                                                                false,
                                                                @"*.atm",
                                                                String.Empty,
                                                                (uint)(System.IO.NotifyFilters.FileName | System.IO.NotifyFilters.LastWrite | System.IO.NotifyFilters.CreationTime | System.IO.NotifyFilters.Size | System.IO.NotifyFilters.LastAccess | System.IO.NotifyFilters.Attributes),
                                                                true);

            myWatcher.Changed += myWatcher_Changed;

            char aTemp;
            while ((aTemp = Console.ReadKey().KeyChar) != 'c')
            {
                switch(aTemp)
                {
                    case 's':
                        myWatcher.StopWatching();
                        break;
                    case 'a':
                        myWatcher.RestartWatching();
                        break;
                }
            }
        }

        static void myWatcher_Changed(object sender, System.IO.FileSystemEventArgs e)
        {
            Console.WriteLine(String.Format("File {0} has been changed..", e.FullPath));
        }
    }
}
