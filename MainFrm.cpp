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

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "PBXClient.h"

//#include <atlconv.h>
#include "MainFrm.h"

#include "OutlookContactsDlg.h"
#include "MissedCallsDlg.h"
#include <iphlpapi.h>

#include "ActiveCallsDlg.h"
#include "FindContactDlg.h"

#include "semaphore.h"
#include <afxinet.h>

#include "MapContactFieldsDlg.h"

#include "MapiEx.h"
#include <atlbase.h>

#pragma comment(lib, "iphlpapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//char* DATABASE_ACCESS_MUTEX = "Database Access Mutex";
TCHAR* ORIGINATE_FILE_MUTEX = _T("Originate mutex");

void LoadFolder(MAPIFolderPtr pFolder, CppSQLite3DB *db);

CMAPIEx gMapiEx;
CMainFrame *g_pMainWindow = NULL;


/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_COMMAND(ID_FILE_SETTINGS, OnFileSettings)
	ON_COMMAND(ID_FILE_CONNECT, OnFileConnect)
	ON_COMMAND(ID_FILE_CALL, OnFileCall)
	ON_COMMAND(ID_FILE_ACTIVECALLS, OnFileActiveCalls)
	ON_COMMAND(ID_FILE_ADDCONTACT, OnFileAddContact)
	ON_COMMAND(ID_FILE_FIND, OnFileFind)	
	ON_COMMAND(ID_FILE_ASTERISKDEBUGINFO, OnFileDebugInfo)
	ON_COMMAND(ID_FILE_MISSEDCALLS, OnFileMissedCalls)
	
	ON_COMMAND(ID_REFRESHCONTACTS_IMPORTFROMOUTLOOK, OnFileRefreshContactsFromOutlook)
	ON_COMMAND(ID_REFRESHCONTACTS_IMPORTFROMCSVFILE, OnFileRefreshContactsFromCSV)	

	ON_UPDATE_COMMAND_UI(ID_FILE_CONNECT, OnUpdateFileConnect)
	ON_UPDATE_COMMAND_UI(ID_FILE_CALL, OnUpdateFileCall)
	ON_UPDATE_COMMAND_UI(ID_FILE_ADDCONTACT, OnUpdateFileAddContact)

	ON_UPDATE_COMMAND_UI(ID_REFRESHCONTACTS_IMPORTFROMOUTLOOK, OnUpdateFileAddContact)
	ON_UPDATE_COMMAND_UI(ID_REFRESHCONTACTS_IMPORTFROMCSVFILE, OnUpdateFileAddContact)
	
	ON_UPDATE_COMMAND_UI(ID_FILE_FIND, OnUpdateFileFindContact)	
	ON_WM_COPYDATA()
	ON_WM_SETFOCUS()
	ON_MESSAGE(WM_ICON_NOTIFY, OnTrayNotification)
	ON_MESSAGE(OUTLOOK_MESSAGE, OnShowDialDialog)
	ON_MESSAGE(STOP_COMM_MESSAGE, OnStopSocketCommunication)
	ON_MESSAGE(WM_TASKBARNOTIFIERCLICKED, OnCallNotifyDialogClick)
	ON_MESSAGE(NEW_CALL_MESSAGE, OnNewCallMsg)
	ON_MESSAGE(WM_AUTHENTICATE, OnAuthenticateMsg)
	ON_MESSAGE(WM_RELOAD_OUTLOOK_CONTACTS, OnReloadOutlookContactsMsg)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	g_pMainWindow = this;
	m_bSignOutMessageBoxActive = FALSE;
	m_socketManager.authenticated = FALSE;
	//m_SignInMenuItemData = 0;
	pOutlookApp = NULL;

	//wchar_t *wch
	/*char *ch = gettext("Help");
	wchar_t *wch = (TCHAR*)ch;
	CString s(gettext("Help"));
	ch=ch;*/
}

CMainFrame::~CMainFrame()
{

}


void CMainFrame::CreateDatabaseTable(CString table, CString fields) {
	CppSQLite3DB db;
	CppSQLite3Query q;    
	db.open(CStringA(OUTCALL_DB));

	CString create_table_string = fields;
	create_table_string.MakeLower();	
	table.MakeLower();		

	try {
		q = db.execQuery("select * from sqlite_master");

		while (!q.eof()) {
			if (CString(q.fieldValue(1)).MakeLower()==table) {
				if (CString(q.fieldValue(4)).MakeLower()!=create_table_string) {
					db.execDML(CString("drop table " + table).GetBuffer());
				}
				break;
			}
			q.nextRow();
		}
		
		db.execDML(fields.GetBuffer());	
	} catch (CppSQLite3Exception& e) { }
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

#ifdef ENABLE_DBGLOG
	InitializeFileLogger();	
#endif	

	DBG_LOG("----------------- OutCALL Started --------------------");	

	HICON hIcon = ::theApp.LoadIcon(IDR_MAINFRAME);
	SetIcon(hIcon, TRUE);
	SetIcon(hIcon, FALSE);
	
	Init();
	this->SetWindowText(_T("BSOC Main Application Window"));
	
	DisplayWindow(FALSE);

	m_bSigningIn = FALSE;
	m_bCancelSignIn = FALSE;
	m_bManualSignInPerformed = FALSE;

	if (((::theApp.GetProfileInt("Settings", "AutomaticSignIn", 1)==1) || (::theApp.m_bShellOpenFile)) 
		&& (IsOutCALLConfigured())) {
		SignIn(TRUE);
	}	
	
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	if(cs.hMenu)
		DestroyMenu(cs.hMenu);
	cs.hMenu	 = NULL;

	cs.style	 = WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_OVERLAPPED;
	cs.dwExStyle = WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_TOOLWINDOW | WS_EX_TOPMOST;

	cs.x		 = GetSystemMetrics(SM_CXMAXIMIZED) - (CF_WINDOW_WIDTH +  30);
	cs.y		 = GetSystemMetrics(SM_CYMAXIMIZED) - (CF_WINDOW_HEIGHT + 30);
	cs.cx		 = CF_WINDOW_WIDTH;
	cs.cy		 = CF_WINDOW_HEIGHT;

	cs.lpszClass = AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW,
      ::LoadCursor(NULL, IDC_ARROW),
      (HBRUSH) ::GetStockObject(HOLLOW_BRUSH),
      NULL);

	return TRUE;
}	

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	// if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
	//	  return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


LRESULT CMainFrame::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
	return m_TrayIcon.OnSysTrayNotification(wParam, lParam);
}



