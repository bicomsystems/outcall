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

// PBXClient.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "PBXClient.h"

#include "MainFrm.h"
#include "SettingsDlg.h"
#include "base64.h"

#include <atlbase.h>

#include "semaphore.h"

#include <stdio.h>
#include <stdlib.h>

#include "afxwin.h"
#include ".\pbxclient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define WSA_VERSION  MAKEWORD(2,0)
#define MAX_HOSTNAME 256
#define MAX_HOSTADDR 40

/////////////////////////////////////////////////////////////////////////////
// CPBXClientApp

extern TCHAR* ORIGINATE_FILE_MUTEX;

TCHAR OUTCALL_DB[256];
BOOL AUTH_ENABLED = 0;


int SplitString(CString text, CString token, vector<CString> &ret) {
	int index;
	int count = 0;

	ret.clear();

	if (text=="") {
		return 0;
	}

	while ((index=text.Find(token))!=-1) {
		count++;
		ret.push_back(text.Mid(0, index));
		text = text.Mid(index+token.GetLength());		
	}

	ret.push_back(text);
	return count+1;

	/*CString line;
	int nRet, nStart=0;	
	ret.clear();
	if (text.Find(token, 0)==-1) {
		ret.push_back(text);
		return 1;
	}
	while ((nRet=text.Find(token, nStart))!=-1) {
		line = text.Mid(nStart, nRet-nStart);
		ret.push_back(line);		
		nStart = nRet+token.GetLength();
	}
	if (nStart<text.GetLength()) {		
		ret.push_back(text.Mid(nStart));
	}
	return ret.size();*/
}

BEGIN_MESSAGE_MAP(CPBXClientApp, CWinApp)
	//{{AFX_MSG_MAP(CPBXClientApp)
	//ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_ABOUT, OnFileAbout)
	ON_COMMAND(ID_FILE_ONLINEHTML, OnFileOnlineHtml)
	ON_COMMAND(ID_HELP_ONLINEPDF, OnFileOnlinePdf)
	ON_COMMAND(ID_HELP_CONTACTSUPPORT, OnFileContactSupport)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPBXClientApp construction

CPBXClientApp::CPBXClientApp()
{	
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance	
	m_bLoadingContacts = FALSE;
	m_pDialForm = NULL;

	m_bSignedIn = false;
	
	m_build = OUTCALL_VERSION;


	TCHAR app_data_path[256];
	SHGetFolderPath(NULL,CSIDL_APPDATA,NULL,0,app_data_path);
	CreateDirectory(CString(app_data_path) + _T("\\") + CString(APP_NAME), NULL);
	_tcscpy(OUTCALL_DB, CString(app_data_path) + _T("\\") + CString(APP_NAME) + _T("\\") + CString(APP_NAME) + _T(".db"));
}


// Retrieve a string value from INI file or registry.
CString CPBXClientApp::GetProfileString(const char *lpszSection, const char *lpszEntry,	const char* lpszDefault) {
	return CWinApp::GetProfileString(CString(lpszSection), CString(lpszEntry), CString(lpszDefault));
}

// Sets a string value to INI file or registry.
BOOL CPBXClientApp::WriteProfileString(const char *lpszSection, const char *lpszEntry,	CString lpszValue) {
	return CWinApp::WriteProfileString(CString(lpszSection), CString(lpszEntry), lpszValue);
}

// Retrieve an integer value from INI file or registry.
UINT CPBXClientApp::GetProfileInt(const char *lpszSection, const char *lpszEntry, int nDefault) {
    return CWinApp::GetProfileInt(CString(lpszSection), CString(lpszEntry), nDefault);
}

// Sets an integer value to INI file or registry.
BOOL CPBXClientApp::WriteProfileInt(const char *lpszSection, const char *lpszEntry, int nValue) {
	return CWinApp::WriteProfileInt(CString(lpszSection), CString(lpszEntry), nValue);
}

//char *CPBXClientApp::ConvertFromUnicodeToMB(const wchar_t *in) {
//	int nLen = sizeof(char)*wcslen(in)*2+2;
//    char *chSQL = (char*)malloc(nLen);
//    if (wcstombs(chSQL, in, nLen)==-1) {
//		free(chSQL);
//        return NULL;
//	} else {
//        return chSQL;        
//	}
//}
//
//wchar_t *CPBXClientApp::ConvertFromMBToUnicode(const char *in) {
//	int nLen = sizeof(char)*strlen(in)+2;
//    wchar_t *wchSQL = (wchar_t*)malloc(nLen);
//    if (mbstowcs(wchSQL, in, strlen(in))==-1) {
//		free(wchSQL);
//        return NULL;
//	} else {
//        return wchSQL;        
//	}
//}

