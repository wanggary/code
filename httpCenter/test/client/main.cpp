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
#include <Modules/httpCenter/httpClient/ncHttpClient.h>
#include <Modules/cloudCommon/ncJson.h>

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

int main(int argc,  char** argv)
{
	//init_daemon();//初始化为Daemon
	//initLibs();
	
	// 测试httpclient
	try {
		{
			ncHttpClient client;
			string url = "127.0.0.1:8192/xxx";
			string content = "";
			string contentType = "";
			int timeOut = 60;
			ncHttpResponse resp;
		
			client.Post (url, content, contentType, timeOut, resp);
			printf("xxx1:\n %d\n %s\n", resp.code, resp.body.c_str());
		}
		{
			ncHttpClient client;
			string url = "127.0.0.1:8192/ping";
			string content = "";
			string contentType = "";
			int timeOut = 60;
			ncHttpResponse resp;
		
			client.Post (url, content, contentType, timeOut, resp);
			printf("xxx2:\n %d\n %s\n", resp.code, resp.body.c_str());
		}
		{
			ncHttpClient client;
			string url = "127.0.0.1:8192/dboperator?method=createlogdatabase&userid=liuhao&tokenid=123";
			string content = "";
			string contentType = "";
			int timeOut = 60;
			
			// 构造http内容
			JSON::Object json;
			json["docid"] = "327107091";
			json["account"] = "liuhao";
			json["password"] = "123456";
			JSON::Writer::write (json, content);
			
			// 发送请求
			ncHttpResponse resp;
			client.Post (url, content, contentType, timeOut, resp);
			
			// 解析返回的结果
			JSON::Value value;
			JSON::Reader::read (value, resp.body.c_str (), resp.body.length ());
			String xx1 = value["result1"].s().c_str ();
			String xx2 = value["result2"].s().c_str();
			bool xx3 = value["result3"].b();
			
			
			printf("xxx3:\n %d\n %s\n", resp.code, resp.body.c_str());
		}
	}
	catch (Exception& e) {
		printf("\nException:%s\n", e.toString().getCStr());
	}
	
	
	return 0;
};
