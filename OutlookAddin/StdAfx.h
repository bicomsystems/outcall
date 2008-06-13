// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__6937A550_DC8A_4FA1_898F_7181327DB666__INCLUDED_)
#define AFX_STDAFX_H__6937A550_DC8A_4FA1_898F_7181327DB666__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT


#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>			// MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>			// MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


#include <atlbase.h>

#include <mapix.h>
#include <mapiutil.h>
#include <mapitags.h>
#include <mapiform.h>
#include <initguid.h>
//#include <mapiguid.h>
#include <exchext.h>
#include <exchform.h>
#include <mapidefs.h>
#include <mapispi.h>
#include <imessage.h>
#include <ocidl.h>

#pragma comment(lib,"mapi32")
#pragma comment(lib,"Rpcrt4")
#pragma comment(lib,"Version") 

// If you have Outlook 2000 installed on your machine, then use the following lines
//#import "mso9.dll"
//#import "msoutl9.olb"


// Outlook 2003
#import "mso.dll"
#import "msoutl.olb"
/*#import "C:\Program Files\Common Files\Microsoft Shared\Office11\mso.dll" named_guids
#import "C:\Program Files\Microsoft Office\Office11\MSOUTL.OLB" \
	no_namespace exclude("_IRecipientControl", "_DRecipientControl")*/

#define APP_NAME	_T("OutCALL")
#define APP_REG_KEY "Software\\BicomSystems\\OutCALL\\Toolbar"

// If you have Outlook 2002 installed on your machine, then use the following lines
//#import "mso.dll" 
//#import "msoutl.olb"

// If you have Outlook 2003 installed on your machine, then use the following lines
//#import "mso11.dll" 
//#import "msoutl11.olb"
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__6937A550_DC8A_4FA1_898F_7181327DB666__INCLUDED_)
