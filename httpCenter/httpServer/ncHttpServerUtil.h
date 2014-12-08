/*******************************************************************************
ncHttpServerUtil.h:
	Copyright (c) Eisoo Software Inc.(2014), All rights reserved

Purpose:
	Implementation file for the ncHttpServer

Author:
	wang.shenjian@eisoo.com

Creating Time:
	2014-11-26
*******************************************************************************/

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

//
// http返回码
//
#define NC_HTTP_OK					 200	/**< request completed ok */
#define NC_HTTP_NO_CONTENT			 204	/**< request does not have content */
#define NC_HTTP_MOVEPERM			 301	/**< the uri moved permanently */
#define NC_HTTP_MOVETEMP			 302	/**< the uri moved temporarily */
#define NC_HTTP_NOT_MODIFIED		 304	/**< page was not modified from last */
#define NC_HTTP_BAD_REQUEST			 400	/**< invalid http request was made */
#define NC_HTTP_ACCESS_TOKEN_INVALID 401	/**< invalid access token */
#define NC_HTTP_NOT_AUTHORIZED		 403	/**< not allowed to access for this user  */
#define NC_HTTP_NOT_FOUND			 404	/**< could not find content for uri */
#define NC_HTTP_BAD_METHOD			 405	/**< method not allowed for this uri */
#define NC_HTTP_ENTITY_TOO_LARGE	 413	/**<  */
#define NC_HTTP_EXPECT_ATION_FAILED 417	/**< we can't handle this expectation */
#define NC_HTTP_INTERNAL			 500	/**< internal error */
#define NC_HTTP_NOT_IMPLEMENTED	 501	/**< not implemented */
#define NC_HTTP_SERV_UNAVAIL		 503	/**< the server is not available */
#define NC_HTTP_PROCESSING			 508	/**< request is processing */
#define NC_HTTP_HAS_PASWORD			 509	/**< out link need password */

////////////////////////////////////////////////////////////////////////////////
//
// ncHttpBuffer
//

struct ncHttpBuffer
{
	ncHttpBuffer ()
		: buffer (0)
		, length (0)
	{
	}
	
	char* buffer;
	size_t length;
};

////////////////////////////////////////////////////////////////////////////////
// ncHttpServerUtil
//

class ncHttpServerUtil
{
public:
	static evutil_socket_t bindPort (unsigned short port);
	
	static void GetContent (evhttp_request* req, ncHttpBuffer& binary);
	static void GetBoundary (evhttp_request* req, ncHttpBuffer& binary);
	static string GetXRealIP (evhttp_request* req);
	static void Split (const ncHttpBuffer& origalStr,const ncHttpBuffer& boundary, ncHttpBuffer& binary, ncHttpBuffer& body);
	static void GenerateBoundary (const int lenth, string& boundary);
};

////////////////////////////////////////////////////////////////////////////////
// ncCallbackObj
//

template<class T>
class ncCallbackObj
{
public:
	T* _pCallback;
	void (T::*_pCallbackfun) (evhttp_request*);
};

template<class T>
inline void callbackByEvent (evhttp_request* req, void* arg)
{
	ncCallbackObj<T>* pObj = reinterpret_cast<ncCallbackObj<T>*> (arg);
	((pObj->_pCallback)->*(pObj->_pCallbackfun)) (req);
}

////////////////////////////////////////////////////////////////////////////////
//
// evbuffer的智能指针
//

class ncAutoEVBuffer
{
public:
	ncAutoEVBuffer ();
	ncAutoEVBuffer (const string& body);
	~ncAutoEVBuffer ();
	void Add (const string& body);
	void Add (unsigned char* buf, size_t len);
	void SendReply (evhttp_request *req, int code, const char *reason);
	void EnableLocking ();
	evbuffer* get ();

private:
	evbuffer* _evbuf;
};

////////////////////////////////////////////////////////////////////////////////
//
// evkeyvalq的智能指针
//

class ncAutoEVKeyValQ
{
public:
	ncAutoEVKeyValQ (evhttp_request* req);
	~ncAutoEVKeyValQ (void);
	String Find (const char* key, bool emptyThrow = true);
	
private:
	evkeyvalq _kvq;
};


