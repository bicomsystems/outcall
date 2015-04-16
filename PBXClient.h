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

// PBXClient.h : main header file for the PBXCLIENT application
//

#if !defined(AFX_PBXCLIENT_H__E79B70A0_9234_4028_8052_2CA702CBC136__INCLUDED_)
#define AFX_PBXCLIENT_H__E79B70A0_9234_4028_8052_2CA702CBC136__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#include <vector>
#include <map>
using namespace std;

#include "MyLabel.h"

/////////////////////////////////////////////////////////////////////////////
// CPBXClientApp:
// See PBXClient.cpp for the implementation of this class
//

int SplitString(CString text, CString token, vector<CString> &ret);


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

protected:
	virtual BOOL OnInitDialog();	
	
	DECLARE_MESSAGE_MAP()

public:
	void OnOK();

	CMyLabel m_version;
	CMyLabel m_bsLink;
	afx_msg void OnStnClickedStaticLinkBs();		
	CButton m_btnOK;
	CStatic m_picture;
	CFont cf;	
	
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();	

	void InitLocaleGUI();
};


class CPBXClientApp : public CWinApp
{
public:
	CPBXClientApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPBXClientApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

public:
	//{{AFX_MSG(CPBXClientApp)
	//afx_msg void OnAppAbout();
	afx_msg void OnFileAbout();	
	afx_msg void OnFileOnlineHtml();
	afx_msg void OnFileOnlinePdf();
	afx_msg void OnFileContactSupport();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


public:			
	CString GetInstallDir();	
	BOOL m_bDeveloperVersion;
	BOOL m_bShellOpenFile;
    
	int m_build;
	void ExitApplication();
		
	BOOL m_bLoadingContacts;
	
	map<CString, CString> m_OutcallChannels;

	CWnd *m_pDialForm;
	
	bool m_bSignedIn;	

	BOOL CheckExtensionAuthorization(CString username, CString pass, CString protocol);

	void SetMenuItemText(CMenu *pMenu, UINT nID, CString text, bool bByPos=false);

	void SetLanguage(CString lang);

	bool CheckForExistingDialWindow();

	CString BuildOriginateCommand(CString channel, CString numberToDial, CString callerid, CString additional_options="", CString context="");
	void FormatOutgoingNumber(CString &number);

	// Retrieve an integer value from INI file or registry.
	UINT GetProfileInt(const char *lpszSection, const char *lpszEntry, int nDefault);

	// Sets an integer value to INI file or registry.
	BOOL WriteProfileInt(const char *lpszSection, const char *lpszEntry, int nValue);

	// Retrieve a string value from INI file or registry.
	CString GetProfileString(const char *lpszSection, const char *lpszEntry, const char* lpszDefault = NULL);

	// Sets a string value to INI file or registry.
	BOOL WriteProfileString(const char *lpszSection, const char *lpszEntry,	CString lpszValue);

	/*char* ConvertFromUnicodeToMB(const wchar_t *in);
	wchar_t* ConvertFromMBToUnicode(const char *in);*/

	CString EncodePassword(CString in);
	CString DecodePassword(CString in);

private:
	BOOL m_bAllowCmdLineOriginate;
	void ReadConfigFile(CString path);
	void IntegrateIntoOutlook();

	CAboutDlg m_AboutDlg;	

};


extern CPBXClientApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PBXCLIENT_H__E79B70A0_9234_4028_8052_2CA702CBC136__INCLUDED_)
