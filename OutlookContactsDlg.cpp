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

// OutlookContactsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PBXClient.h"
#include "OutlookContactsDlg.h"
#include "MainFrm.h"
#include "semaphore.h"
#include "FindContactDlg.h"

#include "LoadingContactsDlg.h"

//#include <atlconv.h>

#include "CPPSQLite3.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// COutlookContactsDlg dialog

COutlookContactsDlg::COutlookContactsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COutlookContactsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COutlookContactsDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);	
	contactToDisplay="";
	m_bMultipleCalls=FALSE;
	m_LoadContacts=TRUE;
}

void COutlookContactsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COutlookContactsDlg)
	DDX_Control(pDX,IDC_CONTACT,m_contacts);
	// DDX_Control(pDX,IDC_PHONE,m_numbers);
	DDX_Control(pDX,IDC_EDIT_NUMBER,m_phoneNumber);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_COMBO_DIAL_FROM, m_DialFrom);
	DDX_Control(pDX, IDC_STATIC_SEPARATOR, m_separator);
	DDX_Control(pDX, IDC_STATIC_SEPARATOR2, m_separator2);
	DDX_Control(pDX, IDC_BUTTON_SKIP, m_btnSkip);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Control(pDX, IDOK, m_btnDial);
	DDX_Control(pDX, IDCANCEL, m_btnExit);	
	DDX_Control(pDX, IDC_BUTTON_OPTIONS, m_btnOptions);	
}

BEGIN_MESSAGE_MAP(COutlookContactsDlg, CDialog)
	//{{AFX_MSG_MAP(COutlookContactsDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_CONTACT, OnSelchangeContacts)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_SKIP, OnBnClickedButtonSkip)
	ON_WM_SHOWWINDOW()
	ON_WM_SETFOCUS()
	ON_WM_ACTIVATE()
	ON_MESSAGE(WM_SHOW_LOADING_CONTACTS_DLG_MSG, OnDisplayLoadingContactsDlg)

	ON_MESSAGE(WM_ENABLE_DIAL_BUTTON, OnEnableDialButton)
	
	//ON_MESSAGE(WM_LOADCONTACTS_FINISHED_MSG, OnLoadContactsFinishedMessage)
	
	//ON_BN_CLICKED(IDC_BUTTON_REFRESH, OnBnClickedButtonRefresh)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnLvnItemchangedList)

	ON_COMMAND(ID_IMPORTCONTACTS_FROMOUTLOOK, OnPopupRefreshFromOutlook)	
	ON_COMMAND(ID_IMPORTCONTACTS_FROMCSVFILE, OnPopupRefreshFromCSV)		

	ON_COMMAND(ID_DISPLAY_FIRSTNAME, OnPopupFirstname)
	ON_COMMAND(ID_DISPLAY_LASTNAME, OnPopupLastname)
	ON_COMMAND(ID_POPUP_HIDECONTACTSWITHNOTELEPHONENUMBERS, OnPopupHideContacts)
	

	ON_BN_CLICKED(IDC_BUTTON_OPTIONS, OnBnClickedButtonOptions)
//	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnNMDblclkList)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_FIND, OnBnClickedButtonFind)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnNMDblclkList)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COutlookContactsDlg message handlers

