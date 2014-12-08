/*******************************************************************************
ncHttpServer.cpp:
	Copyright (c) Eisoo Software Inc.(2014), All rights reserved

Purpose:
	Implementation file for the ncHttpServer

Author:
	wang.shenjian@eisoo.com

Creating Time:
	2014-11-26
*******************************************************************************/

#include "ncHttpServer.h"

// public static
evutil_socket_t bindPort (unsigned short port)
{
	sockaddr_in in;
	memset (&in, 0, sizeof (in));		//所有成员初始化为0
	in.sin_family = AF_INET;
	in.sin_port = htons (port);		//将无符号短整型数值转换成网络字节顺序(低位存高位)
	in.sin_addr.s_addr = htonl (INADDR_ANY);

	evutil_socket_t fd = socket (AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
		return -1;

	if (evutil_make_socket_nonblocking (fd) < 0) {
		evutil_closesocket (fd);
		return -1;
	}

	int on = 1;
	if (setsockopt (fd, SOL_SOCKET, SO_KEEPALIVE, (const char *)&on, sizeof (on)) < 0) {
		evutil_closesocket (fd);
		return -1;
	}

	if (evutil_make_listen_socket_reuseable (fd) < 0) {
		evutil_closesocket (fd);
		return -1;
	}

	if (::bind (fd, (sockaddr*)&in, sizeof (sockaddr_in)) < 0) {
		evutil_closesocket (fd);
		return -1;
	}

	listen (fd, 128);
	return fd;
}

////////////////////////////////////////////////////////////////////////////////
// ncHttpServer
//

// public
ncHttpServer::ncHttpServer(unsigned int port)
	: _serverPort (port)
	, _httpServer (0)
	, _eventBase (0)
	, _fd (-1)
	, _networkSize (1)
	, _maxHeadersSize (0)
	, _maxBodySize (0)
{
	evthread_use_pthreads ();
}

// public
ncHttpServer::~ncHttpServer()
{
	stop ();
}


void 
ncHttpServer::start (void)
{

	try {
		_fd = bindPort (_serverPort);
		if (_fd == -1) {
			printf ("bind port error!\n");
			return;
		}

		for (size_t i = 0; i < _networkSize; ++i) {
			execute();
		}
	}
	catch (Exception& e) {
		stop ();
		throw;
	}
}

void 
ncHttpServer::stop (void)
{
	try {
			if (_eventBase) {
				event_base_loopexit (_eventBase, 0);
			}
			
			if (_httpServer) {
				evhttp_free (_httpServer);
				_httpServer = 0;
			}
			
			if (_eventBase) {
				event_base_free (_eventBase);
				_eventBase = 0;
			}
			
			if (_fd != -1) {
				evutil_closesocket(_fd);
				_fd = -1;
			}

	}
	catch (Exception& e) {
		throw e;
	}
	catch (...) {
		throw;
	}
}

void
ncHttpServer::execute (void)
{
	evutil_socket_t	_evsock = _fd;
	//ncHttpServer* _server = this;
	const int64 maxHeadersSize = 4194304;
	const int64 maxBodySize = 6291456;
	
	try {
		//event_config* cfg = event_config_new ();
		//event_base* _eventBase = event_base_new_with_config (cfg);
		event_base* _eventBase = event_base_new ();

		const int flags = LEV_OPT_REUSEABLE | LEV_OPT_THREADSAFE;
		evconnlistener* listener = evconnlistener_new (_eventBase, 0, 0, flags, 0, _evsock);
		if(!listener) {
			throw;
		}

		evhttp* _httpServer = evhttp_new (_eventBase);
		evhttp_set_max_headers_size (_httpServer, maxHeadersSize);
		evhttp_set_max_body_size (_httpServer, maxBodySize);
		//evhttp_set_allowed_methods (_httpServer, EVHTTP_REQ_POST|EVHTTP_REQ_POST);

		evhttp_bound_socket* boundSocket = evhttp_bind_listener (_httpServer, listener);
		if (!boundSocket) {
			throw;
		}

		// 注册回调函数
		this->registerCallbacks (_httpServer);

		evconnlistener_enable (listener);

		printMessage2 (_T("http server start OK!"));

		event_base_dispatch (_eventBase);
	}
	catch (Exception& e) {
		throw e;
	}
}
