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

// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__B340B4BF_7E20_43CF_919D_DD156CA78345__INCLUDED_)
#define AFX_MAINFRM_H__B340B4BF_7E20_43CF_919D_DD156CA78345__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SysTray.h"
#include "TaskbarNotifier.h"
#include "SettingsDlg.h"
#include "DebugDlg.h"

#define CF_WINDOW_WIDTH		256
#define CF_WINDOW_HEIGHT	180

#define TIMER_CONNECTING				5
#define TIMER_SIGN_IN					10
#define TIMER_REFRESH_CONTACTS			15
#define TIMER_ORIGINATE_CALL			20


#include "SocketManager.h"
#include "SocketComm.h"
#include "CppSQLite3.h"

//extern char* DATABASE_ACCESS_MUTEX;

UINT RefreshContactsProc( LPVOID pParam );

class CMainFrame : public CFrameWnd
{
	
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnFileSettings();
	afx_msg void OnFileConnect();
	afx_msg void OnFileCall();
	afx_msg void OnFileFind();
	afx_msg void OnFileActiveCalls();	
	afx_msg void OnFileAddContact();
	afx_msg void OnFileDebugInfo();
	afx_msg void OnFileMissedCalls();
	afx_msg void OnFileRefreshContactsFromOutlook();
	afx_msg void OnFileRefreshContactsFromCSV();
	afx_msg void OnUpdateFileConnect(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileCall(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileAddContact(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileFindContact(CCmdUI* pCmdUI);
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);	
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	BOOL m_bCancelSignIn;
	BOOL m_bSigningIn;
	BOOL m_bManualSignInPerformed;

	void CreateDatabaseTable(CString table, CString fields);

private:
	void OpenContact(CTaskbarNotifier *notifyWnd);
	BOOL DisplayContact(MAPIFolderPtr pFolder, CString fullName);
	CString FindContact(CString callerID1, CString callerID2, CString &callerPhoneNumber);
	void UpdateDissableIcon();
	
	BOOL DisplayWindow(BOOL bShow);	
	BOOL Init();
	
	void SignIn(BOOL manualSignIn);
		
	BOOL m_SignInMenuItemData;
	
	LRESULT OnAuthenticateMsg(WPARAM wParam, LPARAM lParam);
	LRESULT OnTrayNotification(WPARAM wParam, LPARAM lParam);
	LRESULT OnShowDialDialog(WPARAM wParam, LPARAM lParam);	
	LRESULT OnStopSocketCommunication(WPARAM wParam, LPARAM lParam);
	LRESULT OnCallNotifyDialogClick(WPARAM wParam, LPARAM lParam);
	LRESULT OnNewCallMsg(WPARAM wParam, LPARAM lParam);
	
	CBrush		m_BrushToolTip;
	CBrush		m_BrushShadow;

	_ApplicationPtr pOutlookApp; //Outlook Application Instance
	CString contactToDisplay;
	
	CTaskbarNotifier m_wndTaskbarNotifier[5];
	CDebugDlg m_DebugDlg;

	BOOL m_bSignOutMessageBoxActive;
	

public:
	void ShowNotificationDialog(CString message);
	BOOL IsOutCALLConfigured();
	void OnTrayIconDblClick();
	void UpdateTrayIcon();	

	CSocketManager m_socketManager;
	CSysTray m_TrayIcon;

	void SignOut();	
	void GetCSVValues(CString line, vector<CString> &values);
	HRESULT LoadProfileDetails(vector<CString> &vProfiles, int &nDefault, bool bInitializeMAPI);

	void LoadOutlookContacts(BOOL bDeleteExistingContacts/*if this parameter is NULL, load from Outlook, otherwise load from CSV*/);
	void CancelLoadOutlookContacts();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__B340B4BF_7E20_43CF_919D_DD156CA78345__INCLUDED_)
