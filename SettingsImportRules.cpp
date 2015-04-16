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

// SettingsImportRules.cpp : implementation file
//

#include "stdafx.h"
#include "PBXClient.h"
#include "SettingsImportRules.h"
#include "OutlookRuleDlg.h"
#include "MainFrm.h"
#include <vector>
#include "LoadingContactsDlg.h"

// Sort the item in the list by length in descending order.
static int CALLBACK 
ListCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
   // lParamSort contains a pointer to the list view control.
   CListCtrl* pListCtrl = (CListCtrl*) lParamSort;
   
   int nLen1 = ((CString*)lParam1)->GetLength();
   int nLen2 = ((CString*)lParam2)->GetLength();
   
   //return strcmp(pListCtrl->GetItemText(lParam1, 0), pListCtrl->GetItemText(lParam2, 0));
   
   if (nLen1<nLen2)
	   return 1;
   else if (nLen1>nLen2)
	   return -1;
   else
	   return 0;
}

// CSettingsImportRules

IMPLEMENT_DYNAMIC(CSettingsImportRules, CDialog)

CSettingsImportRules::CSettingsImportRules(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingsImportRules::IDD, pParent)
{
}

CSettingsImportRules::~CSettingsImportRules()
{
}

void CSettingsImportRules::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
}

BEGIN_MESSAGE_MAP(CSettingsImportRules, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnBnClickedButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnBnClickedButtonEdit)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE_DATABASE, OnBnClickedButtonUpdateDatabase)
	ON_NOTIFY(LVN_ODSTATECHANGED, IDC_LIST, OnLvnOdstatechangedList)
	ON_NOTIFY(NM_CLICK, IDC_LIST, OnNMClickList)
	ON_NOTIFY(NM_SETFOCUS, IDC_LIST, OnNMSetfocusList)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CSettingsImportRules message handlers

void CSettingsImportRules::LoadSettings() {
	int nItem;
	RECT rect;

	m_list.GetClientRect(&rect);
	m_list.InsertColumn(0, _("Numbers starting with"), LVCFMT_LEFT, rect.right*0.5);
	m_list.InsertColumn(1, _("Replace with"), LVCFMT_LEFT, rect.right*0.45);

	LONG style = GetWindowLong(m_list.GetSafeHwnd(), GWL_STYLE);
	style |= (LVS_EX_FULLROWSELECT | LVS_REPORT | LVS_SHOWSELALWAYS);
	style &= ~LVS_SORTDESCENDING;
	style &= ~LVS_SORTASCENDING;
	SetWindowLong(m_list.GetSafeHwnd(), GWL_STYLE, style);
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	
	std::vector<CString> rules;
	SplitString(::theApp.GetProfileString("Settings", "ImportOutlookRules", ""), " || ", rules);
	for (int i=0; i<rules.size(); i++) {
		if ((i%2)==0) {
			nItem = m_list.InsertItem(m_list.GetItemCount(), rules[i]);
			m_list.SetItemData(nItem, (DWORD_PTR)new CString(rules[i]));
		} else {
			m_list.SetItemText(nItem, 1, rules[i]);
		}
	}

	m_list.SortItems(ListCompareProc, (LPARAM)&m_list);
}


void CSettingsImportRules::OnBnClickedButtonAdd()
{
	COutlookRuleDlg dlg;
	dlg.list = &m_list;
	if (dlg.DoModal()==IDOK) {
		m_list.SortItems(ListCompareProc, (LPARAM)&m_list);
	}
}

void CSettingsImportRules::OnBnClickedButtonEdit()
{
	COutlookRuleDlg dlg;
	dlg.list = &m_list;
	dlg.edit = TRUE;
	if (dlg.DoModal()==IDOK) {
		m_list.SortItems(ListCompareProc, (LPARAM)&m_list);
	}
}

void CSettingsImportRules::OnBnClickedButtonRemove()
{
    if (MessageBox(_("Are you sure you want to delete the selected items?"), APP_NAME, MB_YESNO | MB_ICONQUESTION)==IDYES) {
		int nItem = m_list.GetNextItem(-1, LVNI_SELECTED);
		while (nItem!=-1) {
			delete (CString*)m_list.GetItemData(nItem);
			m_list.DeleteItem(nItem);
			nItem--;
			nItem = m_list.GetNextItem(nItem, LVNI_SELECTED);			
		}
	}
	BOOL bEnable = (m_list.GetNextItem(-1, LVNI_SELECTED)!=-1)?TRUE:FALSE;
	GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(bEnable);
}

void CSettingsImportRules::OnBnClickedButtonUpdateDatabase()
{
	((CMainFrame*)::theApp.m_pMainWnd)->LoadOutlookContacts(NULL);
	CLoadingContactsDlg dlg;
	dlg.DoModal();
}

void CSettingsImportRules::OnLvnOdstatechangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVODSTATECHANGE pStateChanged = reinterpret_cast<LPNMLVODSTATECHANGE>(pNMHDR);
	// TODO: Add your control notification handler code here
	BOOL bEnable = (m_list.GetNextItem(-1, LVNI_SELECTED)!=-1)?TRUE:FALSE;
	GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(bEnable);

	*pResult = 0;
}

void CSettingsImportRules::OnNMClickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	BOOL bEnable = (m_list.GetNextItem(-1, LVNI_SELECTED)!=-1)?TRUE:FALSE;
	GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(bEnable);
	*pResult = 0;
}

void CSettingsImportRules::OnNMSetfocusList(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	BOOL bEnable = (m_list.GetNextItem(-1, LVNI_SELECTED)!=-1)?TRUE:FALSE;
	GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(bEnable);
	*pResult = 0;
}

void CSettingsImportRules::OnDestroy()
{
	CDialog::OnDestroy();

	for (int i=0; i<m_list.GetItemCount(); i++)
		delete (CString*)m_list.GetItemData(i);
}

void CSettingsImportRules::InitLocaleGUI() {
	GetDlgItem(IDC_STATIC_GROUP)->SetWindowText(_("Outlook Import Rules"));
    GetDlgItem(IDC_BUTTON_ADD)->SetWindowText(_("&Add"));
	GetDlgItem(IDC_BUTTON_REMOVE)->SetWindowText(_("&Remove"));
	GetDlgItem(IDC_BUTTON_EDIT)->SetWindowText(_("&Edit"));
	GetDlgItem(IDC_BUTTON_UPDATE_DATABASE)->SetWindowText(_("&Update Database"));
	CString note;
	note.Format(_("NOTE: The following rules apply for Outlook Contacts' phone numbers, and for Outgoing Calls. " \
						 "If you change some of the rules, do not forget to update %s database."), CString(APP_NAME));
	GetDlgItem(IDC_STATIC_DESCRIPTION)->SetWindowText(note);
}
