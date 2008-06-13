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

// SettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PBXClient.h"
#include "SettingsDlg.h"
#include "MainFrm.h"

#include <atlbase.h>

#include "base64.h"

#include "stdafx.h"
#include <Windows.h>
#include <Assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg dialog


CSettingsDlg::CSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingsDlg::IDD, pParent)	
{
	//{{AFX_DATA_INIT(CSettingsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_LINK_BS, m_bsLink);	
	DDX_Control(pDX, IDC_TAB, m_tab);
}


BEGIN_MESSAGE_MAP(CSettingsDlg, CDialog)
	ON_BN_CLICKED(IDAPPLY, OnBnClickedApply)	
	ON_STN_CLICKED(IDC_STATIC_LINK_BS, OnStnClickedStaticLinkBs)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, OnTcnSelchangeTab)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg message handlers

BOOL CSettingsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();	
	
	// TODO: Add extra initialization here	

	m_dlgServer.Create(IDD_DIALOG_SETTINGS_SERVER, &m_tab);
	m_dlgGeneral.Create(IDD_DIALOG_SETTINGS_GENERAL, &m_tab);
	m_dlgExtensions.Create(IDD_DIALOG_SETTINGS_EXTENSIONS, &m_tab);
	m_dlgOutlook.Create(IDD_DIALOG_SETTINGS_OUTLOOK, &m_tab);
	m_dlgDR.Create(IDD_DIALOG_SETTINGS_DIALING_RULES, &m_tab);
	m_dlgImportRules.Create(IDD_DIALOG_SETTINGS_IMPORT_RULES, &m_tab);


	m_dlgServer.SetParent(&m_tab);
	m_dlgGeneral.SetParent(&m_tab);
	m_dlgExtensions.SetParent(&m_tab);
	m_dlgOutlook.SetParent(&m_tab);
	m_dlgDR.SetParent(&m_tab);
	m_dlgImportRules.SetParent(&m_tab);

	int x = 2;
	int y = 22;
	COLORREF cr = m_tab.GetDC()->GetBkColor();
	

	m_dlgServer.SetWindowPos(&this->wndNoTopMost, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	m_dlgGeneral.SetWindowPos(&this->wndNoTopMost, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	m_dlgExtensions.SetWindowPos(&this->wndNoTopMost, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	m_dlgOutlook.SetWindowPos(&this->wndNoTopMost, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	m_dlgDR.SetWindowPos(&this->wndNoTopMost, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	m_dlgImportRules.SetWindowPos(&this->wndNoTopMost, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	m_dlgGeneral.ShowWindow(1);
	
	m_tab.InsertItem(0, _("General"));
	m_tab.InsertItem(1, _("Dialing rules"));
	m_tab.InsertItem(2, _("Server"));
	m_tab.InsertItem(3, _("Extensions"));
	m_tab.InsertItem(4, _("Outlook"));
	m_tab.InsertItem(5, _("Outlook Import Rules"));


	m_dlgGeneral.LoadSettings();
	m_dlgServer.LoadSettings();
	m_dlgExtensions.LoadSettings();
	m_dlgOutlook.LoadSettings();
	m_dlgDR.LoadSettings();
	m_dlgImportRules.LoadSettings();
	
	m_bsLink.SetWindowText(BRAND_POWERED_BY);	
	m_bsLink.SetTextColor(RGB(0,0,255));
	m_bsLink.SetCursor(LoadCursor(NULL, IDC_HAND));

	m_dlgServer.GetDlgItemText(IDC_EDIT_SERVER, old_server);
	m_dlgServer.GetDlgItemText(IDC_EDIT_USERNAME, old_username);
	m_dlgServer.GetDlgItemText(IDC_EDIT_PASSWORD, old_password);
	m_dlgServer.GetDlgItemText(IDC_EDIT_PORT, old_port);
	
	SetWindowPos(&this->wndNoTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

	InitLocaleGUI();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSettingsDlg::OnOK() 
{
	// TODO: Add extra validation here
	m_bSettingsSaved = false;
	SaveSettings();
	if (m_bSettingsSaved==false)
		return;

	/*for (int i=0; i<m_dlgExtensions.m_list.GetItemCount(); i++) {
		delete (CString*)(m_dlgExtensions.m_list.GetItemData(i));
	}*/
		
	CDialog::OnOK();
}


void CSettingsDlg::UpdateRegistryExtensions() {
	HKEY hKey = HKEY_CURRENT_USER;
	CString sKeyName = REG_KEY_EXTENSIONS;
	LONG retcode;
	HKEY hOpenKey = NULL;
	DWORD dwType = REG_SZ;
	TCHAR str[MAX_REG_KEY_NAME];
	memset( str, '\0', sizeof(str));
	
	retcode = RegOpenKeyEx(hKey, (LPCTSTR)sKeyName, 0, KEY_ALL_ACCESS, &hOpenKey);
	
	if( retcode != (DWORD)ERROR_SUCCESS ) {
		if (RegCreateKey(hKey, sKeyName, &hOpenKey) != ERROR_SUCCESS) {			
			return;
		}
	}

	BYTE *data;
	data = new BYTE[100];
	memset( data, '\0', sizeof(data));

	DWORD Size;
	DWORD dwNo = 0;


	for (int i = 0, retCode = ERROR_SUCCESS; retCode == ERROR_SUCCESS; i++) 
    {		
		Size = MAX_REG_KEY_NAME;
		DWORD dwNo = 100;
		DWORD value;
        
		retCode = RegEnumValue(hOpenKey,i,str,&Size,NULL,&dwType,data,&dwNo);

		if (retCode != (DWORD) ERROR_NO_MORE_ITEMS)// && retCode != ERROR_INSUFFICIENT_BUFFER)
		{
			RegDeleteValue(hOpenKey, str);
            i--;						
			retCode = ERROR_SUCCESS;
        }
    }

	if(hKey)
		RegCloseKey( hOpenKey );
	
	delete[] data;

	CString extension, szProtocol;
	CListCtrl &m_list = m_dlgExtensions.m_list;
	int protocol;
	for (int i=0; i<m_list.GetItemCount(); i++) {
		extension = m_list.GetItemText(i, 0) + "#####" + *((CString*)m_list.GetItemData(i));
		szProtocol = m_list.GetItemText(i, 1);
		if (szProtocol=="SIP")
			protocol=0;
		else if (szProtocol=="IAX")
			protocol=1;
		else if (szProtocol=="SCCP")
			protocol=2;
		AfxGetApp()->WriteProfileInt(_T("Extensions"), extension, protocol);
	}
}

void CSettingsDlg::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class
	if (m_dlgGeneral.IsDlgButtonChecked(IDC_CHECK_SHOW_AT_STARTUP))
		::theApp.WriteProfileInt("Settings", "ShowSettingsOnStartup", 0);
	else
		::theApp.WriteProfileInt("Settings", "ShowSettingsOnStartup", 1);

	/*for (int i=0; i<m_dlgExtensions.m_list.GetItemCount(); i++) {
		delete (CString*)(m_dlgExtensions.m_list.GetItemData(i));
	}*/
    
	CDialog::OnCancel();
}


void CSettingsDlg::SaveSettings() {	    
	CPBXClientApp* pApp = (CPBXClientApp*)AfxGetApp();
	CString value;	

	CMainFrame *mainFrame = ((CMainFrame*)::theApp.m_pMainWnd);

	if(pApp)
	{
		/*********************		SERVER SETTINGS		***********************/

		m_dlgServer.GetDlgItemText(IDC_EDIT_SERVER, value);
		if (value=="") {
			MessageBox(_("Invalid value for server!"), APP_NAME, MB_OK | MB_ICONEXCLAMATION);
			m_tab.SetCurSel(2);
			OnTabSelectionChanged();
			m_dlgServer.GetDlgItem(IDC_EDIT_SERVER)->SetFocus();
			return;
		}
		pApp->WriteProfileString("Settings", "server", value);

		m_dlgServer.GetDlgItemText(IDC_EDIT_PORT, value);
		if (value=="") {
			MessageBox(_("Invalid value for port!"), APP_NAME, MB_OK | MB_ICONEXCLAMATION);
			m_tab.SetCurSel(2);
			OnTabSelectionChanged();
            m_dlgServer.GetDlgItem(IDC_EDIT_PORT)->SetFocus();
			return;
		}
		pApp->WriteProfileString("Settings", "port", value);

		m_dlgServer.GetDlgItemText(IDC_EDIT_USERNAME, value);
		if (value=="") {
			MessageBox(_("Invalid username!"), APP_NAME, MB_OK | MB_ICONEXCLAMATION);
			m_tab.SetCurSel(2);
			OnTabSelectionChanged();
			m_dlgServer.GetDlgItem(IDC_EDIT_USERNAME)->SetFocus();
			return;
		}
		pApp->WriteProfileString("Settings", "username", value);

		m_dlgServer.GetDlgItemText(IDC_EDIT_PASSWORD, value);
		pApp->WriteProfileString("Settings", "password", ::theApp.EncodePassword(value));


		/*********************		DIALING RULES		***********************/
		m_dlgDR.SaveSettings();
		
		
		/*********************		EXTENSIONS		***********************/

		UpdateRegistryExtensions();

		

		/*********************		GENERAL SETTINGS		***********************/

		m_dlgGeneral.GetDlgItemText(IDC_EDIT_DELAY_TIME, value);
		if (value=="") {
			MessageBox(_("Invalid delay time!"), APP_NAME, MB_OK | MB_ICONEXCLAMATION);
			return;
		}		
		pApp->WriteProfileString("Settings", "DelayTime", value);		
		
		CRegKey regKey;
		int nIndex;
		regKey.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"));
		
		if (m_dlgGeneral.IsDlgButtonChecked(IDC_CHECK_AUTOMATIC_START)) {
			pApp->WriteProfileInt("Settings", "AutomaticStart", 1);
			value = ::theApp.GetInstallDir() + "\\" + CString(APP_NAME) +".exe background";			
			regKey.SetValue(value, CString(APP_NAME));
		} else {
			pApp->WriteProfileInt("Settings", "AutomaticStart", 0);
			regKey.DeleteValue(CString(APP_NAME));
		}
		regKey.Close();

		if (m_dlgGeneral.IsDlgButtonChecked(IDC_CHECK_AUTOMATIC_CONNECT))
			pApp->WriteProfileInt("Settings", "AutomaticSignIn", 1);
		else
			pApp->WriteProfileInt("Settings", "AutomaticSignIn", 0);


		if (m_dlgGeneral.IsDlgButtonChecked(IDC_CHECK_SHOW_AT_STARTUP))
			pApp->WriteProfileInt("Settings", "ShowSettingsOnStartup", 0);
		else
			pApp->WriteProfileInt("Settings", "ShowSettingsOnStartup", 1);


		if (m_dlgGeneral.m_chkIgnore.GetCheck()) {
			CString digits;
			int iValue;
			m_dlgGeneral.m_cboDigits.GetWindowText(digits);
			if (digits!="") {
				iValue = _tstoi(digits.GetBuffer());
				pApp->WriteProfileInt("Settings", "IgnoreCalls", iValue);
			} else {
				//MessageBox(_("Invalid digits number!"), APP_NAME, MB_ICONINFORMATION);
				pApp->WriteProfileInt("Settings", "IgnoreCalls", 0);
			}
		} else {
            pApp->WriteProfileInt("Settings", "IgnoreCalls", 0);
		}
		

		if (m_dlgGeneral.SetLanguage()) {
            InitLocaleGUI();
		}


        /*********************		OUTLOOK SETTINGS		***********************/

		//CString OSEValue = "4.0;Outxxx.dll;7;00000000000000;0000000;OutXXX";
		bool bOIChanged = (m_dlgOutlook.m_bOutlookIntegrationOldValue!=m_dlgOutlook.IsDlgButtonChecked(IDC_CHECK_O_INTEGRATION));
		
		if (m_dlgOutlook.GetDlgItem(IDC_CHECK_O_INTEGRATION)->IsWindowEnabled() && bOIChanged) { // admin account is needed to enable/disable outlook integration
			if (m_dlgOutlook.IsDlgButtonChecked(IDC_CHECK_O_INTEGRATION)) {
				//char szValue[256];
				//DWORD nLen = 256;
				BOOL oiEnabled = FALSE; //outlook integration flag
				CString addinPath = ((CPBXClientApp*)AfxGetApp())->GetInstallDir() + "\\" + CString(APP_NAME) + ".dll";
				CString szAddinValue = "4.0;" + addinPath + ";1;11111111111111;1111111";			

				if (regKey.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Exchange\\Client\\Extensions"))==0) {
					/*if ((regKey.SetValue(szAddinValue, CString(APP_NAME))==0) &&
						(regKey.SetValue(OSEValue, "Outlook Setup Extension")==0)) {
							oiEnabled=TRUE;		
					}*/			
					
					if (regKey.SetValue(szAddinValue, CString(APP_NAME))==0) {
						oiEnabled=TRUE;
					}
					regKey.Close();
				}


				if (!oiEnabled) {
					MessageBox(_("Outlook integration failed!"), APP_NAME, MB_OK | MB_ICONERROR);
				}
			} else {
				if (regKey.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Exchange\\Client\\Extensions"))==0) {
					regKey.DeleteValue(CString(APP_NAME));
					//regKey.SetValue(OSEValue, "Outlook Setup Extension");
					regKey.Close();
				}
			}

			// Clear Outlook cache file to refresh plugins list
			TCHAR path[256];
			try {
				if (SUCCEEDED(SHGetFolderPath(NULL,CSIDL_LOCAL_APPDATA,NULL,0,path)))
					CFile::Remove(CString(path) + _T("\\Microsoft\\Outlook\\extend.dat"));
			} catch (...) {}		
		}


		if (m_dlgOutlook.IsDlgButtonChecked(IDC_CHECK_NEW_CONTACT_WINDOW))
			pApp->WriteProfileInt("Settings", "DisplayNewContactWindow", 1);
		else
			pApp->WriteProfileInt("Settings", "DisplayNewContactWindow", 0);

		if (m_dlgOutlook.IsDlgButtonChecked(IDC_CHECK_SEARCH_O_CONTACTS))
			pApp->WriteProfileInt("Settings", "UseOCDatabase", 1);
		else
			pApp->WriteProfileInt("Settings", "UseOCDatabase", 0);

		if (m_dlgOutlook.IsDlgButtonChecked(IDC_CHECK_ENABLE_O_FEATURES)) {
			pApp->WriteProfileInt("Settings", "OutlookFeatures", 1);			
		} else {
			pApp->WriteProfileInt("Settings", "OutlookFeatures", 0);
		}

		m_dlgOutlook.m_cboMailProfile.GetWindowText(value);
		pApp->WriteProfileString("Settings", "MailProfile", value);

		pApp->WriteProfileInt("Settings", "NNCallerID", m_dlgOutlook.m_chkNNCallerID.GetCheck()?1:0);
		pApp->WriteProfileInt("Settings", "FilterContacts", m_dlgOutlook.m_chkFilterContacts.GetCheck()?1:0);

		if (m_dlgOutlook.GetCheckedRadioButton(IDC_RADIO_FIRSTLAST, IDC_RADIO_LASTFIRST)==IDC_RADIO_FIRSTLAST)
			pApp->WriteProfileInt("Settings", "DisplaySettings", 1);
		else
			pApp->WriteProfileInt("Settings", "DisplaySettings", 2);		

		pApp->WriteProfileInt("Settings", "RefreshContactList", m_dlgOutlook.m_chkRefresh.GetCheck()?1:0);
		CString refreshInterval;
		m_dlgOutlook.m_editRefreshInterval.GetWindowText(refreshInterval);		
		if (refreshInterval!="") {
			pApp->WriteProfileString("Settings", "RefreshInterval", refreshInterval);
		}
		if ((m_dlgOutlook.m_chkRefresh.GetCheck()) && (m_dlgOutlook.IsDlgButtonChecked(IDC_CHECK_ENABLE_O_FEATURES))) {
			int interval = 30;
			if (refreshInterval!="") {
				interval = _tstoi(refreshInterval.GetBuffer());
				if (interval==0) {
					pApp->WriteProfileString("Settings", "RefreshInterval", "30");
					MessageBox(_("Invalid refresh interval specified! Program will use the default value of 30 minutes."), APP_NAME, MB_ICONINFORMATION);
					interval = 30;
				}
			}
			mainFrame->SetTimer(TIMER_REFRESH_CONTACTS, interval * 60000, NULL);
		} else {
			mainFrame->KillTimer(TIMER_REFRESH_CONTACTS);
		}
	}

	/*********************		OUTLOOK IMPORT RULES SETTINGS		***********************/
	CString rules;
	CListCtrl& list = m_dlgImportRules.m_list;
	for (int i=0; i<list.GetItemCount(); i++) {
		if (i==0)
			rules = list.GetItemText(i, 0);
		else
            rules += " || " + list.GetItemText(i, 0);
		
		rules += " || " + list.GetItemText(i, 1);
	}
	::theApp.WriteProfileString("Settings", "ImportOutlookRules", rules);

	
	CString new_server, new_username, new_password, new_port;
	m_dlgServer.GetDlgItemText(IDC_EDIT_SERVER, new_server);
	m_dlgServer.GetDlgItemText(IDC_EDIT_USERNAME, new_username);
	m_dlgServer.GetDlgItemText(IDC_EDIT_PASSWORD, new_password);
	m_dlgServer.GetDlgItemText(IDC_EDIT_PORT, new_port);

	bool bConnect = ((old_server!=new_server) || (old_username!=new_username) || (old_password!=new_password) || (old_port!=new_port) || ::theApp.m_bSignedIn==false);
	if (bConnect) {
		mainFrame->SignOut();		
        mainFrame->SetTimer(TIMER_SIGN_IN, 1000, NULL);
	}
	old_server=new_server;
	old_username=new_username;
	old_password=new_password;
	old_port=new_port;
	
	mainFrame->UpdateTrayIcon();

	m_bSettingsSaved = true;
}

void CSettingsDlg::OnBnClickedApply()
{
	SaveSettings();
}


void CSettingsDlg::OnStnClickedStaticLinkBs()
{
	ShellExecute(NULL, _T("open"), BRAND_WEB_PAGE, _T(""), 0, SW_SHOWNORMAL);
}

void CSettingsDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnTabSelectionChanged();

	*pResult = 0;
}


void CSettingsDlg::OnTabSelectionChanged() {
    int nTab = m_tab.GetCurSel();

	m_dlgGeneral.ShowWindow(nTab==0);
	m_dlgDR.ShowWindow(nTab==1);
	m_dlgServer.ShowWindow(nTab==2);
	m_dlgExtensions.ShowWindow(nTab==3);
	m_dlgOutlook.ShowWindow(nTab==4);
	m_dlgImportRules.ShowWindow(nTab==5);
}


void CSettingsDlg::InitLocaleGUI() {
	SetWindowText(CString(APP_NAME) + " - " + _("Settings"));
	GetDlgItem(IDOK)->SetWindowText(_("&OK"));
	GetDlgItem(IDCANCEL)->SetWindowText(_("&Cancel"));
	GetDlgItem(IDAPPLY)->SetWindowText(_("&Apply"));
	GetDlgItem(IDC_STATIC_POWERED_BY)->SetWindowText(_("Powered by:"));	

	m_dlgOutlook.InitLocaleGUI();
	m_dlgGeneral.InitLocaleGUI();
	m_dlgExtensions.InitLocaleGUI();
	m_dlgServer.InitLocaleGUI();
	m_dlgDR.InitLocaleGUI();
	m_dlgImportRules.InitLocaleGUI();

	CString tabText;
	TCITEM tcItem;
	tcItem.iImage=-1;
	tcItem.lParam=NULL;
	/*tcItem.dwState=NULL;
	tcItem.dwStateMask=NULL;*/	
	for (int i=0; i<m_tab.GetItemCount(); i++) {
		if (i==0)
			tabText = _("General");
		else if (i==1)
            tabText = _("Dialing rules");
		else if (i==2)
            tabText = _("Server");
		else if (i==3)
            tabText = _("Extensions");
		else if (i==4)
            tabText = _("Outlook");
		else if (i==5)
            tabText = _("Outlook Import Rules");

		tcItem.pszText = tabText.GetBuffer();
		tcItem.cchTextMax = tabText.GetLength();
		m_tab.SetItem(i, &tcItem);        
	}
	m_tab.Invalidate();
}