BOOL COutlookContactsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(CString(APP_NAME) + " - " + _("Dial"));

	/*if (::theApp.m_pDialForm!=NULL) {		
		SendMessage(WM_CLOSE);
		::theApp.m_pDialForm->SetForegroundWindow();
		::theApp.m_pDialForm->SetFocus();
		return FALSE;
	}*/	

	//PostMessage(WM_LOADCONTACTS_MSG, 0, 0);

	::theApp.m_pDialForm = this;

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	//SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_bFilterContacts = ::theApp.GetProfileInt("Settings", "FilterContacts", 0)?TRUE:FALSE;

	//SetWindowPos(&this->wndTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	LoadExtensions();

	if (m_LoadContacts==FALSE) {
		RECT pRect;
		RECT rect;
		GetWindowRect(&pRect);		

		GetDlgItem(IDC_STATIC_CONTACT)->ShowWindow(FALSE);
		GetDlgItem(IDC_BUTTON_FIND)->ShowWindow(FALSE);
		GetDlgItem(IDC_BUTTON_OPTIONS)->ShowWindow(FALSE);
		m_separator2.ShowWindow(FALSE);
		m_list.ShowWindow(FALSE);
		GetDlgItem(IDC_STATIC_PHONE_NUMBERS)->ShowWindow(FALSE);		
		m_contacts.ShowWindow(FALSE);

		GetDlgItem(IDC_STATIC_CONTACT)->GetWindowRect(&rect);
		ScreenToClient(&rect);

		GetDlgItem(IDC_STATIC_PHONE)->SetWindowPos(NULL, rect.left, rect.top, 0, 0, SWP_NOSIZE);
		m_contacts.GetWindowRect(&rect);
		ScreenToClient(&rect);
		GetDlgItem(IDC_EDIT_NUMBER)->SetWindowPos(NULL, rect.left, rect.top, 0, 0, SWP_NOSIZE);
		GetDlgItem(IDC_BUTTON_FIND)->GetWindowRect(&rect);
		ScreenToClient(&rect);
		
		m_btnDial.GetWindowRect(&pRect);
		ScreenToClient(&pRect);
		m_btnDial.SetWindowPos(NULL, pRect.left, rect.top, 0, 0, SWP_NOSIZE);

		m_btnExit.GetWindowRect(&pRect);
		ScreenToClient(&pRect);
		m_btnExit.SetWindowPos(NULL, pRect.left, rect.top, 0, 0, SWP_NOSIZE);

		m_btnSkip.GetWindowRect(&pRect);
		ScreenToClient(&pRect);
		m_btnSkip.SetWindowPos(NULL, pRect.left, rect.top, 0, 0, SWP_NOSIZE);			

		GetWindowRect(&rect);
		m_btnDial.GetWindowRect(&pRect);
        ScreenToClient(&pRect);        
		SetWindowPos(&this->wndNoTopMost,0,0,rect.right-rect.left,pRect.bottom+40,SWP_NOMOVE);

		if ((m_Callees.size()>0) && (m_bMultipleCalls)) {
			SetWindowText(CString(APP_NAME) + " - " + _("Dial") + " [" + m_Callees[0] + "]");
			m_Callees.erase(m_Callees.begin());
			m_DialFrom.SelectString(-1, m_CallerPhones[0]);
			m_CallerPhones.erase(m_CallerPhones.begin());
			m_phoneNumber.SetFocus();
			m_btnSkip.ShowWindow(TRUE);
		}		
	} else {
		RECT rect;
		m_list.GetClientRect(&rect);
		m_list.InsertColumn(0, _("Phone number"), LVCFMT_LEFT, rect.right*0.5);
		m_list.InsertColumn(1, _("Location"), LVCFMT_LEFT, rect.right*0.5);
		LONG style = GetWindowLong(m_list.GetSafeHwnd(), GWL_STYLE);
		style |= (LVS_EX_FULLROWSELECT | LVS_REPORT | LVS_SHOWSELALWAYS);
		style &= ~LVS_SORTDESCENDING;
		style &= ~LVS_SORTASCENDING;
		SetWindowLong(m_list.GetSafeHwnd(), GWL_STYLE, style);
		m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);		
	}	

	displayOptions = ::theApp.GetProfileInt("Settings", "DisplaySettings", 1);
	menu.LoadMenu(IDR_MAINFRAME);
	
	m_phoneNumber.SetFocus();

	InitLocaleGUI();

	SetWindowPos(&this->wndTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	this->SetForegroundWindow();
	
	return TRUE; //FALSE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void COutlookContactsDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR COutlookContactsDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void COutlookContactsDlg::LoadContacts()
{	
	CppSQLite3DB db;
	db.open(CStringA(OUTCALL_DB));

	std::vector<CString> *phoneNumbers;
	std::vector<CString> *OutlookPhoneNumbers = NULL;
	CString ContactToSelect;
	bool bOutlookContactFound = false;
	CString number, fullName, name, surname, temp, company, prefix;	
	
	int i;
	int num;

	// First free memory of the existing items in the list	
	for (int i=0; i<m_contacts.GetCount(); i++) {
		phoneNumbers = (std::vector<CString>*)m_contacts.GetItemData(i);
		if (phoneNumbers!=NULL) {
			delete phoneNumbers;
			phoneNumbers = NULL;
			m_contacts.SetItemData(i, NULL);
		}
	}
	m_contacts.ResetContent();

	i = contactToDisplay.Find(_T("#####"));
	if (i!=-1) {
		CString num = contactToDisplay.Mid(i+5);
		std::vector<CString> ret;
		contactToDisplay = contactToDisplay.Left(i);
		OutlookPhoneNumbers = new std::vector<CString>;
		SplitString(contactToDisplay, " | ", ret);
		int size = ret.size();
		if (ret.size()==3) {
			if (displayOptions==1) {				
                contactToDisplay = ret[0];
				if (ret[1]!="")
					contactToDisplay += (" " + ret[1]);
				if (ret[2]!="")
					contactToDisplay += (" " + ret[2]);								
			} else {
                contactToDisplay = ret[2];
				if (ret[0]!="")
					contactToDisplay += (" " + ret[0]);
				if (ret[1]!="")
					contactToDisplay += (" " + ret[1]);								
			}
		} else {
			contactToDisplay.Replace(_T(" | "), _T(""));
		}
		if (num!="") {
			int pos;
			std::vector<CString> numbers;
			CString location;
            SplitString(num, " | ", numbers);
			for (i=0; i<numbers.size(); i++) {
				if (numbers[i]=="")
					continue;
				pos = numbers[i].Find(_T(":"));
				location = numbers[i].Left(pos);
				num = numbers[i].Mid(pos+1);
				OutlookPhoneNumbers->push_back(num + "  (" + location + ")");
			}
		}
	}

	try {
    
		CppSQLite3Query q = db.execQuery("SELECT * FROM Contacts");
		bool bSelectedOutlookContact = false;

		while (!q.eof())
		{	
			bSelectedOutlookContact = false;
			if (displayOptions==1) {
				fullName = q.fieldValue(1);						
				temp = q.fieldValue(2);
				if (temp!="")
					fullName += (" " + temp);
				temp = q.fieldValue(3);
				if (temp!="")
					fullName += (" " + temp);
			} else {
				fullName = q.fieldValue(3);			
				temp = q.fieldValue(1);
				if (temp!="")
					fullName += (" " + temp);
				temp = q.fieldValue(2);
				if (temp!="")
					fullName += (" " + temp);
			}

			if (contactToDisplay==fullName && OutlookPhoneNumbers) {
				bSelectedOutlookContact=true;				
			}

			fullName += (" " + CString(q.fieldValue(4)));
			fullName = fullName.TrimLeft();

			company = q.fieldValue(5);
			if (company.Trim()!="") {
				if (fullName!="")
					fullName += (" (" + CString(q.fieldValue(5)) + ")");
				else
					fullName = CString(q.fieldValue(5));
			}

			if (bSelectedOutlookContact && bOutlookContactFound==false) {
                bOutlookContactFound = true;
                ContactToSelect = fullName;
                phoneNumbers = OutlookPhoneNumbers;
			} else { 
				phoneNumbers = new std::vector<CString>;

				for (int i=0; i<=18; i++) {
					number = q.fieldValue(i+6);
					num = i;
					if (num==0)
						temp = "Assistant";
					else if (num==1)
						temp = "Business";
					else if (num==2)
						temp = "Business 2";
					else if (num==3)
						temp = "Business Fax";
					else if (num==4)
						temp = "Callback";
					else if (num==5)
						temp = "Car";
					else if (num==6)
						temp = "Company";
					else if (num==7)
						temp = "Home";
					else if (num==8)
						temp = "Home 2";
					else if (num==9)
						temp = "Home Fax";
					else if (num==10)
						temp = "ISDN";
					else if (num==11)
						temp = "Mobile";
					else if (num==12)
						temp = "Other";
					else if (num==13)
						temp = "Other Fax";
					else if (num==14)
						temp = "Pager";
					else if (num==15)
						temp = "Primary";
					else if (num==16)
						temp = "Radio";
					else if (num==17)
						temp = "Telex";
					else if (num==18)
						temp = "TTY/TDD";

					if (number!="")
						phoneNumbers->push_back(number + "  (" + temp + ")");
				}
			}
			if (phoneNumbers->size()!=0 || !m_bFilterContacts)
				m_contacts.SetItemData(m_contacts.AddString(fullName), (DWORD)phoneNumbers);
			else
				delete phoneNumbers;

			q.nextRow();
		}	
	} catch (CppSQLite3Exception& e) { }

	if (bOutlookContactFound==false && contactToDisplay!="" && OutlookPhoneNumbers) {
        m_contacts.SetItemData(m_contacts.AddString(contactToDisplay), (DWORD)OutlookPhoneNumbers);        
		ContactToSelect = contactToDisplay;
	}

	if (m_contacts.GetCount()<=0) {
		//m_contacts.EnableWindow(FALSE);	
		//m_btnOptions.EnableWindow(FALSE);
		//MessageBox("There are no contacts in any of your outlook folders!", APP_NAME, MB_OK);		
	} else {			
		if (ContactToSelect!="") {
			if (m_contacts.SelectString(-1, ContactToSelect)==-1)
				m_contacts.SetCurSel(0);			
		} else {
			m_contacts.SetCurSel(0);
		}
		/*char ch[20];
		itoa(m_contacts.GetCount(), ch, 10);*/
		CString str;
		str.Format(_("%d Contacts"), m_contacts.GetCount());
		//SetWindowText(CString(APP_NAME) + " - " + _("Dial") + " (" + str + ")");
		GetDlgItem(IDC_STATIC_CONTACT)->SetWindowText(_("Contact") + CString(" (") + str + "):");
		OnSelchangeContacts();
	}
}



void COutlookContactsDlg::OnDestroy() 
{
	::theApp.m_pDialForm = NULL;

	std::vector<CString> *phoneNumbers;
	for (int i=0; i<m_contacts.GetCount(); i++) {
		phoneNumbers = (std::vector<CString>*)m_contacts.GetItemData(i);
		if (phoneNumbers!=NULL) {
			delete phoneNumbers;
			phoneNumbers = NULL;
			m_contacts.SetItemData(i, NULL);
		}
	}

	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here	
}


void COutlookContactsDlg::DisplayContactNumbers() {
	m_list.DeleteAllItems();
	CString number, location, data;
	int nIndex, nItem;	
	std::vector<CString> *phoneNumbers = (std::vector<CString>*)m_contacts.GetItemData(m_contacts.GetCurSel());
	if (phoneNumbers!=NULL) {
		
		for (int i=0; i<phoneNumbers->size(); i++) {
			number = "";
			location = "";
			data = (*phoneNumbers)[i];
			nIndex = data.ReverseFind(_T('('));
			if (nIndex!=-1) {
				number = data.Left(nIndex-2);
				location = data.Mid(nIndex+1, data.GetLength()-nIndex-2);
			}
			nItem = m_list.InsertItem(m_list.GetItemCount(), number);
			m_list.SetItemText(nItem, 1, location);
			
		}
		
		m_list.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED | LVIS_FOCUSED);
		if (m_list.GetItemCount()>0) {
			m_phoneNumber.SetWindowText(m_list.GetItemText(0, 0));
			if (m_phoneNumber.GetFocus()) {
				m_phoneNumber.SetSel(0, -1, TRUE);
			}
		} else {
			m_phoneNumber.SetWindowText(_T(""));
		}		
	}
}

