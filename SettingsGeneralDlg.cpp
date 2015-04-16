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

// SettingsGeneralDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PBXClient.h"
#include "SettingsGeneralDlg.h"
#include "AddLanguageDlg.h"
#include ".\settingsgeneraldlg.h"


// CSettingsGeneralDlg dialog

using namespace std;


IMPLEMENT_DYNAMIC(CSettingsGeneralDlg, CDialog)
CSettingsGeneralDlg::CSettingsGeneralDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingsGeneralDlg::IDD, pParent)
{	
}

CSettingsGeneralDlg::~CSettingsGeneralDlg()
{
}

void CSettingsGeneralDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);	
	DDX_Control(pDX, IDC_CHECK_IGNORE_CALLS, m_chkIgnore);
	DDX_Control(pDX, IDC_COMBO_MIN_DIGITS, m_cboDigits);	
	DDX_Control(pDX, IDC_COMBO_LANGUAGE, m_cboLanguage);	
}


BEGIN_MESSAGE_MAP(CSettingsGeneralDlg, CDialog)
	ON_BN_CLICKED(IDC_CHECK_IGNORE_CALLS, OnBnClickedCheckIgnoreCalls)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_BUTTON_ADD_LANGUAGE, OnBnClickedButtonAddLanguage)
END_MESSAGE_MAP()


// CSettingsGeneralDlg message handlers

void CSettingsGeneralDlg::LoadSettings() {
    CPBXClientApp* pApp = (CPBXClientApp*)AfxGetApp();
	CString value;	
	int iValue;

	value = pApp->GetProfileString("Settings", "DelayTime", "3");
	this->SetDlgItemText(IDC_EDIT_DELAY_TIME, value);

	iValue = pApp->GetProfileInt("Settings", "AutomaticStart", 1);
	this->CheckDlgButton(IDC_CHECK_AUTOMATIC_START, (iValue==1)?1:0);

	iValue = pApp->GetProfileInt("Settings", "AutomaticSignIn", 1);
	this->CheckDlgButton(IDC_CHECK_AUTOMATIC_CONNECT, (iValue==1)?1:0);

	iValue = pApp->GetProfileInt("Settings", "ShowSettingsOnStartup", 0);
	this->CheckDlgButton(IDC_CHECK_SHOW_AT_STARTUP, (iValue==1)?0:1);			

	iValue = pApp->GetProfileInt("Settings", "IgnoreCalls", 0);
	m_chkIgnore.SetCheck((iValue==0)?FALSE:1);
	TCHAR chDigits[50];
	for (int i=2; i<26; i++) {
		_itot(i, chDigits, 10);
		m_cboDigits.AddString(chDigits);
	}		
	_itot(iValue, chDigits, 10);
	if (iValue==0) {
		m_cboDigits.EnableWindow(0);
		m_cboDigits.SetCurSel(0);
	} else {
		m_cboDigits.EnableWindow(1);
		m_cboDigits.SelectString(-1, chDigits);
	}	
	
	FILE *file = _tfopen(::theApp.GetInstallDir()+"\\languages.txt", _T("r"));
	if (file) {		
		CString str;
		int pos;
		char ch[100];
		while (!feof(file)) {
			str = fgets(ch, 100, file);
			if (str.GetLength()<4) {
                continue;
			}
			pos = str.Find(_T(" "));
			languages[str.Left(pos)] = str.Mid(pos+1, str.GetLength()-pos-3);			
		}
		fclose(file);
	}

	WIN32_FIND_DATA FileData;
	HANDLE hFind;
	map<CString, CString>::iterator iter;

	CString CurrLanguage = pApp->GetProfileString("Settings", "Language", "");

	m_cboLanguage.AddString(CString("(") + _("Default language") + CString(")"));

	hFind = FindFirstFile(::theApp.GetInstallDir() + "\\lang\\*.*", &FileData);
	if (hFind != INVALID_HANDLE_VALUE) 
	{		
		while (true) {
			if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (_tcscmp(FileData.cFileName, _T(".")) && _tcscmp(FileData.cFileName, _T(".."))) {
					iter = languages.find(FileData.cFileName);
					if (iter!=languages.end())
						m_cboLanguage.AddString(iter->second);
					else
						m_cboLanguage.AddString(FileData.cFileName);
				}
			}
			if (!FindNextFile(hFind, &FileData))
				break;
		}
		
		FindClose(hFind);
	}

	if (CurrLanguage=="") {
		m_cboLanguage.SetCurSel(0);
	} else {
		iter = languages.find(CurrLanguage);
		if (iter!=languages.end()) {
			m_cboLanguage.SelectString(-1, iter->second);            
		} else {		
			if (m_cboLanguage.SelectString(-1, CurrLanguage)==CB_ERR) {
				m_cboLanguage.SetCurSel(0);
			}
		}
	}
}


