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

#include <event.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/thread.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/keyvalq_struct.h>
#include <event2/http_compat.h>
#include <event2/event_compat.h>
#include <abprec.h>
#include <syswrap/syswrap.h>

////////////////////////////////////////////////////////////////////////////////
// ncHttpServer
//

class ncHttpServer
{
public:
	ncHttpServer(unsigned int port);
	~ncHttpServer();
	virtual void start (void);
	virtual void stop (void);
	virtual void registerCallbacks (evhttp* httpServer) = 0;
	void execute (void);

public:
	unsigned int						_serverPort;
	evhttp*								_httpServer;
	event_base*							_eventBase;
	evutil_socket_t						_fd;
	const int							_networkSize;
	const int64							_maxHeadersSize;
	const int64							_maxBodySize;
};

#endif // __NC_HTTPSERVER_H__