void CMainFrame::OnDestroy() 
{
	CFrameWnd::OnDestroy();
	if ((m_socketManager.IsOpen()) && (m_socketManager.authenticated)) {		
		CString packetString="Action: Logoff";
		m_socketManager.SendData(packetString);
	}	
	m_socketManager.StopComm();	
}


void CMainFrame::OnPaint() 
{
	CPaintDC dc(this); // device context for painting	
}


BOOL CMainFrame::Init()
{	
	// Create the Tray Icon.
	UINT iconID;
	CString toolTipText;

	/****** Allow outcall to use Outlook MAPI ****/
	CRegKey regKey;
	if (regKey.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows Messaging Subsystem\\MSMapiApps"))==0) {
		regKey.SetValue(_T("Microsoft Outlook"), _T("outcall.exe"));
		regKey.Close();
	}

	if (IsOutCALLConfigured()) {
		iconID = IDI_ICON_DISCONNECTED;
		toolTipText = CString(APP_NAME) + " - Not Signed In";
	} else {
		iconID = IDI_ICON_RED;
		toolTipText = CString(APP_NAME) + " - Not Configured";
	}

	m_TrayIcon.Create(this, 
		WM_ICON_NOTIFY, 
		toolTipText, 
		AfxGetApp()->LoadIcon(iconID),
		IDR_MAINFRAME);
	m_TrayIcon.iconResourceID = iconID;
	
	
	// Create the Brushes
	m_BrushToolTip.CreateSolidBrush(GetSysColor(COLOR_INFOBK));
	m_BrushShadow.CreateSolidBrush(GetSysColor(COLOR_GRAYTEXT));

	m_DebugDlg.Create(IDD_DIALOG_DEBUG, this);	
	
	// Initialize socket manager
	m_socketManager.SetServerState( false );	// run as client
	m_socketManager.SetSmartAddressing( false );	// always send to server
	m_socketManager.SetMessageWindow(&(m_DebugDlg.m_edit));
	m_socketManager.trayIcon = &m_TrayIcon;
	m_socketManager.mainFrame = this;	

	for (int i=0; i<5; i++) {
		m_wndTaskbarNotifier[i].Create(this);
		m_wndTaskbarNotifier[i].SetSkin(POPUP_BITMAP_ID);
		m_wndTaskbarNotifier[i].SetTextFont(_T("Tahoma"),85,TN_TEXT_NORMAL,TN_TEXT_UNDERLINE);
 		m_wndTaskbarNotifier[i].SetTextColor(RGB(0,0,0),RGB(0,0,0));
		m_wndTaskbarNotifier[i].SetTextRect(CRect(10,53,m_wndTaskbarNotifier[i].m_nSkinWidth-10,m_wndTaskbarNotifier[i].m_nSkinHeight-25));	
	}


	if ((::theApp.GetProfileInt("Settings", "RefreshContactList", 1)==1) && 
		(::theApp.GetProfileInt("Settings", "OutlookFeatures", 1)==1)) {
		int interval = _tstoi(::theApp.GetProfileString("Settings", "RefreshInterval", "30").GetBuffer());
		SetTimer(TIMER_REFRESH_CONTACTS, interval * 60000, NULL);
	}


	//m_TrayIcon.ShowBaloon();	
	SetTimer(TIMER_ORIGINATE_CALL, 1000, NULL);
		
	return TRUE;
}


BOOL CMainFrame::DisplayWindow(BOOL bShow)
{
	CRect HiddenRect;
	HiddenRect.left   = GetSystemMetrics(SM_CXMAXIMIZED) * 4;
	HiddenRect.top	  = GetSystemMetrics(SM_CYMAXIMIZED) * 4;
	HiddenRect.right  = HiddenRect.left + CF_WINDOW_WIDTH;
	HiddenRect.bottom = HiddenRect.top  + CF_WINDOW_HEIGHT;
			
	MoveWindow(&HiddenRect);
			
	return TRUE;
}


void CMainFrame::OnFileAddContact() {
	AfxBeginThread(AddNewOutlookContactThreadProc, NULL);
	//try
	//	{					
	//		_ApplicationPtr pApp; //Outlook Application Instance
	//		pApp.CreateInstance(__uuidof(Application));
	//		IDispatchPtr contactPtr;
	//		_ContactItemPtr cItem;

	//		contactPtr = pApp->CreateItem(olContactItem);
	//		if (contactPtr!=NULL) {
	//			cItem = (_ContactItemPtr)contactPtr;
	//			cItem->Display();
	//		}						
	//		pApp.Release();			
	//	}		
	//	catch(_com_error &e)
	//	{
	//		TRACE((char*)e.Description());
	//	}	
}

void CMainFrame::OnFileSettings()
{
	// TODO: Add your command handler code here
	CWnd *pWnd = FindWindow(NULL, CString(APP_NAME) + " - " + _("Settings"));
	if (pWnd==NULL) {
		CSettingsDlg dlg;
		dlg.DoModal();
	} else {
		pWnd->BringWindowToTop();
	}
}


void CMainFrame::OnFileConnect() 
{
	if (m_SignInMenuItemData == 0) { //Sign In
		m_bManualSignInPerformed = TRUE;
		m_TrayIcon.SetTooltipText(CString(APP_NAME) + " - " + _("Signing In"));
		m_TrayIcon.SetIcon(IDI_ICON_CONNECTING2);
		SetTimer(TIMER_CONNECTING, 500, NULL);
		if ((m_bSigningIn==FALSE) && (m_socketManager.authenticated==FALSE))
            SignIn(TRUE);				
	} else if (m_SignInMenuItemData == 1) { //Cancel Sign In
		if (m_bSigningIn) {
			m_bCancelSignIn = TRUE;
			m_TrayIcon.SetTooltipText(CString(APP_NAME) + " - " + _("Cancelling Sign In..."));
		}
	} else if (m_SignInMenuItemData == 2) { //Sign Out
		if ((m_bSigningIn==FALSE) && (m_socketManager.IsOpen())) {			
			if (m_socketManager.authenticated) {
				if (m_bSignOutMessageBoxActive)
					return;
				m_bSignOutMessageBoxActive = TRUE;
				if (::MessageBox(this->GetSafeHwnd(), _("Are you sure you want to sign out?"), APP_NAME, MB_YESNO | MB_ICONQUESTION)==IDNO) {
					m_bSignOutMessageBoxActive = FALSE;
					return;
				}
				m_bSignOutMessageBoxActive = FALSE;
				CString packetString="Action: Logoff";
				m_socketManager.SendData(packetString);
			}
			m_socketManager.m_bManualSignOut = TRUE;
			m_socketManager.StopComm();
			m_TrayIcon.SetTooltipText(CString(APP_NAME) + " - " + _("Not Signed In"));
			m_TrayIcon.SetIcon(IDI_ICON_DISCONNECTED);
			m_socketManager.authenticated = FALSE;
		}
	}	
}


