#pragma once


class CPrivilegeEnabler
//
//	Enables privileges for this process
//	first time CDirectoryChangeWatcher::WatchDirectory() is called.
//
//	It's a singleton.
//
{
private:
	CPrivilegeEnabler();//ctor
public:
	~CPrivilegeEnabler(){};	
	static CPrivilegeEnabler & Instance();
};