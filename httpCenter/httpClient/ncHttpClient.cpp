/*******************************************************************************
ncHttpClient.cpp:
	Copyright (c) Eisoo Software Inc.(2014), All rights reserved

Purpose:
	Implementation file for the ncHttpClient

Author:
	wang.shenjian@eisoo.com

Creating Time:
	2014-11-26
*******************************************************************************/

#include <abprec.h>
#include "ncHttpClient.h"

static inline string& trim(string &s) 
{
	s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
	 s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
	return s;
}

////////////////////////////////////////////////////////////////////////////////
// ncHttpClient
//

/** curl_global_init
// 初始化libcurl: 这个函数只能用一次,。(其实在调用curl_global_cleanup 函数后仍然可再用)
// 不要在每个线程中都调用curl_global_init，应该将该函数的调用放在主线程中
// CURLcode curl_global_init(long flags);
// CURL_GLOBAL_ALL		//初始化所有的可能的调用。
// CURL_GLOBAL_SSL		//初始化支持 安全套接字层。
// CURL_GLOBAL_WIN32	//初始化win32套接字库。
// CURL_GLOBAL_NOTHING	//没有额外的初始化。
/// curl_global_init(CURL_GLOBAL_ALL);
*/

/** curl_global_cleanup
// 结束libcurl使用的时候，用来对curl_global_init做的工作清理
// 不要在每个线程中都调用curl_global_init，应该将该函数的调用放在主线程中。
///curl_global_cleanup ();
*/

// public
ncHttpClient::ncHttpClient()
	: _curl(NULL)
{
	/** curl_easy_init
	//得到 easy interface型指针
	// CURL *curl_easy_init( );
	// 一个会话的开始. 它会返回一个easy_handle(CURL*对象), 一般用在easy系列的函数中
	// 首先一个基本原则就是：绝对不应该在线程之间共享同一个libcurl handle(CURL *对象)，
	// 不管是easy handle还是multi handle, 一个线程每次只能使用一个handle。
	*/
	_curl = curl_easy_init ();
}

// public
ncHttpClient::~ncHttpClient()
{
	// void curl_easy_cleanup(CURL *handle);
	// 结束一个会话.与curl_easy_init配合着用
	curl_easy_cleanup (_curl);
}

