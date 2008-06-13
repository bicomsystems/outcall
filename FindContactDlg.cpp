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

// FindContactDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PBXClient.h"
#include "FindContactDlg.h"

#include "MainFrm.h"

#include "CppSQLite3.h"


// CFindContactDlg dialog

IMPLEMENT_DYNAMIC(CFindContactDlg, CDialog)
CFindContactDlg::CFindContactDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFindContactDlg::IDD, pParent)
{
}

CFindContactDlg::~CFindContactDlg()
{
}

void CFindContactDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);	
	DDX_Control(pDX, IDC_LIST_RESULTS, m_list);
	DDX_Control(pDX, IDC_EDIT_SEARCH, m_editQuery);
	DDX_Control(pDX, IDC_COMBO1, m_DialFrom);
	DDX_Control(pDX, IDC_EDIT_PHONE_NUMBER, m_phoneNumber);
	DDX_Control(pDX, IDC_BUTTON_DIAL, m_btnDial);
}


BEGIN_MESSAGE_MAP(CFindContactDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_DIAL, OnBnClickedDial)	
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_NOTIFY(HDN_ITEMDBLCLICK, 0, OnHdnItemdblclickListResults)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_RESULTS, OnNMDblclkListResults)
	ON_EN_CHANGE(IDC_EDIT_SEARCH, OnEnChangeEditSearch)
	ON_NOTIFY(NM_CLICK, IDC_LIST_RESULTS, OnNMClickListResults)

	ON_MESSAGE(WM_ENABLE_DIAL_BUTTON, OnEnableDialButton)
END_MESSAGE_MAP()



LRESULT CFindContactDlg::OnEnableDialButton(WPARAM wParam, LPARAM lParam) {
    m_btnDial.EnableWindow(1);
	return 1;
}

// CFindContactDlg message handlers

void CFindContactDlg::OnBnClickedDial()
{
	BYTE byBuffer[512] = { 0 };				
	CPBXClientApp* pApp = (CPBXClientApp*)AfxGetApp();
	CString packetString;	
	CString numberToDial;
	int nLen;
	int nIndex=-1;
	int nSelected = m_list.GetNextItem(-1, LVNI_SELECTED);

	m_phoneNumber.GetWindowText(numberToDial);

	CString extension, channel;
	int protocol=-1;
	m_DialFrom.GetWindowText(extension);
	if (m_DialFrom.GetCount()==0)
		return;
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
	

	/*CString outgoing_context = pApp->GetProfileString("Settings", "OutgoingContext", "default");
	if (outgoing_context=="")
		outgoing_context="default";
	CString prefix = pApp->GetProfileString("Settings", "OutgoingPrefix", "");
	numberToDial = prefix + numberToDial;*/	

	/*packetString = "Action: Originate\r\n";
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
		
	/*nLen = packetString.GetLength();	
	strcpy((LPSTR)byBuffer, packetString);	*/
	
	if (packetString!=_T("")) {
		m_btnDial.EnableWindow(0);
		((CMainFrame*)(::theApp.m_pMainWnd))->m_socketManager.SendData(packetString);
	}
    
	/*if (((CMainFrame*)(::theApp.m_pMainWnd))->m_socketManager.WriteComm(byBuffer, nLen, INFINITE) <= 0) {		
		MessageBox(_("Some error ocurred while trying to place the call! Try again."), APP_NAME, MB_OK | MB_ICONERROR);	
		m_btnDial.EnableWindow(1);
	}*/
}