CString CPBXClientApp::EncodePassword(CString in) {
	std::string encoded;
	
	unsigned char *chInBuf;
#ifdef _UNICODE
	/*chIn = ConvertFromUnicodeToMB(in);
	if (chIn==NULL)
		return _T("");*/
	CStringA ansi(in);
	chInBuf = (unsigned char *)ansi.GetBuffer();	
	//free(chIn);
#else
	chInBuf = (unsigned char *)in.GetBuffer();	
#endif

	int length = strlen((char*)chInBuf);
	for (int i=0; i<length; i++) {
		chInBuf[i]+=10;
	}

	encoded = base64_encode(chInBuf, length);
	
	return CString(encoded.c_str());
}


CString CPBXClientApp::DecodePassword(CString in) {
	//char *chIn;
	std::string encoded;
	std::string decoded;
#ifdef _UNICODE
	/*chIn = ConvertFromUnicodeToMB(in);
	if (chIn==NULL)
		return _T("");*/
	CStringA ansi(in);
	encoded = ansi.GetBuffer();	
	//free(chIn);
#else
	encoded = in;	
#endif

	decoded = base64_decode(encoded);
	
	int length = decoded.length();	
	for (int i=0; i<decoded.length(); i++)
		decoded[i]-=10;
	
	return CString(decoded.c_str());
}


// This function is the place where Originate command for Asterisk manager is built (for the entire application)
CString CPBXClientApp::BuildOriginateCommand(CString channel, CString numberToDial, CString callerid, 
	CString additional_options, CString context) {

#ifdef MULTI_TENANT
	CString username = ::theApp.GetProfileString("Settings", "username", "");
	CString tenantPrefix = username.Left(3);
	callerid = username.Mid(3, username.GetLength());
	int protocol = ::theApp.GetProfileInt("Settings", "protocol", 0);
	if (protocol==0)
		channel = _T("SIP/") + username;
	else if (protocol==1)
		channel = _T("IAX2/") + username;
	else if (protocol==2)
		channel = _T("SCCP/") + username; 	

	/*** First check if we are authorized ***/
	if (AUTH_ENABLED) {
		CString ret = ((CMainFrame*)m_pMainWnd)->m_socketManager.FindCalledExtension(channel);
		if (ret=="") {
			MessageBox(NULL, _("You are not authorized to place calls from the selected extension."), APP_NAME, MB_ICONINFORMATION);
			return _T("");
		}
	}

	CString result;
	
	FormatOutgoingNumber(numberToDial);

	CString outgoing_prefix = GetProfileString("Settings", "OutgoingPrefix", "");
	if (outgoing_prefix!="")
		numberToDial = outgoing_prefix + numberToDial;

    result = "Action: Originate\r\n";
	result += "Channel: " + channel + "\r\n";
	result += "Exten: " + numberToDial + "\r\n";	
	result += "Context: t-" + tenantPrefix + "\r\n";
	result += "Priority: 1\r\n";
	result += "CallerID: " + callerid + "\r\n";
	if (additional_options!="")
		result += additional_options + "\r\n";
	result += "\r\n";

#else
	/*** First check if we are authorized ***/
	if (AUTH_ENABLED) {
		CString ret = ((CMainFrame*)m_pMainWnd)->m_socketManager.FindCalledExtension(channel);
		if (ret=="") {
			MessageBox(NULL, _("You are not authorized to place calls from the selected extension."), APP_NAME, MB_ICONINFORMATION);
			return _T("");
		}
	}

	CString result;
	if (context=="") {
		context = GetProfileString("Settings", "OutgoingContext", "default");
		if (context=="") {
			context="default";
		}
	}
	FormatOutgoingNumber(numberToDial);

	CString outgoing_prefix = GetProfileString("Settings", "OutgoingPrefix", "");
	if (outgoing_prefix!="")
		numberToDial = outgoing_prefix + numberToDial;

    result = "Action: Originate\r\n";
	result += "Channel: " + channel + "\r\n";
	result += "Exten: " + numberToDial + "\r\n";	
	result += "Context: " + context + "\r\n";	
	result += "Priority: 1\r\n";
	result += "CallerID: " + callerid + "\r\n";
	if (additional_options!="")
		result += additional_options + "\r\n";
	result += "\r\n";
#endif

	return result;
}

