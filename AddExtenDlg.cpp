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

// AddExtenDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PBXClient.h"
#include "AddExtenDlg.h"
#include ".\addextendlg.h"

#include "MainFrm.h"


// CAddExtenDlg dialog

IMPLEMENT_DYNAMIC(CAddExtenDlg, CDialog)
CAddExtenDlg::CAddExtenDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAddExtenDlg::IDD, pParent)
{
	edit = FALSE;
}

CAddExtenDlg::~CAddExtenDlg()
{
}

void CAddExtenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_EXTENSION, m_extension);
	DDX_Control(pDX, IDC_COMBO_PROTOCOL, m_protocol);
	DDX_Control(pDX, IDC_EDIT_PASSWORD, m_pass);
	DDX_Control(pDX, IDC_EDIT_PASSWORD2, m_pass1);
}


BEGIN_MESSAGE_MAP(CAddExtenDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CAddExtenDlg message handlers

BOOL CAddExtenDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_protocol.AddString(_T("SIP"));
    m_protocol.AddString(_T("IAX"));
	m_protocol.AddString(_T("SCCP"));

	if (edit) {
		CString number;		
		nSelected = list->GetNextItem(-1, LVNI_SELECTED);
		number = list->GetItemText(nSelected, 0);
		m_extension.SetWindowText(number);
		m_protocol.SelectString(-1, list->GetItemText(nSelected, 1));		
        m_pass.SetWindowText(*(CString*)(list->GetItemData(nSelected)));
		m_pass1.SetWindowText(*(CString*)(list->GetItemData(nSelected)));
	} else {
		m_protocol.SetCurSel(0);	
	}

	m_extension.SetFocus();
	m_extension.SetSel(0, -1);

	if (AUTH_ENABLED==0) {
		GetDlgItem(IDC_STATIC_PASSWORD)->ShowWindow(0);
		GetDlgItem(IDC_STATIC_PASSWORD2)->ShowWindow(0);
		m_pass.ShowWindow(0);
		m_pass1.ShowWindow(0);
		RECT rect, rect2;
		GetDlgItem(IDC_STATIC_PASSWORD)->GetWindowRect(&rect);
		ScreenToClient(&rect);
        GetDlgItem(IDOK)->GetWindowRect(&rect2);
        ScreenToClient(&rect2);
		GetDlgItem(IDOK)->SetWindowPos(&this->wndBottom, rect2.left, rect.top+10, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		GetDlgItem(IDCANCEL)->GetWindowRect(&rect2);
        ScreenToClient(&rect2);
		GetDlgItem(IDCANCEL)->SetWindowPos(&this->wndBottom, rect2.left, rect.top+10, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		GetWindowRect(&rect2);

		GetWindowRect(&rect);
		GetDlgItem(IDOK)->GetWindowRect(&rect2);
		SetWindowPos(&this->wndBottom, 0, 0, rect.right-rect.left, rect.bottom-rect.top-(rect.bottom-rect2.bottom)+15, SWP_NOMOVE | SWP_NOZORDER);
	}

	InitLocaleGUI();

    return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CAddExtenDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CString exten, protocol, pass, pass2;
	int nItem;
	m_extension.GetWindowText(exten);
	m_protocol.GetWindowText(protocol);

	if (exten=="") {
		MessageBox(_("Invalid phone number!"), APP_NAME, MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	if (AUTH_ENABLED) {
		m_pass.GetWindowText(pass);
		m_pass1.GetWindowText(pass2);

		if (pass!=pass2) {
			MessageBox(_("Password fields doesn't match!"), APP_NAME, MB_OK | MB_ICONEXCLAMATION);
			return;
		}


		if (::theApp.CheckExtensionAuthorization(exten, pass, protocol)==FALSE) {
			MessageBox(_("Either the extension or password is invalid. Please check the extension, password and protocol again!"), APP_NAME, MB_OK | MB_ICONEXCLAMATION);
			return;
		}
	}

	if (edit) {
		list->SetItem(nSelected, 0, LVIF_TEXT, exten, 0, 0, 0, 0);
		list->SetItem(nSelected, 1, LVIF_TEXT, protocol, 0, 0, 0, 0);
		if (AUTH_ENABLED) {
			list->SetItem(nSelected, 2, LVIF_TEXT, _T("Authorized"), 0, 0, 0, 0);
		}
		delete (CString*)(list->GetItemData(nSelected));
		list->SetItemData(nSelected, (DWORD_PTR)(new CString(pass)));
	} else {
		nItem = list->InsertItem(list->GetItemCount(), exten);
		list->SetItem(nItem, 1, LVIF_TEXT, protocol, 0, 0, 0, 0);
		if (AUTH_ENABLED) {
			list->SetItem(nItem, 2, LVIF_TEXT, _T("Authorized"), 0, 0, 0, 0);
		}
		list->SetItemData(nItem, (DWORD_PTR)(new CString(pass)));
	}
	
	OnOK();
}


void CAddExtenDlg::InitLocaleGUI() {
	if (edit)
		SetWindowText(_("Edit extension"));
	else
		SetWindowText(_("Add extension"));

	GetDlgItem(IDC_STATIC_EXTENSION)->SetWindowText(CString(_("&Extension")) + ":");
    GetDlgItem(IDC_STATIC_PROTOCOL)->SetWindowText(CString(_("&Protocol")) + ":");
	GetDlgItem(IDC_STATIC_PASSWORD)->SetWindowText(CString(_("Pass&word")) + ":");
	GetDlgItem(IDC_STATIC_PASSWORD2)->SetWindowText(CString(_("&Re-type password")) + ":");
	GetDlgItem(IDOK)->SetWindowText(_("&OK"));
	GetDlgItem(IDCANCEL)->SetWindowText(_("&Cancel"));
}