void CMainFrame::SignOut() {
	if ((m_bSigningIn==FALSE) && (m_socketManager.IsOpen())) {			
		if (m_socketManager.authenticated) {
			CString packetString="Action: Logoff";
			m_socketManager.SendData(packetString);
		}
		m_socketManager.m_bManualSignOut = TRUE;
		m_socketManager.StopComm();
		m_TrayIcon.SetTooltipText(CString(APP_NAME) + " - " + _("Not Signed In"));
		m_TrayIcon.SetIcon(IDI_ICON_DISCONNECTED);
		m_socketManager.authenticated = FALSE;
	}
}


LRESULT CMainFrame::OnAuthenticateMsg(WPARAM wParam, LPARAM lParam) {
	CPBXClientApp* pApp = (CPBXClientApp*)AfxGetApp();
	CString packetString="";
	BOOL signAutomatically = pApp->GetProfileInt("Settings", "AutomaticSignIn", 1);

	if ((m_bCancelSignIn) || (m_bSigningIn==FALSE)) {				
		m_bSigningIn = FALSE;
		m_bCancelSignIn = FALSE;
		m_socketManager.StopComm();
		KillTimer(TIMER_CONNECTING);
		m_TrayIcon.SetTooltipText(CString(APP_NAME) + " - " + _("Not Signed In"));
		m_TrayIcon.SetIcon(IDI_ICON_DISCONNECTED);
		return 1;
	}

	if (m_bManualSignInPerformed==FALSE) {
		m_TrayIcon.SetTooltipText(CString(APP_NAME) + " - " + _("Signing In"));
		m_TrayIcon.SetIcon(IDI_ICON_CONNECTING2);
		SetTimer(TIMER_CONNECTING, 500, NULL);
	}

	packetString = "Action: Challenge\r\n";
	packetString += "AuthType: MD5\r\n";
	packetString += "\r\n";	
	
	if (m_socketManager.WatchComm()) {
		m_socketManager.SendData(packetString);
	} else {
		m_bSigningIn = FALSE;
		m_bCancelSignIn = FALSE;
		KillTimer(TIMER_CONNECTING);
		m_TrayIcon.SetTooltipText(CString(APP_NAME) + " - " + _("Not Signed In"));
		m_TrayIcon.SetIcon(IDI_ICON_DISCONNECTED);
		m_socketManager.StopComm();
		if (signAutomatically)
			SetTimer(TIMER_SIGN_IN, 20000, NULL);
	}

    return 1;
}


UINT ConnectThreadProc( LPVOID pParam )
{
    CMainFrame* pMainFrame = (CMainFrame*)pParam;
	CPBXClientApp* pApp = (CPBXClientApp*)AfxGetApp();
	CString server = pApp->GetProfileString("Settings", "server", "");
	CString port = pApp->GetProfileString("Settings", "port", DEFAULT_PORT);
	long err;
    BOOL bAuthenticate = FALSE;

	IP_ADAPTER_INFO AdapterInfo[16];			// Allocate information for up to 16 NICs
	DWORD dwBufLen = sizeof(AdapterInfo);		// Save the memory size of buffer
	BOOL networkInitialized=FALSE;
	CString ipAddress;	
	PIP_ADAPTER_INFO pAdapterInfo;

	pMainFrame->m_socketManager.StopComm();

	while (1) {
		if (GetAdaptersInfo(AdapterInfo, &dwBufLen)!=0) {
			Sleep(1500);
			continue;
		}
		pAdapterInfo = AdapterInfo;
		do {
			if (strstr(pAdapterInfo->Description, "VMware Virtual Ethernet Adapter")==NULL) {
				ipAddress = pAdapterInfo->IpAddressList.IpAddress.String;
				if (ipAddress!="0.0.0.0") {
					networkInitialized = TRUE;
					break;
				}
			}
			pAdapterInfo = pAdapterInfo->Next;
		} while(pAdapterInfo);
	    		
		if (networkInitialized)
			break;
		else if (pMainFrame->m_bCancelSignIn)
			goto end;
		else
			Sleep(1500);
	}
	
	
	err = pMainFrame->m_socketManager.ConnectTo(server, port, AF_INET, SOCK_STREAM);

	if ((err!=0) && (pMainFrame->m_bManualSignInPerformed)) {		
		if (err==-1) {
			MessageBox(NULL, _("Connection to server couldn't be established! Details: socket already open."), APP_NAME, MB_ICONERROR);        
		} else {
			LPTSTR MessageBuffer;
			DWORD dwFormatFlags=FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM ;
			FormatMessage(dwFormatFlags, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &MessageBuffer, 0, NULL);
			CString str;
			str.Format(_("Connection to server couldn't be established! Details: %s"), MessageBuffer);
			MessageBox(NULL, str, APP_NAME, MB_ICONERROR);
			LocalFree(MessageBuffer);
		}		
	}

	if (err==0) {
		if (!pMainFrame->m_bCancelSignIn) {			
			PostMessage(pMainFrame->GetSafeHwnd(), WM_AUTHENTICATE, 0, 0);
			bAuthenticate = TRUE;			
		} else {
			pMainFrame->m_socketManager.StopComm();
		}
	}

end:
	if (!bAuthenticate) {				
		if ((::theApp.GetProfileInt("Settings", "AutomaticSignIn", 1)==1) && (!pMainFrame->m_bCancelSignIn)) {
			pMainFrame->SetTimer(TIMER_SIGN_IN, 20000, NULL);
		}
		pMainFrame->m_bSigningIn = FALSE;
		pMainFrame->m_bCancelSignIn = FALSE;
		KillTimer(pMainFrame->GetSafeHwnd(), TIMER_CONNECTING);
		pMainFrame->m_TrayIcon.SetTooltipText(CString(APP_NAME) + " - " + _("Not Signed In"));
		pMainFrame->m_TrayIcon.SetIcon(IDI_ICON_DISCONNECTED);
	}
	
    return 0;   // thread completed successfully
}