// This function formats the outgoing number acording to the rules defined under "Outlook Import Rules" tab in Settings dialog
void CPBXClientApp::FormatOutgoingNumber(CString &number) {
	CString prefix;
	std::vector<CString> rules;
	std::vector<CString> prefixes;
	std::vector<CString> replacements;	

	SplitString(::theApp.GetProfileString("Settings", "ImportOutlookRules", ""), " || ", rules);
	for (int i=0; i<rules.size(); i++) {
        if ((i%2)==0)
			prefixes.push_back(rules[i]);
		else
			replacements.push_back(rules[i]);
	}
	for (int i=0; i<prefixes.size(); i++) {
		prefix = prefixes[i];
		if (number.Left(prefix.GetLength())==prefix) {
			number = replacements[i] + number.Mid(prefix.GetLength());
			break;
		}
	}
	number.Replace(_T("-"), _T(""));
	number.Replace(_T("("), _T(""));
	number.Replace(_T(")"), _T(""));
	number.Replace(_T(" "), _T(""));
}


/////////////////////////////////////////////////////////////////////////////
// The one and only CPBXClientApp object

CPBXClientApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CPBXClientApp initialization


void CPBXClientApp::SetMenuItemText(CMenu *pMenu, UINT nID, CString text, bool bByPos) {
	MENUITEMINFO miInfo;
	miInfo.cbSize = sizeof(miInfo);
	miInfo.fMask = MIIM_TYPE;
	miInfo.fType = MFT_STRING;
	miInfo.dwTypeData = text.GetBuffer();
	pMenu->SetMenuItemInfo(nID, &miInfo, bByPos?MF_BYPOSITION:MF_BYCOMMAND);
}


void CPBXClientApp::SetLanguage(CString lang) {		
#ifdef _UNICODE
    gettext_putenv(CStringA("LANG="+lang));
    bindtextdomain (GETTEXT_DOMAIN, CStringA(GetInstallDir()+"\\lang"));	
	bind_textdomain_codeset(GETTEXT_DOMAIN, "UCS-2LE");
	/*char *ch = ConvertFromUnicodeToMB(CString("LANG=")+lang);
	if (ch) {
		gettext_putenv(ch);
        free(ch);
	} else {
		return;
	}		
	ch = ConvertFromUnicodeToMB(GetInstallDir()+"\\lang");
	if (ch) {
		bindtextdomain (GETTEXT_DOMAIN, ch);
		free(ch);
	} else {
        return;
	}*/
#else
	gettext_putenv(CString("LANG=")+lang);
    bindtextdomain (GETTEXT_DOMAIN, GetInstallDir()+"\\lang");
#endif
	//setlocale(LC_ALL, "DECIMAL");
	textdomain (GETTEXT_DOMAIN);	
}


