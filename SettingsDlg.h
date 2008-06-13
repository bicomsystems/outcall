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

#if !defined(AFX_SETTINGSDLG_H__0B746EE2_970F_49FB_9F78_2D8020053F34__INCLUDED_)
#define AFX_SETTINGSDLG_H__0B746EE2_970F_49FB_9F78_2D8020053F34__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SettingsDlg.h : header file
//

#include "afxcmn.h"
#include "MyLabel.h"

#include "SettingsGeneralDlg.h"
#include "SettingsOutlookDlg.h"
#include "SettingsExtensionsDlg.h"
#include "SettingsServerDlg.h"
#include "SettingsDialingRules.h"
#include "SettingsImportRules.h"


#include <map>
#include "afxwin.h"
using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg dialog

class CSettingsDlg : public CDialog
{
// Construction
public:
	CSettingsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSettingsDlg)
	enum { IDD = IDD_DIALOG_SETTINGS };	
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettingsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSettingsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void SaveSettings();
	void UpdateRegistryExtensions();

	void InitLocaleGUI();

	bool m_bSettingsSaved;

	CString old_server, old_username, old_password, old_port;
    void OnTabSelectionChanged();

protected:
	virtual void OnCancel();

public:
	afx_msg void OnBnClickedApply();		
	
	CMyLabel m_bsLink;
	afx_msg void OnStnClickedStaticLinkBs();
	CTabCtrl m_tab;

	CSettingsServerDlg m_dlgServer;
	CSettingsExtensionsDlg m_dlgExtensions;
	CSettingsOutlookDlg m_dlgOutlook;
	CSettingsGeneralDlg m_dlgGeneral;
	CSettingsDialingRules m_dlgDR;
	CSettingsImportRules m_dlgImportRules;

	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGSDLG_H__0B746EE2_970F_49FB_9F78_2D8020053F34__INCLUDED_)