void CMainFrame::SignIn(BOOL manualSignIn) {
	TRACE("\n\nSIGN IN PROCEDURE\n\n");

	if (m_bSigningIn)
		return;

	m_socketManager.authenticated = FALSE;
	m_bManualSignInPerformed = manualSignIn;
	m_bCancelSignIn = FALSE;
	m_bSigningIn = TRUE;	

	if (manualSignIn) {
		m_TrayIcon.SetTooltipText(CString(APP_NAME) + " - " + _("Signing In"));
		m_TrayIcon.SetIcon(IDI_ICON_CONNECTING2);
		SetTimer(TIMER_CONNECTING, 500, NULL);
	}
	AfxBeginThread(ConnectThreadProc, this);
}


void CMainFrame::OnFileDebugInfo() {
	m_DebugDlg.InitLocaleGUI();	
	m_DebugDlg.ShowWindow(TRUE);
	m_DebugDlg.BringWindowToTop();
}


void CMainFrame::OnFileActiveCalls() {
	CWnd *pWnd = FindWindow(NULL, CString(APP_NAME) + " - " + _("Active Calls"));
	if (pWnd==NULL) {
		CActiveCallsDlg dlg;
		dlg.DoModal();
	} else {
		pWnd->BringWindowToTop();
	}	
}

void CMainFrame::OnFileFind() {
	CWnd *pWnd = FindWindow(NULL, CString(APP_NAME) + " - " + _("Find"));
	if (pWnd==NULL) {
		CFindContactDlg dlg;
		dlg.DoModal();
	} else {
		pWnd->BringWindowToTop();
	}    
}

void CMainFrame::OnFileCall()
{
	if (::theApp.CheckForExistingDialWindow())
		return;

	COutlookContactsDlg dlg;
	dlg.socketManager = &m_socketManager;
	if (::theApp.GetProfileInt("Settings", "OutlookFeatures", 1)!=1) {
		dlg.m_LoadContacts=FALSE;
	}
	dlg.DoModal();
}

void CMainFrame::OnUpdateFileConnect(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if (IsOutCALLConfigured()) {
		pCmdUI->Enable(TRUE);		
		if (m_socketManager.IsOpen()) {
			if (m_socketManager.authenticated) {
				pCmdUI->SetText(_("Sign Out"));				
				m_SignInMenuItemData = 2;
			} else if (m_bCancelSignIn) {
				pCmdUI->SetText(_("Cancelling..."));
				pCmdUI->Enable(FALSE);				
			} else {
				pCmdUI->SetText(_("Cancel Sign In"));				
				m_SignInMenuItemData = 1;
			}
		} else {
			if (m_bSigningIn) {
				if (m_bCancelSignIn) {
					pCmdUI->SetText(_("Cancelling..."));
					pCmdUI->Enable(FALSE);					
				} else if (m_bManualSignInPerformed) {
					pCmdUI->SetText(_("Cancel Sign In"));					
					m_SignInMenuItemData = 1;
				} else {
					pCmdUI->SetText(_("Sign In"));					
					m_SignInMenuItemData = 0;
				}
			} else {
				pCmdUI->SetText(_("Sign In"));				
				m_SignInMenuItemData = 0;
			}
		}
	} else {		
		pCmdUI->Enable(FALSE);
		pCmdUI->SetText(_("Sign In"));		
		m_SignInMenuItemData = 0;
	}	
}

void CMainFrame::OnUpdateFileCall(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable((m_socketManager.IsOpen() && m_socketManager.authenticated));
}

BOOL CMainFrame::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct) 
{
	// TODO: Add your message handler code here and/or call default	
	int dataLength = pCopyDataStruct->cbData;
	char *data = new char[dataLength+3];

	strncpy(data, (char*)pCopyDataStruct->lpData, dataLength);
	data[dataLength]='\0';

	CString strRecievedText = data;

	if (strRecievedText.Left(9).MakeLower()=="originate") {
		if (m_socketManager.IsOpen() && m_socketManager.authenticated) {
			int cnt=0;
			int nStart = strRecievedText.Find(_T(";;;"), 0)+3;
			int nEnd;
			CString channel, extension, callerid = "", var1="", var2="";
			while ((nEnd=strRecievedText.Find(_T(";;;"), nStart))!=-1) {
				if (cnt==0)
					channel = strRecievedText.Mid(nStart, nEnd-nStart);
				else if(cnt == 1)
					callerid = strRecievedText.Mid(nStart, nEnd-nStart);
				else if (cnt == 2)
					extension = strRecievedText.Mid(nStart, nEnd-nStart);
				else if (cnt == 3)
					var1 = strRecievedText.Mid(nStart, nEnd-nStart);
				
				cnt++;
				nStart = nEnd+3;
			}
			if (strRecievedText.Right(3)!=";;;")
				var1 = strRecievedText.Mid(nStart);

			/*CString outgoing_context = AfxGetApp()->GetProfileString("Settings", "OutgoingContext", "default");
			if (outgoing_context=="")
				outgoing_context="default";
			CString prefix = ::theApp.GetProfileString("Settings", "OutgoingPrefix", "");
			extension = prefix + extension;*/

			CString packetString = ::theApp.BuildOriginateCommand(channel, extension, callerid, "Variable: referenceid=" + var1);

			/*CString packetString = "Action: Originate\r\n";
			packetString += "Channel: " + channel + "\r\n";
			packetString += "Exten: " + extension + "\r\n";
			packetString += "Context: " + outgoing_context + "\r\n";
			packetString += "Priority: 1\r\n";
			packetString += "CallerID: " + callerid + "\r\n";
			packetString += "Variable: referenceid=" + var1 + "\r\n";
			//packetString += "ActionID: 123456\r\n";
			packetString += "\r\n";*/
			
			if (packetString!=_T("")) {
				m_socketManager.SendData(packetString);
			}
		}
	} else {
		if (strRecievedText!="Dial") {
			int nIndex = strRecievedText.Find(_T("#####"));
			if (nIndex!=-1)
				contactToDisplay=strRecievedText.Mid(nIndex+5);
			else
                contactToDisplay = "";
		}

		this->PostMessage(OUTLOOK_MESSAGE, 0, 0);
	}

	delete data;
	
	return CFrameWnd::OnCopyData(pWnd, pCopyDataStruct);
}


LRESULT CMainFrame::OnShowDialDialog(WPARAM wParam, LPARAM lParam) {
	if (::theApp.CheckForExistingDialWindow())
		return 1;

	COutlookContactsDlg dlg;	

	if (m_socketManager.IsOpen() && m_socketManager.authenticated) {
		dlg.contactToDisplay=contactToDisplay;
		dlg.socketManager = &m_socketManager;
		dlg.m_LoadContacts=TRUE;
		
		::theApp.m_pMainWnd->ActivateTopParent();
		
		dlg.DoModal();
	} else {
		::MessageBox(NULL, _("You must be Signed In in order to place calls from Outlook."), APP_NAME, MB_OK | MB_ICONERROR);
	}

	return 1;
}