void COutlookContactsDlg::OnSelchangeContacts() 
{
	// TODO: Add your control notification handler code here
	DisplayContactNumbers();
}


void COutlookContactsDlg::OnOK() {
	BYTE byBuffer[512] = { 0 };				
	int nLen;
	int nIndex=-1;
	int nSelected;

	CPBXClientApp* pApp = (CPBXClientApp*)AfxGetApp();
	CString packetString;
	
	CString numberToDial;
	CString outgoing_context = pApp->GetProfileString("Settings", "OutgoingContext", "default");
	if (outgoing_context=="")
		outgoing_context="default";

	m_phoneNumber.GetWindowText(numberToDial);
	
	CString extension, channel;
	int protocol = -1;

	if (m_DialFrom.GetCount()==0)
		return;

	m_DialFrom.GetWindowText(extension);
	nIndex = extension.Find(_T("  (SIP)"));
	if (nIndex!=-1) {
        protocol = 0;
		extension = extension.Left(nIndex);
		channel = "SIP/" + extension;
	} else if ((nIndex = extension.Find(_T("  (IAX)")))!=-1) {
		protocol = 1;
		extension = extension.Left(nIndex);
		channel = "IAX2/" + extension;
	} else if ((nIndex = extension.Find(_T("  (SCCP)")))!=-1) {
		protocol = 2;
		extension = extension.Left(nIndex);
		channel = "SCCP/" + extension;
	}	

	/*CString prefix = ::theApp.GetProfileString("Settings", "OutgoingPrefix", "");
	numberToDial = prefix + numberToDial;	

	packetString = "Action: Originate\r\n";
	if (protocol==0)
		packetString += "Channel: SIP/" + extension + "\r\n";
	else if (protocol==1)
		packetString += "Channel: IAX2/" + extension + "\r\n";
	else if (protocol==2)
		packetString += "Channel: SCCP/" + extension + "\r\n";
	
	packetString += "Exten: " + numberToDial + "\r\n";
	packetString += "Context: " + outgoing_context + "\r\n";
	packetString += "Priority: 1\r\n";
	packetString += "CallerID: " + extension + "\r\n";	
	packetString += "ActionID: outcall_dial_action\r\n";	
	packetString += "\r\n";*/
	packetString = ::theApp.BuildOriginateCommand(channel, numberToDial, extension, "ActionID: outcall_dial_action");
			
	//USES_CONVERSION;
	//nLen = packetString.GetLength(); // + 1;
	//strcpy((LPSTR)byBuffer, T2CA(packetString));
	//strcpy((LPSTR)byBuffer, packetString);
	
	if (packetString!=_T("")) {
		m_btnDial.EnableWindow(0);
		socketManager->SendData(packetString);
	}
	
	if (m_bMultipleCalls) {
		if (m_Callees.size()>0) {
			SetWindowText(CString(APP_NAME) + " - " + _("Dial") + " [" + m_Callees[0] + "]");
			m_Callees.erase(m_Callees.begin());
			m_DialFrom.SelectString(-1, m_CallerPhones[0]);
			m_CallerPhones.erase(m_CallerPhones.begin());
			m_phoneNumber.SetWindowText(_T(""));
			m_phoneNumber.SetFocus();
		} else {
			CDialog::OnOK();
		}
	}
}


