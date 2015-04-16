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

// MissedCallsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PBXClient.h"
#include "MissedCallsDlg.h"
#include ".\missedcallsdlg.h"

#include "OutlookContactsDlg.h"
#include "MainFrm.h"
#include "SocketManager.h"

//#include <atlconv.h>

#include <string>
#include "CPPSQLite3.h"


// CMissedCallsDlg dialog

IMPLEMENT_DYNAMIC(CMissedCallsDlg, CDialog)
CMissedCallsDlg::CMissedCallsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMissedCallsDlg::IDD, pParent)
{
}

CMissedCallsDlg::~CMissedCallsDlg()
{
}

void CMissedCallsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_PHONE, m_cboPhone);
	DDX_Control(pDX, IDC_COMBO_DATE, m_cboDate);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Control(pDX, IDC_COMBO_SHOW, m_cboShow);
	DDX_Control(pDX, IDC_STATIC_FRAME, m_frame);
	DDX_Control(pDX, IDC_BUTTON_ADD_CONTACT, m_btnAddContact);
}


BEGIN_MESSAGE_MAP(CMissedCallsDlg, CDialog)
	ON_NOTIFY(LVN_ODSTATECHANGED, IDC_LIST, OnLvnOdstatechangedList)
	ON_NOTIFY(NM_CLICK, IDC_LIST, OnNMClickList)
	ON_NOTIFY(NM_SETFOCUS, IDC_LIST, OnNMSetfocusList)
	ON_CBN_SELCHANGE(IDC_COMBO_PHONE, OnCbnSelchangeComboPhone)
	ON_CBN_SELCHANGE(IDC_COMBO_DATE, OnCbnSelchangeComboDate)
	ON_BN_CLICKED(IDC_BUTTON_CALL, OnBnClickedButtonCall)	
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnBnClickedButtonRemove)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_MESSAGE(WM_REFRESH_MC_LIST, OnMsgRefresh)
	ON_CBN_SELCHANGE(IDC_COMBO_SHOW, OnCbnSelchangeComboShow)
	ON_BN_CLICKED(IDC_BUTTON_ADD_CONTACT, OnBnClickedButtonAddContact)
END_MESSAGE_MAP()


// CMissedCallsDlg message handlers

BOOL CMissedCallsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_list.Init();
	m_cboShow.AddString(_("Missed Calls"));
	m_cboShow.AddString(_("Received Calls"));
	m_cboShow.AddString(_("Placed Calls"));
	m_cboShow.SetCurSel(0);

	m_cboPhone.AddString(_("Show all"));
	LoadExtensions();
	m_cboPhone.SetCurSel(0);
	m_cboDate.AddString(_("Show all"));
	m_cboDate.AddString(_("Today"));
	m_cboDate.SetCurSel(1);

	SetWindowPos(&this->wndNoTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);	

	UpdateList();

	InitLocaleGUI();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CMissedCallsDlg::LoadExtensions() {
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
				m_cboPhone.AddString(userName + "  (SIP)");
			else if (value==1)
				m_cboPhone.AddString(userName + "  (IAX)");
			else if (value==2)
				m_cboPhone.AddString(userName + "  (SCCP)");
			
						
			retCode = ERROR_SUCCESS;
        }
    }

	if(hKey)
		RegCloseKey( hOpenKey );
	
	delete[] data;
	
}
void CMissedCallsDlg::OnLvnOdstatechangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVODSTATECHANGE pStateChanged = reinterpret_cast<LPNMLVODSTATECHANGE>(pNMHDR);
	// TODO: Add your control notification handler code here
	BOOL bEnable = (m_list.GetNextItem(-1, LVNI_SELECTED)!=-1)?TRUE:FALSE;
	GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_CALL)->EnableWindow(bEnable);
	
	m_btnAddContact.EnableWindow(bEnable && (::theApp.GetProfileInt("Settings", "OutlookFeatures", 1)==1));

	*pResult = 0;
}

void CMissedCallsDlg::OnNMClickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	BOOL bEnable = (m_list.GetNextItem(-1, LVNI_SELECTED)!=-1)?TRUE:FALSE;
	GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_CALL)->EnableWindow(bEnable);	
	m_btnAddContact.EnableWindow(bEnable && (::theApp.GetProfileInt("Settings", "OutlookFeatures", 1)==1));

	*pResult = 0;
}

void CMissedCallsDlg::OnNMSetfocusList(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	BOOL bEnable = (m_list.GetNextItem(-1, LVNI_SELECTED)!=-1)?TRUE:FALSE;
	GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_CALL)->EnableWindow(bEnable);	
	m_btnAddContact.EnableWindow(bEnable && (::theApp.GetProfileInt("Settings", "OutlookFeatures", 1)==1));

	*pResult = 0;
}

