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

// OutlookContactsDlg.h : header file
//

#if !defined(AFX_OUTLOOKCONTACTSDLG_H__073E32D3_7E7B_49A9_A3DB_A6138FADFB0C__INCLUDED_)
#define AFX_OUTLOOKCONTACTSDLG_H__073E32D3_7E7B_49A9_A3DB_A6138FADFB0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// COutlookContactsDlg dialog
#include <vector>
#include "SocketManager.h"
#include "afxwin.h"
#include "afxcmn.h"

/*#import "C:\Program Files\Common Files\Microsoft Shared\Office10\mso.dll" named_guids
#import "C:\Program Files\Microsoft Office\Office\MSOUTL9.OLB" \
	no_namespace exclude("_IRecipientControl", "_DRecipientControl")*/

#define WM_SHOW_LOADING_CONTACTS_DLG_MSG	WM_USER+1
//#define WM_LOADCONTACTS_FINISHED_MSG	WM_USER+2

class COutlookContactsDlg : public CDialog
{
// Construction
public:
	COutlookContactsDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(COutlookContactsDlg)
	enum { IDD = IDD_OUTLOOKCONTACTS_DIALOG };
	CComboBox m_contacts;
	CComboBox m_numbers;
	CEdit m_phoneNumber;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COutlookContactsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;	

	// Generated message map functions
	//{{AFX_MSG(COutlookContactsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnImport();
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeContacts();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


private:
	void DisplayContactNumbers();
	void LoadContacts();
	void LoadExtensions();	
	
	LRESULT OnDisplayLoadingContactsDlg(WPARAM wParam, LPARAM lParam);
	//LRESULT OnLoadContactsFinishedMessage(WPARAM wParam, LPARAM lParam);
	LRESULT OnEnableDialButton(WPARAM wParam, LPARAM lParam);

	//CDialog m_DlgLoadingContacts;
	BOOL m_bFilterContacts;

public:
	CString contactToDisplay;
	BOOL m_LoadContacts;
	vector<CString> m_Callees;
	vector<CString> m_CallerPhones;
	BOOL m_bMultipleCalls;
	CSocketManager *socketManager;
	
	CComboBox m_DialFrom;
	CStatic m_separator;
	CStatic m_separator2;
	CButton m_btnSkip;
	afx_msg void OnBnClickedButtonSkip();
	CListCtrl m_list;
	CButton m_btnDial;
	CButton m_btnExit;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	
	afx_msg void OnPopupRefreshFromOutlook();
	afx_msg void OnPopupRefreshFromCSV();

	afx_msg void OnPopupFirstname();
	afx_msg void OnPopupLastname();
	afx_msg void OnPopupHideContacts();
	
	afx_msg void OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult);	
	
	CMenu menu;
	afx_msg void OnBnClickedButtonOptions();
	CButton m_btnOptions;
	int displayOptions;

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonFind();
	afx_msg void OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult);	

	void InitLocaleGUI();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OUTLOOKCONTACTSDLG_H__073E32D3_7E7B_49A9_A3DB_A6138FADFB0C__INCLUDED_)