void COutlookContactsDlg::LoadExtensions() {
	HKEY hKey = HKEY_CURRENT_USER;
	CString sKeyName = REG_KEY_EXTENSIONS;
	LONG retcode;
	HKEY hOpenKey = NULL;
	DWORD dwType = REG_SZ;
	TCHAR str[MAX_REG_KEY_NAME];
	memset( str, '\0', sizeof(str));
	CString userName;
	int nIndex;
	
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
			if ((nIndex=userName.Find(_T("#####")))!=-1)
				userName = userName.Left(nIndex);
			if (value==0)
				m_DialFrom.AddString((userName +  "  (SIP)"));
			else if (value==1)
				m_DialFrom.AddString((userName +  "  (IAX)"));			
			else if (value==2)
				m_DialFrom.AddString((userName +  "  (SCCP)"));			
						
			retCode = ERROR_SUCCESS;
        }
    }

	if(hKey)
		RegCloseKey( hOpenKey );

	m_DialFrom.SetCurSel(0);

	if (m_DialFrom.GetCount()<=1)
		m_DialFrom.EnableWindow(FALSE);
	
	delete[] data;
}


void COutlookContactsDlg::OnBnClickedButtonSkip() {
	if (m_Callees.size()>0) {
		SetWindowText(CString(APP_NAME) + " - " + _("Dial") + " [" + m_Callees[0] + "]");
		m_Callees.erase(m_Callees.begin());
		m_DialFrom.SelectString(-1, m_CallerPhones[0]);
		m_CallerPhones.erase(m_CallerPhones.begin());
		m_phoneNumber.SetWindowText(_T(""));
		m_phoneNumber.SetFocus();
	} else {
		CDialog::OnOK();
	}
}

void COutlookContactsDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	SetWindowPos(&this->wndNoTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

	if (m_LoadContacts) {
		if (::theApp.m_bLoadingContacts) {
			// we must use PostMessage because we want the Dial window to be displayed before we display Wait dialog
			PostMessage(WM_SHOW_LOADING_CONTACTS_DLG_MSG, 0, 0);
		} else {
			LoadContacts();
		}
	}
}


void COutlookContactsDlg::OnSetFocus(CWnd* pOldWnd)
{	
	CDialog::OnSetFocus(pOldWnd);
}


LRESULT COutlookContactsDlg::OnEnableDialButton(WPARAM wParam, LPARAM lParam) {
    m_btnDial.EnableWindow(1);
	return 1;
}


LRESULT COutlookContactsDlg::OnDisplayLoadingContactsDlg(WPARAM wParam, LPARAM lParam) {		
	CLoadingContactsDlg dlg;
	dlg.DoModal();
    LoadContacts();	
	return 1;
}


void COutlookContactsDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);	
	// TODO: Add your message handler code here	
}

void COutlookContactsDlg::OnPopupRefreshFromOutlook()
{	
	/*std::vector<CString> *phoneNumbers;
	for (int i=0; i<m_contacts.GetCount(); i++) {
		phoneNumbers = (std::vector<CString>*)m_contacts.GetItemData(i);
		if (phoneNumbers!=NULL) {
			delete phoneNumbers;
			phoneNumbers = NULL;
		}
	}
	m_contacts.ResetContent();*/
	
	((CMainFrame*)::theApp.m_pMainWnd)->LoadOutlookContacts(NULL);
	CLoadingContactsDlg dlg;
	dlg.DoModal(); //display wait dialog until all contacts from outlook are imported into our database
	LoadContacts();
}


