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

// ActiveCallsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PBXClient.h"
#include "ActiveCallsDlg.h"

#include "MainFrm.h"


// CActiveCallsDlg dialog

IMPLEMENT_DYNAMIC(CActiveCallsDlg, CDialog)
CActiveCallsDlg::CActiveCallsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CActiveCallsDlg::IDD, pParent)
{
}

CActiveCallsDlg::~CActiveCallsDlg()
{
}

void CActiveCallsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CActiveCallsDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_HANGUP, OnBnClickedButtonHangup)
	ON_MESSAGE(WM_REFRESH_ACTIVE_CALLS, OnRefreshMessage)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CActiveCallsDlg message handlers

LRESULT CActiveCallsDlg::OnRefreshMessage(WPARAM wParam, LPARAM lParam) {
    LoadActiveCalls();
	return 1;
}


void CActiveCallsDlg::OnBnClickedButtonHangup()
{
	int nItem = m_list.GetNextItem(-1, LVNI_SELECTED);
	if (nItem==-1)
		return;
	if (MessageBox(_("Are you sure you want to hangup the selected channels?"), APP_NAME, MB_YESNO | MB_ICONQUESTION)==IDYES) {
		map<CString, CString> &OutcallChannels = ::theApp.m_OutcallChannels;
		map<CString, CString>::iterator iter;
		CString channel;
		while (nItem!=-1) {
			channel = *((CString*)m_list.GetItemData(nItem));
			iter = OutcallChannels.find(channel);			
			((CMainFrame*)::theApp.m_pMainWnd)->m_socketManager.SendData("Action: Hangup\r\nChannel: " + channel + "\r\n\r\n");
			OutcallChannels.erase(iter);
			delete (CString*)m_list.GetItemData(nItem);
			m_list.DeleteItem(nItem);
			nItem--;
			nItem = m_list.GetNextItem(nItem, LVNI_SELECTED);
		}
	}
}

BOOL CActiveCallsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();	

	RECT rect;
	m_list.GetClientRect(&rect);
	m_list.InsertColumn(0, _("From"), LVCFMT_LEFT, rect.right*0.5);
	m_list.InsertColumn(1, _("To"), LVCFMT_LEFT, rect.right*0.45);
	LONG style = GetWindowLong(m_list.GetSafeHwnd(), GWL_STYLE);
	style |= (LVS_EX_FULLROWSELECT | LVS_REPORT | LVS_SHOWSELALWAYS);
	style &= ~LVS_SORTDESCENDING;
	style &= ~LVS_SORTASCENDING;
	SetWindowLong(m_list.GetSafeHwnd(), GWL_STYLE, style);
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	LoadActiveCalls();

	InitLocaleGUI();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CActiveCallsDlg::LoadActiveCalls() {
	map<CString, CString> &OutcallChannels = ::theApp.m_OutcallChannels;
	map<CString,CString>::iterator iter;
	CString data, from, to;
	int index;

	for (int i=0; i<m_list.GetItemCount(); i++) {
		delete (CString*)m_list.GetItemData(i);	
	}
	m_list.DeleteAllItems();

	for (iter=OutcallChannels.begin(); iter != OutcallChannels.end(); iter++) {		
		data = iter->second;
		index = data.Find(_T("||"));
		from = data.Left(index);
		to = data.Mid(index+2);
		index = m_list.InsertItem(m_list.GetItemCount(), from);
		m_list.SetItemText(index, 1, to);
		m_list.SetItemData(index, (DWORD_PTR)new CString(iter->first));
	}
}

void CActiveCallsDlg::OnDestroy()
{
	CDialog::OnDestroy();

	for (int i=0; i<m_list.GetItemCount(); i++) {
		delete (CString*)m_list.GetItemData(i);	
	}
}


void CActiveCallsDlg::InitLocaleGUI() {
	SetWindowText(CString(APP_NAME) + " - " + _("Active Calls"));
	GetDlgItem(IDC_STATIC_ACTIVE_CALLS)->SetWindowText(CString(_("Active Calls")) + ":");
    GetDlgItem(IDCANCEL)->SetWindowText(_("&Cancel"));
	GetDlgItem(IDC_BUTTON_HANGUP)->SetWindowText(_("&Hangup"));
	GetDlgItem(IDOK)->SetWindowText(_("Close"));
}
