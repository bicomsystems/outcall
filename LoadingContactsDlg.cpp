// LoadingContactsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PBXClient.h"
#include "LoadingContactsDlg.h"
#include "MainFrm.h"
#include ".\loadingcontactsdlg.h"

#define TIMER_CHECK_LOADING_STATUS	WM_USER+1

// CLoadingContactsDlg dialog

IMPLEMENT_DYNAMIC(CLoadingContactsDlg, CDialog)
CLoadingContactsDlg::CLoadingContactsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLoadingContactsDlg::IDD, pParent)
{	
}

CLoadingContactsDlg::~CLoadingContactsDlg()
{
}

void CLoadingContactsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLoadingContactsDlg, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CLoadingContactsDlg message handlers

#ifdef _WIN64
void CLoadingContactsDlg::OnTimer(UINT_PTR nIDEvent)
#else
void CLoadingContactsDlg::OnTimer(UINT nIDEvent)
#endif
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent==TIMER_CHECK_LOADING_STATUS) {
		if (::theApp.m_bLoadingContacts==FALSE) {
			KillTimer(TIMER_CHECK_LOADING_STATUS);
			OnOK();
		}
	}
	CDialog::OnTimer(nIDEvent);
}

void CLoadingContactsDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	((CMainFrame*)::theApp.m_pMainWnd)->CancelLoadOutlookContacts();
	//OnCancel();
}

BOOL CLoadingContactsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	SetTimer(TIMER_CHECK_LOADING_STATUS, 500, NULL);
	GetDlgItem(IDC_STATIC_PLEASE_WAIT)->SetWindowText(_("Loading Outlook contacts. This may take some time. Please wait..."));
	GetDlgItem(IDCANCEL)->SetWindowText(_("&Cancel"));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
