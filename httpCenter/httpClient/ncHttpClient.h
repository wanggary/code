/*******************************************************************************
ncHttpClient.h:
	Copyright (c) Eisoo Software Inc.(2014), All rights reserved

Purpose:
	Implementation file for the ncHttpClient

Author:
	wang.shenjian@eisoo.com

Creating Time:
	2014-11-26
*******************************************************************************/

#ifndef __NC_HTTPCLIENT_H__
#define __NC_HTTPCLIENT_H__

#include <curl/curl.h>
#include "ncHttpDef.h"

////////////////////////////////////////////////////////////////////////////////
// ncHttpClient
//

class ncHttpClient
{
public:
	ncHttpClient();
	~ncHttpClient();
	
	void Post(const string& url, const string& content, const string& contentType, const int timeout, ncHttpResponse& response);
	void Get (const string& url, const int timeout, ncHttpResponse& response);
	
protected:
    static size_t OnWriteResponseBody (void *contents, size_t size, size_t nmemb, void *userdata);
    static size_t OnWriteResponseHeader (void *contents, size_t size, size_t nmemb, void *userdata);
	
private:
	ThreadMutexLock					_sendLock;
	void*								_curl;			// curl handle
};

#endif // __NC_HTTPCLIENT_H__