LRESULT CMainFrame::OnStopSocketCommunication(WPARAM wParam, LPARAM lParam) {
	m_bSigningIn = FALSE;
	m_bCancelSignIn = FALSE;
	KillTimer(TIMER_CONNECTING);
	Sleep(500);
	m_socketManager.StopComm();
	return 1;
}


LRESULT CMainFrame::OnNewCallMsg(WPARAM wParam, LPARAM lParam) {
	ShowNotificationDialog("find_contact");
	return 1;
}


void CMainFrame::OnTrayIconDblClick() {
	if (IsOutCALLConfigured()) {			
		if (m_socketManager.IsOpen() && m_socketManager.authenticated) {
            if (::theApp.CheckForExistingDialWindow())
				return;

			COutlookContactsDlg dlg;
			dlg.socketManager = &m_socketManager;
			if (::theApp.GetProfileInt("Settings", "OutlookFeatures", 1)!=1) {
				dlg.m_LoadContacts=FALSE;				
			}
			dlg.DoModal();
		}
	}
}



LRESULT CMainFrame::OnCallNotifyDialogClick(WPARAM wParam, LPARAM lParam) {
	int ndIndex = (int)wParam; // notification dialog index	
	OpenContact(&(m_wndTaskbarNotifier[ndIndex]));
	return 1;
}



BOOL CMainFrame::DisplayContact(MAPIFolderPtr pFolder, CString fullName) {
	_ItemsPtr pItems;	
	_ContactItemPtr pContact;
	_FoldersPtr folders;
	CString restrictionStr = "[Full Name] = '" + fullName + "'";
		
	
	if (pFolder->GetDefaultItemType()==olContactItem) {
		pItems = pFolder->Items->Restrict(restrictionStr.AllocSysString());
		if (pItems!=NULL) {
			pContact = pItems->GetFirst();
			if (pContact!=NULL) {
				pContact->Display();				
				return TRUE;
			}
		}
	}

			
	folders = pFolder->GetFolders();
	pFolder = folders->GetFirst();
	while (pFolder!=NULL) {
		if (DisplayContact(pFolder, fullName)==TRUE) {
			return TRUE;	
		}
		pFolder = folders->GetNext();
	}
	
	return FALSE;
}

LRESULT CMainFrame::OnReloadOutlookContactsMsg(WPARAM wParam, LPARAM lParam) {
	LoadOutlookContacts(FALSE);
	return 1;
}

/** This is thread which will display Outlook's New Contact window,
	and reload Outlook contacts after the window is closed **/

UINT AddNewOutlookContactThreadProc( LPVOID pParam ) {
	CString name;
	CString number;

	if (pParam) {
		CString *contact_info = (CString*)pParam;
		std::vector<CString> info;
		SplitString(*contact_info, "###", info);
		delete contact_info;

		name = info[0];
		if (info.size()>=2) {			
			number = info[1];
		}
	}	

	DBG_LOG("---------- BEGIN OCW ----------");

	_ApplicationPtr pApp;
	HRESULT hres=CoInitialize(NULL); // CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (FAILED(hres)) {
		CStringA msg;
		msg.Format("Outlook contact window could not be opened. Failed to initialize COM library. Error code = %X", hres);
		DBG_LOG(msg);
		DBG_LOG("---------- END OCW ----------");
		return 0;
	}
	
	hres = pApp.CreateInstance(__uuidof(Application)); // WideString("Outlook.Application").data());
	if (FAILED(hres)) {
		CoUninitialize();
		CStringA msg;
		msg.Format("Outlook contact window could not be opened. Failed to create Outlook Instance. Error code = %X", hres);
		DBG_LOG(msg);
		DBG_LOG("---------- END OCW ----------");
		return 0;
	}

	try {
		LPDISPATCH contactPtr;
		_ContactItemPtr cItem = pApp->CreateItem(olContactItem);
		//contactPtr = (LPDISPATCH)pApp->CreateItem(olContactItem);
		if (cItem!=NULL) {
			// Fixes bugs in Outlook when called using COM, we must create a temp Outlook window and close it
			// If not, contact will not be created correctly (known to work on Outlook 2007)
			//cItem = (_ContactItemPtr)contactPtr;
			cItem->Display();
			cItem->Close(olDiscard);
			cItem = (LPDISPATCH)pApp->CreateItem(olContactItem);

			if (cItem!=NULL) {
				//cItem = (_ContactItemPtr)contactPtr;
				cItem->put_FullName(name.AllocSysString());
				cItem->put_BusinessTelephoneNumber(number.AllocSysString());
				cItem->Display(TRUE);
				PostMessage(g_pMainWindow->GetSafeHwnd(), WM_RELOAD_OUTLOOK_CONTACTS, 0, 0);
				//cItem->Release();
			} else {
                DBG_LOG("Failed to create Outlook Contact #2");
            }
		} else {
			DBG_LOG("Failed to create Outlook Contact #1");
		}
	} catch (...) {
		DBG_LOG("Exception raised.");
	}

	pApp.Release();
	CoUninitialize();

	DBG_LOG("---------- END OCW ----------");

	return 0;
}

void CMainFrame::OpenContact(CTaskbarNotifier *notifyWnd) {
	if (notifyWnd->callNotification) {
		try
		{					
			if (notifyWnd->contactExists) {
				// open existing contact in Outlook

				_ApplicationPtr pApp; //Outlook Application Instance
				pApp.CreateInstance(__uuidof(Application));
				IDispatchPtr contactPtr;
				_ContactItemPtr cItem;

				_FoldersPtr folders = pApp->GetNamespace(_bstr_t("MAPI"))->GetFolders();
				MAPIFolderPtr pFolder = folders->GetFirst(); //Personal folders (root folder)
				//fullName is like: "Name Surname  (phone)"
				CString fullName = notifyWnd->contactFullName;
				int nIndex = fullName.ReverseFind('(');
				if (nIndex!=-1)
					fullName = fullName.Left(nIndex-2);				
				DisplayContact(pFolder, fullName);
			} else {
				// Display "New Contact" window in Outlook
				CString callerID1 = notifyWnd->callerID1;
				CString callerID2 = notifyWnd->callerID2;

				CString *info = new CString(callerID2 + "###" + callerID1);
				AfxBeginThread(AddNewOutlookContactThreadProc, info);
			}			
		}		
		catch(_com_error &e)
		{
			TRACE((char*)e.Description());
		}
	}
}

