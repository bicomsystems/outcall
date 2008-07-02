/*
 * Copyright (c) 2003-2007, Bicom Systems Ltd.
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

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__CB200829_7768_47D1_A2B5_FFAFE60105A2__INCLUDED_)
#define AFX_STDAFX_H__CB200829_7768_47D1_A2B5_FFAFE60105A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


#if _MSC_VER > 1200

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0500	// Change this to the appropriate value to target IE 5.0 or later.
#endif

#endif


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS


#include "resource.h"

//extern char* (*gettext)(const char *__msgid);
//extern char* (*bindtextdomain)(const char *__domainname, const char *__dirname);
//extern char* (*textdomain)(const char *__domainname);

#define USE_GETTEXT_PUTENV
#include <libintl.h>

//#define _(String) (TCHAR*)gettext(String)


#ifdef _UNICODE
	extern CString GetUnicodeText(const char *String);
	#define _(String) GetUnicodeText(String)
#else
	#define _(String) gettext(String)
#endif


// Outlook 2000
//#import "mso9.dll" named_guids
//#import "MSOUTL9.OLB" no_namespace exclude("_IRecipientControl", "_DRecipientControl")
/*#import "C:\Program Files\Common Files\Microsoft Shared\Office10\mso.dll" named_guids
#import "C:\Program Files\Microsoft Office\Office\MSOUTL9.OLB" \
	no_namespace exclude("_IRecipientControl", "_DRecipientControl")*/

// Outlook 2003
#import "mso.dll" named_guids
#import "MSOUTL.OLB" no_namespace exclude("_IRecipientControl", "_DRecipientControl")
/*#import "C:\Program Files\Common Files\Microsoft Shared\Office11\mso.dll" named_guids
#import "C:\Program Files\Microsoft Office\Office11\MSOUTL.OLB" \
	no_namespace exclude("_IRecipientControl", "_DRecipientControl")*/


#define APP_NAME				_T("OutCALL")
#define WM_ICON_NOTIFY			WM_USER+10
#define OUTLOOK_MESSAGE			WM_USER+20
#define STOP_COMM_MESSAGE		WM_USER+30
#define NEW_CALL_MESSAGE		WM_USER+40
#define WM_REFRESH_MC_LIST		WM_USER+50
#define WM_AUTHENTICATE			WM_USER+60
#define WM_REFRESH_ACTIVE_CALLS WM_USER+70
#define WM_ENABLE_DIAL_BUTTON   WM_USER+80

//#define WM_TRAY_ICON_DBLCLICK	WM_USER+40

#define MAX_REG_KEY_NAME		100
//#define MAX_REG_KEY_VALUE		32767

#define BRAND_COPYRIGHT		_T("(c) Copyright Bicom Systems Ltd. 2003-2008")
#define BRAND_POWERED_BY	_T("Bicom Systems Ltd.")
#define BRAND_WEB_PAGE		_T("http://www.bicomsystems.com")

#define LINK_DOCS_HTML		_T("http://www.bicomsystems.com/docs/outcall/1.0/html/")
#define LINK_DOCS_PDF		_T("http://www.bicomsystems.com/docs/outcall/1.0/pdf/")

#define POPUP_BITMAP_ID		IDB_POPUP

#define OUTCALL_VERSION		55

#define REG_KEY_APP			_T("BicomSystems")
#define APP_REG_KEY			_T("Software\\BicomSystems\\OutCALL")
#define REG_KEY_EXTENSIONS	_T("Software\\BicomSystems\\OutCALL\\Extensions")

//#define OUTCALL_DB	"outcall.db"
#define GETTEXT_DOMAIN "outcall"
extern TCHAR OUTCALL_DB[256];
extern BOOL AUTH_ENABLED;

#undef fprintf; // libintl.h defines fprintf which in debug version causes access violation

#ifdef ENABLE_DBGLOG	
	extern FILE *g_hLogFile;
	extern CString g_LogFilePath;
	extern void InitializeFileLogger();

  		#define DBG_LOG(message) \
		g_hLogFile = _tfopen(g_LogFilePath, _T("a")); \
		if (g_hLogFile) { \
			time_t now; \
			struct tm* currentTime; \
			time(&now); \
			currentTime = localtime(&now); \
			fprintf(g_hLogFile, "%02d.%02d.%d %02d:%02d - %s\r\n", currentTime->tm_mday, \
			currentTime->tm_mon + 1, currentTime->tm_year + 1900, currentTime->tm_hour, currentTime->tm_min, message); \
			fclose(g_hLogFile); \
		}
#else
	#define DBG_LOG(s)
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__CB200829_7768_47D1_A2B5_FFAFE60105A2__INCLUDED_)
