/*
 * Copyright (c) 2003-2010, Bicom Systems Ltd.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   - Redistributions of source code must retain the above copyright notice,
 *	   this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright
 *	   notice, this list of conditions and the following disclaimer in the
 *	   documentation and/or other materials provided with the distribution.
 *   - Neither the name of the Bicom Systems Ltd nor the names of its
 *	   contributors may be used to endorse or promote products derived from this
 *	   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * 
 * Denis Komadaric,
 * Bicom Systems Ltd.
 */

// stdafx.cpp : source file that includes just the standard includes
//	PBXClient.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

//char* (*gettext)(const char *__msgid);
//char* (*bindtextdomain)(const char *__domainname, const char *__dirname);
//char* (*textdomain)(const char *__domainname);

#ifdef _UNICODE
	CString GetUnicodeText(const char *String) {
        char *ch = gettext(String);
		if (strcmp(ch, String)==0)
			return CString((char*)ch);
		else
			return CString((wchar_t*)ch);
	}
#endif

#ifdef ENABLE_DBGLOG
	CString g_LogFilePath;
	FILE *g_hLogFile = NULL;

	void InitializeFileLogger() {
        TCHAR dir[256];
		CString path;
		if (SHGetFolderPath(NULL,CSIDL_APPDATA,NULL,0,dir)==S_OK) {			
            path = CString(dir) + _T("\\") + CString(APP_NAME);
            CreateDirectory(path, NULL);
		} else {
            if (GetCurrentDirectory(256, dir)==0)
				return;
			else
				path = dir;
		}					
        g_LogFilePath = path + _T("\\outcall.log");
		g_hLogFile = _tfopen(g_LogFilePath, _T("a"));
		if (g_hLogFile) {
			long size;
			fseek(g_hLogFile, 0, SEEK_END);
			size = ftell(g_hLogFile);
			if (size>5*1024*1024) { // 5 MB maximum size
				rewind(g_hLogFile);
			}
			fclose(g_hLogFile);
		}
	}
#endif

bool IsNumeric(const char *p)
{
	for ( ; *p; p++)
		if (*p < '0' || *p > '9')
			return false;
	return true;
}

CString EscapeSQLString(CString s) {
	s.Replace(L"'", L"''");
	return s;
}