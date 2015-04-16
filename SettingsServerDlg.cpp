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

// SettingsServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PBXClient.h"
#include "SettingsServerDlg.h"

#include "base64.h"


// CSettingsServerDlg dialog

IMPLEMENT_DYNAMIC(CSettingsServerDlg, CDialog)
CSettingsServerDlg::CSettingsServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingsServerDlg::IDD, pParent)
{
}

CSettingsServerDlg::~CSettingsServerDlg()
{
}

void CSettingsServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_PROTOCOL, m_protocol);
}


BEGIN_MESSAGE_MAP(CSettingsServerDlg, CDialog)
END_MESSAGE_MAP()


// CSettingsServerDlg message handlers

void CSettingsServerDlg::LoadSettings() {
    CPBXClientApp* pApp = (CPBXClientApp*)AfxGetApp();
	CString value;

	//((CEdit*)GetDlgItem(IDC_EDIT_PASSWORD))->SetPasswordChar('*');

	value = pApp->GetProfileString("Settings", "server", "");
	this->SetDlgItemText(IDC_EDIT_SERVER, value);

	value = pApp->GetProfileString("Settings", "port", DEFAULT_PORT);
	this->SetDlgItemText(IDC_EDIT_PORT, value);
	
	value = pApp->GetProfileString("Settings", "username", "");		
	this->SetDlgItemText(IDC_EDIT_USERNAME, value);

	value = pApp->GetProfileString("Settings", "password", "");
	this->SetDlgItemText(IDC_EDIT_PASSWORD, ::theApp.DecodePassword(value));

#ifdef MULTI_TENANT
	m_protocol.AddString(_T("SIP"));
    m_protocol.AddString(_T("IAX"));
	//m_protocol.AddString(_T("SCCP"));
	int protocol = pApp->GetProfileInt("Settings", "protocol", 0);
	if (protocol>1) {
		pApp->WriteProfileInt("Settings", "protocol", 0);
		protocol=0;
	}
	m_protocol.SetCurSel(protocol);
#endif
}


//CString CSettingsServerDlg::EncodePassword(CString in) {
//	std::string encoded;
//	int i;
//	int length = in.GetLength();	
//	TCHAR *inBuf = in.GetBuffer(length);
//
//	for (i=0; i<length; i++) {
//		inBuf[i]+=10;
//	}
//
//	encoded = base64_encode((unsigned char*)inBuf, length);	
//
//	return CString(encoded.c_str());
//}
//
//
//CString CSettingsServerDlg::DecodePassword(CString in) {		
//	std::string encoded = (LPCTSTR)in;
//	std::string decoded = base64_decode(encoded);
//	int length = decoded.length();
//	
//	for (int i=0; i<decoded.length(); i++)
//		decoded[i]-=10;
//	
//	
//	return CString(decoded.c_str());
//}


void CSettingsServerDlg::InitLocaleGUI() {
	GetDlgItem(IDC_STATIC_GROUP)->SetWindowText(_("Server settings"));
    GetDlgItem(IDC_STATIC_SERVER)->SetWindowText(_("&Server") + CString(":"));

#ifdef MULTI_TENANT
	GetDlgItem(IDC_STATIC_USERNAME)->SetWindowText(_("&Extension") + CString(":"));
	GetDlgItem(IDC_STATIC_PASSWORD)->SetWindowText(_("&Secret") + CString(":"));
#else
	GetDlgItem(IDC_STATIC_USERNAME)->SetWindowText(_("&Username") + CString(":"));
	GetDlgItem(IDC_STATIC_PASSWORD)->SetWindowText(_("&Password") + CString(":"));	
#endif

	GetDlgItem(IDC_STATIC_PORT)->SetWindowText(_("&Port") + CString(":"));
	GetDlgItem(IDC_STATIC_PROTOCOL)->SetWindowText(CString(_("&Protocol")) + ":");

#ifndef MULTI_TENANT
	GetDlgItem(IDC_STATIC_PROTOCOL)->ShowWindow(FALSE);
	GetDlgItem(IDC_COMBO_PROTOCOL)->ShowWindow(FALSE);
#endif
}