void COutlookContactsDlg::OnPopupRefreshFromCSV()
{	
	/*std::vector<CString> *phoneNumbers;
	for (int i=0; i<m_contacts.GetCount(); i++) {
		phoneNumbers = (std::vector<CString>*)m_contacts.GetItemData(i);
		if (phoneNumbers!=NULL) {
			delete phoneNumbers;
			phoneNumbers = NULL;
		}
	}
	m_contacts.ResetContent();*/
	BOOL bDeleteExisting = 1;
	if (MessageBox(_("Do you want to save your existing contacts?"), APP_NAME, MB_YESNO)==IDNO) {
		bDeleteExisting=2;
	}
		
	((CMainFrame*)::theApp.m_pMainWnd)->LoadOutlookContacts(bDeleteExisting);
	CLoadingContactsDlg dlg;
	dlg.DoModal(); //display wait dialog until all contacts from outlook are imported into our database
	LoadContacts();
}

void COutlookContactsDlg::OnPopupFirstname() {
	displayOptions = 1;
	/*std::vector<CString> *phoneNumbers;
	for (int i=0; i<m_contacts.GetCount(); i++) {
		phoneNumbers = (std::vector<CString>*)m_contacts.GetItemData(i);
		if (phoneNumbers!=NULL) {
			delete phoneNumbers;
			phoneNumbers = NULL;
		}
	}
	m_contacts.ResetContent();*/
	LoadContacts();
}

void COutlookContactsDlg::OnPopupLastname() {
	displayOptions = 2;
	/*std::vector<CString> *phoneNumbers;
	for (int i=0; i<m_contacts.GetCount(); i++) {
		phoneNumbers = (std::vector<CString>*)m_contacts.GetItemData(i);
		if (phoneNumbers!=NULL) {
			delete phoneNumbers;
			phoneNumbers = NULL;
		}
	}
	m_contacts.ResetContent();*/
	LoadContacts();
}

afx_msg void COutlookContactsDlg::OnPopupHideContacts() {
    m_bFilterContacts = !m_bFilterContacts;
	::theApp.WriteProfileInt("Settings", "FilterContacts", m_bFilterContacts?1:0);
	/*std::vector<CString> *phoneNumbers;
	for (int i=0; i<m_contacts.GetCount(); i++) {
		phoneNumbers = (std::vector<CString>*)m_contacts.GetItemData(i);
		if (phoneNumbers!=NULL) {
			delete phoneNumbers;
			phoneNumbers = NULL;
		}
	}
	m_contacts.ResetContent();*/
	LoadContacts();	
}