void CMainFrame::ShowNotificationDialog(CString message) {	
	CString callerID1 = m_socketManager.callerID1;
	CString callerID2 = m_socketManager.callerID2;
	CString QueueExtension = m_socketManager.QueueExtension;
	CString channel = m_socketManager.channel;
	CString callDate = m_socketManager.callDate;
	CString callTime = m_socketManager.callTime;
	time_t call_time;
	time(&call_time);
	
	CallInfo call_info;

	call_info.sdate = callDate;
	call_info.stime = callTime;
	call_info.call_time = call_time;

	CString callerPhoneNumber;

	int wndToShow = 0;
	CString contactFullName="not_found";
	CString szDelayTime = ::theApp.GetProfileString("Settings", "DelayTime", "3");
	CString calledExtension = m_socketManager.calledExtension;
	int delayTime;

	bool bCID1Numeric=true, bCID2Numeric=true;
	bool bShowNNCallerID = ::theApp.GetProfileInt("Settings", "NNCallerID", 1)==1?true:false;

	if (callerID1=="") {
		bCID1Numeric=false;
	} else {
		for (int i=0; i<callerID1.GetLength(); i++) {
			if ((callerID1[i]>=48 && callerID1[i]<=57) || callerID1[i]=='+') {
				continue;
			} else {
				bCID1Numeric=false;						
				break;
			}
		}
	}

	if (callerID2=="") {
		bCID2Numeric=false;
	} else {
		for (int i=0; i<callerID2.GetLength(); i++) {
			if ((callerID2[i]>=48 && callerID2[i]<=57) || callerID2[i]=='+') {
				continue;
			} else {
				bCID2Numeric=false;
				break;
			}
		}
	}


	if (szDelayTime=="0") {
		delayTime = 0;
	} else {
		delayTime = _tstoi(szDelayTime.GetBuffer(10));
		if (delayTime==0)
			; //delayTime=3000;
		else
			delayTime = delayTime * 1000;
	}


	for (int i=0; i<5; i++) {
		if (m_wndTaskbarNotifier[i].visible)
			wndToShow=i+1;
	}

	if (wndToShow>=5)
		return; //all notification dialogs are already displayed, no more resources
	
	m_wndTaskbarNotifier[wndToShow].nDisplayedAlready = wndToShow;

	if (message!="find_contact") {
		m_wndTaskbarNotifier[wndToShow].callNotification = FALSE;
		m_wndTaskbarNotifier[wndToShow].Show(message, 500, delayTime, 500, 1);		
	} else {
		m_wndTaskbarNotifier[wndToShow].callNotification = TRUE;

		if ((::theApp.GetProfileInt("Settings", "UseOCDatabase", 1)==1) &&
			(::theApp.GetProfileInt("Settings", "OutlookFeatures", 1)==1))
			contactFullName = FindContact(callerID1, callerID2, callerPhoneNumber);
		else
			contactFullName="not_found";

		if (contactFullName=="not_found") {
			if (callerID2=="")
				message.Format(_("You have a Call\nFrom: %s\nTo: %s"), callerID1, calledExtension);				
			else if (callerID1!=callerID2)
                message.Format(_("You have a Call\nFrom: %s\nTo: %s"), callerID1 + "  (" + callerID2 + ")", calledExtension);						
			else
				message.Format(_("You have a Call\nFrom: %s\nTo: %s"), callerID1, calledExtension);				
            
			if (QueueExtension!="")
				message.Append(" (" + QueueExtension + ")");

			m_wndTaskbarNotifier[wndToShow].contactExists = FALSE;
			m_wndTaskbarNotifier[wndToShow].callerID1 = callerID1;
			m_wndTaskbarNotifier[wndToShow].callerID2 = callerID2;

			call_info.bOutlookContact = false;
			if (callerID2!="") {
				call_info.cid = callerID1+"  (" + callerID2 + ")";
			} else {
				call_info.cid = callerID1;
			}
			
			m_socketManager.m_MissedCalls[channel] = call_info;				
		} else {
            message.Format(_("You have a Call\nFrom: %s\nTo: %s"), contactFullName, calledExtension);
			if (QueueExtension!="")
				message.Append(" (" + QueueExtension + ")");
			
			contactFullName = contactFullName.Left(contactFullName.ReverseFind('(')-1);
			m_wndTaskbarNotifier[wndToShow].contactExists = TRUE;
			m_wndTaskbarNotifier[wndToShow].contactFullName = contactFullName;

			call_info.bOutlookContact = true;
			call_info.cid = contactFullName+"  (" + callerPhoneNumber + ")";
			m_socketManager.m_MissedCalls[channel] = call_info;
		}
								
		m_wndTaskbarNotifier[wndToShow].Show(message, 500, delayTime, 500, 1);
		if ((m_wndTaskbarNotifier[wndToShow].contactExists==FALSE) &&
			(::theApp.GetProfileInt("Settings", "DisplayNewContactWindow", 1)==1) &&
			(::theApp.GetProfileInt("Settings", "OutlookFeatures", 1)==1) && (bShowNNCallerID || bCID1Numeric==true || bCID2Numeric==true))
				OpenContact(&(m_wndTaskbarNotifier[wndToShow]));				
	}	
}


