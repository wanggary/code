/*******************************************************************************
ncHttpServer.h:
	Copyright (c) Eisoo Software Inc.(2014), All rights reserved

Purpose:
	ncHttpServer demo

Author:
	wang.shenjian@eisoo.com

Creating Time:
	2014-11-28
*******************************************************************************/

#include "../server/ncHttpServer.h"
#include <abprec.h>
#include <syswrap/syswrap.h>

/**
 *	callbackHandler
 */
void callbackHandler (struct evhttp_request *req, void *argv)
{
	printf ("have callbacked!\n");
	
	struct evbuffer *buf;
	buf = evbuffer_new();
 
	// 输出
	const char * str = evhttp_request_get_host(req);
	const char *decode_uri =  evhttp_request_get_uri(req) ;
	// URL解码
	char *url = evhttp_decode_uri(decode_uri);
	printf( "[%s][%s]\n" , str , decode_uri);
   
   
	evbuffer_add_printf(buf, "hello http\n");
	evhttp_send_reply(req, HTTP_OK, "OK", buf);
 
	// 内存释放
	evbuffer_free(buf);
}

/**
 * httpserver子类
 */

class httpServerDemo : public ncHttpServer
{
public:
	httpServerDemo (unsigned int port);
	//~httpServerDemo ();

	void registerCallbacks (evhttp* httpServer);
};

//public
httpServerDemo::httpServerDemo (unsigned int port)
	: ncHttpServer (port)
{
}

void
httpServerDemo::registerCallbacks (evhttp* httpServer)
{
	printf ("have registered!\n");
	if (httpServer == NULL) {
		printf ("httpServer is NULL!\n");
	}
	evhttp_set_cb (httpServer, "/hellohttp", callbackHandler, NULL);
	evhttp_set_gencb(httpServer, callbackHandler, NULL);
}

/**
 *main
 */

void initLibs ()
{
	// 初始化应用程序环境
	static AppContext appCtx (AB_APPLICATION_NAME);
	AppContext::setInstance (&appCtx);
	LibManager::getInstance ()->initLibs (AppSettings::getCFLAppSettings (), &appCtx, 0);
}

int main (int argc, char* argv[])
{
	initLibs ();
	
	unsigned int port = 8801;
	try {
			//httpServerDemo *server = new httpServerDemo (port);
			httpServerDemo server (port);
			server.start ();
			
			//struct event_base *base = event_base_new();
    		//struct evhttp *httpd = evhttp_new(base);
    		//evhttp_bind_socket(httpd,"0.0.0.0",19800);
    		//evhttp_set_gencb(httpd, callbackHandler, NULL);
    		//event_base_dispatch(base); 
	}
	
	catch (Exception& e) {
		printf("%s\n", e.toString().getCStr());
	}
	
	catch (...) {
		printf ("unknown error!\n");
	}
	
	return 0;
}
