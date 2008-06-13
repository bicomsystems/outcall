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

// SettingsOutlookDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PBXClient.h"
#include "SettingsOutlookDlg.h"
#include ".\settingsoutlookdlg.h"

#include "MainFrm.h"

#include <atlbase.h>


// CSettingsOutlookDlg dialog

IMPLEMENT_DYNAMIC(CSettingsOutlookDlg, CDialog)
CSettingsOutlookDlg::CSettingsOutlookDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingsOutlookDlg::IDD, pParent)
{
}

CSettingsOutlookDlg::~CSettingsOutlookDlg()
{
}

void CSettingsOutlookDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_REFRESH_INTERVAL, m_editRefreshInterval);
	DDX_Control(pDX, IDC_CHECK_REFRESH_CONTACTS, m_chkRefresh);	
	DDX_Control(pDX, IDC_CHECK_NN_CALLERID, m_chkNNCallerID);
	DDX_Control(pDX, IDC_CHECK_FILTER_CONTACTS, m_chkFilterContacts);
	DDX_Control(pDX, IDC_COMBO_MAIL_PROFILE, m_cboMailProfile);
}


BEGIN_MESSAGE_MAP(CSettingsOutlookDlg, CDialog)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_O_FEATURES, OnBnClickedCheckEnableOFeatures)
	ON_BN_CLICKED(IDC_CHECK_REFRESH_CONTACTS, OnBnClickedCheckRefreshContacts)
END_MESSAGE_MAP()


// CSettingsOutlookDlg message handlers

void CSettingsOutlookDlg::LoadSettings() {
	CRegKey regKey;
	TCHAR szValue[256];
	DWORD nLen = 256;
	CPBXClientApp* pApp = (CPBXClientApp*)AfxGetApp();
	int iValue;

	if (regKey.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Exchange\\Client\\Extensions"))==0) {
		if (regKey.QueryValue(szValue, CString(APP_NAME), &nLen)==0)
			this->CheckDlgButton(IDC_CHECK_O_INTEGRATION, 1);
		else
			this->CheckDlgButton(IDC_CHECK_O_INTEGRATION, 0);
		regKey.Close();
	} else { // this user is not admin
		GetDlgItem(IDC_CHECK_O_INTEGRATION)->EnableWindow(0);
		GetDlgItem(IDC_CHECK_O_INTEGRATION)->SetWindowText(_("Enable Outloo&k integration (Administrator privileges required to change this option)"));
		this->CheckDlgButton(IDC_CHECK_O_INTEGRATION, 1);
	}

	m_bOutlookIntegrationOldValue = IsDlgButtonChecked(IDC_CHECK_O_INTEGRATION);

	iValue = pApp->GetProfileInt("Settings", "OutlookFeatures", 0);
	this->CheckDlgButton(IDC_CHECK_ENABLE_O_FEATURES, (iValue==1)?1:0);

	iValue = pApp->GetProfileInt("Settings", "DisplayNewContactWindow", 1);
	this->CheckDlgButton(IDC_CHECK_NEW_CONTACT_WINDOW, (iValue==1)?1:0);

	iValue = pApp->GetProfileInt("Settings", "UseOCDatabase", 1);
	this->CheckDlgButton(IDC_CHECK_SEARCH_O_CONTACTS, (iValue==1)?1:0);

	iValue = pApp->GetProfileInt("Settings", "NNCallerID", 1);
	m_chkNNCallerID.SetCheck(iValue==1);

	iValue = pApp->GetProfileInt("Settings", "FilterContacts", 0);
	m_chkFilterContacts.SetCheck(iValue==1);

	iValue = pApp->GetProfileInt("Settings", "DisplaySettings", 1);		
    if (iValue==1)
		CheckRadioButton(IDC_RADIO_FIRSTLAST, IDC_RADIO_LASTFIRST, IDC_RADIO_FIRSTLAST);
	else
		CheckRadioButton(IDC_RADIO_FIRSTLAST, IDC_RADIO_LASTFIRST, IDC_RADIO_LASTFIRST);
	

	iValue = pApp->GetProfileInt("Settings", "RefreshContactList", 1);
	m_chkRefresh.SetCheck((iValue==1)?1:0);
	m_editRefreshInterval.SetWindowText(pApp->GetProfileString("Settings", "RefreshInterval", "30"));
	if (iValue==0)
		m_editRefreshInterval.EnableWindow(FALSE);

	iValue = pApp->GetProfileInt("Settings", "UseOCDatabase", 1);
	this->CheckDlgButton(IDC_CHECK_SEARCH_O_CONTACTS, (iValue==1)?1:0);

	vector<CString> MailProfiles;
	int nDefault=-1;
	((CMainFrame*)::theApp.m_pMainWnd)->LoadProfileDetails(MailProfiles, nDefault, true);
	CString CurrentMailProfile = pApp->GetProfileString("Settings", "MailProfile", "");

	if (MailProfiles.size()>0) {
        m_cboMailProfile.InsertString(0, MailProfiles[0]);
	}
	if (CurrentMailProfile!="" && m_cboMailProfile.FindString(0,CurrentMailProfile)==-1) {
		m_cboMailProfile.AddString(CurrentMailProfile);
		m_cboMailProfile.SelectString(0, CurrentMailProfile);
	} else {
		if (m_cboMailProfile.GetCount()>0)
			m_cboMailProfile.SetCurSel(0);
	}

    
	/*int nSelection=-1;
	for (int i = 0; i < MailProfiles.size(); i++) {
		m_cboMailProfile.AddString(MailProfiles[i]);
		if (MailProfiles[i]==CurrentMailProfile) {
			nSelection = i;
		}
	}
	if (nSelection==-1) {
		if (nDefault!=-1) {
			nSelection = nDefault;
		} else {
            nSelection = 0;
		}
	}
	if (m_cboMailProfile.GetCount()>0) {
		m_cboMailProfile.SetCurSel(nSelection);		
	}*/

	OnBnClickedCheckEnableOFeatures();
	/*if ( !IsDlgButtonChecked(IDC_CHECK_ENABLE_O_FEATURES) ) {			
		this->GetDlgItem(IDC_CHECK_NEW_CONTACT_WINDOW)->EnableWindow(0);			
		this->GetDlgItem(IDC_CHECK_SEARCH_O_CONTACTS)->EnableWindow(0);
		m_chkRefresh.EnableWindow(0);
		m_editRefreshInterval.EnableWindow(0);
		GetDlgItem(IDC_RADIO_FIRSTLAST)->EnableWindow(0);
		GetDlgItem(IDC_RADIO_LASTFIRST)->EnableWindow(0);
		m_chkFilterContacts.EnableWindow(0);
		m_chkNNCallerID.EnableWindow(0);
	}*/

}