void CMissedCallsDlg::OnCbnSelchangeComboPhone()
{	
	UpdateList();
}

void CMissedCallsDlg::OnCbnSelchangeComboDate()
{
	UpdateList();
}


void CMissedCallsDlg::UpdateList() {
	CString phone;
	CString sDate;
	time_t long_time;
	struct tm* currentTime;
	int tableSel = m_cboShow.GetCurSel();

	m_cboPhone.GetWindowText(phone);

	if (m_cboDate.GetCurSel()>0) {
		time(&long_time);
		currentTime = localtime(&long_time);
		sDate.Format(_T("%02d.%02d.%d"), currentTime->tm_mday, currentTime->tm_mon + 1, currentTime->tm_year + 1900);
		//sprintf(sDate.GetBuffer(15), "%02d.%02d.%d", currentTime->tm_mday, currentTime->tm_mon + 1, currentTime->tm_year + 1900);
		//sDate.ReleaseBuffer();
	}
	
	if (m_cboPhone.GetCurSel()==0) { //show all
		if (m_cboDate.GetCurSel()==0) {
			if (tableSel==0) {
				m_list.query = "select * from MissedCalls";
				m_list.updateQuery = "update MissedCalls set NewCall=0";
			} else if (tableSel==1) {
                m_list.query = "select * from RecivedCalls";
			} else if (tableSel==2) {
				m_list.query = "select * from PlacedCalls";
			}
		} else {
			if (tableSel==0) {
				m_list.query = "select * from MissedCalls where Date='" + sDate + "'";
				m_list.updateQuery = "update MissedCalls set NewCall=0 where Date='" + sDate + "'";
			} else if (tableSel==1) {
                m_list.query = "select * from RecivedCalls where Date='" + sDate + "'";
			} else if (tableSel==2) {
				m_list.query = "select * from PlacedCalls where Date='" + sDate + "'";
			}
		}
	} else {
		if (tableSel==0) {
			m_list.query = "select * from MissedCalls where Callee='" + phone + "'";
			m_list.updateQuery = "update MissedCalls set NewCall=0 where Callee='" + phone + "'";
			if (m_cboDate.GetCurSel()>0) {
				m_list.query += " and Date='" + sDate + "'";
				m_list.updateQuery += " and Date='" + sDate + "'";
			} 			
		} else if (tableSel==1) {
			if (m_cboDate.GetCurSel()>0)
				m_list.query = "select * from RecivedCalls where Callee='" + phone + "' and Date='" + sDate + "'";
			else
				m_list.query = "select * from RecivedCalls where Callee='" + phone + "'";
		} else if (tableSel==2) {
			if (m_cboDate.GetCurSel()>0)
				m_list.query = "select * from PlacedCalls where CallerID='" + phone + "' and Date='" + sDate + "'";
			else
				m_list.query = "select * from PlacedCalls where CallerID='" + phone + "'";
		}
	}
	m_list.sort_asc=false;
	m_list.sort_column=2;
	if (tableSel==0) {
		m_list.UpdateList();
		CppSQLite3DB db;
		db.open(CStringA(OUTCALL_DB));

		CString callerID, callee, sTime, query;

		int nIndex;
		for (int i=0; i<m_list.GetItemCount(); i++) {
			callerID = m_list.GetItemText(i, 0);
			callee = m_list.GetItemText(i, 1);
			sDate = m_list.GetItemText(i, 2);
			nIndex = sDate.Find(_T(", "));
			sTime = sDate.Mid(nIndex+2);
			sDate = sDate.Mid(0, nIndex);			
			query = "update MissedCalls set NewCall=2 where CallerID='" + callerID + "' and Callee='" + callee + "' and Date='" + sDate +"' and Time='" + sTime + "' and NewCall=1";
			try {
				db.execDML(query.GetBuffer());
			} catch (CppSQLite3Exception& e) { }
		}
	} else {
		m_list.UpdatePlacedRecivedList();	
	}
}


LRESULT CMissedCallsDlg::OnMsgRefresh(WPARAM wParam, LPARAM lParam) {
    UpdateList();
	return 1;
}



