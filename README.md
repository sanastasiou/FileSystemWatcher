FileSystemWatcher
=================

A CLI wrapper for a C FileSystemWrapper to use in windows.

The "default" FileSystemWatcher implementation in .NET has problems:

https://connect.microsoft.com/VisualStudio/feedback/details/687372/unhandled-exception-in-filesystemwatcher-suspect-caused-by-filename-in-japanese-locale

https://connect.microsoft.com/VisualStudio/feedback/details/832560/filesystemwatcher-listener-raising-an-unhandled-exception

https://social.msdn.microsoft.com/Forums/vstudio/en-US/4465cafb-f4ed-434f-89d8-c85ced6ffaa8/filesystemwatcher-reliability

https://connect.microsoft.com/VisualStudio/feedback/details/780775/filesystemwatcher-overlapped-listener-raising-an-exception-on-malformed-notifications

And probably others. I noticed the bugs during the implementation of a plugin. My plugin was crashing when a lot of files were removed/added.

This project , relies on a pure C/C++ implementation of a file system watcher and is more robust. On top of it is a CLI wrapper which exposes the same events as the original class. So one could easily use this wrapper instead of the standard .net class.