void 
ncHttpClient::Post(const string& url, const string& content, const string& contentType, const int timeout, ncHttpResponse& response)
{
    AutoLock<ThreadMutexLock> lock (&_sendLock);
	
	/** curl_easy_setopt
    // 设置传输选项:set url
	// CURLcode curl_easy_setopt(CURL *handle, CURLoption option, parameter);
	// 告诉curl库.程序将有如何的行为
	// parameter 这个参数 既可以是个函数的指针,也可以是某个对象的指针,也可以是个long型的变量.它用什么这取决于第二个参数.
	// 各种 CURLoption 类型的选项(都在curl.h库里有定义)
	// CURLOPT_URL ：设置访问URL
	// CURLOPT_WRITEFUNCTION ：回调函数原型为：size_t function( void *ptr, size_t size, size_t nmemb, void *stream); 函数将在libcurl接收到数据后被调用
	// CURLOPT_WRITEDATA ：传递指针给libcurl，用于表明 CURLOPT_WRITEFUNCTION 函数中的stream指针的来源
	// CURLOPT_HEADERFUNCTION :回调函数原型为 size_t function( void *ptr, size_t size,size_t nmemb, void *stream); libcurl一旦接收到http 头部数据后将调用该函数
	// CURLOPT_HEADERDATA :传递指针给libcurl，该指针表明 CURLOPT_HEADERFUNCTION 函数的stream指针的来源
	// CURLOPT_READFUNCTION : 函数原型是：size_t function(void *ptr, size_t size, size_t nmemb,void *stream),libCurl需要读取数据传递给远程主机时将调用
	// CURLOPT_READDATA : 表明 CURLOPT_READFUNCTION 函数原型中的stream指针来源
	// CURLOPT_PROGRESSFUNCTION :指定的函数正常情况下每秒被libcurl调用一次，
	// CURLOPT_NOPROGRESS :为了使 CURLOPT_PROGRESSFUNCTION 被调用，CURLOPT_NOPROGRESS必须被设置为false
	// CURLOPT_PROGRESSDATA :指定的参数将作为 CURLOPT_PROGRESSFUNCTION 指定函数的第一个参数
	// CURLOPT_TIMEOUT : 设置传输时间
	// CURLOPT_CONNECTIONTIMEOUT :设置连接等待时间
	// CURLOPT_FOLLOWLOCATION :设置重定位URL
	// CURLOPT_VERBOSE : 将CURLOPT_VERBOSE属性设置为1，libcurl会输出通信过程中的一些细节。
					如果使用的是http协 议，请求头/响应头也会被输出。将CURLOPT_HEADER设为1，这些头信息将出现在消息的内容中。
	// CURLOPT_RANGE : 指定char *参数传递给libcurl，用于指明http域的RANGE头域，例如：
					 表示头500个字节：bytes=0-499
					 表示第二个500字节：bytes=500-999
					 表示最后500个字节：bytes=-500
					 表示500字节以后的范围：bytes=500-
					 第一个和最后一个字节：bytes=0-0,-1
					 同时指定几个范围：bytes=500-600,601-999
	// CURLOPT_RESUME_FROM : 传递一个long参数给libcurl，指定你希望开始传递的偏移量
	*/
    curl_easy_setopt (_curl, CURLOPT_URL, url.c_str());

    // POST http method
	// HTTP支持GET, HEAD或者POST提交请求,libcurl默认以GET方式提交请求
    curl_easy_setopt (_curl, CURLOPT_HTTPPOST, 1);

	// 设置 content-type
	struct curl_slist *headers = NULL;
    string str = "Content-Type: ";
    if (contentType.size () == 0) {
        str += "application/json";
    }
    else {
        str += contentType;
    }
	
    /** curl_slist_append
	// 消息头:我们可以通过 CURLOPT_HTTPHEADER 属性手动替换、添加或删除相应 的HTTP消息头
	// 请求消息头用于告诉服务器如何处理请求
	// 响应消息头则告诉浏览器如何处理接收到的数据
	// 当使用libcurl发送http请求时，它会自动添加一些默认http头
					Host ：http1.1（大部分http1.0)版本都要求客户端请求提供这个信息头。
					Pragma ："no-cache" 表示不要缓冲数据。
					Accept ："* /*" (第一个*后无空格)表示允许接收任何类型的数据
					Expect ： 以POST的方式提交请求时，会设置该消息头为"100-continue"，
							它要求服务器在正式处理该请求之前，返回一 个"OK"消息。如果POST的数据很小，libcurl可能不会设置该消息头。
	
	// curl_slist_append ：对于已经存在的消息头，可以重新设置它的值
	// 				headers = curl_slist_append(headers, "Accept:Agent-007"); 
					headers = curl_slist_append(headers, "Host:munged.host.line"); 
	对于一个已经存在的消息头，设置它的内容为空，libcurl在发送请求时就不会同时提交该消息头：
					headers = curl_slist_append(headers, "Accept:");
    */
    headers = curl_slist_append (headers, str.c_str ());
    curl_easy_setopt (_curl, CURLOPT_HTTPHEADER, headers);

    // 设置 body
    curl_easy_setopt (_curl, CURLOPT_POSTFIELDS, content.c_str ());
    curl_easy_setopt (_curl, CURLOPT_POSTFIELDSIZE, content.size ());

    // 注意这里类成员函数不能作为回调函数，必须采用类静态成员函数
    // 因为类成员函数采用的__thiscall调用规则
    // 而c的回调函数采用的__stdcal调用规则
    // 设置处理http response的body回调函数
    curl_easy_setopt (_curl, CURLOPT_WRITEFUNCTION, OnWriteResponseBody);
    curl_easy_setopt (_curl, CURLOPT_WRITEDATA, (void *)&response);

    // 设置处理http response的header回调函数
    curl_easy_setopt (_curl, CURLOPT_HEADERFUNCTION, OnWriteResponseHeader);
    curl_easy_setopt (_curl, CURLOPT_HEADERDATA, (void *)&response);

    // curl_easy_perform :设置超时
    curl_easy_setopt (_curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt (_curl, CURLOPT_NOSIGNAL, 1L);
	
    /** curl_easy_perform
	// 该函数是完成curl_easy_setopt指定的所有选项
	// 返回0意味一切ok，非0代表错误发生。主要错误码说明：
	// CURLE_OK :任务完成一切都好
	// CURLE_UNSUPPORTED_PROTOCOL :不支持的协议，由URL的头部指定
	// CURLE_COULDNT_CONNECT :不能连接到remote 主机或者代理
	// CURLE_REMOTE_ACCESS_DENIED :访问被拒绝
	// CURLE_HTTP_RETURNED_ERROR :Http返回错误
	// CURLE_READ_ERROR :读本地文件错误
	// 要获取详细的错误描述字符串，可以通过const char *curl_easy_strerror(CURLcode errornum ) 这个函数取得.
	*/
    CURLcode ret = curl_easy_perform (_curl);
    if (ret != CURLE_OK) {
        curl_slist_free_all (headers);
        if (ret == CURLE_COULDNT_CONNECT) {
            // 连接中断
        }
        else if (ret == CURLE_OPERATION_TIMEDOUT) {
            // POST超时
        }
        else if ((ret == CURLE_SEND_ERROR) || (ret == CURLE_RECV_ERROR) || (ret == CURLE_GOT_NOTHING)) {
            // 网络抖动，错误
        }
        else {
            // curl 其它错误
        }
		String errStr (curl_easy_strerror(ret));
		throw Exception(errStr.getCStr());
    }

    long retHttpCode = 0;
	
	/** curl_easy_getinfo
	// 获取应答头中特定的信息，比如应答码、cookies列表等
	// CURLcode curl_easy_getinfo(CURL *curl, CURLINFO info, ... ); 
	// info参数就是我们需要获取的内容，下面是一些参数值
	// CURLINFO_RESPONSE_CODE :获取应答码
	// CURLINFO_HEADER_SIZE :头大小
	// CURLINFO_COOKIELIST : cookies列表
	// 除了获取应答信息外，这个函数还能获取curl的一些内部信息，如请求时间、连接时间等等。
	*/
    curl_easy_getinfo (_curl, CURLINFO_RESPONSE_CODE, &retHttpCode);
    response.code = static_cast<int> (retHttpCode);
	
	// 释放消息头资源
    curl_slist_free_all (headers);
}

void
ncHttpClient::Get (const string & url, const int timeout, ncHttpResponse & response)
{
    AutoLock<ThreadMutexLock> lock (&_sendLock);

    // set url
    curl_easy_setopt (_curl, CURLOPT_URL, url.c_str ());

    // 注意这里类成员函数不能作为回调函数，必须采用类静态成员函数
    // 因为类成员函数采用的__thiscall调用规则
    // 而c的回调函数采用的__stdcal调用规则
    // 设置处理http response的body回调函数
    curl_easy_setopt (_curl, CURLOPT_WRITEFUNCTION, OnWriteResponseBody);
    curl_easy_setopt (_curl, CURLOPT_WRITEDATA, (void *)&response);

    // 设置处理http response的header回调函数
    curl_easy_setopt (_curl, CURLOPT_HEADERFUNCTION, OnWriteResponseHeader);
    curl_easy_setopt (_curl, CURLOPT_HEADERDATA, (void *)&response);

    // 设置超时
    curl_easy_setopt (_curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt (_curl, CURLOPT_NOSIGNAL, 1L);

    // 发送请求
    CURLcode ret = curl_easy_perform (_curl);
    if (ret != CURLE_OK) {
        if (ret == CURLE_COULDNT_CONNECT) {
            // 连接中断
        }
        else if (ret == CURLE_OPERATION_TIMEDOUT) {
            // POST超时
        }
        else if ((ret == CURLE_SEND_ERROR) || (ret == CURLE_RECV_ERROR) || (ret == CURLE_GOT_NOTHING)) {
            // 网络抖动，错误
        }
        else {
            // curl 其它错误
        }
		
		String errorMsg(curl_easy_strerror(ret));
		throw Exception(errorMsg.getCStr());
    }

    long retHttpCode = 0;
    curl_easy_getinfo (_curl, CURLINFO_RESPONSE_CODE, &retHttpCode);
    response.code = static_cast<int> (retHttpCode);
}


size_t 
ncHttpClient::OnWriteResponseBody (void *contents, size_t size, size_t nmemb, void *userdata)
{
    ncHttpResponse* r = reinterpret_cast<ncHttpResponse*> (userdata);
    r->body.append (reinterpret_cast<const char*>(contents), size * nmemb);

    return (size * nmemb);
}

size_t 
ncHttpClient::OnWriteResponseHeader (void *contents, size_t size, size_t nmemb, void *userdata)
{
    ncHttpResponse* r = reinterpret_cast<ncHttpResponse*> (userdata);
    string header (reinterpret_cast<char*> (contents), size * nmemb);
    size_t seperator = header.find_first_of (":");
    if (string::npos == seperator) {
        trim (header);// 去掉首尾空格
        if (0 == header.length ()) {
            return (size* nmemb); //blank line;
        }
        r->headers[header] = "present";
    }
    else {
        string key = header.substr (0, seperator);
        trim (key);
        string value = header.substr (seperator + 1);
        trim (value);
        r->headers[key] = value;
    }

    return (size* nmemb);
}



