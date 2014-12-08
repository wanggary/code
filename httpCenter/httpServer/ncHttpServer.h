/*******************************************************************************
ncHttpServer.h:
	Copyright (c) Eisoo Software Inc.(2014), All rights reserved

Purpose:
	Implementation file for the ncHttpServer

Author:
	wang.shenjian@eisoo.com

Creating Time:
	2014-11-26
*******************************************************************************/

#ifndef __NC_HTTPSERVER_H__
#define __NC_HTTPSERVER_H__

#include "ncHttpServerUtil.h"


////////////////////////////////////////////////////////////////////////////////
// ncHttpServer
//

template<class T>
class ncHttpServer
{
public:
	ncHttpServer(unsigned int port);
	~ncHttpServer();
	
	void RegisterCallbacks (const char* uri, T* pcallback, void (T::*pCallbackfun) (evhttp_request*));
	void start (void);
	void stop (void);

private:
	void init ();
	
private:
	ThreadMutexLock							_lock;
	unsigned int								_serverPort;
	map<string, ncCallbackObj<T>* >		_callbacks;
	
	evutil_socket_t							_fd;
	evhttp* 									_httpServer;
	event_base*								_eventBase;
	evconnlistener*							_listener;
};

#include "ncHttpServer.inl"

#endif // __NC_HTTPSERVER_H__