bool CPBXClientApp::CheckForExistingDialWindow() {
    if (m_pDialForm!=NULL) {
		m_pDialForm->SetWindowPos(&CWnd::wndTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		m_pDialForm->SetForegroundWindow();
        m_pDialForm->SetFocus();
		m_pDialForm->SetWindowPos(&CWnd::wndNoTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		return true;
	} else {
		return false;
	}
}

BOOL CPBXClientApp::InitInstance()
{
	
	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(REG_KEY_APP);

	CString lang = GetProfileString("Settings", "Language", "");
	SetLanguage(lang);
	   
	CString cmdLine = this->m_lpCmdLine;

	HWND hwnd = FindWindow(NULL, _T("BSOC Main Application Window"));

	m_bShellOpenFile = FALSE;	
	m_bAllowCmdLineOriginate = 1;	
	m_bDeveloperVersion = TRUE;

	if (cmdLine.Left(9).MakeLower()=="originate") {
		Semaphore s(ORIGINATE_FILE_MUTEX);
		CStdioFile debugFile(::theApp.GetInstallDir()+"\\originate.tmp", CFile::modeCreate | CFile::modeWrite);
		debugFile.SeekToEnd();
		debugFile.WriteString(cmdLine);
		debugFile.Close();		
		return TRUE;
	}	

	if (hwnd!=NULL) { //only one instance of OutCALL can be run		
		CString str;
		str.Format(_("%s is already running."), APP_NAME);
		MessageBox(NULL, str, APP_NAME, MB_ICONINFORMATION | MB_OK);
		return FALSE;
	}

	WSADATA		WSAData = { 0 };
	if ( 0 != WSAStartup( WSA_VERSION, &WSAData ) )
	{
		// Tell the user that we could not find a usable
		// WinSock DLL.
		if ( LOBYTE( WSAData.wVersion ) != LOBYTE(WSA_VERSION) ||
			 HIBYTE( WSAData.wVersion ) != HIBYTE(WSA_VERSION) )
			 ::MessageBox(NULL, _("Incorrect version of WS2_32.dll found."), APP_NAME, MB_OK | MB_ICONERROR);

		WSACleanup( );
		return FALSE;
	}	
	

	if (_tcscmp(this->m_lpCmdLine, _T("background"))==0) {
		Sleep(10000);
	} else if (cmdLine!="") {
		m_bShellOpenFile = TRUE;
		ReadConfigFile(cmdLine);		
	}

	AfxOleInit();
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CMainFrame* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;

	pFrame->CreateDatabaseTable("RecivedCalls", "CREATE TABLE RecivedCalls(Date TEXT, Time TEXT, Callee TEXT, CallerID TEXT, Sec NUMERIC)");
	pFrame->CreateDatabaseTable("PlacedCalls", "CREATE TABLE PlacedCalls(Date TEXT, Time TEXT, Callee TEXT, CallerID TEXT, Sec NUMERIC)");
	pFrame->CreateDatabaseTable("MissedCalls", "CREATE TABLE MissedCalls(NewCall NUMERIC, Date TEXT, Time TEXT, Callee TEXT, CallerID TEXT, Sec NUMERIC)");
	pFrame->CreateDatabaseTable("Contacts", "CREATE TABLE Contacts(Title TEXT, FirstName TEXT, MiddleName TEXT, LastName TEXT, Suffix TEXT, CompanyName TEXT, Assistant TEXT, Business TEXT, Business2 TEXT, BusinessFax TEXT, Callback TEXT, Car TEXT, Company TEXT, Home TEXT, Home2 TEXT, HomeFax TEXT, ISDN TEXT, Mobile TEXT, Other TEXT, OtherFax TEXT, Pager TEXT, PrimaryTel TEXT, Radio TEXT, Telex TEXT, TTYTDD TEXT)");

	// create and load the frame with its resources
	int left = (GetSystemMetrics(SM_CXSCREEN)-500)/2;	
	int top = (GetSystemMetrics(SM_CYSCREEN)-500)/2;
	

	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | WS_MINIMIZE | FWS_ADDTOTITLE, NULL,
		NULL);
	pFrame->MoveWindow(left, top, left+500, top+500);


	// The one and only window has been initialized, so show and update it.
	pFrame->ShowWindow(SW_HIDE);
	pFrame->UpdateWindow();

	//pFrame->SetWindowPos(&pFrame->wndTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	this->m_pMainWnd = pFrame;	
		
	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object.

	IntegrateIntoOutlook();
		
	BOOL showSettingsDlg = (GetProfileInt("Settings", "ShowSettingsOnStartup", 0)==1?TRUE:FALSE);
	CSettingsDlg settingsDlg;
	
	if (showSettingsDlg || (pFrame->IsOutCALLConfigured()==false)) {
		settingsDlg.DoModal();		
	}

	if (GetProfileInt("Settings", "ContactsLoaded", 0)==0 && GetProfileInt("Settings", "OutlookFeatures", 1)==1) {
		pFrame->LoadOutlookContacts(NULL);		
	}

	return TRUE;
}


void CPBXClientApp::IntegrateIntoOutlook() {
	CRegKey regKey;
	DWORD value;

	// Check if Outlook is installed when OutCALL runs for the first time, and if so
	// write the appropriate registry keys

	if (regKey.Open(HKEY_CURRENT_USER, APP_REG_KEY)==ERROR_SUCCESS) {
		if (regKey.QueryValue(value, _T("TryOutlookIntegration"))!=ERROR_SUCCESS) {
			regKey.SetValue((DWORD)0, _T("TryOutlookIntegration"));
			regKey.Close();

			TCHAR path[256];			

			//WIN32_FIND_DATA FindFileData;
			//HANDLE hFind;

			SHGetFolderPath(NULL,CSIDL_LOCAL_APPDATA,NULL,0,path);
			//
			//hFind = FindFirstFile(CString(path) + "\\Microsoft\\Outlook\\outlook.pst", &FindFileData);
			//if (hFind == INVALID_HANDLE_VALUE) {
   //             AfxGetApp()->WriteProfileInt("Settings", "OutlookFeatures", 0);
			//	return; // outlook is not installed                    
			//} else {
   //             FindClose(hFind);
			//}

			if (regKey.Open(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Office\\Outlook"))!=ERROR_SUCCESS) {
                ::theApp.WriteProfileInt("Settings", "OutlookFeatures", 0);
				return; // outlook is not installed                    
			} else {
				regKey.Close();
			}

			try {
				CFile::Remove(CString(path) + "\\Microsoft\\Outlook\\extend.dat"); //this will refresh outlook cache for plugins (addins)
			} catch (...) {
			}

			CString addinPath = GetInstallDir() + "\\" + CString(APP_NAME) + ".dll";
			CString szAddinValue = "4.0;" + addinPath + ";1;11111111111111;1111111";

			if (regKey.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Exchange\\Client\\Extensions"))==0) {
				regKey.SetValue(szAddinValue, CString(APP_NAME));					
				regKey.Close();
			}	

			::theApp.WriteProfileInt("Settings", "OutlookFeatures", 1);
		} else {
			regKey.Close();			
		}
	}
}



void CPBXClientApp::ExitApplication() {
    OnAppExit();
}


BOOL CPBXClientApp::CheckExtensionAuthorization(CString username, CString pass, CString protocol) {
	BOOL bFound=FALSE;
	map<CString, CString> &m_sipUsers = ((CMainFrame*)m_pMainWnd)->m_socketManager.m_sipUsers;
	map<CString, CString> &m_iaxUsers = ((CMainFrame*)m_pMainWnd)->m_socketManager.m_iaxUsers;
	map<CString, CString> &m_sccpUsers = ((CMainFrame*)m_pMainWnd)->m_socketManager.m_sccpUsers;
	map<CString, CString>::iterator iter;

	if (protocol.MakeLower()=="sip") {
		for (iter = m_sipUsers.begin(); iter != m_sipUsers.end(); iter++) {
			if ((iter->first==username) && (iter->second==pass)) {
				bFound = TRUE;
				break;
			}
		}
	} else if (protocol.MakeLower()=="iax") {
		for (iter = m_iaxUsers.begin(); iter != m_iaxUsers.end(); iter++) {
			if ((iter->first==username) && (iter->second==pass)) {
				bFound = TRUE;
				break;
			}
		}
	} else if (protocol.MakeLower()=="sccp") {
		for (iter = m_sccpUsers.begin(); iter != m_sccpUsers.end(); iter++) {
			if ((iter->first==username) && (iter->second==pass)) {
				bFound = TRUE;
				break;
			}
		}
	}

	return bFound;
}



/////////////////////////////////////////////////////////////////////////////
// CPBXClientApp message handlers


void CPBXClientApp::OnFileAbout() 
{
	// TODO: Add your command handler code here	
	if (IsWindow(m_AboutDlg.m_hWnd)) {
		m_AboutDlg.InitLocaleGUI();
		m_AboutDlg.ShowWindow(1);
		m_AboutDlg.BringWindowToTop();
	} else {
		CString strWndClass=AfxRegisterWndClass(0,AfxGetApp()->LoadStandardCursor(IDC_ARROW),GetSysColorBrush(COLOR_WINDOW),NULL);	
		int w = 350;
		int h = 145;
		m_AboutDlg.CreateEx(0,strWndClass,NULL,WS_POPUPWINDOW | WS_BORDER | WS_CAPTION,0,0,w,h,NULL,NULL);		
		RECT rcDesktop;
		::SystemParametersInfo(SPI_GETWORKAREA,0,&rcDesktop,0);
		int nDesktopWidth=rcDesktop.right-rcDesktop.left;
		int nDesktopHeight=rcDesktop.bottom-rcDesktop.top;
		int x = (nDesktopWidth-w)/2;
		int y = (nDesktopHeight-h)/2;
		SetWindowPos(m_AboutDlg.m_hWnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		m_AboutDlg.ShowWindow(1);
	}	
}


void CPBXClientApp::OnFileOnlineHtml() {	
	ShellExecute(NULL, _T("open"), LINK_DOCS_HTML, _T(""), 0, SW_SHOWNORMAL);
}

void CPBXClientApp::OnFileOnlinePdf() {	
	ShellExecute(NULL, _T("open"), LINK_DOCS_PDF, _T(""), 0, SW_SHOWNORMAL);
}

void CPBXClientApp::OnFileContactSupport() 
{
	// TODO: Add your command handler code here
	MessageBox(NULL, _("Please contact your administrator for technical support."), APP_NAME, MB_ICONINFORMATION);
}


int CPBXClientApp::ExitInstance() 
{	
	// Terminate use of the WS2_32.DLL
	WSACleanup();
	
	return CWinApp::ExitInstance();
}


CString CPBXClientApp::GetInstallDir() {
	int nIndex;
	CString installDir = m_pszHelpFilePath;
	nIndex = installDir.ReverseFind('\\');
	installDir = installDir.Left(nIndex);
	return installDir;
}


void CPBXClientApp::ReadConfigFile(CString path) {
	
	CStdioFile *file;	
	TRY {
		file = new CStdioFile();
		if (!file->Open(path, CFile::modeRead)) {
			delete file;
			return;
		} else {
			CString line, key, value;
			vector<CString> extension;
			int nIndex;
			while (file->ReadString(line)) {
				nIndex = line.Find(_T("="));
				if (nIndex!=-1) {
					key = line.Left(nIndex);
					value = line.Mid(nIndex+1);
					if (key=="server")
						::theApp.WriteProfileString("Settings", "server", value);
					else if (key=="port")
						::theApp.WriteProfileString("Settings", "port", value);
					else if (key=="manager_username")
						::theApp.WriteProfileString("Settings", "username", value);
					else if (key=="manager_password") {
						////encode password first, then write to registry
						//int i;
						//int length = value.GetLength();
						//char *inBuf = value.GetBuffer(length);
						//for (i=0; i<length; i++) {
						//	inBuf[i]+=10;
						//}							
						//std::string encoded = base64_encode((unsigned char*)inBuf, length);
						::theApp.WriteProfileString("Settings", "password", ::theApp.EncodePassword(value));
					} else if (key=="extension") {
						if (SplitString(value, "|||", extension)>=3) {
							if (extension[0].MakeLower()=="sip")
								CWinApp::WriteProfileInt(_T("Extensions"), extension[1] + "#####" + extension[2], 0);
							else if (extension[0].MakeLower()=="iax")
								CWinApp::WriteProfileInt(_T("Extensions"), extension[1] + "#####" + extension[2], 1);
							else if (extension[0].MakeLower()=="sccp")
								CWinApp::WriteProfileInt(_T("Extensions"), extension[1] + "#####" + extension[2], 2);
						}						
					}
				}
			}							

			file->Close();
			delete file;			
		}		
	}	
	CATCH( CFileException, e )
	{		
		delete file;		
	}
	END_CATCH
}


CAboutDlg::CAboutDlg()
{	
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_STN_CLICKED(IDC_STATIC_BS_LINK, OnStnClickedStaticLinkBs)		
	ON_WM_PAINT()	
	ON_WM_CREATE()
	ON_BN_CLICKED(IDOK, OnOK)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	SetWindowPos(&this->wndNoTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);	

	RECT rect;
	rect.left = 20;
	rect.top = 10;
	rect.right = 40;
	rect.bottom = 40;

	this->SetWindowPos(&this->wndBottom, 0, 0, 100, 100,SWP_NOMOVE | SWP_NOSIZE);

	m_picture.Create(_T(""), WS_CHILD, rect, this, IDC_STATIC_PICTURE);
	m_picture.SetIcon(LoadIcon(NULL, MAKEINTRESOURCE(IDR_MAINFRAME)));
	m_picture.SetWindowPos(&this->wndBottom, 20, 10, 20, 20,SWP_NOZORDER);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}




void CAboutDlg::OnStnClickedStaticLinkBs()
{
	ShellExecute(NULL, _T("open"), BRAND_WEB_PAGE, _T(""), 0, SW_SHOWNORMAL);
}

int CAboutDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;


	LOGFONT lf;
	LOGFONT lf1;
	
	GetObject(GetStockObject(SYSTEM_FONT),sizeof(lf),&lf);

	cf.DeleteObject();
	
	cf.CreatePointFont(10 * 10, _T("Tahoma"));
	cf.GetLogFont(&lf1);
	
	lf.lfHeight=lf1.lfHeight;
	lf.lfWidth=lf1.lfWidth;	
	lf.lfWeight = FW_NORMAL;	
	_tcscpy(lf.lfFaceName, _T("Tahoma"));
	
	cf.DeleteObject();
	cf.CreateFontIndirect(&lf);
	this->SetFont(&cf);
	

	
	// TODO:  Add your specialized creation code here
	RECT rect;	

	this->SetIcon(LoadIcon(NULL, MAKEINTRESOURCE(IDR_MAINFRAME)), false);	

	HBITMAP hBmp =(HBITMAP)::LoadImage(AfxGetInstanceHandle(),
												   MAKEINTRESOURCE(IDB_BITMAP_OUTCALL),
												   IMAGE_BITMAP,
												   0,
												   0,
												   LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);

	m_picture.Create(_T("text"), WS_VISIBLE | WS_CHILD | SS_BITMAP | SS_CENTERIMAGE, CRect(0,13,50,35), this); // IDC_STATIC_PICTURE);
	m_picture.SetBitmap(hBmp);
	//m_picture.SetWindowPos(&this->wndBottom, 10, 10, 40, 40,SWP_NOZORDER);	
	m_picture.ShowWindow(1);	
	
	m_version.Create(_T(""), WS_CHILD, rect, this, IDC_STATIC_VERSION);	
	m_version.SetFont(lf);	
	m_version.SetWindowPos(&this->wndBottom, 45, 10, 320, 20, SWP_NOZORDER);	
	m_version.ShowWindow(1);	
	m_version.Invalidate();

	m_bsLink.Create(BRAND_COPYRIGHT, WS_CHILD, rect, this, IDC_STATIC_BS_LINK);	
	m_bsLink.ModifyStyle(0, SS_NOTIFY);
	m_bsLink.SetFont(lf);
	m_bsLink.SetWindowPos(&this->wndBottom, 45, 35, 320, 20, SWP_NOZORDER);	
	m_bsLink.ShowWindow(1);
	m_bsLink.UpdateWindow();
	
	m_bsLink.SetTextColor(RGB(0,0,255));
	m_bsLink.SetCursor(LoadCursor(NULL, IDC_HAND));
	
	m_btnOK.Create(_("&OK"), WS_CHILD, rect, this, IDOK);
	m_btnOK.SetWindowPos(&this->wndBottom, 220, 74, 100, 23, SWP_NOZORDER);
	m_btnOK.ShowWindow(1);
	m_btnOK.SetFont(&cf);
	
	m_version.ShowWindow(1);

	this->Invalidate();
	this->UpdateWindow();

	InitLocaleGUI();

	return 0;
}

void CAboutDlg::InitLocaleGUI() {
	char chBuild[10];
	CString str;
	str.Format(_("About %s..."), APP_NAME);
	SetWindowText(str);
	m_btnOK.SetWindowText(_("&OK"));	
	itoa(::theApp.m_build, chBuild, 10);
	CString sBuild = chBuild;
	if (sBuild.GetLength()<2)
		sBuild = "0" + sBuild;
		
#ifdef MULTI_TENANT
	m_version.SetWindowText(CString(APP_NAME) + ", " + _("Version") + " 1." + sBuild + " (MT Edition)");
#else
	m_version.SetWindowText(CString(APP_NAME) + ", " + _("Version") + " 1." + sBuild);
#endif

}


void CAboutDlg::OnOK() {
	DestroyWindow();
}

void CAboutDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialog::OnPaint() for painting messages
	dc.SetBkColor(RGB(0,0,0));
	RECT rect;
	GetClientRect(&rect);	
	dc.FillSolidRect(CRect(0,0,rect.right,rect.bottom), GetSysColor( COLOR_BTNFACE ));
}