void CFindContactDlg::Search() {
	CppSQLite3DB db;
	CppSQLite3Query q;

	if (::theApp.m_bLoadingContacts) {
		MessageBox(_("Synchronizing with Outlook Contacts. Please wait..."));
		return;
	}
    
	db.open(CStringA(OUTCALL_DB));

	CString number, fullName, name, surname, temp, prefix;	
	CString query;
	int i;
	int num, nItem;

	m_list.DeleteAllItems();

	int displayOptions = ::theApp.GetProfileInt("Settings", "DisplaySettings", 1);

	m_editQuery.GetWindowText(query);

	std::string qq;

	if (query.Trim()=="")
		qq = CStringA("SELECT * FROM Contacts");
	else
		qq = CStringA("SELECT * FROM Contacts where FirstName LIKE '" + query + "%' or LastName LIKE '" + query + "%' or CompanyName LIKE '" + query + "%'");
    	
	//q.get_result(qq);

	try {

		q = db.execQuery(qq.c_str());

		while (!q.eof())
		{	
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

			fullName += (" " + CString(q.fieldValue(4)));
			fullName = fullName.TrimLeft();

			for (int i=0; i<=18; i++) {
				number = q.fieldValue(i+6);
				if (number=="")
					continue;
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

				nItem = m_list.InsertItem(m_list.GetItemCount(), fullName);
				m_list.SetItemText(nItem, 1, number);
				m_list.SetItemText(nItem, 2, temp);
				m_list.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED | LVIS_FOCUSED);
			}
			q.nextRow();
		}	
	} catch (CppSQLite3Exception& e) { }
}


void CFindContactDlg::LoadExtensions() {
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


	for (int i = 0, retCode = ERROR_SUCCESS; retCode == ERROR_SUCCESS; i++) 
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
	
	delete[] data;
}


void CFindContactDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

BOOL CFindContactDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowPos(&this->wndNoTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	
	RECT rect;
	m_list.GetClientRect(&rect);
	m_list.InsertColumn(0, _("Name"), LVCFMT_LEFT, rect.right*0.3);
	m_list.InsertColumn(2, _("Phone number"), LVCFMT_LEFT, rect.right*0.3);
	m_list.InsertColumn(3, _("Location"), LVCFMT_LEFT, rect.right*0.3);
	LONG style = GetWindowLong(m_list.GetSafeHwnd(), GWL_STYLE);
	style |= (LVS_EX_FULLROWSELECT | LVS_REPORT | LVS_SHOWSELALWAYS);
	style &= ~LVS_SORTDESCENDING;
	style &= ~LVS_SORTASCENDING;
	SetWindowLong(m_list.GetSafeHwnd(), GWL_STYLE, style);
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	CheckRadioButton(IDC_RADIO1, IDC_RADIO3, IDC_RADIO1);

	LoadExtensions();
	Search();

	m_editQuery.SetFocus();

	InitLocaleGUI();

	return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFindContactDlg::OnHdnItemdblclickListResults(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);	

	*pResult = 0;
}

void CFindContactDlg::OnNMDblclkListResults(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (m_list.GetNextItem(-1, LVNI_SELECTED)!=-1) {
		OnBnClickedDial();		
	}
	*pResult = 0;
}

void CFindContactDlg::OnEnChangeEditSearch()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	Search();
}

void CFindContactDlg::OnNMClickListResults(NMHDR *pNMHDR, LRESULT *pResult)
{
	int nItem = m_list.GetNextItem(-1, LVNI_SELECTED);
	if (nItem==-1)
		return;

	m_phoneNumber.SetWindowText(m_list.GetItemText(nItem, 1));

	*pResult = 0;
}


void CFindContactDlg::InitLocaleGUI() {
	SetWindowText(CString(APP_NAME) + " - " + _("Find"));

	GetDlgItem(IDC_STATIC_DIAL_FROM)->SetWindowText(_("Dial from") + CString(":"));
    GetDlgItem(IDC_STATIC_SEARCH)->SetWindowText(_("Search") + CString(":"));
	GetDlgItem(IDC_STATIC_PHONE)->SetWindowText(_("Phone number") + CString(":"));	
	GetDlgItem(IDC_STATIC_PHONE_NUMBERS)->SetWindowText(_("Phone numbers") + CString(":"));	

	GetDlgItem(IDC_BUTTON_DIAL)->SetWindowText(_("&Dial"));	
	GetDlgItem(IDCANCEL)->SetWindowText(_("&Close"));
}
