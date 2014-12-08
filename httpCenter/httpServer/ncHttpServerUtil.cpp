/*******************************************************************************
ncHttpServerUtil.cpp:
	Copyright (c) Eisoo Software Inc.(2014), All rights reserved

Purpose:
	Implementation file for the ncHttpServer

Author:
	wang.shenjian@eisoo.com

Creating Time:
	2014-11-26
*******************************************************************************/

#include <abprec.h>
#include "ncHttpServerUtil.h"

////////////////////////////////////////////////////////////////////////////////
// ncHttpServerUtil
//

// public 
evutil_socket_t
ncHttpServerUtil::bindPort (unsigned short port)
{
	/**
	struct sockaddr_in 
	{ 
		short 			sin_family;		//一般来说AF_INET（地址族）PF_INET（协议族）,在socket编程中只能是AF_INET
		unsigned short	sin_port;		//存储端口号（使用网络字节顺序）,普通数字可以用htons()函数转换成网络数据格式的数字,在linux下，端口号的范围0~65535,同时0~1024范围的端口号已经被系统使用或保留。
		in_addr 		sin_addr;		//存储IP地址，使用in_addr这个数据结构
		unsigned char	sin_zero[8];//没有实际意义,只是为了让sockaddr与sockaddr_in两个数据结构保持大小相同而保留的空字节
	};
	struct in_addr 
	{ 
		unsigned long s_addr; 			//存储IP地址（按照网络字节顺序）
	};
	inet_addr()    将字符串点数格式地址转化成无符号长整型（unsigned long s_addr s_addr;）
	inet_aton()    将字符串点数格式地址转化成NBO
	inet_ntoa ()     将NBO地址转化成字符串点数格式
	htons()    "Host to Network Short"
	htonl()    "Host to Network Long"
	ntohs()    "Network to Host Short"
	ntohl()    "Network to Host Long"
	常用的是htons(),inet_addr()正好对应结构体的端口类型和地址类型
	htons/l和ntohs/l等数字转换都不能用于地址转换，因为地址都是点数格式，所以地址只能采用数字/字符串转换如inet_aton,inet_ntoa;
	唯一可以用于地址转换的htons是针对INADDR_ANY
	*/
	sockaddr_in in;
	memset (&in, 0, sizeof (in));	// 拷贝第二个参数到in从头开始的sizeof(in)个字符里并返回in指针，用于将一段内存初始化为某个值，此处将struct sockaddr_in各成员初始化为0
	in.sin_family = AF_INET;
	in.sin_port = htons (port);
	in.sin_addr.s_addr = htonl (INADDR_ANY);// INADDR_ANY就是指定地址为0.0.0.0的地址，这个地址事实上表示不确定地址，或“所有地址”、“任意地址”
	
	
	/** evutil_socket_t
	#ifdef WIN32
	#define evutil_socket_t intptr_t
	#else
	#define evutil_socket_t int
	#endif
	*/
	evutil_socket_t fd = socket (AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
		return -1;
	
	// 将其设置为非阻塞的（设置Unix中的O_NONBLOCK标志和Windows中的FIONBIO标志）
	if (evutil_make_socket_nonblocking (fd) < 0) {
		evutil_closesocket (fd);
		return -1;
	}
	
	// 设置套接字的属性
	int on = 1;
	if (setsockopt (fd, SOL_SOCKET, SO_KEEPALIVE, (const char *)&on, sizeof (on)) < 0) {
		evutil_closesocket (fd);
		return -1;
	}
	
	// 确保关闭监听套接字后，它使用的地址可以立即被另一个套接字使用，
	// 在Unix中它设置SO_REUSEADDR标志，在Windows中则不做任何操作。
	if (evutil_make_listen_socket_reuseable (fd) < 0) {
		evutil_closesocket (fd);
		return -1;
	}

	// 绑定本地地址到套接字
	if (::bind (fd, (sockaddr*)&in, sizeof (sockaddr_in)) < 0) {
		evutil_closesocket (fd);
		return -1;
	}
	
	// 监听申请的连接，并设置等待连接队列的最大长度。
	listen (fd, 128);
	return fd;
}


// public static
void
ncHttpServerUtil::GetContent (evhttp_request* req, ncHttpBuffer& binary)
{
	evbuffer* buffer = evhttp_request_get_input_buffer (req);
	binary.length = evbuffer_get_length (buffer);
	binary.buffer = reinterpret_cast<char*> (evbuffer_pullup (buffer, -1));
}

// public static
void
ncHttpServerUtil::GetBoundary (evhttp_request* req, ncHttpBuffer& binary)
{
	const char* headContentType = evhttp_find_header (req->input_headers, "Content-Type");
	if (headContentType == NULL) {
		throw;//todo:xxx
	}

	string bound ("boundary=");
	size_t headContentLength = strlen (headContentType);
	binary.buffer = std::search (const_cast<char*> (headContentType),
								 const_cast<char*> (headContentType) + headContentLength,
								 bound.c_str (),
								 bound.c_str () + bound.size ());
	
	if (binary.buffer == (const_cast<char*> (headContentType) + headContentLength)) {
		throw;//todo:xxx
	}
	binary.buffer += bound.size ();
	binary.length = strlen (binary.buffer);
}

string
ncHttpServerUtil::GetXRealIP (evhttp_request* req)
{
	const char* headIp = evhttp_find_header (req->input_headers, "X-Real-IP");

	if (!headIp || *headIp == '\0') {
		return string ();
	}
	
	return string (headIp);
}

// public static
void
ncHttpServerUtil::Split (const ncHttpBuffer& origalStr, const ncHttpBuffer& boundary, ncHttpBuffer& binary, ncHttpBuffer& body)
{
	// 找第一个分隔符
	binary.buffer = std::search (origalStr.buffer, origalStr.buffer + origalStr.length, boundary.buffer, boundary.buffer + boundary.length);
	if (binary.buffer == (origalStr.buffer + origalStr.length)) {
		throw;//todo:xxx
	}

	// 找双空行分隔符
	string newLine ("\r\n\r\n");
	binary.buffer = std::search (binary.buffer + boundary.length,
								 origalStr.buffer + origalStr.length,
								 newLine.c_str (),
								 newLine.c_str () + newLine.size ());
	if (binary.buffer == (origalStr.buffer + origalStr.length)) {
		throw;//todo:xxx
	}

	// 找到了二进制头指针
	binary.buffer += newLine.size ();

	// 找第二个分隔符
	body.buffer = std::search (binary.buffer, origalStr.buffer + origalStr.length, boundary.buffer, boundary.buffer + boundary.length);
	if (body.buffer == (origalStr.buffer + origalStr.length)) {
		throw;//todo:xxx
	}

	// 设置二进制的长度(4为一个换行符和两个杠)
	binary.length = body.buffer - binary.buffer - 4;

	// 找双空行分隔符
	body.buffer = std::search (body.buffer + boundary.length,
							   origalStr.buffer + origalStr.length,
							   newLine.c_str (),
							   newLine.c_str () + newLine.size ());
	if (body.buffer == (origalStr.buffer + origalStr.length)) {
		throw;//todo:xxx
	}

	// 找到了Body头指针
	body.buffer += newLine.size ();

	// 找第三个分隔符
	char* last = std::search (body.buffer, origalStr.buffer + origalStr.length, boundary.buffer, boundary.buffer + boundary.length);
	if (last == (origalStr.buffer + origalStr.length)) {
		throw;//todo:xxx
	}

	// 设置二进制的长度(4为一个换行符和两个杠)
	body.length = last - body.buffer - 4;
}

// public static
void
ncHttpServerUtil::GenerateBoundary (const int lenth, string& boundary)
{
	const char hex[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	boundary.reserve (lenth);
	for (int i = 0; i < lenth; ++i) {
		int x = rand () % 62;
		boundary.append (1, hex[x]);
	}
}

////////////////////////////////////////////////////////////////////////////////
// ncAutoEVBuffer
//

ncAutoEVBuffer::ncAutoEVBuffer ()
	: _evbuf (0)
{
	_evbuf = evbuffer_new ();
	if (_evbuf == 0) {
		throw;//todo:xxx
	}
}

ncAutoEVBuffer::ncAutoEVBuffer (const string& body)
	: _evbuf (0)
{
	_evbuf = evbuffer_new ();
	if (_evbuf == 0) {
		throw;//todo:xxx
	}
	Add (body);
}

ncAutoEVBuffer::~ncAutoEVBuffer ()
{
	evbuffer_free (_evbuf);
}

void 
ncAutoEVBuffer::Add (const string& body)
{
	if (evbuffer_add (_evbuf, body.c_str (), body.length ())) {
		throw;//todo:xxx
	}
}

void 
ncAutoEVBuffer::Add (unsigned char* buf, size_t len)
{
	if (evbuffer_add (_evbuf, buf, len)) {
		throw;//todo:xxx
	}
}

void 
ncAutoEVBuffer::SendReply (evhttp_request *req, int code, const char *reason)
{
	evhttp_send_reply (req, code, reason, _evbuf);
}

void 
ncAutoEVBuffer::EnableLocking ()
{
	if (evbuffer_enable_locking (_evbuf, 0) == -1) {
		throw;//todo:xxx
	}
}

evbuffer* 
ncAutoEVBuffer::get ()
{
	return _evbuf;
}


////////////////////////////////////////////////////////////////////////////////
// ncAutoEVBuffer
//

ncAutoEVKeyValQ::ncAutoEVKeyValQ (evhttp_request* req)
{
	evhttp_parse_query (evhttp_request_get_uri (req), &_kvq);
}

ncAutoEVKeyValQ::~ncAutoEVKeyValQ (void)
{
	evhttp_clear_headers (&_kvq);
}

String 
ncAutoEVKeyValQ::Find (const char* key, bool emptyThrow/* = true*/)
{
	if (!key) {
		throw;//todo:xxx
	}

	const char* header = evhttp_find_header (&_kvq, key);
	if (!header || *header == '\0') {
		if (emptyThrow) {
			throw;//todo:xxx
		}
		else {
			return String::EMPTY;
		}
	}
	else {
		return String (header);
	}
}