void CMissedCallsDlg::OnBnClickedButtonCall()
{
	// TODO: Add your control notification handler code here
	CSocketManager *socketManager = &(((CMainFrame*)::theApp.m_pMainWnd)->m_socketManager);
	if ((socketManager->IsOpen()==FALSE) || (socketManager->authenticated==FALSE)) {
		MessageBox(_("You must be Signed In in order to place calls."), APP_NAME, MB_OK | MB_ICONINFORMATION);
		return;
	}

	if (::theApp.CheckForExistingDialWindow())
		return;

	CString callerID, channel, extension, protocol, packetString;
	CString number1, number2, numberToDial;
	BYTE byBuffer[512] = { 0 };				
	int nLen;
	int nItem, nIndex, nIndex2;
	int tableSel = m_cboShow.GetCurSel();

	CString outgoing_context = ::theApp.GetProfileString("Settings", "OutgoingContext", "default");
	if (outgoing_context=="")
		outgoing_context="default";

	nItem = m_list.GetNextItem(-1, LVNI_SELECTED);
	if (nItem==-1) {
		MessageBox(_("Select some item first!"), APP_NAME, MB_OK | MB_ICONINFORMATION);
		return;
	}
	if (nItem!=-1) {
		if ((tableSel==0) || (tableSel==1))
			channel = m_list.GetItemText(nItem, 1);
		else
			channel = m_list.GetItemText(nItem, 0);

		nIndex = channel.ReverseFind('(');
		protocol = channel.Mid(nIndex+1, channel.GetLength()-nIndex-2);
		extension = channel.Left(nIndex);
		extension = extension.Trim();
		if (protocol.MakeUpper()=="IAX" || protocol.MakeUpper()=="IAX2")
			channel = "IAX2/" + extension;
		else if (protocol.MakeUpper()=="SIP")
			channel = "SIP/" + extension;
		else if (protocol.MakeUpper()=="SCCP")
			channel = "SCCP/" + extension;

		if ((tableSel==0) || (tableSel==1))
			callerID = m_list.GetItemText(nItem, 0);
		else
			callerID = m_list.GetItemText(nItem, 1);

		nIndex = callerID.Find(_T("  ("));
		//nIndex2 = callerID.ReverseFind(_T(")"));
		if (nIndex==-1) {
			number1 = callerID.Trim();
			number2 = number1;
		} else {
			number1 = callerID.Left(nIndex).Trim();
			number2 = callerID.Mid(nIndex+3, callerID.GetLength()-nIndex-4);
		}

		if (IsNumeric(CStringA(number1).GetBuffer())) {
			numberToDial = number1;
		} else if (IsNumeric(CStringA(number2).GetBuffer())) {
			numberToDial = number2;
		} else {
			MessageBox(_T("CallerID is invalid."));
			return;
		}

		CString packetString = ::theApp.BuildOriginateCommand(channel, numberToDial, extension, "ActionID: outcall_dial_action");
		
		if (packetString!=_T("")) {
			socketManager->SendData(packetString);
		}
	}
}


void CMissedCallsDlg::OnBnClickedButtonRemove()
{
	// TODO: Add your control notification handler code here
	CString sDate, sTime, callee, callerID;
	int nItem = m_list.GetNextItem(-1, LVNI_SELECTED);
	if (nItem==-1)
		return;
	if (MessageBox(_("Are you sure you want to delete the selected items?"), APP_NAME, MB_YESNO | MB_ICONQUESTION)==IDYES) {		
		CppSQLite3DB db;
		db.open(CStringA(OUTCALL_DB));			
		
		CString query, table;
		if (m_cboShow.GetCurSel()==0)
			table="MissedCalls";
		else if (m_cboShow.GetCurSel()==1)
			table="RecivedCalls";
		else
			table="PlacedCalls";		

		int nIndex;

		try {

			db.execDML("begin transaction");

			while (nItem!=-1) { //from to time date
				callerID = m_list.GetItemText(nItem, 0);
				callee = m_list.GetItemText(nItem, 1);
				sDate = m_list.GetItemText(nItem, 2);
				nIndex = sDate.Find(_T(", "));
				sTime = sDate.Mid(nIndex+2);
				sDate = sDate.Mid(0, nIndex);

				query = "delete from " + table + " where (CallerID='" + EscapeSQLString(callerID) + "' and Callee='" + 
					EscapeSQLString(callee) + "' and Date='" + sDate +"' and Time='" + sTime + "')";
				db.execDML(query.GetBuffer());
				
				m_list.DeleteItem(nItem);
				nItem--;
				nItem = m_list.GetNextItem(nItem, LVNI_SELECTED);
			}

			db.execDML("end transaction");		

		} catch (CppSQLite3Exception& e) { }
	}
	BOOL bEnable = (m_list.GetNextItem(-1, LVNI_SELECTED)!=-1)?TRUE:FALSE;
	GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_CALL)->EnableWindow(bEnable);	
	m_btnAddContact.EnableWindow(bEnable && (::theApp.GetProfileInt("Settings", "OutlookFeatures", 1)==1));
}

