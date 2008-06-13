// OutlookRuleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PBXClient.h"
#include "OutlookRuleDlg.h"


// COutlookRuleDlg dialog

IMPLEMENT_DYNAMIC(COutlookRuleDlg, CDialog)
COutlookRuleDlg::COutlookRuleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COutlookRuleDlg::IDD, pParent)
{
	edit = FALSE;
}

COutlookRuleDlg::~COutlookRuleDlg()
{
}

void COutlookRuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PREFIX, m_prefix);
	DDX_Control(pDX, IDC_EDIT_REPLACEMENT, m_replacement);
}


BEGIN_MESSAGE_MAP(COutlookRuleDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

// COutlookRuleDlg message handlers

BOOL COutlookRuleDlg::OnInitDialog() {
    
	CDialog::OnInitDialog();

	if (edit) {
		CString number;		
		nSelected = list->GetNextItem(-1, LVNI_SELECTED);		
		m_prefix.SetWindowText(list->GetItemText(nSelected, 0));
		m_replacement.SetWindowText(list->GetItemText(nSelected, 1));
	}

	m_prefix.SetFocus();

	InitLocaleGUI();

    return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void COutlookRuleDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CString prefix, replacement;
	int nItem;
	m_prefix.GetWindowText(prefix);
	m_replacement.GetWindowText(replacement);

	if (prefix=="") {
		MessageBox(_("Invalid prefix!"), APP_NAME, MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	if (edit) {
		list->SetItem(nSelected, 0, LVIF_TEXT, prefix, 0, 0, 0, 0);
		list->SetItem(nSelected, 1, LVIF_TEXT, replacement, 0, 0, 0, 0);
		delete (CString*)(list->GetItemData(nSelected));
		list->SetItemData(nSelected, (DWORD_PTR)(new CString(prefix)));
	} else {
		nItem = list->InsertItem(list->GetItemCount(), prefix);
		list->SetItem(nItem, 1, LVIF_TEXT, replacement, 0, 0, 0, 0);
		list->SetItemData(nItem, (DWORD_PTR)(new CString(prefix)));
	}

	OnOK();
}


void COutlookRuleDlg::InitLocaleGUI() {
	if (edit)
		SetWindowText(_("Edit Outlook Rule"));
	else
		SetWindowText(_("Add Outlook Rule"));

	GetDlgItem(IDC_STATIC_PREFIX)->SetWindowText(CString(_("&Prefix")) + ":");
    GetDlgItem(IDC_STATIC_REPLACEMENT)->SetWindowText(CString(_("&Replace with")) + ":");
	GetDlgItem(IDOK)->SetWindowText(_("&OK"));
	GetDlgItem(IDCANCEL)->SetWindowText(_("&Cancel"));
}