void COutlookContactsDlg::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	if (pNMLV->uNewState!=3)
		return;

	m_phoneNumber.SetWindowText(m_list.GetItemText(pNMLV->iItem, 0));	

	*pResult = 0;
}

void COutlookContactsDlg::OnBnClickedButtonOptions()
{
	RECT rect;
	CMenu *pMenu = menu.GetSubMenu(1);
	m_btnOptions.GetWindowRect(&rect);
	BOOL b;	

	::theApp.SetMenuItemText(pMenu->GetSubMenu(1), ID_DISPLAY_FIRSTNAME, _("Fi&rst Name Middle Name, Last Name"));
	::theApp.SetMenuItemText(pMenu->GetSubMenu(1), ID_DISPLAY_LASTNAME, _("&Last Name, First name Middle Name"));
	
	::theApp.SetMenuItemText(pMenu, 0, _("Import Contacts"), true);
	::theApp.SetMenuItemText(pMenu->GetSubMenu(0), ID_IMPORTCONTACTS_FROMOUTLOOK, _("From Outlook"));
	::theApp.SetMenuItemText(pMenu->GetSubMenu(0), ID_IMPORTCONTACTS_FROMCSVFILE, _("From CSV file"));

	if (displayOptions==1) {
		b=pMenu->GetSubMenu(1)->CheckMenuItem(ID_DISPLAY_FIRSTNAME, MF_BYCOMMAND | MF_CHECKED);
		b=pMenu->GetSubMenu(1)->CheckMenuItem(ID_DISPLAY_LASTNAME, MF_BYCOMMAND | MF_UNCHECKED);
	} else {
		b=pMenu->GetSubMenu(1)->CheckMenuItem(ID_DISPLAY_LASTNAME, MF_BYCOMMAND | MF_CHECKED);
		b=pMenu->GetSubMenu(1)->CheckMenuItem(ID_DISPLAY_FIRSTNAME, MF_BYCOMMAND | MF_UNCHECKED);
	}
	if (m_bFilterContacts)
		pMenu->CheckMenuItem(ID_POPUP_HIDECONTACTSWITHNOTELEPHONENUMBERS, MF_BYCOMMAND | MF_CHECKED);
	else
		pMenu->CheckMenuItem(ID_POPUP_HIDECONTACTSWITHNOTELEPHONENUMBERS, MF_BYCOMMAND | MF_UNCHECKED);

	::theApp.SetMenuItemText(pMenu, ID_POPUP_HIDECONTACTSWITHNOTELEPHONENUMBERS, _("&Hide contacts with no telephone numbers"));

	pMenu->TrackPopupMenu(TPM_LEFTALIGN, rect.left, rect.bottom, this);
}

void COutlookContactsDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}

void COutlookContactsDlg::OnBnClickedButtonFind()
{
	CFindContactDlg dlg;
	dlg.DoModal();
}

void COutlookContactsDlg::OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (m_list.GetNextItem(-1, LVNI_SELECTED)!=-1) {
		OnOK();		
	}
	*pResult = 0;
}


void COutlookContactsDlg::InitLocaleGUI() {
	GetDlgItem(IDC_STATIC_DIAL_FROM)->SetWindowText(_("Dial from") + CString(":"));
	GetDlgItem(IDC_STATIC_CONTACT)->SetWindowText(_("Contact") + CString(":"));
	GetDlgItem(IDC_STATIC_PHONE)->SetWindowText(_("Phone number") + CString(":"));
	GetDlgItem(IDC_STATIC_PHONE_NUMBERS)->SetWindowText(_("Phone numbers") + CString(":"));
	GetDlgItem(IDC_BUTTON_FIND)->SetWindowText(_("&Find"));
	GetDlgItem(IDC_BUTTON_OPTIONS)->SetWindowText(_("&Options"));
	GetDlgItem(IDC_BUTTON_SKIP)->SetWindowText(_("&Skip"));	
	GetDlgItem(IDOK)->SetWindowText(_("&Dial"));
	GetDlgItem(IDCANCEL)->SetWindowText(_("E&xit"));
}
