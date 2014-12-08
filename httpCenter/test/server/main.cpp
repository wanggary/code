/*******************************************************************************
main.cpp:
	Copyright (c) Eisoo Software Inc.(2014), All rights reserved

Purpose:
	Implementation file for the sqlengined

Author:
	liu.hao@eisoo.com

Creating Time:
	2014-11-19
*******************************************************************************/

#include <abprec.h>
#include <Modules/httpCenter/httpServer/ncHttpServer.h>

////////////////////////////////////////////////////////////////////////////////
// main
//

void initLibs ()
{
	// 初始化应用程序环境
	static AppContext appCtx (AB_APPLICATION_NAME);
	AppContext::setInstance (&appCtx);
	LibManager::getInstance ()->initLibs (AppSettings::getCFLAppSettings (), &appCtx, 0);
} 

class testHttpSvr
{
public:
	testHttpSvr(unsigned int port)
		:_server(port)
	{
	}
	
	void callback1 (evhttp_request* req)
	{
		evhttp_remove_header (req->output_headers, "Content-Tpye");
		evhttp_add_header (req->output_headers, "Content-Type", "text/html; charset=UTF-8");
		evhttp_send_reply (req, 1, "NoReason", 0);
	}
	
	void callback2 (evhttp_request* req)
	{
		evhttp_send_reply (req, 3, "NoReason", 0);
	}
	
	void start()
	{
		_server.RegisterCallbacks ("/xxx", this, &testHttpSvr::callback1);
		_server.RegisterCallbacks ("", this, &testHttpSvr::callback1);
		_server.start();
	}
	
protected:
	ncHttpServer<testHttpSvr>		_server;
};


int main(int argc,  char** argv)
{
	//init_daemon();//初始化为Daemon
	//initLibs();
	
	// 测试httpServer
	try {
		testHttpSvr svr (8192);
		svr.start();
		
	}
	catch (Exception& e) {
		printf("%s\n", e.toString().getCStr());
	}
	
	
	return 0;
};
