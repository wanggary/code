/*******************************************************************************
ncHttpServer.inl:
	Copyright (c) Eisoo Software Inc.(2014), All rights reserved

Purpose:
	Implementation file for the ncHttpServer

Author:
	wang.shenjian@eisoo.com

Creating Time:
	2014-11-26
*******************************************************************************/

#ifndef __NC_HTTPSERVER_INL__
#define __NC_HTTPSERVER_INL__

////////////////////////////////////////////////////////////////////////////////
// ncHttpServer
//

// public
template<class T>
ncHttpServer<T>::ncHttpServer(unsigned int port)
	:_lock ()
	,_serverPort (port)
	,_httpServer (NULL)
{
	init ();
}

// public
template<class T>
ncHttpServer<T>::~ncHttpServer()
{
	stop();
}

template<class T>
void 
ncHttpServer<T>::RegisterCallbacks(const char* uri, T* pcallback, void (T::*pCallbackfun) (evhttp_request*))
{
	AutoLock<ThreadMutexLock> lock (&_lock);
	
	typename map<string, ncCallbackObj<T>* >::const_iterator iter = _callbacks.find (uri);
	ncCallbackObj<T>* pCallbackObj = NULL;
	if (iter != _callbacks.end ()) {
		pCallbackObj = iter->second;
	}
	else {
		pCallbackObj = new ncCallbackObj<T>;
		pCallbackObj->_pCallback = pcallback;
		pCallbackObj->_pCallbackfun = pCallbackfun;
		_callbacks.insert (make_pair (uri, pCallbackObj));
	}
	if (strlen (uri)) {
		evhttp_set_cb (_httpServer, uri, callbackByEvent<T>, pCallbackObj);
	}
	else {
		evhttp_set_gencb (_httpServer, callbackByEvent<T>, pCallbackObj);
	}
}

template<class T>
void 
ncHttpServer<T>::start (void)
{
	AutoLock<ThreadMutexLock> lock (&_lock);
	
	// 暂时允许监听新连接
	evconnlistener_enable (_listener);

	// event_base_dispatch（）等同于没有设置标志的event_base_loop（）。
	// 所以，event_base_dispatch（）将一直运行，直到没有已经注册的事件了，
	// 或者调用了event_base_loopbreak（）或者event_base_loopexit（）为止。
	event_base_dispatch (_eventBase);
}

template<class T>
void 
ncHttpServer<T>::stop (void)
{
	// 释放资源
	try {
		if (_eventBase) {
			event_base_loopexit(_eventBase, 0);
		}
		
		for (typename map<string, ncCallbackObj<T>* >::iterator iter = _callbacks.begin(); iter != _callbacks.end(); ++iter) {
			if (iter->second) {
				delete iter->second;
				iter->second = 0;
			}
		}
		
		if (_httpServer) {
			evhttp_free(_httpServer);
			_httpServer = 0;
		}
		
		if (_listener) {
			evconnlistener_free(_listener);
			_listener = 0;
		}
		
		if (_eventBase) {
			event_base_free(_eventBase);
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

template<class T>
void 
ncHttpServer<T>::init ()
{
	evthread_use_pthreads ();
	
	AutoLock<ThreadMutexLock> lock (&_lock);
	
	try {
		_fd = ncHttpServerUtil::bindPort (_serverPort);
	
		const int64 maxHeadersSize = 4194304;
		const int64 maxBodySize = 6291456;
	
		// 分配一个event_config
		event_config* cfg = event_config_new ();
		// 获取新的event_base
		_eventBase = event_base_new_with_config (cfg);
		/**
		LEV_OPT_REUSEABLE ：某些平台在默认情况下，关闭某监听套接字后，要过一会儿其他套接字才可以绑定到同一个端口。
							设置这个标志会让libevent标记套接字是可重用的，
							这样一旦关闭，可以立即打开其他套接字，在相同端口进行监听
		LEV_OPT_THREADSAFE : 为监听器分配锁，这样就可以在多个线程中安全地使用了。这是2.0.8-rc的新功能
		*/
		const int flags = LEV_OPT_REUSEABLE | LEV_OPT_THREADSAFE;
		
		// 分配和返回一个新的连接监听器对象
		// 连接监听器使用event_base来得知什么时候在给定的监听套接字上有新的TCP连接。新连接到达时，监听器调用你给出的回调函数。
		_listener = evconnlistener_new (_eventBase, 0, 0, flags, 0, _fd);
		if(!_listener) {
			throw;
		}
		
		_httpServer = evhttp_new (_eventBase);
		evhttp_set_max_headers_size (_httpServer, maxHeadersSize);
		evhttp_set_max_body_size (_httpServer, maxBodySize);
		evhttp_set_allowed_methods (_httpServer, EVHTTP_REQ_POST|EVHTTP_REQ_GET);

		evhttp_bound_socket* boundSocket = evhttp_bind_listener (_httpServer, _listener);
		if (!boundSocket) {
			throw;
		}

	}
	catch (Exception& e) {
		throw e;
	}
}

#endif //__NC_HTTPSERVER_INL__