BOOL CSettingsGeneralDlg::SetLanguage() {
	CString lang;

	if (m_cboLanguage.GetCurSel()==0) {
        lang = "";
	} else {
		m_cboLanguage.GetWindowText(lang);
	}

	if (::theApp.GetProfileString("Settings", "Language", "")==lang) {
        return false;
	}

	if (lang!="") {
		map<CString, CString>::iterator iter;
		for (iter=languages.begin(); iter!=languages.end(); iter++) {
			if (iter->second==lang) {
				lang = iter->first;
				break;
			}
		}	
	}
	
	::theApp.WriteProfileString("Settings", "Language", lang);
	::theApp.SetLanguage(lang);

	return true;
}


void CSettingsGeneralDlg::OnBnClickedCheckIgnoreCalls()
{
	// TODO: Add your control notification handler code here
	m_cboDigits.EnableWindow(m_chkIgnore.GetCheck());
}

void CSettingsGeneralDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialog::OnPaint() for painting messages
}

HBRUSH CSettingsGeneralDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	/*if (pWnd==this || nCtlColor==CTLCOLOR_STATIC) {		
		return m_brush;
	}*/

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}

int CSettingsGeneralDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_brush.CreateSolidBrush(GetParent()->GetDC()->GetBkColor());	

	return 0;
}


void CSettingsGeneralDlg::OnBnClickedButtonAddLanguage()
{
	vector<CString> langs;
	map<CString, CString>::iterator iter;

	for (iter=languages.begin(); iter!=languages.end(); iter++) {
		if (m_cboLanguage.FindString(-1, iter->second)==LB_ERR) {
			langs.push_back(iter->second);			
		}
	}

	CString NewLang, LanguageCode;
	CAddLanguageDlg dlg(this, langs, &NewLang);
	if (dlg.DoModal()!=IDOK) {
        return;        
	}

	CString LangFile = CString(GETTEXT_DOMAIN)+".mo";

	CFileDialog OpenDlg(1, 0, LangFile, OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, _T("Language Files (*.mo)|*.mo||"));

	if (OpenDlg.DoModal()!=IDOK) {
		return;
	}

	LanguageCode = NewLang;
	for (iter=languages.begin(); iter!=languages.end(); iter++) {
		if (iter->second==NewLang) {			
			LanguageCode = iter->first;
			break;
		}
	}

	BOOL bDirectoryCreated=FALSE;
	int err;

	CreateDirectory(::theApp.GetInstallDir() + _T("\\lang"), NULL);	
	err = GetLastError();
	
	if (err==ERROR_SUCCESS || err==ERROR_ALREADY_EXISTS) {
		CreateDirectory(::theApp.GetInstallDir() + _T("\\lang\\") + LanguageCode, NULL);	
		err = GetLastError();
		if (err==ERROR_SUCCESS || err==ERROR_ALREADY_EXISTS) {
			CreateDirectory(::theApp.GetInstallDir() + _T("\\lang\\") + LanguageCode + _T("\\LC_MESSAGES"), NULL);
            bDirectoryCreated = (err==ERROR_SUCCESS || err==ERROR_ALREADY_EXISTS);
		}
	}

	if (!bDirectoryCreated) {
		MessageBox(_("Language directory could not be created."), APP_NAME, MB_ICONERROR);
		return;
	}

	if (CopyFile(OpenDlg.GetPathName(), ::theApp.GetInstallDir() + _T("\\lang\\") + LanguageCode + 
		_T("\\LC_MESSAGES\\") + LangFile, false)==false) {
		MessageBox(_("Language file could not be copied."), APP_NAME, MB_ICONERROR);
		RemoveDirectory(::theApp.GetInstallDir() + _T("\\lang\\") + LanguageCode);
		return;
	} else {
		int pos = m_cboLanguage.AddString(NewLang);
		m_cboLanguage.SetCurSel(pos);
		MessageBox(_("Language was successfully added."), APP_NAME, MB_ICONINFORMATION);
	}
}


void CSettingsGeneralDlg::InitLocaleGUI() {
	CString str;

	GetDlgItem(IDC_STATIC_GROUP)->SetWindowText(_("General settings"));
    GetDlgItem(IDC_STATIC_POPUP_DELAY)->SetWindowText(_("Call notification dialog &delay (sec)"));

	str.Format(_("Start %s at Windows startup"), APP_NAME);
	GetDlgItem(IDC_CHECK_AUTOMATIC_START)->SetWindowText(str);
	GetDlgItem(IDC_CHECK_AUTOMATIC_CONNECT)->SetWindowText(_("Allow automatic Sign In when connected to the &Internet"));
	GetDlgItem(IDC_CHECK_IGNORE_CALLS)->SetWindowText(_("Minimum CallerID length for which to display popup notification:"));
	
	GetDlgItem(IDC_CHECK_SHOW_AT_STARTUP)->SetWindowText(_("Don't s&how this window at startup"));
	
	GetDlgItem(IDC_STATIC_LANGUAGE)->SetWindowText(_("Language") + CString(":"));	
	
	GetDlgItem(IDC_STATIC_FRAME_LANGUAGE)->SetWindowText(_("Language"));
	GetDlgItem(IDC_BUTTON_ADD_LANGUAGE)->SetWindowText(_("Add another language"));	
}