void CMissedCallsDlg::OnCancel()
{
	CppSQLite3DB db;
	db.open(CStringA(OUTCALL_DB));

	try {
		db.execDML("update MissedCalls set NewCall=0 where NewCall=2");	
	} catch (CppSQLite3Exception& e) { }

	CDialog::OnCancel();
}

void CMissedCallsDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CppSQLite3DB db;
	db.open(CStringA(OUTCALL_DB));
	try {
		db.execDML("update MissedCalls set NewCall=0 where NewCall=2");
	} catch (CppSQLite3Exception& e) { }

	OnOK();
}

void CMissedCallsDlg::OnCbnSelchangeComboShow()
{
	int sel = m_cboShow.GetCurSel();
	if (sel==0)
		m_frame.SetWindowText(_("Missed Calls"));
	else if (sel==1)
		m_frame.SetWindowText(_("Received Calls"));
	else if (sel==2)
		m_frame.SetWindowText(_("Placed Calls"));

	UpdateList();
}

void CMissedCallsDlg::OnBnClickedButtonAddContact()
{
	int nItem = m_list.GetNextItem(-1, LVNI_SELECTED);
	int tableSel = m_cboShow.GetCurSel();
	int nIndex;
	double number;
	CString callerID, callerID1="", callerID2="", sNumber="";

	CString phone_number, full_name;	

	while (nItem!=-1) {
		if ((tableSel==0) || (tableSel==1))
			callerID = m_list.GetItemText(nItem, 0);
		else
			callerID = m_list.GetItemText(nItem, 1);

		callerID1 = callerID;
		nIndex = callerID1.Find(_T("  ("));
		if (nIndex!=-1) {	
			callerID1 = callerID1.Left(nIndex);
			callerID2 = callerID.Mid(nIndex+3);
			callerID2 = callerID2.Left(callerID2.GetLength()-1);
		}		

		number = _tstof(callerID1.GetBuffer());
		if (number!=0) {
			sNumber = callerID1;
		} else {
			number = _tstof(callerID2.GetBuffer());
			if (number!=0)
				sNumber = callerID2;
		}

		if (number!=0) {
			phone_number = sNumber;
			if (sNumber==callerID1)
				full_name = callerID2;
			else
				full_name = callerID1;				
		} else {
			full_name = callerID1;
			phone_number = callerID2;
		}

		//try
		//{
		//	_ApplicationPtr pApp; //Outlook Application Instance
		//	pApp.CreateInstance(__uuidof(Application));
		//	IDispatchPtr contactPtr;
		//	_ContactItemPtr cItem;

		//	contactPtr = pApp->CreateItem(olContactItem);
		//	if (contactPtr!=NULL) {
		//		cItem = (_ContactItemPtr)contactPtr;
		//		cItem->put_BusinessTelephoneNumber(phone_number.AllocSysString());
		//		cItem->put_FullName(full_name.AllocSysString());
		//		cItem->Display();	
		//	}			
		//}		
		//catch(_com_error &e)
		//{
		//	TRACE((char*)e.Description());
		//}
		//pApp.Release();
		
		CString *info = new CString(full_name + "###" + phone_number);
		AfxBeginThread(AddNewOutlookContactThreadProc, info);

		break;

		//nItem = m_list.GetNextItem(nItem, LVNI_SELECTED);
	}	
}


void CMissedCallsDlg::InitLocaleGUI() {
	this->SetWindowText(CString(APP_NAME) + " - " + "Call History");

	GetDlgItem(IDC_STATIC_FILTER)->SetWindowText(_("Filter"));
    GetDlgItem(IDC_STATIC_SHOW)->SetWindowText(_("Show") + CString(":"));
	GetDlgItem(IDC_STATIC_PHONE)->SetWindowText(_("Phone") + CString(":"));	
	GetDlgItem(IDC_STATIC_DATE)->SetWindowText(_("Date") + CString(":"));

	GetDlgItem(IDC_STATIC_FRAME)->SetWindowText(_("Missed Calls"));
	GetDlgItem(IDC_BUTTON_CALL)->SetWindowText(_("&Call"));	
	GetDlgItem(IDC_BUTTON_ADD_CONTACT)->SetWindowText(_("&Add Outlook Contact"));
	GetDlgItem(IDC_BUTTON_REMOVE)->SetWindowText(_("&Remove selection"));
	GetDlgItem(IDOK)->SetWindowText(_("&Close"));
}
