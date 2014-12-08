/*******************************************************************************
ncHttpDef.h:
	Copyright (c) Eisoo Software Inc.(2014), All rights reserved

Purpose:
	Implementation file for the ncHttpDef

Author:
	wang.shenjian@eisoo.com

Creating Time:
	2014-11-26
*******************************************************************************/

#ifndef __NC_HTTPDEF_H__
#define __NC_HTTPDEF_H__

////////////////////////////////////////////////////////////////////////////////
// ncHttpResponse
//

struct ncHttpResponse {
	int code;
	string body;
	map<string, string> headers;

	ncHttpResponse ()
		:code (0)
		,body ()
		,headers ()
	{
	}
};




#endif // __NC_HTTPDEF_H__