CString CMainFrame::FindContact(CString callerID1, CString callerID2, CString &callerPhoneNumber) {
    CppSQLite3DB db;	    
	db.open(CStringA(OUTCALL_DB));

	//Query q(::theApp.db);
	
	CString retValue=_T("not_found");
	CStringA phoneNumber;	

	for (int i=0; i<2; i++) {
        if (i==0)
			phoneNumber = callerID1;
		else
			phoneNumber = callerID2;

		if (phoneNumber=="")
			continue;

		try {

			CppSQLite3Query q = db.execQuery(CStringA("select * from Contacts where Assistant='" + phoneNumber + "' or Business='" +
			phoneNumber + "' or Business2='" + phoneNumber + "' or BusinessFax='" + phoneNumber + 
			"' or Callback='" + phoneNumber + "' or Car='" + phoneNumber + "' or Company='" + 
			phoneNumber + "' or Home='" + phoneNumber + "' or Home2='" + phoneNumber + "' or HomeFax='" + phoneNumber +
			"' or ISDN='" + phoneNumber + "' or Mobile='" + phoneNumber + "' or Other='" + phoneNumber +
			"' or OtherFax='" + phoneNumber + "' or Pager='" + phoneNumber + "' or PrimaryTel='" + phoneNumber +
			"' or Radio='" + phoneNumber + "' or Telex='" + phoneNumber + "' or TTYTDD='" + phoneNumber + "'").GetBuffer());		

			if (!q.eof()) {
				CString fullName = CString(q.fieldValue(1)) + " " + q.fieldValue(3);
				int index=6;
				if (phoneNumber==q.fieldValue(index))
					retValue = fullName + " (Assistant)";
				else if (phoneNumber==q.fieldValue(index+1))
					retValue = fullName + " (Business)";
				else if (phoneNumber==q.fieldValue(index+2))
					retValue = fullName + " (Business2)";
				else if (phoneNumber==q.fieldValue(index+3))
					retValue = fullName + " (BusinessFax)";
				else if (phoneNumber==q.fieldValue(index+4))
					retValue = fullName + " (Callback)";
				else if (phoneNumber==q.fieldValue(index+5))
					retValue = fullName + " (Car)";
				else if (phoneNumber==q.fieldValue(index+6))
					retValue = fullName + " (Company)";
				else if (phoneNumber==q.fieldValue(index+7))
					retValue = fullName + " (Home)";
				else if (phoneNumber==q.fieldValue(index+8))
					retValue = fullName + " (Home2)";
				else if (phoneNumber==q.fieldValue(index+9))
					retValue = fullName + " (HomeFax)";
				else if (phoneNumber==q.fieldValue(index+10))
					retValue = fullName + " (ISDN)";
				else if (phoneNumber==q.fieldValue(index+11))
					retValue = fullName + " (Mobile)";
				else if (phoneNumber==q.fieldValue(index+12))
					retValue = fullName + " (Other)";
				else if (phoneNumber==q.fieldValue(index+13))
					retValue = fullName + " (OtherFax)";
				else if (phoneNumber==q.fieldValue(index+14))
					retValue = fullName + " (Pager)";
				else if (phoneNumber==q.fieldValue(index+15))
					retValue = fullName + " (PrimaryTel)";
				else if (phoneNumber==q.fieldValue(index+16))
					retValue = fullName + " (Radio)";
				else if (phoneNumber==q.fieldValue(index+17))
					retValue = fullName + " (Telex)";
				else if (phoneNumber==q.fieldValue(index+18))
					retValue = fullName + " (TTYTDD)";
			}		

			if (retValue!=_T("not_found")) {
				callerPhoneNumber = phoneNumber;
				break;
			}
		} catch (CppSQLite3Exception& e) { }
	}

	return retValue;	
}


#ifdef _WIN64
void CMainFrame::OnTimer(UINT_PTR nIDEvent)
#else
void CMainFrame::OnTimer(UINT nIDEvent)
#endif
{
	// TODO: Add your message handler code here and/or call default		
	if (nIDEvent==TIMER_CONNECTING) {
		UINT iconID = m_TrayIcon.iconResourceID;
		if ((iconID!=IDI_ICON_CONNECTING1) && (iconID!=IDI_ICON_CONNECTING2)) {
			KillTimer(nIDEvent);			
		} else {
			if (iconID==IDI_ICON_CONNECTING1)
				m_TrayIcon.SetIcon(IDI_ICON_CONNECTING2);
			else
				m_TrayIcon.SetIcon(IDI_ICON_CONNECTING1);			
				
		}
	} else if (nIDEvent==TIMER_SIGN_IN) {
		KillTimer(TIMER_SIGN_IN);
		SignIn(FALSE);
	} else if (nIDEvent==TIMER_REFRESH_CONTACTS) {		
		LoadOutlookContacts(NULL);		
	} else if (nIDEvent==TIMER_ORIGINATE_CALL) {
		Semaphore s(ORIGINATE_FILE_MUTEX);
		CStdioFile debugFile;
		CString outgoing_context = ::theApp.GetProfileString("Settings", "OutgoingContext", "default");
		if (outgoing_context=="")
			outgoing_context="default";
		
		if (debugFile.Open(::theApp.GetInstallDir()+"\\originate.tmp", CFile::modeRead)) {
			CString strRecievedText;
			debugFile.ReadString(strRecievedText);
			if (strRecievedText.Left(9).MakeLower()=="originate") {
				if (m_socketManager.IsOpen() && m_socketManager.authenticated) {
					int cnt=0;
					int nStart = strRecievedText.Find(_T(";;;"), 0)+3;
					int nEnd;
					CString channel, extension, callerid = "", var1="", var2="";
					while ((nEnd=strRecievedText.Find(_T(";;;"), nStart))!=-1) {
						if (cnt==0)
							channel = strRecievedText.Mid(nStart, nEnd-nStart);
						else if(cnt == 1)
							callerid = strRecievedText.Mid(nStart, nEnd-nStart);
						else if (cnt == 2)
							extension = strRecievedText.Mid(nStart, nEnd-nStart);
						else if (cnt == 3)
							var1 = strRecievedText.Mid(nStart, nEnd-nStart);
						
						cnt++;
						nStart = nEnd+3;
					}
					if (strRecievedText.Right(3)!=";;;")
						var1 = strRecievedText.Mid(nStart);

					//CString prefix = ::theApp.GetProfileString("Settings", "OutgoingPrefix", "");
					//extension = prefix + extension;

					//CString packetString = "Action: Originate\r\n";
					//packetString += "Channel: " + channel + "\r\n";
					//packetString += "Exten: " + extension + "\r\n";
					//packetString += "Context: " + outgoing_context + "\r\n";	
					//packetString += "Priority: 1\r\n";
					//packetString += "CallerID: " + callerid + "\r\n";
					//packetString += "Variable: referenceid=" + var1 + "\r\n";
					////packetString += "ActionID: 123456\r\n";
					//packetString += "\r\n";

					CString packetString = ::theApp.BuildOriginateCommand(channel, extension, callerid, "Variable: referenceid=" + var1);
					
					if (packetString!=_T("")) {
						m_socketManager.SendData(packetString);
					}
				}
			}			
			debugFile.Close();
			debugFile.Remove(::theApp.GetInstallDir()+"\\originate.tmp");
		}		
	}
	
	CFrameWnd::OnTimer(nIDEvent);
}



BOOL CMainFrame::IsOutCALLConfigured() {
	if (::theApp.GetProfileString("Settings", "server", "")=="")
		return FALSE;		
	else if (::theApp.GetProfileString("Settings", "username", "")=="")
		return FALSE;	
	else
		return TRUE;
}



