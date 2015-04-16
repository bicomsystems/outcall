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

// SettingsExtensionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PBXClient.h"
#include "SettingsExtensionsDlg.h"
#include ".\settingsextensionsdlg.h"
#include "AddExtenDlg.h"


// CSettingsExtensionsDlg dialog

IMPLEMENT_DYNAMIC(CSettingsExtensionsDlg, CDialog)
CSettingsExtensionsDlg::CSettingsExtensionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingsExtensionsDlg::IDD, pParent)
{
}

CSettingsExtensionsDlg::~CSettingsExtensionsDlg()
{
}

void CSettingsExtensionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CSettingsExtensionsDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnBnClickedButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnBnClickedButtonEdit)
	ON_NOTIFY(LVN_ODSTATECHANGED, IDC_LIST, OnLvnOdstatechangedList)
	ON_NOTIFY(NM_SETFOCUS, IDC_LIST, OnNMSetfocusList)
	ON_NOTIFY(NM_CLICK, IDC_LIST, OnNMClickList)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CSettingsExtensionsDlg message handlers

void CSettingsExtensionsDlg::LoadSettings() {
	HKEY hKey = HKEY_CURRENT_USER;
	CString sKeyName = REG_KEY_EXTENSIONS;
	LONG retcode;
	HKEY hOpenKey = NULL;
	DWORD dwType = REG_SZ;
	TCHAR str[MAX_REG_KEY_NAME];
	memset( str, '\0', sizeof(str));
	CString userName, password, protocol;
	int nIndex, nItem;
	RECT rect;

	m_list.GetClientRect(&rect);
	if (AUTH_ENABLED) {
		m_list.InsertColumn(0, _("Extension"), LVCFMT_LEFT, rect.right*0.33);
		m_list.InsertColumn(1, _("Protocol"), LVCFMT_LEFT, rect.right*0.33);
		m_list.InsertColumn(2, _("Status"), LVCFMT_LEFT, rect.right*0.33);
	} else {
		m_list.InsertColumn(0, _("Extension"), LVCFMT_LEFT, rect.right*0.50);
		m_list.InsertColumn(1, _("Protocol"), LVCFMT_LEFT, rect.right*0.45);
	}

	LONG style = GetWindowLong(m_list.GetSafeHwnd(), GWL_STYLE);
	style |= (LVS_EX_FULLROWSELECT | LVS_REPORT | LVS_SHOWSELALWAYS);
	style &= ~LVS_SORTDESCENDING;
	style &= ~LVS_SORTASCENDING;
	SetWindowLong(m_list.GetSafeHwnd(), GWL_STYLE, style);
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);


	/************************ LOAD EXTENSIONS **************************/

	retcode = RegOpenKeyEx(hKey, (LPCTSTR)sKeyName, 0, KEY_READ, &hOpenKey);
	
	if( retcode != (DWORD)ERROR_SUCCESS )
		return;

	BYTE *data;
	data = new BYTE[100];
	memset( data, '\0', sizeof(data));

	DWORD Size;
	DWORD dwNo = 0;

#ifdef MULTI_TENANT
	for (int i = 0, retCode = ERROR_SUCCESS; retCode == ERROR_SUCCESS && i<1; i++) 
#else
	for (int i = 0, retCode = ERROR_SUCCESS; retCode == ERROR_SUCCESS; i++) 
#endif
    {
		Size = MAX_REG_KEY_NAME;
		DWORD dwNo = 100;
		DWORD value;
        
		retCode = RegEnumValue(hOpenKey,i,str,&Size,NULL,&dwType,data,&dwNo);

		if (retCode != (DWORD) ERROR_NO_MORE_ITEMS)// && retCode != ERROR_INSUFFICIENT_BUFFER)
		{
			memcpy(&value, data, sizeof(DWORD));
			userName = str;			
			if ((nIndex=userName.Find(_T("#####")))!=-1) {
				password = userName.Mid(nIndex+5);
				userName = userName.Left(nIndex);				
			} else {
				password = "";
			}

			nItem = m_list.InsertItem(m_list.GetItemCount(), userName);

			if (value==0) {
				m_list.SetItemText(nItem, 1, _T("SIP"));
				protocol = "sip";
			} else if (value==1) {			
				m_list.SetItemText(nItem, 1, _T("IAX"));
				protocol = "iax";
			} else if (value==2) {			
				m_list.SetItemText(nItem, 1, _T("SCCP"));
				protocol = "sccp";
			}

			if (AUTH_ENABLED) {
				if (::theApp.m_bSignedIn) {
					if (::theApp.CheckExtensionAuthorization(userName, password, protocol))
						m_list.SetItemText(nItem, 2, _T("Authorized"));
					else
						m_list.SetItemText(nItem, 2, _T("Not Authorized"));
				} else {
					m_list.SetItemText(nItem, 2, _T("Not Authorized"));
				}
			}

			m_list.SetItemData(nItem, (DWORD_PTR)(new CString(password)));
			
						
			retCode = ERROR_SUCCESS;
        }
    }

	if(hKey)
		RegCloseKey( hOpenKey );
	
	delete[] data;    
}

void CSettingsExtensionsDlg::OnBnClickedButtonAdd()
{
	if (::theApp.m_bSignedIn==false && AUTH_ENABLED) {
		AfxMessageBox(_("You must be Signed In to add new extensions."), MB_ICONINFORMATION);
		return;
	} else {
		CAddExtenDlg dlg;
		dlg.list = &m_list;
		dlg.DoModal();	
	}
}


void CSettingsExtensionsDlg::OnBnClickedButtonEdit()
{
	if (::theApp.m_bSignedIn==false && AUTH_ENABLED) {
		AfxMessageBox(_("You must be Signed In to edit extensions."), MB_ICONINFORMATION);
		return;
	} else {
		CAddExtenDlg dlg;
		dlg.list = &m_list;
		dlg.edit = TRUE;
		dlg.DoModal();
	}
}


void CSettingsExtensionsDlg::OnBnClickedButtonRemove()
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


void CSettingsExtensionsDlg::OnLvnOdstatechangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVODSTATECHANGE pStateChanged = reinterpret_cast<LPNMLVODSTATECHANGE>(pNMHDR);
	// TODO: Add your control notification handler code here
	BOOL bEnable = (m_list.GetNextItem(-1, LVNI_SELECTED)!=-1)?TRUE:FALSE;
	GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(bEnable);

	*pResult = 0;
}

void CSettingsExtensionsDlg::OnNMSetfocusList(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	BOOL bEnable = (m_list.GetNextItem(-1, LVNI_SELECTED)!=-1)?TRUE:FALSE;
	GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(bEnable);
	*pResult = 0;
}

void CSettingsExtensionsDlg::OnNMClickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	BOOL bEnable = (m_list.GetNextItem(-1, LVNI_SELECTED)!=-1)?TRUE:FALSE;
	GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(bEnable);
	*pResult = 0;
}


void CSettingsExtensionsDlg::InitLocaleGUI() {
	GetDlgItem(IDC_STATIC_GROUP)->SetWindowText(_("Extensions"));
    GetDlgItem(IDC_BUTTON_ADD)->SetWindowText(_("&Add"));
	GetDlgItem(IDC_BUTTON_REMOVE)->SetWindowText(_("&Remove"));
	GetDlgItem(IDC_BUTTON_EDIT)->SetWindowText(_("&Edit"));
}

void CSettingsExtensionsDlg::OnDestroy()
{
	CDialog::OnDestroy();
	
	for (int i=0; i<m_list.GetItemCount(); i++)
		delete (CString*)m_list.GetItemData(i);		
}