void CSettingsOutlookDlg::OnBnClickedCheckEnableOFeatures()
{
	if (IsDlgButtonChecked(IDC_CHECK_ENABLE_O_FEATURES)) {
		GetDlgItem(IDC_CHECK_NEW_CONTACT_WINDOW)->EnableWindow(1);			
		GetDlgItem(IDC_CHECK_SEARCH_O_CONTACTS)->EnableWindow(1);
		m_chkRefresh.EnableWindow(1);
		m_editRefreshInterval.EnableWindow(1);
		GetDlgItem(IDC_RADIO_FIRSTLAST)->EnableWindow(1);
		GetDlgItem(IDC_RADIO_LASTFIRST)->EnableWindow(1);
		m_chkFilterContacts.EnableWindow(1);
		m_chkNNCallerID.EnableWindow(1);
		m_cboMailProfile.EnableWindow(1);
	} else {
		GetDlgItem(IDC_CHECK_NEW_CONTACT_WINDOW)->EnableWindow(0);			
		GetDlgItem(IDC_CHECK_SEARCH_O_CONTACTS)->EnableWindow(0);
		m_chkRefresh.EnableWindow(0);
		m_editRefreshInterval.EnableWindow(0);
		GetDlgItem(IDC_RADIO_FIRSTLAST)->EnableWindow(0);
		GetDlgItem(IDC_RADIO_LASTFIRST)->EnableWindow(0);
		m_chkFilterContacts.EnableWindow(0);
		m_chkNNCallerID.EnableWindow(0);
		m_cboMailProfile.EnableWindow(0);
	}
}

void CSettingsOutlookDlg::OnBnClickedCheckRefreshContacts()
{	
	m_editRefreshInterval.EnableWindow(m_chkRefresh.GetCheck());
}


void CSettingsOutlookDlg::InitLocaleGUI() {
	GetDlgItem(IDC_STATIC_OUTLOOK_SETTINGS)->SetWindowText(_("Outlook settings"));
	GetDlgItem(IDC_STATIC_CONTACT_DETAILS)->SetWindowText(_("Contact details"));
    GetDlgItem(IDC_CHECK_O_INTEGRATION)->SetWindowText(_("Enable Outloo&k integration"));
    GetDlgItem(IDC_CHECK_ENABLE_O_FEATURES)->SetWindowText(_("Enable Outlook &features"));
	GetDlgItem(IDC_CHECK_SEARCH_O_CONTACTS)->SetWindowText(_("&Search Outlook Contacts for incoming calls"));
	GetDlgItem(IDC_CHECK_NEW_CONTACT_WINDOW)->SetWindowText(_("Automatically display &New Contact Window for unknown incoming calls"));
    GetDlgItem(IDC_CHECK_REFRESH_CONTACTS)->SetWindowText(_("R&efresh contacts interval (minutes):"));
    GetDlgItem(IDC_CHECK_NN_CALLERID)->SetWindowText(_("Di&splay New Contact Window for non-numeric CallerID"));
	GetDlgItem(IDC_CHECK_FILTER_CONTACTS)->SetWindowText(_("Do not display contacts with no telephone num&bers defined"));
	GetDlgItem(IDC_STATIC_CONTACT_DETAILS)->SetWindowText(_("Contact details"));
    GetDlgItem(IDC_RADIO_FIRSTLAST)->SetWindowText(_("Fi&rst Name Middle Name, Last Name"));
	GetDlgItem(IDC_RADIO_LASTFIRST)->SetWindowText(_("&Last Name, First name Middle Name"));
	GetDlgItem(IDC_STATIC_MAIL_PROFILE)->SetWindowText(_("Mail Profile"));	
	GetDlgItem(IDC_STATIC_CHOOSE_MAIL_PROFILE)->SetWindowText(_("Choose Mail Profile:"));
}