void CMainFrame::OnUpdateFileAddContact(CCmdUI* pCmdUI) {
	if (::theApp.GetProfileInt("Settings", "OutlookFeatures", 1)==1)
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

afx_msg void CMainFrame::OnUpdateFileFindContact(CCmdUI* pCmdUI) {
    if (::theApp.GetProfileInt("Settings", "OutlookFeatures", 1)==1)
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}


void CMainFrame::UpdateTrayIcon() {
	if (IsOutCALLConfigured() && (m_TrayIcon.iconResourceID==IDI_ICON_RED)) {
		m_TrayIcon.SetIcon(IDI_ICON_DISCONNECTED);
		m_TrayIcon.SetTooltipText(CString(APP_NAME) + " - " + _("Not Signed In"));
	}
}



void CMainFrame::OnFileMissedCalls() {
	CWnd *pWnd = FindWindow(NULL, CString(APP_NAME) + " - " + _("Call History"));
	if (pWnd==NULL) {
		CMissedCallsDlg dlg;
		dlg.DoModal();
	} else {
		pWnd->BringWindowToTop();
	}	
}

void CMainFrame::OnFileRefreshContactsFromOutlook() {
	LoadOutlookContacts(NULL);
}

void CMainFrame::LoadOutlookContacts(BOOL bDeleteExistingContacts/*if this parameter is NULL, load from Outlook, otherwise load from CSV*/) {
	if (::theApp.m_bLoadingContacts)
		return;
	::theApp.m_bLoadingContacts = TRUE;
	AfxBeginThread(RefreshContactsProc, (void*)bDeleteExistingContacts);
}

void CMainFrame::CancelLoadOutlookContacts() {
	gMapiEx.StopLoadingContacts();
}

void CMainFrame::OnFileRefreshContactsFromCSV() {
	BOOL bDeleteExisting = 1;
	if (MessageBox(_("Do you want to save your existing contacts?"), APP_NAME, MB_YESNO)==IDNO) {
		bDeleteExisting=2;
	}
	LoadOutlookContacts(bDeleteExisting);	
}



void CMainFrame::GetCSVValues(CString line, vector<CString> &values) {
	CString value;
	TCHAR *buff = line.GetBuffer();
	int bFirst=-1;
	int bSecond=-1;

	//bool bPreviousApostrof=false;
	//bool bApostrof=false;
	//bool bValueStarted=false;
	values.clear();
	
	for (int i=0; i<line.GetLength(); i++) {		
		if (buff[i]==('"')) {
            if (bFirst==-1)
				bFirst = i;
			else if (bSecond==i-1)
				bSecond = -1;
			else
				bSecond = i;
		} else if (buff[i]==',') {
			if (bFirst!=-1 && bSecond!=-1) {
				value = line.Mid(bFirst+1, bSecond-bFirst-1);
				value.Replace(_T("\"\""), _T("\""));
				values.push_back(value);
				bFirst = -1;
				bSecond = -1;
			} else if (bFirst==-1 && bSecond==-1) {
				values.push_back("");
			}
			if (i==(line.GetLength()-1)) {
                values.push_back("");                                
			}
		}		
	}

	if (bFirst!=-1 && bSecond!=-1) {
		value = line.Mid(bFirst+1, bSecond-bFirst-1);
		value.Replace(_T("\"\""), _T("\""));
		values.push_back(value);
	}
}


UINT RefreshContactsProc( LPVOID pParam ) {

	if (pParam==NULL) { // Load from Outlook directly
				
		if (!gMapiEx.Login()) { // initialize MAPI session
			gMapiEx.Logout();
			DBG_LOG("Failed to initialize MAPI");
			MessageBox(NULL, _("Failed to initialize MAPI!"), APP_NAME, MB_ICONERROR);
            ::theApp.m_bLoadingContacts = FALSE;	
			return 0;
		}
		
		gMapiEx.LoadOutlookContacts();
		::theApp.WriteProfileInt("Settings", "ContactsLoaded", 1);

		gMapiEx.Logout(); // close MAPI session
	} else {
		CFileDialog OpenDlg(1, 0, 0, OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, _T("CSV Files (*.csv)|*.csv||"));

		CppSQLite3DB db;
		db.open(CStringA(OUTCALL_DB));

		if (OpenDlg.DoModal()==IDOK) {			

			try {				
				if ((int)pParam==2) {
					db.execDML("DELETE FROM Contacts");
				}				

				FILE *file = _tfopen((LPCTSTR)OpenDlg.GetPathName(), _T("r"));
				if (file) {
					db.execDML("begin transaction");

					vector<int> field_map;
					vector<CString> fields;
					vector<CString> values;
					vector<CString> temp_values;


					char ch[4097];
					CString line;
					CString str;

					CMainFrame *pMainFrame = (CMainFrame*)::theApp.m_pMainWnd;

					if (!feof(file)) {
						line = fgets(ch, 4096, file);
						pMainFrame->GetCSVValues(line, fields);
						/*SplitString(line, "\",", fields);						
						for (int i=0; i<fields.size(); i++) {
							fields[i] = fields[i].Mid(1);
						}*/
					}										
					int nFields = fields.size();					

					CMapContactFieldsDlg dlg(NULL, &field_map, &fields);
					dlg.DoModal();

					int nPos, nPos2;
					CString query, value;
					bool bEmpty;

					while (!feof(file)) {
						line = fgets(ch, 4096, file);
						if (line.Trim()=="")
							continue;						
						
						pMainFrame->GetCSVValues(line, values);

						for (int i=values.size(); i<nFields; i++) {
							values.push_back(""); //just to make sure all values are there for existing fields
						}

						query = "INSERT INTO Contacts values(";

						for (int i=0; i<field_map.size(); i++) {
							if (field_map[i]==-1) {
								query += "'',";
							} else {
								value = values[field_map[i]];
								value.Replace(_T("'"), _T("''"));
                                query += "'" + value + "',";							
							}
						}
                        
						query = query.Mid(0, query.GetLength()-1);
						query += ")";

						db.execDML(query.GetBuffer());
					}
					
					db.execDML("end transaction");
					::theApp.WriteProfileInt("Settings", "ContactsLoaded", 1);                    
				} else {
					MessageBox(NULL, _("Error opening CSV file."), APP_NAME, MB_ICONERROR);
                    
				}                
			} catch (CppSQLite3Exception& e) {
                MessageBox(NULL, CString(e.errorMessage()), APP_NAME, MB_ICONERROR);	            		
			}
		}
	}
	
	::theApp.m_bLoadingContacts = FALSE;
	
	return 0;
}

HRESULT CMainFrame::LoadProfileDetails(vector<CString> &vProfiles, int &nDefault, bool bInitializeMAPI)
{
	return gMapiEx.LoadProfileDetails(vProfiles/*, nDefault, bInitializeMAPI*/);
}





