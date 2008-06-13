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

// OutlookAddin.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "OutlookAddin.h"
#include "MyAddin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define OLEXT_NAME	"OutlookAddin"
//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// COutlookAddinApp

BEGIN_MESSAGE_MAP(COutlookAddinApp, CWinApp)
	//{{AFX_MSG_MAP(COutlookAddinApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COutlookAddinApp construction

COutlookAddinApp::COutlookAddinApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only COutlookAddinApp object

COutlookAddinApp theApp;


LPEXCHEXT CALLBACK ExchEntryPoint()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CMyAddin *pExt = new CMyAddin();
	return pExt;
}

STDAPI DllRegisterServer()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	char szFileName[MAX_PATH]={0};
	DWORD dwPath = GetModuleFileName(AfxGetInstanceHandle(), szFileName, _MAX_PATH ); 
	if (dwPath != 0)
	{
		CString csName = OLEXT_NAME;
		char szModuleFileNameShort[512]={0};
		GetShortPathName(szFileName,szModuleFileNameShort,512);

		HKEY hKey = NULL;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Exchange\\Client\\Extensions",0L, KEY_WRITE, &hKey)==ERROR_SUCCESS)
		{
			char szExtensionValue[_MAX_PATH + 100];
			sprintf(szExtensionValue,"4.0;%s;1;11111111111111;1111111",szModuleFileNameShort);
			if (RegSetValueEx(hKey, csName.GetBuffer(0),0L, REG_SZ,(unsigned char*)szExtensionValue, (DWORD)strlen(szExtensionValue))!=ERROR_SUCCESS)
			{
				RegCloseKey(hKey);
				return FALSE;
			}
			char szUpdateRegistry[] = "4.0;Outxxx.dll;7;00000000000000;0000000;OutXXX";
			if (RegSetValueEx(hKey, "Outlook Setup Extension",0L, REG_SZ, (unsigned char*)szUpdateRegistry, (DWORD)strlen(szUpdateRegistry))!=ERROR_SUCCESS)
			{
				RegCloseKey(hKey);
				return FALSE;
			} 
			RegCloseKey(hKey);
		}
	}

	return S_OK;
}

STDAPI DllUnregisterServer()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CString csName = OLEXT_NAME;
	HKEY hKey = NULL;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Exchange\\Client\\Extensions", 0L, KEY_ALL_ACCESS, &hKey)==ERROR_SUCCESS)
	{
		if (RegDeleteValue(hKey,csName.GetBuffer(0))!=ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			return FALSE;
		}

		char szUpdateRegistry[] = "4.0;Outxxx.dll;7;00000000000000;0000000;OutXXX";
		if (RegSetValueEx(hKey, "Outlook Setup Extension", 0L, REG_SZ, (unsigned char*)szUpdateRegistry, (DWORD)strlen(szUpdateRegistry))!=ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			return FALSE;
		}
		RegCloseKey(hKey);
	}	

	return S_OK;
} 
